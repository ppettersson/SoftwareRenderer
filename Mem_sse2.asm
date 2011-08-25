; Exported functions
global _MemSet32_SSE2


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

; void MemSet32_SSE2(void *dst, dword src, size_t count);
%define dst   ebp + 8
%define src   ebp + 12
%define count ebp + 16
align 16
_MemSet32_SSE2:
  Prologue

  ; ToDo: Fix up alignment
  ;       Unroll.

  mov         ecx, [count]
  mov         edx, ecx

  ; Deal with 128 bit values and unroll 8 times.
  shr         ecx, 2 + 3
  jz          .handleLeftOvers

  ; Duplicate the src into a 128 bit register.
  ; ToDo: This can probably be done faster...
  movd        xmm0, [src]
  movdqa      xmm1, xmm0
  pslldq      xmm0, 4
  por         xmm1, xmm0
  movdqa      xmm0, xmm1
  pslldq      xmm1, 8
  por         xmm0, xmm1

  mov         edi, [dst]

  ; Inner loop, copy four pixels at a time.
  ; ToDo: This is not optimal for small loops, might be worth checking the size and have a few different unrollings.
  ; ToDo: This is using unaligned memory copy, not ideal.
.loop:
  movdqu      [edi + 16 * 0], xmm0
  movdqu      [edi + 16 * 1], xmm0
  movdqu      [edi + 16 * 2], xmm0
  movdqu      [edi + 16 * 3], xmm0
  movdqu      [edi + 16 * 4], xmm0
  movdqu      [edi + 16 * 5], xmm0
  movdqu      [edi + 16 * 6], xmm0
  movdqu      [edi + 16 * 7], xmm0

  add         edi, 16 * 8

  dec         ecx
  jnz         .loop

  ; Handle the 1-3 pixels that are left.
.handleLeftOvers
  mov         ecx, edx
  and         ecx, 3
  jz          .done

  rep         stosd

.done
  Epilogue
