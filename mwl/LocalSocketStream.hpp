#ifndef __MWL__LOCALSOCKETSTREAM__HPP__
#define __MWL__LOCALSOCKETSTREAM__HPP__

#include <mwl/Channel.hpp>
#include <string>
#include <sys/un.h>

namespace mwl {

class LocalSocketStream : public Channel
{
		friend class LocalSocketStreamServer;
	private:
		std::string path;
		struct sockaddr_un addr;
	private:
		virtual int init(int, Server *);
		void init(const std::string &, struct sockaddr_un &);
		LocalSocketStream();
	public:
		LocalSocketStream(const std::string &);
		virtual ~LocalSocketStream();
		virtual int open();
		virtual int close();
		virtual int send(const Head &, const void *, unsigned int);
		virtual int recv(Head &, void *, unsigned int);
};

}

#endif
