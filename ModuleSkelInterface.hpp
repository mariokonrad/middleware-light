#ifndef __MODULESKELINTERFACE__HPP__
#define __MODULESKELINTERFACE__HPP__

#include <stdint.h>
#include <Message.hpp>
#include <Channel.hpp>

class ModuleSkelInterface
{
	public:
		virtual ~ModuleSkelInterface() {}
		virtual int receive(Channel *) = 0;
};

#endif
