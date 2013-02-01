#include <mwl/LocalSocketStream.hpp>
#include <mwl/Message.hpp>
#include <iostream>
#include <cstdio>
#include <test.hpp> // generated

int main(int, char **)
{
	std::cout << "client started" << std::endl;

	mwl::LocalSocketStream sock("/tmp/demo.sock");
	if (sock.open() < 0) {
		perror("open");
		return -1;
	}

	// send message to server

	mwl::Head head;
	head.src = 0;
	head.dst = 0;
	head.type = test::A::TYPE;
	head.size = sizeof(test::A);

	test::A msg;
	msg.a = 123;
	msg.b = 12345;
	msg.c = 12345678;
	msg.d = 1234567890;
	test::hton(msg);

	uint8_t buf[sizeof(msg)];
	test::serialize(buf, msg);

	for (int i = 0; i < 3; ++i) {
		int rc = sock.send(head, buf, sizeof(buf));
		if (rc < 0) {
			std::cerr << "ERROR: rc=" << rc << std::endl;
			perror("write");
			return -1;
		}
	}

	// let the server terminate

	head.src = 0;
	head.dst = 0;
	head.type = test::shutdown::TYPE;
	head.size = sizeof(test::shutdown);

	test::shutdown term;
	test::hton(term);
	uint8_t buf_term[sizeof(term)];
	test::serialize(buf_term, term);
	int rc = sock.send(head, buf_term, sizeof(buf_term));
	if (rc < 0) {
		std::cerr << "ERROR: rc=" << rc << std::endl;
		perror("write");
		return -1;
	}

	sock.close();
	return 0;
}

