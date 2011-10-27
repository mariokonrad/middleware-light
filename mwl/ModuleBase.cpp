#include <mwl/ModuleBase.hpp>
#include <mwl/Thread.hpp>
#include <mwl/Message.hpp>
#include <mwl/MessageFactory.hpp>
#include <mwl/Channel.hpp>
#include <iostream>

namespace mwl {

ModuleBase::ModuleBase(MessageFactory * message_factory)
	: server(this)
	, max_queue_size(1)
	, message_factory(message_factory)
{}

ModuleBase::~ModuleBase()
{
	if (message_factory) {
		delete message_factory;
		message_factory = NULL;
	}
}

void ModuleBase::set_max_queue_size(unsigned int size)
{
	if (size > 0) max_queue_size = size;
}

void ModuleBase::add(Server * server, bool auto_destroy)
{
	this->server.add(server, auto_destroy);
}

void ModuleBase::set_max_clients(unsigned int max_clients)
{
	server.set_max_clients(max_clients);
}

void ModuleBase::run()
{
	if (!message_factory) {
		std::cerr << "ERROR: no message factory defined" << std::endl;
		return;
	}

	Thread server_thread(&server);
	if (server_thread.start()) {
		std::cerr << "ERROR: cannot start server thread" << std::endl;
		return;
	}

	// TODO: erarly termination of module

	while (!terminate()) {
		mtx.lock();
		while (queue.empty()) non_empty.wait(mtx);
		Message * msg = queue.front();
		queue.pop_front();
		mtx.unlock();
		dispatch_message(msg);
		message_factory->dispose_message(msg);
	}
	server.stop();
	server_thread.join();

	while (queue.size()) {
		message_factory->dispose_message(queue.front());
		queue.pop_front();
	}
}

int ModuleBase::receive(Channel * channel)
{
	if (!channel) return -1;
	if (!message_factory) return -2;
	Message * msg = message_factory->create_message();
	if (!msg) {
		std::cerr << "ERROR: cannot create message" << std::endl;
		return -3;
	}
	int rc = channel->recv(msg->head, msg->buf, msg->size);
	if (rc > 0) {
		bool success = false;
		mtx.lock();
		if (queue.size() < max_queue_size) {
			success = true;
			queue.push_back(msg);
			non_empty.broadcast();
		}
		mtx.unlock();
		if (success) return rc;
		std::cerr << "ERROR: cannot handle message, discarding" << std::endl;
	}
	message_factory->dispose_message(msg);
	return rc;
}

}

