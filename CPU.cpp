#include "SoftwareRenderer.h"
#include "CPU.h"

enum CPU_Features
{
  kCPU_MMX    = 1 << 0,
  kCPU_SSE    = 1 << 1,
  kCPU_SSE2   = 1 << 2,
  kCPU_SSE3   = 1 << 3,
  kCPU_SSSE3  = 1 << 4,
  kCPU_SSE4_1 = 1 << 5,
  kCPU_SSE4_2 = 1 << 6
};

dword GetCpuFeatures()
{
  dword c = 0, d = 0;

  __asm
  {
    // ToDo:
    // The ID flag (bit 21) in the EFLAGS register indicates support for the CPUID instruction.
    // If a software procedure can set and clear this flag, the processor executing the
    // procedure supports the CPUID instruction.

    // Read out the feature set of the current CPU.
    mov   eax, 1
    cpuid
    mov   c, ecx
    mov   d, edx
  }

  // The default is fall back on full C implementation.
  dword cpuFeatures = 0;

  if (c & (1 << 0))
    cpuFeatures |= kCPU_SSE3;
  if (c & (1 << 10))
    cpuFeatures |= kCPU_SSSE3;
  if (c & (1 << 20))
    cpuFeatures |= kCPU_SSE4_1;
  if (c & (1 << 21))
    cpuFeatures |= kCPU_SSE4_2;

  if (d & (1 << 24))
    cpuFeatures |= kCPU_MMX;
  if (d & (1 << 26))
    cpuFeatures |= kCPU_SSE;
  if (d & (1 << 27))
    cpuFeatures |= kCPU_SSE2;

  return cpuFeatures;
}

// C versions.
extern dword BlendNormal1_C       (dword src, dword dst);
extern dword BlendMultiply1_C     (dword src, dword dst);
extern dword BlendAdditive1_C     (dword src, dword dst);
extern dword BlendSubtractive1_C  (dword src, dword dst);
extern dword BlendScreen1_C       (dword src, dword dst);
extern dword BlendLighten1_C      (dword src, dword dst);
extern dword BlendDarken1_C       (dword src, dword dst);
extern void BlendNormal_C         (dword *src, dword *dst, dword num);
extern void BlendMultiply_C       (dword *src, dword *dst, dword num);
extern void BlendAdditive_C       (dword *src, dword *dst, dword num);
extern void BlendSubtractive_C    (dword *src, dword *dst, dword num);
extern void BlendScreen_C         (dword *src, dword *dst, dword num);
extern void BlendLighten_C        (dword *src, dword *dst, dword num);
extern void BlendDarken_C         (dword *src, dword *dst, dword num);

// MMX versions.
extern dword BlendNormal1_MMX     (dword src, dword dst);
extern dword BlendMultiply1_MMX   (dword src, dword dst);
extern dword BlendAdditive1_MMX   (dword src, dword dst);
extern dword BlendSubtractive1_MMX(dword src, dword dst);
//extern dword BlendScreen1_MMX     (dword src, dword dst);
//extern dword BlendLighten1_MMX    (dword src, dword dst);
//extern dword BlendDarken1_MMX     (dword src, dword dst);
extern "C" {
extern void BlendNormal_MMX       (dword *src, dword *dst, dword num);
extern void BlendMultiply_MMX     (dword *src, dword *dst, dword num);
extern void BlendAdditive_MMX     (dword *src, dword *dst, dword num);
extern void BlendSubtractive_MMX  (dword *src, dword *dst, dword num);
//extern void BlendScreen_MMX       (dword *src, dword *dst, dword num);
//extern void BlendLighten_MMX      (dword *src, dword *dst, dword num);
//extern void BlendDarken_MMX       (dword *src, dword *dst, dword num);
}


// The dispatch table.
dword (*BlendNormal1)             (dword src, dword dst) = BlendNormal1_C;
dword (*BlendMultiply1)           (dword src, dword dst) = BlendMultiply1_C;
dword (*BlendAdditive1)           (dword src, dword dst) = BlendAdditive1_C;
dword (*BlendSubtractive1)        (dword src, dword dst) = BlendSubtractive1_C;
dword (*BlendScreen1)             (dword src, dword dst) = BlendScreen1_C;
dword (*BlendLighten1)            (dword src, dword dst) = BlendLighten1_C;
dword (*BlendDarken1)             (dword src, dword dst) = BlendDarken1_C;
void (*BlendNormal)               (dword *src, dword *dst, dword num) = BlendNormal_C;
void (*BlendMultiply)             (dword *src, dword *dst, dword num) = BlendMultiply_C;
void (*BlendAdditive)             (dword *src, dword *dst, dword num) = BlendAdditive_C;
void (*BlendSubtractive)          (dword *src, dword *dst, dword num) = BlendSubtractive_C;
void (*BlendScreen)               (dword *src, dword *dst, dword num) = BlendScreen_C;
void (*BlendLighten)              (dword *src, dword *dst, dword num) = BlendLighten_C;
void (*BlendDarken)               (dword *src, dword *dst, dword num) = BlendDarken_C;


void SetupDispatchTable()
{
  dword cpuFeatures = GetCpuFeatures();

  if (cpuFeatures & kCPU_MMX)
  {
    BlendNormal1      = BlendNormal1_MMX;
    BlendMultiply1    = BlendMultiply1_MMX;
    BlendAdditive1    = BlendAdditive1_MMX;
    BlendSubtractive1 = BlendSubtractive1_MMX;
    //BlendScreen1      = BlendScreen1_MMX;
    //BlendLighten1     = BlendLighten1_MMX;
    //BlendDarken1      = BlendDarken1_MMX;
    BlendNormal       = BlendNormal_MMX;
    BlendMultiply     = BlendMultiply_MMX;
    BlendAdditive     = BlendAdditive_MMX;
    BlendSubtractive  = BlendSubtractive_MMX;
    //BlendScreen       = BlendScreen_MMX;
    //BlendLighten      = BlendLighten_MMX;
    //BlendDarken       = BlendDarken_MMX;
  }
}
