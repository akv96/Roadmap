#include "Library/platform.h"

#ifdef WINDOWS_OS
    #include <windows.h>
#else
    #error
#endif

typedef u8* va_args;

#define VAGet(args, type) *(type *)VAGet_(args)

#include "Code/string.c"

internal_function va_args
VABegin(void *Format)
{
    va_args Result = 0;

    Result = (u8 *)Format + POINTER_SIZE;
    return Result;
}

internal_function va_args
VAEnd()
{
    va_args Result = 0;

    return Result;
}

internal_function void *
VAGet_(va_args *Args)
{
    void *Result = *Args;

    *Args += POINTER_SIZE;

    return Result;
}

void PlatformWriteConsole(char *String)
{
#ifdef WINDOWS_OS
    OutputDebugStringA(String);
#else
    #error
#endif
}

uptr PlatformFormatString(char *Buffer, uptr BufferSize, char *Format, ...)
{
    uptr Result = 0;

    if(BufferSize < sizeof(char))
    {
        return Result;
    }

    va_args Args = VABegin(&Format);

    char *Octal = "01234567";
    char *Decimal = "0123456789";
    char *HexadecimalLowercase = "0123456789abcdef";
    char *HexadecimalUppercase = "0123456789ABCDEF";
    for(char *c = Format; *c != '\0';)
    {
        if(*c != '%')
        {
            Result += CopyCharacterToBuffer(Buffer + Result, BufferSize - Result, *c);
            c++;
            continue;
        }

        if(*++c == '%')
        {
            Result += CopyCharacterToBuffer(Buffer + Result, BufferSize - Result, *c);
            c++;
        }
        else if(*c == 'c')
        {
            int Value = VAGet(&Args, int);
            Result += CopyCharacterToBuffer(Buffer + Result, BufferSize - Result, Value);
            c++;
        }
        else if((*c == 'd') || (*c == 'i'))
        {
            s32 Value = VAGet(&Args, s32);
            Result += CopySigned32ToBuffer(Buffer + Result, BufferSize - Result, Value, 10, Decimal);
            c++;
        }
        else if(*c == 'f')
        {
            f64 Value = VAGet(&Args, f64);
            Result += CopyFloat64ToBuffer(Buffer + Result, BufferSize - Result, Value, 6);
            c++;
        }
        else if((c[0] == 'l') && (c[1] == 'l') && ((c[2] == 'd') || (c[2] == 'i')))
        {
            s64 Value = VAGet(&Args, s64);
            Result += CopySigned64ToBuffer(Buffer + Result, BufferSize - Result, Value, 10, Decimal);
            c += 3;
        }
        else if((c[0] == 'l') && (c[1] == 'l') && (c[2] == 'u'))
        {
            u64 Value = VAGet(&Args, u64);
            Result += CopyUnsigned64ToBuffer(Buffer + Result, BufferSize - Result, Value, 10, Decimal);
            c += 3;
        }
        else if(*c == 'o')
        {
            u32 Value = VAGet(&Args, u32);
            Result += CopyUnsigned32ToBuffer(Buffer + Result, BufferSize - Result, Value, 8, Octal);
            c++;
        }
        else if(*c == 'p')
        {
            void *Value = VAGet(&Args, void *);
            Result += CopyUnsigned64ToBuffer(Buffer + Result, BufferSize - Result, (u64)Value, 16, HexadecimalLowercase);
            c++;
        }
        else if(*c == 's')
        {
            char *Value = VAGet(&Args, char *);
            Result += CopyStringToBuffer(Buffer + Result, BufferSize - Result, Value, StringLength(Value));
            c++;
        }
        else if(*c == 'u')
        {
            u32 Value = VAGet(&Args, u32);
            Result += CopyUnsigned32ToBuffer(Buffer + Result, BufferSize - Result, Value, 10, Decimal);
            c++;
        }
        else if((*c == 'x') || (*c == 'X'))
        {
            u32 Value = VAGet(&Args, u32);
            
            if(*c == 'x')
            {
                Result += CopyUnsigned32ToBuffer(Buffer + Result, BufferSize - Result, Value, 16, HexadecimalLowercase);
            }
            else
            {
                Result += CopyUnsigned32ToBuffer(Buffer + Result, BufferSize - Result, Value, 16, HexadecimalUppercase);
            }

            c++;
        }
        else
        {
            Assert(0);
        }
    }

    Args = VAEnd();
    Buffer[Result] = '\0';

    return Result;
}

void *PlatformAllocateMemory(uptr Size)
{
    void *Result = 0;

#ifdef WINDOWS_OS
    Result = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if(!Result)
    {
        PlatformWriteConsole("VirtualAlloc() failed\n");
        return Result;
    }
#else
    #error
#endif

    return Result;
}

bool PlatformFreeMemory(void *Memory)
{
    bool Result = 0;

    if(!Memory)
    {
        return Result;
    }

#ifdef WINDOWS_OS
    if(!VirtualFree(Memory, 0, MEM_RELEASE))
    {
        PlatformWriteConsole("VirtualFree() failed\n");
        return Result;
    }
#else
    #error
#endif

    Result = 1;
    return Result;
}

bool PlatformReadFile(file *File, char *FileName)
{
    bool Result = 0;

#ifdef WINDOWS_OS
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(FileHandle == INVALID_HANDLE_VALUE)
    {
        Log("CreateFileA() failed: 0x%X\n", GetLastError());
        return Result;
    }

    LARGE_INTEGER FileSize = {0};
    if(!GetFileSizeEx(FileHandle, &FileSize))
    {
        Log("GetFileSizeEx() failed: 0x%X\n", GetLastError());
        CloseHandle(FileHandle);
        return Result;
    }
    else if(FileSize.QuadPart > MAX_U32)
    {
        Log("File size is too big > %u\n", MAX_U32);
        CloseHandle(FileHandle);
        return Result;
    }

    File->Memory = VirtualAlloc(0, FileSize.QuadPart, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if(!File->Memory)
    {
        Log("VirtualAlloc() failed: 0x%X\n", GetLastError());
        CloseHandle(FileHandle);
        return Result;
    }

    DWORD BytesRead = 0;
    if(!ReadFile(FileHandle, File->Memory, (DWORD)FileSize.QuadPart, &BytesRead, 0))
    {
        DWORD Error = GetLastError();
        if(Error != ERROR_IO_PENDING)
        {
            Log("VirtualAlloc() failed: 0x%X\n", GetLastError());
            VirtualFree(File->Memory, 0, MEM_RELEASE);
            File->Memory = 0;
            CloseHandle(FileHandle);
            return Result;
        }
    }
    else if(BytesRead != FileSize.QuadPart)
    {
        Log("Bytes read %u from file is not equal to expected bytes to read %u\n", BytesRead, (u32)FileSize.QuadPart);
        VirtualFree(File->Memory, 0, MEM_RELEASE);
        File->Memory = 0;
        CloseHandle(FileHandle);
        return Result;
    }

    File->Size = FileSize.QuadPart;

    CloseHandle(FileHandle);
#else
    #error
#endif

    Result = 1;
    return Result;
}

bool PlatformFreeFile(file *File)
{
    bool Result = 0;

    if(!File->Memory)
    {
        Log("Attempt to free memory that is 0\n");
        return Result;
    }

#ifdef WINDOWS_OS
    if(!VirtualFree(File->Memory, 0, MEM_RELEASE))
    {
        Log("VirtualFree() failed: 0x%X\n", GetLastError());
        return Result;
    }
#else
    #error
#endif

    Result = 1;
    return Result;
}