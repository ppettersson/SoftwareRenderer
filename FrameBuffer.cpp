#include "SoftwareRenderer.h"
#include <stdlib.h>

dword  screenWidth   = 0,
	screenHeight  = 0;

dword *frameBuffer   = NULL;
dword *clearLine     = NULL;

void QuitFrameBuffer();


bool InitFrameBuffer(dword width, dword height)
{
	QuitFrameBuffer();

	screenWidth   = width;
	screenHeight  = height;

	frameBuffer = (dword *)AllocAlign(sizeof(dword) * screenWidth * screenHeight);
	clearLine   = (dword *)AllocAlign(sizeof(dword) * screenWidth);

	return true;
}

void QuitFrameBuffer()
{
	if (frameBuffer)
	{
		FreeAlign(frameBuffer);
		frameBuffer = NULL;
	}
	if (clearLine)
	{
		FreeAlign(clearLine);
		clearLine = NULL;
	}
}

void Clear(dword color)
{
#ifdef USE_MMX_ASM
	MemSet32(frameBuffer, color, screenWidth * screenHeight);
#else
	//for (dword i = 0; i < screenWidth * screenHeight; ++i)
	//  frameBuffer[i] = color;

	// Unfortunately memcpy/memset only handles 8 bit data, so we copy one line's worth of 32 bit colors into an array...
	for (dword i = 0; i < screenWidth; ++i)
		clearLine[i] = color;

	// ...and then we use memcpy for all the lines.
	for (dword i = 0; i < screenHeight; ++i)
		MemCpy(frameBuffer + i * screenWidth, clearLine, screenWidth * sizeof(dword));
#endif
}

void DrawPixel(dword x, dword y, dword color)
{
	frameBuffer[x + y * screenWidth] = color;
}

void DrawPixelBlend(dword x, dword y, dword color, dword (BlendFunc)(dword src, dword dst))
{
	DrawPixel(x, y, BlendFunc(color, frameBuffer[x + y * screenWidth]));
}

