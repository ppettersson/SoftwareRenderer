; Exported functions
global _BlendNormal1_MMX
global _BlendOver1_MMX
global _BlendMultiply1_MMX
global _BlendAdditive1_MMX
global _BlendSubtractive1_MMX
global _BlendScreen1_MMX
;global _BlendLighten1_MMX
;global _BlendDarken1_MMX
global _BlendNormal_MMX
global _BlendOver_MMX
global _BlendMultiply_MMX
global _BlendAdditive_MMX
global _BlendSubtractive_MMX
;global _BlendScreen_MMX
;global _BlendLighten_MMX
;global _BlendDarken_MMX


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

; Calculate 256 - src.alpha and then duplicate this into four words in an mmx register
;
; %1 = dword ptr, the pixel to grab the alpha value from, e.g. "esi"
; The result will be left in mm4
;
; eax, ebx and mm5 will be clobbered.
;
%macro ExpandOneMinusAlpha 1
  ; Get one minus src0.alpha into a word.
  mov         eax, 0x0100       ; 256
  mov         ebx, [%1]
  shr         ebx, 24
  sub         eax, ebx          ; eax = 256 - src0.alpha

  ; Duplicate the word four times into a 64 bit register.
  mov         ebx, eax
  shl         ebx, 16
  or          eax, ebx          ; eax = [256-srca][256-srca]
  movd        mm4, eax
  movq        mm5, mm4
  psllq       mm5, 32
  por         mm4, mm5          ; mm4 = [256-srca][256-srca][256-srca][256-srca]
%endmacro


; Arg1 * alpha + Arg2 * (1 - alpha)
;
; dword BlendNormal1_MMX(dword src, dword dst)
%define src   ebp + 8
%define dst   ebp + 12
align 16
_BlendNormal1_MMX:
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  ; Save away registers.
  push        ebx

  pxor        mm0, mm0          ; zero

  ; Unpack source pixel to a word per component.
  movd        mm1, [src]
  punpcklbw   mm1, mm0

  ; Unpack destination pixel to a word per component.
  movd        mm2, [dst]
  punpcklbw   mm2, mm0

  ; Extract the alpha value in the source ARGB pixel and expand it into all four channels.
  ; src_alpha  = src & 0xff000000
  ; src_alpha |= src_alpha >> 8
  ; src_alpha |= src_alpha >> 16
  mov         eax, [src]
  and         eax, 0xff000000   ; mask off RGB so we only have the alpha. eax = AA000000
  mov         ebx, eax
  shr         ebx, 8            ; ebx = 00AA0000
  or          eax, ebx          ; eax = AAAA0000
  mov         ebx, eax
  shr         ebx, 16           ; ebx = 0000AAAA
  or          eax, ebx          ; eax = AAAAAAAA

  movd        mm3, eax          ; mm3 = source alpha

  ; Unpack one minus source alpha in all components to words.
  pcmpeqw     mm4, mm4          ; mm4 = ff ff ff ff ff ff ff ff
  psubusb     mm4, mm3          ; mm4 = 1 - source alpha

  ; Unpack the source alpha and (1 - source alpha) to words.
  punpcklbw   mm3, mm0
  punpcklbw   mm4, mm0

  ; mm1 = source pixel * source alpha
  pmullw      mm1, mm3
  psrlw       mm1, 8

  ; mm2 = destination pixel * (1 - source alpha)
  pmullw      mm2, mm4
  psrlw       mm2, 8

  ; Add the source and destination together.
  paddusw     mm1, mm2

  ; Pack it again.
  packuswb    mm1, mm0

  ; Store the result in eax.
  movd        eax, mm1

  ; Restore clobbered register.
  pop         ebx

  ; Restore stack frame and return to callee.
  pop         ebp
  ret


; Arg1 + Arg2 * (1 - alpha)
;
; dword BlendOver1_MMX(dword src, dword dst)
%define src   ebp + 8
%define dst   ebp + 12
align 16
_BlendOver1_MMX:
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  ; Save away registers.
  push        ebx

  pxor        mm0, mm0          ; zero

  ; Unpack source pixel to a word per component.
  movd        mm1, [src]
  punpcklbw   mm1, mm0

  ; Unpack destination pixel to a word per component.
  movd        mm2, [dst]
  punpcklbw   mm2, mm0

  ; Extract the alpha value in the source ARGB pixel and expand it into all four channels.
  ; src_alpha  = src & 0xff000000
  ; src_alpha |= src_alpha >> 8
  ; src_alpha |= src_alpha >> 16
  mov         eax, [src]
  and         eax, 0xff000000   ; mask off RGB so we only have the alpha. eax = AA000000
  mov         ebx, eax
  shr         ebx, 8            ; ebx = 00AA0000
  or          eax, ebx          ; eax = AAAA0000
  mov         ebx, eax
  shr         ebx, 16           ; ebx = 0000AAAA
  or          eax, ebx          ; eax = AAAAAAAA

  movd        mm3, eax          ; mm3 = source alpha

  ; Unpack one minus source alpha in all components to words.
  ; ToDo: Ideally we'd use 256 not 255 here.
  pcmpeqw     mm4, mm4          ; mm4 = ff ff ff ff ff ff ff ff
  psubusb     mm4, mm3          ; mm4 = ff - source alpha

  ; Unpack the (1 - source alpha) to words.
  punpcklbw   mm4, mm0

  ; mm2 = destination pixel * (ff - source alpha)
  pmullw      mm2, mm4
  psrlw       mm2, 8

  ; Add the source and destination together.
  paddusw     mm1, mm2

  ; Pack it again.
  packuswb    mm1, mm0

  ; Store the result in eax.
  movd        eax, mm1

  ; Restore clobbered register.
  pop         ebx

  ; Restore stack frame and return to callee.
  pop         ebp
  ret


; Arg1 * Arg2
;
; dword BlendMultiply1_MMX(dword src, dword dst)
%define src   ebp + 8
%define dst   ebp + 12
align 16
_BlendMultiply1_MMX:
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  movd        mm0, [src]
  movd        mm1, [dst]

  pxor        mm7, mm7    ; zero

  punpcklbw   mm0, mm7    ; expand to words per component
  punpcklbw   mm1, mm7

  pmullw      mm0, mm1    ; multiply per (word) component.
  psrlw       mm0, 8      ; shift down to the lower 8 bits in each word.

  packuswb    mm0, mm7    ; pack it again

  ; Store the result in eax.
  movd        eax, mm0

  ; Restore stack frame and return to callee.
  pop         ebp
  ret


; Arg1 + Arg2
;
; dword BlendAdditive1_MMX(dword src, dword dst)
%define src   ebp + 8
%define dst   ebp + 12
align 16
_BlendAdditive1_MMX:
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  ; Load up the source and destination operands.
  movd        mm0, [src]
  movd        mm1, [dst]

  ; Additive blend with saturation.
  paddusb     mm0, mm1

  ; Store the result in eax.
  movd        eax, mm0

  ; Restore stack frame and return to callee.
  pop         ebp
  ret


; Arg1 - Arg2
;
; dword BlendSubtractive1_MMX(dword src, dword dst)
%define src   ebp + 8
%define dst   ebp + 12
align 16
_BlendSubtractive1_MMX:
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  ; Load up the source and destination operands.
  movd        mm0, [src]
  movd        mm1, [dst]

  ; Subtractive blend with saturation.
  psubusb     mm0, mm1

  ; Store the result in eax.
  movd        eax, mm0

  ; Restore stack frame and return to callee.
  pop         ebp
  ret


; Arg1 + Arg2 - Arg1 * Arg2
;
; dword BlendScreen1_MMX(dword src, dword dst)
%define src   ebp + 8
%define dst   ebp + 12
align 16
_BlendScreen1_MMX:
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  ; mm0 = zero
  pxor        mm0, mm0

  ; Get the src and dst pixels and expand them to words to avoid saturation when adding.
  movd        mm1, [src]
  punpcklbw   mm1, mm0
  movd        mm2, [dst]
  punpcklbw   mm2, mm0

  ; mm3 = src + dst
  movq        mm3, mm1
  paddusw     mm3, mm2

  ; Multiply them together and shift the result back into the lower bytes.
  pmullw      mm1, mm2
  psrlw       mm1, 8

  ; mm3 = (src + dst) - (src * dst)
  psubusb     mm3, mm1

  ; Pack them back into bytes.
  packuswb    mm3, mm0

  ; Store the result in eax
  movd        eax, mm3

  ; Restore stack frame and return to callee.
  pop         ebp
  ret

%if 0 ; pcmpgtb is a signed compare, need unsigned compare...
; Max(Arg1, Arg2)
;
; dword BlendLighten1_MMX(dword src, dword dst)
%define src   ebp + 8
%define dst   ebp + 12
align 16
_BlendLighten1_MMX:
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  movd        mm1, [src]  ; mm1 = src
  movd        mm2, [dst]  ; mm2 = dst

  movq        mm3, mm1
  pcmpgtb     mm3, mm2    ; mm3 = ">" mask

  pand        mm1, mm3    ; mm1 = src & mask
  pandn       mm3, mm2    ; mm3 = dst & inverse-mask

  por         mm1, mm3    ; mm1 = (src & mask) | (dst & inverse-mask)

  movd        eax, mm1    ; Store the result.

  ; Restore stack frame and return to callee.
  pop         ebp
  ret


; Min(Arg1, Arg2)
;
; dword BlendDarken1_MMX(dword src, dword dst)
%define src   ebp + 8
%define dst   ebp + 12
align 16
_BlendDarken1_MMX:
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp

  movd        mm1, [src]  ; mm1 = src
  movd        mm2, [dst]  ; mm2 = dst

  movq        mm3, mm1
  pcmpgtb     mm3, mm2    ; mm3 = ">" mask

  pand        mm2, mm3    ; mm2 = dst & mask
  pandn       mm3, mm1    ; mm3 = src & inverse-mask

  por         mm2, mm3    ; (src & mask) | (dst & inverse-mask)

  movd        eax, mm2    ; Store the result.

  ; Restore stack frame and return to callee.
  pop         ebp
  ret
%endif


; void BlendNormal_MMX(dword *src, dword *dst, dword num)
%define src   ebp + 8
%define dst   ebp + 12
%define num   ebp + 16
align 16
_BlendNormal_MMX:
  prologue

  ; Get the arguments into registers.
  mov         esi, [src]
  mov         edi, [dst]
  mov         ecx, [num]

  ; We'll work in 64 bit so we need to special case the last pixel if it's odd.
  mov         edx, ecx
  and         edx, 1

  ; num /= 2
  shr         ecx, 1

  pxor        mm7, mm7          ; mm7 = zero

  ; The inner loop that does the multiply, 2 pixels at a time.
.loop
  ; Unpack source pixel to a word per component.
  movq        mm1, [esi]        ; mm1 = [src0][src1]
  movq        mm5, mm1          ; mm5 = [src0][src1]    Take a copy since we'll have to have the two pixels separately.

  ; Unpack destination pixel to a word per component.
  movq        mm2, [edi]        ; mm2 = [dst0][dst1]
  movq        mm6, mm2          ; mm6 = [dst0][dst1]    Take a copy since we'll have to handle the two pixel separately.

  ; -- Handle the first pixel here --

  ; Extract the alpha value in the source ARGB pixel and expand it into all four channels.
  ; src_alpha  = src & 0xff000000
  ; src_alpha |= src_alpha >> 8
  ; src_alpha |= src_alpha >> 16
  movd        eax, mm1
  and         eax, 0xff000000   ; mask off RGB so we only have the alpha. eax = AA000000
  mov         ebx, eax
  shr         ebx, 8            ; ebx = 00AA0000
  or          eax, ebx          ; eax = AAAA0000
  mov         ebx, eax
  shr         ebx, 16           ; ebx = 0000AAAA
  or          eax, ebx          ; eax = AAAAAAAA

  movd        mm3, eax          ; mm3 = source alpha

  ; Unpack the pixels.
  punpcklbw   mm1, mm7          ; mm1 = [src0], unpack lower 32 bits to words
  punpcklbw   mm2, mm7          ; mm2 = [dst0], unpack lower 32 bits to words.

  ; Unpack one minus source alpha in all components to words.
  pcmpeqw     mm4, mm4          ; mm4 = ff ff ff ff ff ff ff ff
  psubusb     mm4, mm3          ; mm4 = 1 - source alpha 

  ; Unpack the source alpha and (1 - source alpha) to words.
  punpcklbw   mm3, mm7
  punpcklbw   mm4, mm7

  ; mm1 = source pixel * source alpha
  pmullw      mm1, mm3
  psrlw       mm1, 8

  ; mm2 = destination pixel * (1 - source alpha)
  pmullw      mm2, mm4
  psrlw       mm2, 8

  ; Add the source and destination together.
  paddusw     mm1, mm2          ; mm1 now holds the result of src0 * src0.a + dst0 * (1 - src0.a)

  ; -- Handle the second pixel here --

  movq        mm2, mm5          ; mm2 = [src0][src1]
  psrlq       mm2, 32           ; only work with the higher 32 bits, mm2 = [0000][src1]

  ; Expand the alpha into all four channels.
  ; src_alpha  = src & 0xff000000
  ; src_alpha |= src_alpha >> 8
  ; src_alpha |= src_alpha >> 16
  movd        eax, mm2
  and         eax, 0xff000000   ; mask off RGB so we only have the alpha. eax = AA000000
  mov         ebx, eax
  shr         ebx, 8            ; ebx = 00AA0000
  or          eax, ebx          ; eax = AAAA0000
  mov         ebx, eax
  shr         ebx, 16           ; ebx = 0000AAAA
  or          eax, ebx          ; eax = AAAAAAAA

  movd        mm3, eax          ; mm3 = source alpha

  ; Unpack one minus source alpha in all components to words.
  pcmpeqw     mm4, mm4          ; mm4 = ff ff ff ff ff ff ff ff
  psubusb     mm4, mm3          ; mm4 = 1 - source alpha 

  ; Unpack the source alpha and (1 - source alpha) to words.
  punpcklbw   mm3, mm7
  punpcklbw   mm4, mm7

  punpckhbw   mm5, mm7          ; mm5 = [src1], unpack higher 32 bits to words.
  punpckhbw   mm6, mm7          ; mm6 = [dst1], unpack higher 32 bits to words.

  ; mm5 = source pixel * source alpha
  pmullw      mm5, mm3
  psrlw       mm5, 8

  ; mm6 = destination pixel * (1 - source alpha)
  pmullw      mm6, mm4
  psrlw       mm6, 8

  ; Add the source and destination together.
  paddusw     mm5, mm6

  ; Pack the two pixels together again.
  packuswb    mm1, mm5

  movq        [edi], mm1        ; Copy the 64 bit result back into memory.

  add         edi, 8
  add         esi, 8

  dec         ecx
  jnz         .loop

  ; Handle the extra odd pixel if there was one.
  cmp         edx, 0
  jz          BlendNormal_MMX_Done

  ; Unpack source pixel to a word per component.
  movd        mm1, [esi]
  punpcklbw   mm1, mm7

  ; Unpack destination pixel to a word per component.
  movd        mm2, [edi]
  punpcklbw   mm2, mm7

  ; Extract the alpha value in the source ARGB pixel.
  mov         eax, [esi]
  and         eax, 0xff000000   ; mask off RGB so we only have the alpha. eax = AA000000

  ; Expand the alpha into all four channels.
  ; src_alpha  = src & 0xff000000
  ; src_alpha |= src_alpha >> 8
  ; src_alpha |= src_alpha >> 16
  mov         ebx, eax
  shr         ebx, 8            ; ebx = 00AA0000
  or          eax, ebx          ; eax = AAAA0000
  mov         ebx, eax
  shr         ebx, 16           ; ebx = 0000AAAA
  or          eax, ebx          ; eax = AAAAAAAA

  movd        mm3, eax          ; mm3 = source alpha

  ; Unpack one minus source alpha in all components to words.
  pcmpeqw     mm4, mm4          ; mm4 = ff ff ff ff ff ff ff ff
  psubusb     mm4, mm3          ; mm4 = 1 - source alpha 

  ; Unpack the source alpha and (1 - source alpha) to words.
  punpcklbw   mm3, mm7
  punpcklbw   mm4, mm7

  ; mm1 = source pixel * source alpha
  pmullw      mm1, mm3
  psrlw       mm1, 8

  ; mm2 = destination pixel * (1 - source alpha)
  pmullw      mm2, mm4
  psrlw       mm2, 8

  ; Add the source and destination together.
  paddusw     mm1, mm2

  ; Pack it again.
  packuswb    mm1, mm7

  movd        [edi], mm1

BlendNormal_MMX_Done:
  epilogue


; void BlendOver_MMX(dword *src, dword *dst, dword num)
%define src   ebp + 8
%define dst   ebp + 12
%define num   ebp + 16
align 16
_BlendOver_MMX:
  prologue

  ; Get the arguments into registers.
  mov         esi, [src]
  mov         edi, [dst]
  mov         ecx, [num]

  ; We'll work in 64 bit so we need to special case the last pixel if it's odd.
  mov         edx, ecx
  and         edx, 1

  ; num /= 2
  shr         ecx, 1

  pxor        mm0, mm0          ; mm0 = zero

  ; The inner loop that does the multiply, 2 pixels at a time.
.loop
  ; Grab the source pixels.
  movq        mm1, [esi]        ; mm1 = [src0][src1]

  ; Grab the destination pixels and unpack to a word per component.
  movq        mm2, [edi]        ; mm2 = [dst0][dst1]
  movq        mm3, mm2          ; mm3 = [dst0][dst1]
  punpcklbw   mm2, mm0          ; mm2 = [dst0]
  punpckhbw   mm3, mm0          ; mm3 = [dst1]

  ; mm4 = [256-srca][256-srca][256-srca][256-srca]
  ExpandOneMinusAlpha esi

  ; Multiply dst0 with one minus src alpha.
  ; Then shift down the result to bytes again.
  pmullw      mm2, mm4
  psrlw       mm2, 8

  ; mm4 = [256-srca][256-srca][256-srca][256-srca]
  ExpandOneMinusAlpha (esi + 4)

  ; Multiply dst1 with one minus src alpha.
  ; Then shift down the result to bytes again.
  pmullw      mm3, mm4
  psrlw       mm3, 8

  ; Pack dst0 and dst1 back into a 64 bit register.
  packuswb    mm2, mm3

  ; Add with the source pixels.
  paddusb     mm1, mm2

  movq        [edi], mm1        ; Copy the 64 bit result back into memory.

  add         edi, 8
  add         esi, 8

  dec         ecx
  jnz         .loop

  ; Handle the extra odd pixel if there was one.
  cmp         edx, 0
  jz          BlendOver_MMX_Done

  ; Only the lower 32 bits of the mmx registers are used.
  movd        mm1, [esi]
  movd        mm2, [edi]
  punpcklbw   mm2, mm0
  ExpandOneMinusAlpha esi
  pmullw      mm2, mm4
  psrlw       mm2, 8
  packuswb    mm2, mm0
  paddusb     mm1, mm2

  movd        [edi], mm1

BlendOver_MMX_Done:
  epilogue


; void BlendMultiply_MM(dword *src, dword *dst, dword num)
%define src   ebp + 8
%define dst   ebp + 12
%define num   ebp + 16
align 16
_BlendMultiply_MMX:
  prologue

  ; Get the arguments into registers.
  mov         esi, [src]
  mov         edi, [dst]
  mov         ecx, [num]

  ; We'll work in 64 bit so we need to special case the last pixel if it's odd.
  mov         edx, ecx
  and         edx, 1

  ; num /= 2
  shr         ecx, 1

  pxor        mm7, mm7    ; mm7 = zero

  ; The inner loop that does satured adds on bytes, 2 pixels at a time.
.loop:
  movq        mm0, [edi]  ; Copy the 64 bit destination data into register 0.
  movq        mm1, [esi]  ; Copy the 64 bit source data into register 1.

  movq        mm2, mm0    ; mm2 = [dst0][dst1]
  punpcklbw   mm2, mm7    ; mm2 = dst0 as words.
  movq        mm3, mm0    ; mm3 = [dst0][dst1]
  punpckhbw   mm3, mm7    ; mm3 = dst1 as words.

  movq        mm4, mm1    ; mm4 = [src0][src1]
  punpcklbw   mm4, mm7    ; mm4 = src0 as words
  movq        mm5, mm1    ; mm5 = [src0][src1]
  punpckhbw   mm5, mm7    ; mm5 = src1 as words

  pmullw      mm2, mm4    ; mm2 = dst0 * src0
  psrlw       mm2, 8      ; shifted down to the lower 8 bits in each word.

  pmullw      mm3, mm5    ; mm3 = dst1 * src1
  psrlw       mm3, 8      ; shifted down to the lower 8 bits in each word.

  packuswb    mm2, mm3    ; pack it again, mm2 = [src0 * dst0] [src1 * dst1]

  movq        [edi], mm2  ; Copy the 64 bit result back into memory.

  add         edi, 8
  add         esi, 8

  dec         ecx
  jnz         .loop

  ; Handle the extra odd pixel if there was one.
  cmp         edx, 0
  jz          BlendMultiply_MMX_Done

  movd        mm0, [edi]
  movd        mm1, [esi]

  punpcklbw   mm0, mm7
  punpcklbw   mm1, mm7

  pmullw      mm0, mm1
  psrlw       mm0, 8

  packuswb    mm0, mm7

  movd        [edi], mm0

BlendMultiply_MMX_Done:
  epilogue


; void BlendAdditive(dword *src, dword *dst, dword num)
%define src   ebp + 8
%define dst   ebp + 12
%define num   ebp + 16
align 16
_BlendAdditive_MMX:
  prologue

  ; Get the arguments into registers.
  mov         esi, [src]
  mov         edi, [dst]
  mov         ecx, [num]

  ; We'll work in 64 bit so we need to special case the last pixel if it's odd.
  mov         edx, ecx
  and         edx, 1

  ; num /= 2
  shr         ecx, 1

  ; The inner loop that does satured adds on bytes, 2 pixels at a time.
.loop:
  movq        mm0, [edi]  ; Copy the 64 bit destination data into register 0.
  movq        mm1, [esi]  ; Copy the 64 bit source data into register 1.

  paddusb     mm0, mm1    ; Do saturated addition on each unsigned byte.

  movq        [edi], mm0  ; Copy the 64 bit result back into memory.

  add         edi, 8
  add         esi, 8

  dec         ecx
  jnz         .loop

  ; Handle the extra odd pixel if there was one.
  cmp         edx, 0
  jz          BlendAdditive_MMX_Done

  movd        mm0, [edi]
  movd        mm1, [esi]

  paddusb     mm0, mm1

  movd        [edi], mm0

BlendAdditive_MMX_Done:
  epilogue


; void BlendSubtractive_MMX(dword *src, dword *dst, dword num)
;
%define src   ebp + 8
%define dst   ebp + 12
%define num   ebp + 16
align 16
_BlendSubtractive_MMX:
  prologue

  ; Get the arguments into registers.
  mov         esi, [src]
  mov         edi, [dst]
  mov         ecx, [num]

  ; We'll work in 64 bit so we need to special case the last pixel if it's odd.
  mov         edx, ecx
  and         edx, 1

  ; num /= 2
  shr         ecx, 1

  ; The inner loop that does satured adds on bytes, 2 pixels at a time.
.loop:
  movq        mm0, [edi]  ; Copy the 64 bit destination data into register 0.
  movq        mm1, [esi]  ; Copy the 64 bit source data into register 1.

  psubusb     mm0, mm1    ; Do saturated subtraction on each unsigned byte.

  movq        [edi], mm0  ; Copy the 64 bit result back into memory.

  add         edi, 8
  add         esi, 8

  dec         ecx
  jnz         .loop

  ; Handle the extra odd pixel if there was one.
  cmp         edx, 0
  jz          BlendSubtractive_MMX_Done

  movd        mm0, [edi]
  movd        mm1, [esi]

  psubusb     mm0, mm1

  movd        [edi], mm0

BlendSubtractive_MMX_Done:
  epilogue


%if 0   ; Can't use pcmpgtb, it's a SIGNED compare, unsigned is needed here...
; void BlendLighten(dword *src, dword *dst, dword num)
%define src   ebp + 8
%define dst   ebp + 12
%define num   ebp + 16
align 16
_BlendLighten_MMX:
  prologue

  ; Get the arguments into registers.
  mov         esi, [src]
  mov         edi, [dst]
  mov         ecx, [num]

  ; We'll work in 64 bit so we need to special case the last pixel if it's odd.
  mov         edx, ecx
  and         edx, 1

  ; num /= 2
  shr         ecx, 1

  ; The inner loop that does takes the max for the pixel components, 2 pixels at a time.
.loop:
  movq        mm0, [edi]  ; Copy the 64 bit destination data into register 0.
  movq        mm1, [esi]  ; Copy the 64 bit source data into register 1.

  ; Create the mask.
  movq        mm2, mm0
  pcmpgtb     mm2, mm1    ; mm2 = ">" mask.

  ; Use the mask to get the max of the components.
  pand        mm0, mm2    ; mm0 = src & mask
  pandn       mm2, mm1    ; mm2 = dst & inverse-mask

  por         mm0, mm2    ; mm0 = (src & mask) | (dst & inverse-mask)

  movq        [edi], mm0  ; Copy the 64 bit result back into memory.

  add         edi, 8
  add         esi, 8

  dec         ecx
  jnz         .loop

  ; Handle the extra odd pixel if there was one.
  cmp         edx, 0
  jz          BlendLighten_MMX_Done

  movd        mm0, [edi]
  movd        mm1, [esi]

  ; Create the mask.
  movq        mm2, mm0
  pcmpgtb     mm2, mm0    ; mm2 = ">" mask.

  ; Use the mask to get the max of the components.
  pand        mm0, mm2    ; mm0 = src & mask
  pandn       mm2, mm1    ; mm2 = dst & inverse-mask

  por         mm0, mm2    ; mm0 = (src & mask) | (dst & inverse-mask)

  movd        [edi], mm0

BlendLighten_MMX_Done:
  epilogue
%endif
