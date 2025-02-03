#include <stdio.h>
#include <string.h>
#include <immintrin.h>

#include "common.h"
#include "source_buf.h"
#include "token.h"
#include "lex.h"

TokenArray 
lex(char* Src, u64 SrcSize)
{
    TokenArray Result = create_empty_tokens_array(SrcSize + 4);

    Result.Src = Src;

    char LastChar = 0;
    b8 ChContinue;
    b8 StrContinue;
    b8 EscContinue;
    b8 LnCommContinue;
    b8 BlockCommContinue;
    __m256i CurrVec = load_vector(Src);
    __m256i SrcCurrVec = load_vector(Src);

    for (u64 i = 0; i < SrcSize; i += VECTOR_SIZE) {
        __m256i NextVec = load_vector(Src + i + VECTOR_SIZE);
        const __m256i SrcNextVec = load_vector(Src + i + VECTOR_SIZE);
        __m256i Tags = run_sublexers(&CurrVec, &NextVec, SrcCurrVec, LastChar, &ChContinue, &EscContinue, &StrContinue, &LnCommContinue, &BlockCommContinue);

        int Size;
        __m256i Indexes;

        find_token_indexes(&Tags, &Indexes, &Size);
        append_tokens(&Result, Tags, Indexes, Size, i);
        _mm256_storeu_si256((__m256i*) (Src + i), CurrVec);
        LastChar = Src[i + 31];
        CurrVec = NextVec;
        SrcCurrVec = SrcNextVec;
    }

    u16 LUT[256] = {};

    populate_keyword_lut(LUT);
    find_keywords(&Result, LUT);

    return Result;
}

__forceinline u8 
Hash(u64 Val)
{
    return ((((Val >> 32) ^ Val) & 0xFFFFFFFF) * (u64) 3523216747) >> 32;
}

void 
populate_keyword_lut(u16* LUT)
{
    const char* keywords[] = {
        "", "var", "const", "type", "procedure", "begin",
        "call", "do", "else", "end", "if", "read", "then",
        "while", "write", "repeat", "until"
    };

    int NumKeywords = ARRAY_COUNT(keywords);

    for (int i = 1; i < NumKeywords; ++i) {
        char keywords_copy[9] = {};
        strncpy(keywords_copy, keywords[i], 8);
        u64 Val = *((u64*) keywords_copy);
        u8 HashVal = Hash(Val);

        LUT[HashVal] = i;
    }
}

__m256i 
single_character_classification(__m256i Input)
{
    __m256i LowerNibbleMask = _mm256_set_epi8(
        0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
        0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
        0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
        0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F
    );
    __m256i LUT1 = _mm256_set_epi8(
        3,  15,  15,  11,  15,  3,  1,  1,
        0,  1,  1,  0,  0,  0,  1,  0,
        3,  15,  15,  11,  15,  3,  1,  1,
        0,  1,  1,  0,  0,  0,  1,  0
    );
    __m256i Mask1 = _mm256_shuffle_epi8(LUT1, _mm256_and_si256(LowerNibbleMask, Input));
    Input = _mm256_srli_epi32 (Input, 4);
    __m256i LUT2 = _mm256_set_epi8(
        0,  0,  0,  0,  0,  0,  0,  0,
        8,  0,  4,  0,  2,  1,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        8,  0,  4,  0,  2,  1,  0,  0
    );
    __m256i Mask2 = _mm256_shuffle_epi8(LUT2, _mm256_and_si256(LowerNibbleMask, Input));
    __m256i Mask = _mm256_and_si256(Mask1, Mask2);
    __m256i Cmp = _mm256_cmpeq_epi8(Mask, _mm256_setzero_si256());

    return _mm256_xor_si256(Cmp, _mm256_set1_epi8(-1));
}

void 
find_keywords(TokenArray* TokArr, u16* LUT)
{
    const char* keywords[] = {
        "", "var", "const", "type", "procedure", "begin",
        "call", "do", "else", "end", "if", "read", "then",
        "while", "write", "repeat", "until"
    };

    const TokenKind keyword_kinds[] = {
        (TokenKind) 0, TokenKind_Var, TokenKind_Const, TokenKind_Type,
        TokenKind_Procedure, TokenKind_Begin, TokenKind_Call,
        TokenKind_Do, TokenKind_Else, TokenKind_End,
        TokenKind_If, TokenKind_Read, TokenKind_Then, TokenKind_While,
        TokenKind_Write, TokenKind_Repeat, TokenKind_Until
    };

    for (int i = 0; i < TokArr->Size; i += VECTOR_SIZE) {
        const __m256i CurrVec = load_vector((const char*) TokArr->TokenTypes + i);
        const __m256i Mask = _mm256_cmpeq_epi8(CurrVec, _mm256_set1_epi8(TokenKind_Identifier));
        int Size;
        __declspec(align(32)) u8 TokenIndexes[] = {
            0, 1, 2, 3, 4, 5, 6, 7,
            8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29, 30, 31
        };

        _mm256_pext((__m256i*) TokenIndexes, Mask, &Size);

        for (int j = 0; j < Size; ++j) {
            int pos = i + TokenIndexes[j];
            char str[9] = {};

            strncpy(str, TokArr->Src + TokArr->TokenLocations[pos], 8);

            const u64 Val = *(u64*) str;
            const u8 HashVal = Hash(Val);
            const u8 KeywordPos = LUT[HashVal];
            b8 are_equal = !strcmp(TokArr->Src + TokArr->TokenLocations[pos], keywords[KeywordPos]);
            TokenKind keyword_type = keyword_kinds[KeywordPos];
            TokArr->TokenTypes[pos] = (TokenKind) (TokenKind_Identifier ^ ((keyword_type ^ TokenKind_Identifier) & -!!are_equal));
        }
    }
}

void 
_mm256_pext(__m256i* Vec, __m256i Mask, int* Size)
{
    const u64* Vec64 = (u64*) Vec;
    const u64* Mask64 = (u64*) &Mask;

    *Size = 0;

    for (int i = 0; i < 4; ++i) {
        const u64 Tmp = _pext_u64(Vec64[i], Mask64[i]);
        *(u64*) ((u8*) Vec + *Size) = Tmp;
        *Size += _mm_popcnt_u64(Mask64[i]) >> 3;
    }
}

__m256i 
non_zero_mask(const __m256i Vec)
{
    __m256i Mask = _mm256_cmpeq_epi8(Vec, _mm256_setzero_si256());
    Mask = _mm256_xor_si256(Mask, _mm256_set1_epi32(-1));

    return Mask;
}

void 
find_token_indexes(__m256i* TokTags, __m256i* TokIdx, int* Size)
{
    __m256i Mask = non_zero_mask(*TokTags);

    *TokIdx = _mm256_setr_epi8(
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31
    );

    _mm256_pext(TokIdx, Mask, Size);
    _mm256_pext(TokTags, Mask, Size);
}

void 
replace_whitespace(__m256i* Vec)
{
    __m256i white_spaces_mask = _mm256_cmpeq_epi8(*Vec, _mm256_set1_epi8(' '));
    white_spaces_mask = _mm256_or_si256(white_spaces_mask, _mm256_cmpeq_epi8(*Vec, _mm256_set1_epi8('\n')));
    white_spaces_mask = _mm256_or_si256(white_spaces_mask, _mm256_cmpeq_epi8(*Vec, _mm256_set1_epi8('\t')));
    white_spaces_mask = _mm256_or_si256(white_spaces_mask, _mm256_cmpeq_epi8(*Vec, _mm256_set1_epi8('\r')));
    *Vec = _mm256_blendv_epi8(*Vec, _mm256_setzero_si256(), white_spaces_mask);
}

void 
replace_token_body(__m256i *Vec) {
    __m256i Mask = _mm256_cmpeq_epi8(*Vec, _mm256_set1_epi8(TokenKind_Body));
    *Vec = _mm256_blendv_epi8(*Vec, _mm256_setzero_si256(), Mask);
}

b8 
is_empty(__m256i Vec)
{
    __m256i C = _mm256_cmpeq_epi8(Vec, _mm256_setzero_si256());
    return _mm256_movemask_epi8(C) == UINT32_MAX;               // NOTE(nathan): relying on x86 build
}

__m256i 
run_sublexers(__m256i* CurrVec, __m256i* NextVec, const __m256i SrcCurrVec, u8 LastChar, b8* ChContinue, b8* EscContinue, b8* StrContinue, b8* LnCommContinue, b8* BlockCommContinue)
{
    __m256i Tags = _mm256_setzero_si256();
    lex_inline_comments(CurrVec, *NextVec, LnCommContinue);

    if (is_empty(*CurrVec))
        return Tags;

    lex_two_character_symbol(CurrVec, NextVec, &Tags);
    lex_single_character_symbol(CurrVec, *NextVec, &Tags, LastChar);
    replace_whitespace(CurrVec);
    lex_identifiers(*CurrVec, &Tags, LastChar == 0);
    lex_numeric_constants(*CurrVec, &Tags, LastChar == 0);
    replace_token_body(&Tags);

    return Tags;
}

TokenArray 
lex_file(SourceBuf* SrcBuf)
{
    TokenArray TokArr = lex((char*) SrcBuf->Data, SrcBuf->Length);
    append_token(&TokArr, create_token(TokenKind_EOF, SrcBuf->Length));
    return TokArr;
}

__m256i 
_mm256_cmpistrm_any(__m128i Match, __m256i Vec)
{
    __m128i LowVec = _mm256_extractf128_si256(Vec, 0);
    __m128i HighVec = _mm256_extractf128_si256(Vec, 1);
    __m128i LowOutsideRangeMask = _mm_cmpestrm(Match, 16, LowVec, 16, (1 << 6));
    __m128i HighOutsideRangeMask = _mm_cmpestrm(Match, 16, HighVec, 16, (1 << 6));
    return _mm256_set_m128i(HighOutsideRangeMask, LowOutsideRangeMask);
}

__m256i 
_mm256_cmpistrm_range(__m128i Ranges, __m256i Vec, int NumRanges)
{
    __m128i LowVec = _mm256_extractf128_si256(Vec, 0);
    __m128i HighVec = _mm256_extractf128_si256(Vec, 1);
    __m128i LowOutsideRangeMask = _mm_cmpestrm(Ranges, NumRanges, LowVec, 16, (1 << 6) | (1 << 2));
    __m128i HighOutsideRangeMask = _mm_cmpestrm(Ranges, NumRanges, HighVec, 16, (1 << 6) | (1 << 2));
    return _mm256_set_m128i(HighOutsideRangeMask, LowOutsideRangeMask);
}

__m256i 
numeric_periods_mask(__m256i CurrVec, __m256i NextVec, u8 LastChar)
{
    u64 IsPeriod = _mm256_movemask_epi8(_mm256_cmpeq_epi8(CurrVec, _mm256_set1_epi8('.')));
    u64 HasNumAfter = _mm256_movemask_epi8(num_mask(look_ahead_one(CurrVec, NextVec)));
    u64 HasNumBefore = _mm256_movemask_epi8(num_mask(CurrVec));
    HasNumBefore = (HasNumBefore << 1) | (LastChar >= 0 && LastChar <= 9);
    return get_mask(IsPeriod & (HasNumBefore | HasNumAfter));
}

void 
lex_single_character_symbol(__m256i* CurrVec, __m256i NextVec, __m256i* Tags, u8 LastChar)
{
    __m256i Mask = single_character_classification(*CurrVec);
    Mask = _mm256_xor_si256(Mask, numeric_periods_mask(*CurrVec, NextVec, LastChar));
    *Tags = _mm256_blendv_epi8(*Tags, *CurrVec, Mask);
    *CurrVec = _mm256_blendv_epi8(*CurrVec, _mm256_setzero_si256(), Mask);
}

__m256i 
look_ahead_one(__m256i CurrVec, __m256i NextVec)
{
    return _mm256_alignr_epi8(_mm256_permute2x128_si256(NextVec, CurrVec, 3), CurrVec, 1);
}

__m256i 
look_ahead_two(__m256i CurrVec, __m256i NextVec)
{
    return _mm256_alignr_epi8(_mm256_permute2x128_si256(NextVec, CurrVec, 3), CurrVec, 2);
}

__m256i 
get_mask(const uint32_t Mask)
{
    __m256i VMask = _mm256_set1_epi32(Mask);
    const __m256i Shuffle = _mm256_setr_epi64x(0x0000000000000000, 0x0101010101010101, 0x0202020202020202, 0x0303030303030303);
    VMask = _mm256_shuffle_epi8(VMask, Shuffle);
    const __m256i BitMask = _mm256_set1_epi64x(0x7FBFDFEFF7FBFDFE);
    VMask = _mm256_or_si256(VMask, BitMask);
    return _mm256_cmpeq_epi8(VMask, _mm256_set1_epi64x(-1));
}

void remove_prefix_64(__m256i *Vec, u64 Prefix) {
    *Vec = _mm256_blendv_epi8(_mm256_set1_epi8(0), *Vec, _mm256_setr_epi64x(0xFFFFFFFFFFFFFFFF - Prefix, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,  0xFFFFFFFFFFFFFFFF));
}

void 
lex_two_character_symbol(__m256i* CurrVec, __m256i* NextVec, __m256i* Tags)
{
    const __m256i Shifted1 = look_ahead_one(*CurrVec, *NextVec);
    const char *FirstBytes = ":.!><|&";
    uint32_t FirstMasks[7];

    for (int i = 0; i < 7; ++i)
        FirstMasks[i] = _mm256_movemask_epi8(_mm256_cmpeq_epi8(*CurrVec, _mm256_set1_epi8(FirstBytes[i])));

    const char *SecondBytes = "=.|&><";
    uint32_t SecondMasks[6];

    for (int i = 0; i < 6; ++i)
        SecondMasks[i] = _mm256_movemask_epi8(_mm256_cmpeq_epi8(Shifted1,_mm256_set1_epi8(SecondBytes[i])));

    const uint8_t PunctData[9][2] = {
        {3, 0},
        {4, 0},
        {2, 0},
        {1, 1},
        {0, 0},
        {5, 2},
        {6, 3},
        {3, 4},
        {4, 5}
    };

    uint32_t Mask = 0;

    for (int i = 0; i < 9; ++i) {
        const uint8_t X = PunctData[i][0];
        const uint8_t Y = PunctData[i][1];
        Mask = Mask | (FirstMasks[X] & SecondMasks[Y]);
    }

    Mask = Mask ^ (Mask & (Mask << 1) & (Mask >> 1));
    Mask = Mask ^ (Mask & (Mask << 1));
    __m256i TokKinds = _mm256_sub_epi8(_mm256_adds_epu8(*CurrVec, Shifted1), _mm256_set1_epi8(2));
    *Tags = _mm256_blendv_epi8(*Tags, TokKinds, get_mask(Mask));
    *Tags = _mm256_blendv_epi8(*Tags, _mm256_setzero_si256(), get_mask(Mask << 1));
    *CurrVec = _mm256_blendv_epi8(*CurrVec, _mm256_setzero_si256(), get_mask(Mask | Mask << 1));
    uint8_t Carry = ((Mask & (1 << 31)) >> 31) * 0xFF;
    remove_prefix_64(NextVec, Carry);
}

__m256i 
alpha_mask(__m256i Vec)
{
    __m128i Ranges = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'z', 'a', 'Z', 'A');
    return _mm256_cmpistrm_range(Ranges, Vec, 4);
}

void 
lex_identifiers(__m256i CurrVec, __m256i* Tags, b8 LastEmpty)
{
    __m256i IsAlpha = alpha_mask(CurrVec);
    __m256i IsUnderscore = _mm256_cmpeq_epi8(CurrVec, _mm256_set1_epi8('_'));
    __m256i IdentStartMask = _mm256_or_si256(IsAlpha, IsUnderscore);
    u32 HasWhitespaceBefore = _mm256_movemask_epi8(_mm256_cmpeq_epi8(CurrVec, _mm256_setzero_si256()));
    HasWhitespaceBefore = (HasWhitespaceBefore << 1) | LastEmpty;
    IdentStartMask = _mm256_and_si256(IdentStartMask, get_mask(HasWhitespaceBefore));
    *Tags = _mm256_blendv_epi8(*Tags, _mm256_set1_epi8(TokenKind_Identifier), IdentStartMask);
}

__m256i 
num_mask(__m256i Vec)
{
    __m128i Ranges = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '9', '0');
    return _mm256_cmpistrm_range(Ranges, Vec, 2);
}

void 
lex_numeric_constants(const __m256i CurrVec, __m256i* Tags, const b8 LastEmpty)
{
    __m256i NumStartMask = num_mask(CurrVec);
    uint32_t HasWhitespaceBefore = _mm256_movemask_epi8(_mm256_cmpeq_epi8(CurrVec, _mm256_setzero_si256()));
    HasWhitespaceBefore = (HasWhitespaceBefore << 1) | LastEmpty;
    __m256i IsNumPeriod = _mm256_cmpeq_epi8(CurrVec, _mm256_set1_epi8('.'));
    NumStartMask = _mm256_and_si256(NumStartMask, get_mask(HasWhitespaceBefore));
    NumStartMask = _mm256_or_si256(NumStartMask, _mm256_and_si256(IsNumPeriod, get_mask(HasWhitespaceBefore)));
    *Tags = _mm256_blendv_epi8(*Tags, _mm256_set1_epi8(TokenKind_Number), NumStartMask);
}

void 
lex_inline_comments(__m256i* CurrVec, __m256i NextVec, b8* LnCommContinue)
{
    const __m256i Shifted1 = look_ahead_one(*CurrVec, NextVec);
    __m256i IsSlash = _mm256_cmpeq_epi8(*CurrVec, _mm256_set1_epi8('/'));
    __m256i HasSlashAfter = _mm256_cmpeq_epi8(Shifted1, _mm256_set1_epi8('/'));
    __m256i CommentStart = _mm256_and_si256(IsSlash, HasSlashAfter);
    __m256i CommentEnd = _mm256_cmpeq_epi8(*CurrVec, _mm256_set1_epi8('\n'));
    __m256i Region;
    uint32_t Region32;
    bool Ok;

    do {
        uint32_t RegionMask = _mm256_movemask_epi8(_mm256_or_si256(CommentStart, CommentEnd));
        RegionMask ^= *LnCommContinue;
        Region32 = _mm_cvtsi128_si32(_mm_clmulepi64_si128(_mm_set_epi32(0, 0, 0, RegionMask), _mm_set1_epi8(-1), 0));
        __m256i OutsideRegion = get_mask(~Region32);
        Region = get_mask(Region32);
        __m256i MistakesEnd = _mm256_and_si256(Region, CommentEnd);
        CommentEnd = _mm256_xor_si256(CommentEnd, MistakesEnd);
        Ok = is_empty(MistakesEnd);

        if (Ok) {
            __m256i MistakesStart = _mm256_and_si256(OutsideRegion, CommentStart);
            CommentStart = _mm256_xor_si256(CommentStart, MistakesStart);
            Ok = is_empty(MistakesStart);
        }
    } while (!Ok);

    *LnCommContinue = (Region32 >> 31) & 1;
    *CurrVec = _mm256_blendv_epi8(*CurrVec, _mm256_setzero_si256(), Region);
}


__m256i 
load_vector(const char* Pos)
{
    return _mm256_loadu_si256((__m256i*) Pos);
}
