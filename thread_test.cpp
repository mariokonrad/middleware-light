#include <iostream>
#include <Executor.hpp>
#include <Runnable.hpp>

class Foo : public Runnable
{
	private:
		std::string name;
	public:
		Foo(const std::string & name)
			: name(name)
		{}

		virtual void run()
		{
			for (int i = 0; i < 10; ++i) {
				std::cout << name << " : " << i << std::endl;
				usleep(10000);
			}
		}
};

int main(int, char **)
{
	Foo foo("Foo"), bar("Bar");

/*
	Thread t0(&foo);
	Thread t1(&bar);
	t0.start();
	t1.start();
	t0.join();
	t1.join();
*/

	Executor ex;
	ex.start();
	ex.execute(&foo);
	ex.execute(&bar);
	ex.execute(NULL);
	ex.join();

	return 0;
}

