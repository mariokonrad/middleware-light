#include <mwl/DefaultMessageFactory.hpp>
#include <mwl/Message.hpp>
#include <cstdlib>

namespace mwl {

DefaultMessageFactory::DefaultMessageFactory(unsigned int message_size)
	: message_size(message_size)
{}

DefaultMessageFactory::~DefaultMessageFactory()
{}

Message * DefaultMessageFactory::create_message()
{
	mwl::Message * msg = new mwl::Message;
	msg->size = message_size;
	msg->buf = (msg->size == 0) ? NULL : new uint8_t[msg->size];
	return msg;
}

void DefaultMessageFactory::dispose_message(Message * msg)
{
	if (!msg) return;
	if (msg->buf) {
		delete [] msg->buf;
		msg->buf = NULL;
	}
	delete msg;
}

}

