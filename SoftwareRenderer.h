#ifndef SOFTWARE_RENDERER_H
#define SOFTWARE_RENDERER_H

// -- Config ------------------------------------------------------------------

#define USE_MMX_ASM

// -- Types -------------------------------------------------------------------

typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   dword;

typedef float           real;

class Point2
{
public:
  dword  x, y;

  Point2()                                      { }
  Point2(dword _x, dword _y) : x(_x), y(_y)     { }
};

class Image
{
public:
  Image() : width(0), height(0), data(0)        { }
  Image(dword w, dword h) : width(w), height(h) { data = new dword [w * h]; }
  ~Image()                                      { if (data) delete [] data; }

  dword  width,
         height,
        *data;
};

// -- FrameBuffer -------------------------------------------------------------

bool Init(dword width, dword height);
void Quit();
bool PresentFrame();

void Clear(dword color);


extern dword  screenWidth;
extern dword  screenHeight;
extern dword *frameBuffer;

// -- Color -------------------------------------------------------------------

// Color conversion routines for the ARGB8888 format.

const dword kColorBlack      = 0x00000000;
const dword kColorWhite      = 0x00ffffff;
const dword kColorRed        = 0x00ff0000;
const dword kColorGreen      = 0x0000ff00;
const dword kColorBlue       = 0x000000ff;
const dword kColorLightGrey  = 0x00aaaaaa;
const dword kColorDarkGrey   = 0x00111111;

const dword kColorAlpha      = 0xff000000;


// From individual byte components to RGBA pixel.
inline dword Color(byte red, byte green, byte blue)             { return (blue << 0) | (green << 8) | (red << 16); }
inline dword Color(byte red, byte green, byte blue, byte alpha) { return (alpha << 24) | Color(red, green, blue); }

// From individual real components to RGBA pixel.
inline dword Color(real red, real green, real blue)             { return Color((byte)(red * 255.0f), (byte)(green * 255.0f), (byte)(blue * 255.0f)); }

// Convert from RGBA pixel to individual real components.
inline void Color(dword color, real &red, real &green, real &blue)
{
  red   = ((color & kColorRed)   >> 16) / 255.0f;
  green = ((color & kColorGreen) >>  8) / 255.0f;
  blue  = ((color & kColorBlue)  >>  0) / 255.0f;
}

// Convert from RGBA pixel to individual integer components.
inline void Color(dword color, byte &red, byte &green, byte &blue)
{
  red   = (byte)((color & kColorRed)   >> 16);
  green = (byte)((color & kColorGreen) >>  8);
  blue  = (byte)((color & kColorBlue)  >>  0);
}

// Scale the color.
void ColorScale(dword &color, real scale);
void ColorScale(dword &color, dword s);

// -- Misc --------------------------------------------------------------------

template <class T>  T Abs(T x)                    { return x < 0 ? -x : x; }
template <class T>  T Max(const T &a, const T &b) { return a > b ? a : b; }
template <class T>  T Min(const T &a, const T &b) { return a < b ? a : b; }
template <class T>  void Swap(T &a, T &b)         { T tmp = a; a = b; b = tmp; }

template <class T>
T Clamp(const T &x, const T &min, const T &max)
{
  if (x < min)
    return min;
  else if (x > max)
    return max;
  else
    return x;
}

#ifdef USE_MMX_ASM
void MemCpy(void *dst, const void *src, size_t count);
void MemSet32(void *dst, dword src, size_t count);
#else
#include <memory.h>
#define MemCpy  memcpy
#endif

#define ASSERT(x)   { if (!(x)) { __asm int 3 } }

// -- Image -------------------------------------------------------------------

Image *LoadTGA(const char *fileName);

void Blit(Image *image, dword x, dword y);
void Blit(Image *image, dword x, dword y, dword w, dword h);
void BlitBlend(Image *image, dword x, dword y, void (BlendFunc)(dword *src, dword *dst, dword num));
void Resize(Image *image, dword newWidth, dword newHeight);
void CreateAlphaChannel(Image *image, bool mask);

// -- DrawPrimitives ----------------------------------------------------------

void DrawPixel(dword x, dword y, dword color);
void DrawPixelBlend(dword x, dword y, dword color, dword (BlendFunc)(dword src, dword dst));
void DrawLine(dword x0, dword y0, dword x1, dword y1, dword color);

void DrawPolygon_Flat(dword numPoints, Point2 *points, dword color);
void DrawPolygon_Shaded(dword numPoints, Point2 *points, dword *colors);
void DrawPolygon_Textured(dword numPoints, Point2 *points, Point2 *texCoords, Image &texture);
void DrawPolygon_ShadedTextured(dword numPoints, Point2 *points, Point2 *texCoords, dword *colors, Image &texture);

void DrawPolygon_Textured_Blend(dword numPoints, Point2 *points, Point2 *texCoords, Image &texture, dword (*BlendFunc)(dword src, dword dst));
void DrawPolygon_ShadedTextured_Blend(dword numPoints, Point2 *points, Point2 *texCoords, dword *colors, Image &texture, dword (*BlendFunc)(dword src, dword dst));

void DrawTriangle_Flat(Point2 *points, dword color);

void DrawString(dword x, dword y, const char *text, dword color = 0xffffffff);


// -- Blend -------------------------------------------------------------------

// All the blend functions use MMX but don't clear the EMMS state after they've been called.
// After you're finished with all the blend operations or if you need to use floating point
// math you need to call the Emms() function.

// These functions are a bit of a waste since they only work on 32 bits instead
// of processing multiple pixels at the same time.
extern dword (*BlendNormal1)(dword src, dword dst);
extern dword (*BlendMultiply1)(dword src, dword dst);
extern dword (*BlendAdditive1)(dword src, dword dst);
extern dword (*BlendSubtractive1)(dword src, dword dst);
extern dword (*BlendScreen1)(dword src, dword dst);
extern dword (*BlendLighten1)(dword src, dword dst);
extern dword (*BlendDarken1)(dword src, dword dst);

// These blend functions operate on multiple pixels at a time.
extern void (*BlendNormal)(dword *src, dword *dst, dword num);
extern void (*BlendMultiply)(dword *src, dword *dst, dword num);
extern void (*BlendAdditive)(dword *src, dword *dst, dword num);
extern void (*BlendSubtractive)(dword *src, dword *dst, dword num);
extern void (*BlendScreen)(dword *src, dword *dst, dword num);
extern void (*BlendLighten)(dword *src, dword *dst, dword num);
extern void (*BlendDarken)(dword *src, dword *dst, dword num);

// Leave mmx mode to restore floating point functionality.
// This must be called after any of the Blend functions.
#if defined(USE_MMX_ASM)
inline void BlendEnd()  { __asm emms }
#else
inline void BlendEnd()  { }
#endif

typedef void (*BlendFunc_t)(dword *src, dword *dst, dword num);

enum BlendType
{
  kBlend_Normal,
  kBlend_Multiply,
  kBlend_Additive,
  kBlend_Subtractive,
  kBlend_Screen,
  kBlend_Lighten,
  kBlend_Darken,

  kBlend_Max
};

BlendFunc_t BlendFunc(BlendType t);
const char *BlendFuncName(BlendType t);

// -- CrossFade ---------------------------------------------------------------

void CrossFade(dword *dst, dword stride, 
               dword *srcA, dword *srcB,
               dword width, dword height, dword alpha);

// -- StretchBlit -------------------------------------------------------------

void StretchBlitNearest(dword *dst, dword dstStride,
                        dword dstX, dword dstY,
                        dword dstWidth, dword dstHeight,
                        dword *src, dword srcStride, 
                        dword srcWidth, dword srcHeight);

void StretchBlitBiLinear(dword *dst, dword dstStride,
                         dword dstX, dword dstY,
                         dword dstWidth, dword dstHeight,
                         dword *src, dword srcStride, 
                         dword srcWidth, dword srcHeight);


#endif // SOFTWARE_RENDERER_H
