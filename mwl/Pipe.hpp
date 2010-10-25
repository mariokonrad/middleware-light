#ifndef __MWL__PIPE__HPP__
#define __MWL__PIPE__HPP__

#include <mwl/Device.hpp>
#include <stdint.h>

namespace mwl {

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

}

#endif
