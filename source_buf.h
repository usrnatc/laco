#if !defined(__SOURCE_BUF_H__)
#define __SOURCE_BUF_H__

#include "common.h"

struct SourceBuf {
    char* Name;
    void* Data;
    u64 Length;
};

SourceBuf MakeSourceBufFromFile(char* Filepath);
void DestroySourceBuf(SourceBuf* Buffer);

#endif // __SOURCE_BUF_H__