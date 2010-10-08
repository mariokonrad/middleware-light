#include <ModuleServer.hpp>
#include <ModuleBaseInterface.hpp>
#include <Channel.hpp>
#include <Server.hpp>
#include <iostream>

ModuleServer::ModuleServer(ModuleBaseInterface * module)
	: module(module)
	, do_terminate(false)
	, max_clients(0)
{
	pipe.open();
}

ModuleServer::~ModuleServer()
{
	pipe.close();
	while (clients.size()) {
		Channel * channel = clients.front();
		clients.pop_front();
		selector.remove(channel);
		Channel::dispose(channel);
	}
	while (servers.size()) {
		ServerListEntry entry = servers.front();
		servers.pop_front();
		selector.remove(entry.server);
		if (entry.auto_destroy) Server::dispose(entry.server);
	}
}

void ModuleServer::add(Server * server, bool auto_destroy)
{
	servers.push_back(ServerListEntry(server, auto_destroy));
}

void ModuleServer::set_max_clients(unsigned int max_clients)
{
	this->max_clients = max_clients;
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
	Channel * channel = server->create_channel();
	int rc = server->accept(channel);
	if (rc < 0) {
		server->dispose_channel(channel);
		std::cerr << "ERROR: cannot accept connection" << std::endl;
		return true;
	}

	if ((max_clients > 0) && (clients.size() >= max_clients)) {
		std::cerr << "ERROR: too many connections" << std::endl;
		server->dispose_channel(channel);
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
		Channel::dispose(channel);
	}
	return true;
}

void ModuleServer::run()
{
	selector.add(&pipe);
	for (ServerList::iterator i = servers.begin(); i != servers.end(); ++i) selector.add(i->server);

	Selector::Devices devices;
	devices.reserve(servers.size() + 16); // reserve space for 16 connections, growing if more
	while (!terminate()) {
		devices.clear();
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

