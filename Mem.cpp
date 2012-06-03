#include "SoftwareRenderer.h"
#include <memory.h>

// Can't beat the performance of msvcrt memcpy, it's using sse2, adjusting for alignments etc...
//void MemCpy_C(void *dst, const void *src, size_t count)
//{
//  memcpy(dst, src, count);
//}

void MemSet32_C(void *dst, dword src, size_t count)
{
	dword *d = (dword *)dst;
	for (size_t i = 0; i < count; ++i)
		d[i] = src;
}
