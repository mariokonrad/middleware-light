#include <mwl/Thread.hpp>
#include <mwl/Runnable.hpp>
#include <stddef.h>

namespace mwl {

void * Thread::execute_thread(void * ptr)
{
	if (ptr) {
		Thread * t = reinterpret_cast<Thread *>(ptr);
		t->runnable->run();
	}
	::pthread_exit(NULL);
}

Thread::Thread(Runnable * runnable)
	: runnable(runnable)
{}

Thread::~Thread()
{}

int Thread::start()
{
	if (!runnable) return -1;
	return ::pthread_create(&tid, NULL, execute_thread, this);
}

}

