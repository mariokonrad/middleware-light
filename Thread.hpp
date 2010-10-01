#ifndef __THREAD__HPP__
#define __THREAD__HPP__

#include <ThreadBase.hpp>

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

#endif
