@echo off
setlocal enabledelayedexpansion

set root_base=..\..\..\..\..\
set bin_base=..\..\..\bin\
set trd_base=..\..\..\3rd\

if not exist %bin_base% (
    echo %bin_base% not exist
    goto :error
)

set lib_name=lccl
if exist %root_base%%lib_name% (
    xcopy %root_base%%lib_name%\bin %bin_base% /S /Y /C
    if exist %root_base%%lib_name%\3rd\ (
        for /d %%d in ("%root_base%%lib_name%\3rd\*") do (
            xcopy %%d\lib %bin_base% /S /Y /C
        )
    )
) else if exist %trd_base%%lib_name% (
    xcopy %trd_base%%lib_name%\lib %bin_base% /S /Y /C
) else (
    echo please put https://github.com/Lujiang0111/%lib_name% sourece in %root_base%%lib_name% or lib in %trd_base%%lib_name%
    goto :error
)

endlocal
exit /b 0

:error
endlocal
exit /b 1