#include <mwl/LocalSocketStream.hpp>
#include <mwl/LocalSocketStreamServer.hpp>
#include <mwl/Runnable.hpp>
#include <mwl/Executor.hpp>
#include <mwl/Selector.hpp>
#include <mwl/Message.hpp>
#include <iostream>
#include <memory>
#include <cstdio>
#include <test.hpp> // generated

class Client : public mwl::Runnable
{
	private:
		mwl::Channel * conn;
		int cnt;
	public:
		Client(mwl::Channel * conn)
			: conn(conn)
			, cnt(2)
		{}

		virtual ~Client()
		{}

		virtual bool terminate() const
		{
std::cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << ":" << std::endl;
			return cnt < 0;
		}

		virtual void run()
		{
			if (!conn) return;
			int rc = -1;

			rc = mwl::Selector::select(*conn);
			if (rc < 0) {
				std::cerr << "ERROR: rc=" << rc << std::endl;
				perror("select");
				cnt = -1;
				return;
			}

			mwl::Head head;
			uint8_t buf[test::MAX_BODY_SIZE];

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

			std::cout
				<< "head:{"
				<< " src=" << head.src
				<< " dst=" << head.dst
				<< " type=" << head.type
				<< " size=" << head.size
				<< " }"
				<< std::endl;

			switch (head.type) {
				case test::shutdown::TYPE:
					cnt = -1;
					break;
				case test::A::TYPE:
					test::A msg;
					test::deserialize(msg, buf);
					test::ntoh(msg);
					std::cout
						<< "msg:{"
						<< " a=" << static_cast<int>(msg.a)
						<< " b=" << msg.b
						<< " c=" << msg.c
						<< " d=" << msg.d
						<< " } "
						<< " cnt=" << cnt
						<< std::endl;
					--cnt;
					break;
			}

		}
};

int main(int, char **)
{
	std::cout << "server started" << std::endl;

	const char * sockname = "/tmp/demo.sock";
	unlink(sockname);

	mwl::LocalSocketStreamServer sock(sockname);
	if (sock.open() < 0) {
		perror("open");
		return -1;
	}

	int rc;
	mwl::Executor exec;
	exec.start();

	rc = mwl::Selector::select(sock);
	if (rc < 0) {
		std::cerr << "ERROR: cannot accept connection" << std::endl;
		perror("select");
		return -1;
	}

	mwl::Channel * conn = sock.create_channel();
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

