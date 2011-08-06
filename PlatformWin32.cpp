#ifdef _WIN32

#include "SoftwareRenderer.h"
#include "Platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// The BITMAPINFO struct only defines one color and is supposed to be used with
// dynamic allocation, so instead derive from that and add the two extra colors
// we need to set up the bitfields.
struct BITMAPINFO_EX : public BITMAPINFO
{
  RGBQUAD extraColors[2];
};

// The window handle.
static HWND           window = NULL;
static HDC            deviceContext = NULL;
static BITMAPINFO_EX  bitmapInfo;
static dword         *currentFrameBuffer = NULL;

static LRESULT CALLBACK MessageHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_PAINT:
    if (currentFrameBuffer)
    {
      RECT clientRect;
      GetClientRect(window, &clientRect);
      StretchDIBits(deviceContext, 0, 0, clientRect.right, clientRect.bottom, 0, 0, 
        bitmapInfo.bmiHeader.biWidth, -bitmapInfo.bmiHeader.biHeight, currentFrameBuffer,
        &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
      ValidateRect(window, NULL);
    }
    break;

  case WM_KEYDOWN:
    // Close on Escape key down.
    if ((wParam & 0xff) != 27)
      break;

    // Fall through.

  case WM_CLOSE:
    PlatformClose();
    ExitProcess(0);
    break;
  }

  return DefWindowProc(window, message, wParam, lParam);
}

bool PlatformOpen(dword width, dword height)
{
  #define CLASS_NAME  TEXT("SoftwareRenderer")

  // Register a regular window.
  WNDCLASSEX windowClass;
  memset(&windowClass, 0, sizeof(windowClass));
  windowClass.cbSize = sizeof(windowClass);
  windowClass.style = CS_OWNDC;
  windowClass.lpfnWndProc = MessageHandler;
  windowClass.lpszClassName = CLASS_NAME;
  windowClass.hCursor = LoadCursor(0, IDC_ARROW);
  if (!RegisterClassEx(&windowClass))
    return false;

  // The window size needs to be adjusted for the window frame.
  RECT r = { 0, 0, width, height };
  if (!AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, FALSE, 0))
    return false;
  r.right -= r.left;
  r.bottom -= r.top;

  // Create the window.
  window = CreateWindowEx(0, CLASS_NAME, CLASS_NAME, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, r.right, r.bottom, NULL, NULL, NULL, NULL);
  if (window == NULL)
    return false;
  ShowWindow(window, SW_SHOW);

  // Grab and hold on to the device context.
  deviceContext = GetDC(window);
  if (!deviceContext)
    return false;

  // Set up the bitmap that we're going to blit to the screen.
  memset(&bitmapInfo, 0, sizeof(bitmapInfo));
  BITMAPINFOHEADER &bitmapHeader = bitmapInfo.bmiHeader;
  bitmapHeader.biSize = sizeof(bitmapHeader);
  bitmapHeader.biWidth = width;
  bitmapHeader.biHeight = -((int)height);  // Make the bitmap top-down instead of the default bottom-up.
  bitmapHeader.biPlanes = 1;
  bitmapHeader.biBitCount = 32;
  bitmapHeader.biCompression = BI_BITFIELDS;  // To let Windows know the RGB order.
  ((dword *)bitmapInfo.bmiColors)[0] = 0x00ff0000;
  ((dword *)bitmapInfo.bmiColors)[1] = 0x0000ff00;
  ((dword *)bitmapInfo.bmiColors)[2] = 0x000000ff;
  return true;
}

bool PlatformUpdate(dword *frameBuffer)
{
  // Store the buffer to draw.
  currentFrameBuffer = frameBuffer;

  // Trigger a repaint.
  InvalidateRect(window, NULL, FALSE);
  SendMessage(window, WM_PAINT, 0, 0);

  // Process any Window message that's been queued up.
  MSG msg;
  while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Yield this thread to play nice with other programs.
  Sleep(0);
  return true;
}

void PlatformSetWindowCaption(const char *caption)
{
  SetWindowText(window, caption);
}

void PlatformClose()
{
  currentFrameBuffer = NULL;
  ReleaseDC(window, deviceContext);
  DestroyWindow(window);
  window = NULL;
  deviceContext = NULL;
}

extern int main();
int CALLBACK WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine, int show)
{
  return main();
}

#endif // _WIN32
