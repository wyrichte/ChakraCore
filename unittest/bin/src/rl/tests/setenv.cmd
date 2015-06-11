:: make sure jc.exe exists
where jc.exe >nul 2>&1
if errorlevel 1 echo ERROR: Jc.exe is not on the path. & exit /b 1

:: set up the environment for rl.exe
set path=%path%;%CD%\bin\x86
set REGRESS=%CD%
