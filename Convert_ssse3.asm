; Exported functions
global _ConvertRGBAtoBGRA_SSSE3

; Initialized variables.
section .data
align 16
rgba_shuffle_mask  db   0, 3, 2, 1,   4, 7, 6, 5,   8, 11, 10, 9,   12, 15, 14, 13

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

; void ConvertRGBAtoBGRA_SSE3(void *dst, dword src, dword num);
%define dst   ebp + 8
%define src   ebp + 12
%define num   ebp + 16
align 16
_ConvertRGBAtoBGRA_SSSE3:
  Prologue

  ; Get the arguments into registers.
  mov         esi, [src]
  mov         edi, [dst]
  mov         ecx, [num]

  ; We'll work in 128 bit so we need to handle the special case if it isn't aligned.
  mov         edx, ecx
  shr         ecx, 2 + 3        ; ecx = num / 4, then unrolled 8 times.
  jz .handleLeftOvers0

  mov         eax, 0
.loop0
  movdqu      xmm0, [esi + eax + 16 * 0]
  movdqu      xmm1, [esi + eax + 16 * 1]
  movdqu      xmm2, [esi + eax + 16 * 2]
  movdqu      xmm3, [esi + eax + 16 * 3]
  movdqu      xmm4, [esi + eax + 16 * 4]
  movdqu      xmm5, [esi + eax + 16 * 5]
  movdqu      xmm6, [esi + eax + 16 * 6]
  movdqu      xmm7, [esi + eax + 16 * 7]

  pshufb      xmm0, [rgba_shuffle_mask]
  pshufb      xmm1, [rgba_shuffle_mask]
  pshufb      xmm2, [rgba_shuffle_mask]
  pshufb      xmm3, [rgba_shuffle_mask]
  pshufb      xmm4, [rgba_shuffle_mask]
  pshufb      xmm5, [rgba_shuffle_mask]
  pshufb      xmm6, [rgba_shuffle_mask]
  pshufb      xmm7, [rgba_shuffle_mask]

  movdqu      [edi + eax + 16 * 0], xmm0
  movdqu      [edi + eax + 16 * 1], xmm1
  movdqu      [edi + eax + 16 * 2], xmm2
  movdqu      [edi + eax + 16 * 3], xmm3
  movdqu      [edi + eax + 16 * 4], xmm4
  movdqu      [edi + eax + 16 * 5], xmm5
  movdqu      [edi + eax + 16 * 6], xmm6
  movdqu      [edi + eax + 16 * 7], xmm7

  add         eax, 16 * 8

  dec         ecx
  jnz         .loop0

  ; Move the source and destination pointers forward in case any
  ; the two left over cases are hit.
  add         esi, eax
  add         edi, eax

  ; 4-32 pixels left.
.handleLeftOvers0:
  mov         ecx, edx
  and         ecx, 31           ; ecx = number of left over pixels to handle.
  shr         ecx, 2
  jz          .handleLeftOvers1

  mov         eax, 0
.loop1
  movdqu      xmm0, [esi + eax]
  pshufb      xmm0, [rgba_shuffle_mask]
  movdqu      [edi + eax], xmm0

  add         eax, 16

  dec         ecx
  jnz         .loop1

  ; Move the source and destination pointers forward in case any
  ; the two left over cases are hit.
  add         esi, eax
  add         edi, eax

  ; 1-3 pixels left.
.handleLeftOvers1:
  mov         ecx, edx
  and         ecx, 3
  jz          .done

  ; Preserve the count.
  mov         ebx, ecx

.loop2
  ; Copy one pixel from memory.
  movd        xmm1, [esi]

  ; Shift previous pixel up a step if there was one.
  pslldq      xmm0, 4
  ; Merge the result from previous pixels.
  por         xmm0, xmm1
  ; Move to next pixel.
  add         esi, 4

  dec         ecx
  jnz         .loop2

  ; Do the shuffle on 1-3 pixels (and 12-15 garbage pixels).
  pshufb      xmm0, [rgba_shuffle_mask]

  mov         ecx, ebx
.loop3
  ; Copy pixel data to memory.
  movd        [edi + ecx * 4 - 4], xmm0

  ; Shift previous pixel down one step if there was one.
  psrldq      xmm0, 4

  dec         ecx
  jnz         .loop3

.done:
  Epilogue
