# Use this script to run tests remotely on an ARM64 dev machine
# grabs your arm64_debug build for the tests

param(
    [string]$binflavor="arm64_debug",
    [switch]$showSkips=$false,
    [switch]$createcsv=$false,
    [switch]$debugOut=$false
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
echo "    <?xml version=`"1.0`" encoding=`"utf-8`"?>
<Project ToolsVersion=`"15.0`" xmlns=`"http://schemas.microsoft.com/developer/msbuild/2003`">
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
    [string]$testdirbase = Join-Path $machineShare "alltests";
    [string]$testfullbase = Join-Path $testdirbase "unittests";
    [string]$testcorebase = Join-Path $testdirbase "tests";
    [string]$bindir = Join-Path $machineShare $binflavor;
    echo "Copying tests...";
    # Copy all of our tests - this means that there's going to be a couple minutes
    # of copying files the first time, but that changing test sets is faster later
    # Flags are Mirror NoJobHeader NoJobSummary NoProgressbar NoDirectoryList
    robocopy /MIR /NJH /NJS /NP /NDL $chakraFullDir\core\test $testcorebase;
    robocopy /MIR /NJH /NJS /NP /NDL $chakraFullDir\unittest $testfullbase;
    echo "Copying binaries...";
    robocopy /MIR /NJH /NJS /NP /NDL $chakraFullDir\Build\VcBuild\bin\$binflavor $bindir;
    echo "Invoking test runner...";
    Invoke-Command -ComputerName $machineName -ArgumentList $machineShare,$bindir,$testcorebase,$testfullbase,$constantSwitches,$showSkips,$debugOut -ErrorAction Stop -ScriptBlock {
        param([string]$machineShare, [string]$bindir, [string]$testcorebase, [string]$testfullbase, [string]$constantSwitches, [bool]$showskips, [bool]$debugOut);
        echo "Remote test runner started";
        # A few of the arm64 machines have postmortem debuggers set up.
        # In general, this is good; however, we hit an issue when doing
        # test runs, as the debugger will get spawned and just hang our
        # run. To work around this, the script will kill windbg when it
        # is running during the test run!
        echo "Preventing Just-In-Time debugger interference"
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

        echo "Setting up test run...";

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

        # We need to figure out the local path to the share, because cmd is old
        [string]$wdtemp = $testfullbase.Replace("\\","");
        $wdtempsegs = $wdtemp.Split("\");
        [string]$computername = $wdtempsegs[0];
        [string]$sharename = $wdtempsegs[1];
        $shares = Get-WmiObject -Class Win32_Share;
        $share = $shares | Where-Object { $_.Name -eq $sharename };
        $workingDirectory = $share.Path;
        for($i=2; $i -lt $wdtempsegs.Length; $i++)
        {
            $workingDirectory = Join-Path $workingDirectory $wdtempsegs[$i];
        }

        if($debugOut)
        {
            echo "computername is $computername";
            echo "sharename is $sharename";
            echo "workingdirectory is $workingDirectory";
        }

        # Now we can get to actually running jshost on these
        [string]$rlbin = Join-Path $testcorebase "runtests.cmd";

        # Arg setup
        [string]$binaryName = "jshost.exe";
        [string]$bindirarg = Join-Path $bindir "..";
        #$failingDirs = "`"" + "Array,AsmJs,AsyncDebug,bailout,Basics,Bugs,crossthread,Date,Debugger,DebuggerCommon,Error,ErrorCommon,es6,FixedFields,Function,HeapEnum,host,iasd,InlineCaches,InternalProfile,Intl,IntlCore,jd,Miscellaneous,msrc,NativeUnitTests,Object,Opegen,Operators,Optimizer,Profiler,sca,StackTrace,strict,Strings,SunSpider,SunSpider1.0.2,SunSpiderFunctionality,UnifiedRegex,V8,V8strict,V8_Functionality" + "`"";
        [string]$dirs = "`"" + "fieldopts,JSON,KrakenFunctionality,Lib,loop,Math,Regex,typedarray,utf8" + "`"";
        [string]$extraflags = "`"-oopjit-`"";

        $stdoutloc = New-TemporaryFile;
        $stderrloc = New-TemporaryFile;

        echo "Starting test run... (output will display all at once at the end)";

        $rlproc = Start-Process -FilePath $rlbin -ArgumentList "-arm64","-debug","-binary",$binaryName,"-bindir",$bindirarg,"-dirs",$dirs,"-extrahostflags",$extraflags -Wait -WorkingDirectory $workingDirectory -RedirectStandardOutput $stdoutloc.FullName -RedirectStandardError $stderrloc.FullName;

        [string]$output = Get-Content -Raw -Path $stdoutloc.FullName;
        [string]$error = Get-Content -Raw -Path $stderrloc.FullName;
        [int]$exitcode = $rlproc.ExitCode;

        echo $output;
        echo $error;

        echo "Re-enabling JIT debugging...";
        Stop-Job $windbgkiller;
    }
}
catch [System.Management.Automation.Remoting.PSRemotingTransportException]
{
    echo "Connection failure - did you run Enable-PSRemoting on the target machine?";
    exit -1;
}
echo "Successfully finished";
exit 0;