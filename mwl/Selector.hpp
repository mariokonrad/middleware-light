#ifndef __MWL__SELECTOR__HPP__
#define __MWL__SELECTOR__HPP__

#include <vector>
#include <sys/select.h>

namespace mwl {

class Device;

class Selector
{
	public:
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
		int select(Devices &);
		static int select(Device &);
};

}

#endif
