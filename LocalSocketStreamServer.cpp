#include <LocalSocketStreamServer.hpp>
#include <LocalSocketStream.hpp>
#include <sys/socket.h>

LocalSocketStreamServer::LocalSocketStreamServer(const std::string & path)
	: path(path)
{
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path)-1);
}

LocalSocketStreamServer::~LocalSocketStreamServer()
{
	close();
}

int LocalSocketStreamServer::open()
{
	::unlink(path.c_str());
	fd = ::socket(PF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0) return -1;
	int rc = ::bind(fd, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr));
	if (rc < 0) {
		close();
		return -1;
	}
	rc = ::listen(fd, 0);
	if (rc < 0) {
		close();
		return -1;
	}
	return 0;
}

int LocalSocketStreamServer::close()
{
	if (fd >= 0) {
		::close(fd);
		fd = -1;
	}
	return 0;
}

Channel * LocalSocketStreamServer::create_channel()
{
	return new LocalSocketStream;
}

void LocalSocketStreamServer::dispose_channel(Channel * channel)
{
	if (channel) delete channel;
}

int LocalSocketStreamServer::accept(Channel * ch)
{
	if (fd < 0) return -1;
	if (!ch) return -1;
	LocalSocketStream * lss = dynamic_cast<LocalSocketStream *>(ch);
	if (!lss) return -1;
	struct sockaddr_un addr;
	socklen_t len = sizeof(addr);
	int sock = ::accept(fd, reinterpret_cast<sockaddr *>(&addr), &len);
	if (sock < 0) return -1;
	lss->init(sock, this);
	lss->init(path, addr);
	return 0;
}

