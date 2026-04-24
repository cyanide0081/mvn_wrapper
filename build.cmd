@rem supports Clang and MSVC
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
    set "LFLAGS=-lmsvcrt -ladvapi32 -lonecore -nostdlib -Wl,/NODEFAULTLIB:libcmt,/INCLUDE:PROGRAM_NAME"
    if "%~1" == "debug" (
        set "DFLAGS=-O0 -g -gcodeview"
    ) else (
        set "DFLAGS=-Os -DNDEBUG"
    )
) else if "%CC%" == "cl" (
    set "FLAGS=/W3"
    set "LFLAGS=msvcrt.lib advapi32.lib onecore.lib /link /NODEFAULTLIB:libcmt /INCLUDE:PROGRAM_NAME /out:mvn.exe"
    if "%~1" == "debug" (
        set "DFLAGS=/Od /Zi"
    ) else (
        set "DFLAGS=/Os /DNDEBUG"
    )
)

@echo on
%CC% src\mvn.c %FLAGS% %DFLAGS% %LFLAGS%
@echo off

:exit
endlocal
