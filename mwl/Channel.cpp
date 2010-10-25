#include <mwl/Channel.hpp>
#include <mwl/Server.hpp>
#include <cstdlib>

namespace mwl {

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

}

