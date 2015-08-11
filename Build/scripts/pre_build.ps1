# 
# Pre-build script
#
# This script is fairly simple. It checks if it's running
# in a VSO. If it is, it uses the VSO environment variables
# to figure out the commit that triggered the build, and if
# such a commit exists, it saves it's description to the build
# output to make it easy to inspect builds. This will also 
# clone the core repository if it doesn't exist, or sync it
# if it does. If the commit description has a core commit 
# referenced in it, it will sync the core repo to that particular
# commit
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
    $outputDir = $Env:TF_BUILD_DROPLOCATION

    Push-Location $sourcesDir

    # The access token for this user is cached on the build controller
    # machine itself. If the token expires, we need to log back onto the
    # machine and re-enter it to update the cache
    $coreRepoUrl = "mshttps://devdiv.visualstudio.com/DefaultCollection/DevDiv/_git/ChakraCore"
    $cloneCommand = "$gitExe clone $coreRepoUrl core 2>&1"

    $coreRepoPath = Join-Path -Path $sourcesDir -ChildPath "core"
    if (Test-Path ($coreRepoPath)) {
        Write-Host "Pulling master branch"
        Push-Location $coreRepoPath
        iex "$gitExe checkout master 2>&1"
        iex "$gitExe reset --hard 2>&1"
        iex "$gitExe pull 2>&1"
        Pop-Location
    } else {
        Write-Host "Cloning master branch"
        $output = iex $cloneCommand
        if ($LastExitCode -ne 0) {
            Write-Host "Clone failed with following output:`n$output"
        } else {
            Write-Host "Cloning core repo succeeded"
        }
    }

    if ($coreHash -ne $null) {
        Write-Host "Syncing to $coreHash"
        Push-Location $coreRepoPath
        iex "$gitExe checkout $coreHash 2>&1"
        Pop-Location
    } else {
        Write-Host "Note: No core hash specified"
    }

    if (-not(Test-Path -Path $outputDir)) {
        New-Item -Path $outputDir -ItemType Directory -Force
    }

    $outputFile = Join-Path -Path $outputDir -ChildPath "change.txt"
    iex $command | Out-File $outputFile
    Pop-Location
}

