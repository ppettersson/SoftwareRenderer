#include "SoftwareRenderer.h"

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
        //da  = (dst & kColorAlpha) >> 24,

        inv = 255 - sa,

        r   = (sr * sa + dr * inv) >> 8,
        g   = (sg * sa + dg * inv) >> 8,
        b   = (sb * sa + db * inv) >> 8;

  return Color((byte)r, (byte)g, (byte)b);
}

dword BlendMultiply1_C(dword src, dword dst)
{
  // Arg1 * Arg2
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
        //sa  = (src & kColorAlpha) >> 24,

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
        //da  = (dst & kColorAlpha) >> 24,

        r   = (sr * dr) >> 8,
        g   = (sg * dg) >> 8,
        b   = (sb * db) >> 8;

  return Color((byte)r, (byte)g, (byte)b);
}

dword BlendAdditive1_C(dword src, dword dst)
{
  // Arg1 + Arg2
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
        //sa  = (src & kColorAlpha) >> 24,

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
        //da  = (dst & kColorAlpha) >> 24,

        r   = Min(sr + dr, (dword)255),
        g   = Min(sg + dg, (dword)255),
        b   = Min(sb + db, (dword)255);

  return Color((byte)r, (byte)g, (byte)b);
}

dword BlendSubtractive1_C(dword src, dword dst)
{
  // Arg1 - Arg2
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
        //sa  = (src & kColorAlpha) >> 24,

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
        //da  = (dst & kColorAlpha) >> 24,

        r   = (dword)Max((int)dr - (int)sr, 0),
        g   = (dword)Max((int)dg - (int)sg, 0),
        b   = (dword)Max((int)db - (int)sb, 0);

  return Color((byte)r, (byte)g, (byte)b);
}

dword BlendScreen1_C(dword src, dword dst)
{
  // Arg1 + Arg2 - Arg1 * Arg2
  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
        //sa  = (src & kColorAlpha) >> 24,

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
        //da  = (dst & kColorAlpha) >> 24,

        r   = Clamp((int)(sr + dr - ((sr * dr) >> 8)), (int)0, (int)255),  // ToDo: underflow...
        g   = Clamp((int)(sg + dg - ((sg * dg) >> 8)), (int)0, (int)255),
        b   = Clamp((int)(sb + db - ((sb * db) >> 8)), (int)0, (int)255);

  return Color((byte)r, (byte)g, (byte)b);
}

dword BlendLighten1_C(dword src, dword dst)
{
  // Max(Arg1, Arg2)

  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
        //sa  = (src & kColorAlpha) >> 24,

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
        //da  = (dst & kColorAlpha) >> 24,

        r   = Max(sr, dr),
        g   = Max(sg, dg),
        b   = Max(sb, db);

  return Color((byte)r, (byte)g, (byte)b);
}

dword BlendDarken1_C(dword src, dword dst)
{
  // Min(Arg1, Arg2)

  dword sr  = (src & kColorRed)   >> 16,
        sg  = (src & kColorGreen) >>  8,
        sb  = (src & kColorBlue)  >>  0,
        //sa  = (src & kColorAlpha) >> 24,

        dr  = (dst & kColorRed)   >> 16,
        dg  = (dst & kColorGreen) >>  8,
        db  = (dst & kColorBlue)  >>  0,
        //da  = (dst & kColorAlpha) >> 24,

        r   = Min(sr, dr),
        g   = Min(sg, dg),
        b   = Min(sb, db);

  return Color((byte)r, (byte)g, (byte)b);
}

void BlendNormal_C(dword *src, dword *dst, dword num)
{
  for (dword i = 0; i < num; ++i)
    dst[i] = BlendNormal1_C(src[i], dst[i]);
}

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
  case kBlend_Multiply:     return "Multiply";
  case kBlend_Additive:     return "Additive";
  case kBlend_Subtractive:  return "Subtractive";
  case kBlend_Screen:       return "Screen";
  case kBlend_Lighten:      return "Lighten";
  case kBlend_Darken:       return "Darken";

  default:                  return "Normal";
  }
}
