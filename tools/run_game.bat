@echo off
reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT

if %OS%==32BIT (
Pushd %~dp0\stk-code\build-i686\bin\
supertuxkart.exe
popd
)
if %OS%==64BIT (
Pushd %~dp0\stk-code\build-x86_64\bin\
supertuxkart.exe
popd
)
