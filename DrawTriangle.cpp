#include "SoftwareRenderer.h"

// Half space method of rendering a triangle.
// This is beneficial when you want to use parallelism (e.g. SIMD).

#if 0

int LineEquation(const Point2 &p0, const Point2 &p1, const Point2 &p)
{
  return (p1.x - p0.x) * (p.y - p0.y) - (p1.y - p0.y) * (p.x - p0.x);
}

void DrawTriangle_Flat(Point2 *points, dword color)
{
  // Find the minimal bounding volume.
  dword x0 = Min(points[0].x, Min(points[1].x, points[2].x));
  dword x1 = Max(points[0].x, Max(points[1].x, points[2].x));
  dword y0 = Min(points[0].y, Min(points[1].y, points[2].y));
  dword y1 = Max(points[0].y, Max(points[1].y, points[2].y));

  // Scan through the bounding box and do the half space test.
  for (dword y = y0; y < y1; ++y)
  {
    for (dword x = x0; x < x1; ++x)
    {
      // If the pixel is inside each of the three lines half space then the 
      // pixel is inside the triangle.
      Point2 p(x, y);
      if ((LineEquation(points[0], points[1], p) > 0) &&
          (LineEquation(points[1], points[2], p) > 0) &&
          (LineEquation(points[2], points[0], p) > 0))
      {
        //DrawPixel(x, y, color);
        DrawPixel(x, y, Color(x & 0xff, y & 0xff, 0, 0xff));
      }
    }
  }
}

#else

// ToDo: fill convention - topleft

void DrawTriangle_Flat(Point2 *points, dword color)
{
  // Find the minimal bounding volume.
  const dword x0 = Min(points[0].x, Min(points[1].x, points[2].x));
  const dword x1 = Max(points[0].x, Max(points[1].x, points[2].x));
  const dword y0 = Min(points[0].y, Min(points[1].y, points[2].y));
  const dword y1 = Max(points[0].y, Max(points[1].y, points[2].y));

  // Pre-calculate the half space equation for the 3 edges.
  const dword dx0 = points[1].x - points[0].x;
  const dword dx1 = points[2].x - points[1].x;
  const dword dx2 = points[0].x - points[2].x;

  const dword dy0 = points[1].y - points[0].y;
  const dword dy1 = points[2].y - points[1].y;
  const dword dy2 = points[0].y - points[2].y;

  const dword cy0 = dx0 * (y0 - points[0].y);
  const dword cy1 = dx1 * (y0 - points[1].y);
  const dword cy2 = dx2 * (y0 - points[2].y);

  const dword cx0 = dy0 * (x0 - points[0].x);
  const dword cx1 = dy1 * (x0 - points[1].x);
  const dword cx2 = dy2 * (x0 - points[2].x);

  // (point1.x - point0.x) * (y0 - point0.y) - (point1.y - point0.y) * (x0 - point0.x)
  dword ce0 = cy0 - cx0;
  dword ce1 = cy1 - cx1;
  dword ce2 = cy2 - cx2;

  // Scan through the bounding box and do the half space test.
  for (dword y = y0; y < y1; ++y)
  {
    // The start value for each of the three edges when scanning horizontally.
    int e0 = ce0,
        e1 = ce1,
        e2 = ce2;

    for (dword x = x0; x < x1; ++x)
    {
      // If the pixel is inside each of the three edges half space then the 
      // pixel is inside the triangle.
      if ((e0 > 0) && (e1 > 0) && (e2 > 0))
      {
        DrawPixel(x, y, color);
      }

      // Step the three edges one horizontal increment.
      e0 -= dy0;
      e1 -= dy1;
      e2 -= dy2;
    }

    // Increment the three edges when moving to the next scan line.
    ce0 += dx0;
    ce1 += dx1;
    ce2 += dx2;
  }
}

#endif
