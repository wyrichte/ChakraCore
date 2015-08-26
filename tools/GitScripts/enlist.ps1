#
# Basic Enlistment script
#

param (
    [switch]$installTools,
    [string]$pythonInstallPath = "C:\Python35",
    [string]$repoRoot = $null
)

if ($repoRoot -eq $null) {
    if (Test-Path Env:\REPO_ROOT) {
        $repoRoot = Env:\REPO_ROOT
    }
}

if ($repoRoot -eq $null) {
    throw "Error: Please pass in -RepoRoot:<path to repo>"
}

if ($installTools) {
    $gitInstaller = "\\cpvsbuild\drops\dd\Git\InstallGit.cmd"
    $pythonInstallConfig = "AssociateFiles=0 InstallAllUsers=1 Include_Launcher=0 SimpleInstall=1"

    if ($pythonInstallPath -ne $null) {
        $pythonInstallConfig += " DefaultAllUsersTargetDir=`"$pythonInstallPath`""
    }

    $pythonInstaller = "\\chakrafs01\RepoTools\dep\python-installer.exe"

    $gitCommand = Get-Command "git.exe" -ErrorAction SilentlyContinue
    if ($gitCommand -eq $null) {
        Write-Host "Git not found on path- Installing"
        Start-Process -FilePath $gitInstaller -Wait -NoNewWindow
    } else {
        Write-Host "Git found- skipping install"
    }

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

    Write-Host "Configuring python"
    $result = ((iex "pip list") | Select-String gitpython)

    if ($result -eq $null) {
        Write-Host "gitpython not installed- installing"
        Start-Process -FilePath "pip" -ArgumentList "install gitpython" -Wait -NoNewWindow
    } else {
        Write-Host "gitpython found"
    }
} else {
    $cmd = Get-Command "git.exe" -ErrorAction SilentlyContinue
    if ($cmd -eq $null) {
        Write-Host "WARNING: git not found in the path and install skipped"
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
    $gitInstallDirectory = Split-Path (Split-Path $gitCommand.Source)
}

$Env:Path += ";$(Join-Path $gitInstallDirectory `"libexec\git-core`")"

# Turn safecrlf to off- the devdiv install script sets this to true
git config core.safecrlf false


$repoFolderName = Split-Path -Leaf $repoRoot
$repoParent = Split-Path $repoRoot
if (-not (Test-Path $repoParent))
{
    throw "Error: '$repoParent' doesn't exist"
}

Push-Location $repoParent
git clone --recurse "https://devdiv.visualstudio.com/DefaultCollection/DevDiv/_git/Chakra" $repoFolderName
Pop-Location

Write-Host "`nEnvironment setup completed- run $repoRoot\tools\GitScripts\init.cmd to launch environment"