#include "SoftwareRenderer.h"

void DrawVerticalLine(dword x, dword y0, dword y1, dword color)
{
  if (y0 > y1)
    Swap(y0, y1);

  dword *buffer = frameBuffer + (x + y0 * screenWidth);

  // Note that unrolling this won't help as we're completely bottlenecked by memory accesses.
  dword dy = y1 - y0;
  while (dy--)
  {
    *buffer = color;
    buffer += screenWidth;
  }
}

void DrawHorizontalLine(dword y, dword x0, dword x1, dword color)
{
  if (x0 > x1)
    Swap(x0, x1);

  dword *buffer = frameBuffer + (x0 + y * screenWidth);

  // Note that the complier handles this well and converts it directly to an optimized memory copy.
  dword dx = x1 - x0;
  while (dx--)
    *buffer++ = color;
}

void DrawLine_Real(dword x0, dword y0, dword x1, dword y1, dword color)
{
  int dx = x1 - x0,
      dy = y1 - y0;

  real x = (real)x0,
       y = (real)y0,
       xInc,
       yInc;
  int  steps;

  if (Abs(dx) > Abs(dy))
  {
    if (dx > 0)
    {
      xInc  = 1.0f;
      yInc  = dy / (real)dx;
    }
    else
    {
      xInc  = -1.0f;
      yInc  = dy / (real)-dx;
    }

    steps = Abs(dx);
  }
  else
  {
    if (dy > 0)
    {
      xInc  = dx / (real)dy;
      yInc  = 1.0f;
    }
    else
    {
      xInc  = dx / (real)-dy;
      yInc  = -1.0f;
    }

    steps = Abs(dy);
  }

  for (int i = 0; i < steps; ++i)
  {
    // Letting the compiler handle the real to int cast was faster than special asm tricks.
    int ix = (int)x,
        iy = (int)y;

    frameBuffer[ix + iy * screenWidth] = color;

    x += xInc;
    y += yInc;
  }
}

void DrawLine_Bresenham(dword x0, dword y0, dword x1, dword y1, dword color)
{
  // Save half the line drawing cases by swapping the start and end points if y0 is greater than y1.
  // As a result, dy is always >= 0, and only the octants 0 to 3 cases need to be handled.
  if (y0 > y1)
  {
    Swap(x0, x1);
    Swap(y0, y1);
  }

  // Calculate the deltas.
  int dx = x1 - x0,
      dy = y1 - y0;

  // See if we should step forward/right or backward/left.
  int xInc;
  if (dx > 0)
  {
    xInc  = 1;
  }
  else
  {
    // In this case we also negate the delta.
    dx    = -dx;
    xInc  = -1;
  }

  // Setup initial error term and values used inside drawing loop.
  int dx2             = dx * 2,
      dy2             = dy * 2,
      majorSteps,                           // How many pixels to draw in the major axis.
      errorTerm,                            // The running check for when it's time to step the minor axis.
      errorIncMinor,                        // The increment to the error term when stepping along the minor axis.
      errorIncMajor,                        // The increment to the error term when stepping along the major axis.
      bufferIncMinor  = screenWidth + xInc, // The increment to the draw buffer pointer when stepping along the minor axis.
      bufferIncMajor;                       // The increment to the draw buffer pointer when stepping along the major axis.
  if (dx > dy)
  {
    // Draws a line in octant 0 or 3 (Abs(dx) >= dy).
    majorSteps      = dx;
    errorTerm       = dy2 - dx;
    errorIncMinor   = dy2 - dx2;
    errorIncMajor   = dy2;
    bufferIncMajor  = xInc;
  }
  else
  {
    // Draws a line in octant 1 or 2 (Abs(dx) < dy).
    majorSteps      = dy;
    errorTerm       = dx2 - dy;
    errorIncMinor   = dx2 - dy2;
    errorIncMajor   = dx2;
    bufferIncMajor  = screenWidth;
  }

  // Setup the buffer pointer and draw the first pixel.
  dword *buffer = frameBuffer + x0 + y0 * screenWidth;
  *buffer = color;

  // Draw the line.
  while (majorSteps--)
  {
    // Increment both the error term and the buffer pointer.
    if (errorTerm >= 0)
    {
      buffer    += bufferIncMinor;
      errorTerm += errorIncMinor;
    }
    else
    {
      buffer    += bufferIncMajor;
      errorTerm += errorIncMajor;
    }

    // Draw the pixel.
    *buffer = color;
  }
}

void DrawLine(dword x0, dword y0, dword x1, dword y1, dword color)
{
  // First check for special cases.
  int dx = x1 - x0,
      dy = y1 - y0;

  if (dx == 0)
  {
    if (dy == 0)
    {
      DrawPixel(x0, y0, color);
      return;
    }

    DrawVerticalLine(x0, y0, y1, color);
    return;
  }
  
  if (dy == 0)
  {
    DrawHorizontalLine(y0, x0, x1, color);
    return;
  }

  // ToDo: Also check for diagonal cases here.

  // The basic real version perfoms better, probably due to no branching inside the loop.
  DrawLine_Real(x0, y0, x1, y1, color);
  //DrawLine_Bresenham(x0, y0, x1, y1, color);
}

