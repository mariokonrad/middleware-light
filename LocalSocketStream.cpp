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

int LocalSocketStream::init(int fd, Server * root_server)
{
	close();
	this->root_server = root_server;
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

int LocalSocketStream::send(const Head & head, const void * buf, unsigned int size)
{
	if (fd < 0) return -1;
	if (!buf) return -1;
	if (size == 0) return -1;

	Head clone_head(head);
	uint8_t buf_head[sizeof(head)];
	hton(clone_head);
	serialize(buf_head, clone_head);

	int rc = ::write(fd, buf_head, sizeof(buf_head));
	if (rc < 0) return -1;

	rc = ::write(fd, buf, size);
	if (rc < 0) return -1;
	return rc;
}

int LocalSocketStream::recv(Head & head, void * buf, unsigned int size)
{
	if (fd < 0) return -1;
	if (!buf) return -1;
	if (size == 0) return -1;

	uint8_t buf_head[sizeof(head)];
	int rc = ::read(fd, buf_head, sizeof(buf_head));
	if (rc < 0) return -1;
	if (rc == 0) return 0;
	deserialize(head, buf_head);
	ntoh(head);

	if (head.size > size) return -2;

	rc = ::read(fd, buf, head.size);
	if (rc < 0) return -3;
	return rc;
}

