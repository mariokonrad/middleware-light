#include <LocalSocketStream.hpp>
#include <LocalSocketStreamServer.hpp>
#include <Runnable.hpp>
#include <Executor.hpp>
#include <Selector.hpp>
#include <iostream>
#include <memory>
#include <Message.hpp>
#include <test.hpp> // generated

class Client : public Runnable
{
	private:
		Channel * conn;
		int cnt;
	public:
		Client(Channel * conn)
			: conn(conn)
			, cnt(2)
		{}

		virtual ~Client()
		{}
		
		virtual bool terminate() const
		{
			return cnt < 0;
		}

		virtual void run()
		{
			if (!conn) return;
			int rc = -1;

			rc = Selector::select(*conn);
			if (rc < 0) {
				std::cerr << "ERROR: rc=" << rc << std::endl;
				perror("select");
				cnt = -1;
				return;
			}

			Head head;
			test::A msg;
			uint8_t buf[sizeof(msg)];

/*
			rc = conn->recv(buf, sizeof(head));
std::cerr << "rc=" << rc << std::endl;
			rc = conn->recv(buf, sizeof(msg));
std::cerr << "rc=" << rc << std::endl;
*/

			rc = conn->recv(head, buf, sizeof(buf));
			if (rc == 0) {
				std::cerr << "ERROR: connection closed by peer, cnt=" << cnt << std::endl;
				cnt = -1;
				return;
			} else if (rc < 0) {
				std::cerr << "ERROR: rc=" << rc << std::endl;
				perror("read");
				cnt = -1;
				return;
			}
			if (rc != sizeof(buf)) {
				std::cerr << "ERROR: size mismatch, rc=" << rc << " expected=" << sizeof(buf) << std::endl;
				cnt = -1;
				return;
			}

			test::deserialize(msg, buf);

			test::ntoh(msg);

			std::cout
				<< "head:{"
				<< " src=" << head.src
				<< " dst=" << head.dst
				<< " type=" << head.type
				<< " size=" << head.size
				<< " } "
				<< "msg:{"
				<< " a=" << static_cast<int>(msg.a)
				<< " b=" << msg.b
				<< " c=" << msg.c
				<< " d=" << msg.d
				<< " } "
				<< " cnt=" << cnt
				<< std::endl;

			--cnt;
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

	rc = Selector::select(sock);
	if (rc < 0) {
		std::cerr << "ERROR: cannot accept connection" << std::endl;
		perror("select");
		return -1;
	}

	Channel * conn = sock.create_channel();
	rc = sock.accept(conn);
	if (rc < 0) {
		std::cerr << "ERROR: cannot accept connection" << std::endl;
		return -1;
	}

	exec.execute(new Client(conn), true);
//	exec.execute(NULL);
	exec.join();
	sock.dispose_channel(conn);

	sock.close();
	return 0;
}

