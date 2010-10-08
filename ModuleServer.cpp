#include <ModuleServer.hpp>
#include <ModuleSkelInterface.hpp>
#include <Channel.hpp>
#include <Server.hpp>
#include <iostream>

ModuleServer::ModuleServer(ModuleSkelInterface * module)
	: module(module)
	, do_terminate(false)
{
	pipe.open();
}

ModuleServer::~ModuleServer()
{
	pipe.close();
	while (clients.size()) {
		delete clients.front();
		clients.pop_front();
	}
}

void ModuleServer::add(Server * server)
{
	servers.push_back(server);
}

void ModuleServer::stop()
{
	do_terminate = true;
	pipe.write(0);
}

bool ModuleServer::handle_pipe(Device * device)
{
	Pipe * pipe = dynamic_cast<Pipe *>(device);
	if (!pipe) return false;
	uint32_t v;
	pipe->read(v);
	return true;
}

bool ModuleServer::handle_server(Device * device)
{
	Server * server = dynamic_cast<Server *>(device);
	if (!server) return false;
	Channel * channel = server->create();
	int rc = server->accept(channel);
	if (rc < 0) {
		if (channel) delete channel;
		std::cerr << "ERROR: cannot accept connection" << std::endl;
		return true;
	}
	selector.add(channel);
	clients.push_back(channel);
	return true;
}

bool ModuleServer::handle_channel(Device * device)
{
	Channel * channel = dynamic_cast<Channel *>(device);
	if (!channel) return false;
	if (!module) return true;
	int rc = module->receive(channel);
	if (rc < 0) {
		std::cerr << "ERROR: cannot read message" << std::endl;
	} else if (rc == 0) {
		// client has been disconnected
		selector.remove(device);
		clients.erase(find(clients.begin(), clients.end(), channel));
		delete channel;
	}
	return true;
}

void ModuleServer::run()
{
	selector.add(&pipe);
	for (ServerList::iterator i = servers.begin(); i != servers.end(); ++i) selector.add(*i);

	while (!terminate()) {
		Selector::Devices devices;
		int rc = selector.select(devices);
		if (terminate()) break;
		if (rc < 0) {
			std::cerr << "ERROR: wait on connection" << std::endl;
			return;
		}
		for (Selector::Devices::iterator i = devices.begin(); i != devices.end(); ++i) {
			Device * device = *i;
			if (handle_pipe(device)) continue;
			if (handle_server(device)) continue;
			if (handle_channel(device)) continue;
		}
	}
}

bool ModuleServer::terminate() const
{
	return do_terminate;
}

