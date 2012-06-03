#include "SoftwareRenderer.h"
#include "Timer.h"

#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

static DWORD startTime = 0;
static DWORD lastTime  = 0;

static DWORD fpsStartTime = 0;
static DWORD fpsNumFrames = 0;

static int fpsAverage = 0;

static real fps = 0;
static real timePassed;

void InitTimer()
{
	timeBeginPeriod(1);
	startTime = timeGetTime();
	lastTime  = startTime;

	fpsStartTime = startTime;
	fpsNumFrames = 0;

	fpsAverage = 0;
}

void UpdateTimer()
{
	// Basic frame rate counter.
	DWORD currentTime = timeGetTime();

	fps = 1000.0f / (currentTime - lastTime);
	lastTime = currentTime;

	timePassed = (currentTime - startTime) / 1000.0f;

	fpsNumFrames++;
	if ((currentTime - fpsStartTime) > 1000)
	{
		fpsAverage = (int)fpsNumFrames;

		fpsStartTime = currentTime;
		fpsNumFrames = 0;
	}
}

real GetTimePassed()
{
	return timePassed;
}

real GetFps()
{
	return fps;
}

int GetFpsAverage()
{
	return fpsAverage;
}

dword GetTimeStamp()
{
	return timeGetTime();
}