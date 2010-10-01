#include <LocalSocketStream.hpp>
#include <LocalSocketStreamServer.hpp>
#include <Runnable.hpp>
#include <Executor.hpp>
#include <Selector.hpp>
#include <iostream>
#include <memory>
#include <test.hpp> // generated

typedef std::auto_ptr<LocalSocketStream> LocalSocketStreamPtr;

class Client : public Runnable
{
	private:
		LocalSocketStreamPtr conn;
	public:
		Client(LocalSocketStreamPtr conn)
			: conn(conn)
		{}

		virtual ~Client()
		{
			conn.reset();
		}
		
		virtual void run()
		{
			if (!conn.get()) return;
			int rc = -1;

			rc = Selector::select(*conn);
			if (rc < 0) {
				std::cerr << "ERROR: rc=" << rc << std::endl;
				perror("select");
				return;
			}

			test::A msg;
			uint8_t buf[sizeof(msg)];

			rc = conn->recv(buf, sizeof(buf));
			if (rc < 0) {
				std::cerr << "ERROR: rc=" << rc << std::endl;
				perror("read");
				return;
			}
			if (rc != sizeof(buf)) {
				std::cerr << "ERROR: size mismatch" << std::endl;
				return;
			}

			test::deserialize(msg, buf);
			test::ntoh(msg);

			std::cout << "msg: {"
				<< " a=" << static_cast<int>(msg.a)
				<< " b=" << msg.b
				<< " c=" << msg.c
				<< " d=" << msg.d
				<< " }" << std::endl;
		}
};

int main(int, char **)
{
	std::cout << "server started" << std::endl;

	const char * sockname = "/tmp/demo.sock";
	unlink(sockname);

	LocalSocketStreamServer sock(sockname);
	if (sock.open() < 0) {
		perror("open");
		return -1;
	}

	int rc;
	Executor exec;
	exec.start();

	LocalSocketStreamPtr conn(new LocalSocketStream);
	rc = Selector::select(sock);
	if (rc < 0) {
		std::cerr << "ERROR: cannot accept connection" << std::endl;
		perror("select");
		return -1;
	}

	rc = sock.accept(conn.get());
	if (rc < 0) {
		std::cerr << "ERROR: cannot accept connection" << std::endl;
		return -1;
	}

	exec.execute(new Client(conn), true);
	exec.execute(NULL);
	exec.join();

	sock.close();
	return 0;
}

