#include "SoftwareRenderer.h"

#pragma warning(disable : 4799) // function 'x' has no EMMS instruction

#if defined(USE_MMX_ASM)

dword BlendNormal1_MMX(dword src, dword dst)
{
  // Arg1 * alpha + Arg2 * (1 - alpha)
  dword result;
  __asm
  {
    pxor      mm0, mm0          // zero
    
    // Unpack source pixel to a word per component.
    movd      mm1, src
    punpcklbw mm1, mm0

    // Unpack destination pixel to a word per component.
    movd      mm2, dst
    punpcklbw mm2, mm0

    // Extract the alpha value in the source ARGB pixel and expand it into all four channels.
    // src_alpha  = src & 0xff000000
    // src_alpha |= src_alpha >> 8
    // src_alpha |= src_alpha >> 16
    mov       eax, src
    and       eax, 0xff000000   // mask off RGB so we only have the alpha. eax = AA000000
    mov       ebx, eax
    shr       ebx, 8            // ebx = 00AA0000
    or        eax, ebx          // eax = AAAA0000
    mov       ebx, eax
    shr       ebx, 16           // ebx = 0000AAAA
    or        eax, ebx          // eax = AAAAAAAA

    movd      mm3, eax          // mm3 = source alpha

    // Unpack one minus source alpha in all components to words.
    pcmpeqw   mm4, mm4          // mm4 = ff ff ff ff ff ff ff ff
    psubusb   mm4, mm3          // mm4 = 1 - source alpha 

    // Unpack the source alpha and (1 - source alpha) to words.
    punpcklbw mm3, mm0
    punpcklbw mm4, mm0

    // mm1 = source pixel * source alpha
    pmullw    mm1, mm3
    psrlw     mm1, 8

    // mm2 = destination pixel * (1 - source alpha)
    pmullw    mm2, mm4
    psrlw     mm2, 8

    // Add the source and destination together.
    paddusw   mm1, mm2

    // Pack it again.
    packuswb  mm1, mm0

    movd      result, mm1
  }
  return result;
}

dword BlendMultiply1_MMX(dword src, dword dst)
{
  // Arg1 * Arg2
  dword result;
  __asm
  {
    movd        mm0, src
    movd        mm1, dst

    pxor        mm7, mm7    // zero

    punpcklbw   mm0, mm7    // expand to words per component
    punpcklbw   mm1, mm7

    pmullw      mm0, mm1    // multiply per (word) component.
    psrlw       mm0, 8      // shift down to the lower 8 bits in each word.

    packuswb    mm0, mm7    // pack it again

    movd        result, mm0
  }
  return result;
}

dword BlendAdditive1_MMX(dword src, dword dst)
{
  // Arg1 + Arg2
  dword result;
  __asm
  {
    movd    mm0, src
    movd    mm1, dst

    paddusb mm0, mm1

    movd    result, mm0
  }
  return result;
}

dword BlendSubtractive1_MMX(dword src, dword dst)
{
  // Arg1 - Arg2
  dword result;
  __asm
  {
    movd    mm0, dst
    movd    mm1, src

    psubusb mm0, mm1

    movd    result, mm0
  }
  return result;
}

dword BlendScreen1_MMX(dword src, dword dst)
{
  // Arg1 + Arg2 - Arg1 * Arg2
  dword result;
  __asm
  {
    // mm0 = zero
    pxor      mm0, mm0

    // Get the src and dst pixels and expand them to words to avoid saturation when adding.
    movd      mm1, src
    punpcklbw mm1, mm0
    movd      mm2, dst
    punpcklbw mm2, mm0

    // mm3 = src + dst
    movq      mm3, mm1
    paddusw   mm3, mm2

    // Multiply them together and shift the result back into the lower bytes.
    pmullw    mm1, mm2
    psrlw     mm1, 8

    // mm3 = (src + dst) - (src * dst)
    psubusb   mm3, mm1

    // Pack them back into bytes.
    packuswb  mm3, mm0

    movd      result, mm3
  }
  return result;
}


extern "C" {
void BlendNormal_MMX(dword *src, dword *dst, dword num);
void BlendMultiply_MMX(dword *src, dword *dst, dword num);
void BlendAdditive_MMX(dword *src, dword *dst, dword num);
void BlendSubtractive_MMX(dword *src, dword *dst, dword num);
}

#if 0
void BlendNormal_MMX(dword *src, dword *dst, dword num)
{
  __asm
  {
    // Get the arguments into registers.
    mov         esi, src
    mov         edi, dst
    mov         ecx, num

    // We'll work in 64 bit so we need to special case the last pixel if it's odd.
    mov         edx, ecx
    and         edx, 1

    // num /= 2
    shr         ecx, 1

    pxor        mm7, mm7    // mm7 = zero

    // The inner loop that does the multiply, 2 pixels at a time.
  L0:
    // Unpack source pixel to a word per component.
    movq        mm1, [esi]        // mm1 = [src0][src1]
    movq        mm5, mm1          // mm5 = [src0][src1] // Take a copy since we'll have to have the two pixels separately.

    // Unpack destination pixel to a word per component.
    movq        mm2, [edi]        // mm2 = [dst0][dst1]
    movq        mm6, mm2          // mm6 = [dst0][dst1] // Take a copy since we'll have to handle the two pixel separately.

    // -- Handle the first pixel here --

    // Extract the alpha value in the source ARGB pixel and expand it into all four channels.
    // src_alpha  = src & 0xff000000
    // src_alpha |= src_alpha >> 8
    // src_alpha |= src_alpha >> 16
    movd        eax, mm1
    and         eax, 0xff000000   // mask off RGB so we only have the alpha. eax = AA000000
    mov         ebx, eax
    shr         ebx, 8            // ebx = 00AA0000
    or          eax, ebx          // eax = AAAA0000
    mov         ebx, eax
    shr         ebx, 16           // ebx = 0000AAAA
    or          eax, ebx          // eax = AAAAAAAA

    movd        mm3, eax          // mm3 = source alpha

    // Unpack the pixels.
    punpcklbw   mm1, mm7          // mm1 = [src0], unpack lower 32 bits to words
    punpcklbw   mm2, mm7          // mm2 = [dst0], unpack lower 32 bits to words.

    // Unpack one minus source alpha in all components to words.
    pcmpeqw     mm4, mm4          // mm4 = ff ff ff ff ff ff ff ff
    psubusb     mm4, mm3          // mm4 = 1 - source alpha 

    // Unpack the source alpha and (1 - source alpha) to words.
    punpcklbw   mm3, mm7
    punpcklbw   mm4, mm7

    // mm1 = source pixel * source alpha
    pmullw      mm1, mm3
    psrlw       mm1, 8

    // mm2 = destination pixel * (1 - source alpha)
    pmullw      mm2, mm4
    psrlw       mm2, 8

    // Add the source and destination together.
    paddusw     mm1, mm2          // mm1 now holds the result of src0 * src0.a + dst0 * (1 - src0.a)

    // -- Handle the second pixel here --

    movq        mm2, mm5          // mm2 = [src0][src1]
    psrlq       mm2, 32           // only work with the higher 32 bits, mm2 = [0000][src1]

    // Expand the alpha into all four channels.
    // src_alpha  = src & 0xff000000
    // src_alpha |= src_alpha >> 8
    // src_alpha |= src_alpha >> 16
    movd        eax, mm2
    and         eax, 0xff000000   // mask off RGB so we only have the alpha. eax = AA000000
    mov         ebx, eax
    shr         ebx, 8            // ebx = 00AA0000
    or          eax, ebx          // eax = AAAA0000
    mov         ebx, eax
    shr         ebx, 16           // ebx = 0000AAAA
    or          eax, ebx          // eax = AAAAAAAA

    movd        mm3, eax          // mm3 = source alpha

    // Unpack one minus source alpha in all components to words.
    pcmpeqw     mm4, mm4          // mm4 = ff ff ff ff ff ff ff ff
    psubusb     mm4, mm3          // mm4 = 1 - source alpha 

    // Unpack the source alpha and (1 - source alpha) to words.
    punpcklbw   mm3, mm7
    punpcklbw   mm4, mm7

    punpckhbw   mm5, mm7          // mm5 = [src1], unpack higher 32 bits to words.
    punpckhbw   mm6, mm7          // mm6 = [dst1], unpack higher 32 bits to words.

    // mm5 = source pixel * source alpha
    pmullw      mm5, mm3
    psrlw       mm5, 8

    // mm6 = destination pixel * (1 - source alpha)
    pmullw      mm6, mm4
    psrlw       mm6, 8

    // Add the source and destination together.
    paddusw     mm5, mm6

    // Pack the two pixels together again.
    packuswb    mm1, mm5

    movq        [edi], mm1  // Copy the 64 bit result back into memory.

    add         edi, 8
    add         esi, 8

    dec         ecx
    jnz         L0

    // Handle the extra odd pixel if there was one.
    cmp         edx, 0
    jz          Done

    // Unpack source pixel to a word per component.
    movd        mm1, [esi]
    punpcklbw   mm1, mm7

    // Unpack destination pixel to a word per component.
    movd        mm2, [edi]
    punpcklbw   mm2, mm7

    // Extract the alpha value in the source ARGB pixel.
    mov         eax, [esi]
    and         eax, 0xff000000   // mask off RGB so we only have the alpha. eax = AA000000

    // Expand the alpha into all four channels.
    // src_alpha  = src & 0xff000000
    // src_alpha |= src_alpha >> 8
    // src_alpha |= src_alpha >> 16
    mov         ebx, eax
    shr         ebx, 8            // ebx = 00AA0000
    or          eax, ebx          // eax = AAAA0000
    mov         ebx, eax
    shr         ebx, 16           // ebx = 0000AAAA
    or          eax, ebx          // eax = AAAAAAAA

    movd        mm3, eax          // mm3 = source alpha

    // Unpack one minus source alpha in all components to words.
    pcmpeqw     mm4, mm4          // mm4 = ff ff ff ff ff ff ff ff
    psubusb     mm4, mm3          // mm4 = 1 - source alpha 

    // Unpack the source alpha and (1 - source alpha) to words.
    punpcklbw   mm3, mm7
    punpcklbw   mm4, mm7

    // mm1 = source pixel * source alpha
    pmullw      mm1, mm3
    psrlw       mm1, 8

    // mm2 = destination pixel * (1 - source alpha)
    pmullw      mm2, mm4
    psrlw       mm2, 8

    // Add the source and destination together.
    paddusw     mm1, mm2

    // Pack it again.
    packuswb    mm1, mm7

    movd        [edi], mm1

  Done:
  }
}

void BlendMultiply_MMX(dword *src, dword *dst, dword num)
{
  __asm
  {
    // Get the arguments into registers.
    mov         esi, src
    mov         edi, dst
    mov         ecx, num

    // We'll work in 64 bit so we need to special case the last pixel if it's odd.
    mov         edx, ecx
    and         edx, 1

    // num /= 2
    shr         ecx, 1

    pxor        mm7, mm7    // mm7 = zero

    // The inner loop that does satured adds on bytes, 2 pixels at a time.
  L0:
    movq        mm0, [edi]  // Copy the 64 bit destination data into register 0.
    movq        mm1, [esi]  // Copy the 64 bit source data into register 1.

    movq        mm2, mm0    // mm2 = [dst0][dst1]
    punpcklbw   mm2, mm7    // mm2 = dst0 as words.
    movq        mm3, mm0    // mm3 = [dst0][dst1]
    punpckhbw   mm3, mm7    // mm3 = dst1 as words.

    movq        mm4, mm1    // mm4 = [src0][src1]
    punpcklbw   mm4, mm7    // mm4 = src0 as words
    movq        mm5, mm1    // mm5 = [src0][src1]
    punpckhbw   mm5, mm7    // mm5 = src1 as words

    pmullw      mm2, mm4    // mm2 = dst0 * src0
    psrlw       mm2, 8      // shifted down to the lower 8 bits in each word.

    pmullw      mm3, mm5    // mm3 = dst1 * src1
    psrlw       mm3, 8      // shifted down to the lower 8 bits in each word.

    packuswb    mm2, mm3    // pack it again, mm2 = [src0 * dst0] [src1 * dst1]

    movq        [edi], mm2  // Copy the 64 bit result back into memory.

    add         edi, 8
    add         esi, 8

    dec         ecx
    jnz         L0

    // Handle the extra odd pixel if there was one.
    cmp         edx, 0
    jz          Done

    movd        mm0, [edi]
    movd        mm1, [esi]

    punpcklbw   mm0, mm7
    punpcklbw   mm1, mm7

    pmullw      mm0, mm1
    psrlw       mm0, 8

    packuswb    mm0, mm7

    movd        [edi], mm0

  Done:
  }
}

void BlendAdditive_MMX(dword *src, dword *dst, dword num)
{
  __asm
  {
    // Get the arguments into registers.
    mov         esi, src
    mov         edi, dst
    mov         ecx, num

    // We'll work in 64 bit so we need to special case the last pixel if it's odd.
    mov         edx, ecx
    and         edx, 1

    // num /= 2
    shr         ecx, 1

    // The inner loop that does satured adds on bytes, 2 pixels at a time.
  L0:
    movq        mm0, [edi]  // Copy the 64 bit destination data into register 0.
    movq        mm1, [esi]  // Copy the 64 bit source data into register 1.

    paddusb     mm0, mm1    // Do saturated addition on each unsigned byte.

    movq        [edi], mm0  // Copy the 64 bit result back into memory.

    add         edi, 8
    add         esi, 8

    dec         ecx
    jnz         L0

    // Handle the extra odd pixel if there was one.
    cmp         edx, 0
    jz          Done

    movd        mm0, [edi]
    movd        mm1, [esi]

    paddusb     mm0, mm1

    movd        [edi], mm0

  Done:
  }
}

void BlendSubtractive_MMX(dword *src, dword *dst, dword num)
{
  __asm
  {
    // Get the arguments into registers.
    mov         esi, src
    mov         edi, dst
    mov         ecx, num

    // We'll work in 64 bit so we need to special case the last pixel if it's odd.
    mov         edx, ecx
    and         edx, 1

    // num /= 2
    shr         ecx, 1

    // The inner loop that does satured adds on bytes, 2 pixels at a time.
  L0:
    movq        mm0, [edi]  // Copy the 64 bit destination data into register 0.
    movq        mm1, [esi]  // Copy the 64 bit source data into register 1.

    psubusb     mm0, mm1    // Do saturated subtraction on each unsigned byte.

    movq        [edi], mm0  // Copy the 64 bit result back into memory.

    add         edi, 8
    add         esi, 8

    dec         ecx
    jnz         L0

    // Handle the extra odd pixel if there was one.
    cmp         edx, 0
    jz          Done

    movd        mm0, [edi]
    movd        mm1, [esi]

    psubusb     mm0, mm1

    movd        [edi], mm0

  Done:
  }
}
#endif 0

#endif
