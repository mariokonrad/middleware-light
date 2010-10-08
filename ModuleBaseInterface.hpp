#ifndef __MODULEBASEINTERFACE__HPP__
#define __MODULEBASEINTERFACE__HPP__

#include <stdint.h>
#include <Message.hpp>
#include <Channel.hpp>

class ModuleBaseInterface
{
	public:
		virtual ~ModuleBaseInterface() {}
		virtual int receive(Channel *) = 0;
};

#endif
