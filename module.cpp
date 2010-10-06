#include <LocalSocketStream.hpp>
#include <LocalSocketStreamServer.hpp>
#include <Thread.hpp>
#include <Runnable.hpp>
#include <Selector.hpp>
#include <list>
#include <memory>
#include <iostream>
#include <Message.hpp>
#include <test.hpp> // generated

#define PING  do { std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl; } while (0)

#define UNUSED_ARG(a)   static_cast<void>(a)

enum { MAX_BODY_SIZE = 2048 };

static const char * SOCKNAME = "/tmp/demo.sock";

class ModuleSkelBase // {{{
{
	public:
		virtual ~ModuleSkelBase() {}
		virtual int received(const Head &, const uint8_t *) = 0;
}; // }}}

class ModuleServer : public Runnable // {{{
{
	private:
		typedef std::auto_ptr<LocalSocketStreamServer> ServerPtr;
		typedef std::auto_ptr<LocalSocketStream> ClientPtr;
		typedef std::list<LocalSocketStream *> ClientList;
	private:
		ModuleSkelBase * module;
		Selector selector;
		ServerPtr server;
		ClientList clients;
	protected:
		virtual void run();
		virtual bool terminate() const;
	public:
		ModuleServer(ModuleSkelBase * = NULL);
		virtual ~ModuleServer();
};

ModuleServer::ModuleServer(ModuleSkelBase * module)
	: module(module)
{
	unlink(SOCKNAME);
	server = ServerPtr(new LocalSocketStreamServer(SOCKNAME));
	if (server->open() < 0) {
		server.reset();
		return;
	}
}

ModuleServer::~ModuleServer()
{
	server.reset();
}

void ModuleServer::run()
{
	// TODO: client connection ending
	// TODO: graceful termination
	// TODO: multiple server

	if (!server.get()) return;
	selector.add(server.get());
	for (;;) {
		Selector::Devices devices;
		int rc = selector.select(devices);
		if (rc < 0) {
			std::cerr << "ERROR: wait on connection" << std::endl;
			return;
		}

		for (Selector::Devices::iterator i = devices.begin(); i != devices.end(); ++i) {
			Device * device = *i;

			if (device == server.get()) {
				ClientPtr s = ClientPtr(new LocalSocketStream);
				rc = server->accept(s.get());
				if (rc < 0) {
					std::cerr << "ERROR: cannot accept connection" << std::endl;
					continue;
				}
				selector.add(s.get());
				clients.push_back(s.release());
				continue;
			}

			LocalSocketStream * s = dynamic_cast<LocalSocketStream *>(device);
			if (!s) {
				std::cerr << "ERROR: unknown object type" << std::endl;
				continue;
			}

			Head head;
			uint8_t buf[sizeof(head)+MAX_BODY_SIZE];
			rc = s->recv(buf, sizeof(buf));
			if (rc < 0) {
				std::cerr << "ERROR: cannot read head" << std::endl;
				continue;
			} else if (rc == 0) {
				// client has been disconnected
				selector.remove(device);
				continue;
			} else {
				if (module) {
					deserialize(head, buf);
					ntoh(head);
					rc = module->received(head, buf+sizeof(head));
					if (rc < 0) {
						std::cerr << "ERROR: cannot handle message, discarding" << std::endl;
						continue;
					}
				}
			}
		}
	}
}

bool ModuleServer::terminate() const
{
	// TODO
	return true;
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
		Queue queue;
		pthread_mutex_t mtx;
		pthread_cond_t non_empty;
	protected:
		bool do_terminate;
	private:
		template <class T> void dispatch(const Head &, const uint8_t *);
	protected:
		virtual void recv(const Head &, const test::A &) = 0;
		virtual void recv(const Head &, const test::B &) = 0;
		virtual void recv(const Head &, const test::C &) = 0;
		virtual void recv(const Head &, const test::D &) = 0;
	public:
		ModuleSkel();
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

ModuleSkel::ModuleSkel()
	: do_terminate(false)
{
	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&non_empty, NULL);
}

ModuleSkel::~ModuleSkel()
{
	pthread_cond_destroy(&non_empty);
	pthread_mutex_destroy(&mtx);
}

void ModuleSkel::run()
{
	ModuleServer server(this);
	Thread server_thread(&server);
	if (server_thread.start()) {
		std::cerr << "ERROR: cannot start server thread" << std::endl;
		return;
	}

	// TODO: graceful and erarly termination of module and module server

	while (!do_terminate) {
		pthread_mutex_lock(&mtx);
		while (queue.empty()) pthread_cond_wait(&non_empty, &mtx);
		Entry entry = queue.front();
		queue.pop_front();
		pthread_mutex_unlock(&mtx);

		switch (entry.head.type) {
			case test::A::TYPE: dispatch<test::A>(entry.head, entry.buf); break;
			case test::B::TYPE: dispatch<test::B>(entry.head, entry.buf); break;
			case test::C::TYPE: dispatch<test::C>(entry.head, entry.buf); break;
			case test::D::TYPE: dispatch<test::D>(entry.head, entry.buf); break;
			default: break; // ingore all unknown
		}
	}

	server_thread.join();
}

int ModuleSkel::received(const Head & head, const uint8_t * buf)
{
	int rc = -1;
	pthread_mutex_lock(&mtx);
	if (queue.size() < MAX_QUEUE_SIZE) {
		rc = 0;
		queue.push_back(Entry(head, buf));
		pthread_cond_broadcast(&non_empty);
	}
	pthread_mutex_unlock(&mtx);
	return rc;
}

// }}}

class Module : public ModuleSkel // {{{
{
	protected:
		virtual void recv(const Head &, const test::A &);
		virtual void recv(const Head &, const test::B &);
		virtual void recv(const Head &, const test::C &);
		virtual void recv(const Head &, const test::D &);
	public:
		virtual ~Module();
		virtual bool terminate() const;
};

Module::~Module()
{}

bool Module::terminate() const
{
	return do_terminate;
}

void Module::recv(const Head &, const test::A &)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
}

void Module::recv(const Head &, const test::B &)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
}

void Module::recv(const Head &, const test::C &)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
}

void Module::recv(const Head &, const test::D &)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
}

// }}}

class ModuleStub // {{{
{
	private:
		typedef std::auto_ptr<LocalSocketStream> ClientPtr;
	private:
		ClientPtr conn;
	public:
		ModuleStub();
		virtual ~ModuleStub();
		template <class T> int send(const T &);
};

ModuleStub::ModuleStub()
{
	conn = ClientPtr(new LocalSocketStream(SOCKNAME));
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

	uint8_t buf[sizeof(Head)+MAX_BODY_SIZE];

	Head head;
	head.src = 0;
	head.dst = 0;
	head.type = T::TYPE;
	head.size = sizeof(T);

	hton(head);
	serialize(buf, head);

	T clone_msg(msg);
	test::hton(clone_msg);
	test::serialize(buf+sizeof(Head), clone_msg);

	return conn->send(buf, sizeof(Head)+sizeof(T));
}

// }}}

int main(int argc, char ** argv)
{
	if (argc != 2) {
		std::cout << "usage: " << argv[0] << " [client|server]" << std::endl;
		return -1;
	}

	std::string type(argv[1]);

	if (type == "client") {
		ModuleStub module;
		test::A msg;
		msg.a = 123;
		msg.b = 12345;
		msg.c = 12345678;
		msg.d = 1234567890;
		module.send(msg);
	} else if (type == "server") {
		Module module;
		module.run();
	} else {
		std::cout << "unknown type '" << argv[1] << "'" << std::endl;
		return -1;
	}
	return 0;
}

