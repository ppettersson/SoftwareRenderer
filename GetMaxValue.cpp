#include "SoftwareRenderer.h"

byte GetMaxByteValue_C(byte *data, dword num)
{
	byte value = data[0];
	for (dword i = 1; i < num; ++i)
		if (data[i] > value)
			value = data[i];

	return value;
}

word GetMaxWordValue_C(word *data, dword num)
{
	word value = data[0];
	for (dword i = 1; i < num; ++i)
		if (data[i] > value)
			value = data[i];

	return value;
}
