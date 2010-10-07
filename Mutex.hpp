#ifndef __MUTEX__HPP__
#define __MUTEX__HPP__

#include <pthread.h>

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

#endif
