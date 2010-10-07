#include <LocalSocketStream.hpp>
#include <LocalSocketStreamServer.hpp>
#include <Thread.hpp>
#include <Runnable.hpp>
#include <Mutex.hpp>
#include <ConditionVar.hpp>
#include <Selector.hpp>
#include <list>
#include <memory>
#include <iostream>
#include <Message.hpp>
#include <test.hpp> // generated

#define PING  do { std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl; } while (0)

enum { MAX_BODY_SIZE = 2048 };

class ModuleSkelBase // {{{
{
	public:
		virtual ~ModuleSkelBase() {}
		virtual int received(const Head &, const uint8_t *) = 0;
}; // }}}

class ModuleServer : public Runnable // {{{
{
	private:
		typedef std::list<Server *> ServerList;
		typedef std::list<Channel *> ClientList;
	private:
		ModuleSkelBase * module;
		Selector selector;
		ServerList servers;
		ClientList clients;
		bool do_terminate;
	protected:
		virtual void run();
		virtual bool terminate() const;
	public:
		ModuleServer(ModuleSkelBase * = NULL);
		virtual ~ModuleServer();
		void add(Server *);
		void stop();
};

ModuleServer::ModuleServer(ModuleSkelBase * module)
	: module(module)
	, do_terminate(false)
{}

ModuleServer::~ModuleServer()
{
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
	// TODO: graceful termination (how to wake up selector?)
}

void ModuleServer::run()
{
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

			Server * server = dynamic_cast<Server *>(device);
			if (server ) {
				Channel * channel = server->create();
				rc = server->accept(channel);
				if (rc < 0) {
					if (channel) delete channel;
					std::cerr << "ERROR: cannot accept connection" << std::endl;
					continue;
				}
				selector.add(channel);
				clients.push_back(channel);
				continue;
			}

			Channel * channel = dynamic_cast<Channel *>(device);
			if (channel) {
				Head head;
				uint8_t buf[MAX_BODY_SIZE];
				rc = channel->recv(head, buf, sizeof(buf));
				if (rc < 0) {
					std::cerr << "ERROR: cannot read head" << std::endl;
					continue;
				} else if (rc == 0) {
					// client has been disconnected
					selector.remove(device);
					clients.erase(find(clients.begin(), clients.end(), channel));
					delete channel;
					continue;
				} else {
					if (module) {
						rc = module->received(head, buf);
						if (rc < 0) {
							std::cerr << "ERROR: cannot handle message, discarding" << std::endl;
							continue;
						}
					}
				}
			}
		}
	}
}

bool ModuleServer::terminate() const
{
	return do_terminate;
}

// }}}

class ModuleSkel // {{{
	: virtual public ModuleSkelBase
	, virtual public Runnable
{
	private:
		enum { MAX_QUEUE_SIZE = 64 };
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
		std::string sockname;
		Queue queue;
		Mutex mtx;
		ConditionVar non_empty;
	private:
		template <class T> void dispatch(const Head &, const uint8_t *);
	protected:
		virtual void recv(const Head &, const test::A &) = 0;
		virtual void recv(const Head &, const test::B &) = 0;
		virtual void recv(const Head &, const test::C &) = 0;
		virtual void recv(const Head &, const test::D &) = 0;
		virtual void recv(const Head &, const test::terminate &) = 0;
	public:
		ModuleSkel(const std::string &);
		virtual ~ModuleSkel();
		virtual void run();
		virtual int received(const Head &, const uint8_t *);
};

template <class T> void ModuleSkel::dispatch(const Head & head, const uint8_t * buf)
{
	T msg;
	test::deserialize(msg, buf);
	test::ntoh(msg);
	recv(head, msg);
}

ModuleSkel::ModuleSkel(const std::string & sockname)
	: sockname(sockname)
{}

ModuleSkel::~ModuleSkel()
{}

void ModuleSkel::run()
{
	ModuleServer server(this);

	LocalSocketStreamServer socket(sockname);
	if (socket.open() < 0) {
		std::cerr << "ERROR: cannot initialize local socket" << std::endl;
		return;
	}
	server.add(&socket);

	Thread server_thread(&server);
	if (server_thread.start()) {
		std::cerr << "ERROR: cannot start server thread" << std::endl;
		return;
	}

	// TODO: graceful and erarly termination of module and module server

	while (!terminate()) {
		mtx.lock();
		while (queue.empty()) non_empty.wait(mtx);
		Entry entry = queue.front();
		queue.pop_front();
		mtx.unlock();

		switch (entry.head.type) {
			case test::A::TYPE: dispatch<test::A>(entry.head, entry.buf); break;
			case test::B::TYPE: dispatch<test::B>(entry.head, entry.buf); break;
			case test::C::TYPE: dispatch<test::C>(entry.head, entry.buf); break;
			case test::D::TYPE: dispatch<test::D>(entry.head, entry.buf); break;
			case test::terminate::TYPE: dispatch<test::terminate>(entry.head, entry.buf); break;
			default: break; // ingore all unknown
		}
	}
	server.stop();
	server_thread.join();
}

int ModuleSkel::received(const Head & head, const uint8_t * buf)
{
	int rc = -1;
	mtx.lock();
	if (queue.size() < MAX_QUEUE_SIZE) {
		rc = 0;
		queue.push_back(Entry(head, buf));
		non_empty.broadcast();
	}
	mtx.unlock();
	return rc;
}

// }}}

class Module : public ModuleSkel // {{{
{
	protected:
		bool do_terminate;
	protected:
		virtual void recv(const Head &, const test::A &);
		virtual void recv(const Head &, const test::B &);
		virtual void recv(const Head &, const test::C &);
		virtual void recv(const Head &, const test::D &);
		virtual void recv(const Head &, const test::terminate &);
	public:
		Module(const std::string &);
		virtual ~Module();
		virtual bool terminate() const;
};

Module::Module(const std::string & sockname)
	: ModuleSkel(sockname)
	, do_terminate(false)
{}

Module::~Module()
{}

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

class ModuleStub // {{{
{
	private:
		typedef std::auto_ptr<Channel> ClientPtr;
	private:
		ClientPtr conn;
	public:
		ModuleStub(const std::string &);
		virtual ~ModuleStub();
		template <class T> int send(const T &);
};

ModuleStub::ModuleStub(const std::string & sockname)
{
	conn = ClientPtr(new LocalSocketStream(sockname));
	if (conn->open()) {
		std::cerr << "cannot open connection to module" << std::endl;
		conn.reset();
	}
}

ModuleStub::~ModuleStub()
{}

template <class T> int ModuleStub::send(const T & msg)
{
	if (!conn.get()) return -1;

	uint8_t buf[sizeof(T)];
	T clone_msg(msg);
	test::hton(clone_msg);
	test::serialize(buf, clone_msg);

	return conn->send(Head(0, 0, T::TYPE, sizeof(T)), buf, sizeof(T));
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
		ModuleStub module(SOCKNAME);

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

