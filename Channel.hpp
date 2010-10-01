#ifndef __CHANNEL__HPP__
#define __CHANNEL__HPP__

class Channel
{
	public:
		virtual ~Channel() {}
		virtual int open() = 0;
		virtual int close() = 0;
		virtual int send(const void *, unsigned int) = 0;
		virtual int recv(void *, unsigned int) = 0;
};

#endif
