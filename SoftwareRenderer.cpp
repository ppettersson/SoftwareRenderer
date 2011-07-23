#include "SoftwareRenderer.h"
#include "ScanConvert.h"
#include "CPU.h"
#include "Timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

extern "C"
{
#include "../TinyPTC/tinyptc.h"
extern HWND wnd;
}

bool InitFrameBuffer(dword width, dword height);
void QuitFrameBuffer();

bool Init(dword width, dword height)
{
  if (width <= 0 || height <= 0)
    return false;

  SetupDispatchTable();

  InitFrameBuffer(width, height);
  InitScanConvert();

  if (ptc_open("SoftwareRenderer", width, height))
  {
    InitTimer();
    return true;
  }

  return false;
}

bool PresentFrame()
{
  if (frameBuffer && ptc_update(frameBuffer))
  {
    UpdateTimer();

    char msg[256];
    sprintf_s(msg, 256, "SoftwareRenderer - [FPS = %d | %.1f]", GetFpsAverage(), GetFps());
    SetWindowText(wnd, msg);

    return true;
  }

  return false;
}

void Quit()
{
  QuitFrameBuffer();
  QuitScanConvert();

  ptc_close();
}
