#ifndef __MWL__MESSAGEFACTORY__HPP__
#define __MWL__MESSAGEFACTORY__HPP__

namespace mwl {

class Message;

class MessageFactory
{
	public:
		virtual Message * create_message() = 0;
		virtual void dispose_message(Message *) = 0;
};

}

#endif
