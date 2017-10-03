
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
$scriptLoc = $scriptLoc + "\"

if ( !(Test-Path ${scriptLoc}\..\tools\.downloads\) ) {
    $result = New-Item ${scriptLoc}\..\tools\.downloads\ -itemtype directory
}

if (! (Test-Path "${scriptLoc}..\tools\Nuget\") ) {
   Invoke-RestMethod -Uri https://microsoft.pkgs.visualstudio.com/_apis/public/nuget/client/CredentialProviderBundle.zip -Method GET -OutFile "${scriptLoc}..\tools\.downloads\CredentialProviderBundle.zip"
   Expand-Archive -Path "${scriptLoc}..\tools\.downloads\CredentialProviderBundle.zip" -DestinationPath "${scriptLoc}..\tools\Nuget\"
}

pushd "${scriptLoc}..\tools\Nuget"
$p = .\nuget.exe Sources list | select-string "https://microsoft.pkgs.visualstudio.com/_packaging/Universal.Store/nuget/v3/index.json"
if ([string]::IsNullOrEmpty($p)) {
    .\nuget.exe sources Add -Name "Universal.Store" -Source "https://microsoft.pkgs.visualstudio.com/_packaging/Universal.Store/nuget/v3/index.json"
}
.\nuget.exe install -Source "Universal.Store" -Source "Nuget.org" -OutputDirectory ..\ XflowConfig -version 3.0.1703.30004
popd
