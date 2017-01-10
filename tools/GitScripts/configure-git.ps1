#
# Script to configure git
#

param (
    $gitPath = $null
)

$gitRemoteHelperPath = "\\chakrafs01\RepoTools\GCM\Setup.exe"

if ($gitPath -ne $null) {
    $env:Path += ";$gitPath"
}

if ((Get-Command "git.exe" -ErrorAction SilentlyContinue) -eq $null) {
    throw "Error: git.exe is not in your path, please pass in the correct path to git"
}

If (-not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(`
    [Security.Principal.WindowsBuiltInRole] "Administrator")) {
    throw "Error: You need to run this script as admin"
}

$gitPath = (((Get-Command "git.exe").Path) | Split-Path -Parent | Split-Path -Parent)

Write-Host "Git is installed in $gitPath"

$targetPath = Join-Path -Path $gitPath -ChildPath "libexec\git-core"

if (-not (Test-Path $targetPath)) {
    $targetPath = Join-Path -Path $gitPath -ChildPath "usr\bin"
    if (-not (Test-Path $targetPath)) {
        throw "Error: can't find $targetPath in git install directory"
    }
}

Write-Host "Deploying MSFT Git Credential Manager module"
Start-Process -FilePath $gitRemoteHelperPath -ArgumentList "/VerySilent" -Wait -NoNewWindow

$adUser = ([adsi] "LDAP://$(whoami /fqdn)")

Write-Host "Configuring git"

$s = git config --global --get user.name

if (!$s) {
    Write-Host "Name is $($adUser.FullName)"
    git config --global user.name $adUser.FullName
}

$s = git config --global --get user.email

if (!$s) {
    Write-Host "Email is $($adUser.Mail)"
    git config --global user.email $adUser.Mail
}

Write-Host "Configuring smart git defaults"

# Does index comparison to FS data in parallel
git config --global core.preloadindex true

# Undocumented but recommended to be set on Windows machines
git config --global core.fscache true

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
Write-Host @"
Following git config values set:
- Name/email (global- set if it's not already set)
- Credential helper: manager
- PreLoadIndex = true (global)
- SafeCrlf = false (local)
- FSCache = true (global)
- Pull.Rebase = preserve (local)
"@