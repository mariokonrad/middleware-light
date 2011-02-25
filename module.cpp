#include <mwl/LocalSocketStream.hpp>
#include <mwl/LocalSocketStreamServer.hpp>
#include <mwl/ModuleBase.hpp>
#include <mwl/DefaultMessageFactory.hpp>
#include <iostream>
#include <test.hpp> // generated

#define PING  do { std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl; } while (0)

// TODO: find other solution  for 'dispatch_message'

class Module // {{{
	: public mwl::ModuleBase
	, virtual public test::ModuleInterface
{
	protected:
		bool do_terminate;
	protected:
		virtual void recv(const mwl::Head &, const test::A &);
		virtual void recv(const mwl::Head &, const test::B &);
		virtual void recv(const mwl::Head &, const test::C &);
		virtual void recv(const mwl::Head &, const test::D &);
		virtual void recv(const mwl::Head &, const test::terminate &);
	protected:
		virtual void dispatch_message(mwl::Message *);
	public:
		Module(const std::string &);
		virtual ~Module();
		virtual bool terminate() const;
};

Module::Module(const std::string & sockname)
	: mwl::ModuleBase(new mwl::DefaultMessageFactory(test::MAX_BODY_SIZE))
	, do_terminate(false)
{
	set_max_queue_size(64);
	set_max_clients(0);
	mwl::Server * socket = new mwl::LocalSocketStreamServer(sockname);
	if (socket->open() < 0) {
		mwl::Server::dispose(socket);
		std::cerr << "ERROR: cannot initialize local socket" << std::endl;
		return;
	} else {
		add(socket, true);
	}
}

Module::~Module()
{}

bool Module::terminate() const
{
	return do_terminate;
}

void Module::dispatch_message(mwl::Message * msg)
{
	dispatch(msg->head, msg->buf);
}

void Module::recv(const mwl::Head &, const test::A & m)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": { " << static_cast<int>(m.a) << " " << m.b << " " << m.c << " " << m.d << " }" << std::endl;
}

void Module::recv(const mwl::Head &, const test::B & m)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": { " << m.a << " " << m.b << " }" << std::endl;
}

void Module::recv(const mwl::Head &, const test::C &)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
}

void Module::recv(const mwl::Head &, const test::D &)
{
	std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
}

void Module::recv(const mwl::Head &, const test::terminate &)
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
		mwl::LocalSocketStream conn(SOCKNAME);
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

