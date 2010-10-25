#ifndef __MWL__LOCALSOCKETSTREAMSERVER__HPP__
#define __MWL__LOCALSOCKETSTREAMSERVER__HPP__

#include <mwl/Server.hpp>
#include <string>
#include <sys/un.h>

namespace mwl {

class LocalSocketStreamServer : public Server
{
	private:
		std::string path;
		struct sockaddr_un addr;
	public:
		LocalSocketStreamServer(const std::string &);
		virtual ~LocalSocketStreamServer();
		virtual int open();
		virtual int close();
		virtual Channel * create_channel();
		virtual void dispose_channel(Channel *);
		virtual int accept(Channel *);
};

}

#endif
