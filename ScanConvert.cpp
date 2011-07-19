#include "ScanConvert.h"
#include <stdlib.h>

Span *spans   = NULL;
int   spanY0  = 0,
      spanY1  = 0;

void QuitScanConvert()
{
  if (spans)
  {
    delete [] spans;
    spans = NULL;
  }
}

void InitScanConvert()
{
  QuitScanConvert();

  spans = new Span [screenHeight];
}
