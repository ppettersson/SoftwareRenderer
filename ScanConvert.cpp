#include "ScanConvert.h"
#include <stdlib.h>

Span *spans   = NULL;
int   spanY0  = 0,
	spanY1  = 0;

void QuitScanConvert()
{
	if (spans)
	{
		FreeAlign(spans);
		spans = NULL;
	}
}

void InitScanConvert()
{
	QuitScanConvert();

	spans = (Span *)AllocAlign(sizeof(Span) * screenHeight);
}
