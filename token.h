#if !defined(__TOKEN_H__)
#define __TOKEN_H__

#define VECTOR_SIZE 32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <immintrin.h>

#include "common.h"

enum TokenKind : u8 {
    // one byte punctuators
    TokenKind_Exclaim = 33,
    TokenKind_OpenParen = 40,
    TokenKind_CloseParen = 41,
    TokenKind_Star = 42,
    TokenKind_Plus = 43,
    TokenKind_Minus = 45,
    TokenKind_Slash = 47,
    TokenKind_Colon = 58,
    TokenKind_Semicolon = 59,
    TokenKind_Less = 60,
    TokenKind_Equal = 61,
    TokenKind_Greater = 62,
    TokenKind_OpenSquareBracket = 91,
    TokenKind_CloseSquareBracket = 93,
    TokenKind_BitwiseOr = 124,
    TokenKind_BitwiseAnd = 38,
    TokenKind_Tilde = 126,

    // two byte punctuators
    TokenKind_Assign = 117,
    TokenKind_Range = 90,
    TokenKind_ExclaimEqual = 92,
    TokenKind_GreaterEqual = 121,
    TokenKind_LessEqual = 119,
    TokenKind_LogicalOr = 246,
    TokenKind_LogicalAnd = 74,
    TokenKind_RightShift = 122,
    TokenKind_LeftShift = 118,

    // keywords
    TokenKind_Var = 3,
    TokenKind_Const = 4,
    TokenKind_Type = 5,
    TokenKind_Procedure = 6,
    TokenKind_Begin = 7,
    TokenKind_Call = 8,
    TokenKind_Do = 9,
    TokenKind_Else = 10,
    TokenKind_End = 11,
    TokenKind_If = 12,
    TokenKind_Read = 13,
    TokenKind_Then = 14,
    TokenKind_While = 15,
    TokenKind_Write = 16,
    TokenKind_Repeat = 17,
    TokenKind_Until = 18,

    TokenKind_Identifier = 1,
    TokenKind_Number = 2,

    TokenKind_EOF = 0,
    TokenKind_Body = 255,
};

struct TokenInfo {
    TokenKind Type;
    u32 Location;
};

struct TokenArray {
    u64 Size;
    u64 Capacity;
    u32* TokenLocations;
    char* Src;
    TokenKind* TokenTypes;
};

void token_to_string(char* Dst, const TokenInfo Tok, const char* Src);
void append_tokens(TokenArray* TokArr, __m256i Types, __m256i Locations, int Size, u32 StartIndex);

inline void
better_strcpy(char* Dst, const char* Src)
{
    const char* SrcPtr = Src;

    while (*SrcPtr)
        *Dst++ = *SrcPtr++;
}

__forceinline TokenInfo
create_token(TokenKind Type, u32 Location)
{
    TokenInfo Result = {};

    Result.Type = Type;
    Result.Location = Location;

    return Result;
}

__forceinline void
print_tokens(const TokenArray TokArr) 
{
    for (u32 i = 0; i < TokArr.Size; ++i) {
        TokenInfo Tok = create_token(TokArr.TokenTypes[i], TokArr.TokenLocations[i]);
        char Str[100] = {};

        token_to_string(Str, Tok, TokArr.Src);

        printf("<loc:%d> %s\n", Tok.Location, Str);
    }
}

#define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ? 0 : errno)

__forceinline void
append_token(TokenArray* TokArr, TokenInfo Tok)
{
    TokArr->TokenTypes[TokArr->Size] = Tok.Type;
    TokArr->TokenLocations[TokArr->Size] = Tok.Location;
    ++TokArr->Size;
}

inline TokenArray
create_empty_tokens_array(u64 Cap)
{
    TokenKind* ToksKinds;
    u32* ToksLocs;
    const u64 Alignment = VECTOR_SIZE;
    int Result = posix_memalign((void**) &ToksKinds, Alignment, Cap * sizeof(TokenKind));
    Result |= posix_memalign((void**) &ToksLocs, Alignment, Cap * sizeof(u32));

    if (Result)
        fprintf(stderr, "Error: failed to allocate memory\n");

    TokenArray TokArr = {};
    TokArr.TokenTypes = ToksKinds;
    TokArr.TokenLocations = ToksLocs;
    TokArr.Capacity = Cap;

    return TokArr;
}

__forceinline void
free_token_array(TokenArray TokArr)
{
    _aligned_free(TokArr.TokenLocations);
    _aligned_free(TokArr.TokenTypes);
}

#endif // __TOKEN_H__