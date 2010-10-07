#include <Mutex.hpp>

Mutex::Mutex()
{
	pthread_mutex_init(&mtx, NULL);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&mtx);
}

void Mutex::lock()
{
	pthread_mutex_lock(&mtx);
}

void Mutex::unlock()
{
	pthread_mutex_unlock(&mtx);
}

