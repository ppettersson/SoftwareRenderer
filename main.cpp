#include "SoftwareRenderer.h"
#include "Timer.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

extern "C" { extern HWND wnd; }

const real PI = 3.1415926535f;

Image *texture1 = NULL,
      *texture2 = NULL,
      *texture3 = NULL,
      *texture4 = NULL,
      *texture5 = NULL;


// Utility function to handle key presses.
// Note that it blocks execution until key is released (to avoid polling -> event handling)
bool KeyDown(int code)
{
  if (GetAsyncKeyState(code) & 0x8000)
  {
    while (GetAsyncKeyState(code) & 0x8000)
      ;

    return true;
  }

  return false;
}

// Draw a bunch of horizontal lines.
void TestLines0()
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

// Draw a bunch of vertical lines.
void TestLines1()
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
void TestLines2()
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

void TestPolygon(real timePassed)
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

void TestBlendedPolygon()
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

void TestTriangle()
{
  Point2 points[] =
  {
    Point2((dword)(0.5f * screenWidth), (dword)(0.1f * screenHeight)),
    Point2((dword)(0.9f * screenWidth), (dword)(0.9f * screenHeight)),
    Point2((dword)(0.1f * screenWidth), (dword)(0.9f * screenHeight))
  };

  const int kNumPoints = sizeof(points) / sizeof(points[0]);

  DrawTriangle_Flat(points, kColorWhite);
}

void TestBlendedBlit()
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

void TestCrossFade(real timePassed)
{
  Image *t0 = texture3;
  Image *t1 = texture4;

  CrossFade(frameBuffer, screenWidth, t1->data, t0->data, t0->width, t0->height, (byte)(Abs(sinf(timePassed)) * 255));
}

void TestStretchBlit(real timePassed)
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

int main(int argc, char **argv)
{
  if (!Init(1024, 768))
    return -1;

  texture1 = LoadTGA("test.tga");   // Lionhead
  texture2 = LoadTGA("test2.tga");  // Wooden wall
  texture3 = LoadTGA("test3.tga");  // Earth
  texture4 = LoadTGA("test4.tga");  // Lena

  if (!texture1 || !texture2 || !texture3 || !texture4)
    return -1;

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

  InitTimer();

  for (;;)
  {
    UpdateTimer();    

    char msg[256];
    sprintf_s(msg, 256, "SoftwareRenderer - [FPS = %d | %.1f]", fpsAverage, fps);
    SetWindowText(wnd, msg);


    // Clear screen to non-black to detect pixel errors easier.
    Clear(kColorLightGrey);

    // Test cases.
    //TestLines2();
    //TestPolygon(timePassed);
    //TestBlendedPolygon();

    //TestCrossFade(timePassed);
    //TestStretchBlit(timePassed);
    TestBlendedBlit();
    //TestTriangle();

    // Swap buffers.
    if (!PresentFrame())
      break;
  }

  Quit();
  return 0;
}
