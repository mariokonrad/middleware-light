#ifndef __MWL__RUNNABLE__HPP__
#define __MWL__RUNNABLE__HPP__

namespace mwl {

class Runnable
{
	public:
		virtual ~Runnable() {}
		virtual void run() = 0;
		virtual bool terminate() const = 0;
};

}

#endif
