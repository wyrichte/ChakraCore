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
sd changes -s submitted //depot/th2_edge_stage_dev3/...@%changelist%,%changelist%>%targetFolder%\%changelist%.txt
for /F "tokens=4,5" %%a in (%targetFolder%\%changelist%.txt) do set syncTimeStamp=%%a:%%b
call sdx sync //depot/th2_edge_stage_dev3/...@%syncTimeStamp% > %targetFolder%\sync.txt 2>&1
if [%errorlevel%] neq [0] call sdx sync -nofastsync //depot/th2_edge_stage_dev3/...@%syncTimeStamp% > %targetFolder%\sync_slow.txt 2>&1
if [%errorlevel%] neq [0] set syncfail=true

goto :eof

:BuildAndTrain

rmdir /s/q %targetFolder%

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

REM === old way to check linker update
REM pushd %SDXMAPROOT%\tools
REM sd have %SDXMAPROOT%\tools\dev12\x32\%_BuildArch%\link.exe|find "#%expectLinkerRev%"
REM if [%errorlevel%] neq [0] (
REM   set errmsg=looks like linker has updated, please update this batch file to use new pgort lib/dll.
REM   echo %errmsg%
REM   echo %errmsg%>%targetFolder%\pogo.err
REM   goto :eof
REM )
REM popd
REM ===

REM look for matching linker version
set matching_build=
for /F "tokens=2" %%a in ('filever %SDXMAPROOT%\tools\dev12\x32\%_BuildArch%\link.exe /VA ^| find "ProductVersion"') do set linker_version=%%a
for /F "tokens=5" %%a in ('filever %SDXMAPROOT%\tools\dev12\x32\%_BuildArch%\link.exe /VA ^| find "FileVersion"') do set linker_build=%%a
set linker_build=%linker_build:(= %
set linker_build=%linker_build:)=%
for /F %%a in ('echo %linker_build%') do set linker_build=%%a
echo %linker_build%
set linker_sharepath=\\vcfs\builds\VS\feature_%linker_build%

for /F %%a in ('dir /b /ad /o-n %linker_sharepath%') do (
  for /F "tokens=2" %%a in ('filever /V %linker_sharepath%\%%a\binaries.%_BuildArch%ret\bin\%pgort_Arch%\link.exe ^| find "ProductVersion"') do set current_linker_version=%%a
  if [!current_linker_version!] equ [%linker_version%] set matching_build=%linker_sharepath%\%%a
)

echo VC tools build to use: %matching_build%

if [%matching_build%] equ [] (
  set errmsg=Can not find a matching link.exe under %linker_sharepath%.
  echo %errmsg%
  echo %errmsg%>%targetFolder%\pogo.err
  goto :eof
)

REM copied from \\vcfs\Builds\VS\feature_WinCCompLKG\1358695
REM http://fcib/request.aspx: need approval to checkin binary
REM set pgortlib=%SDXROOT%\inetcore\jscript\tools\External\lib\%pgort_Arch%\pgort.lib

copy /Y %matching_build%\binaries.%_BuildArch%ret\lib\%pgort_Arch%\pgort.lib %SDXROOT%\inetcore\jscript\tools\External\lib\pgort.lib
set pgortlib=%SDXROOT%\inetcore\jscript\tools\External\lib\pgort.lib

REM set jcpath=inetcore\jscript\exe\jc\release
REM set chakrapath=inetcore\jscript\Dll\JScript\release
REM set chakratestpath=inetcore\jscript\Dll\JScript\test
set jcpath=inetcore\jscript\private\bin\jc\release
set chakrapath=inetcore\jscript\private\bin\Chakra\release
set chakratestpath=inetcore\jscript\private\bin\Chakra\test
set BUILD_PGO=
REM goto :do_pgi

:do_compile
call :compile
call :backup_none_pogo_bin

if not exist %_NTTREE%\jscript\jshost.exe (
  build -c -parent -dir %SDXROOT%\inetcore\jscript\private\bin\jshost
)

if not exist %_NTTREE%\jscript\jshost.exe (
  echo failed to build jshost.exe
  exit /B
)

:do_pgi
echo.> %pgifolder%\linkall.rsp
call :linkpgi chakra.dll %chakrapath%
REM call :linkpgi jc.exe %jcpath%
call contool @%pgifolder%\linkall.rsp >%pgifolder%\linkall.log
call :linkMSBuildPGI ChakraTest.dll %chakratestpath%

:do_train
echo.> %pgifolder%\trainall.rsp
call :train chakra.dll %chakrapath% jshost.exe 0
call :train ChakraTest.dll %chakratestpath% jshost.exe 1
REM call :train jc.exe %jcpath% jc.exe
call contool @%pgifolder%\trainall.rsp >%pgifolder%\trainall.log

:do_pgo
echo.> %pgofolder%\linkall.rsp
call :pgoAll3Binaries * %pgofolder%
call :pgoAll3Binaries sunspider* %pgofolder%_sunspider
call :pgoAll3Binaries kraken* %pgofolder%_kraken
call :pgoAll3Binaries octane* %pgofolder%_octane
call contool @%pgofolder%\linkall.rsp >%pgofolder%\linkall.log
set BUILD_PGO=

:do_srcsrv
binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo %pgofolder%\chakra.dll
REM binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo %pgofolder%\ChakraTest.dll
REM binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo %pgofolder%\jc.exe
REM binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo_sunspider %pgofolder%_sunspider\jc.exe
REM binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo_kraken %pgofolder%_kraken\jc.exe
REM binplace.exe /R %_nttree%\PoGO /s %_nttree%\Symbols.pri\PoGO /j /:DBG /:NOCV -:LOGPDB /:DEST pgo_octane %pgofolder%_octane\jc.exe

set PDB_SRC_STREAM=1
call %sdxroot%\tools\PostBuildScripts\CaptureSourceFileList.cmd
call %sdxroot%\tools\PostBuildScripts\PdbSrcStream.cmd
set PDB_SRC_STREAM=

copy /Y %_nttree%\Symbols.pri\PoGO\pgo\dll\chakra.pdb %pgofolder%\chakra.pdb
copy /Y %_nttree%\Symbols.pri\PoGO\pgo\dll\ChakraTest.pdb %pgofolder%\ChakraTest.pdb
REM copy /Y %_nttree%\Symbols.pri\PoGO\pgo\exe\jc.pdb %pgofolder%\jc.pdb
REM copy /Y %_nttree%\Symbols.pri\PoGO\pgo_sunspider\exe\jc.pdb %pgofolder%_sunspider\jc.pdb
REM copy /Y %_nttree%\Symbols.pri\PoGO\pgo_kraken\exe\jc.pdb %pgofolder%_kraken\jc.pdb
REM copy /Y %_nttree%\Symbols.pri\PoGO\pgo_octane\exe\jc.pdb %pgofolder%_octane\jc.pdb

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

:linkMSBuildPGI
setlocal
set binary=%~1
set binname=%~n1
set binext=%~x1
set buildpath=%2
set _target=%pgifolder%\%binary%
md %_target% >nul 2>&1

del /F %OBJECT_ROOT%\%buildpath%\%_BuildAlt%\%binname%.pgd

pushd %SDXROOT%\%buildpath%
REM Force Link by moving the dll (NEED REVIEW)
REM move %_nttree%\jscript\%_binary%  %_target%\%binary%

SET BUILD_PGO=PGI
call build /cZ -jpath %pgifolder% -j pgi_%binname%
popd

copy /Y %pgifolder%\pgi_%binname% %pgofolder%\pgi_%binname%
copy  /Y  %OBJECT_ROOT%\%buildpath%\%_BuildAlt%\%binname%.pgd  %_target%\%binname%.pgd
copy  /Y %OBJECT_ROOT%\%buildpath%\%_BuildAlt%\%binary%  %_target%\%binary%
endlocal&goto :eof

:pgoAll3Binaries 
setlocal
set pattern=%1
set _pgofolder=%2
md %_pgofolder% >nul 2>&1

REM only build jc.exe for investigating test weight
if [%pattern%] equ [*] (
  call :pgo chakra.dll chakra.dll_%pattern%.pgc %_pgofolder% %chakrapath%
  call :pgoMSBuild ChakraTest.dll ChakraTest.dll_%pattern%.pgc %_pgofolder% %chakratestpath%
)
REM call :pgo jc.exe jc.exe_%pattern%.pgc %_pgofolder% %jcpath%
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
echo -d2:-PogoDeadOpt -d2:-PogoColdOpt -d2:-pagesize16384 -d2:-PPhase0 /EmitPogoPhaseInfo >> %_pgofolder%\%_binname%.rsp
echo /out:%_pgofolder%\%_binary% >> %_pgofolder%\%_binname%.rsp

echo pushd %SDXROOT%\%_relativepath% ^& link  @%_pgofolder%\%_binname%.rsp ^> %_pgofolder%\lnk_%_binname%.log 2^>^&1 ^&popd >> %pgofolder%\linkall.rsp

goto :eof

:pgoMSBuild
setlocal
set _binary=%~1
set _binname=%~n1
set _binext=%~x1
set _ext=%_binext:.=%
set _pgcpattern=%2
set _pgofolder=%3
set _relativepath=%4
set _pgdfile=%_binname%.pgd
set _target=%pgifolder%\%_binary%


copy  /Y %_target%\%_pgdfile%   %OBJECT_ROOT%\%_relativepath%\%_BuildAlt%\%_pgdfile%

echo pushd %SDXROOT%\%_relativepath%> %_target%\pgo.bat
echo SET BUILD_PGO=PGO>> %_target%\pgo.bat
REM Force Link by moving the dll (NEED REVIEW)
echo move %_nttree%\jscript\%_binary%  %_target%\%_binary%>> %_target%\pgo.bat

REM call build /l -jpath %pgofolder% -j %_binary%_pgo
echo call msbuild.cmd "razzle.chakra.test.vcxproj" /nologo /p:BuildingInSeparatePasses=true /p:BuildingWithBuildExe=true /clp:NoSummary /verbosity:normal /Target:BuildLinked /p:Pass=Link /p:ObjectPath=%OBJECT_ROOT%\%_relativepath%\>> %_target%\pgo.bat
popd>> %_target%\pgo.bat
echo copy /Y %_NTTREE%\jscript\jshost.exe %_pgofolder%\>> %_target%\pgo.bat
echo copy /Y %_NTTREE%\Symbols.pri\jscript\exe\jshost.pdb %_pgofolder%\jshost.pdb>> %_target%\pgo.bat
echo copy  /Y %_NTTREE%\jscript\%_binary% %_pgofolder%\%_binary%>>%_target%\pgo.bat
echo copy  /Y %OBJECT_ROOT%\%_relativepath%\%_BuildAlt%\%_pgdfile% %_pgofolder%\%_pgdfile%>> %_target%\pgo.bat
echo del %OBJECT_ROOT%\%_relativepath%\%_BuildAlt%\*.pgc>> %_target%\pgo.bat
echo pushd %_target% ^& pgo.bat  ^> %_pgofolder%\pgo_%_binname%.log 2^>^&1 ^&popd>> %pgofolder%\linkall.rsp

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
set is_msbuild=%4

copy /Y %_NTTREE%\jscript\jshost.exe %_target%\jshost.exe
copy /Y %_NTTREE%\Symbols.pri\jscript\exe\jshost.pdb %_target%\jshost.pdb

copy /Y %matching_build%\binaries.%_BuildArch%ret\bin\%pgort_Arch%\pgort120.dll %_target%\pgort120.dll
copy /Y %matching_build%\binaries.%_BuildArch%ret\bin\%pgort_Arch%\pgort120.pdb %_target%\pgort120.pdb
copy /Y %matching_build%\binaries.%_BuildArch%ret\bin\%pgort_Arch%\msvcr120.* %_target%\

del %_target%\*.pgc
if /I [%is_msbuild%] equ [1] del %OBJECT_ROOT%\%buildpath%\%_BuildAlt%\*.pgc

echo path=%_target%>%_target%\train.bat
for %%A in (%SDXROOT%\inetcore\jscript\unittest\SunSpider1.0.2\*.js) do call :RunTrainingTest %%A SunSpider %_target% %binary% %training_exe% %is_msbuild% %buildpath%
for %%A in (%SDXROOT%\inetcore\jscript\unittest\Kraken\*.js) do call :RunTrainingTest %%A Kraken %_target% %binary% %training_exe% %is_msbuild% %buildpath%
for %%A in (%SDXROOT%\inetcore\jscript\unittest\Octane\*.js) do call :RunTrainingTest %%A Octane %_target% %binary% %training_exe% %is_msbuild% %buildpath%

if /I [%is_msbuild%] equ [1] (
   echo copy /Y %_target%\%binname%*.pgc  %OBJECT_ROOT%\%buildpath%\%_BuildAlt%\>>%_target%\train.bat
)

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
set is_msbuild=%6
set buildpath=%7

if /I [%testfilename%] equ [OctaneFull] goto :eof
set trainCmd=%_target%\%training_exe% %testfile%
echo %trainCmd%>>%_target%\train.bat

if /I [%is_msbuild%] equ [0] (
	echo move %_target%\%binname%%binext%^^!1.pgc %_target%\%binname%%binext%_%testset%_%testfilename%.pgc>>%_target%\train.bat
)
endlocal&goto :eof

:compile
set old_BUILD_OACR=%BUILD_OACR%
set BUILD_OACR=
set BUILD_DEBUG=1

REM goto :blah

call build -c -dir %SDXROOT%\inetcore\jscript\core\lib -jpath %pgifolder% -j build_lib
REM call build -cZ -dir %SDXROOT%\%chakrapath%;%SDXROOT%\%chakratestpath%;%SDXROOT%\%jcpath%;%SDXROOT%\inetcore\jscript\Exe\jshost\release -jpath %pgifolder%  -j build_dll
REM call build -cZ -dir %SDXROOT%\%chakrapath%;%SDXROOT%\%chakratestpath%;%SDXROOT%\inetcore\jscript\private\bin\jshost -jpath %pgifolder%  -j build_dll
call build -cZ -dir %SDXROOT%\%chakrapath%;%SDXROOT%\inetcore\jscript\private\bin\jshost -jpath %pgifolder%  -j build_dll

set rebuild_full=
if not exist %_NTTREE%\chakra.dll set rebuild_full=1
REM if not exist %_NTTREE%\jscript\ChakraTest.dll set rebuild_full=1
REM if not exist %_NTTREE%\jscript\jc.exe set rebuild_full=1
REM if [%rebuild_full%] equ [1] build -parent -dir %SDXROOT%\%chakrapath%;%SDXROOT%\%chakratestpath%;%SDXROOT%\%jcpath%;%SDXROOT%\inetcore\jscript\Exe\jshost\release -jpath %pgifolder%  -j build_full
REM if [%rebuild_full%] equ [1] build -parent -dir %SDXROOT%\%chakrapath%;%SDXROOT%\%chakratestpath%;%SDXROOT%\inetcore\jscript\private\bin\jshost -jpath %pgifolder%  -j build_full
if [%rebuild_full%] equ [1] build -parent -dir %SDXROOT%\%chakrapath%;%SDXROOT%\inetcore\jscript\private\bin\jshost -jpath %pgifolder%  -j build_full

set BUILD_DEBUG=1
set BUILD_OACR=%old_BUILD_OACR%
goto :eof

:backup_none_pogo_bin
copy /Y %_NTTREE%\chakra.dll %targetFolder%\chakra.dll
copy /Y %_NTTREE%\Symbols.pri\retail\dll\chakra.pdb %targetFolder%\chakra.pdb

copy /Y %_NTTREE%\jscript\ChakraTest.dll %targetFolder%\ChakraTest.dll
copy /Y %_NTTREE%\Symbols.pri\jscript\dll\ChakraTest.pdb %targetFolder%\ChakraTest.pdb

REM copy /Y %_NTTREE%\jscript\jc.exe %targetFolder%\jc.exe
REM copy /Y %_NTTREE%\Symbols.pri\jscript\exe\jc.pdb %targetFolder%\jc.pdb

copy /Y %_NTTREE%\jscript\jshost.exe %targetFolder%\jshost.exe
copy /Y %_NTTREE%\Symbols.pri\jscript\exe\jshost.pdb %targetFolder%\jshost.pdb

goto :eof

:usage
echo pgo.bat ^<buildbase ^| buildmin ^| buildopt^> [changelistnumber]
goto :eof


