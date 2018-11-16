:: ============================================================================
::
:: runcoretests.cmd
::
:: Runs RL tests specified by core\test\rlexedirs.xml configured for jshost.
:: Tests are run by calling core\test\runtests.cmd with appropriate flags.
::
:: ============================================================================
@echo off
setlocal

set _JSHOST=1

set _RootDir=%~dp0..
set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -BaselineMode

call %~dp0InstallBinariesForRazzleUnittest.cmd

pushd %_RootDir%\core\test
call %_RootDir%\core\test\runtests.cmd -binary ch.exe -binDir %_RootDir%\core\Build\VcBuild\Bin -nottags html -nottags exclude_jshost -nottags require_winglob %*
popd

exit /b %ERRORLEVEL%
