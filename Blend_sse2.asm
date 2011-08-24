; Exported functions
global _BlendOver_SSE2

; Constant data.
section .data
align 16
constant_256_words  times 8  dw 256

; Code
section .text

; #define R_SHUFFLE_D( x, y, z, w ) (( (w) & 3 ) << 6 | ( (z) & 3 ) << 4 | ( (y) & 3 ) << 2 | ( (x) & 3 ))


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

; Perform composite over with premultiplied alpha.
; xmm0 should hold zeros.
; xmm1 should hold up to 4 pixels of source pixels.
; xmm2 should hold up to 4 pixels of destination pixels.
; The result will be left in xmm1.
; The following registers will be clobbered: xmm3-5
%macro BlendOverOp 0
  ; Unpack into two registers with words per component. This expansion is
  ; necessary to perform the multiplication without loosing precision.
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
%endmacro


; void BlendOver_SSE2(dword *src, dword *dst, dword num)
%define src   ebp + 8
%define dst   ebp + 12
%define num   ebp + 16
align 16
_BlendOver_SSE2:
  Prologue

  ; Get the arguments into registers.
  mov         esi, [src]
  mov         edi, [dst]
  mov         ecx, [num]

  ; We'll need a register with zeros when unpacking.
  pxor        xmm0, xmm0        ; xmm0 = zero

  ; We'll work in 128 bit so we need to handle the special case if it isn't aligned.
  mov         edx, ecx
  and         edx, 3            ; edx = number of left over pixels to handle.
  shr         ecx, 2            ; ecx = num / 4
  jz BlendOver_SSE2_HandleLeftOvers

  ; The inner loop that does the blend operation, 4 pixels at a time.
;align16 - ToDo: not sure if it's worth aligning the mainloop as well?
.loop0
  ; Grab the source pixels.
  ; Unfortunately we can't guarantee that the source or destination address are
  ; properly aligned, so we'll have to use the slower reads here.
  movdqu      xmm1, [esi]       ; xmm1 = [src0][src1][src2][src3]

  ; Grab the destination pixels.
  movdqu      xmm2, [edi]       ; xmm2 = [dst0][dst1][dst2][dst3]

  ; Blend up to four pixels.
  BlendOverOp

  ; Copy the result back into memory, unaligned.
  movdqu      [edi], xmm1

  ; Move on to the next 4 pixels.
  add         esi, 16
  add         edi, 16

  dec         ecx
  jnz         .loop0

BlendOver_SSE2_HandleLeftOvers:
  ; Handle the extra odd pixels if there are any.
  cmp         edx, 0
  jz          BlendOver_SSE2_Done

  ; We have 1-3 pixels left to do, manually fill the source and destination
  ; registers and then do the blend operation on all 4 pixels even if it only
  ; happens to be one.
  mov         ecx, edx        ; Preserve the count to use it when storing the result.
  mov         eax, edi        ; Preserve the destination as well.
.loop1
  ; Copy pixel data from memory.
  movd        xmm3, [esi]
  movd        xmm4, [edi]

  ; Shift previous pixel up a step if there was one.
  ; Note that these shift operations work on whole bytes, not bits.
  pslldq      xmm1, 4
  pslldq      xmm2, 4
  ; Merge the result from previous pixels.
  por         xmm1, xmm3
  por         xmm2, xmm4

  ; Move to the next pixel.
  add         esi, 4
  add         edi, 4

  dec         ecx
  jnz         .loop1

  ; Blend up to four pixels.
  BlendOverOp

  mov         ecx, edx
.loop2
  ; Move to the previous pixel.
  sub         edi, 4

  ; Copy pixel data to memory.
  movd        [edi], xmm1

  ; Shift previous pixel down a step if there was one.
  psrldq      xmm1, 4

  dec         ecx
  jnz         .loop2

BlendOver_SSE2_Done:
  Epilogue
