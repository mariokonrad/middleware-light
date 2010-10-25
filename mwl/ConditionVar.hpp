#ifndef __MWL__CONDITIONVAR__HPP__
#define __MWL__CONDITIONVAR__HPP__

#include <pthread.h>

namespace mwl {

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

}

#endif
