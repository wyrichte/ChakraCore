REM @Echo off
setlocal EnableDelayedExpansion

REM if [%_BuildArch%%_BuildType%] neq [x86fre] (
REM  echo Only x86fre is supported for now
REM  goto :eof
REM )

if [%outputRoot%] equ [] set outputRoot=%_NTTREE%\jscript\PoGO

set pogo_mode=%1
if [%pogo_mode%] equ [] set pogo_mode=buildopt

set changelist=%2
set targetFolder=current
if [%changelist%] neq [] set targetFolder=%changelist%
set targetFolder=%outputRoot%\%targetFolder%

REM build baseline pogo, all __forceinline keep as is
if [%pogo_mode%] equ [buildbase] (
  set __FORCE_INLINE_LEVEL=
  set targetFolder=%targetFolder%_base
  call :BuildAndTrain 
  goto :eof
)

REM build opt pogo, tuned to turn on a few __forceinline, most old __forceinline are changed to inline
if [%pogo_mode%] equ [buildopt]  (
  set __FORCE_INLINE_LEVEL=2
  call :BuildAndTrain 
  goto :eof
)

REM build opt pogo, no __forceinline, this generates the minimal image
if [%pogo_mode%] equ [buildmin]  (
  set __FORCE_INLINE_LEVEL=1
  set targetFolder=%targetFolder%_min
  call :BuildAndTrain
  goto :eof
)

if [%pogo_mode%] equ [cleanup]  (
  call :cleanup
  goto :eof
)

goto :usage

:cleanup
set __FORCE_INLINE_LEVEL=
set CHAKRA_POGO_INSTRUMENT=
set POGO_INSTRUMENT=
set POGO_OPTIMIZE=
set CHAKRA_POGO_OPTIMIZE=
del "%OBJECT_ROOT%\%_BuildAlt%\pgi_status\chakra.dll.instr" >nul 2>&1

goto :eof

REM end main routine

:syncsource
set syncTimeStamp=
sd changes -s submitted //depot/fbl_ie_stage_dev3/...@%changelist%,%changelist%>%targetFolder%\%changelist%.txt
for /F "tokens=4,5" %%a in (%targetFolder%\%changelist%.txt) do set syncTimeStamp=%%a:%%b
call sdx sync //depot/fbl_ie_stage_dev3/...@%syncTimeStamp% > %targetFolder%\sync.txt 2>&1
if [%errorlevel%] neq [0] call sdx sync -nofastsync //depot/fbl_ie_stage_dev3/...@%syncTimeStamp% > %targetFolder%\sync_slow.txt 2>&1
if [%errorlevel%] neq [0] set syncfail=true

goto :eof

:BuildAndTrain

md %targetFolder% >nul 2>&1
set pgofolder=%targetFolder%\pgo
set pgifolder=%targetFolder%\pgi

md %pgofolder% >nul 2>&1
md %pgifolder% >nul 2>&1


set syncfail=false
if [%changelist%] neq [] call :syncsource
if [%syncfail%] neq [false] exit /b %errorlevel%

set expectLinkerRev=
set machine_type=
if /I [%_BuildArch%] equ [x86] (
  set pgort_Arch=i386
  set machine_type=ix86
  set expectLinkerRev=5
) else (
  set pgort_Arch=%_BuildArch%
  set machine_type=%_BuildArch%
  set expectLinkerRev=4
)

pushd %SDXMAPROOT%\tools
sd have %SDXMAPROOT%\tools\dev12\x32\%_BuildArch%\link.exe|find "#%expectLinkerRev%"
if [%errorlevel%] neq [0] (
  set errmsg=looks like linker has updated, please update this batch file to use new pgort lib/dll.
  echo %errmsg%
  echo %errmsg%>%targetFolder%\pogo.err
  goto :eof
)
popd

REM copied from \\vcfs\Builds\VS\feature_WinCCompLKG\1358695
REM http://fcib/request.aspx: need approval to checkin binary
REM set pgortlib=%SDXROOT%\inetcore\jscript\tools\External\lib\%pgort_Arch%\pgort.lib
set pgortlib=\\vcfs\builds\VS\feature_WinCCompLKG\1358695\binaries.%_BuildArch%ret\lib\%pgort_Arch%\pgort.lib

set jcpath=inetcore\jscript\exe\jc\release
set chakrapath=inetcore\jscript\Dll\JScript\release
set chakratestpath=inetcore\jscript\Dll\JScript\test

REM goto :do_pgi

:do_compile
call :compile
call :backup_none_pogo_bin

if not exist %_NTTREE%\jscript\jshost.exe (
  build -c -parent -dir %SDXROOT%\inetcore\jscript\Exe\jshost\release
)

if not exist %_NTTREE%\jscript\jshost.exe (
  echo failed to build jshost.exe
  exit /B
)

:do_pgi
echo.> %pgifolder%\linkall.rsp
call :linkpgi chakra.dll %chakrapath%
call :linkpgi chakratest.dll %chakratestpath%
call :linkpgi jc.exe %jcpath%
call contool @%pgifolder%\linkall.rsp >%pgifolder%\linkall.log

:do_train
echo.> %pgifolder%\trainall.rsp
call :train chakra.dll %chakrapath% jshost.exe
call :train chakratest.dll %chakratestpath% jshost.exe
call :train jc.exe %jcpath% jc.exe
call contool @%pgifolder%\trainall.rsp >%pgifolder%\trainall.log

:do_pgo
echo.> %pgofolder%\linkall.rsp
call :pgoAll3Binaries * %pgofolder%
call :pgoAll3Binaries sunspider* %pgofolder%_sunspider
call :pgoAll3Binaries kraken* %pgofolder%_kraken
call :pgoAll3Binaries octane* %pgofolder%_octane
call contool @%pgofolder%\linkall.rsp >%pgofolder%\linkall.log

:do_srcsrv
binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo %pgofolder%\chakra.dll
binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo %pgofolder%\chakratest.dll
binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo %pgofolder%\jc.exe
binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo_sunspider %pgofolder%_sunspider\jc.exe
binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo_kraken %pgofolder%_kraken\jc.exe
binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo_octane %pgofolder%_octane\jc.exe

set PDB_SRC_STREAM=1
call %sdxroot%\tools\PostBuildScripts\CaptureSourceFileList.cmd
call %sdxroot%\tools\PostBuildScripts\PdbSrcStream.cmd
set PDB_SRC_STREAM=

copy /Y %_nttree%\Symbols.pri\PoGO\pgo\dll\chakra.pdb %pgofolder%\chakra.pdb
copy /Y %_nttree%\Symbols.pri\PoGO\pgo\dll\chakratest.pdb %pgofolder%\chakratest.pdb
copy /Y %_nttree%\Symbols.pri\PoGO\pgo\exe\jc.pdb %pgofolder%\jc.pdb
copy /Y %_nttree%\Symbols.pri\PoGO\pgo_sunspider\exe\jc.pdb %pgofolder%_sunspider\jc.pdb
copy /Y %_nttree%\Symbols.pri\PoGO\pgo_kraken\exe\jc.pdb %pgofolder%_kraken\jc.pdb
copy /Y %_nttree%\Symbols.pri\PoGO\pgo_octane\exe\jc.pdb %pgofolder%_octane\jc.pdb

goto :eof
REM end :BuildAndTrain

:linkpgi
setlocal
set binary=%~1
set binname=%~n1
set binext=%~x1
set buildpath=%2
set _target=%pgifolder%\%binary%
md %_target% >nul 2>&1
set rspfile=%_target%\%binname%.rsp

copy /Y %OBJECT_ROOT%\%buildpath%\%_BuildAlt%\lnk.rsp %rspfile%
echo /machine:%machine_type% >> %rspfile%
echo /ltcg:pgi >> %rspfile%
echo /pgd:%_target%\%binary%.pgd %pgortlib% >> %rspfile%
echo /out:%_target%\%binary% >> %rspfile%
echo cd /d %SDXROOT%\%buildpath% ^& link @%rspfile% >> %pgifolder%\linkall.rsp
REM echo %linkcmd% >> %pgifolder%\linkall.rsp
REM call %linkcmd%
endlocal&goto :eof


:pgoAll3Binaries 
setlocal
set pattern=%1
set _pgofolder=%2
md %_pgofolder% >nul 2>&1

REM only build jc.exe for investigating test weight
if [%pattern%] equ [*] (
  call :pgo chakra.dll chakra.dll_%pattern%.pgc %_pgofolder% %chakrapath%
  call :pgo chakratest.dll chakratest.dll_%pattern%.pgc %_pgofolder% %chakratestpath%
)
call :pgo jc.exe jc.exe_%pattern%.pgc %_pgofolder% %jcpath%
goto :eof

:pgo
setlocal
set _binary=%~1
set _binname=%~n1
set _binext=%~x1
set _ext=%_binext:.=%
set _pgcpattern=%2
set _pgofolder=%3
set _relativepath=%4
set _pgdfile=%_binary%.pgd
set _target=%pgifolder%\%_binary%

copy /Y %_target%\%_pgdfile% %_pgofolder%\%_pgdfile%
set pgomgr_cmd="%SDXMAPROOT%\tools\dev12\x32\%_BuildArch%\pgomgr.exe" /merge:1 %_target%\%_pgcpattern% %_pgofolder%\%_pgdfile%
echo %pgomgr_cmd%
%pgomgr_cmd%

copy /Y %_NTTREE%\jscript\jshost.exe %_pgofolder%\
copy /Y %_NTTREE%\Symbols.pri\jscript\exe\jshost.pdb %_pgofolder%\jshost.pdb


copy /Y %OBJECT_ROOT%\%_relativepath%\%_BuildAlt%\lnk.rsp %_pgofolder%\%_binname%.rsp
echo /machine:%machine_type% >> %_pgofolder%\%_binname%.rsp
echo /ltcg:pgo >> %_pgofolder%\%_binname%.rsp
echo /pgd:%_pgofolder%\%_binary%.pgd >> %_pgofolder%\%_binname%.rsp
echo -d2:-PogoDeadOpt -d2:-PogoColdOpt -d2:-pagesize16384 -d2:-PPhase0 >> %_pgofolder%\%_binname%.rsp
echo /out:%_pgofolder%\%_binary% >> %_pgofolder%\%_binname%.rsp

echo pushd %SDXROOT%\%_relativepath% ^& link  @%_pgofolder%\%_binname%.rsp ^> %_pgofolder%\lnk_%_binname%.log 2^>^&1 ^&popd >> %pgofolder%\linkall.rsp

goto :eof

:srcsrv
setlocal
set _binary=%~1
binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo %_pgofolder%\%_binary%
set PDB_SRC_STREAM=1
call %sdxroot%\tools\PostBuildScripts\CaptureSourceFileList.cmd
call %sdxroot%\tools\PostBuildScripts\PdbSrcStream.cmd
set PDB_SRC_STREAM=

copy /Y %_nttree%\Symbols.pri\PoGO\pgo\%_ext%\chakra.pdb %_pgofolder%\chakra.pdb

goto :eof

:train
setlocal
set binary=%~1
set binname=%~n1
set binext=%~x1
set buildpath=%2
set _target=%pgifolder%\%binary%
set training_exe=%3

copy /Y %_NTTREE%\jscript\jshost.exe %_target%\jshost.exe
copy /Y %_NTTREE%\Symbols.pri\jscript\exe\jshost.pdb %_target%\jshost.pdb

copy /Y \\vcfs\builds\VS\feature_WinCCompLKG\1358695\binaries.%_BuildArch%ret\bin\%pgort_Arch%\pgort120.dll %_target%\pgort120.dll
copy /Y \\vcfs\builds\VS\feature_WinCCompLKG\1358695\binaries.%_BuildArch%ret\bin\%pgort_Arch%\pgort120.pdb %_target%\pgort120.pdb
copy /Y \\vcfs\builds\VS\feature_WinCCompLKG\1358695\binaries.%_BuildArch%ret\bin\%pgort_Arch%\msvcr120.* %_target%\

del %_target%\*.pgc
echo path=%_target%>%_target%\train.bat
for %%A in (%SDXROOT%\inetcore\jscript\unittest\SunSpider1.0.2\*.js) do call :RunTrainingTest %%A SunSpider %_target% %binary% %training_exe%
for %%A in (%SDXROOT%\inetcore\jscript\unittest\Kraken\*.js) do call :RunTrainingTest %%A Kraken %_target% %binary% %training_exe%
for %%A in (%SDXROOT%\inetcore\jscript\unittest\Octane\*.js) do call :RunTrainingTest %%A Octane %_target% %binary% %training_exe%
echo %_target%\train.bat >>%pgifolder%\trainall.rsp
endlocal&goto :eof

:RunTrainingTest
setlocal
set testfile=%1
set testfilename=%~n1
set testset=%2
set _target=%3
set binary=%~4
set binname=%~n4
set binext=%~x4
set training_exe=%5
if /I [%testfilename%] equ [OctaneFull] goto :eof
set trainCmd=%_target%\%training_exe% %testfile%
echo %trainCmd%>>%_target%\train.bat
echo move %_target%\%binname%%binext%^^!1.pgc %_target%\%binname%%binext%_%testset%_%testfilename%.pgc>>%_target%\train.bat
endlocal&goto :eof

:compile
set old_BUILD_OACR=%BUILD_OACR%
set BUILD_OACR=
set BUILD_DEBUG=1

REM goto :blah

call build -c -dir %SDXROOT%\inetcore\jscript\lib -jpath %pgifolder% -j build_lib
call build -cZ -dir %SDXROOT%\%chakrapath%;%SDXROOT%\%chakratestpath%;%SDXROOT%\%jcpath%;%SDXROOT%\inetcore\jscript\Exe\jshost\release -jpath %pgifolder%  -j build_dll

set rebuild_full=
if not exist %_NTTREE%\chakra.dll set rebuild_full=1
if not exist %_NTTREE%\jscript\chakratest.dll set rebuild_full=1
if not exist %_NTTREE%\jscript\jc.exe set rebuild_full=1
if [%rebuild_full%] equ [1] build -parent -dir %SDXROOT%\%chakrapath%;%SDXROOT%\%chakratestpath%;%SDXROOT%\%jcpath%;%SDXROOT%\inetcore\jscript\Exe\jshost\release -jpath %pgifolder%  -j build_full

set BUILD_DEBUG=1
set BUILD_OACR=%old_BUILD_OACR%
goto :eof

:backup_none_pogo_bin
copy /Y %_NTTREE%\chakra.dll %targetFolder%\chakra.dll
copy /Y %_NTTREE%\Symbols.pri\retail\dll\chakra.pdb %targetFolder%\chakra.pdb

copy /Y %_NTTREE%\jscript\chakratest.dll %targetFolder%\chakratest.dll
copy /Y %_NTTREE%\Symbols.pri\jscript\dll\chakratest.pdb %targetFolder%\chakratest.pdb

copy /Y %_NTTREE%\jscript\jc.exe %targetFolder%\jc.exe
copy /Y %_NTTREE%\Symbols.pri\jscript\exe\jc.pdb %targetFolder%\jc.pdb

copy /Y %_NTTREE%\jscript\jshost.exe %targetFolder%\jshost.exe
copy /Y %_NTTREE%\Symbols.pri\jscript\exe\jshost.pdb %targetFolder%\jshost.pdb

goto :eof

:usage
echo pgo.bat ^<buildbase ^| buildmin ^| buildopt^> [changelistnumber]
goto :eof


