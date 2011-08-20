#include "SoftwareRenderer.h"

#if 1

void CrossFade(dword *dst, dword stride,
               dword *srcA, dword *srcB,
               dword width, dword height, dword alpha)
{
  byte invAlpha  = (byte)(255 - alpha);

  for (dword y = 0; y < height; ++y)
  {
    for (dword x = 0; x < width; ++x)
    {
      dword offset = x + y * height;

      byte img0_r, img0_g, img0_b;
      Color(srcA[offset], img0_r, img0_g, img0_b);

      byte img1_r, img1_g, img1_b;
      Color(srcB[offset], img1_r, img1_g, img1_b);

      byte r = (byte)(((dword)img0_r * invAlpha + (dword)img1_r * alpha) >> 8);
      byte g = (byte)(((dword)img0_g * invAlpha + (dword)img1_g * alpha) >> 8);
      byte b = (byte)(((dword)img0_b * invAlpha + (dword)img1_b * alpha) >> 8);

      dst[x + y * stride] = Color(r, g, b);
    }
  }
}

#elif 1

void CrossFade(dword *dst, dword stride,
               dword *srcA, dword *srcB,
               dword width, dword height, dword alpha)
{
  dword invAlpha = 255 - alpha;

  alpha |= alpha << 8;
  alpha |= alpha << 16;

  invAlpha |= invAlpha << 8;
  invAlpha |= invAlpha << 16;

  __asm
  {
    pxor      mm0, mm0    // zero

    movd      mm3, alpha
    punpcklbw mm3, mm0

    movd      mm4, invAlpha
    punpcklbw mm4, mm0
  }

  dword yOffset = 0;
  for (dword y = 0; y < height; ++y)
  {
    for (dword x = 0; x < width; ++x)
    {
      dword offset = x + yOffset;

      dword color0 = srcA[offset];
      dword color1 = srcB[offset];

      dword result;
      __asm
      {
        // Move the variables into registers and unpack them to words.
        movd      mm1, color0
        punpcklbw mm1, mm0

        // mm1 = color0 * alpha
        pmullw    mm1, mm3

        movd      mm2, color1
        punpcklbw mm2, mm0

        // mm2 = color1 * invAlpha
        pmullw    mm2, mm4

        // mm1 = color0 * alpha + color1 + invAlpha
        paddusw   mm1, mm2

        // shift the result back into bytes.
        psrlw     mm1, 8
        // Pack the result back to word
        packuswb  mm1, mm0

        movd      result, mm1
      }

      dst[x] = result;
    }

    yOffset += width;
    dst += stride;
  }

  __asm emms
}

#elif 0

// ToDo: need to figure out exactly how it works:
// - send in easier test data (images & controlled alpha values).
// - go through each instruction used and read up in the intel reference manual.

void CrossFade(dword *dst, dword stride,
               dword *srcA, dword *srcB,
               dword width, dword height, dword alpha)
{
  for (dword y = 0; y < height; ++y)
  {
    for (dword x = 0; x < width; ++x)
    {
      dword *srcA_offset = srcA + x + y * width;
      dword *srcB_offset = srcB + x + y * width;

      dword *dst_offset = dst + x + y * stride;

      __asm
      {
        mov       edi, srcA_offset
        mov       esi, srcB_offset
        mov       edx, dst_offset

        pxor      mm6, mm6		        // make MM6==0
        mov       eax, alpha
        mov       ebx, eax
        shl       ebx, 16
        add       eax, ebx
        movd      mm7, eax
        movq      mm6, mm7
        punpckldq mm7, mm6	          // MM7=alpha

        //////////inner LOOP/////////////

        movq      mm0, [esi]  	      // pixels a
        movq      mm1, [edi]		      // pixels b
        punpcklbw mm0, mm6	          // byte-->word(pixel 1)
        punpcklbw mm1, mm6	          // byte-->word(pixel 1)
        psubw     mm0, mm1		        // a-b
        pmullw    mm0, mm7		        // pixel 1 * alpha
        psrlw     mm0, 8		          // shifts word elements 8 to the right.
        paddb     mm0, mm1		        // add (b) to result
        packuswb  mm0, mm6    	      // convert back into byte form
        movq      [edx], mm0
        
        //////////inner LOOP/////////////
      }
    }
  }

  __asm { emms }
}

#endif
