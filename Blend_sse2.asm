; Exported functions
global _BlendOver_SSE2

; Constant data.
section .data
align 16
constant_256_words  times 8  dw 256

; Code
section .text

; Function helper macros
%macro prologue 0
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  ; Save away all registers.
  pushad
%endmacro

%macro epilogue 0
  ; Restore all clobbered registers.
  popad

  ; Restore stack frame and return to callee.
  pop         ebp
  ret
%endmacro

; #define R_SHUFFLE_D( x, y, z, w ) (( (w) & 3 ) << 6 | ( (z) & 3 ) << 4 | ( (y) & 3 ) << 2 | ( (x) & 3 ))

; void BlendOver_SSE2(dword *src, dword *dst, dword num)
%define src   ebp + 8
%define dst   ebp + 12
%define num   ebp + 16
align 16
_BlendOver_SSE2:
  prologue

  ; Get the arguments into registers.
  mov         esi, [src]
  mov         edi, [dst]
  mov         ecx, [num]

  ; We'll work in 128 bit so we need to handle the special case if it isn't aligned.
  mov         edx, ecx
  and         edx, 3            ; edx = number of left over pixels to handle.
  shr         ecx, 2            ; ecx = num / 4
  jz BlendOver_SSE2_HandleLeftOvers

  pxor        xmm0, xmm0        ; xmm0 = zero

  ; The inner loop that does the blend operation, 4 pixels at a time.
.loop
  ; Grab the source pixels.
  ; Unfortunately we can't guarantee that the source or destination address are
  ; properly aligned, so we'll have to use the slower reads here.
  movdqu      xmm1, [esi]       ; xmm1 = [src0][src1][src2][src3]

  ; Grab the destination pixels and unpack into two registers with words per
  ; component. This expansion is necessary to perform the multiplication
  ; without loosing precision.
  movdqu      xmm2, [edi]       ; xmm2 = [dst0][dst1][dst2][dst3]
  movdqa      xmm3, xmm2        ; xmm3 = [dst0][dst1][dst2][dst3]
  punpcklbw   xmm2, xmm0        ; xmm2 = [dst0][dst1]
  punpckhbw   xmm3, xmm0        ; xmm3 = [dst2][dst3]

  ; Expand the two lower source pixels to words.
  movdqa      xmm4, xmm1
  punpcklbw   xmm4, xmm0
  ; Broadcast the alpha channel to all the words.
  pshuflw     xmm5, xmm4, 11111111b
  pshufhw     xmm4, xmm5, 11111111b
  ; Subtract from 256.
  movdqa      xmm5, [constant_256_words]
  psubusw     xmm5, xmm4        ; xmm5 = [256-srca][256-srca]

  ; Multiply [dst0][dst1] with one minus src alpha.
  ; Then shift down the result to bytes again.
  pmullw      xmm2, xmm5
  psrlw       xmm2, 8           ; xmm2 = [dst0*(256-src0.a)][dst1*(256-src1.a)]

  ; Expand the two higher source pixels to words.
  movdqa      xmm4, xmm1
  punpckhbw   xmm4, xmm0
  ; Broadcast the alpha channel to all the words.
  pshuflw     xmm5, xmm4, 11111111b
  pshufhw     xmm4, xmm5, 11111111b
  ; Subtract from 256.
  movdqa      xmm5, [constant_256_words]
  psubusw     xmm5, xmm4        ; xmm5 = [256-srca][256-srca]

  ; Multiply [dst2][dst3] with one minus src alpha.
  ; Then shift down the result to bytes again.
  pmullw      xmm3, xmm5
  psrlw       xmm3, 8           ; xmm3 = [dst2*(256-src2.a)][dst3*(256-src3.a)]

  ; Pack the destination pixels back into a 128 bit register again.
  packuswb    xmm2, xmm3        ; xmm2 = [dst0*(256-src0.a)][dst1*(256-src1.a)][dst2*(256-src2.a)][dst3*(256-src3.a)]

  ; Add with the source pixels.
  paddusb     xmm1, xmm2        ; xmm1 = src + dst * src.a

  ; Copy the result back into memory, unaligned.
  movdqu      [edi], xmm1

  ; Move on to the next 4 pixels.
  add         edi, 16
  add         esi, 16

  dec         ecx
  jnz         .loop

  ; Handle the extra odd pixel if there was one.
BlendOver_SSE2_HandleLeftOvers:
  cmp         edx, 0
  jz          BlendOver_SSE2_Done

;  !!!ToDo!!!

BlendOver_SSE2_Done:
  epilogue
