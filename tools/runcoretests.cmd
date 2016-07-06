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

set _RootDir=%~dp0..
set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -bvt -BaselineMode

pushd %_RootDir%\core\test
call %_RootDir%\core\test\runtests.cmd -binary jshost.exe -binDir %_RootDir%\Build\VcBuild\Bin -nottags html -nottags exclude_jshost %*
popd

exit /b %ERRORLEVEL%
