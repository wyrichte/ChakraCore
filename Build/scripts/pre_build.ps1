param (
    [string]$srcpath = "",
    [string]$binpath = "",
    [string]$objpath = "",
    [string]$oauth
)

$CoreScriptDir = "$PSScriptRoot\..\..\core\build\scripts"

$OuterScriptRoot = $PSScriptRoot;
. "$CoreScriptDir\pre_post_util.ps1"

& $PSScriptRoot\..\..\core\Build\Scripts\pre_build.ps1 -oauth $oauth -srcpath $srcpath -binpath $binpath -objpath $objpath

$exitCode = $LastExitCode;

if (Test-Path Env:\TF_BUILD_SOURCEGETVERSION)
{
    $commitHash = ($Env:TF_BUILD_SOURCEGETVERSION).split(':')[2];
    $gitExe = GetGitPath;

    $outputDir = $Env:TF_BUILD_DROPLOCATION
    $outputFile = Join-Path -Path $outputDir -ChildPath "change.txt"

    Push-Location $srcpath;
    $summary = iex "$gitExe diff --submodule $commitHash~1..$commitHash core"
    Pop-Location
    if ($summary -and ($summary -is [System.Array]))
    {
        Push-Location "$srcpath\core"
        $commits = $summary[0].split()[2].replace(":","");
        Write-Output "" | Out-File $outputFile -Append
        Write-Output "=======================================" | Out-File $outputFile -Append
        Write-Output "Core Commits: $commits" | Out-File $outputFile -Append
        Write-Output "" | Out-File $outputFile -Append
        (iex "$gitExe log --name-status $commits") | Out-File $outputFile -Append
        Pop-Location
    }
}
