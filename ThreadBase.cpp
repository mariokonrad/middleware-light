#include <ThreadBase.hpp>
#include <unistd.h>

ThreadBase::~ThreadBase()
{}

int ThreadBase::join()
{
	return pthread_join(tid, NULL);
}

void ThreadBase::yield()
{
	usleep(0);
}

