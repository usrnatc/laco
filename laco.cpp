#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "source_buf.h"
#include "lex.h"

internal inline void
usage(void)
{
    fprintf(stderr, "Usage: laco <source-file>\n");
    exit(-1);
}

int
main(int argc, char** argv)
{
    if (argc != 2)
        usage();

    SourceBuf SourceBuffer = MakeSourceBufFromFile(argv[1]);

    if (!SourceBuffer.Data || !SourceBuffer.Length)
        return -2;

    printf("***************\n");
    printf("** %s\n", SourceBuffer.Name);
    printf("** %llu bytes\n", SourceBuffer.Length);
    printf("***************\n");

    clock_t LexStart = clock();
    TokenArray TokArr = lex_file(&SourceBuffer);
    clock_t LexEnd = clock();
    clock_t LexTotal = LexEnd - LexStart;
    f64 LexCPUTimeUsed = ((f64) (LexTotal)) / CLOCKS_PER_SEC * 1000;

    print_tokens(TokArr);

    printf("\n**********************************************************\n");
    printf("** STATISTICS\n");
    printf("**********************************************************\n");
    printf("** Lexing\n");
    printf("**********************************************************\n");
    printf("** TotalTokens     :  %llu tokens\n", TokArr.Size);
    printf("** TotalTime       : ~%ld ms\n", LexTotal);
    printf("** CPUTime         : ~%lf ms\n", LexCPUTimeUsed);
    printf("**********************************************************\n\n");

    // tokens = lex source
    // parsetree = parse tokens
    // ast = semantic analyse parsetree
    // LIR = generate ir from ast
    // l1 optimise
    // l2 optimise
    // generate code
    // l3 optimise
    // generate machine code
    // output file

    DestroySourceBuf(&SourceBuffer);

    return 0;
}