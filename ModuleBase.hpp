#ifndef __MODULEBASE__HPP__
#define __MODULEBASE__HPP__

#include <ModuleBaseInterface.hpp>
#include <Runnable.hpp>
#include <ModuleServer.hpp>
#include <Mutex.hpp>
#include <ConditionVar.hpp>
#include <list>

class Message;
class Server;
class Channel;

class ModuleBase
	: virtual public ModuleBaseInterface
	, virtual public Runnable
{
	private:
		typedef std::list<Message *> Queue;
	private:
		ModuleServer server;
		unsigned int max_queue_size;
		Queue queue;
		Mutex mtx;
		ConditionVar non_empty;
	private:
		virtual void dispatch_message(Message *) = 0;
		virtual Message * create_message() = 0;
		virtual void dispose_message(Message *) = 0;
	protected:
		void set_max_queue_size(unsigned int);
	public:
		ModuleBase();
		virtual ~ModuleBase();
		virtual void run();
		virtual int receive(Channel *);
		void add(Server *, bool = false);
		void set_max_clients(unsigned int);
};

#endif
