:: ============================================================================
::
:: initenv.cmd
::
:: Initializes cmd.exe environment with useful variables, including default
:: build config, and useful doskey macros.  Particularly handy doskey macros
:: added are jshost and ch for calling each on the default build config.
::
:: CONSIDER create a core only analog of this script for OSS community
::
:: Jump down to :addDoskeyMacros below to see all the doskey macros made
:: available from this script.
::
:: Suggested environment set up (if you don't already have something):
::
:: 1. Create %UserProfile%\initcmdenv.cmd script with the following content:
::
::    @echo off
::
::    doskey /macrofile=%~dp0doskeymacros.txt
::
:: 2. Create %UserProfile%\doskeymacros.txt with the following content
::    (change ch1 etc macros to reflect your chakrafull repo paths):
::
::    sa=                         doskey /macrofile=%UserProfile%\doskeymacros.txt
::    ea=                         gvim %UserProfile%\doskeymacros.txt
::
::    ch1=                        cd /d W:\ch1 & .\initenv.cmd
::    ch2=                        cd /d W:\ch2 & .\initenv.cmd
::    ch3=                        cd /d W:\ch3 & .\initenv.cmd
::    chsec=                      cd /d W:\ch-security-fixes & .\initenv.cmd
::
::    ..=                         cd ..\$*
::    ...=                        cd ..\..\$*
::    ....=                       cd ..\..\..\$*
::    .....=                      cd ..\..\..\..\$*
::    ......=                     cd ..\..\..\..\..\$*
::    .......=                    cd ..\..\..\..\..\..\$*
::
:: 3. Change your Command Prompt shortcut to call %UserProfile%\initcmdenv.cmd
::
::    Add ` /k "%USERPROFILE%\initcmdenv.cmd"` to the cmd.exe string in the
::    Target field of the shortcut's Properties dialog
::
:: 4. When you open Command Prompt via the shortcut (e.g. winkey "Command"
::    <enter>) you will have these convenient system wide macros, plus be able
::    to switch to any of your ChakraFull repo clones by using ch1, ch2, etc.
::
:: ============================================================================
@if "%_echo%"=="" (echo off) else (echo on)

rem Do not call setlocal here since we want to expose the variables
rem set by this script to the calling environment.

goto :main

:: ============================================================================
:: Initialize repo root and first time init defaults
:: ============================================================================
:init

  rem Set the repo root paths even if ChakraEnvInitialized is 1 in case this
  rem is an initenv.cmd script called from a different local repo

  set ChakraFullRepoRoot=%~dp0

  rem trick to get rid of trailing backslash on ChakraFullRepoRoot (the \. is key)
  for /f %%p in ("%ChakraFullRepoRoot%\.") do set ChakraFullRepoRoot=%%~fp

  set ChakraCoreRepoRoot=%ChakraFullRepoRoot%\core

  rem Now bail out on initialization if we were already initialized
  if "%ChakraEnvInitialized%" == "1" (
    goto :eof
  )

  rem Initialize with default build config x64debug
  set ChakraBuildArch=x64
  set ChakraBuildType=debug

  rem Set legacy build config vars to work with existing scripts
  set _BuildArch=%ChakraBuildArch%
  set _BuildType=%ChakraBuildType%
  set REPO_ROOT=%ChakraFullRepoRoot%

  rem Add msbuild to path
  call %ChakraCoreRepoRoot%\Build\scripts\add_msbuild_path.cmd

  goto :eof

:: ============================================================================
:: Add useful doskey macros
:: ============================================================================
:addDoskeyMacros

  if "%ChakraEnvInitialized%" == "1" (
    goto :eof
  )

  rem Call jshost or ch for the current default build
  doskey jshost=        %%ChakraFullBinDir%%\jshost.exe $*
  doskey ch=            %%ChakraCoreBinDir%%\ch.exe $*

  rem Open solutions in VS macros (using default association with .sln extension)
  doskey fullsln=       "%%ChakraFullRepoRoot%%\Build\Chakra.Full.sln"
  doskey coresln=       "%%ChakraCoreRepoRoot%%\Build\Chakra.Core.sln"

  rem Open ChakraHub with current repo automatically set in settings
  rem (Assume default install path since it is clickonce)
  doskey chakrahub=     "%%AppData%%\Microsoft\Windows\Start Menu\Programs\ChakraHub\ChakraHub.appref-ms" reporoot=%%ChakraFullRepoRoot%%

  rem Change/push directory macros
  doskey full=          cd /d "%%ChakraFullRepoRoot%%"\$*
  doskey core=          cd /d "%%ChakraCoreRepoRoot%%"\$*

  doskey ct=            cd /d "%%ChakraCoreRepoRoot%%\test"\$*
  doskey ut=            cd /d "%%ChakraFullRepoRoot%%\unittest"\$*
  doskey ft=            cd /d "%%ChakraFullRepoRoot%%\unittest"\$*

  doskey pfull=         pushd "%%ChakraFullRepoRoot%%"\$*
  doskey pcore=         pushd "%%ChakraCoreRepoRoot%%"\$*

  doskey pct=           pushd "%%ChakraCoreRepoRoot%%\test"\$*
  doskey put=           pushd "%%ChakraFullRepoRoot%%\unittest"\$*
  doskey pft=           pushd "%%ChakraFullRepoRoot%%\unittest"\$*

  rem Echo binary paths for convenience (e.g. copy pasting)
  doskey jshostpath=    echo %%ChakraFullBinDir%%\jshost.exe
  doskey chpath=        echo %%ChakraCoreBinDir%%\ch.exe
  doskey fullbinpath=   echo %%ChakraFullBinDir%%
  doskey corebinpath=   echo %%ChakraCoreBinDir%%

  rem Change current build flavor
  doskey x86=           %%ChakraFullRepoRoot%%\initenv.cmd -x86 -%%ChakraBuildType%%
  doskey x64=           %%ChakraFullRepoRoot%%\initenv.cmd -x64 -%%ChakraBuildType%%
  doskey arm=           %%ChakraFullRepoRoot%%\initenv.cmd -arm -%%ChakraBuildType%%
  doskey arm64=         %%ChakraFullRepoRoot%%\initenv.cmd -arm64 -%%ChakraBuildType%%

  doskey debug=         %%ChakraFullRepoRoot%%\initenv.cmd -%%ChakraBuildArch%% -debug
  doskey test=          %%ChakraFullRepoRoot%%\initenv.cmd -%%ChakraBuildArch%% -test
  doskey release=       %%ChakraFullRepoRoot%%\initenv.cmd -%%ChakraBuildArch%% -release

  doskey x86debug=      %%ChakraFullRepoRoot%%\initenv.cmd -x86 -debug
  doskey x86test=       %%ChakraFullRepoRoot%%\initenv.cmd -x86 -test
  doskey x86release=    %%ChakraFullRepoRoot%%\initenv.cmd -x86 -release

  doskey x64debug=      %%ChakraFullRepoRoot%%\initenv.cmd -x64 -debug
  doskey x64test=       %%ChakraFullRepoRoot%%\initenv.cmd -x64 -test
  doskey x64release=    %%ChakraFullRepoRoot%%\initenv.cmd -x64 -release

  doskey armdebug=      %%ChakraFullRepoRoot%%\initenv.cmd -arm -debug
  doskey armtest=       %%ChakraFullRepoRoot%%\initenv.cmd -arm -test
  doskey armrelease=    %%ChakraFullRepoRoot%%\initenv.cmd -arm -release

  doskey arm64debug=      %%ChakraFullRepoRoot%%\initenv.cmd -arm64 -debug
  doskey arm64test=       %%ChakraFullRepoRoot%%\initenv.cmd -arm64 -test
  doskey arm64release=    %%ChakraFullRepoRoot%%\initenv.cmd -arm64 -release

  doskey buildfull=     %%ChakraFullRepoRoot%%\tools\GitScripts\build.cmd $*
  doskey buildcore=     %%ChakraFullRepoRoot%%\tools\GitScripts\build.cmd /core $*

  doskey testfull=      %%ChakraFullRepoRoot%%\tools\rununittests.cmd -%%ChakraBuildArch%% -%%ChakraBuildType%% $*
  doskey testcore=      %%ChakraFullRepoRoot%%\tools\runcoretests.cmd -%%ChakraBuildArch%% -%%ChakraBuildType%% $*

  doskey corebuild=     %%ChakraFullRepoRoot%%\core\jenkins\buildone.cmd %%ChakraBuildArch%% %%ChakraBuildType%% $*
  doskey coretests=     %%ChakraFullRepoRoot%%\core\jenkins\testone.cmd %%ChakraBuildArch%% %%ChakraBuildType%% $*

  goto :eof

:: ============================================================================
:: Parse the user arguments into environment variables
:: ============================================================================
:parseArgs

  :NextArgument

  if /i "%1" == "-x86"              set ChakraBuildArch=x86&                                    goto :ArgOk
  if /i "%1" == "-x64"              set ChakraBuildArch=x64&                                    goto :ArgOk
  if /i "%1" == "-arm"              set ChakraBuildArch=arm&                                    goto :ArgOk
  if /i "%1" == "-arm64"            set ChakraBuildArch=arm64&                                  goto :ArgOk
  if /i "%1" == "-debug"            set ChakraBuildType=debug&                                  goto :ArgOk
  if /i "%1" == "-test"             set ChakraBuildType=test&                                   goto :ArgOk
  if /i "%1" == "-release"          set ChakraBuildType=release&                                goto :ArgOk

  if /i "%1" == "-x86debug"         set ChakraBuildArch=x86&set ChakraBuildType=debug&          goto :ArgOk
  if /i "%1" == "-x64debug"         set ChakraBuildArch=x64&set ChakraBuildType=debug&          goto :ArgOk
  if /i "%1" == "-armdebug"         set ChakraBuildArch=arm&set ChakraBuildType=debug&          goto :ArgOk
  if /i "%1" == "-arm64debug"       set ChakraBuildArch=arm64&set ChakraBuildType=debug&        goto :ArgOk
  if /i "%1" == "-x86test"          set ChakraBuildArch=x86&set ChakraBuildType=test&           goto :ArgOk
  if /i "%1" == "-x64test"          set ChakraBuildArch=x64&set ChakraBuildType=test&           goto :ArgOk
  if /i "%1" == "-armtest"          set ChakraBuildArch=arm&set ChakraBuildType=test&           goto :ArgOk
  if /i "%1" == "-arm64test"        set ChakraBuildArch=arm64&set ChakraBuildType=test&         goto :ArgOk
  if /i "%1" == "-x86release"       set ChakraBuildArch=x86&set ChakraBuildType=release&        goto :ArgOk
  if /i "%1" == "-x64release"       set ChakraBuildArch=x64&set ChakraBuildType=release&        goto :ArgOk
  if /i "%1" == "-armrelease"       set ChakraBuildArch=arm&set ChakraBuildType=release&        goto :ArgOk
  if /i "%1" == "-arm64release"     set ChakraBuildArch=arm64&set ChakraBuildType=release&      goto :ArgOk

  if not "%1" == "" (
    echo Unknown argument: %1
    echo Read initenv.cmd for more information
    set _fAbort=1
  )

  goto :eof

  :ArgOk
  shift

  goto :NextArgument

:: ============================================================================
:: Sets the (new) default build config
:: ============================================================================
:setBuildConfig

  if "%ChakraEnvInitialized%" == "1" (
    rem We've already been called once so let user know we're switching to a
    rem new build configuration and/or repo path
    echo Switched to %ChakraBuildArch%%ChakraBuildType% on %ChakraFullRepoRoot%
  )

  set "ChakraFullBinDir=%ChakraFullRepoRoot%\Build\VcBuild\bin\%ChakraBuildArch%_%ChakraBuildType%"
  set "ChakraCoreBinDir=%ChakraCoreRepoRoot%\Build\VcBuild\bin\%ChakraBuildArch%_%ChakraBuildType%"

  rem Set legacy build config vars to work with existing scripts
  set _BuildArch=%ChakraBuildArch%
  set _BuildType=%ChakraBuildType%

  rem Change prompt to show current repo root and build config.
  rem The doskey macros are relative to these values.
  prompt [%ChakraFullRepoRoot% %ChakraBuildArch%%ChakraBuildType%] $P$G

  goto :eof

:: ============================================================================
:: Main script
:: ============================================================================
:main

  call :init
  call :parseArgs %*
  if "%_fAbort%" == "1" (
    set _fAbort=
    exit /b 1
  )
  call :addDoskeyMacros
  call :setBuildConfig

  set ChakraEnvInitialized=1

  exit /b 0
