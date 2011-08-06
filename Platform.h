#ifndef PLATFORM_H
#define PLATFORM_H

bool PlatformOpen(dword width, dword height);
bool PlatformUpdate(dword *frameBuffer);
void PlatformSetWindowCaption(const char *caption);
void PlatformClose();

#endif // PLATFORM_H
