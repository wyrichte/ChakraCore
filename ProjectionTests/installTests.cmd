@echo off
setlocal

set _Dest=
set _SetupArg=

:NextArgument
if "%1" == "-?" (
	goto Usage
)else if "%1" == "" (
	goto RunSetup
)else (
	set _Dest=\\%1\c$\ProjectionTests
	goto ArgLoop
)

:ArgLoop
shift
goto :NextArgument

:RunSetup
if "%_Dest%" == "" (
	:Usage
	echo Usage: installTests.cmd ^<machine name^>
	echo   ^<machine name^>: tests will be installed at \\^<machine name^>\c$\ProjectionTests
	exit /b 0
)

echo Running Setup Steps...
call %SDXROOT%\inetcore\jscript\ProjectionTests\Tests\setup.cmd %_SetupArg%

Robocopy %SDXROOT%\inetcore\jscript\ProjectionTests %_Dest% /mir
xcopy /y %_NTTREE%\jscript\rl.exe %_Dest%\Tests

if not exist "%_Dest%\JsHost" (
    mkdir "%_Dest%\JsHost"
)

if not exist "%_Dest%\JsHost\JsHost.exe.local" (
    mkdir "%_Dest%\JsHost\JsHost.exe.local"
)

Robocopy %_NTTREE%\projection\winrt "%_Dest%\JsHost" /mir
xcopy /y %_NTTREE%\projection\*.js "%_Dest%"

if exist %_NTTREE%\jscript\pdm.dll (
    xcopy /y %_NTTREE%\jscript\pdm.dll "%_Dest%\JsHost\JsHost.exe.local\"
)

if exist %_NTTREE%\jscript\msdbg2.dll (
    xcopy /y %_NTTREE%\jscript\msdbg2.dll "%_Dest%\JsHost\JsHost.exe.local\"
)

if exist %_NTTREE%\jscript\pdmproxy100.dll (
    xcopy /y %_NTTREE%\jscript\pdmproxy100.dll "%_Dest%\JsHost\JsHost.exe.local\"
)

rem delete TakeRegistryAdminOwnership if present
if exist "%_Dest%\Registration\TakeRegistryAdminOwnership.exe" (
  del /q "%_Dest%\Registration\TakeRegistryAdminOwnership.exe"
)

endlocal
