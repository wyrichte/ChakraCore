@echo off
REM This script is for source server indexing
REM It read out all the source file paths from the pdb, and query the commit id in git repo,
REM then generate srcsrv stream and push to the pdb.
REM In debugger, it can read out the srcsrv stream and run the related command to fetch the required source file.
REM usage: srcsrv.bat <full|core> <root_dir> <file | [file|...]>

setlocal

set srctool=%~dp0srctool.exe
if not exist %srctool% echo Cannot find srctool.exe.&goto :end

set pdbstr=%~dp0pdbstr.exe
if not exist %pdbstr% echo Cannot find pdbstr.exe.&goto :end

set git=C:\1image\Git\bin\git.exe
REM hack for my test machine
if not exist %git% set git=C:\PROGRA~1\Git\cmd\git.exe&set debug=true
if not exist %git% echo Cannot find git.exe.&goto :end

set contool=%~dp0contool.exe
if exist %contool% set usecontool=true

set sort=%systemroot%\system32\sort.exe
set find=%systemroot%\system32\find.exe

set coreOnly=
if [%1] equ [core] set coreOnly=true&shift
set fullBuild=
if [%1] equ [full] set fullBuild=true&shift

if [%coreOnly%%fullBuild%] equ [] echo First arguments must be 'full' or 'core' &goto :end
)

if [%coreOnly%] equ [true] set corerootdir=%~dpnx1
if [%fullBuild%] equ [true] set fullrootdir=%~dpnx1&set corerootdir=%~dpnx1\core
shift
echo full: %fullrootdir%
echo core: %corerootdir%

:loop
if [%1] neq [] (
    for /F %%F in ('dir /s/b %1') do call :processFile %%F
    shift
    goto :loop
)

:end
endlocal&goto :eof

:processFile
set pdbFile=%~dpn1.pdb
echo === Processing %pdbFile% ===
if not exist %pdbFile% echo File %pdbFile% does not exist.&goto :eof


set srcsrvStream=%pdbFile%.srcsrv.txt

echo Generating srcsrv stream %srcsrvStream%...
call :generateSrcSrvStream>%srcsrvStream%

echo Pushing srcsrv stream to %pdbFile% ...
%pdbstr% -w -p:%pdbFile% -i:%srcsrvStream% -s:srcsrv
if [%debug%] neq [true] del %srcsrvStream%>nul 2>&1
goto :eof

:generateSrcSrvStream
if [%usecontool%] equ [true] (
    set contoolCoreRsp=%pdbFile%.core.rsp
    set contoolRsp=%pdbFile%.rsp
) else (
    set contoolCoreRsp=
    set contoolRsp=
)
echo SRCSRV: ini ------------------------------------------------
echo VERSION=1
echo VERCTRL=ChakraGIT
echo SRCSRV: variables ------------------------------------------
echo SRCSRVTRG=%%targ%%\%%var2%%\%%fnbksl%%(%%var3%%)\%%var4%%\%%fnfile%%(%%var1%%)
echo SRCSRVCMD=cscript.exe \\chakrafs\fs\Tools\srcsrv\gitsrcsrv.wsf %%var2%% %%var3%% %%var4%% %%srcsrvtrg%%
echo SRCSRV: source files ---------------------------------------
if [%fullBuild%] equ [true] call :fullBuild
if [%coreOnly%] equ [true] call :coreOnly
echo SRCSRV: end ------------------------------------------------

if [%debug%] neq [true] del %contoolCoreRsp%>nul 2>&1
if [%debug%] neq [true] del %contoolRsp%>nul 2>&1
goto :eof

:coreOnly
if [%contoolCoreRsp%] neq [] del %contoolCoreRsp%>nul 2>&1
cd /d %corerootdir%
for /F %%F in ('%srctool% -s %pdbFile%^|%sort%^|%find% /I "%corerootdir%"') do call :format %%F %corerootdir% ChakraCore %contoolCoreRsp%
if exist %contoolCoreRsp% %contool% @%contoolCoreRsp%|%find% /V "Command:"|%find% /V "Output:"|%find% /I "chakra"
goto :eof

:fullBuild
if [%contoolRsp%] neq [] del %contoolRsp%>nul 2>&1
cd /d %fullrootdir%
for /F %%F in ('%srctool% -s %pdbFile%^|%sort%^|%find% /I "%fullrootdir%\private"') do  call :format %%F %fullrootdir% Chakra %contoolRsp%
if exist %contoolRsp% %contool% @%contoolRsp%|%find% /V "Command:"|%find% /V "Output:"|%find% /I "chakra"

if [%contoolCoreRsp%] neq [] del %contoolCoreRsp%>nul 2>&1
cd /d %corerootdir%
for /F %%F in ('%srctool% -s %pdbFile%^|%sort%^|%find% /I "%corerootdir%"') do call :format %%F %corerootdir% ChakraCore %contoolCoreRsp%
if exist %contoolCoreRsp% %contool% @%contoolCoreRsp%|%find% /V "Command:"|%find% /V "Output:"|%find% /I "chakra"
goto :eof

:format
REM srctool output filenames are all lowercase, but git is case sensitive, 
REM restore filename with right case here
set fullpath=%~dpnx1
set rootpath=%~dpnx2
set proj=%3
set contoolRspFile=%4
if /I [%fullpath%] equ [] goto :eof
call set relative=%%fullpath:%rootpath%\=%%
set relative=%relative: =%
set gitPath=%relative:\=/%
if [%contoolRspFile%] neq [] echo %git% log -n 1 --pretty="%full%*%proj%*%gitPath%*%%H" -- %gitPath% >> %contoolRspFile%
if [%contoolRspFile%] equ [] %git% log -n 1 --pretty="%full%*%proj%*%gitPath%*%%H" -- %gitPath%
goto :eof
