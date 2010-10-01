#include <LocalSocketStream.hpp>
#include <iostream>

int main(int, char **)
{
	std::cout << "client started" << std::endl;

	LocalSocketStream sock("/tmp/demo.sock");
	if (sock.open() < 0) {
		perror("open");
		return -1;
	}

	uint32_t data = 123456789;
	int rc = sock.send(&data, sizeof(data));
	if (rc < 0) {
		std::cerr << "ERROR: rc=" << rc << std::endl;
		perror("write");
		return -1;
	}

	sock.close();
	return 0;
}

