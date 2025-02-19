@echo off
setlocal enabledelayedexpansion

set root_base=..\..\..\..\..\
set bin_base=..\..\..\bin\
set trd_base=..\..\..\3rd\
set dep_base=dep\

rmdir /Q /S %bin_base%
rmdir /Q /S %dep_base%

mkdir %dep_base%include
mkdir %dep_base%lib

set lib_name=lccl
if exist %root_base%%lib_name% (
    mkdir %dep_base%include\%lib_name%
    xcopy %root_base%%lib_name%\include %dep_base%include\%lib_name% /S /Y /C
    xcopy %root_base%%lib_name%\bin %dep_base%lib /S /Y /C
) else if exist %trd_base%%lib_name% (
    mkdir %dep_base%include\%lib_name%
    xcopy %trd_base%%lib_name%\include %dep_base%include\%lib_name% /S /Y /C
    xcopy %trd_base%%lib_name%\lib %dep_base%lib /S /Y /C
) else (
    echo please put https://github.com/Lujiang0111/%lib_name% sourece in %root_base%%lib_name% or lib in %trd_base%%lib_name%
    goto :error
)

endlocal
exit /b 0

:error
endlocal
exit /b 1