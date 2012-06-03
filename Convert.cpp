#include "SoftwareRenderer.h"

void ConvertRGBAtoBGRA_C(dword *dst, dword *src, dword num)
{
	byte *d = (byte *)dst,
		*s = (byte *)src;

	while (num--)
	{
		d[0] = s[2];
		d[1] = s[1];
		d[2] = s[0];
		d[3] = s[3];

		d += 4;
		s += 4;
	}
}
