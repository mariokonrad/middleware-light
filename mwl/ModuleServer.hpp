#ifndef __MWL__MODULESERVER__HPP__
#define __MWL__MODULESERVER__HPP__

#include <mwl/Runnable.hpp>
#include <mwl/Pipe.hpp>
#include <mwl/Selector.hpp>
#include <list>
#include <cstdlib>

namespace mwl {

class ModuleBaseInterface;
class Server;
class Channel;

class ModuleServer
	: public Runnable
{
	private:
		struct ServerListEntry
		{
			ServerListEntry(Server * server, bool auto_destroy = false)
				: server(server)
				, auto_destroy(auto_destroy)
			{}

			Server * server;
			bool auto_destroy;
		};
		typedef std::list<ServerListEntry> ServerList;
		typedef std::list<Channel *> ClientList;
	private:
		ModuleBaseInterface * module;
		Selector selector;
		ServerList servers;
		ClientList clients;
		bool do_terminate;
		Pipe pipe;
		unsigned int max_clients;
	private:
		bool handle_pipe(Device *);
		bool handle_server(Device *);
		bool handle_channel(Device *);
	protected:
		virtual void run();
		virtual bool terminate() const;
	public:
		ModuleServer(ModuleBaseInterface * = NULL);
		virtual ~ModuleServer();
		void add(Server *, bool = false);
		void set_max_clients(unsigned int);
		void stop();
};

}

#endif
