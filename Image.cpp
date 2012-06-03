#include "SoftwareRenderer.h"
#include <stdlib.h>
#include <fstream>

#pragma pack(push)
#pragma pack(1) // Dont pad the following struct.
struct TgaHeader
{
	byte  idLength,           // Length of optional identification sequence.
		paletteType,        // Indicates whether a palette is present (1=has palette)
		imageType;          // Image data type (0=none,1=indexed,2=rgb,3=grey,+8=rle packed)
	word  firstPaletteEntry,  // First palette index, if present.
		numPaletteEntries;  // Number of palette entries, if present.
	byte  paletteBits;        // Number of bits per palette entry.
	word  x,                  // Horizontal pixel coordinate of lower left of image.
		y,                  // Vertical pixel coordinate of lower left of image.
		width,              // Image width in pixels.
		height;             // Image height in pixels.
	byte  depth,              // Image color depth (bits per pixel).
		descriptor;         // Image attribute flags.
};
#pragma pack(pop)

Image::Image(dword w, dword h)
	: width(w)
	, height(h)
{
	dword numPixels = w * h;
	data = numPixels ? (dword *)AllocAlign(numPixels * sizeof(dword)) : NULL;
}

Image::~Image()
{
	FreeAlign(data);
}

Image *LoadTGA(const char *fileName)
{
	std::ifstream file(fileName, std::ios::binary);
	if (!file)
		return NULL;

	TgaHeader header;
	file.read((char *)&header, sizeof(header));

	Image *image;

	dword i;
	switch (header.depth)
	{
	case 32:
		image = new Image(header.width, header.height);
		file.read((char *)image->data, image->width * image->height * sizeof(dword));
		// ToDo: This has not been tested and it would be much cleaner and faster to use an in-place conversion routine instead.
		//ConvertRGBAtoBGRA(image->data, image->data, image->width * image->height);
		break;

	case 24:
		image = new Image(header.width, header.height);
		for (i = 0; i < image->width * image->height; ++i)
		{
			byte rgb[3];
			file.read((char *)rgb, 3);
			image->data[i] = Color(rgb[2], rgb[1], rgb[0]); // Note the RGB to BGRA conversion here.
		}
		break;

	default:
		return NULL;
	}

	if (!(header.descriptor & 0x20)) // vertical flip
	{
		int pitch = image->width * 4;
		byte *line = (byte *)AllocAlign(sizeof(byte) * pitch);
		byte *buffer = (byte *)image->data;

		for (int i = 0; i < (int)image->height / 2; i++)
		{
			int startOffset = i * pitch,
				endOffset   = (image->height - 1 - i) * pitch;

			// Copy from end of bitmap to temporary buffer.
			MemCpy(line, buffer + endOffset, pitch);

			// Copy from start of bitmap to end of bitmap.
			MemCpy(buffer + endOffset, buffer + startOffset, pitch);

			// Copy from temporary buffer to start of bitmap.
			MemCpy(buffer + startOffset, line, pitch);
		}
		FreeAlign(line);
	}

	return image;
}

void Blit(Image *image, dword x, dword y)
{
	dword *dst = frameBuffer + x + y * screenWidth,
		*src = image->data;

	for (dword y = 0; y < image->height; ++y)
	{
		MemCpy(dst, src, image->width * sizeof(dword));

		dst += screenWidth;
		src += image->width;
	}
}

void Blit(Image *image, dword x, dword y, dword w, dword h)
{
	StretchBlitBiLinear(frameBuffer, screenWidth, x, y, w, h, image->data, image->width, image->width, image->height);
}

void BlitBlend(Image *image, dword x, dword y, void (BlendFunc)(dword *src, dword *dst, dword num))
{
	dword *dst = frameBuffer + x + y * screenWidth,
		*src = image->data;

	for (dword y = 0; y < image->height; ++y)
	{
		BlendFunc(src, dst, image->width);

		dst += screenWidth;
		src += image->width;
	}

	BlendEnd();
}

void Resize(Image *image, dword newWidth, dword newHeight)
{
	dword *newData = (dword *)AllocAlign(sizeof(dword) * newWidth * newHeight);
	StretchBlitBiLinear(newData, newWidth, 0, 0, newWidth, newHeight, image->data, image->width, image->width, image->height);

	FreeAlign(image->data);

	image->width  = newWidth;
	image->height = newHeight;
	image->data   = newData;
}

void CreateAlphaChannel(Image *image, bool mask)
{
	for (dword i = 0; i < image->width * image->height; ++i)
	{
		dword r = (image->data[i] & 0x00ff0000) >> 16;
		dword g = (image->data[i] & 0x0000ff00) >>  8;
		dword b = (image->data[i] & 0x000000ff) >>  0;

		dword a;

		if (mask)
			a = (r & g & b) ? 0xff : 0;
		else
			a = (r + g + b * 2) / 4;

		image->data[i] = (a << 24) | (r << 16) | (g << 8) | (b << 0);
	}
}
