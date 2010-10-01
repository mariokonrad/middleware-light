#ifndef __MESSAGE__HPP__
#define __MESSAGE__HPP__

#include <stdint.h>

struct Head {
	uint32_t src;
	uint32_t dst;
	uint32_t type;
	uint32_t size;
} __attribute__((packed));

void hton(struct Head &);
void ntoh(struct Head &);
int serialize(uint8_t *, const struct Head &);
int deserialize(struct Head &, const uint8_t *);

#endif
