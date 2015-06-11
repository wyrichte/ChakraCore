::::::::::::::::::::::::::::::::::::::::::::::::
::
:: BuildTestsCab.cmd
::
:: Build a cab containing all the ProjectionTests collateral.
:: This script should be run from the enlistment as part of build only during SNAP builds.
::
:: echo Usage: BuildTestsCab.cmd [DestinationPath] [SourceRootPath] [Filename] [PublicRoot]
::
:: DestinationPath should be the location where the cab file should be built.
::     During build this translates to $(OBJ_PATH)\$O. 
::     Note: The destination must be under $(OBJ_PATH) since we are running this in PASS2.
::
:: SourceRootPath should be the location where the ProjectionTests subfolder is located.
::     During build this translates to $(JSCRIPT_ROOT).
::
:: Filename should be the name of the cab file to build.
::     During build this should translate to $(TARGETNAME).cab - Tests.cab.
::
:: PublicRoot should be the location where the enlistment publics are located.
::     During build this translates to $(PUBLIC_ROOT).
::
::::::::::::::::::::::::::::::::::::::::::::::::

setlocal EnableDelayedExpansion

set _DestinationPath=%1
set _ProjectionTestsRoot=%2\ProjectionTests
set _CabFilename=%3
set _PublicRoot=%4
set _SDKRoot=%_PublicRoot%\sdk
set _MetadataRoot=%_SDKRoot%\winmetadata
set _IncludedExtensions=cmd,js,html,baseline,config,exe,dll,bat,ps1,psm1,xml,man,winmd,rsp
set _FileFilter=%_MetadataRoot%\*.winmd

for %%i in (%_IncludedExtensions%) do (
    set _FileFilter=!_FileFilter! %_ProjectionTestsRoot%\*.%%i
)

echo cabarc.exe -r -p -P %_ProjectionTestsRoot:~3%\ -P %_SDKRoot:~3%\ N %_DestinationPath%\%_CabFilename% %_FileFilter%
cabarc.exe -r -p -P %_ProjectionTestsRoot:~3%\ -P %_SDKRoot:~3%\ N %_DestinationPath%\%_CabFilename% %_FileFilter%

endlocal