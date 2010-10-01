#include <LocalSocketStream.hpp>
#include <LocalSocketStreamServer.hpp>
#include <Runnable.hpp>
#include <Executor.hpp>
#include <iostream>
#include <memory>

typedef std::auto_ptr<LocalSocketStream> LocalSocketStreamPtr;

class Server : public Runnable
{
	private:
		LocalSocketStreamPtr conn;
	public:
		Server(LocalSocketStreamPtr conn)
			: conn(conn)
		{}

		virtual ~Server()
		{
			conn.reset();
		}
		
		virtual void run()
		{
			if (!conn.get()) return;
			uint32_t data = 0;
			int rc = conn->recv(&data, sizeof(data));
			if (rc < 0) {
				std::cerr << "ERROR: rc=" << rc << std::endl;
				perror("read");
				return;
			}
			std::cout << "data: " << data << std::endl;
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

	Executor exec;
	exec.start();

	LocalSocketStreamPtr conn(new LocalSocketStream);
	int rc = sock.accept(*conn);
	if (rc < 0) {
		std::cerr << "ERROR: cannot accept connection" << std::endl;
		return -1;
	}

	exec.execute(new Server(conn), true);
	exec.execute(NULL);
	exec.join();

	sock.close();
	return 0;
}

