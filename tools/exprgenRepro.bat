@setlocal ENABLEDELAYEDEXPANSION enableextensions
@echo off
set _failed=0
set base=%~dpnx1
set arg1=%1 
echo %base%
if "%base%" == "" (
  echo This tool can run baseline and test version of jshost command given an exprgen generated .js file. 
  echo A repro.bat is generated allowing to repro a failed test. Also a log.txt is generated with details of
  echo all failures etc.
  echo Usage: %0 [dir or file] [Path to jshost.exe | additional args to test jshost run]
  echo     dir: Directory in which exprgen bug*.js files can be found
  echo     file: Bug file
  echo     Path to jshost.exe is optional - it will use the jshost.exe in the current %%PATH%% otherwise  
  goto exit
)
if exist log.txt (
   del log.txt
)
set jshost=jshost.exe
if NOT "%2" == "" (
   if exist "%2" (
      set jshost=%2
      shift /2
   )
)
set otherArgs=
:Loop
if "%2"=="" goto Continue
    set otherArgs=%otherArgs% %2
    shift /2
goto Loop
:Continue

set is_dir=0
pushd %arg1% >NUL 2>NUL
if "%ERRORLEVEL%"=="0" ( 
   set is_dir=1
   popd 
)
if "%is_dir%" == "1" (
	for /f %%i in ('dir /b %base%\bug*.js') do (
        call :runFile "%base%\%%i"
		if "!_failed!" == "1" (
		   goto exit
		)
	)
) else (
	call :runFile "%base%"
)




endlocal
:exit
goto :eof

:runFile
setlocal ENABLEDELAYEDEXPANSION enableextensions
set switch=
    set file=%~1
	set failed=0
	for /f "tokens=* delims=" %%s in ('findstr /lip /c:"//Switches:" %file%') do (
		set switch=%%s
		set sw=!switch:~11!  
	)
    if "%sw%"=="" (
        set failed=1
        call :writelog "Failure parsing 'Switches:' comment in the .js file"  	  
        goto return
    )

	for /f "tokens=* delims=" %%s in ('findstr /lip /c:"//Baseline Switches:" %file%') do (
		set switch=%%s
		set baseSw=!switch:~20!  
	)
    if "%baseSw%"=="" (
        set failed=1
        call :writelog "Failure parsing 'Baseline Switches:' comment in the .js file"
        goto return

    )    
    
   call :writelog "%jshost% !baseSw! %file%"
   %jshost%  !baseSw! %file% > base.txt 2> errBase.txt
   call :writelog "%jshost%  !sw! %file% %otherArgs%"
   %jshost% !sw! %file% %otherArgs% > test.txt  2> errTest.txt
   fc base.txt test.txt 1>NUL
    if "%ERRORLEVEL%"=="1" ( 
   	  call :writelog "-----------------------------------------------------"
	  call :writelog "FAILED. base.txt and test.txt do not match. Diff:"
	  call :writelog  "-----------------------------------------------------"
	  set failed=1
	  fc base.txt test.txt
	  fc base.txt test.txt >> log.txt
	  echo Repro file generated: repro.bat
	  echo %jshost% -crashonexception !sw! %file% %%* > repro.bat
      goto nextdiff
   )
:nextdiff
    fc errBase.txt errTest.txt 1>NUL
    if "%ERRORLEVEL%"=="1" ( 
  	  call :writelog "-----------------------------------------------------"
	  call :writelog "FAILED. errBase.txt and errTest.txt do not match. Diff:"
	  call :writelog "-----------------------------------------------------"
	  set failed=1
	  fc errBase.txt errTest.txt
	  fc errBase.txt errTest.txt >> log.txt
	  echo Repro file generated: repro.bat
	  echo %jshost% !sw! %file% %%* > repro.bat
	  goto return
   )
   if "!failed!" neq "1" (
         call :writeLog "PASSED"
         call :writeLog "."
   )
:return
endlocal & set _failed=!failed! & goto :eof

:writelog
setlocal ENABLEDELAYEDEXPANSION enableextensions
set msg=%~1
echo %msg%
echo %msg% >> log.txt
:return
