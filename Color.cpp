#include "SoftwareRenderer.h"

void ColorScale(dword &color, real scale)
{
	if (scale == 0)
	{
		color = 0;
		return;
	}
	if (scale == 1.0f)
		return;

	dword s = (dword)(scale * 256);

	dword r = (color & kColorRed)   >> 16,
		g = (color & kColorGreen) >> 8,
		b = (color & kColorBlue)  >> 0;

	r *= s;
	g *= s;
	b *= s;

	r >>= 8;
	g >>= 8;
	b >>= 8;

	color = (b << 0) | (g << 8) | (r << 16);
}

void ColorScale(dword &color, dword s)
{
	if (s == 0)
	{
		color = 0;
		return;
	}
	if (s == 255)
		return;

	dword r = (color & kColorRed)   >> 16,
		g = (color & kColorGreen) >> 8,
		b = (color & kColorBlue)  >> 0;

	r *= s;
	g *= s;
	b *= s;

	r >>= 8;
	g >>= 8;
	b >>= 8;

	color = (b << 0) | (g << 8) | (r << 16);
}
