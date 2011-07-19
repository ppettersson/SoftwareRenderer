#include "SoftwareRenderer.h"

#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4799) // function 'x' has no EMMS instruction

void StretchBlitNearest(dword *dst, dword dstStride,
                        dword dstX, dword dstY,
                        dword dstWidth, dword dstHeight,
                        dword *src, dword srcStride, 
                        dword srcWidth, dword srcHeight)
{
  // The ratio between source and destination dimensions.
  // This is used as the step value when we increment for each pixel.
  dword  incX = (srcWidth << 16) / dstWidth,
         incY = (srcHeight << 16) / dstHeight;

  // The fixed point y position.
  dword y = 0;

  // Step through each row.
  for (dword h = 0; h < dstHeight; ++h)
  {
    // The fixed point x position.
    dword x = 0;

    // Clamp the fixed point y position to the final int position
    // and pre multiply the stride for quicker access.
    dword yOffset = (y >> 16) * srcStride;

    // Step through each column in this row.
    for (dword w = 0; w < dstWidth; ++w)
    {
      // Clamp the fixed point x position to the final int position
      // and copy the pixel found there.
      dst[w] = src[(x >> 16) + yOffset];

      // Step incrementally in the source position.
      x += incX;
    }

    // Move on to the next line.
    dst += dstStride;
    y += incY;
  }
}

#if 1
void StretchBlitBiLinear(dword *dst, dword dstStride,
                         dword dstX, dword dstY,
                         dword dstWidth, dword dstHeight,
                         dword *src, dword srcStride, 
                         dword srcWidth, dword srcHeight)
{
  ASSERT(srcWidth >= 2 && srcHeight >= 2 && dstWidth >= 2 && dstHeight >= 2);

  // Calculate the step values needed when stretching the source dimensions to
  // the destination dimensions.
  real  incX = srcWidth / (real)dstWidth,
        incY = srcHeight / (real)dstHeight;

  // The actual y coordinate that is clamped to get the integer coordinate that
  // is sampled from. The difference is then used as the scale factor when 
  // blending.
  real y = 0;
  
  // Guard against overflow by skipping the last row.
  for (dword h = 0; h < dstHeight - 1; ++h)
  {
    // Clamp down the y values for this and the next line.
    dword v0    = (dword)y,
          v1    = v0 + 1;

    // Get the percentage of influence each of those two lines have on the
    // result.
    real  dy    = y - (real)v0,
          invDy = 1.0f - dy;

    // Offsets into the data streams.
    dword v0Offset = v0 * srcStride,
          v1Offset = v1 * srcStride;

    // The actual x coordinate that is clamped to get the integer coordinate
    // that is sampled from. The difference is then used as the scale factor
    // when blending.
    real x = 0;

    // Guard against overflow by skipping the last column.
    dword w = 0;
    for (; w < dstWidth - 1; ++w)
    {
      // Clamp down the x values for this and the next column.
      dword u0 = (dword)x,
            u1 = u0 + 1;

      // Grad the color values for the four neighboring pixels that are being
      // blended into one.
      dword color00 = src[u0 + v0Offset], // ToDo: Optimization: should be possible to grab both of
            color10 = src[u1 + v0Offset], //       these two in one 64 bit read. We could also save
            color01 = src[u0 + v1Offset], //       this for the next step to avoid grabbing more
            color11 = src[u1 + v1Offset]; //       than necessary.

      // Get the percentage of influence each of the two columns have on the
      // result.
      real dx     = x - (real)u0,
           invDx  = 1.0f - dx;

      // Scale each of the four colors with their position relative to the sampled point.
      ColorScale(color00, invDx * invDy);
      ColorScale(color10, dx    * invDy);
      ColorScale(color01, invDx * dy);
      ColorScale(color11, dx    * dy);

      // And we have the final result.
      dst[w] = color00 + color01 + color10 + color11;

      x += incX;
    }

    // Handle the last pixel / column.
    dword u0 = (dword)x;

    // Grad the color values for the two neighboring pixels that are being
    // blended into one.
    dword color00 = src[u0 + v0Offset],
          color01 = src[u0 + v1Offset];

    // Scale each of the two colors with their position relative to the sampled point.
    ColorScale(color00, invDy);
    ColorScale(color01, dy);

    // And we have the final result.
    dst[w] = color00 + color01;


    // Move on to the next row.
    dst += dstStride;
    y += incY;
  }

  // Handle the last row.

  // Clamp down the y values for this and the next line.
  dword v0    = (dword)y;

  // Offsets into the data streams.
  dword v0Offset = v0 * srcStride;

  // The actual x coordinate that is clamped to get the integer coordinate
  // that is sampled from. The difference is then used as the scale factor
  // when blending.
  real x = 0;

  // Guard against overflow by skipping the last column.
  dword w = 0;
  for (; w < dstWidth - 1; ++w)
  {
    // Clamp down the x values for this and the next column.
    dword u0 = (dword)x,
          u1 = u0 + 1;

    // Grad the color values for the two neighboring pixels that are being
    // blended into one.
    dword color00 = src[u0 + v0Offset],
          color10 = src[u1 + v0Offset];

    // Get the percentage of influence each of the two columns have on the
    // result.
    real  dx     = x - (real)u0,
          invDx  = 1.0f - dx;

    // Scale each of the two colors with their position relative to the sampled point.
    ColorScale(color00, invDx);
    ColorScale(color10, dx);

    // And we have the final result.
    dst[w] = color00 + color10;

    x += incX;
  }

  // Handle the last pixel.
  dst[w] = src[(dword)x + v0Offset];
}
#elif 0

void StretchBlitBiLinear(dword *dst, dword dstStride,
                         dword dstX, dword dstY,
                         dword dstWidth, dword dstHeight,
                         dword *src, dword srcStride, 
                         dword srcWidth, dword srcHeight)
{
  int dx = (srcWidth << 16) / dstWidth;
  int dy = (srcHeight << 16) / dstHeight;
  int pixels = (srcWidth * srcHeight) - 1;

  int startx = 0;
  int starty = 0;

  int ccy = starty;

  for (int yy = 0; yy < dstHeight; yy++)
  {
    int ty = (ccy >> 16) * srcWidth;
    int ccx = startx;
    dword *current = dst;

    __int64 ddv = ((ccy) >> 8) & 255;
    ddv += (ddv << 32) + (ddv << 16);

    for (int xx = 0; xx < dstWidth; xx++)
    {
      int ad1 = ty + (ccx >> 16);
      int ad2 = ad1 + 1;
      int ad3 = ad1 + srcWidth;
      int ad4 = ad2 + srcWidth;

      __asm
      {
        // for packing/unpacking
        pxor    mm7, mm7
        mov     eax, [ccx]
        shr     eax, 8
        and     eax, 255
        movd    mm4, eax
        movq    mm5, mm4
        psllq   mm5, 16
        paddd   mm4, mm5
        psllq   mm5, 16
        // convert ddu to 64 bit
        paddd   mm4, mm5

        mov     edx, [pixels]
        mov     eax, [ad1]
        cmp     eax, edx
        jl      noclipad1
        mov     eax, edx

      noclipad1:
        mov     ebx, [ad2]
        cmp     ebx, edx
        jl      noclipad2
        mov     ebx, edx

      noclipad2:
        mov     ecx, [ad3]
        cmp     ecx, edx
        jl      noclipad3
        mov     ecx, edx

      noclipad3:
        mov     edx, [ad4]
        cmp     edx, [pixels]
        jl      noclipad4
        mov     edx, [pixels]

      noclipad4:
        mov     esi, [src]

  ///////////////////////////////////////////////////////

        // mm5 = color1 + (ddu * (color2 - color1));
        movd    mm5, [esi + eax * 4] // color1
        movd    mm1, [esi + ebx * 4] // color2

        punpcklbw mm5, mm7           // convert to 64 bit
        punpcklbw mm1, mm7           // convert to 64 bit

        movq    mm3, mm5             // save color1 for later

        pmullw  mm1, mm4             // mul color2 with ddu(mm4)
        psrlw   mm1, 8               // div by 256
        paddb   mm5, mm1             // add to color1

        pmullw  mm3, mm4             // mul color1 with ddu(mm4)
        psrlw   mm3, 8               // div by 256
        psubb   mm5, mm3             // sub from color1...

  ///////////////////////////////////////////////////////

        // mm6 = color3 + (ddu * (color4 - color3));
        movd    mm6, [esi + ecx * 4] // color3
        movd    mm1, [esi + edx * 4] // color4

        punpcklbw mm6, mm7           // convert to 64 bit
        punpcklbw mm1, mm7           // convert to 64 bit

        movq    mm3, mm6             // save color3 for later

        pmullw  mm1, mm4             // mul color4 with ddu
        psrlw   mm1, 8               // div by 256
        paddb   mm6, mm1             // add to color3

        pmullw  mm3, mm4             // mul color3 with ddu(mm4)
        psrlw   mm3, 8               // div by 256
        psubb   mm6, mm3             // sub from color3...

  //////////////////////////////////////////////////////

        movq    mm4, [ddv]

        movq    mm3, mm5

        pmullw  mm6, mm4
        psrlw   mm6, 8
        paddb   mm5, mm6

        pmullw  mm3, mm4
        psrlw   mm3, 8
        psubb   mm5, mm3

        packuswb mm5, mm7

        mov     esi, [current]
        movd    [esi], mm5           // done!

        add     current, 4
      }
      ccx += dx;
    }
    ccy += dy;
    dst += dstStride;
  }
  __asm emms;
}

#else

typedef unsigned int ARGB0;

const unsigned __int64 mm_zero = 0;

#ifdef _DEBUG
__declspec(naked)
#endif
__forceinline void setup_bilinear_colors()
{
    /*
     * Assumes
     *  mm0: argb00
     *  mm1: argb10
     *  mm2: argb01
     *  mm3: argb11
     *  mm7: v|v|v|v (in .14 format)
     *
     * Returns:
     *  mm2: argb00 + v * (argb01 - argb00) (in .4 format)
     *  mm3: argb10 + v * (argb11 - argb10) (in .4 format)
     */
    __asm
    {
        punpcklbw mm0, [mm_zero]    // argb00
        punpcklbw mm1, [mm_zero]    // argb10
        punpcklbw mm2, [mm_zero]    // argb01
        punpcklbw mm3, [mm_zero]    // argb11

                            // T0
        psubw mm2, mm0      // (argb01 - argb00).0
        psllw mm2, 6        // (argb01 - argb00).6
        pmulhw mm2, mm7     // v.14 * (argb01 - argb00).6 / .16
        psllw mm0, 4        // (argb00).4
        paddw mm2, mm0      // (argb00).4 + (v * (argb01 - argb00)).4

                            // T1
        psubw mm3, mm1      // (argb11 - argb10).0
        psllw mm3, 6        // (argb11 - argb10).6
        pmulhw mm3, mm7     // v.14 * (argb11 - argb10).6 / .16
        psllw mm1, 4        // (argb10).4
        paddw mm3, mm1      // (argb10).4 + (v * (argb11 - argb10)).4

#ifdef _DEBUG
        ret
#endif
    }
}

#ifdef _DEBUG
__declspec(naked)
#endif
__forceinline void bilinear_blend_colors4()
{
    /*
     * Assumes:
     *  mm2: T0: argb00 + v * (argb01 - argb00) (in .4 format)
     *  mm3: T1: argb10 + v * (argb11 - argb10) (in .4 format)
     *  ebx: u
     *
     * Returns:
     *  packed color in mm4
     *
     * Stomps:
     *  mm4, mm6
     *
     */
    __asm
    {
        movd mm6, ebx       // 0| 0| 0| u
        punpcklwd mm6, mm6  // 0| 0| u| u
        punpckldq mm6, mm6  // u| u| u| u

        movq mm4, mm3       // (T1).4
        psubw mm4, mm2      // (T1 - T0).4
        pmulhw mm4, mm6     // u.14 * (T1 - T0).4 / .16
        psllw mm4, 2        // (u * (T1 - T0)).4
        paddw mm4, mm2      // (T0).4 + (u * (T1 - T0)).4

        psraw mm4, 4        // T0 + u * (T1 - T0)

        packuswb mm4, mm4

#ifdef _DEBUG
        ret
#endif
    }
}

void BB_BilinearStretchBlitARGB8888(
    void *dest, int dx, int dy, int dw, int dh, int dpitch,
    void *src,  int sx, int sy, int sw, int sh, int spitch)
{
    int r_um;
    int r_v;
    int r_vm;

    int safe_lines;
    int unsafe_lines;

    unsigned int SrcPitch = spitch / sizeof(ARGB0);
    unsigned int DestPitch = dpitch / sizeof(ARGB0);
    ARGB0 *Src1 = (ARGB0 *)src + sy * SrcPitch + sx;
    ARGB0 *Src2 = Src1 + SrcPitch;
    ARGB0 *Dest = (ARGB0 *)dest + dy * DestPitch + dx;

    ARGB0 *Src1Max = Src1 + sw - 1;
    ARGB0 *Src2Max = Src2 + sw - 1;
    ARGB0 *SrcMax = Src1 + sh * SrcPitch;

    if(dw < 1 || dh < 1 || sw < 2 || sh < 2)
        return;

    r_um = 0x4000 * sw / dw;
    r_v = 0;
    r_vm = 0x4000 * sh / dh;

    safe_lines = ((sw - 2) << 14) / r_um;
    unsafe_lines = dw - safe_lines;

    __asm pxor mm7, mm7                 //  v

    while(dh--)
    {
        __asm
        {
            mov edi, [Dest]
            mov esi, [Src1]
            mov edx, [Src2]

            push ebx
            xor ebx, ebx                // u

            movd mm0, [esi]             // Src[0]
            movd mm1, [esi + 4]         // Src[1]
            movd mm2, [edx]             // Src2[0]
            movd mm3, [edx + 4]         // Src2[1]
        }

        setup_bilinear_colors();

        __asm
        {
            mov ecx, [safe_lines]
            test ecx, ecx
            jz done_safe_lines

do_safe_lines:
        }

        bilinear_blend_colors4();

        __asm
        {
            movd [edi], mm4
            add edi, 4

            add ebx, [r_um]

            cmp ebx, 0x4000             // if(u > 0x4000)
            jb L1

            mov eax, ebx
            shr eax, 14                 // u >> 14
            and ebx, 0x3fff             // u & (0x4000 - 1)

            lea esi, [esi + eax * 4]
            lea edx, [edx + eax * 4]

            movd mm0, [esi]             // Src1[0]
            movd mm1, [esi + 4]         // Src1[1]
            movd mm2, [edx]             // Src2[0]
            movd mm3, [edx + 4]         // Src2[1]
        }

        setup_bilinear_colors();

        __asm
        {
L1:
            dec ecx
            jnz do_safe_lines

done_safe_lines:

            mov ecx, [unsafe_lines]

            test ecx, ecx
            jz done_unsafe_lines

do_unsafe_lines:
        }

        bilinear_blend_colors4();

        __asm
        {
            movd [edi], mm4
            add edi, 4

            add ebx, [r_um]

            cmp ebx, 0x4000             // if(u > 0x4000)
            jb L2

            mov eax, ebx
            shr eax, 14                 // u >> 14

            and ebx, 0x3fff             // u & (0x4000 - 1)

            lea esi, [esi + eax * 4]
            lea edx, [edx + eax * 4]

            mov eax, 4

            cmp esi, [Src1Max]
            jb DoneCheck
            mov esi, [Src1Max]
            mov edx, [Src2Max]
            xor eax, eax
DoneCheck:

            movd mm0, [esi]             // Src1[0]
            movd mm1, [esi + eax]       // Src1[1]
            movd mm2, [edx]             // Src2[0]
            movd mm3, [edx + eax]       // Src2[1]
        }

        setup_bilinear_colors();

        __asm
        {
L2:
            dec ecx
            jnz do_unsafe_lines

done_unsafe_lines:

            pop ebx
        }

        r_v += r_vm;
        if(r_v > 0x4000)
        {
            Src1 += SrcPitch * (r_v >> 14);
            r_v &= (0x4000 - 1);

            if(Src1 >= SrcMax)
            {
                Src1 = SrcMax - SrcPitch;
                Src2 = Src1;
            }
            else
            {
                Src2 = Src1 + SrcPitch;

                if(Src2 >= SrcMax)
                    Src2 = Src1;
            }

            Src1Max = Src1 + sw - 1;
            Src2Max = Src2 + sw - 1;
        }

        __asm
        {
            movd mm7, [r_v]
            punpcklwd mm7, mm7
            punpckldq mm7, mm7          // v| v| v| v
        }

        Dest += DestPitch;
    }

    __asm emms;
}

void StretchBlitBiLinear(dword *dst, dword dstStride,
                         dword dstX, dword dstY,
                         dword dstWidth, dword dstHeight,
                         dword *src, dword srcStride, 
                         dword srcWidth, dword srcHeight)
{
  BB_BilinearStretchBlitARGB8888(dst, dstX, dstY, dstWidth, dstHeight, dstStride * 4,
                                 src, 0, 0, srcWidth, srcHeight, srcStride * 4);
}

#endif
