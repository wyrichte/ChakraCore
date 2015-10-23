@echo off
setlocal

set RAZZLE_TOOLS_PATH=%~1
set PLATFORM=%~2
set LIB_DESTINATION=%~3
set BIN_DESTINATION=%~4

set _BuildArch=%PLATFORM%
if "%PLATFORM%"=="win32" (
  set _BuildArch=x86
) else if "%PLATFORM%"=="x64" (
  set _BuildArch=amd64
)

copy /Y %RAZZLE_TOOLS_PATH%\%_BuildArch%\pgort.lib %LIB_DESTINATION%\pgort.lib
copy /Y %RAZZLE_TOOLS_PATH%\%_BuildArch%\pgort120.dll %BIN_DESTINATION%\pgort120.dll
copy /Y %RAZZLE_TOOLS_PATH%\%_BuildArch%\pgort120.pdb %BIN_DESTINATION%\pgort120.pdb
copy /Y %RAZZLE_TOOLS_PATH%\%_BuildArch%\msvcr120.* %BIN_DESTINATION%

endlocal
goto:eof
