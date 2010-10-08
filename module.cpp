#include <LocalSocketStream.hpp>
#include <LocalSocketStreamServer.hpp>
#include <Thread.hpp>
#include <Runnable.hpp>
#include <Mutex.hpp>
#include <ConditionVar.hpp>
#include <ModuleSkelInterface.hpp>
#include <ModuleServer.hpp>
#include <list>
#include <memory>
#include <iostream>
#include <Message.hpp>
#include <test.hpp> // generated

#define PING  do { std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl; } while (0)

class ModuleSkel // {{{
	: virtual public ModuleSkelInterface
	, virtual public Runnable
{
	private:
		struct Entry {
			Entry() {}

			Entry(const Head & head, const uint8_t * buffer)
				: head(head)
			{
				memcpy(buf, buffer, (head.size > sizeof(buf)) ? sizeof(buf) : head.size);
			}

			Head head;
			uint8_t buf[test::MAX_BODY_SIZE];
		};
		typedef std::list<Entry> Queue;
	private:
		ModuleServer server;
		unsigned int max_queue_size;
		Queue queue;
		Mutex mtx;
		ConditionVar non_empty;
	private:
		virtual void dispatch_message(const Head &, const uint8_t *) = 0;
	public:
		ModuleSkel(unsigned int);
		virtual ~ModuleSkel();
		virtual void run();
		virtual int receive(Channel *);
		void add(Server *);
};

ModuleSkel::ModuleSkel(unsigned int max_queue_size)
	: server(this)
	, max_queue_size(max_queue_size)
{}

ModuleSkel::~ModuleSkel()
{}

void ModuleSkel::add(Server * server)
{
	this->server.add(server);
}

void ModuleSkel::run()
{
	Thread server_thread(&server);
	if (server_thread.start()) {
		std::cerr << "ERROR: cannot start server thread" << std::endl;
		return;
	}

	// TODO: erarly termination of module

	while (!terminate()) {
		mtx.lock();
		while (queue.empty()) non_empty.wait(mtx);
		Entry entry = queue.front();
		queue.pop_front();
		mtx.unlock();
		dispatch_message(entry.head, entry.buf);

	}
	server.stop();
	server_thread.join();
}

int ModuleSkel::receive(Channel * channel)
{
	if (!channel) return -1;
	Entry entry;
	int rc = channel->recv(entry.head, entry.buf, sizeof(entry.buf));
	if (rc > 0) {
		bool failure = true;
		mtx.lock();
		if (queue.size() < max_queue_size) {
			failure = false;
			queue.push_back(entry);
			non_empty.broadcast();
		}
		mtx.unlock();
		if (failure) std::cerr << "ERROR: cannot handle message, discarding" << std::endl;
	}
	return rc;
}

// }}}

class Module // {{{
	: public ModuleSkel
	, virtual public test::ModuleInterface
{
	private:
		typedef std::auto_ptr<Server> ServerPtr;
	private:
		ServerPtr socket;
	protected:
		bool do_terminate;
	protected:
		virtual void recv(const Head &, const test::A &);
		virtual void recv(const Head &, const test::B &);
		virtual void recv(const Head &, const test::C &);
		virtual void recv(const Head &, const test::D &);
		virtual void recv(const Head &, const test::terminate &);
	protected:
		virtual void dispatch_message(const Head & head, const uint8_t * buf)
		{ dispatch(head, buf); }
	public:
		Module(const std::string &);
		virtual ~Module();
		virtual bool terminate() const;
};

Module::Module(const std::string & sockname)
	: ModuleSkel(64)
	, do_terminate(false)
{
	socket = ServerPtr(new LocalSocketStreamServer(sockname));
	if (socket->open() < 0) {
		std::cerr << "ERROR: cannot initialize local socket" << std::endl;
		return;
	}
	add(socket.get());
}

Module::~Module()
{
	socket.reset();
}

bool Module::terminate() const
{
	return do_terminate;
}

void Module::recv(const Head &, const test::A & m)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": { " << static_cast<int>(m.a) << " " << m.b << " " << m.c << " " << m.d << " }" << std::endl;
}

void Module::recv(const Head &, const test::B & m)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": { " << m.a << " " << m.b << " }" << std::endl;
}

void Module::recv(const Head &, const test::C &)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
}

void Module::recv(const Head &, const test::D &)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
}

void Module::recv(const Head &, const test::terminate &)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
	do_terminate = true;
}

// }}}

int main(int argc, char ** argv)
{
	static const char * SOCKNAME = "/tmp/demo.sock";

	if (argc != 2) {
		std::cout << "usage: " << argv[0] << " [client|server]" << std::endl;
		return -1;
	}

	std::string type(argv[1]);

	if (type == "client") {
		LocalSocketStream conn(SOCKNAME);
		if (conn.open()) {
			std::cerr << "cannot open connection to module" << std::endl;
			return -1;
		}

		test::ModuleStub module(&conn);

		test::A msg_a;
		msg_a.a = 123;
		msg_a.b = 12345;
		msg_a.c = 12345678;
		msg_a.d = 1234567890;

		test::B msg_b;
		msg_b.a = 3.141f;
		msg_b.b = 2.718;

		test::terminate msg_term;

		module.send(msg_a);
		module.send(msg_b);
		module.send(msg_a);
		module.send(msg_term);
	} else if (type == "server") {
		Module module(SOCKNAME);
		module.run();
	} else {
		std::cout << "unknown type '" << argv[1] << "'" << std::endl;
		return -1;
	}
	return 0;
}

