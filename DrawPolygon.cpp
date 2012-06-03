#include "SoftwareRenderer.h"
#include "ScanConvert.h"
#include "ThreadPoolWin32.h"

static Image *currentTexture = 0;
static dword currentColor = 0;
static dword (*currentBlendFunc)(dword src, dword dst) = 0;

#if 1 //def USE_WORK_POOL
#	define DISPATCH(func, param)	ThreadPool::GetInstance().Run(func, param)
#	define WAIT()					ThreadPool::GetInstance().Wait()
#else
#	define DISPATCH(func, param)	func(param)
#	define WAIT()
#endif

static void ScanLine_Flat(int y)
{
	Span &s = spans[y];
	for (int x = s.span[0].x; x < s.span[1].x; ++x)
		DrawPixel(x, y, currentColor); // ToDo: change this to MemSet.
}

static void ScanLine_Shaded(int y)
{
	Span &s = spans[y];

	int x0 = s.span[0].x,
		x1 = s.span[1].x;

	ColorInterpolator color(s.span[0].color, s.span[1].color, 1.0f / (x1 - x0));

	for (int x = x0; x < x1; ++x)
		DrawPixel(x, y, color.Step());
}

static void ScanLine_Textured(int y)
{
	Span &s = spans[y];

	int x0 = s.span[0].x,
		x1 = s.span[1].x;

	real invDelta = 1.0f / (x1 - x0);

	Point2Interpolator texCoordInterpolator(s.span[0].texCoord, s.span[1].texCoord, invDelta);

	for (int x = x0; x < x1; ++x)
	{
		Point2 texCoord = texCoordInterpolator.Step();

		dword color = currentTexture->data[texCoord.x + texCoord.y * currentTexture->width];
		DrawPixel(x, y, color);
	}
}

static void ScanLine_ShadedTextured(int y)
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

		dword texColor   = currentTexture->data[texCoord.x + texCoord.y * currentTexture->width];
		dword shadeColor = color.Step();

		real tr, tg, tb, 
			sr, sg, sb;
		Color(texColor, tr, tg, tb);
		Color(shadeColor, sr, sg, sb);

		DrawPixel(x, y, Color(tr * sr, tg * sg, tb * sb));
	}
}

static void ScanLine_Textured_Blend(int y)
{
	Span &s = spans[y];

	int x0 = s.span[0].x,
		x1 = s.span[1].x;

	real invDelta = 1.0f / (x1 - x0);

	Point2Interpolator texCoordInterpolator(s.span[0].texCoord, s.span[1].texCoord, invDelta);

	for (int x = x0; x < x1; ++x)
	{
		Point2 texCoord = texCoordInterpolator.Step();

		dword color = currentTexture->data[texCoord.x + texCoord.y * currentTexture->width];

		DrawPixelBlend(x, y, color, currentBlendFunc);
	}
}

static void ScanLine_ShadedTextured_Blend(int y)
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

		dword texColor   = currentTexture->data[texCoord.x + texCoord.y * currentTexture->width];
		dword shadeColor = color.Step();

		real tr, tg, tb, 
			sr, sg, sb;
		Color(texColor, tr, tg, tb);
		Color(shadeColor, sr, sg, sb);

		DrawPixelBlend(x, y, Color(tr * sr, tg * sg, tb * sb), currentBlendFunc);
	}
}

void DrawPolygon_Flat(dword numPoints, Point2 *points, dword color)
{
	if (numPoints < 3)
		return;

	VertexType_P vertices;
	vertices.points = points;
	ScanConvert(numPoints, vertices);

	currentColor = color;
	for (int y = spanY0; y < spanY1; ++y)
		DISPATCH(ScanLine_Flat, y);
	WAIT();
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
		DISPATCH(ScanLine_Shaded, y);
	WAIT();
}

void DrawPolygon_Textured(dword numPoints, Point2 *points, Point2 *texCoords, Image &texture)
{
	if (numPoints < 3)
		return;

	VertexType_PT vertices;
	vertices.points     = points;
	vertices.texCoords  = texCoords;
	ScanConvert(numPoints, vertices);

	currentTexture = &texture;
	for (int y = spanY0; y < spanY1; ++y)
		DISPATCH(ScanLine_Textured, y);
	WAIT();
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

	currentTexture = &texture;
	for (int y = spanY0; y < spanY1; ++y)
		DISPATCH(ScanLine_ShadedTextured, y);
	WAIT();
}

void DrawPolygon_Textured_Blend(dword numPoints, Point2 *points, Point2 *texCoords, Image &texture, dword (BlendFunc)(dword src, dword dst))
{
	if (numPoints < 3)
		return;

	VertexType_PT vertices;
	vertices.points     = points;
	vertices.texCoords  = texCoords;
	ScanConvert(numPoints, vertices);

	currentTexture = &texture;
	currentBlendFunc = BlendFunc;
	for (int y = spanY0; y < spanY1; ++y)
		DISPATCH(ScanLine_Textured_Blend, y);
	WAIT();
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

	currentTexture = &texture;
	currentBlendFunc = BlendFunc;
	for (int y = spanY0; y < spanY1; ++y)
		DISPATCH(ScanLine_ShadedTextured_Blend, y);
	WAIT();
}
