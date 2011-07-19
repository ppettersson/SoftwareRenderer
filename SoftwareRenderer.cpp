#include "SoftwareRenderer.h"
#include "ScanConvert.h"
#include "CPU.h"
#include <stdlib.h>

extern "C"
{
#include "../TinyPTC/tinyptc.h"
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

  return ptc_open("SoftwareRenderer", width, height) ? true : false;
}

bool PresentFrame()
{
  if (frameBuffer)
    return ptc_update(frameBuffer) ? true : false;

  return false;
}

void Quit()
{
  QuitFrameBuffer();
  QuitScanConvert();

  ptc_close();
}
