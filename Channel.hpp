#ifndef __CHANNEL__HPP__
#define __CHANNEL__HPP__

#include <Device.hpp>
#include <Message.hpp>

class Channel : public Device
{
	protected:
		virtual int init(int) = 0;
	public:
		virtual ~Channel() {}
		virtual int send(const Head &, const void *, unsigned int) = 0;
		virtual int recv(Head &, void *, unsigned int) = 0;
};

#endif
