#ifndef __LOCALSOCKETSTREAM__HPP__
#define __LOCALSOCKETSTREAM__HPP__

#include <Channel.hpp>
#include <string>
#include <sys/un.h>

class LocalSocketStream : public Channel
{
		friend class LocalSocketStreamServer;
	private:
		std::string path;
		struct sockaddr_un addr;
	private:
		virtual int init(int);
		void init(const std::string &, struct sockaddr_un &);
	public:
		LocalSocketStream();
		LocalSocketStream(const std::string &);
		virtual ~LocalSocketStream();
		virtual int open();
		virtual int close();
		virtual int send(const void *, unsigned int);
		virtual int recv(void *, unsigned int);
};

#endif
