#ifndef __LOCALSOCKETSTREAMSERVER__HPP__
#define __LOCALSOCKETSTREAMSERVER__HPP__

#include <string>
#include <sys/un.h>

class LocalSocketStream;

class LocalSocketStreamServer
{
	private:
		int fd;
		std::string path;
		struct sockaddr_un addr;
	public:
		LocalSocketStreamServer(const std::string &);
		virtual ~LocalSocketStreamServer();
		virtual int open();
		virtual int close();
		int accept(LocalSocketStream &);
};

#endif
