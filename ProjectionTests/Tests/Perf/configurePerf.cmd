@echo off
setlocal

call :SetupEnv
md %PERF_TOOLS_DIR%
call :CopyFromBuild bin\WINRT\Trust\Activation\TakeRegistryAdminOwnership.exe 
call :CopyFromBuild bin\sfpdisable.exe
call :CopyFromBuild bin\sfpenable.exe
call :CopyFromBuild bin\NTTEST\COMTEST\WINRT\Cookbook\SetupExampleABI.cmd
call :CopyFromBuild bin\NTTEST\COMTEST\WINRT\Cookbook\Windows.RuntimeTest.Example.*
goto :EOF

:SetupEnv
SETLOCAL ENABLEDELAYEDEXPANSION
SET KEY="HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion"
FOR /F "tokens=3" %%I IN ('REG QUERY !KEY! /v BuildLabEx') DO (
    SET VALUE=%%I
)
FOR /F "delims=. tokens=1-5" %%I IN ('ECHO !VALUE!') DO (
    SET BUILD=\\winbuilds\release\%%L\%%I.%%J.%%M\%%K
)
set PERF_TOOLS_DIR=C:\perf-tools
goto :EOF

:CopyFromBuild
call :SetupEnv
IF EXIST %PERF_TOOLS_DIR%\%~n1%~x1
IF EXIST %BUILD%\%1 (
    copy /Y %BUILD%\%1 %SYSTEMDRIVE%\windows\system32
) ELSE (
    ECHO File Not Found: %BUILD%\%1
    ECHO Your build may be too old.
    pause
)
goto :EOF

