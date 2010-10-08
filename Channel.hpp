#ifndef __CHANNEL__HPP__
#define __CHANNEL__HPP__

#include <Device.hpp>
#include <Message.hpp>

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

#endif
