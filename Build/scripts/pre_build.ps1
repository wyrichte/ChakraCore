param (
    [string]$srcpath = "",
    [string]$binpath = "",
    [string]$objpath = "",
    [string]$oauth
)

$CoreScriptDir = "$PSScriptRoot\..\..\core\build\scripts"

$OutterScriptRoot = $PSScriptRoot;
. "$CoreScriptDir\pre_post_util.ps1"

& $PSScriptRoot\..\..\core\Build\Scripts\pre_build.ps1 -oauth $oauth -srcpath $srcpath -binpath $binpath -objpath $objpath


