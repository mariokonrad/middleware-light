#include <mwl/Server.hpp>

namespace mwl {

Server::~Server()
{}

void Server::dispose(Server * server)
{
	if (server) delete server;
}

}

