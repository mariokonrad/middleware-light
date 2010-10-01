#include <Message.hpp>
#include <endian.hpp>
#include <cstring>

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
	memcpy(buf, &m.src, sizeof(m.src));
	buf += sizeof(m.src);
	memcpy(buf, &m.dst, sizeof(m.dst));
	buf += sizeof(m.dst);
	memcpy(buf, &m.type, sizeof(m.type));
	buf += sizeof(m.type);
	memcpy(buf, &m.size, sizeof(m.size));
	buf += sizeof(m.size);
	return 0;
}

int deserialize(struct Head & m, const uint8_t * buf)
{
	if (!buf) return -1;
	memcpy(&m.src, buf, sizeof(m.src));
	buf += sizeof(m.src);
	memcpy(&m.dst, buf, sizeof(m.dst));
	buf += sizeof(m.dst);
	memcpy(&m.type, buf, sizeof(m.type));
	buf += sizeof(m.type);
	memcpy(&m.size, buf, sizeof(m.size));
	buf += sizeof(m.size);
	return 0;
}

