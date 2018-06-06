@echo off
setlocal

set NUGET_PACKAGE_NAME=RazzleTools
set PACKAGE_PATH=%SDXROOT%\onecoreuap\inetcore\jscript\packages
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\%NUGET_PACKAGE_NAME%
set NUGET_BASE_PATH=%NUGET_PACKAGE_SOURCE_DIR%\stage
pushd %~dp0

rmdir /q /s %NUGET_BASE_PATH%
mkdir %NUGET_BASE_PATH%

call robocopy /e %OSBuildToolsRoot%\ILC\x86\lib\Native %NUGET_BASE_PATH%\x86 /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %OSBuildToolsRoot%\ILC\arm\lib\Native %NUGET_BASE_PATH%\arm /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %OSBuildToolsRoot%\ILC\amd64\lib\Native %NUGET_BASE_PATH%\amd64 /NFL /NDL /NJH /NJS /nc /ns /np

call robocopy /e %SDXROOT%\public %NUGET_BASE_PATH%\public /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %SDXROOT%\tools\providers %NUGET_BASE_PATH%\providers /NFL /NDL /NJH /NJS /nc /ns /np

call robocopy /e %SDXROOT%\tools\x86 %NUGET_BASE_PATH%\x86 /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %SDXROOT%\tools\amd64 %NUGET_BASE_PATH%\amd64 /NFL /NDL /NJH /NJS /nc /ns /np

call copyPgortBin.bat %SDXROOT%\tools "x86,amd64,arm,arm64" %NUGET_BASE_PATH%
rem These binaries gets populated when running sdx root
call robocopy /e %OSBuildUtilitiesRoot%\TAEF\ %NUGET_BASE_PATH% /NFL /NDL /NJH /NJS /nc /ns /np

rem most tools dependencies have been moved to a vpack
call robocopy /e %OSBuildToolsRoot%\vc\ %NUGET_BASE_PATH%\vc /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %OSBuildToolsRoot%\vc\HostX86\x86 %NUGET_BASE_PATH%\x86 /NFL /NDL /NJH /NJS /nc /ns /np
call robocopy /e %OSBuildToolsRoot%\vc\HostX86\amd64 %NUGET_BASE_PATH%\amd64 /NFL /NDL /NJH /NJS /nc /ns /np

call ..\create_package.cmd
popd
endlocal
