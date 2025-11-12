@rem currently only compiles with MSVC or LLVM Clang on Windows
@echo off
setlocal

where /q "clang" && (
	set "CC=clang"
) || where /q "cl" && (
	set "CC=cl"
) || (
	echo no suitable compiler found
	goto exit
)

if "%CC%" == "clang" (
	set "FLAGS=-o mvn.exe -std=c99 -Wall -Wextra -Wpedantic"
	set "LFLAGS=-lmsvcrt -nostdlib -Xlinker /NODEFAULTLIB:libcmt"
	if "%~1" equ "debug" (
		set "DFLAGS=-O0 -g -gcodeview"
	) else (
		set "DFLAGS=-Os -DMODE_RELEASE"
	)
) else if "%CC%" == "cl" (
	set "FLAGS=/W3"
	set "LFLAGS=msvcrt.lib /link /NODEFAULTLIB:libcmt /out:mvn.exe"
	if "%~1" equ "debug" (
		set "DFLAGS=/Od /Zi"
	) else (
		set "DFLAGS=/Os /DMODE_RELEASE"
	)
)

@echo on
%CC% mvn.c %FLAGS% %DFLAGS% %LFLAGS%
@echo off

:exit
endlocal
