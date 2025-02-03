#if !defined(__LEX_H__)
#define __LEX_H__

#include "common.h"
#include "source_buf.h"
#include "token.h"

TokenArray lex(char* Src, u64 Size);
u8 Hash(u64 Val);
void populate_keyword_lut(u16* LUT);
__m256i v_classification_one_byte(__m256i Input);
void find_keywords(TokenArray* TokArr, u16* LUT);
void _mm256_pext(__m256i* Vec, __m256i Mask, int* Size);
__m256i non_zero_mask(const __m256i Vec);
void find_token_indexes(__m256i* TokTags, __m256i* TokIdx, int* Size);
void replace_whitespace(__m256i* Vec);
void replace_token_body(__m256i* Vec);
b8 is_empty(__m256i Vec);
__m256i run_sublexers(__m256i* CurrVec, __m256i* NextVec, const __m256i SrcCurrVec, u8 LastChar, b8* ChContinue, b8* EscContinue, b8* StrContinue, b8* LnCommContinue, b8* BlockCommContinue);
TokenArray lex_file(SourceBuf* SrcBuf);
__m256i _mm256_cmpistrm_any(__m128i Match, __m256i Vec);
__m256i _mm256_cmpistrm_range(__m128i Ranges, __m256i Vec, int NumRanges);
__m256i numeric_periods_mask(__m256i CurrVec, __m256i NextVec, u8 LastChar);
void one_byte_punct_sub_lex(__m256i* CurrVec, __m256i NextVec, __m256i* Tags, u8 LastChar);
__m256i look_ahead_one(__m256i CurrVec, __m256i NextVec);
__m256i look_ahead_two(__m256i CurrVec, __m256i NextVec);
__m256i get_mask(const uint32_t Mask);
void remove_prefix_64(__m256i* Vec, u64 Prefix);
void two_byte_punct_sub_lex(__m256i* CurrVec, __m256i* NextVec, __m256i* Tags);
__m256i alpha_mask(__m256i Vec);
void identifiers_sub_lex(__m256i CurrVec, __m256i* Tags, b8 LastEmpty);
__m256i num_mask(__m256i Vec);
void numeric_const_sub_lex(const __m256i CurrVec, __m256i* Tags, const b8 LastEmpty);
void line_comments_sub_lex(__m256i* CurrVec, __m256i NextVec, b8* LnCommContinue);
__m256i load_vector(const char* Pos);

#endif // __LEX_H__