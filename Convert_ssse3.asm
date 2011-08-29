; Exported functions
global _ConvertRGBAtoBGRA_SSSE3

; Initialized variables.
section .data
align 16
; The new position of each byte in the register after the shuffle operation.
rgba_shuffle_mask  db   2, 1, 0, 3,   6, 5, 4, 7,   10, 9, 8, 11,   14, 13, 12, 15

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

; The inner loop that shuffles the byte.
; The argument should be either movdqa if memory is aligned or movdqu if it isn't.
%macro ShuffleBytes 1
  ; We'll work in 128 bit so we need to handle the special case if it isn't aligned.
  mov         edx, ecx
  shr         ecx, 2 + 3        ; ecx = num / 4, then unrolled 8 times.
  jz          %%handle_up_to_31_pixels

  mov         eax, 0
%%loop_32_pixels:
  ; Load from memory into registers.
  %1          xmm0, [esi + eax + 16 * 0]
  %1          xmm1, [esi + eax + 16 * 1]
  %1          xmm2, [esi + eax + 16 * 2]
  %1          xmm3, [esi + eax + 16 * 3]
  %1          xmm4, [esi + eax + 16 * 4]
  %1          xmm5, [esi + eax + 16 * 5]
  %1          xmm6, [esi + eax + 16 * 6]
  %1          xmm7, [esi + eax + 16 * 7]

  ; Perform the shuffle operation.
  pshufb      xmm0, [rgba_shuffle_mask]
  pshufb      xmm1, [rgba_shuffle_mask]
  pshufb      xmm2, [rgba_shuffle_mask]
  pshufb      xmm3, [rgba_shuffle_mask]
  pshufb      xmm4, [rgba_shuffle_mask]
  pshufb      xmm5, [rgba_shuffle_mask]
  pshufb      xmm6, [rgba_shuffle_mask]
  pshufb      xmm7, [rgba_shuffle_mask]

  ; Store to memory from registers.
  %1          [edi + eax + 16 * 0], xmm0
  %1          [edi + eax + 16 * 1], xmm1
  %1          [edi + eax + 16 * 2], xmm2
  %1          [edi + eax + 16 * 3], xmm3
  %1          [edi + eax + 16 * 4], xmm4
  %1          [edi + eax + 16 * 5], xmm5
  %1          [edi + eax + 16 * 6], xmm6
  %1          [edi + eax + 16 * 7], xmm7

  add         eax, 16 * 8

  dec         ecx
  jnz         %%loop_32_pixels

  ; Move the source and destination pointers forward in case any
  ; the two left over cases are hit.
  add         esi, eax
  add         edi, eax

  ; 4-32 pixels left.
%%handle_up_to_31_pixels:
  mov         ecx, edx
  and         ecx, 31           ; ecx = number of left over pixels to handle.
  shr         ecx, 2
  jz          %%handle_up_to_3_pixels

  mov         eax, 0
%%loop_4_pixels:
  %1          xmm0, [esi + eax]
  pshufb      xmm0, [rgba_shuffle_mask]
  %1          [edi + eax], xmm0

  add         eax, 16

  dec         ecx
  jnz         %%loop_4_pixels

  ; Move the source and destination pointers forward in case the
  ; last left over cases is hit.
  add         esi, eax
  add         edi, eax

  ; 1-3 pixels left.
%%handle_up_to_3_pixels:
  mov         ecx, edx
  and         ecx, 3
  jz          .done

  ; Preserve the count.
  mov         ebx, ecx

%%loop_load_last_3_pixels:
  ; Copy one pixel from memory.
  movd        xmm1, [esi]

  ; Shift previous pixel up a step if there was one.
  pslldq      xmm0, 4
  ; Merge the result from previous pixels.
  por         xmm0, xmm1
  ; Move to next pixel.
  add         esi, 4

  dec         ecx
  jnz         %%loop_load_last_3_pixels

  ; Do the shuffle on 1-3 pixels (and 12-15 garbage pixels).
  pshufb      xmm0, [rgba_shuffle_mask]

  mov         ecx, ebx
%%loop_store_last_3_pixels:
  ; Copy pixel data to memory.
  movd        [edi + ecx * 4 - 4], xmm0

  ; Shift previous pixel down one step if there was one.
  psrldq      xmm0, 4

  dec         ecx
  jnz         %%loop_store_last_3_pixels
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

  ; Check if we can use the aligned version for extra speed.
  ; This means that both the source and destination pointers
  ; are 16 byte aligned.
  mov         eax, esi
  and         eax, 15
  jnz         .unaligned_loop
  mov         ebx, edi
  and         ebx, 15
  jnz         .unaligned_loop

  ShuffleBytes movdqa
  Epilogue

  ; We have to use unaligned memory access. This is not quite
  ; as fast, but the difference is quite small on modern CPUs
  ; like newer Core2's.
.unaligned_loop:
  ShuffleBytes movdqu

.done
  Epilogue
