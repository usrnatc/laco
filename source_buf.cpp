#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include "common.h"
#include "source_buf.h"


SourceBuf
MakeSourceBufFromFile(char* Filepath)
{
    SourceBuf Result = {};
    DWORD ResultErr;
    HANDLE ResultHandle;
    DWORD ResultLength;
    DWORD ResultBytesRead;

    ResultHandle = CreateFileA(Filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (ResultHandle == INVALID_HANDLE_VALUE) {
        ResultErr = GetLastError();
        fprintf(stderr, "Error(%lu): could not open \"%s\" for reading\n", ResultErr, Filepath);
        return Result;
    }

    ResultLength = GetFileSize(ResultHandle, NULL);

    if (ResultLength == INVALID_FILE_SIZE) {
        ResultErr = GetLastError();
        fprintf(stderr, "Error(%lu): \"%s\" has an invalid file size\n", ResultErr, Filepath);
        CloseHandle(ResultHandle);
        return Result;
    }

    Result.Data = VirtualAlloc(NULL, ResultLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!Result.Data) {
        fprintf(stderr, "Error: could not allocate buffer of %lu bytes for \"%s\"\n", ResultLength, Filepath);
        CloseHandle(ResultHandle);
        return Result;
    }

    if(!ReadFile(ResultHandle, Result.Data, ResultLength, &ResultBytesRead, NULL) || ResultBytesRead != ResultLength) {
        fprintf(stderr, "Error: failed to read \"%s\"\n", Filepath);
        VirtualFree(Result.Data, 0, MEM_RELEASE);
        CloseHandle(ResultHandle);
        return Result;
    }

    CloseHandle(ResultHandle);
    Result.Length = (u64) ResultLength;
    Result.Name = Filepath;

    return Result;
}

void
DestroySourceBuf(SourceBuf* Buffer)
{
    if (!Buffer->Length)
        return;

    VirtualFree(Buffer->Data, 0, MEM_RELEASE);
}