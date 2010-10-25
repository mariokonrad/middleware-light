#ifndef __MWL__CHANNEL__HPP__
#define __MWL__CHANNEL__HPP__

#include <mwl/Device.hpp>
#include <mwl/Message.hpp>

namespace mwl {

class Server;

class Channel : public Device
{
	protected:
		Server * root_server;
	protected:
		virtual int init(int, Server *) = 0;
	public:
		Channel();
		virtual ~Channel();
		virtual int send(const Head &, const void *, unsigned int) = 0;
		virtual int recv(Head &, void *, unsigned int) = 0;

		static void dispose(Channel *);
};

}

#endif
