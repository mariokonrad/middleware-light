#ifndef __CONDITIONVAR__HPP__
#define __CONDITIONVAR__HPP__

#include <pthread.h>

class Mutex;

class ConditionVar
{
	private:
		pthread_cond_t cond;
	public:
		ConditionVar();
		~ConditionVar();
		void signal();
		void broadcast();
		void wait(Mutex &);
};

#endif
