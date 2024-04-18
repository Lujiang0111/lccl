@echo off
setlocal enabledelayedexpansion

set bin_base=..\..\bin\

if not exist %bin_base% (
    echo %bin_base% not exist
    goto :error
)

set src_base=..\..\..\..\

set lib_name=lccl
if exist %src_base%%lib_name% (
    xcopy %src_base%%lib_name%\bin %bin_base% /S /Y /C
) else (
    echo please put https://github.com/Lujiang0111/%lib_name% in %src_base%
    goto :error
)

endlocal
exit /b 0

:error
endlocal
exit /b 1