#ifndef SCAN_CONVERT_H
#define SCAN_CONVERT_H

#include "SoftwareRenderer.h"
#include "Interpolator.h"

// -- Span buffer -------------------------------------------------------------

struct Span
{
	struct Attribute
	{
		int     x;
		dword   color;
		Point2  texCoord;
	};

	Attribute span[2];  // start, end
};

extern Span *spans;
extern int   spanY0;
extern int   spanY1;


// -- Vertex types ------------------------------------------------------------

struct VertexType_P
{
	Point2 *points;
};

struct VertexType_PC
{
	Point2 *points;
	dword  *colors;
};

struct VertexType_PT
{
	Point2 *points;
	Point2 *texCoords;
};

struct VertexType_PCT
{
	Point2 *points;
	dword  *colors;
	Point2 *texCoords;
};

inline void ScanConvertLine(int topIndex, int bottomIndex, VertexType_P vertices, int side)
{
	int dy = vertices.points[bottomIndex].y - vertices.points[topIndex].y;
	if (dy == 0)
		return;

	real invDy = 1.0f / dy;
	Span *s = spans + vertices.points[topIndex].y;

	IntInterpolator position(vertices.points[topIndex].x, vertices.points[bottomIndex].x, invDy);

	for (int y = 0; y < dy; ++y)
	{
		s->span[side].x = position.Step();
		++s;
	}
}

inline void ScanConvertLine(int topIndex, int bottomIndex, VertexType_PC vertices, int side)
{
	int dy = vertices.points[bottomIndex].y - vertices.points[topIndex].y;
	if (dy == 0)
		return;

	real invDy = 1.0f / dy;
	Span *s = spans + vertices.points[topIndex].y;

	IntInterpolator   position(vertices.points[topIndex].x, vertices.points[bottomIndex].x, invDy);
	ColorInterpolator color(vertices.colors[topIndex], vertices.colors[bottomIndex], invDy);

	for (int y = 0; y < dy; ++y)
	{
		s->span[side].x     = position.Step();
		s->span[side].color = color.Step();
		++s;
	}
}

inline void ScanConvertLine(int topIndex, int bottomIndex, VertexType_PT vertices, int side)
{
	int dy = vertices.points[bottomIndex].y - vertices.points[topIndex].y;
	if (dy == 0)
		return;

	real invDy = 1.0f / dy;
	Span *s = spans + vertices.points[topIndex].y;

	IntInterpolator     position(vertices.points[topIndex].x, vertices.points[bottomIndex].x, invDy);
	Point2Interpolator  texCoord(vertices.texCoords[topIndex], vertices.texCoords[bottomIndex], invDy);

	for (int y = 0; y < dy; ++y)
	{
		s->span[side].x         = position.Step();
		s->span[side].texCoord  = texCoord.Step();
		++s;
	}
}

inline void ScanConvertLine(int topIndex, int bottomIndex, VertexType_PCT vertices, int side)
{
	int dy = vertices.points[bottomIndex].y - vertices.points[topIndex].y;
	if (dy == 0)
		return;

	real invDy = 1.0f / dy;
	Span *s = spans + vertices.points[topIndex].y;

	IntInterpolator     position(vertices.points[topIndex].x, vertices.points[bottomIndex].x, invDy);
	ColorInterpolator   color(vertices.colors[topIndex], vertices.colors[bottomIndex], invDy);
	Point2Interpolator  texCoord(vertices.texCoords[topIndex], vertices.texCoords[bottomIndex], invDy);

	for (int y = 0; y < dy; ++y)
	{
		s->span[side].x         = position.Step();
		s->span[side].color     = color.Step();
		s->span[side].texCoord  = texCoord.Step();
		++s;
	}
}

// -- Interface ---------------------------------------------------------------

template <class VERTEXTYPE>
void ScanConvert(dword numPoints, VERTEXTYPE vertices)
{
	// Find the top and bottom indices/points.
	int topIndex    = 0,
		bottomIndex = 0;

	for (dword i = 1; i < numPoints; ++i)
	{
		if (vertices.points[i].y < vertices.points[topIndex].y)
			topIndex = i;
		else if (vertices.points[i].y > vertices.points[bottomIndex].y)
			bottomIndex = i;
	}

	// Set up the min and max spans we're going to use.
	spanY0 = vertices.points[topIndex].y;
	spanY1 = vertices.points[bottomIndex].y;

	// Find the left and right indices.
	int leftIndex   = topIndex,
		rightIndex  = topIndex;

	if (--leftIndex < 0)
		leftIndex = numPoints - 1;

	if (++rightIndex >= (int)numPoints)
		rightIndex = 0;

	// If we guessed wrong, then we swap them here.
	// ToDo: backface culling?
	if (vertices.points[leftIndex].x > vertices.points[rightIndex].x)
		Swap(leftIndex, rightIndex);

	// Scan convert the left side.
	int leftTopIndex = topIndex;
	for (;;)
	{
		ScanConvertLine(leftTopIndex, leftIndex, vertices, 0);

		// Break if we've reached the bottom point.
		if (leftIndex == bottomIndex)
			break;

		// Continue with the next point.
		leftTopIndex = leftIndex;
		if (--leftIndex < 0)
			leftIndex = numPoints - 1;
	}

	// Scan convert the right side.
	int rightTopIndex = topIndex;
	for (;;)
	{
		ScanConvertLine(rightTopIndex, rightIndex, vertices, 1);

		// Break if we've reached the bottom point.
		if (rightIndex == bottomIndex)
			break;

		// Continue with the next point.
		rightTopIndex = rightIndex;
		if (++rightIndex >= (int)numPoints)
			rightIndex = 0;
	}
}

void InitScanConvert();
void QuitScanConvert();

#endif // SCAN_CONVERT_H
