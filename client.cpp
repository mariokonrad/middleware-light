#include <LocalSocketStream.hpp>
#include <iostream>
#include <test.hpp> // generated

int main(int, char **)
{
	std::cout << "client started" << std::endl;

	LocalSocketStream sock("/tmp/demo.sock");
	if (sock.open() < 0) {
		perror("open");
		return -1;
	}

	test::A msg;
	uint8_t buf[sizeof(msg)];
	msg.a = 123;
	msg.b = 12345;
	msg.c = 12345678;
	msg.d = 1234567890;
	test::hton(msg);
	test::serialize(buf, msg);
	
	int rc = sock.send(buf, sizeof(buf));
	if (rc < 0) {
		std::cerr << "ERROR: rc=" << rc << std::endl;
		perror("write");
		return -1;
	}

	sock.close();
	return 0;
}

