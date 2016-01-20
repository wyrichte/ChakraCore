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

set linker_sharepath=\\vcfs\builds\VS\feature_%linker_build%

for /F %%a in ('dir /b /ad /o-n %linker_sharepath%') do (
  for /F "tokens=2" %%a in ('%FILEVER_PATH% /V %linker_sharepath%\%%a\binaries.x86ret\bin\i386\link.exe ^| find "ProductVersion"') do set current_linker_version=%%a
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

for %%p in (%PLATFORM%) do (
  set arch=%%p
  set dst=%DESTINATION%\!arch!

  if not exist !dst! (
    echo creating directory !dst!
    md !dst!
  )

  if "!arch!"=="x64" (
    set arch=amd64
  )

  set pgort_Arch=!arch!
  if "!pgort_Arch!"=="x86" (
    set pgort_Arch=i386
  )

  echo %matching_build%\binaries.!arch!ret\lib\!pgort_Arch!\pgort.lib =^> !dst!\pgort.lib
  copy /Y %matching_build%\binaries.!arch!ret\lib\!pgort_Arch!\pgort.lib !dst!\pgort.lib

  echo %matching_build%\binaries.!arch!ret\bin\!pgort_Arch!\pgort120.dll =^> !dst!\pgort120.dll
  copy /Y %matching_build%\binaries.!arch!ret\bin\!pgort_Arch!\pgort120.dll !dst!\pgort120.dll

  echo %matching_build%\binaries.!arch!ret\bin\!pgort_Arch!\pgort120.pdb =^> !dst!\pgort120.pdb
  copy /Y %matching_build%\binaries.!arch!ret\bin\!pgort_Arch!\pgort120.pdb !dst!\pgort120.pdb

  echo %matching_build%\binaries.!arch!ret\bin\!pgort_Arch!\msvcr120.* =^> !dst!
  copy /Y %matching_build%\binaries.!arch!ret\bin\!pgort_Arch!\msvcr120.* !dst!

  echo done for !arch!
)

endlocal
goto:eof

:error
endlocal
exit /b 1


