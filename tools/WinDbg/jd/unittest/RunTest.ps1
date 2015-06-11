#
# Simulate rl.exe to run some jd unit tests
#
Param(
    [string]$debugger = "cdb.exe",
    [string]$TestPath = $(Split-Path -parent $MyInvocation.MyCommand.Definition)
)

function ExitError($msg)
{
    Write-Error $msg
    exit -1
}

# Locate jshost.exe and jd.dll
if ($env:_NTTREE) {
    $jsbuild = Join-Path $env:_NTTREE jscript
    if ($env:Path.indexOf($jsbuild) -lt 0) {
        $env:Path += ";$jsbuild"
    }
}
$jshostpath = where.exe jshost.exe
if (!$jshostpath) {
    ExitError "Can't locate jshost.exe. Please build jshost.exe and add to your path."
} elseif (!(Test-Path -PathType Leaf "$(Split-Path -Parent $jshostpath)\jd.dll")) {
    ExitError "Can't locate jd.dll. Please build jd."
}

# Locate debugger cdb.exe
if (!(Test-Path -PathType Leaf $debugger)) {
    [void](where.exe /Q $debugger)
    if ($LASTEXITCODE -ne 0) {
        ExitError "Can't find debugger in either enlistment or path"
    }
}

# Setup test flavors
$flavors = @(
    @{Name="Interpreted"; Flag="-NoNative" },
    @{Name="Native"; Flag="-ForceNative" }
)

# Retrieve tests
$testlist = Join-Path $TestPath rlexe.xml
if (!(Test-Path -PathType Leaf $testlist)) {
    ExitError "No rlexe.xml in $TestPath"
}

$xml = New-Object XML
$xml.Load($testlist)

$tests = $xml."regress-exe".test
foreach ($flavor in $flavors) {
    Write-Host "Running $($flavor.Name)..."
    $i = 0
    $failed = 0
    foreach ($test in $tests) {
        $test = $test.default

        if ($test.tags -and ($test.tags -match "exclude_$($flavor.Name)")) {
            continue
        }

        $command = $test.command
        $flags = @(-split $test."compile-flags") + $flavor.Flag
        $testfile = Join-Path $TestPath $test.files
        $testout = "$testfile.out.$($flavor.Name).$i"

        echo "$debugger -c `"$command`" jshost.exe $flags $testfile"

        # Only output _js_ lines and replace paths to enable baseline comparison
        $bphit = $false
        $pathpattern = "$testfile" -replace '\\','\\'
        $pathreplace = "`$TestPath\$($test.files)"
        &$debugger -c $command jshost.exe @flags $testfile | %{
            $line = $_
            if ($line -match "^_js_!.+$") {
                if ($line -match "^_js_!.*\(${pathpattern}:.+\)$") { # function name (url:line,column)
                    $line -replace "(^_js_!.*\()${pathpattern}(:.+\))$","`$1$pathreplace`$2"
                } else { # function name only
                    $line
                }
            }
        } > $testout

        # Compare with baseline
        $keeptestout = $false
        if ($test.baseline) {
            $testbaseline = Join-Path $TestPath $test.baseline
            if (!(Test-Path $testbaseline)) {
                Write-Warning "Baseline file not found. Create baseline."
                $keeptestout = $true
                mv $testout $testbaseline
            } elseif (diff $(Get-Content $testbaseline) $(Get-Content $testout)) {
                $failed++
                Write-Warning "Test output different to baseline: $testbaseline $testout"
                $keeptestout = $true
            }
        }
        if (!$keeptestout) {
            rm $testout
        }

        $i++
    }

    $flavor.Total = $i
    $flavor.Passed = $i - $failed
    $flavor.Failed = $failed
    Write-Host
}

foreach ($flavor in $flavors) {
    Write-Host "*** $($flavor.Name) ***"
    Write-Host "    Total:  $($flavor.Total)"
    Write-Host "    Passed: $($flavor.Passed)" -ForegroundColor green
    if ($flavor.Failed) {
        Write-Host "    Failed: $($flavor.Failed)" -ForegroundColor red
    }
    Write-Host
}
