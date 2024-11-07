#ifndef PLATFORM_H
#define PLATFORM_H

#include "Header/definitions.h"

void PlatformWriteConsole(char *String);
uptr PlatformFormatString(char *Buffer, uptr BufferSize, char *Format, ...);
void *PlatformAllocateMemory(uptr Size);
bool PlatformFreeMemory(void *Memory);

#define Print(format, ...) do { char *Buffer = (char *)PlatformAllocateMemory(1024); PlatformFormatString(Buffer, 1024, format, __VA_ARGS__); PlatformWriteConsole(Buffer); PlatformFreeMemory(Buffer); } while(0)
#define Log Print

#endif