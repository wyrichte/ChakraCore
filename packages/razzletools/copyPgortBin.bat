@echo off
setlocal
setlocal EnableDelayedExpansion

set RAZZLE_TOOLS_PATH=%~1
set PLATFORM=%~2
set DESTINATION=%~3

set FILEVER_PATH=%RAZZLE_TOOLS_PATH%\x86\filever.exe
set LINK_PATH=%RAZZLE_TOOLS_PATH%\vc\x32\x86\link.exe
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

 rem set linker_sharepath=\\vcfs\builds\VS\feature_WinCCompLKG
set linker_sharepath=\\vcgnt-24-64\Test\Dev14
 rem set linker_sharepath=\\vcfs\drops\LKG\18.1

for /F %%a in ('dir /b /ad /o-n %linker_sharepath%') do (
  set shareroot=%linker_sharepath%\%%a
  for /F %%a in ('dir /b /ad /o-n !shareroot!') do (
    set buildFolder=!shareroot!\%%a
    echo !buildFolder!
    for /F "tokens=2" %%a in ('%FILEVER_PATH% /V !buildFolder!\bin\x32\link.exe ^| find "ProductVersion"') do set current_linker_version=%%a
    echo Build: %%a, Version: !current_linker_version!
    if [!current_linker_version!] equ [%linker_version%] set matching_build=!buildFolder!
  )
)

if [%matching_build%] equ [] (
  set errmsg=Can not find a matching link.exe under %linker_sharepath%.
  echo %errmsg%
  echo %errmsg%>%DESTINATION%\pogo.err
  goto:error
)
echo VC tools build to use: %matching_build%

for %%p in (%PLATFORM%) do (
  set arch=%%p
  set dst=%DESTINATION%\!arch!

  if not exist !dst! (
    echo creating directory !dst!
    md !dst!
  )

  if /I "!arch!"=="x86" (
    set arch=x32
  ) else if /I "!arch!"=="amd64" (
    set arch=x64
  ) else if /I "!arch!"=="arm" (
    set arch=a32
  ) else if /I "!arch!"=="arm64" (
    set arch=a64
  )
  set libArch=!arch!

  if /I "!libArch!"=="x32" (
    set libArch=x86
  ) else if /I "!libArch!"=="x64" (
    set libArch=amd64
  ) else if /I "!libArch!"=="a32" (
    set libArch=arm
  ) else if /I "!libArch!"=="a64" (
    set libArch=arm64
  )

  set archBinFolder=%matching_build%\bin\!arch!
  set archLibFolder=%matching_build%\lib.DO_NOT_REDIST\!libArch!

  del !dst!\pgort.lib
  echo !archLibFolder!\pgort.lib =^> !dst!\pgort.lib
  copy /Y !archLibFolder!\pgort.lib !dst!\pgort.lib

  del !dst!\pgort*.dll
  echo !archBinFolder!\pgort*.dll =^> !dst!\pgort*.dll
  copy /Y !archBinFolder!\pgort*.dll !dst!\pgort*.dll

  echo done for !arch!
)

endlocal
goto:eof

:error
endlocal
exit /b 1


