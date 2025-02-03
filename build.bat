@echo off

if not exist .\build mkdir .\build
pushd .\build

set BinaryName=laco.exe
set CompilerOptions= /DWIN32 /O2 /Oi /Ot /GL /Qpar /fp:fast /fp:except- /Ob3 /Oy- /GS- /guard:cf- /Qvec-report:2 /arch:AVX512 /nologo /FC /WX /W4 /MT /GR- /Gm- /GF /EHa- /guard:ehcont- /D_CRT_SECURE_NO_WARNINGS /diagnostics:caret /Zi /MP31 /DNDEBUG /wd4100 /wd4244
set LinkerOptions=/LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO /time /MACHINE:X64

del *.pdb > NUL 2> NUL

cl %CompilerOptions% /Fe:%BinaryName% ..\*.cpp -link %LinkerOptions%

set LastError=%ERRORLEVEL%
popd

if not %LastError% == 0 goto :end

.\build\laco.exe test_programs\test-base-proc-C-fact.pl0

:end
