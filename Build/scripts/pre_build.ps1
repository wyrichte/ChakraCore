# 
# Post-Build script
#
# This script is fairly simple. It checks if it's running
# in a VSO. If it is, it uses the VSO environment variables
# to figure out the commit that triggered the build, and if
# such a commit exists, it saves it's description to the build
# output to make it easy to inspect builds.
#

if (Test-Path Env:\TF_BUILD_SOURCEGETVERSION)
{
    $commitHash = ($Env:TF_BUILD_SOURCEGETVERSION).split()[2]
    $gitExe = "git.exe"

    if (!(Get-Command $gitExe -ErrorAction SilentlyContinue)) {
        $gitExe = "C:\1image\Git\bin\git.exe"
        if (!(Test-Path $gitExe)) {
            throw "git.exe not found in path- aborting."
        }
    }

    # Get commit description
    $command = "$gitExe log -1 --name-status -p $commitHash"
    $output = iex $command

    $coreHash = $null

    if ($output -match "Matching core commit hash: ([a-fA-F0-9]+)") {
        $coreHash = $Matches[1]
    }
    
    $sourcesDir = $Env:TF_BUILD_SOURCESDIRECTORY

    Push-Location $sourcesDir

    $coreRepoUrl = "https://devdiv.visualstudio.com/DefaultCollection/DevDiv/_git/ChakraCore"
    $cloneCommand = "$gitExe clone $coreRepoUrl"

    $coreRepoPath = Join-Path -Path $sourcesDir -ChildPath "core"
    if (Test-Path ($coreRepoPath)) {
        Write-Host "Pulling master branch"
        Push-Location $coreRepoPath
        iex "$gitExe checkout master"
        iex "$gitExe reset --hard"
        iex "$gitExe pull"
        Pop-Location
    } else {
        Write-Host "Cloning master branch"
        iex $cloneCommand
    }

    if ($coreHash -ne $null) {
        Write-Host "Syncing to $coreHash"
        Push-Location $coreRepoPath
        iex "$gitExe checkout $coreHash"
        Pop-Location
    } else {
        Write-Host "Note: No core hash specified"
    }

    $outputFile = Join-Path -Path $outputDir -ChildPath "change.txt"
    iex $command | Out-File $outputFile
    Pop-Location
}
