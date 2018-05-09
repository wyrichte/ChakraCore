@echo off
setlocal

if not "%SDXROOT%"=="" (
    echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    echo Chakra.ICU does not support being created within Razzle environments,
    echo as Razzle is only used for signing the resulting binaries.
    echo For instructions on how to update the Chakra.ICU package, please go to
    echo "https://aka.ms/chakra/icu-nuget"
    echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    goto :exit
)

set _SDXROOT="%1"

REM Common MSBuild properties for all of the builds
set ChakraICU=shared
set Configuration=Release
set "ChakraICU_Staging=%~dp0stage"
set "SolutionDir=%~dp0..\..\core\Build\"
set BuildChakraICUData=true

if not exist "%ChakraICU_Staging%" mkdir "%ChakraICU_Staging%"

pushd %~dp0..\..\core

if not exist "%CD%\deps\Chakra.ICU\icu" (
    python "%CD%\tools\icu\configure.py"
) else (
    echo ICU has already been downloaded. Continuing...
)

for %%a in (x64 x86 ARM ARM64) do (
    if not exist "%ChakraICU_Staging%\%%a" (
        mkdir "%ChakraICU_Staging%\%%a"
        set Platform=%%a

        if "%%a"=="ARM64" (
            set WindowsSDKDesktopARM64Support=true
        )

        echo Building Chakra.ICU.Common for %%a_%Configuration%
        msbuild /m "%CD%\deps\Chakra.ICU\Chakra.ICU.Common.vcxproj" /verbosity:minimal

        echo Building Chakra.ICU.i18n for %%a_%Configuration%
        msbuild /m "%CD%\deps\Chakra.ICU\Chakra.ICU.i18n.vcxproj" /verbosity:minimal

        echo Building Chakra.ICU.Data for %%a_%Configuration%
        msbuild /m "%CD%\deps\Chakra.ICU\Chakra.ICU.Data.vcxproj" /verbosity:minimal

        cp "%CD%\Build\VcBuild\bin\%%a_release\Chakra.ICU.Common.dll" "%ChakraICU_Staging%\%%a\Chakra.ICU.Common.dll"
        cp "%CD%\Build\VcBuild\bin\%%a_release\Chakra.ICU.Common.dll" "%ChakraICU_Staging%\%%a\icuuc.dll"
        cp "%CD%\Build\VcBuild\bin\%%a_release\Chakra.ICU.i18n.dll" "%ChakraICU_Staging%\%%a\icuin.dll"
        cp "%CD%\Build\VcBuild\bin\%%a_release\Chakra.ICU.Data.dll" "%ChakraICU_Staging%\%%a\Chakra.ICU.Data.dll"
    )
)

set NUGET_PACKAGE_NAME=ChakraICU
set PACKAGE_PATH=%~dp0..
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\%NUGET_PACKAGE_NAME%
set NUGET_BASE_PATH=%NUGET_PACKAGE_SOURCE_DIR%
set NUGET_PACKAGE_OUT=%NUGET_PACKAGE_SOURCE_DIR%\out
set NUGET_PACKAGE_CONFIG=%PACKAGE_PATH%\..\packages.config

start /wait cmd /k "%_SDXROOT%\tools\razzle.cmd dev_build no_ls no_oacr & %~dp0sign.cmd %ChakraICU_Staging% & pause & exit"

call %PACKAGE_PATH%\create_package.cmd

rmdir /Q /S "%ChakraICU_Staging%"

popd

:exit
endlocal
