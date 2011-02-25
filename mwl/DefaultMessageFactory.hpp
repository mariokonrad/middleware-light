#ifndef __MWL__DEFAULTMESSAGEFACTORY__HPP__
#define __MWL__DEFAULTMESSAGEFACTORY__HPP__

#include <mwl/MessageFactory.hpp>

namespace mwl {

class DefaultMessageFactory : public MessageFactory
{
	private:
		unsigned int message_size;
	public:
		DefaultMessageFactory(unsigned int);
		virtual ~DefaultMessageFactory();
		virtual Message * create_message();
		virtual void dispose_message(Message *);
};

}

#endif
