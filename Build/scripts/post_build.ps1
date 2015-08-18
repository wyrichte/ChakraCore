# 
# Post-build script
#
# This script is fairly simple. It checks if it's running
# in a VSO. If it is, it will delete the cloned child core 
# repo
#

if (Test-Path Env:\TF_BUILD_SOURCEGETVERSION)
{
    $sourcesDir = $Env:TF_BUILD_SOURCESDIRECTORY
    $coreDir = "core"

    Push-Location $sourcesDir

    if ((Test-Path -Path $coreDir)) {
        Get-ChildItem $coreDir -Recurse | Remove-Item -Recurse -Force
        Write-Host "Cleaned up '$core' repo"
    }

    Pop-Location
}
