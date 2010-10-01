#include <LocalSocketStream.hpp>
#include <iostream>
#include <Message.hpp>
#include <test.hpp> // generated

int main(int, char **)
{
	std::cout << "client started" << std::endl;

	LocalSocketStream sock("/tmp/demo.sock");
	if (sock.open() < 0) {
		perror("open");
		return -1;
	}

	Head head;
	head.src = 0;
	head.dst = 0;
	head.type = test::A::TYPE;
	head.size = sizeof(test::A);
	hton(head);

	test::A msg;
	msg.a = 123;
	msg.b = 12345;
	msg.c = 12345678;
	msg.d = 1234567890;
	test::hton(msg);

	uint8_t buf[sizeof(Head)+sizeof(msg)];
	serialize(buf, head);
	test::serialize(buf+sizeof(Head), msg);

	for (int i = 0; i < 3; ++i) {
		int rc = sock.send(buf, sizeof(buf));
		if (rc < 0) {
			std::cerr << "ERROR: rc=" << rc << std::endl;
			perror("write");
			return -1;
		}
	}

	sock.close();
	return 0;
}

