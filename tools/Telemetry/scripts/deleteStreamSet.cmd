@setlocal
powershell -ExecutionPolicy Unrestricted -file %~dp0deleteStreamSet.ps1 %*
goto :EOF
