@REM This file is here because Windows server will not run *.wsf files off of
@REM a network share without a prompt by default. This also solves the
@REM problem of using wscript by default also
@REM %windir%\syswow64\cscript //nologo "%~dp0runjs.wsf" %*
@cscript //nologo "%~dp0runjs.wsf" %*
