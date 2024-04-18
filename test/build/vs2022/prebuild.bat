@echo off
setlocal enabledelayedexpansion

set dep_base=..\..\deps\
set bin_base=..\..\bin\

rmdir /Q /S %bin_base%
rmdir /Q /S %dep_base%

mkdir %dep_base%include
mkdir %dep_base%lib

set src_base=..\..\..\..\

set lib_name=lccl
if exist %src_base%%lib_name% (
    mkdir %dep_base%include\%lib_name%
    xcopy %src_base%%lib_name%\include %dep_base%include\%lib_name% /S /Y /C
    xcopy %src_base%%lib_name%\bin %dep_base%lib /S /Y /C
) else (
    echo please put https://github.com/Lujiang0111/%lib_name% in %src_base%
    goto :error
)

endlocal
exit /b 0

:error
endlocal
exit /b 1