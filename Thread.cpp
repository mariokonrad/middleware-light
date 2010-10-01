#include <Thread.hpp>
#include <Runnable.hpp>
#include <stddef.h>

void * Thread::execute_thread(void * ptr)
{
	if (ptr) {
		Thread * t = reinterpret_cast<Thread *>(ptr);
		t->runnable->run();
	}
	pthread_exit(NULL);
}

Thread::Thread(Runnable * runnable)
	: runnable(runnable)
{}

Thread::~Thread()
{}

int Thread::start()
{
	if (!runnable) return -1;
	return pthread_create(&tid, NULL, execute_thread, this);
}

