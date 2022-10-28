@echo off

if %PROCESSOR_ARCHITECTURE%==x86 (
Pushd %~dp0\stk-code\build-i686\bin\
supertuxkart.exe
popd
)

if %PROCESSOR_ARCHITECTURE%==AMD64 (
Pushd %~dp0\stk-code\build-x86_64\bin\
supertuxkart.exe
popd
)

if %PROCESSOR_ARCHITECTURE%==ARM64 (
Pushd %~dp0\stk-code\build-aarch64\bin\
supertuxkart.exe
popd
)

if %PROCESSOR_ARCHITECTURE%==ARM (
Pushd %~dp0\stk-code\build-armv7\bin\
supertuxkart.exe
popd
)
