#ifndef __MWL__MUTEX__HPP__
#define __MWL__MUTEX__HPP__

#include <pthread.h>

namespace mwl {

class Mutex
{
		friend class ConditionVar;
	private:
		pthread_mutex_t mtx;
	public:
		Mutex();
		~Mutex();
		void lock();
		void unlock();
};

}

#endif
