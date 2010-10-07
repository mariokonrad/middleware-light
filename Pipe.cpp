#include <Pipe.hpp>
#include <unistd.h>

Pipe::Pipe()
	: fd_write(-1)
{}

Pipe::~Pipe()
{
	close();
}

int Pipe::open()
{
	int d[2];
	int rc = ::pipe(d);
	if (rc < 0) return -1;
	fd = d[0];
	fd_write = d[1];
	return 0;
}

int Pipe::close()
{
	if (fd_write >= 0) {
		::close(fd_write);
		fd_write = -1;
	}
	if (fd >= 0) {
		::close(fd);
		fd = -1;
	}
	return 0;
}

int Pipe::write(uint32_t v)
{
	if (fd_write < 0) return -1;
	return ::write(fd_write, &v, sizeof(v));
}

int Pipe::read(uint32_t & v)
{
	if (fd < 0) return -1;
	return ::read(fd, &v, sizeof(v));
}

