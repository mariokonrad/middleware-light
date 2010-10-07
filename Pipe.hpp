#ifndef __PIPE__HPP__
#define __PIPE__HPP__

#include <Device.hpp>
#include <stdint.h>

class Pipe : public Device
{
	private:
		int fd_write;
	public:
		Pipe();
		virtual ~Pipe();
		virtual int open();
		virtual int close();
		virtual int write(uint32_t);
		virtual int read(uint32_t &);
};

#endif
