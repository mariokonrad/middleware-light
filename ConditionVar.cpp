#include <ConditionVar.hpp>
#include <Mutex.hpp>

ConditionVar::ConditionVar()
{
	pthread_cond_init(&cond, NULL);
}

ConditionVar::~ConditionVar()
{
	pthread_cond_destroy(&cond);
}

void ConditionVar::signal()
{
	pthread_cond_signal(&cond);
}

void ConditionVar::broadcast()
{
	pthread_cond_broadcast(&cond);
}

void ConditionVar::wait(Mutex & mtx)
{
	pthread_cond_wait(&cond, &mtx.mtx);
}

