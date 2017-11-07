@echo off
setlocal

set NUGET_PACKAGE_NAME=RazzleTools
set PACKAGE_PATH=%SDXROOT%\onecoreuap\inetcore\jscript\packages
set NUGET_BASE_PATH=%SDXROOT%
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\%NUGET_PACKAGE_NAME%
pushd %~dp0

call robocopy /e %OSBuildToolsRoot%\ILC\x86\lib\Native %SDXROOT%\tools\x86 /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %OSBuildToolsRoot%\ILC\arm\lib\Native %SDXROOT%\tools\arm /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %OSBuildToolsRoot%\ILC\amd64\lib\Native %SDXROOT%\tools\amd64 /NFL /NDL /NJH /NJS /nc /ns /np

call copyPgortBin.bat %SDXROOT%\tools "x86,amd64,arm" %SDXROOT%\tools
rem These binaries gets populated when running sdx root
call robocopy /e %OSBuildUtilitiesRoot%\TAEF\ %SDXROOT%\tools\ /NFL /NDL /NJH /NJS /nc /ns /np

rem most tools dependencies have been moved to a vpack
call robocopy /e %OSBuildToolsRoot%\vc\ %SDXROOT%\tools\vc /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %OSBuildToolsRoot%\vc\HostX86\x86 %SDXROOT%\tools\x86 /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %OSBuildToolsRoot%\vc\HostX86\amd64 %SDXROOT%\tools\amd64 /NFL /NDL /NJH /NJS /nc /ns /np

call ..\create_package.cmd
popd
endlocal
