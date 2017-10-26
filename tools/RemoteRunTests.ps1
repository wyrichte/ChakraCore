# Use this script to run tests remotely on an ARM64 dev machine
# grabs your arm64_debug build for the tests

param(
[string]$binflavor="arm64_debug",
[switch]$showSkips=$false
);
$configpath = (Join-Path (Join-Path $Env:UserProfile "ChakraDevConfig") "Chakra.Build.user.props");

try
{
    [xml]$config = Get-Content -Path $configpath;
}
catch [System.Management.Automation.ItemNotFoundException]
{
    echo "Didn't find a Chakra remote development config!";
    echo "Try making $configpath with the following template (replace the machine name with your target machine and the share with a new share):";
echo "    <?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <ARMRemoteMachineName>chakracobaltX</ARMRemoteMachineName>
    <ARMRemoteShare>\\`$(ARMRemoteMachineName)\DevelopmentBinaries</ARMRemoteShare>
  </PropertyGroup>
</Project>";
    exit -1;
}
[string]$machineName = $config.Project.PropertyGroup.ARMRemoteMachineName;
[string]$machineShare = $config.Project.PropertyGroup.ARMRemoteShare.Replace("`$(ARMRemoteMachineName)",$machineName);
[string]$chakraFullDir = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."));
[string]$constantSwitches = "-mic:1 -off:simplejit -oopjit-";

echo "Testing network to $machineName...";
if(!(Test-Connection -ComputerName $machineName -Quiet))
{
    echo "%machinename failed to respond to pings";
    exit -1;
}
echo "Establishing connection and attempting remote work...";
try
{
    [string]$testdir = Join-Path $machineShare "SunSpiderFunctionality";
    [string]$bindir = Join-Path $machineShare $binflavor;
    echo "Copying tests...";
    # Mirror NoJobHeader NoJobSummary NoProgress NoDirectoryList
    robocopy /MIR /NJH /NJS /NP /NDL $chakraFullDir\unittest\SunSpiderFunctionality $testdir;
    echo "Copying binaries...";
    robocopy /MIR /NJH /NJS /NP /NDL $chakraFullDir\Build\VcBuild\bin\$binflavor $bindir;
    echo "Invoking test runner...";
    Invoke-Command -ComputerName $machineName -ArgumentList $machineShare,$bindir,$testdir,$constantSwitches,$showSkips -ErrorAction Stop -ScriptBlock {
        param([string]$machineShare, [string]$bindir, [string]$testdir, [string]$constantSwitches, [bool]$showskips);
        echo "Remote test runner started";
        # A lot of the arm64 machines have postmortem debuggers set up.
        # In general, this is good; however, we hit an issue when doing
        # test runs, as the debugger will get spawned and just hang our
        # run. To work around this, the script will kill windbg when it
        # is running during the test run!
        try {
            $currentprocs = Get-Process -Name windbg -ErrorAction Stop;
            # throws exception if there is one
            echo "Can't run tests while windbg is up on target!";
            exit -1;
        }
        catch [Microsoft.PowerShell.Commands.ProcessCommandException]
        {
            echo "Windbg not currently running on target, will kill any that spawn during run...";
        }
        
        # Now we can get to actually running jshost on these
        [string]$chakraBin = Join-Path $bindir "jshost.exe";

        # Helper function to run a single test
        function runTest([string]$testfile, [string]$additionalArgs, [string]$baseline, [REF]$res)
        {
            [string]$testFileFullPath = Join-Path -Path $testdir -ChildPath $testfile;
            $stdoutloc = New-TemporaryFile;
            $stderrloc = New-TemporaryFile;

            echo "Running `"$chakraBin $constantSwitches $additionalArgs $testFileFullPath`" ...";
            if([string]::IsNullOrWhiteSpace($additionalArgs))
            {
                $additionalArgs = " ";
            }
            $Process = Start-Process -FilePath $chakraBin -ArgumentList $constantSwitches,$additionalArgs,$testFileFullPath -RedirectStandardOutput $stdoutloc.FullName -RedirectStandardError $stderrloc.FullName -Wait;
            
            [string]$output = Get-Content -Raw -Path $stdoutloc.FullName;
            [string]$error = Get-Content -Raw -Path $stderrloc.FullName;
            [int]$exitcode = $Process.ExitCode;

            if(![string]::IsNullOrWhiteSpace($error) -or ($exitcode -ne 0))
            {
                echo "Test terminated with exit code $exitcode";
                echo "stdout:";
                echo $output;
                echo "stderr:";
                echo $error;
                $res.value = $false;
                return;
            }

            if(![string]::IsNullOrWhiteSpace($baseline))
            {
                [string]$baselinepath = Join-Path -Path $testdir -ChildPath $baseline;
                [string]$baselinecontents = Get-Content -Path $baselinepath -Raw;
                if($baselinecontents -ne $output) {
                    echo "Test completed without error code, but baselines differ!";
                    fc.exe $baselinepath $stdoutloc.FullName;
                    $res.Value = $false;
                    return;
                }
            }

            if([string]::IsNullOrWhiteSpace($error) -and ($exitcode -eq 0))
            {
                $res.value = $true;
                return;
            }
        };

        echo "Running selected tests in $testdir";

        # start job to kill windbg
        $windbgkiller = Start-Job -ScriptBlock {
            while($true)
            {
                # Since it's likely that the arm64 machines (at the moment)
                # have windbg as a postmortem debugger, we need to kill the
                # process when they start, as otherwise the tests will lock
                # up on that particular run, and the user will have to kill
                # windbg from the task manager themselves.
                try {
                    $currentprocs = Get-Process -Name windbg -ErrorAction Stop;
                    for($i = 0; $i -lt $currentprocs.Count; $i++)
                    {
                        Stop-Process $currentprocs[$i];
                    }
                }
                catch [Microsoft.Powershell.Commands.ProcessCommandException]
                {
                    # No windbg running, so no action necessary
                }
                Start-Sleep -Seconds 1
            }
        }

        # parse rlexe for the flags so we don't have to update them in two places
        [xml]$rlexe = Get-Content -Path (Join-Path $testdir "rlexe.xml");

        $tests = $rlexe.'regress-exe'.test;
        $enabledtests = `
        "3d-morph.js", `
        "access-binary-trees", `
        "access-fannkuch.js", `
        "access-nbody.js", `
        "access-nsieve.js", `
        "bitops-3bit-bits-in-byte.js", `
        "bitops-bits-in-byte.js", `
        "bitops-bitwise-and.js", `
        "bitops-nsieve-bits.js", `
        "crypto-md5.js", `
        "crypto-sha1.js", `
        "math-partial-sums.js", `
        "regexp-dna.js", `
        "string-base64.js", `
        "string-fasta.js", `
        "string-tagcloud.js", `
        "string-validate-input.js", `
        "TerminatorDoNotRemoveThisReducesDiffComplexityALittle";
        [int]$total = 0;
        [int]$totalpassed = 0;
        foreach($test in $tests) {
            [bool]$cond = $false;
            [string]$file = $test.default.files.Trim();
            foreach($enabled in $enabledtests)
            {
                if($enabled.Trim().ToLower() -eq $file.ToLower())
                {
                    $cond = $true;
                    break;
                }
            }
            if(!$cond)
            {
                if($showskips)
                {
                    echo "Skipping $file, as it's not selected...";
                }
                continue;
            }
            [string]$flags = $test.default.'compile-flags';
            if([string]::IsNullOrWhiteSpace($flags))
            {
                $flags = "";
            }
            else
            {
                $flags = $flags.Trim();
            }
            if($flags.ToLower().Contains("testetwdll"))
            {
                if($showskips)
                {
                    echo "Skipping ETW variant of test, as it is not currently supported...";
                }
                continue;
            }
            [string]$baseline = $test.default.baseline;
            if([string]::IsNullOrWhiteSpace($baseline))
            {
                $baseline = null;
            }
            else
            {
                $baseline = $baseline.Trim();
            }
            [bool]$res = $false;
            runTest $file $flags $baseline ([REF]$res);
            if($res)
            {
                $totalpassed = $totalpassed + 1;
                echo "PASSED";
            }
            else
            {
                echo "FAILED";
            }
            $total = $total + 1;
        }
        Stop-Job $windbgkiller;
        echo "Passed $totalpassed/$total tests";
    }
}
catch [System.Management.Automation.Remoting.PSRemotingTransportException]
{
    echo "Connection failure - did you run Enable-PSRemoting on the target machine?";
    exit -1;
}