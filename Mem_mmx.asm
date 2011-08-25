; Exported functions
global _MemCpy_MMX
global _MemSet32_MMX


; Code
section .text

; Function helper macros
%macro Prologue 0
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  ; Save away all registers.
  pushad
%endmacro

%macro Epilogue 0
  ; Restore all clobbered registers.
  popad

  ; Restore stack frame and return to callee.
  pop         ebp
  ret
%endmacro

; void MemCpy_MMX(void *dst, const void *src, size_t count);
%define dst   ebp + 8
%define src   ebp + 12
%define count ebp + 16
align 16
_MemCpy_MMX:
  Prologue

  ; Get the arguments into registers.
  mov         edi, [dst]
  mov         esi, [src]
  mov         ecx, [count]

  ; We're copying the data in 64 byte chunks.

  ; Store the count so we can handle the left over bytes.
  mov         eax, ecx
  and         eax, 63
    
  ; We want to loop count / 64 times.
  shr         ecx, 6

.loop:
  ; Fill up the mmx registers with source data.
  ; This introduces the cycle latency we need before accessing the registers
  ; and copying them back to the destination.
  movq        mm0, [esi + 8 * 0]
  movq        mm1, [esi + 8 * 1]
  movq        mm2, [esi + 8 * 2]
  movq        mm3, [esi + 8 * 3]
  movq        mm4, [esi + 8 * 4]
  movq        mm5, [esi + 8 * 5]
  movq        mm6, [esi + 8 * 6]
  movq        mm7, [esi + 8 * 7]

  movq        [edi + 8 * 0], mm0
  movq        [edi + 8 * 1], mm1
  movq        [edi + 8 * 2], mm2
  movq        [edi + 8 * 3], mm3
  movq        [edi + 8 * 4], mm4
  movq        [edi + 8 * 5], mm5
  movq        [edi + 8 * 6], mm6
  movq        [edi + 8 * 7], mm7

  ; Increment the pointers.
  add         esi, 64
  add         edi, 64

  ; Check if we're done yet.
  dec         ecx
  jnz         .loop

  ; Leave mmx mode to restore floating point functionality.
  emms

  ; Take care of the left over bytes if there are any.
  mov         ecx, eax
  rep         movsb

  Epilogue

; void MemSet32_MMX(void *dst, dword src, size_t count);
%define dst   ebp + 8
%define src   ebp + 12
%define count ebp + 16
align 16
_MemSet32_MMX:
  Prologue

  ; ToDo: check count, and handle left overs.

  mov         edi, [dst]
  mov         esi, [src]
  mov         ecx, [count]

  ; Duplicate the src into a 64 bit struct.
  ; dword qw_src[2] = { src, src };
  movd        mm1, esi
  psllq       mm1, 32
  movd        mm0, esi
  por         mm0, mm1

  ; We deal with 64 bit values and we unroll the loop 8 times.
  shr         ecx, 1 + 3

.loop:
  movq        [edi + 8 * 0], mm0
  movq        [edi + 8 * 1], mm0
  movq        [edi + 8 * 2], mm0
  movq        [edi + 8 * 3], mm0
  movq        [edi + 8 * 4], mm0
  movq        [edi + 8 * 5], mm0
  movq        [edi + 8 * 6], mm0
  movq        [edi + 8 * 7], mm0
  movq        [edi + 8 * 8], mm0

  add         edi, 64

  dec         ecx
  jnz         .loop

  emms

  Epilogue
