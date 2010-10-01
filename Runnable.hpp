#ifndef __RUNNABLE__HPP__
#define __RUNNABLE__HPP__

class Runnable
{
	public:
		virtual ~Runnable() {}
		virtual void run() = 0;
};

#endif
