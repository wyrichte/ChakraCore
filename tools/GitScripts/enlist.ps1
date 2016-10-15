#
# Basic Enlistment script
#

param (
    [switch]$installTools,
    [string]$pythonInstallPath = "C:\Python35",
    [string]$repoRoot = ""
)

if (-not $repoRoot) {
    if (Test-Path Env:\REPO_ROOT) {
        Write-Host "Found $($Env:REPO_ROOT)"
        $repoRoot = $Env:REPO_ROOT
    }
}

if (-not $repoRoot) {
    throw "Error: Please pass in -RepoRoot:<path to repo>"
}

Write-Host "Repo root is $repoRoot"

if ($installTools) {
    $gitInstaller = "\\cpvsbuild\drops\dd\Git\InstallGit.cmd"
    $gitRemoteHelperPath = "\\chakrafs01\RepoTools\GCM\Setup.exe"
    $pythonInstallConfig = "AssociateFiles=0 InstallAllUsers=1 Include_Launcher=0 SimpleInstall=1"

    if ($pythonInstallPath -ne $null) {
        $pythonInstallConfig += " DefaultAllUsersTargetDir=`"$pythonInstallPath`""
    }

    $pythonInstaller = "\\chakrafs01\RepoTools\dep\python_installer.exe"

    $gitCommand = Get-Command "git.exe" -ErrorAction SilentlyContinue
    if ($gitCommand -eq $null) {
        Write-Host "Git not found on path- Installing"
        Start-Process -FilePath $gitInstaller -Wait -NoNewWindow
    } else {
        Write-Host "Git found- skipping install"
    }

    Write-Host "Deploying MSFT Git Credential Manager module"
    Start-Process -FilePath $gitRemoteHelperPath -ArgumentList "/VerySilent" -Wait -NoNewWindow

    if ($pythonInstallPath -ne $null) {
        if ((Test-Path (Join-Path $pythonInstallPath "python.exe")) -and
            (Test-Path (Join-Path $pythonInstallPath "Scripts"))) {
                $env:Path += ";$pythonInstallPath"
                $env:Path += ";$(Join-Path $pythonInstallPath `"Scripts`")"
            }
    }

    $pythonCommand = Get-Command "python.exe" -ErrorAction SilentlyContinue
    if ($pythonCommand -eq $null) {
        Write-Host "Python not found on path- Installing"
        Start-Process -FilePath $pythonInstaller -ArgumentList $pythonInstallConfig -Wait
    } else {
        Write-Host "Python found- skipping install"
    }

    if ($pythonInstallPath -ne $null) {
        if ((Test-Path (Join-Path $pythonInstallPath "python.exe")) -and
            (Test-Path (Join-Path $pythonInstallPath "Scripts"))) {
                $env:Path += ";$pythonInstallPath"
                $env:Path += ";$(Join-Path $pythonInstallPath `"Scripts`")"
            }
    }

    Write-Host "Configuring python"
    $result = ((iex "pip list") | Select-String gitpython)

    if ($result -eq $null) {
        Write-Host "gitpython not installed- installing"
        Start-Process -FilePath "pip" -ArgumentList "install gitpython" -Wait -NoNewWindow
    } else {
        Write-Host "gitpython found"
    }

    $result = ((iex "pip list") | Select-String plumbum)

    if ($result -eq $null) {
        Write-Host "plumbum not installed- installing"
        Start-Process -FilePath "pip" -ArgumentList "install plumbum" -Wait -NoNewWindow
    } else {
        Write-Host "plumbum found"
    }
} else {
    $cmd = Get-Command "git.exe" -ErrorAction SilentlyContinue
    if ($cmd -eq $null) {
        Write-Host "WARNING: git not found in the path and install skipped"
    } else {
        $gitVersion = git --version
        $versionFields = [regex]::match($gitVersion, 'git version (\d+)\.(\d+)\..*').Groups
        $major = [int]($versionFields[1].Value)
        $minor = [int]($versionFields[2].Value)
        $gitVersionOK = $True

        if ($major -ge 2) {
            if ($minor -lt 5) {
                $gitVersionOK = $False
            }
        } else {
            $gitVersionOK = $False
        }

        if (!$gitVersionOK) {
            Write-Host "
ERROR: Your currently-installed version of Git is out of date. This script requires Git version 2.5 or later.
    Your currently-installed version is:
        $gitVersion
    Please install the latest version of Git from:
        http://www.git-scm.com/download/win
    You can confirm the currently-installed version of Git by running:
        git --version
        "
            exit 1
        }
    }

    $cmd = Get-Command "python.exe" -ErrorAction SilentlyContinue
    if ($cmd -eq $null) {
        Write-Host "WARNING: python not found in the path and install skipped"
    }
}

$gitCommand = Get-Command "git.exe" -ErrorAction SilentlyContinue
if ($gitCommand -eq $null) {
    $gitRegKey = "HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1"
    if (-not (Test-Path $gitRegKey)) {
        $gitRegKey = "HKLM:\Software\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1"
        if (-not (Test-Path $gitRegKey)) {
            throw "Error: Git regkey not found- did you run the installer?"
        }
    }

    $gitInstallDirectory = (Get-ItemProperty $gitRegKey)."Inno Setup: App Path"
    $Env:Path += ";$(Join-Path $gitInstallDirectory `"bin`")"
} else {
    $gitExePath = $gitCommand.Definition

    if ($gitExePath -eq $null) {
        $gitExePath = $gitCommand.Source
    }

    if ($gitExePath -eq $null) {
        throw "Path to git.exe not found in result of Get-Command"
    }

    $gitInstallDirectory = Split-Path (Split-Path $gitExePath)
}

$Env:Path += ";$(Join-Path $gitInstallDirectory `"libexec\git-core`")"

# Turn safecrlf to off- the devdiv install script sets this to true
git config core.safecrlf false


$repoFolderName = Split-Path -Leaf $repoRoot
if ($repoFolderName -eq $null) {
    $repoFolderName = "Chakra";
}

$repoParent = Split-Path $repoRoot
if (-not (Test-Path $repoParent))
{
    throw "Error: '$repoParent' doesn't exist"
}

Push-Location $repoParent
git clone --no-checkout "https://devdiv.visualstudio.com/DefaultCollection/DevDiv/_git/Chakra" $repoFolderName
Push-Location $repoRoot

git config core.autocrlf false
# Set to false because some unit test repros need CRLF instead of LF
git config core.safecrlf false
# Rebase preserving merges by default on pulls
git config pull.rebase preserve

git checkout master
git submodule update --init
Push-Location (Join-Path $repoRoot "core")

git config core.autocrlf true
# Set to false because some unit test repros need CRLF instead of LF
git config core.safecrlf false
# Rebase preserving merges by default on pulls
git config pull.rebase preserve

git rm -rf *
git checkout master --force
Pop-Location
Pop-Location
Pop-Location

$BC4InstallPath = "\\chakrafs01\RepoTools\BC4\BC4.exe"
$BC4KeyPath = "\\chakrafs01\RepoTools\BC4\BC4Key.txt"
$BC4InstallFolder = Join-Path $Env:ProgramFiles "\Beyond Compare 4"
$BC4BinaryPath = Join-Path $BC4InstallFolder "\BComp.exe"

function ConfigGitForBC4 {
    Write-Host "Configuring Git for Beyond Compare 4"
    git config --global diff.tool bc
    git config --global difftool.bc.path $BC4BinaryPath
    git config --global merge.tool bc
    git config --global mergetool.bc.path $BC4BinaryPath
    Write-Host
    Write-Host "Use 'git mergetool' or 'git difftool' to use Beyond Compare from the command line"
}

if (-not (Test-Path $BC4BinaryPath) -and (Get-Command "git.exe" -ErrorAction SilentlyContinue)) {
    $installBC4 = Read-Host 'Would you like to install Beyond Compare 4 and use it as the default diff/merge tool in Git? [Y/n]'
    if ($installBC4 -eq "" -or $installBC4 -eq "Y") {
        Write-Host "Installing Beyond Compare 4"
        Start-Process -FilePath $BC4InstallPath -ArgumentList "/VerySilent" -Wait -NoNewWindow
        Write-Host "Registering Beyond Compare 4 with site license"
        Copy-Item $BC4KeyPath $BC4InstallFolder

        ConfigGitForBC4
    }
} elseif (Test-Path $BC4BinaryPath) {
    Write-Host "Found Beyond Compare 4"
    $useBC4 = Read-Host 'Would you like to use Beyond Compare 4 as the default diff/merge tool in Git? [Y/n]'

    if ($useBC4 -eq "" -or $useBC4 -eq "Y") {
        ConfigGitForBC4
    }
}

Write-Host
Write-Host "`nEnvironment setup completed- run $repoRoot\tools\GitScripts\init.cmd to launch environment"
