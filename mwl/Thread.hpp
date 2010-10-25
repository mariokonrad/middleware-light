#ifndef __MWL__THREAD__HPP__
#define __MWL__THREAD__HPP__

#include <mwl/ThreadBase.hpp>

namespace mwl {

class Runnable;

class Thread : public ThreadBase
{
	protected:
		Runnable * runnable;
	private:
		static void * execute_thread(void *);
	public:
		Thread(Runnable * = NULL);
		virtual ~Thread();
		virtual int start();
};

}

#endif
