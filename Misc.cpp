#include "SoftwareRenderer.h"
#include <memory.h>

#ifdef USE_INLINE_ASM

void MemCpy(void *dst, const void *src, size_t count)
{
  __asm
  {
    // Get the arguments into registers.
    mov   edi, dst
    mov   esi, src
    mov   ecx, count

    // We're copying the data in 64 byte chunks.

    // Store the count so we can handle the left over bytes.
    mov   eax, ecx
    and   eax, 63
    
    // We want to loop count / 64 times.
    shr   ecx, 6

L_copy_64_bytes:
    // Fill up the mmx registers with source data.
    // This introduces the cycle latency we need before accessing the registers
    // and copying them back to the destination.
    movq  mm0, [esi + 8 * 0]
    movq  mm1, [esi + 8 * 1]
    movq  mm2, [esi + 8 * 2]
    movq  mm3, [esi + 8 * 3]
    movq  mm4, [esi + 8 * 4]
    movq  mm5, [esi + 8 * 5]
    movq  mm6, [esi + 8 * 6]
    movq  mm7, [esi + 8 * 7]

    movq  [edi + 8 * 0], mm0
    movq  [edi + 8 * 1], mm1
    movq  [edi + 8 * 2], mm2
    movq  [edi + 8 * 3], mm3
    movq  [edi + 8 * 4], mm4
    movq  [edi + 8 * 5], mm5
    movq  [edi + 8 * 6], mm6
    movq  [edi + 8 * 7], mm7

    // Increment the pointers.
    add   esi, 64
    add   edi, 64

    // Check if we're done yet.
    dec   ecx
    jnz   L_copy_64_bytes

    // Leave mmx mode to restore floating point functionality.
    emms

    // Take care of the left over bytes if there are any.
    mov   ecx, eax
    rep   movsb
  }
}

void MemSet32(void *dst, dword src, size_t count)
{
  // Duplicate the src into a 64 bit struct.
  dword qw_src[2] = { src, src };

  __asm
  {
    mov   edi, dst
    movq  mm0, [qw_src]
    mov   ecx, count

    // We unroll the loop.
    shr   ecx, 1 + 3

L_set_loop:
    movq  [edi + 8 * 0], mm0
    movq  [edi + 8 * 1], mm0
    movq  [edi + 8 * 2], mm0
    movq  [edi + 8 * 3], mm0
    movq  [edi + 8 * 4], mm0
    movq  [edi + 8 * 5], mm0
    movq  [edi + 8 * 6], mm0
    movq  [edi + 8 * 7], mm0
    movq  [edi + 8 * 8], mm0

    add   edi, 64

    dec   ecx
    jnz   L_set_loop

    emms
  }
}

#endif // USE_INLINE_ASM
