param(
    [switch]$unregister,

    [string]
    $regSvr32File = "",

    [string]
    $manifestSDRoot = "$env:WIN_JSHOST_METADATA_BASE_PATH"
)
$mypath = $MyInvocation.MyCommand.Path.Substring(0, $MyInvocation.MyCommand.Path.IndexOf($MyInvocation.MyCommand.Name))
$modulePath = (join-path $mypath "RegisterProjectionABIs.psm1")

write-host "modulePath = $($modulePath)"
Import-Module $modulePath

pushd $manifestSDRoot

write-host "Manifest root: $manifestSDRoot"
$currentPath = (get-location).Path

#copy the TakeRegistryAdminOwnership.exe
copy-item -force (join-path $mypath "TakeRegistryAdminOwnership.exe") $manifestSDRoot

$command = "";
if($unregister) {
    $command = 'unregister-chakraman'
    $modeFriendlyString = "unregistration"
} else {
    $command = '(ls *.man) | register-man -targetdir ' + $manifestSDRoot
    $modeFriendlyString = "registration"
}

write-host -foregroundcolor white "--------------------------------------------"
write-host -foregroundcolor white "Running manifest $($modeFriendlyString) step WITH TEST ONLY"
$detectIfAdminRequired = (invoke-expression "$command -testonly")
$detectIfAdminRequiredBool = (@($detectIfAdminRequired).Count -gt 0)
# write-host "(got `"$($detectIfAdminRequired)`" as return value)"
write-host -foregroundcolor white "/Running manifest $($modeFriendlyString) step WITH TEST ONLY"
write-host -foregroundcolor white "--------------------------------------------"
if ($detectIfAdminRequiredBool)
{
    write-host -foregroundcolor yellow "        NEED ADMIN: $($detectIfAdminRequiredBool)"

    write-host -foregroundcolor white "--------------------------------------------"
    write-host -foregroundcolor white " Running manifest $($modeFriendlyString) as ADMIN"

    # add the regsvr
    if( [string]::IsNullOrEmpty($regSvr32File) -eq $false )
    {
        if($unregister) 
        {
            $command += ";$env:WINDIR\System32\regsvr32.exe /s /u $($regSvr32File);$env:WINDIR\syswow64\regsvr32.exe /s /u $($regSvr32File)"
        }
        else
        {
            $command += ";$env:WINDIR\System32\regsvr32.exe /s $($regSvr32File);$env:WINDIR\syswow64\regsvr32.exe /s $($regSvr32File)"
        }
    }
    
    $command = "Import-Module " + $modulePath + ";pushd $($currentPath);" + $command + ";popd;exit 1"
    $bytes = [System.Text.Encoding]::Unicode.GetBytes($command)
    $encodedCommand = [Convert]::ToBase64String($bytes)
    write-host "Working folder: $($currentPath)"
    write-host "Executing: $($command)"
    $proc = (Start-Process "powershell.exe" -WorkingDirectory "$($currentPath)" -ArgumentList ("-encodedCommand","$encodedCommand","-ExecutionPolicy","remotesigned") -Verb runas -passthru) # -WindowStyle hidden
    write-host -nonewline -foregroundcolor gray "waiting for completion..."
    while( $proc -ne $null -and $proc.HasExited -ne $true )
    {
      sleep 1
    }
    write-host -foregroundcolor green " DONE!"

    # gracefully fallback in case of UAC (exit code not available)
    $procSucceeded = ($proc -ne $null) -and (($proc.ExitCode -eq 1) -or ($proc.ExitCode -eq $null))
    write-host "retValue: $procSucceeded - exitCode = $($proc.ExitCode)"
    write-host "  stderr: $($proc.StandardError)"
    write-host "  status: $($proc.ExitCode)"
    write-host -foregroundcolor white "/Running manifest $($modeFriendlyString) as ADMIN"
    write-host -foregroundcolor white "--------------------------------------------"

    if (!$procSucceeded) {
        write-error "ERROR"
        popd
        exit 2
    }
}

popd
exit 1
