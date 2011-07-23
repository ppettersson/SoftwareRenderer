#include "Input.h"
#include <windows.h>

static int conversionTable[kKey_Last] =
{
  VK_LEFT,
  VK_RIGHT
};

bool KeyPressed(int code)
{
  return !! (GetAsyncKeyState(conversionTable[code]) & 0x8000);
}

bool KeyDown(int code)
{
  if (KeyPressed(code))
  {
    while (KeyPressed(code))
      ;

    return true;
  }

  return false;
}
