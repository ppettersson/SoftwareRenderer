#include "SoftwareRenderer.h"
#include "TestSuite.h"
#include "Input.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const real PI = 3.1415926535f;

Image *texture1 = NULL,
      *texture2 = NULL,
      *texture3 = NULL,
      *texture4 = NULL,
      *texture5 = NULL;

static void TestHorizontalLines(real timePassed)
{
  for (int loop = 0; loop < 50; ++loop)
  {
    const int kNumLines = Min(screenHeight, screenWidth) - 10;

    for (int i = 0; i < kNumLines; i += 2)
    {
      DrawLine(0, i, i, i, kColorGreen);
      DrawPixel(0, i + 1, kColorWhite);
      DrawPixel(i, i + 1, kColorWhite);
    }
  }
}

static void TestVerticalLines(real timePassed)
{
  for (int loop = 0; loop < 50; ++loop)
  {
    const int kNumLines = Min(screenHeight, screenWidth) - 10;

    for (int i = 0; i < kNumLines; i += 2)
    {
      DrawLine(i, 0, i, i, kColorGreen);
      DrawPixel(i + 1, 0, kColorWhite);
      DrawPixel(i + 1, i, kColorWhite);
    }
  }
}

// Draw a bunch of lines in a circle.
static void TestLines(real timePassed)
{
  const int kNumLines = 1000;
  
  real length = (real)(Min(screenWidth, screenHeight) / 2 - 10);

  for (int i = 0; i < kNumLines; ++i)
  {
    real angle = (i / (real)kNumLines) * 2.0f * PI;

    dword  x0 = screenWidth / 2,
                  y0 = screenHeight / 2,
                  x1 = (dword)(x0 + cosf(angle) * length),
                  y1 = (dword)(y0 + sinf(angle) * length);

    DrawLine(x0, y0, x1, y1, kColorGreen);
  }
}

static void TestPolygon(real timePassed)
{
  const int kNumPoints = 5;
  Point2 points[kNumPoints] = 
  {
    Point2((dword)(0.2f * screenWidth), (dword)(0.4f * screenHeight)),
    Point2((dword)(0.5f * screenWidth), (dword)(0.2f * screenHeight)),
    Point2((dword)(0.8f * screenWidth), (dword)(0.4f * screenHeight)),
    Point2((dword)(0.6f * screenWidth), (dword)(0.8f * screenHeight)),
    Point2((dword)(0.4f * screenWidth), (dword)(0.75f * screenHeight))
  };

  dword colors[kNumPoints] =
  {
    kColorRed,
    kColorGreen,
    kColorBlue,
    kColorLightGrey,
    kColorDarkGrey
  };

  Point2 texCoords[kNumPoints] =
  {
    Point2(0, 128),
    Point2(128, 0),
    Point2(255, 128),
    Point2(200, 255),
    Point2(100, 255)
  };

  //// First wobble the points a bit.
  for (int i = 0; i < kNumPoints; ++i)
  {
    const real kAmount     = 20.0f;
    const real kTimeAdjust = 0.01f;

    points[i].x += (dword)(kAmount * sinf(points[i].x * timePassed * kTimeAdjust));
    points[i].y += (dword)(kAmount * cosf(points[i].y * timePassed * kTimeAdjust));
  }

  //// Then rotate the points.
  //for (int i = 0; i < kNumPoints; ++i)
  //{
  //  const real kTimeAdjust = 0.5f;
  //  const real kScale      = 1.5f;

  //  real x = points[i].x - screenWidth / 2.0f,
  //        y = points[i].y - screenHeight / 2.0f;

  //  points[i].x = (dword)(x * kScale * sinf(timePassed * kTimeAdjust) + screenWidth / 2.0f);
  //  points[i].y = (dword)(y * kScale * cosf(timePassed * kTimeAdjust) + screenHeight / 2.0f);
  //}

  //DrawPolygon_Flat(kNumPoints, points, kColorGreen);
  //DrawPolygon_Shaded(kNumPoints, points, colors);
  //DrawPolygon_Textured(kNumPoints, points, texCoords, texture);
  DrawPolygon_ShadedTextured(kNumPoints, points, texCoords, colors, *texture1);

  for (int i = 0; i < kNumPoints; ++i)
    DrawPixel(points[i].x, points[i].y, kColorWhite);

  for (int i = 0; i < kNumPoints - 1; ++i)
    DrawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, kColorWhite);
  DrawLine(points[kNumPoints - 1].x, points[kNumPoints - 1].y, points[0].x, points[0].y, kColorWhite);
}

static void TestBlendedPolygon(real timePassed)
{
  const int kNumPoints = 4;
  Point2 points[kNumPoints] =
  {
    Point2((dword)(0.5f * screenWidth), (dword)(0.1f * screenHeight)),
    Point2((dword)(0.9f * screenWidth), (dword)(0.5f * screenHeight)),
    Point2((dword)(0.5f * screenWidth), (dword)(0.9f * screenHeight)),
    Point2((dword)(0.1f * screenWidth), (dword)(0.5f * screenHeight))
  };

  Point2 texCoords[kNumPoints] =
  {
    Point2(0, 0),
    Point2(255, 0),
    Point2(255, 255),
    Point2(0, 255)
  };

  DrawPolygon_Textured_Blend(kNumPoints, points, texCoords, *texture2, BlendScreen1);
}

static void TestTriangle(real timePassed)
{
  Point2 points[] =
  {
    Point2((dword)(0.5f * screenWidth), (dword)(0.1f * screenHeight)),
    Point2((dword)(0.9f * screenWidth), (dword)(0.9f * screenHeight)),
    Point2((dword)(0.1f * screenWidth), (dword)(0.9f * screenHeight))
  };

  //const int kNumPoints = sizeof(points) / sizeof(points[0]);

  DrawTriangle_Flat(points, kColorWhite);
}

static void TestBlendedBlit(real timePassed)
{
  Image *t0 = texture4;
  Image *t1 = texture3;
  
  Blit(t0, 0, 512);  DrawString(0, 512, "tex 0");
  Blit(t1, 256, 512);  DrawString(256, 512, "tex 1");

  int maxY = screenHeight / t0->height,
      maxX = screenWidth / t0->width;

  for (int y = 0; y < maxY; ++y)
    for (int x = 0; x < maxX; ++x)
    {
      int func = x + y * maxX;
      if (func == kBlend_Max)
        return;

      int x0 = x * t0->width,
          y0 = y * t0->height;

      if ((x0 + t0->width > screenWidth) || (y0 + t0->height > screenHeight))
        return;

      Blit(t0, x0, y0);
      BlitBlend(t1, x0, y0, BlendFunc((BlendType)func));

      DrawString(x0, y0, BlendFuncName((BlendType)func));
    }
}

static void TestCrossFade(real timePassed)
{
  Image *t0 = texture3;
  Image *t1 = texture4;

  static real accumulatedTime = 0;
  accumulatedTime += (timePassed * 0.0005f);

  CrossFade(frameBuffer, screenWidth, t1->data, t0->data, t0->width, t0->height, (byte)(Abs(sinf(accumulatedTime)) * 255));
}

static void TestStretchBlit(real timePassed)
{
  static dword w = 1, h = 1;
  static bool wUp = true, hUp = true;
  
  if (wUp)
  {
    if (w < screenWidth)
      ++w;
    else
      wUp = false;
  }
  else
  {
    if (w > 1)
      --w;
    else
      wUp = true;
  }
  if (hUp)
  {
    if (h < screenHeight)
      ++h;
    else
      hUp = false;
  }
  else
  {
    if (h > 1)
      --h;
    else
      hUp = true;
  }

  Blit(texture3, 0, 0, w, h);
  //Blit(texture5, 0, 0, w, h);

  //Blit(texture5, 0, 0, 256, 256);
}

bool InitTestSuite()
{
  texture1 = LoadTGA("test.tga");   // Lionhead
  texture2 = LoadTGA("test2.tga");  // Wooden wall
  texture3 = LoadTGA("test3.tga");  // Earth
  texture4 = LoadTGA("test4.tga");  // Lena

  if (!texture1 || !texture2 || !texture3 || !texture4)
    return false;

  //texture5 = new Image(2, 2);
  //memset(texture5->data, 0xff, 2 * sizeof(dword));
  //memset(texture5->data + 2, 0, 2 * sizeof(dword));

  texture5 = new Image(2, 2);
  texture5->data[0] = 0xffffffff;
  texture5->data[1] = 0xffffffff;
  texture5->data[2] = 0x00000000;
  texture5->data[3] = 0x00000000;

  CreateAlphaChannel(texture3, false);

  //texture.Resize(512, 512);
  //texture2.Resize(512, 512);
  //texture3.Resize(512, 512);
  //texture4.Resize(512, 512);

  return true;
}

struct Test
{
  const char *name;
  void (*func)(real timePassed);
} tests[] =
{
  { "horizontal lines", TestHorizontalLines },
  { "vertical lines", TestVerticalLines },
  { "lines", TestLines },
  { "polygon", TestPolygon },
  { "blended polygon", TestBlendedPolygon },
  { "triangle", TestTriangle },
  { "blended blit", TestBlendedBlit },
  { "crossfade", TestCrossFade },
  { "stretch blit", TestStretchBlit }
};

#define NumElements(x)  (sizeof(x) / sizeof(x[0]))

const int kNumTests = NumElements(tests);

static int currentTest = kNumTests - 1;

void RunTestSuite()
{
  float timePassed = 1000.0f / 60;  // tmp

  if ((currentTest > 0) && KeyDown(kKey_Left))
  {
    --currentTest;
  }
  else if ((currentTest < (kNumTests - 1) && KeyDown(kKey_Right)))
  {
    ++currentTest;
  }

  tests[currentTest].func(timePassed);

  char message[256];
  sprintf(message, "Test %d/%d, %s", currentTest, kNumTests, tests[currentTest].name);
  DrawString(20, 20, message);
}

void QuitTestSuite()
{
  delete texture1;
  delete texture2;
  delete texture3;
  delete texture4;
  delete texture5;
}
