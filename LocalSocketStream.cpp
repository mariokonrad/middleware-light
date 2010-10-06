#include <LocalSocketStream.hpp>
#include <sys/socket.h>

LocalSocketStream::LocalSocketStream()
{}

LocalSocketStream::LocalSocketStream(const std::string & path)
	: path(path)
{
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path)-1);
}

LocalSocketStream::~LocalSocketStream()
{
	close();
}

void LocalSocketStream::init(const std::string & path, struct sockaddr_un & addr)
{
	this->path = path;
	this->addr = addr;
}

int LocalSocketStream::init(int fd)
{
	close();
	this->fd = fd;
	this->path = "";
	memset(&this->addr, 0, sizeof(this->addr));
	return 0;
}

int LocalSocketStream::open()
{
	if (fd >= 0) return 0;
	fd = ::socket(PF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0) return -1;
	int rc = ::connect(fd, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
	if (rc < 0) {
		close();
		return -1;
	}
	return 0;
}

int LocalSocketStream::close()
{
	if (fd >= 0) {
		::close(fd);
		fd = -1;
	}
	return 0;
}

int LocalSocketStream::send(const void * buf, unsigned int size)
{
	if (fd < 0) return -1;
	if (!buf) return -1;
	if (size == 0) return -1;
	int rc = ::send(fd, buf, size, 0);
	if (rc < 0) return -1;
	return rc;
}

int LocalSocketStream::recv(void * buf, unsigned int size)
{
	if (fd < 0) return -1;
	if (!buf) return -1;
	if (size == 0) return -1;
	int rc = ::recv(fd, buf, size, 0);
	if (rc < 0) return -1;
	return rc;
}

