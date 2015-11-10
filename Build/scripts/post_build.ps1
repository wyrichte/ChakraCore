param (
    [ValidateSet("x86", "x64", "arm", "*")]
    [string]$arch="",

    [ValidateSet("debug", "release", "test", "*")]
    [string]$flavor = "",
    
    [string]$srcpath = "",
    [string]$binpath = "",
    [string]$objpath = "",
    [string]$srcsrvcmdpath = "Build\script\srcsrv.bat",
    [string]$logFile = "",
    [switch]$noaction
)

$CoreScriptDir = "$PSScriptRoot\..\..\core\build\scripts"

$OutterScriptRoot = $PSScriptRoot;
. "$CoreScriptDir\pre_post_util.ps1"
$bvtcmdpath = "$srcpath\tools\runcitests.cmd"

if ($noaction) {
    & $CoreScriptDir\post_build.ps1 -repo "full" -arch $arch -flavor $flavor -srcpath $srcpath -binpath $binpath -objpath $objpath -srcsrvcmdpath $srcsrvcmdpath -bvtcmdpath $bvtcmdpath -noaction -logFile ""$logFile""
} else {
    & $CoreScriptDir\post_build.ps1 -repo "full" -arch $arch -flavor $flavor -srcpath $srcpath -binpath $binpath -objpath $objpath -srcsrvcmdpath $srcsrvcmdpath -bvtcmdpath $bvtcmdpath -logFile ""$logFile""
}
exit $LastExitCode

