#ifndef __MWL__MESSAGE__HPP__
#define __MWL__MESSAGE__HPP__

#include <stdint.h>

namespace mwl {

struct Head
{
	Head()
		: src(0)
		, dst(0)
		, type(0)
		, size(0)
	{}

	Head(uint32_t src, uint32_t dst, uint32_t type, uint32_t size)
		: src(src)
		, dst(dst)
		, type(type)
		, size(size)
	{}

	Head(const Head & h)
		: src(h.src)
		, dst(h.dst)
		, type(h.type)
		, size(h.size)
	{}

	uint32_t src;
	uint32_t dst;
	uint32_t type;
	uint32_t size;
} __attribute__((packed));

struct Message
{
	Head head;
	uint32_t size;
	uint8_t * buf;
};

void hton(struct Head &);
void ntoh(struct Head &);
int serialize(uint8_t *, const struct Head &);
int deserialize(struct Head &, const uint8_t *);

}

#endif
