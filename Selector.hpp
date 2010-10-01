#ifndef __SELECTOR__HPP__
#define __SELECTOR__HPP__

#include <vector>
#include <sys/select.h>

class Device;

class Selector
{
	private:
		typedef std::vector<Device *> Devices;
	private:
		Devices devices;
		int fd_max;
		fd_set rfds_cache;
	private:
		void set_fd_max();
	public:
		Selector();
		~Selector();
		void add(Device *);
		void remove(Device *);
		int select(Device **);
		static int select(Device &);
};

#endif
