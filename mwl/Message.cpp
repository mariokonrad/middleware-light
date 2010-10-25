#include <mwl/Message.hpp>
#include <mwl/endian.hpp>
#include <cstring>

namespace mwl {

void hton(struct Head & m)
{
	::endian::hton(m.src);
	::endian::hton(m.dst);
	::endian::hton(m.type);
	::endian::hton(m.size);
}

void ntoh(struct Head & m)
{
	::endian::ntoh(m.src);
	::endian::ntoh(m.dst);
	::endian::ntoh(m.type);
	::endian::ntoh(m.size);
}

int serialize(uint8_t * buf, const struct Head & m)
{
	if (!buf) return -1;
	memcpy(buf, &m, sizeof(m));
	return 0;
}

int deserialize(struct Head & m, const uint8_t * buf)
{
	if (!buf) return -1;
	memcpy(&m, buf, sizeof(m));
	return 0;
}

}

