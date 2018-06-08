@setlocal
powershell -ExecutionPolicy Unrestricted -file %~dp0deleteTrackerFile.ps1 %*
goto :EOF
