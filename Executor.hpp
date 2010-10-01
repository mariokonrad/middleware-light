#ifndef __EXECUTOR__HPP__
#define __EXECUTOR__HPP__

#include <ThreadBase.hpp>
#include <list>

class Runnable;

class Executor : public ThreadBase
{
	private:
		struct Entry {
			Entry(Runnable * runnable, bool auto_destroy = false)
				: runnable(runnable)
				, auto_destroy(auto_destroy)
			{}

			Runnable * runnable;
			bool auto_destroy;
		};
		typedef std::list<Entry> Queue;
	public:
		typedef Queue::size_type size_type;
	private:
		Queue queue;
		pthread_mutex_t mtx;
		pthread_cond_t non_empty;
	private:
		static void * execute_executor(void *);
	private:
		void run();
	public:
		Executor();
		virtual ~Executor();
		virtual int start();
		virtual int execute(Runnable *, bool = false);
		size_type size();
};

#endif
