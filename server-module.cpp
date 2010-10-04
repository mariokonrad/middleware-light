#include <LocalSocketStream.hpp>
#include <LocalSocketStreamServer.hpp>
#include <Runnable.hpp>
#include <Selector.hpp>
#include <list>
#include <memory>
#include <iostream>
#include <Message.hpp>
#include <test.hpp> // generated

class ModuleServer : public Runnable
{
	private:
		typedef std::auto_ptr<LocalSocketStreamServer> ServerPtr;
		typedef std::auto_ptr<LocalSocketStream> ClientPtr;
		typedef std::list<LocalSocketStream *> ClientList;
	private:
		Selector selector;
		ServerPtr server;
		ClientList clients;
	protected:
		virtual void run();
		virtual bool terminate() const;
	public:
		ModuleServer();
		virtual ~ModuleServer();
};

ModuleServer::ModuleServer()
{
	const char * sockname = "/tmp/demo.sock";
	unlink(sockname);
	server = ServerPtr(new LocalSocketStreamServer(sockname));
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

	if (!server.get()) return;
	selector.add(server.get());
	for (;;) {
		Device * device = NULL;
		int rc = selector.select(&device);
		if (rc < 0) {
			std::cerr << "ERROR: wait on connection" << std::endl;
			return;
		}

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
		uint8_t head_buf[sizeof(head)];
		rc = s->recv(head_buf, sizeof(head_buf));
		if (rc < 0 || rc != sizeof(head)) {
			std::cerr << "ERROR: cannot read head" << std::endl;
			continue;
		}

		deserialize(head, head_buf);
		ntoh(head);

		enum { MAX_BODY_SIZE = 2048 };

		if (head.size > MAX_BODY_SIZE) {
			std::cerr << "ERROR: body to large" << std::endl;
			continue;
		}

		uint8_t buf[MAX_BODY_SIZE]; 
		rc = s->recv(buf, head.size);
		if ((rc < 0) && (rc != static_cast<int>(head.size))) {
			std::cerr << "ERROR: cannot read body" << std::endl;
			continue;
		}

		// TODO: send [head/body] to module
	}
}

bool ModuleServer::terminate() const
{
	// TODO
	return true;
}




int main(int, char **)
{
}

