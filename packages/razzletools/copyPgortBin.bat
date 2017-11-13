REM @echo off
setlocal
setlocal EnableDelayedExpansion

set RAZZLE_TOOLS_PATH=%~1
set PLATFORM=%~2
set DESTINATION=%~3

set FILEVER_PATH=%RAZZLE_TOOLS_PATH%\x86\filever.exe
set LINK_PATH=%OSBuildToolsRoot%\vc\HostX86\x86\link.exe
echo filever: %FILEVER_PATH%
echo link: %LINK_PATH%

REM look for matching linker version
set matching_build=
for /F "tokens=2" %%a in ('%FILEVER_PATH% %LINK_PATH% /VA ^| find "ProductVersion"') do set linker_version=%%a
for /F "tokens=5" %%a in ('%FILEVER_PATH% %LINK_PATH% /VA ^| find "FileVersion"') do set linker_build=%%a
set linker_build=%linker_build:(= %
set linker_build=%linker_build:)=%
for /F %%a in ('echo %linker_build%') do set linker_build=%%a
for /F "tokens=3,4 delims=." %%a in ('echo %linker_version%') do set minorVersion=%%a
echo Linker Build: %linker_build%
echo Linker Version: %linker_version%


REM set linker_sharepath=\\cpvsbuild\drops\dev14\%linker_build%\raw
set linker_sharepath=\\cpvsbuild\drops\dev14\VCToolsRel\raw

set shareroot=%linker_sharepath%
set buildFolder=!shareroot!\%minorVersion%.00
echo !buildFolder!
for /F "tokens=2" %%a in ('%FILEVER_PATH% /V !buildFolder!\binaries.x86ret\bin\i386\link.exe ^| find "ProductVersion"') do set current_linker_version=%%a
echo Build: minorVersion, Version: !current_linker_version!

REM if [!current_linker_version!] equ [%linker_version%]
set matching_build=!buildFolder!

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

  set libArch=!arch!

  if /I "!libArch!"=="x86" (
    set libArch=i386
  )

  set archBinFolder=%matching_build%\binaries.!arch!ret\bin\!libArch!
  set archLibFolder=%matching_build%\binaries.!arch!ret\lib\!libArch!

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


