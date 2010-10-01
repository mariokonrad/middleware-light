#include <Executor.hpp>
#include <Runnable.hpp>

void * Executor::execute_executor(void * ptr)
{
	if (ptr) {
		Executor * executor = reinterpret_cast<Executor *>(ptr);
		executor->run();
	}
	pthread_exit(NULL);
}

void Executor::run()
{
	for (;;) {
		pthread_mutex_lock(&mtx);
		while (queue.empty()) pthread_cond_wait(&non_empty, &mtx);
		Entry entry = queue.front();
		queue.pop_front();
		pthread_mutex_unlock(&mtx);
		if (entry.runnable == NULL) break;
		entry.runnable->run();
		if (entry.auto_destroy) delete entry.runnable;
	}
}

Executor::Executor()
{
	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&non_empty, NULL);
}

Executor::~Executor()
{
	pthread_cond_destroy(&non_empty);
	pthread_mutex_destroy(&mtx);
	queue.clear();
}

int Executor::start()
{
	int rc = pthread_create(&tid, NULL, execute_executor, this);
	yield();
	return rc;
}

int Executor::execute(Runnable * runnable, bool auto_destroy)
{
	pthread_mutex_lock(&mtx);
	queue.push_back(Entry(runnable, auto_destroy));
	pthread_cond_signal(&non_empty);
	pthread_mutex_unlock(&mtx);
	return 0;
}

Executor::size_type Executor::size()
{
	size_type rc = 0;
	pthread_mutex_lock(&mtx);
	rc = queue.size();
	pthread_mutex_unlock(&mtx);
	return rc;
}

