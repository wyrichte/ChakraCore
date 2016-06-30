param (
    [Parameter(Mandatory=$True)]
    [ValidateSet("x86", "x64", "arm", "*")]
    [string]$arch,

    [Parameter(Mandatory=$True)]
    [ValidateSet("debug", "release", "test", "codecoverage", "*")]
    [string]$flavor,

    [ValidateSet("default", "codecoverage", "pogo")]
    [string]$subtype = "default",

    [string]$srcpath = "",
    [string]$binpath = "",
    [string]$objpath = "",
    [string]$srcsrvcmdpath = "Build\script\srcsrv.bat",
    [string]$logFile = "",
    [string[]]$pogo = @("x86","test","x64","test"),
    [switch]$noaction
)

$CoreScriptDir = "$PSScriptRoot\..\..\core\build\scripts"

$OuterScriptRoot = $PSScriptRoot
. "$CoreScriptDir\pre_post_util.ps1"
$bvtcmdpath = "$srcpath\tools\runcitests.cmd"
$pogoscript = "$srcpath\tools\pogo.git.bat"

$noactionSwitchString = ""
if ($noaction) {
    $noactionSwitchString = "-noaction"
}

$pogoList = $pogo -join ','
$postBuildCommmand = "$CoreScriptDir\post_build.ps1 -repo full -arch $arch -flavor $flavor -subtype $subtype -srcpath `"$srcpath`" -binpath `"$binpath`" -objpath `"$objpath`" -srcsrvcmdpath `"$srcsrvcmdpath`" -bvtcmdpath `"$bvtcmdpath`" $noactionSwitchString -logFile `"$logFile`" -pogo $pogoList -pogoscript `"$pogoscript`""

iex $postBuildCommmand

exit $LastExitCode
