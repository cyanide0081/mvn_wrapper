@rem currently only compiles with LLVM Clang on Windows
@echo off
setlocal
if "%~1" equ "debug" (
	set "_FLAGS=-O0 -g -gcodeview
) else (
	set "_FLAGS=-Os -g0 -DMODE_RELEASE -lmsvcrt -Xlinker /NODEFAULTLIB:libcmt
)

@echo on
clang -o mvn.exe mvn.c -std=c99 -Wall -Wextra -pedantic %_FLAGS%
@echo off

endlocal
