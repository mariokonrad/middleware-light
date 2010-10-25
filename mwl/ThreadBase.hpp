#ifndef __MWL__THREADBASE__HPP__
#define __MWL__THREADBASE__HPP__

#include <pthread.h>

namespace mwl {

class ThreadBase
{
	protected:
		pthread_t tid;
	public:
		virtual ~ThreadBase();
		virtual int start() = 0;
		virtual int join();
		virtual void yield();
};

}

#endif
