#ifndef __RUNNABLE__HPP__
#define __RUNNABLE__HPP__

class Runnable
{
	public:
		virtual ~Runnable() {}
		virtual void run() = 0;
		virtual bool terminate() const = 0;
};

#endif
