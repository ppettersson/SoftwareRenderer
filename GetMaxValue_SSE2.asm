; Exported functions
global _GetMaxByteValue_SSE2
global _GetMaxWordValue_SSE2


; Code
section .text

; Function helper macros
%macro Prologue 0
  ; Set up stack frame.
  push        ebp
  mov         ebp, esp
%endmacro

%macro Epilogue 0
  ; Restore stack frame and return to callee.
  pop         ebp
  ret
%endmacro


; byte GetMaxByteValue_SSE2(byte *data, dword num);
%define data  ebp + 8
%define num   ebp + 12
align 16
_GetMaxByteValue_SSE2:
  Prologue

  push        ebx
  push        ecx
  push        edx
  push        esi

  mov         esi, [data]
  mov         ecx, [num]

  ; Store the "rest" in edx
  mov         edx, ecx
  and         edx, 15

  ; Clear out the register holding the return value.
  xor         eax, eax

  ; Convert to 128 bit.
  shr         ecx, 4
  jz          .handleLeftOvers_byte
  ; There's no point if it's only 16 to 31 bytes.
  dec         ecx
  jz          .handleLeftOvers16_byte

  movdqu      xmm0, [esi];            ; Read the first 16 values.
  add         esi, 16

.inner_loop_byte:
  ; Read the next 16 value.
  movdqu      xmm1, [esi]
  add         esi, 16

  ; Max each individual component with the previous 16 values.
  pmaxub      xmm0, xmm1

  ; The next 16 values.
  dec         ecx
  jnz         .inner_loop_byte

  ; Calculate the max of our 16 values.
.max_of_dword_byte:
  mov         ecx, 4
  movd        ebx, xmm0

  ; ToDo:
  ; This may seem like a perfect place to use cmov since Max(a, b) can't really
  ; take advantage of branch predictability, but the fact that it only works on
  ; 16 bit values makes it a bit trickier.
  ; We'd also have to check for availability first.
.max_of_dword_loop_byte:
  cmp         bl, al
  jna         .cmp_forward0_byte
  mov         al, bl
.cmp_forward0_byte:
  cmp         bh, al
  jna         .cmp_forward1_byte
  mov         al, bh
.cmp_forward1_byte:
  shr         ebx, 16
  cmp         bl, al
  jna         .cmp_forward2_byte
  mov         al, bl
.cmp_forward2_byte:
  cmp         bh, al
  jna         .cmp_forward3_byte
  mov         al, bh
.cmp_forward3_byte:

  ; Shift the register down one dword
  psrldq      xmm0, 4
  movd        ebx, xmm0

  dec         ecx
  jnz         .max_of_dword_loop_byte

.handleLeftOvers16_byte:
  add         edx, 16

.handleLeftOvers_byte:
  mov         ecx, edx
  xor         ebx, ebx                ; Clear the register to avoid partial register stall.

.handleLeftOvers_loop:
  mov         bl, [esi]
  cmp         bl, al
  jna         .cmp_forward_left_overs
  mov         al, bl

.cmp_forward_left_overs:
  inc         esi
  dec         ecx
  jnz         .handleLeftOvers_loop

.done_GetMaxByteValue:
  ; The result will be in al.

  pop         esi
  pop         edx
  pop         ecx
  pop         ebx

  Epilogue

%macro GetMaxInnerLoop 1
  %1          xmm0, [esi];            ; Read the first 8 values.
  add         esi, 8 * 2

%%inner_loop:
  ; Read the next 8 values.
  %1          xmm1, [esi]
  add         esi, 8 * 2

  ; Max each individual component with the previous 8 values.
  pmaxuw      xmm0, xmm1

  ; The next 8 values.
  dec         ecx
  jnz         %%inner_loop
%endm

; byte GetMaxWordValue_SSE2(byte *data, dword num);
%define data  ebp + 8
%define num   ebp + 12
align 16
_GetMaxWordValue_SSE2:
  Prologue

  push        esi
  push        ecx
  push        edx

  mov         esi, [data]
  mov         ecx, [num]

  ; Clear out the register where the result will be returned.
  xor         eax, eax

  ; The inner loop with compare 8 values in parallel.
  ; Keep the amount (1-7) of left over values in edx.
  mov         edx, ecx
  and         edx, 7

  shr         ecx, 3
  jz          .handle_left_overs

  ; Need 16 values to get started.
  dec         ecx
  jz          .handle_left_overs_8

  ; Check if we can use aligned memory reads here.
  test        esi, 15
  jnz         .unaligned_inner_loop

  GetMaxInnerLoop movdqa
  jmp         .pair_down

.unaligned_inner_loop:
  GetMaxInnerLoop movdqu

.pair_down:
  ; Pair down the 8 max values to one.
  mov         ecx, 4
  movd        ebx, xmm0

.max_of_dword:
  cmp         bx, ax
  cmova       ax, bx
  shr         ebx, 16
  cmp         bx, ax
  cmova       ax, bx

  ; Shift the register down one dword
  psrldq      xmm0, 4
  movd        ebx, xmm0

  dec         ecx
  jnz         .max_of_dword

  jmp         .handle_left_overs
 
.handle_left_overs_8:
  add         edx, 8

.handle_left_overs:
  cmp         edx, 0
  jna         .done
  mov         ecx, edx
  
.left_overs_loop:
  movzx       ebx, word [esi]
  add         esi, 2
  cmp         bx, ax
  cmova       ax, bx

  dec         ecx
  jnz         .left_overs_loop

.done:
  ; The result  will be in ax.

  pop         edx
  pop         ecx
  pop         esi

  Epilogue
