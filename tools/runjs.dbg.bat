@echo off 
REM The script debugger does not work except under the WOW
if /i NOT "%PROCESSOR_ARCHITECTURE%" == "x86" (
	echo %WINDIR%\sysWow64\cscript.exe //d %~dp0runjs.wsf %*
	%WINDIR%\sysWow64\cscript.exe //d %~dp0runjs.wsf %*
) else (
	echo cscript //d %~dp0runjs.wsf %*
	cscript //d %~dp0runjs.wsf %*
)
