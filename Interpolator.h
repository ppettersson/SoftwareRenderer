#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

class IntInterpolator
{
public:
  IntInterpolator(int start, int end, real invStep)
  {
    x   = (real)start;
    inc = (end - start) * invStep;
  }

  int Step()
  {
    int result = (int)x;
    x += inc;

    return result;
  }

private:
  real x;
  real inc;
};

class ColorInterpolator
{
public:
  ColorInterpolator(dword start, dword end, real invStep)
  {
    Color(start, r, g, b);

    real r1, b1, g1;
    Color(end, r1, g1, b1);

    rInc = (r1 - r) * invStep;
    gInc = (g1 - g) * invStep;
    bInc = (b1 - b) * invStep;
  }

  dword Step()
  {
    dword result = Color(r, g, b);

    r += rInc;
    g += gInc;
    b += bInc;

    return result;
  }

private:
  real r,
       g,
       b,
       rInc,
       gInc,
       bInc;
};

class Point2Interpolator
{
public:
  Point2Interpolator(const Point2 &start, const Point2 &end, real invStep)
  {
    x = (real)start.x;
    y = (real)start.y;

    xInc = ((real)end.x - (real)start.x) * invStep;
    yInc = ((real)end.y - (real)start.y) * invStep;
  }

  Point2 Step()
  {
    Point2 result((dword)x, (dword)y);

    x += xInc;
    y += yInc;

    return result;
  }

private:
  real x,
       y,
       xInc,
       yInc;
};

#endif // INTERPOLATOR_H
