function Get-ScriptDirectory
{
    $Invocation = (Get-Variable MyInvocation -Scope 1).Value;
    if($Invocation.PSScriptRoot)
    {
        $Invocation.PSScriptRoot;
    }
    Elseif($Invocation.MyCommand -and $Invocation.MyCommand.Path)
    {
        Split-Path $Invocation.MyCommand.Path
    }
    else
    {
        $Invocation.InvocationName.Substring(0,$Invocation.InvocationName.LastIndexOf("\"));
    }
}

$scriptLoc = Get-ScriptDirectory

#
#  Verify that cosmos powershell 
#
if (!(Get-Command Import-CosmosStreamFromFile -errorAction SilentlyContinue)) {
  write-host -ForegroundColor Red -BackgroundColor Black "ERROR!"
  write-host -ForegroundColor Red -BackgroundColor Black "Requires Cosmos Powershell DLL to be installed"
  write-host -ForegroundColor Red -BackgroundColor Black "see http://aka.ms/cosmos"
  write-host -ForegroundColor Red -BackgroundColor Black "Or, See instructions here:  https://mscosmos.visualstudio.com/DefaultCollection/CosmosPowerShell/_wiki/wikis/CosmosPowerShell.wiki?wikiVersion=GBwikiMaster&pagePath=%2FOverview%2FInstall%20Cosmos%20PowerShell"
  exit 1
}

#
# ensure release build of Chakra.Utils.dll is built
#
pushd $scriptLoc\..\helpers
msbuild /p:Configuration=Release
popd

#
# publish .net DLL
#
$dll = join-path $scriptLoc "..\helpers\bin\Release\Chakra.Utils.dll"
$dll = resolve-path $dll
if (! (test-path $dll)) {
    write-host "ERROR: Chakra.Utils.dll not found.  Build Release version of Chakra.Utils project"
    exit 1
}
write-host "publishing Chakra.Utils.dll from ${dll}..."
Import-CosmosStreamFromFile -Filename $dll -StreamName https://cosmos15.osdinfra.net/cosmos/Asimov.Partner.osg/shares/asimov.prod.data/PublicPartner/Processed/ChakraJavaScript/bin/Chakra.Utils.dll -Overwrite
write-host "...done."

#
#  Publish Views
#
$viewDir = join-path $scriptLoc "..\Views"
$viewDir = resolve-path $viewDir
write-host "publishing views from ${viewDir}"
Get-Childitem -Path $viewDir -Recurse | foreach-object {

    if (! ($_.Attributes -match 'Directory'))
    {
       $subName = $_.Name
       write-host "pushing $subName to cosmos"
       $targetLocation = "https://cosmos15.osdinfra.net/cosmos/Asimov.Partner.osg/shares/asimov.prod.data/PublicPartner/Processed/ChakraJavaScript/views/" + $subName 
       Import-CosmosStreamFromFile -Filename $_.FullName -StreamName $targetLocation -ForceText -Overwrite
    }
}
write-host "...done."

$xf=join-path $scriptLoc "..\tools\XflowConfig.3.0.1703.30004\tools\XFlowConfig.exe"
if (! (test-path $xf)) {
    write-host "ERROR: XFlowConfig.exe not found!  Run %~dp0getTools.cmd to download necessary tools."
    exit 1
}

# publish ChakraDaily2 Workflow
$wfdefFile2 = join-path $scriptLoc "..\output\ChakraDaily2\ChakraDaily2.wfdef"
$wfdefFile2 = resolve-path $wfdefFile2

&${xf} Deploy -path $wfdefFile2 -type Workflow -xflowServiceUrl https://wfm-data.corp.microsoft.com/xflow/service/ 
write-host ${wfdefFile2} " published"
