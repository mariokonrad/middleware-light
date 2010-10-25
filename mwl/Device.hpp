#ifndef __MWL__DEVICE__HPP__
#define __MWL__DEVICE__HPP__

namespace mwl {

class Device
{
		friend class Selector;
	protected:
		int fd;
	public:
		Device() : fd(-1) {}
		virtual ~Device() {}
		virtual int open() = 0;
		virtual int close() = 0;
};

}

#endif
