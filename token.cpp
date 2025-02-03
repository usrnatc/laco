#include <stdio.h>

#include "common.h"
#include "token.h"

void
token_to_string(char* Dst, const TokenInfo Tok, const char* Src)
{
    switch (Tok.Type) {
        // one byte punctuators
        case TokenKind_Exclaim:
            better_strcpy(Dst, "exclaim  !");
            break;

        case TokenKind_OpenParen:
            better_strcpy(Dst, "open_paren  (");
            break;

        case TokenKind_CloseParen:
            better_strcpy(Dst, "close_paren  )");
            break;

        case TokenKind_Star:
            better_strcpy(Dst, "star  *");
            break;

        case TokenKind_Plus:
            better_strcpy(Dst, "plus  +");
            break;

        case TokenKind_Minus:
            better_strcpy(Dst, "minus  -");
            break;

        case TokenKind_Slash:
            better_strcpy(Dst, "slash  /");
            break;

        case TokenKind_Colon:
            better_strcpy(Dst, "colon  :");
            break;

        case TokenKind_Semicolon:
            better_strcpy(Dst, "semicolon  ;");
            break;

        case TokenKind_Less:
            better_strcpy(Dst, "less  <");
            break;

        case TokenKind_Equal:
            better_strcpy(Dst, "equal  =");
            break;

        case TokenKind_Greater:
            better_strcpy(Dst, "greater  >");
            break;

        case TokenKind_OpenSquareBracket:
            better_strcpy(Dst, "open sb  [");
            break;

        case TokenKind_CloseSquareBracket:
            better_strcpy(Dst, "close sb  ]");
            break;

        case TokenKind_Tilde:
            better_strcpy(Dst, "tilde  ~");
            break;

        // two byte punctuators
        case TokenKind_Assign:
            better_strcpy(Dst, "assign  :=");
            break;

        case TokenKind_Range:
            better_strcpy(Dst, "range  ..");
            break;

        case TokenKind_ExclaimEqual:
            better_strcpy(Dst, "exclaim eq  !=");
            break;

        case TokenKind_GreaterEqual:
            better_strcpy(Dst, "greater eq  >=");
            break;

        case TokenKind_LessEqual:
            better_strcpy(Dst, "less eq  <=");
            break;

        case TokenKind_LogicalAnd:
            better_strcpy(Dst, "log and  &&");
            break;

        case TokenKind_LogicalOr:
            better_strcpy(Dst, "log or  ||");
            break;

        case TokenKind_LeftShift:
            better_strcpy(Dst, "left shift  <<");
            break;

        case TokenKind_RightShift:
            better_strcpy(Dst, "right shift  >>");
            break;

        // keywords
        case TokenKind_Var:
            better_strcpy(Dst, "var  var");
            break;

        case TokenKind_Repeat:
            better_strcpy(Dst, "repeat  repeat");
            break;

        case TokenKind_Until:
            better_strcpy(Dst, "until  until");
            break;

        case TokenKind_Const:
            better_strcpy(Dst, "const  const");
            break;

        case TokenKind_Type:
            better_strcpy(Dst, "type  type");
            break;

        case TokenKind_Procedure:
            better_strcpy(Dst, "procedure  procedure");
            break;

        case TokenKind_Begin:
            better_strcpy(Dst, "begin  begin");
            break;

        case TokenKind_Call:
            better_strcpy(Dst, "call  call");
            break;

        case TokenKind_Do:
            better_strcpy(Dst, "do  do");
            break;

        case TokenKind_Else:
            better_strcpy(Dst, "else  else");
            break;

        case TokenKind_End:
            better_strcpy(Dst, "end  end");
            break;

        case TokenKind_If:
            better_strcpy(Dst, "if  if");
            break;

        case TokenKind_Read:
            better_strcpy(Dst, "read  read");
            break;

        case TokenKind_Then:
            better_strcpy(Dst, "then  then");
            break;

        case TokenKind_While:
            better_strcpy(Dst, "while  while");
            break;

        case TokenKind_Write:
            better_strcpy(Dst, "write  write");
            break;

        case TokenKind_Identifier:
            better_strcpy(Dst, "identifier  ");
            strcat(Dst, (Src + Tok.Location));
            break;

        case TokenKind_Number:
            better_strcpy(Dst, "numeric_constant  ");
            strcat(Dst, (Src + Tok.Location));
            break;

        case TokenKind_EOF:
            better_strcpy(Dst, "eof  ");
            break;

        default:
            better_strcpy(Dst, "INVALID TOKEN TYPE  ");
    }
}

void
append_tokens(TokenArray* TokArr, __m256i Types, __m256i Locs, int Size, u32 StartIndex)
{
    _mm256_storeu_si256((__m256i*) (TokArr->TokenTypes + TokArr->Size), Types);
    u64* Locations64 = (u64*) &Locs;
    __m256i start_idx_vec = _mm256_set1_epi32(StartIndex);

    for (u8 i = 0; i < 4 && Size > 0; ++i) {
        __m256i locs_expanded = _mm256_cvtepu8_epi32(_mm_set_epi64x(0, *(Locations64 + i)));
        locs_expanded = _mm256_add_epi32(locs_expanded, start_idx_vec);
        _mm256_storeu_si256((__m256i *) (TokArr->TokenLocations + TokArr->Size), locs_expanded);
        TokArr->Size += 8;
        Size -= 8;
    }

    TokArr->Size += Size;
}