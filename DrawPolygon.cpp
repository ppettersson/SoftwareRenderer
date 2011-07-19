#include "SoftwareRenderer.h"
#include "ScanConvert.h"

void DrawPolygon_Flat(dword numPoints, Point2 *points, dword color)
{
  if (numPoints < 3)
    return;

  VertexType_P vertices;
  vertices.points = points;

  ScanConvert(numPoints, vertices);

  for (int y = spanY0; y < spanY1; ++y)
  {
    Span &s = spans[y];
    for (int x = s.span[0].x; x < s.span[1].x; ++x)
      DrawPixel(x, y, color); // ToDo: change this to MemSet.
  }
}

void DrawPolygon_Shaded(dword numPoints, Point2 *points, dword *colors)
{
  if (numPoints < 3)
    return;

  VertexType_PC vertices;
  vertices.points = points;
  vertices.colors = colors;

  ScanConvert(numPoints, vertices);

  for (int y = spanY0; y < spanY1; ++y)
  {
    Span &s = spans[y];

    int x0 = s.span[0].x,
        x1 = s.span[1].x;

    ColorInterpolator color(s.span[0].color, s.span[1].color, 1.0f / (x1 - x0));

    for (int x = x0; x < x1; ++x)
      DrawPixel(x, y, color.Step());
  }
}

void DrawPolygon_Textured(dword numPoints, Point2 *points, Point2 *texCoords, Image &texture)
{
  if (numPoints < 3)
    return;

  VertexType_PT vertices;
  vertices.points     = points;
  vertices.texCoords  = texCoords;

  ScanConvert(numPoints, vertices);

  for (int y = spanY0; y < spanY1; ++y)
  {
    Span &s = spans[y];

    int x0 = s.span[0].x,
        x1 = s.span[1].x;

    real invDelta = 1.0f / (x1 - x0);

    Point2Interpolator texCoordInterpolator(s.span[0].texCoord, s.span[1].texCoord, invDelta);

    for (int x = x0; x < x1; ++x)
    {
      Point2 texCoord = texCoordInterpolator.Step();

      dword color = texture.data[texCoord.x + texCoord.y * texture.width];
      DrawPixel(x, y, color);
    }
  }
}

void DrawPolygon_ShadedTextured(dword numPoints, Point2 *points, Point2 *texCoords, dword *colors, Image &texture)
{
  if (numPoints < 3)
    return;

  VertexType_PCT vertices;
  vertices.points     = points;
  vertices.texCoords  = texCoords;
  vertices.colors     = colors;

  ScanConvert(numPoints, vertices);

  for (int y = spanY0; y < spanY1; ++y)
  {
    Span &s = spans[y];

    int x0 = s.span[0].x,
        x1 = s.span[1].x;

    real invDelta = 1.0f / (x1 - x0);

    ColorInterpolator   color(s.span[0].color, s.span[1].color, 1.0f / (x1 - x0));
    Point2Interpolator  texCoordInterpolator(s.span[0].texCoord, s.span[1].texCoord, invDelta);

    for (int x = x0; x < x1; ++x)
    {
      Point2 texCoord = texCoordInterpolator.Step();

      dword texColor   = texture.data[texCoord.x + texCoord.y * texture.width];
      dword shadeColor = color.Step();

      real tr, tg, tb, 
           sr, sg, sb;
      Color(texColor, tr, tg, tb);
      Color(shadeColor, sr, sg, sb);

      DrawPixel(x, y, Color(tr * sr, tg * sg, tb * sb));
    }
  }
}

void DrawPolygon_Textured_Blend(dword numPoints, Point2 *points, Point2 *texCoords, Image &texture, dword (BlendFunc)(dword src, dword dst))
{
  if (numPoints < 3)
    return;

  VertexType_PT vertices;
  vertices.points     = points;
  vertices.texCoords  = texCoords;

  ScanConvert(numPoints, vertices);

  for (int y = spanY0; y < spanY1; ++y)
  {
    Span &s = spans[y];

    int x0 = s.span[0].x,
        x1 = s.span[1].x;

    real invDelta = 1.0f / (x1 - x0);

    Point2Interpolator texCoordInterpolator(s.span[0].texCoord, s.span[1].texCoord, invDelta);

    for (int x = x0; x < x1; ++x)
    {
      Point2 texCoord = texCoordInterpolator.Step();

      dword color = texture.data[texCoord.x + texCoord.y * texture.width];

      DrawPixelBlend(x, y, color, BlendFunc);
    }
  }
}

void DrawPolygon_ShadedTextured_Blend(dword numPoints, Point2 *points, Point2 *texCoords, dword *colors, Image &texture, dword (BlendFunc)(dword src, dword dst))
{
  if (numPoints < 3)
    return;

  VertexType_PCT vertices;
  vertices.points     = points;
  vertices.texCoords  = texCoords;
  vertices.colors     = colors;

  ScanConvert(numPoints, vertices);

  for (int y = spanY0; y < spanY1; ++y)
  {
    Span &s = spans[y];

    int x0 = s.span[0].x,
        x1 = s.span[1].x;

    real invDelta = 1.0f / (x1 - x0);

    ColorInterpolator   color(s.span[0].color, s.span[1].color, 1.0f / (x1 - x0));
    Point2Interpolator  texCoordInterpolator(s.span[0].texCoord, s.span[1].texCoord, invDelta);

    for (int x = x0; x < x1; ++x)
    {
      Point2 texCoord = texCoordInterpolator.Step();

      dword texColor   = texture.data[texCoord.x + texCoord.y * texture.width];
      dword shadeColor = color.Step();

      real tr, tg, tb, 
           sr, sg, sb;
      Color(texColor, tr, tg, tb);
      Color(shadeColor, sr, sg, sb);

      DrawPixelBlend(x, y, Color(tr * sr, tg * sg, tb * sb), BlendFunc);
    }
  }
}

