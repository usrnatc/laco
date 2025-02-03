# PL/0 Compiler with Extended Features

[![Project Status: WIP](https://www.repostatus.org/badges/latest/wip.svg)](https://www.repostatus.org/#wip)

A modern implementation of a PL/0 compiler with additional language features
and SIMD-accelerated lexing. This project is currently under active development.

## Features

- **PL/0 Base Language Support** with extended features including
 - `repeat...until` loop construct
 - Additional bitwise and logical operators
 - Improved type system with subrange types
- **SIMD-Accelerated Lexer** for improved performance
- Planned multi-stage optimisation pipeline (L1 - L3 optimisations)
- Windows x64 target support

## System Requirements

**Important Compatability Notes**:
 - Currently only supports **x64 Windows** systems
 - Requires CPU with SIMD instructions (SSE4.1+ recommended)

## Building the Project:

1. Clone the repository: 
```batch
git clone https://github.com/usrnatc/laco.git
```
2. Run the build script:
```batch
build.bat
```
3. The compiled binary can be found in the `build/` directory

## Project Status

- [x] Lexer
- [ ] Parser
- [ ] Semantic Analysis
- [ ] Code Lowering
- [ ] L1 Optimisations
- [ ] L2 Optimisations
- [ ] Code Generation
- [ ] L3 Optimisations
- [ ] Binary Generation

## Language Grammar

```
ASSIGN          ->      ":="
COLON           ->      ":"
SEMICOLON       ->      ";"
RANGE           ->      ".."
LPAREN          ->      "("
RPAREN          ->      ")"
LBRACKET        ->      "["
RBRACKET        ->      "]"
EQUALS          ->      "="
NEQUALS         ->      "!="
GEQUALS         ->      ">="
GREATER         ->      ">"
LEQUALS         ->      "<="
LOGOR           ->      "||"
LOGAND          ->      "&&"
SHR             ->      ">>"
SHL             ->      "<<"
INV             ->      "~"
LESS            ->      "<"
PLUS            ->      "+"
MINUS           ->      "-"
TIMES           ->      "*"
DIVIDE          ->      "/"
KW_BEGIN        ->      "begin"
KW_CALL         ->      "call"
KW_CONST        ->      "const"
KW_DO           ->      "do"
KW_ELSE         ->      "else"
KW_END          ->      "end"
KW_IF           ->      "if"
KW_PROCEDURE    ->      "procedure"
KW_READ         ->      "read"
KW_THEN         ->      "then"
KW_TYPE         ->      "type"
KW_VAR          ->      "var"
KW_WHILE        ->      "while"
KW_WRITE        ->      "write"
KW_REPEAT       ->      "repeat"
KW_UNTIL        ->      "until"
NUMBER          ->      Digit Digit*
IDENTIFIER      ->      Alpha (Alpha | Digit)*

Digit   ->      "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
Alpha   ->      "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | 
                "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" | 
                "u" | "v" | "w" | "x" | "y" | "z" |
                "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | 
                "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | 
                "U" | "V" | "W" | "X" | "Y" | "Z" | "_"

Program             ->      Block EOF
Block               ->      {Declaration} CompoundStatement
Declaration         ->      ConstDefList | TypeDefList | VarDeclList | ProcedureDef
ConstDefList        ->      KW_CONST ConstDef {ConstDef}
ConstDef            ->      IDENTIFIER EQUALS Constant SEMICOLON
Constant            ->      NUMBER | IDENTIFIER | MINUS Constant
TypeDefList         ->      KW_TYPE TypeDef {TypeDef}
TypeDef             ->      IDENTIFIER EQUALS Type SEMICOLON
Type                ->      TypeIdentifier | SubrangeType
TypeIdentifier      ->      IDENTIFIER
SubrangeType        ->      LBRACKET Constant RANGE Constant RBRACKET
VarDeclList         ->      KW_VAR VarDecl {VarDecl}
VarDecl             ->      IDENTIFIER COLON TypeIdentifier SEMICOLON
ProcedureDef        ->      ProcedureHead EQUALS Block SEMICOLON
ProcedureHead       ->      KW_PROCEDURE IDENTIFIER LPAREN FormalParameters RPAREN
FormalParameters    ->
CompoundStatement   ->      KW_BEGIN StatementList KW_END
StatementList       ->      Statement {SEMICOLON Statement}
Statement           ->      Assignment | CallStatement | ReadStatement | WriteStatement | WhileStatement | IfStatement | CompoundStatement | RepeatStatement
Assignment          ->      LValue ASSIGN Condition
CallStatement       ->      KW_CALL IDENTIFIER LPAREN ActualParameters RPAREN
ActualParameters    ->
ReadStatement       ->      KW_READ LValue
WriteStatement      ->      KW_WRITE Exp
WhileStatement      ->      KW_WHILE Condition KW_THEN Statement KW_ELSE Statement
RepeatStatement     ->      KW_REPEAT StatementList KW_UNTIL Condition
Condition           ->      [INV] RelCondition
RelCondition        ->      Exp [RelOp Exp]
RelOp               ->      EQUALS | NEQUALS | LESS | GREATER | LEQUALS | GEQUALS | LOGAND | LOGOR | SHL | SHR
Exp                 ->      [PLUS | MINUS] Term {(PLUS | MINUS) Term}
Term                ->      Factor {(TIMES | DIVIDE) Factor}
Factor              ->      LPAREN Condition RPAREN | NUMBER | LValue
LValue              ->      IDENTIFIER
```
