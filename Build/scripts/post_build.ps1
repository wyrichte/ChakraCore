param (
    [ValidateSet("x86", "x64", "arm")]
    [string]$arch="",

    [ValidateSet("debug", "release", "test")]
    [string]$flavor = "",
    
    [string]$srcpath = ".\",
    [string]$binpath = "Build\VcBuild\bin\$arch_$flavor",

    [string]$srcsrvcmdpath = "Build\script"
)

$repo = "full"

if ($repo -eq "core") {
    $bvtcmd="test\runcitests.cmd"
} elseif ($repo -eq "full") {
    $bvtcmd="tools\runcitests.cmd"
} else {
    write-error Unknow repo $repo
    exit -1;
}

$srcsrvcmd = "$srcsrvcmdpath\srcsrv.bat"
$pogocmd = ""


$exitcode = 0

# generate srcsrv
if ($srcsrvcmd -ne "" -and (Test-Path $srcsrvcmd) -and (Test-Path $srcpath) -and (Test-Path $binpath)) {
    $cmd = "$srcsrvcmd $repo $srcpath $binpath\*.pdb"
    write-host Running $cmd
    invoke-expression $cmd 
    write-host "ExitCode:" $lastexitcode
    if($lastexitcode -ne 0) {
        Write-Error "Failed"
        $exitcode = $lastexitcode
    }
}

# do PoGO
if ($pogocmd -ne "") {
    $cmd = "$pogocmd"
    write-host Running $cmd
    invoke-expression $cmd 
    write-host "ExitCode:" $lastexitcode
    if($lastexitcode -ne 0) {
        Write-Error "Failed"
        $exitcode = $lastexitcode
    }
}


# run test
if ($bvtcmd -ne "") {
    $cmd = "$bvtcmd -$arch$flavor"
    write-host Running $cmd
    invoke-expression $cmd 
    write-host "ExitCode:" $lastexitcode
    if($lastexitcode -ne 0) {
        Write-Error "Failed"
        $exitcode = $lastexitcode
    }
}

exit $exitcode