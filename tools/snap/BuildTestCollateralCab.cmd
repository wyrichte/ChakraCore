::::::::::::::::::::::::::::::::::::::::::::::::
::
:: BuildTestCollateralCab.cmd
::
:: Build a cab containing all the jscript dev unit test collateral.
:: This script should be run from the enlistment as part of build only during SNAP builds.
::
:: echo Usage: BuildTestCollateralCab.cmd [DestinationPath] [SourceRootPath] [Filename]
::
:: DestinationPath should be the location where the cab file should be built.
::     During build this translates to $(OBJ_PATH)\$O. 
::     Note: The destination must be under $(OBJ_PATH) since we are running this in PASS2.
::
:: SourceRootPath should be the location where the tools and unittest subfolders are located.
::     During build this translates to $(JSCRIPT_ROOT).
::
:: Filename should be the name of the cab file to build.
::     During build this should translate to $(TARGETNAME).cab - Tests.cab.
::
::::::::::::::::::::::::::::::::::::::::::::::::

setlocal EnableDelayedExpansion

set _DestinationPath=%1
set _JscriptRoot=%2
set _ToolsRoot=%_JscriptRoot%\tools
set _UnittestRoot=%_JscriptRoot%\unittest
set _CoreUnittestRoot=%_JscriptRoot%\core\test
set _CabFilename=%3
set _FileFilter=%_ToolsRoot%\*.* %_UnittestRoot%\*.* %_CoreUnittestRoot%\*.*

echo cabarc.exe -r -p -P %_JscriptRoot:~3%\ N %_DestinationPath%\%_CabFilename% %_FileFilter%
cabarc.exe -r -p -P %_JscriptRoot:~3%\ N %_DestinationPath%\%_CabFilename% %_FileFilter%

endlocal
