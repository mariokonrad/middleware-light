#include <Channel.hpp>
#include <Server.hpp>
#include <cstdlib>

Channel::Channel()
	: root_server(NULL)
{}

Channel::~Channel()
{}

void Channel::dispose(Channel * channel)
{
	if (!channel) return;
	if (channel->root_server) {
		Server * server = channel->root_server;
		server->dispose_channel(channel);
	} else {
		delete channel; // delete channel if no server is known
	}
}

