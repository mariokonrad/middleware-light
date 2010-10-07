#include <LocalSocketStream.hpp>
#include <LocalSocketStreamServer.hpp>
#include <Thread.hpp>
#include <Runnable.hpp>
#include <Mutex.hpp>
#include <ConditionVar.hpp>
#include <Selector.hpp>
#include <Pipe.hpp>
#include <list>
#include <memory>
#include <iostream>
#include <Message.hpp>
#include <test.hpp> // generated

#define PING  do { std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl; } while (0)

enum { MAX_BODY_SIZE = 2048 }; // TODO: better way to do this, do not use hardcoded max size

class ModuleServer // {{{
	: public Runnable
{
	private:
		typedef std::list<Server *> ServerList;
		typedef std::list<Channel *> ClientList;
	private:
		test::ModuleSkelInterface * module;
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
		ModuleServer(test::ModuleSkelInterface * = NULL);
		virtual ~ModuleServer();
		void add(Server *);
		void stop();
};

ModuleServer::ModuleServer(test::ModuleSkelInterface * module)
	: module(module)
	, do_terminate(false)
{
	pipe.open();
}

ModuleServer::~ModuleServer()
{
	pipe.close();
	while (clients.size()) {
		delete clients.front();
		clients.pop_front();
	}
}

void ModuleServer::add(Server * server)
{
	servers.push_back(server);
}

void ModuleServer::stop()
{
	do_terminate = true;
	pipe.write(0);
}

bool ModuleServer::handle_pipe(Device * device)
{
	Pipe * pipe = dynamic_cast<Pipe *>(device);
	if (!pipe) return false;
	uint32_t v;
	pipe->read(v);
	return true;
}

bool ModuleServer::handle_server(Device * device)
{
	Server * server = dynamic_cast<Server *>(device);
	if (!server) return false;
	Channel * channel = server->create();
	int rc = server->accept(channel);
	if (rc < 0) {
		if (channel) delete channel;
		std::cerr << "ERROR: cannot accept connection" << std::endl;
		return true;
	}
	selector.add(channel);
	clients.push_back(channel);
	return true;
}

bool ModuleServer::handle_channel(Device * device)
{
	Channel * channel = dynamic_cast<Channel *>(device);
	if (!channel) return false;
	Head head;
	uint8_t buf[MAX_BODY_SIZE];
	int rc = channel->recv(head, buf, sizeof(buf));
	if (rc < 0) {
		std::cerr << "ERROR: cannot read head" << std::endl;
		return true;
	} else if (rc == 0) {
		// client has been disconnected
		selector.remove(device);
		clients.erase(find(clients.begin(), clients.end(), channel));
		delete channel;
		return true;
	} else {
		if (module) {
			rc = module->received(head, buf);
			if (rc < 0) {
				std::cerr << "ERROR: cannot handle message, discarding" << std::endl;
				return true;
			}
		}
	}
	return true;
}

void ModuleServer::run()
{
	selector.add(&pipe);
	for (ServerList::iterator i = servers.begin(); i != servers.end(); ++i) selector.add(*i);

	while (!terminate()) {
		Selector::Devices devices;
		int rc = selector.select(devices);
		if (terminate()) break;
		if (rc < 0) {
			std::cerr << "ERROR: wait on connection" << std::endl;
			return;
		}

		for (Selector::Devices::iterator i = devices.begin(); i != devices.end(); ++i) {
			Device * device = *i;
			if (handle_pipe(device)) continue;
			if (handle_server(device)) continue;
			if (handle_channel(device)) continue;
		}
	}
}

bool ModuleServer::terminate() const
{
	return do_terminate;
}

// }}}

class ModuleSkel // {{{
	: virtual public test::ModuleSkelInterface
	, virtual public Runnable
{
	private:
		struct Entry {
			Entry(const Head & head, const uint8_t * buffer)
				: head(head)
			{
				memcpy(buf, buffer, (head.size > sizeof(buf)) ? sizeof(buf) : head.size);
			}

			Head head;
			uint8_t buf[MAX_BODY_SIZE];
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
		virtual int received(const Head &, const uint8_t *);
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

int ModuleSkel::received(const Head & head, const uint8_t * buf)
{
	int rc = -1;
	mtx.lock();
	if (queue.size() < max_queue_size) {
		rc = 0;
		queue.push_back(Entry(head, buf));
		non_empty.broadcast();
	}
	mtx.unlock();
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

