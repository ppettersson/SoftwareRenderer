#include "SoftwareRenderer.h"
#include "ScanConvert.h"
#include "CPU.h"
#include "Timer.h"
#include "Platform.h"
#include "ThreadPoolWin32.h"
#include <stdio.h>

bool InitFrameBuffer(dword width, dword height);
void QuitFrameBuffer();

bool Init(dword width, dword height)
{
	if (width <= 0 || height <= 0)
		return false;

	SetupDispatchTable();

	InitFrameBuffer(width, height);
	InitScanConvert();

	if (PlatformOpen(width, height))
	{
		InitTimer();
		ThreadPool::GetInstance().Init();	// It's ok if this fails, everything will just run single-threaded instead.
		return true;
	}

	return false;
}

bool PresentFrame()
{
	if (frameBuffer && PlatformUpdate(frameBuffer))
	{
		UpdateTimer();

		char msg[256];
		sprintf_s(msg, 256, "SoftwareRenderer - [FPS = %d | %.1f]", GetFpsAverage(), GetFps());
		PlatformSetWindowCaption(msg);

		return true;
	}

	return false;
}

void Quit()
{
	ThreadPool::GetInstance().Destroy();
	QuitFrameBuffer();
	QuitScanConvert();
	PlatformClose();
}
