#ifndef __MWL__MODULEBASE__HPP__
#define __MWL__MODULEBASE__HPP__

#include <mwl/ModuleBaseInterface.hpp>
#include <mwl/Runnable.hpp>
#include <mwl/ModuleServer.hpp>
#include <mwl/Mutex.hpp>
#include <mwl/ConditionVar.hpp>
#include <list>

namespace mwl {

class Message;
class MessageFactory;
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
		MessageFactory * message_factory;
	private:
		virtual void dispatch_message(Message *) = 0;
	protected:
		void set_max_queue_size(unsigned int);
	public:
		ModuleBase(MessageFactory *);
		virtual ~ModuleBase();
		virtual void run();
		virtual int receive(Channel *);
		void add(Server *, bool = false);
		void set_max_clients(unsigned int);
};

}

#endif
