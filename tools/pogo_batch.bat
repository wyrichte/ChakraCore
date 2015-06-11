@echo off
setlocal
set pogocmd=%~dp0pogo.bat
REM Change 1485037 on 2015/02/08 02:38:34 by REDMOND\leish@IEDRTHOSTH-SDV3
for /F "tokens=2" %%a in ('sd changes //depot/fbl_ie_stage_dev3/inetcore/...@2015/02/08:02:38:34^,^|sort') do call :build_pogo %%a

goto :eof

:build_pogo
if exist %_NTTREE%\jscript\POGO\%1\pgi\build_full.err Echo last build %1 failed&goto :eof
if exist %_NTTREE%\jscript\POGO\%1\pgo\chakra.dll Echo %1 is already built&goto :eof
if not exist \\bptserver1\DailyBuild\fbl_ie_stage_dev3\checkins\%1\%_BuildArch%%_BuildType%\chakra.dll Echo %1 not build on bptserver1&goto :eof
Echo Building change %1...
call %pogocmd% buildopt %1
goto :eof


