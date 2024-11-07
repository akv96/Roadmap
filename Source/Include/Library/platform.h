#ifndef PLATFORM_H
#define PLATFORM_H

#include "Header/definitions.h"

typedef struct
{
    void *Memory;
    uptr Size;
} file;

void PlatformWriteConsole(char *String);
uptr PlatformFormatString(char *Buffer, uptr BufferSize, char *Format, ...);
void *PlatformAllocateMemory(uptr Size);
bool PlatformFreeMemory(void *Memory);
bool PlatformReadFile(file *File, char *FileName);
bool PlatformFreeFile(file *File);

#define Print(format, ...) do { char *Buffer = (char *)PlatformAllocateMemory(1024); PlatformFormatString(Buffer, 1024, format, __VA_ARGS__); PlatformWriteConsole(Buffer); PlatformFreeMemory(Buffer); } while(0)
#define Log(format, ...) Print("File: %s\nFunction: %s\nLine: %u\nMessage: " format "\n", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#endif