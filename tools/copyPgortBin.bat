@echo off
setlocal
setlocal EnableDelayedExpansion

set COPY_PGO_TYPE=%1
set RAZZLE_TOOLS_PATH=%~2
set EXEC_PATH=%~3
set PLATFORM=%~4
set DESTINATION=%~5

set path=%path%;%EXEC_PATH%

set _BuildArch=%PLATFORM%
if "%PLATFORM%"=="win32" (
  set _BuildArch=x86
) else if "%PLATFORM%"=="x64" (
  set _BuildArch=amd64
)

set pgort_Arch=%_BuildArch%
if "%_BuildArch%"=="x86" (
  set pgort_Arch=i386
)

set FILEVER_PATH=%RAZZLE_TOOLS_PATH%\x86\filever.exe
set LINK_PATH=%RAZZLE_TOOLS_PATH%\dev12-fkg\x32\%_BuildArch%\link.exe
echo filever: %FILEVER_PATH%
echo link: %LINK_PATH%

 REM look for matching linker version
set matching_build=
for /F "tokens=2" %%a in ('%FILEVER_PATH% %LINK_PATH% /VA ^| find "ProductVersion"') do set linker_version=%%a
for /F "tokens=5" %%a in ('%FILEVER_PATH% %LINK_PATH% /VA ^| find "FileVersion"') do set linker_build=%%a
set linker_build=%linker_build:(= %
set linker_build=%linker_build:)=%
for /F %%a in ('echo %linker_build%') do set linker_build=%%a
echo Linker Build: %linker_build%
echo Linker Version: %linker_version%

set linker_sharepath=\\vcfs\builds\VS\feature_%linker_build%

for /F %%a in ('dir /b /ad /o-n %linker_sharepath%') do (
  for /F "tokens=2" %%a in ('%FILEVER_PATH% /V %linker_sharepath%\%%a\binaries.%_BuildArch%ret\bin\%pgort_Arch%\link.exe ^| find "ProductVersion"') do set current_linker_version=%%a
  echo Build: %%a, Version: !current_linker_version!
  if [!current_linker_version!] equ [%linker_version%] set matching_build=%linker_sharepath%\%%a
)

echo VC tools build to use: %matching_build%

if [%matching_build%] equ [] (
  set errmsg=Can not find a matching link.exe under %linker_sharepath%.
  echo %errmsg%
  echo %errmsg%>%targetFolder%\pogo.err
  goto:error
)

if "%COPY_PGO_TYPE%"=="bin" (
  goto:copy_bin
)

echo %matching_build%\binaries.%_BuildArch%ret\lib\%pgort_Arch%\pgort.lib
copy /Y %matching_build%\binaries.%_BuildArch%ret\lib\%pgort_Arch%\pgort.lib %DESTINATION%\pgort.lib

if "%COPY_PGO_TYPE%"=="both" (
  goto:copy_bin
)

endlocal
goto:eof

:copy_bin
copy /Y %matching_build%\binaries.%_BuildArch%ret\bin\%pgort_Arch%\pgort120.dll %DESTINATION%\pgort120.dll
copy /Y %matching_build%\binaries.%_BuildArch%ret\bin\%pgort_Arch%\pgort120.pdb %DESTINATION%\pgort120.pdb
copy /Y %matching_build%\binaries.%_BuildArch%ret\bin\%pgort_Arch%\msvcr120.* %DESTINATION%

endlocal
goto:eof

:error
endlocal
exit /b 1


