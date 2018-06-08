@setlocal
powershell -ExecutionPolicy Unrestricted -file %~dp0writeTrackerFile.ps1 %*
goto :EOF
