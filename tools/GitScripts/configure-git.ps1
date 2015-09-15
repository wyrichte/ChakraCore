#
# Script to configure git
#

param (
    $gitPath = $null
)

$gitRemoteHelperPath = "\\ms\GIT\Release\git.remote-helper\git.remote-helper_20150331.01\Release"

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

Write-Host "Deploying MSFT Git Remote helper module"
Copy-Item -Force $gitRemoteHelperPath\* $targetPath

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

Write-Host "Configuring use of mshttps protocol"

git config --global --add url.mshttps://mseng.visualstudio.com/.insteadOf https://mseng.visualstudio.com/
git config --global --add url.mshttps://microsoft.visualstudio.com/.insteadOf https://microsoft.visualstudio.com/
git config --global --add url.mshttps://devdiv.visualstudio.com/.insteadOf https://devdiv.visualstudio.com/
git config --global credential.helper wincred

Write-Host "Configuring smart git defaults"

# Does index comparison to FS data in parallel
git config --global core.preloadindex true

# Undocumented but recommended to be set on Windows machines
git config --global core.fscache true

# Set to false because some unit test repros need CRLF instead of LF
git config core.safecrlf false

# Rebase preserving merges by default on pulls
git config pull.rebase preserve

Write-Host @"
Following git config values set:
- Name/email (global- set if it's not already set)
- Credential helper: wincred
- Using mshttps protocol on certain VSO instances
- PreLoadIndex = true (global)
- SafeCrlf = false (local)
- FSCache = true (global)
- Pull.Rebase = preserve (local)
"@