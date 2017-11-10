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
:: ProjectSdkMetadataPath should be the location where the enlistment publics are located.
::     During build this translates to $(PUBLIC_ROOT)\Internal\$(_PROJECT_DEPOT)\Internal\BuildMetadata.
::
::::::::::::::::::::::::::::::::::::::::::::::::

setlocal EnableDelayedExpansion

set _DestinationPath=%1
set _ProjectionTestsRoot=%2\ProjectionTests
set _CabFilename=%3
set _ProjectSdkMetadataPath=%4
set _IncludedExtensions=cmd,js,html,baseline,config,exe,dll,bat,ps1,psm1,xml,man,winmd,rsp
set _FileFilter=%_ProjectSdkMetadataPath%\Windows.winmd

for %%i in (%_IncludedExtensions%) do (
    set _FileFilter=!_FileFilter! %_ProjectionTestsRoot%\*.%%i
)

echo cabarc.exe -r -p -P %_ProjectionTestsRoot:~3%\ -P %_ProjectSdkMetadataPath:~3%\ N %_DestinationPath%\%_CabFilename% %_FileFilter%
cabarc.exe -r -p -P %_ProjectionTestsRoot:~3%\ -P %_ProjectSdkMetadataPath:~3%\ N %_DestinationPath%\%_CabFilename% %_FileFilter%

endlocal