#include "SoftwareRenderer.h"

// Sometimes we're really using XRGB, and then it's unnecessary to operate on the alpha channel.
//   Likewise, sometimes the alpha is only used for the blend operation and we're not really 
// interested in the resulting alpha channel.
//   This is only useful for the C version, since the assembler versions use SIMD operations
// that work on all the channels at the same time.
#define INCLUDE_ALPHA_CHANNEL

dword BlendNormal1_C(dword src, dword dst)
{
  // Arg1 * alpha + Arg2 * (1 - alpha)
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
        sa  = (src & kColorAlpha) >> 24,

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        da  = (dst & kColorAlpha) >> 24,
#endif

        inv = 256 - sa,

        r   = (sr * sa + dr * inv) >> 8,
        g   = (sg * sa + dg * inv) >> 8,
        b   = (sb * sa + db * inv) >> 8;
#ifdef INCLUDE_ALPHA_CHANNEL
  dword a   = sa + ((da * inv) >> 8);
  return Color((byte)r, (byte)g, (byte)b, (byte)a);
#else
  return Color((byte)r, (byte)g, (byte)b);
#endif
}

#if 1
// src is assumed to already be premultiplied with it's alpha.
// src + dst * (1 - alpha);
dword BlendOver1_C(dword src, dword dst)
{
  // Get 1.0 - source.alpha into a variable.
  // Notice the we use 256 and not 255 here to maintain a bit of precision.
  dword inv = 256 - (src >> 24);

  // Extract the double components of the source pixel.
  dword src_rb =  src & 0x00ff00ff;
  dword src_ag = (src & 0xff00ff00) >> 8;

  // Extract the double component of the destination pixel.
  dword dst_rb =  dst & 0x00ff00ff;
  dword dst_ag = (dst & 0xff00ff00) >> 8;

  // Multiply the destination pixel with the alpha value.
  // The result is a 16 bit value, but since we only have a byte of precision
  // for each color channel in the pixel we'll keep operating on the higher
  // byte and later bit mask out the lower byte.
  dword rb = dst_rb * inv;
  dword ag = dst_ag * inv;

  // The source pixel is premultiplied so no multiplication is needed
  // here, only that we shift it up to be in the same position as the
  // destination pixel, then add it.
  rb += (src_rb << 8);
  ag += (src_ag << 8);

  // To get the final result we need to shift down the red and blue component
  // one byte.
  rb >>= 8;

  // Mask out the channels.
  rb &= 0x00ff00ff;
  ag &= 0xff00ff00;

  // And we get the final result by Or:ing them together.
  return ag | rb;
}
#else
dword BlendOver1_C(dword src, dword dst)
{
  // Arg1 is assumed to already be premultiplied with it's alpha.
  // Arg1 + Arg2 * (1 - alpha);
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
        sa  = (src & kColorAlpha) >> 24,

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
        da  = (dst & kColorAlpha) >> 24,

        inv = 256 - sa,

        r   = sr + ((dr * inv) >> 8),
        g   = sg + ((dg * inv) >> 8),
        b   = sb + ((db * inv) >> 8);
#ifdef INCLUDE_ALPHA_CHANNEL
  dword a   = sa + ((da * inv) >> 8);
  return Color((byte)r, (byte)g, (byte)b, (byte)a);
#else
  return Color((byte)r, (byte)g, (byte)b);
#endif
}
#endif

dword BlendMultiply1_C(dword src, dword dst)
{
  // Arg1 * Arg2
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        sa  = (src & kColorAlpha) >> 24,
#endif

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        da  = (dst & kColorAlpha) >> 24,
#endif

        r   = (sr * dr) >> 8,
        g   = (sg * dg) >> 8,
        b   = (sb * db) >> 8;
#ifdef INCLUDE_ALPHA_CHANNEL
  dword a   = (sa * da) >> 8;
  return Color((byte)r, (byte)g, (byte)b, (byte)a);
#else
  return Color((byte)r, (byte)g, (byte)b);
#endif
}

dword BlendAdditive1_C(dword src, dword dst)
{
  // Arg1 + Arg2
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        sa  = (src & kColorAlpha) >> 24,
#endif

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        da  = (dst & kColorAlpha) >> 24,
#endif

        r   = Min(sr + dr, (dword)255),
        g   = Min(sg + dg, (dword)255),
        b   = Min(sb + db, (dword)255);
#ifdef INCLUDE_ALPHA_CHANNEL
  dword a   = Min(sa + da, (dword)255);
  return Color((byte)r, (byte)g, (byte)b, (byte)a);
#else
  return Color((byte)r, (byte)g, (byte)b);
#endif
}

dword BlendSubtractive1_C(dword src, dword dst)
{
  // Arg1 - Arg2
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        sa  = (src & kColorAlpha) >> 24,
#endif

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        da  = (dst & kColorAlpha) >> 24,
#endif

        r   = (dword)Max((int)dr - (int)sr, 0),
        g   = (dword)Max((int)dg - (int)sg, 0),
        b   = (dword)Max((int)db - (int)sb, 0);
#ifdef INCLUDE_ALPHA_CHANNEL
  dword a   = (dword)Max((int)da - (int)sa, 0);
  return Color((byte)r, (byte)g, (byte)b, (byte)a);
#else
  return Color((byte)r, (byte)g, (byte)b);
#endif
}

dword BlendScreen1_C(dword src, dword dst)
{
  // Arg1 + Arg2 - Arg1 * Arg2
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        sa  = (src & kColorAlpha) >> 24,
#endif

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        da  = (dst & kColorAlpha) >> 24,
#endif

        r   = Clamp((int)(sr + dr - ((sr * dr) >> 8)), (int)0, (int)255),  // ToDo: underflow...
        g   = Clamp((int)(sg + dg - ((sg * dg) >> 8)), (int)0, (int)255),
        b   = Clamp((int)(sb + db - ((sb * db) >> 8)), (int)0, (int)255);
#ifdef INCLUDE_ALPHA_CHANNEL
  dword a   = Clamp((int)(sa + da - ((sa * da) >> 8)), (int)0, (int)255);
  return Color((byte)r, (byte)g, (byte)b, (byte)a);
#else
  return Color((byte)r, (byte)g, (byte)b);
#endif
}

dword BlendLighten1_C(dword src, dword dst)
{
  // Max(Arg1, Arg2)

  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        sa  = (src & kColorAlpha) >> 24,
#endif

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        da  = (dst & kColorAlpha) >> 24,
#endif

        r   = Max(sr, dr),
        g   = Max(sg, dg),
        b   = Max(sb, db);
#ifdef INCLUDE_ALPHA_CHANNEL
  dword a   = Max(sa, da);
  return Color((byte)r, (byte)g, (byte)b, (byte)a);
#else
  return Color((byte)r, (byte)g, (byte)b);
#endif
}

dword BlendDarken1_C(dword src, dword dst)
{
  // Min(Arg1, Arg2)

  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        sa  = (src & kColorAlpha) >> 24,
#endif

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
#ifdef INCLUDE_ALPHA_CHANNEL
        da  = (dst & kColorAlpha) >> 24,
#endif

        r   = Min(sr, dr),
        g   = Min(sg, dg),
        b   = Min(sb, db);
#ifdef INCLUDE_ALPHA_CHANNEL
  dword a   = Min(sa, da);
  return Color((byte)r, (byte)g, (byte)b, (byte)a);
#else
  return Color((byte)r, (byte)g, (byte)b);
#endif
}

void BlendNormal_C(dword *src, dword *dst, dword num)
{
  for (dword i = 0; i < num; ++i)
    dst[i] = BlendNormal1_C(src[i], dst[i]);
}

#if 0
void BlendOver_C(dword *s, dword *d, dword num)
{
  while (num--)
  {
    dword src = *s;
    dword dst = *d;

    // Get 1.0 - source.alpha into a variable.
    // Notice the we use 256 and not 255 here to maintain a bit of precision.
    dword inv = 256 - (src >> 24);

    // Extract the double components of the source pixel.
    dword src_rb =  src & 0x00ff00ff;
    dword src_ag = (src & 0xff00ff00) >> 8;

    // Extract the double component of the destination pixel.
    dword dst_rb =  dst & 0x00ff00ff;
    dword dst_ag = (dst & 0xff00ff00) >> 8;

    // Multiply the destination pixel with the alpha value.
    // The result is a 16 bit value, but since we only have a byte of precision
    // for each color channel in the pixel we'll keep operating on the higher
    // byte and later bit mask out the lower byte.
    dword rb = dst_rb * inv;
    dword ag = dst_ag * inv;

    // The source pixel is premultiplied so no multiplication is needed
    // here, only that we shift it up to be in the same position as the
    // destination pixel, then add it.
    rb += (src_rb << 8);
    ag += (src_ag << 8);

    // To get the final result we need to shift down the red and blue component
    // one byte.
    rb >>= 8;

    // Mask out the channels.
    rb &= 0x00ff00ff;
    ag &= 0xff00ff00;

    // And we get the final result by Or:ing them together.
    *d++ = ag | rb;
  }
}
#else
void BlendOver_C(dword *src, dword *dst, dword num)
{
  for (dword i = 0; i < num; ++i)
    dst[i] = BlendOver1_C(src[i], dst[i]);
}
#endif

void BlendMultiply_C(dword *src, dword *dst, dword num)
{
  for (dword i = 0; i < num; ++i)
    dst[i] = BlendMultiply1_C(src[i], dst[i]);
}

void BlendAdditive_C(dword *src, dword *dst, dword num)
{
  for (dword i = 0; i < num; ++i)
    dst[i] = BlendAdditive1_C(src[i], dst[i]);
}

void BlendSubtractive_C(dword *src, dword *dst, dword num)
{
  for (dword i = 0; i < num; ++i)
    dst[i] = BlendSubtractive1_C(src[i], dst[i]);
}

void BlendScreen_C(dword *src, dword *dst, dword num)
{
  for (dword i = 0; i < num; ++i)
    dst[i] = BlendScreen1_C(src[i], dst[i]);
}

void BlendLighten_C(dword *src, dword *dst, dword num)
{
  for (dword i = 0; i < num; ++i)
    dst[i] = BlendLighten1_C(src[i], dst[i]);
}

void BlendDarken_C(dword *src, dword *dst, dword num)
{
  for (dword i = 0; i < num; ++i)
    dst[i] = BlendDarken1_C(src[i], dst[i]);
}

BlendFunc_t BlendFunc(BlendType t)
{
  switch (t)
  {
  case kBlend_Normal:       return BlendNormal;
  case kBlend_Over:         return BlendOver;
  case kBlend_Multiply:     return BlendMultiply;
  case kBlend_Additive:     return BlendAdditive;
  case kBlend_Subtractive:  return BlendSubtractive;
  case kBlend_Screen:       return BlendScreen;
  case kBlend_Lighten:      return BlendLighten;
  case kBlend_Darken:       return BlendDarken;

  default:                  return BlendNormal;
  }
}

const char *BlendFuncName(BlendType t)
{
  switch (t)
  {
  case kBlend_Normal:       return "Normal";
  case kBlend_Over:         return "Over";
  case kBlend_Multiply:     return "Multiply";
  case kBlend_Additive:     return "Additive";
  case kBlend_Subtractive:  return "Subtractive";
  case kBlend_Screen:       return "Screen";
  case kBlend_Lighten:      return "Lighten";
  case kBlend_Darken:       return "Darken";

  default:                  return "Normal";
  }
}
