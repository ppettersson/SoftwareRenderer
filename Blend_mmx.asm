bits 32

; Exported functions
;global _BlendNormal1_MMX
;global _BlendMultiply1_MMX
;global _BlendAdditive1_MMX
;global _BlendSubtractive_MMX
global _BlendNormal_MMX
global _BlendMultiply_MMX
global _BlendAdditive_MMX
global _BlendSubtractive_MMX


; Variables
section .data


; Code
section .text


; void BlendNormal_MMX(dword *src, dword *dst, dword num)
; {
align 16
_BlendNormal_MMX:
  ; Set up stack frame.
  push ebp
  mov ebp,esp

  ; Save away all registers, ToDo: optimize this by checking which ones are actually used...
  pushad

  ; Get the arguments into registers.
  mov         esi, [ebp + 8]  ; src
  mov         edi, [ebp + 12] ; dst
  mov         ecx, [ebp + 16] ; num

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

  ; Restore all clobbered registers.
  popad

  ; Restore stack frame and return to callee.
  pop ebp
  ret
; }


; void BlendMultiply_MM(dword *src, dword *dst, dword num)
; {
align 16
_BlendMultiply_MMX:
  ; Set up stack frame.
  push ebp
  mov ebp,esp

  ; Save away all registers, ToDo: optimize this by checking which ones are actually used...
  pushad

  ; Get the arguments into registers.
  mov         esi, [ebp + 8]  ; src
  mov         edi, [ebp + 12] ; dst
  mov         ecx, [ebp + 16] ; num

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

  ; Restore all clobbered registers.
  popad

  ; Restore stack frame and return to callee.
  pop ebp
  ret
; }


; void BlendAdditive(dword *src, dword *dst, dword num)
; {
align 16
_BlendAdditive_MMX:
  ; Set up stack frame.
  push ebp
  mov ebp,esp

  ; Save away all registers, ToDo: optimize this by checking which ones are actually used...
  pushad

  ; Get the arguments into registers.
  mov         esi, [ebp + 8]  ; src
  mov         edi, [ebp + 12] ; dst
  mov         ecx, [ebp + 16] ; num

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

  ; Restore all clobbered registers.
  popad

  ; Restore stack frame and return to callee.
  pop ebp
  ret
; }


; void BlendSubtractive_MMX(dword *src, dword *dst, dword num)
; {
align 16
_BlendSubtractive_MMX:
  ; Set up stack frame.
  push ebp
  mov ebp,esp

  ; Save away all registers, ToDo: optimize this by checking which ones are actually used...
  pushad

  ; Get the arguments into registers.
  mov         esi, [ebp + 8]  ; src
  mov         edi, [ebp + 12] ; dst
  mov         ecx, [ebp + 16] ; num

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

  ; Restore all clobbered registers.
  popad

  ; Restore stack frame and return to callee.
  pop ebp
  ret
; }
