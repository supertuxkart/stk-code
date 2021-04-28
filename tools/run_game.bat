@echo off
reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT

if %OS%==32BIT (
Pushd %~dp0\stk-code\build-mingw\bin\
supertuxkart.exe
popd
)
if %OS%==64BIT (
Pushd %~dp0\stk-code\build-mingw64\bin\
supertuxkart.exe
popd
)
