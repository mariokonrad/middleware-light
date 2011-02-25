#ifndef __MWL__MODULEBASEINTERFACE__HPP__
#define __MWL__MODULEBASEINTERFACE__HPP__

namespace mwl {

class Channel;

class ModuleBaseInterface
{
	public:
		virtual ~ModuleBaseInterface() {}
		virtual int receive(Channel *) = 0;
};

}

#endif
