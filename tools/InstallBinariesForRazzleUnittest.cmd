
@echo off
setlocal EnableDelayedExpansion

set _RootDir=%~dp0..
rem Check if we are in a razzle environment
if "%SDXROOT%" neq "" (
  rem Make sure we have the necessary testing binaries
  if /i "%_BuildType%" equ "fre" (
    set _VSOBuildType=test
  ) else (
    set _VSOBuildType=debug
  )
  if /i "%_BuildArch%" equ "amd64" (
    set _VSOBuildArch=x64
  ) else if /i "%_BuildArch%" equ "woa" (
    set _VSOBuildArch=arm
  ) else (
    set _VSOBuildArch=%_BuildArch%
  )
  set __bindir=%_RootDir%\Build\VcBuild\Bin\!_VSOBuildArch!_!_VSOBuildType!
  echo WARNING: Overwriting Jscript binaries for razzle testing
  echo Copy %_NTTREE%\jscript -^> !__bindir!
  robocopy /mir %_NTTREE%\jscript !__bindir!
  robocopy %SDXROOT%\inetcore\devtoolbar\jstools\setup\%_BuildArch%\ !__bindir!\ *.dll
)

endlocal