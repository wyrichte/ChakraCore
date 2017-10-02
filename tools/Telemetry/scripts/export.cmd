@echo off
@setlocal

set __outdir=%~dp0..\output

if not exist "%~dp0..\tools\XflowConfig.3.0.1703.30004\tools\XFlowConfig.exe" (
    echo ERROR: XFlowConfig.exe not found!  Run %~dp0getTools.cmd to download necessary tools.
    goto :EOF
)

pushd %~dp0..\tools\XflowConfig.3.0.1703.30004\tools\
XflowConfig.exe ExportWorkflow -xflowServiceUrl https://wfm-data.corp.microsoft.com/xflow/service/ -name ChakraDaily -outputDir %__outdir%

popd

GOTO :EOF
