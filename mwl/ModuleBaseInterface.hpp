#ifndef __MWL__MODULEBASEINTERFACE__HPP__
#define __MWL__MODULEBASEINTERFACE__HPP__

#include <stdint.h>
#include <mwl/Message.hpp>
#include <mwl/Channel.hpp>

namespace mwl {

class ModuleBaseInterface
{
	public:
		virtual ~ModuleBaseInterface() {}
		virtual int receive(Channel *) = 0;
};

}

#endif
