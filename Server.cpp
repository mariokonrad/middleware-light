#include <Server.hpp>

Server::~Server()
{}

void Server::dispose(Server * server)
{
	if (server) delete server;
}

