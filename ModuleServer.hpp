#ifndef __MODULESERVER__HPP__
#define __MODULESERVER__HPP__

#include <Runnable.hpp>
#include <Pipe.hpp>
#include <Selector.hpp>
#include <list>

class ModuleSkelInterface;
class Server;
class Channel;

class ModuleServer
	: public Runnable
{
	private:
		typedef std::list<Server *> ServerList;
		typedef std::list<Channel *> ClientList;
	private:
		ModuleSkelInterface * module;
		Selector selector;
		ServerList servers;
		ClientList clients;
		bool do_terminate;
		Pipe pipe;
	private:
		bool handle_pipe(Device *);
		bool handle_server(Device *);
		bool handle_channel(Device *);
	protected:
		virtual void run();
		virtual bool terminate() const;
	public:
		ModuleServer(ModuleSkelInterface * = NULL);
		virtual ~ModuleServer();
		void add(Server *);
		void stop();
};

#endif
