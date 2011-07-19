#include "SoftwareRenderer.h"

// Erik's MicroFont code

// The font data
const unsigned char kMicroFont[5 * 43] = 
{
  34, 148, 236, 144, 132, 65, 0, 0, 0, 46, 103, 56, 143, 51, 204, 96, 0, 32, 33, 
  142, 103, 25, 207, 121, 210, 119, 165, 8, 197, 156, 103, 29, 249, 70, 49, 
  139, 141, 6, 16, 8, 49, 24, 0, 34, 191, 77, 40, 136, 37, 72, 0, 0, 83, 32, 
  133, 8, 64, 82, 145, 8, 71, 16, 86, 148, 165, 40, 66, 18, 32, 169, 13, 230, 
  82, 148, 160, 73, 70, 42, 80, 136, 130, 40, 4, 33, 8, 0, 32, 20, 226, 112, 8, 
  35, 156, 7, 0, 149, 35, 25, 78, 112, 140, 112, 0, 128, 8, 150, 247, 33, 46, 
  114, 222, 32, 177, 10, 214, 92, 151, 24, 73, 42, 164, 33, 8, 66, 0, 0, 97, 
  12, 160, 0, 62, 85, 200, 8, 37, 72, 64, 1, 25, 36, 5, 225, 73, 18, 17, 8, 71, 
  16, 16, 148, 165, 40, 66, 82, 36, 169, 8, 206, 80, 101, 4, 73, 42, 170, 34, 
  8, 34, 0, 0, 33, 9, 64, 32, 20, 233, 180, 4, 65, 0, 128, 18, 14, 119, 184, 
  78, 49, 12, 96, 16, 32, 32, 142, 151, 25, 207, 65, 146, 115, 37, 232, 197, 
  144, 20, 184, 70, 17, 81, 35, 140, 22, 3, 224, 49, 24, 0, 
};

// Write a single micro character using the pixelformat "P" where "data" points 
// to the upper left corner and "stride" denotes the number of elements you 
// need to step to step a line down. The colour "fill" will be used to draw 
// the character.
template <typename P>
void WriteMicroChar(char ch, P *data, int stride, const P &fill)
{
  // Only write printable characters (don't count space as printable).
  if (ch > 32 && ch < 127)
  {
    // Get the index into our character array. If it's a lowercase char we need
    // to shift it to the uppercase counterpart since we don't have lowercase
    // chars.
    if (ch > 122)
      ch -= 26;
    else if (ch > 96)
      ch -= 32;
    ch -= 33;

    // Get the byte and bit offset our character starts on for each row.
    int byteOffset = ch * 5 / 8,
        bitOffset = ch * 5 % 8;

    // Step for each pixel row in the character.
    for (int r = 0; r < 5; ++r)
    {
      // Step for each pixel column in the character.
      for (int c = 0; c < 5; ++c)
      {
        // Check if the pixel should be set. 
        if (kMicroFont[r * 43 + byteOffset + (bitOffset + c) / 8] & (0x80 >> ((bitOffset + c) % 8)))
          *data = fill;

        // Step to the next pixel.
        ++data;
      }
      // Step to the next pixel row.
      data += stride - 5;
    }
  }
}

// Write a string of micro characters using the pixelformat "P" where "data" 
// points to the upper left corner and "stride" denotes the number of elements 
// you need to step to step a line down. The colour "fill" will be used to 
// draw the characters.
template <typename P>
void WriteMicroString(const char *ch, P *data, int stride, const P &fill)
{
  int xpos = 0,
      ypos = 0;
  while (*ch != 0)
  {
    if (*ch == 13 || *ch == 10)
    {
      xpos = -6;
      ypos += 6;
      if (ch[1] == 10) 
        ++ch;
    }
    WriteMicroChar(*ch, data + stride * ypos + xpos, stride, fill);
    ++ch;
    xpos += 6;
  }
}

void DrawString(dword x, dword y, const char *text, dword color)
{
  WriteMicroString<dword>(text, frameBuffer + x + y * screenWidth, screenWidth, color);
}
