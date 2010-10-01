#include "endian.hpp"

int main(int, char **)
{
	bool little = endian::is_little();
	return 0;
}

