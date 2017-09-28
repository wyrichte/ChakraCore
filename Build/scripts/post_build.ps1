param (
    [Parameter(Mandatory=$True)]
    [ValidateSet("x86", "x64", "arm", "arm64", "*")]
    [string]$arch,

    [Parameter(Mandatory=$True)]
    [ValidateSet("debug", "release", "test", "codecoverage", "*")]
    [string]$flavor,

    [ValidateSet("default", "codecoverage", "pogo")]
    [string]$subtype = "default",

    [string]$testparams = "",

    [string]$srcpath = "",
    [string]$binpath = "",
    [string]$objpath = "",
    [string]$srcsrvcmdpath = "Build\script\srcsrv.bat",
    [string]$logFile = "",
    [string[]]$pogo = @("x86","test","x64","test"),
    [switch]$noaction
)

$CoreScriptDir = "$PSScriptRoot\..\..\core\build\scripts"

. "$CoreScriptDir\pre_post_util.ps1"
$srcpath, $buildRoot, $objpath, $_ = `
    ComputePaths `
        -arch $arch -flavor $flavor -subtype $subtype -OuterScriptRoot $PSScriptRoot `
        -srcpath $srcpath -buildRoot $binpath -objpath $objpath

$bvtcmdpath = "$srcpath\tools\runcitests.cmd"
$pogoscript = "$srcpath\tools\pogo.git.bat"

$noactionSwitchString = ""
if ($noaction) {
    $noactionSwitchString = "-noaction"
}

$pogoList = $pogo -join ','
$postBuildCommmand = "$CoreScriptDir\post_build.ps1 -repo full -arch $arch -flavor $flavor -subtype $subtype -srcpath `"$srcpath`" -buildRoot `"$binpath`" -objpath `"$objpath`" -srcsrvcmdpath `"$srcsrvcmdpath`" -bvtcmdpath `"$bvtcmdpath`" -testparams `"$testparams`" $noactionSwitchString -logFile `"$logFile`" -pogo $pogoList -pogoscript `"$pogoscript`""

iex $postBuildCommmand

exit $LastExitCode
