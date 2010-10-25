#ifndef __MWL__SERVER__HPP__
#define __MWL__SERVER__HPP__

#include <mwl/Device.hpp>

namespace mwl {

class Channel;

class Server : public Device
{
	public:
		virtual ~Server();
		virtual Channel * create_channel() = 0;
		virtual void dispose_channel(Channel *) = 0;
		virtual int accept(Channel *) = 0;

		static void dispose(Server *);
};

}

#endif
