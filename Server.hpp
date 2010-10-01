#ifndef __SERVER__HPP__
#define __SERVER__HPP__

#include <Device.hpp>

class Channel;

class Server : public Device
{
	public:
		virtual ~Server() {}
		virtual int accept(Channel *) = 0;
};

#endif
