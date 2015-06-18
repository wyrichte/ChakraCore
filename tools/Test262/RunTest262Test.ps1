$test262SetupConfigFilePath     = "{0}\ChakraTest262.config" -f $Env:USERPROFILE
$global:t262Root                = "{0}\Test262" -f $Env:SystemDrive
$global:t262ConfigFilePath      = "{0}\t262.js" -f $global:t262Root
$t262PublicRepo                 = "https://github.com/tc39/test262"
$global:exePath                 = ".\Jshost.exe"
$global:exeArgs                 = "-es6all"
$batchSize                      = 75
$global:runFolder               = $null
$global:knownFailuresListFolder = "\\bptserver1\users\tools\Test262"
$global:outputFile              = ".\Test262Results.log"

$failedTCs = @()
$global:actualErrors = @()
$global:actualErrorMessages = @()

Function Check-Environment
{
    # Make sure Node is already installed
    if ((Get-Command "node" -ErrorAction SilentlyContinue) -eq $null) 
    { 
        Write-Host "Node is not installed on this machine! Please install node and re-run." -ForegroundColor Red
        return $false
    }

    # Make sure git is already installed
    if ((Get-Command "git" -ErrorAction SilentlyContinue) -eq $null) 
    { 
        Write-Host "Git is not installed on this machine! Please install git and re-run." -ForegroundColor Red
        return $false
    }

    return $true
}

Function Print-Usage
{
    Write-Host "powershell RunTest262Test.ps1 {-bin|-b} <full path to your exe> [{-config|-c} <config keys or args to use with the exe>] [{-batchsize|-b} <number of scripts to group together>] [-knownfailures <path to the file containing test cases that are expected to fail>] [-output <Path to the output file>]" -ForegroundColor Yellow
    Write-Host "bin           : Specify the complete path to your exe. e.g.: C:\test262\bin\jshost.exe" -ForegroundColor Yellow
    Write-Host "config        : Specify the args to be used with your exe. e.g.: -es6all. This is the default value." -ForegroundColor Yellow
    Write-Host "batchsize     : Specify the size of the tests to group together. e.g.: 75. 75 is the default value." -ForegroundColor Yellow
    Write-Host "knownFailures : Path to the dir which has files with test cases expected to fail. Default is $knownFailuresListFolder. Maker sure the files should be named KnownFailures.txt or KnownFailures-Es6All.txt etc." -ForegroundColor Yellow
    Write-Host "output        : File to dump the actual test output. Default path is .\Test262Results.log" -ForegroundColor Yellow
}

Function Parse-Args ($cmdArgs)
{
    for ($i = 0; $i -lt $cmdArgs.Length; $i++)
    {
        if (($cmdArgs[$i] -eq "-bin") -or ($cmdArgs[$i] -eq "-b"))
        {
            $i++
            $bin = $cmdArgs[$i]
            if (-not (Test-Path($bin)))
            {
                Write-Host "Executable not found at $bin" -ForegroundColor Red
                return $false
            }
            else
            {
                $global:exePath = $bin
            }
        }
        elseif (($cmdArgs[$i] -eq "-config") -or ($cmdArgs[$i] -eq "-c"))
        {
            $i++
            $global:exeArgs = $cmdArgs[$i]
        }
        elseif (($cmdArgs[$i] -eq "-batchsize") -or ($cmdArgs[$i] -eq "-s"))
        {
            $i++
            $batchSize = $cmdArgs[$i]
        }
        elseif (($cmdArgs[$i] -eq "-dir") -or ($cmdArgs[$i] -eq "-d"))
        {
            $i++
            $global:runFolder = $cmdArgs[$i]
        }
        elseif (($cmdArgs[$i] -eq "-knownfailures") -or ($cmdArgs[$i] -eq "-f"))
        {
            $i++
            $global:knownFailuresListFolder = $cmdArgs[$i]
        }
        elseif (($cmdArgs[$i] -eq "-output") -or ($cmdArgs[$i] -eq "-o"))
        {
            $i++
            $global:outputFile = $cmdArgs[$i]
        }
        else
        {
            Write-Host "Unexpected argument " ($cmdArgs[$i]) -ForegroundColor Red
            Print-Usage
            exit 1
        }
    }

    return $true
}

Function Create-ConfigFile ($rootDir)
{
    $defaultConfigPath = "{0}\t262.js" -f $rootDir

    $configValue = "var t262 = require('test262-harness'); t262.useConfig({ runner: 'console', consolePrintCommand: 'WScript.Echo', consoleCommand: '"
    $configValue += "$global:exePath $global:exeArgs" 
    $configValue += "', batch: $batchSize"
    $configValue += ", batchConfig: {  createEnv: 'WScript.LoadScript("""", ""samethread"")', runBatched: 'env.WScript.LoadScript(test)' },  test262Dir: 'test262' });"
    Set-Content -Encoding Ascii -Path $defaultConfigPath -Value $configValue

    return $defaultConfigPath
}

Function Setup-ConfigFile
{
    Write-Host "Couldn't find the Test262 configuration file at $test262SetupConfigFilePath. Starting the setup"

    $setupChoice = 0
    while ($true)
    {
        Write-Host "Do you want to "
        Write-Host "1. Setup test262 repo yourself?"
        Write-Host "2. Already have a repo and want to use it?"
        Write-Host "3. Use all default options and let the setup file get everything ready for you?"
        Write-Host "4. Skip this step?"
        $setupChoice = Read-Host "Pick your number"

        if ($setupChoice -eq 1)
        {
            Write-Host "All right! Re-run the setup after you are done setting up the repo." -ForegroundColor Yellow
            exit 0
        }
        elseif ($setupChoice -eq 2)
        {
            $p1 = Read-Host "Enter the path to your Test262 repo (pls no backslash at the end)"
            if (-not (Test-Path($p1)))
            {
                Write-Host "Not able to access the path $p1" -ForegroundColor Red
                continue
            }
            $p2 = Read-Host "Enter the path to your Test262 config file"
            if ((-not $p2) -or (-not (Test-Path($p2))))
            {
                $c1 = Read-Host "Do you want to create a default one (y/n)?"
                if ($c1 -eq "y")
                {
                    $p2 = Create-ConfigFile $p1
                    Write-Host "Created a default config file at $p2"
                }
                else
                {
                    continue
                }
            }

            $global:t262Root = $p1
            $global:t262ConfigFilePath = $p2

            break
        }
        elseif ($setupChoice -eq 3)
        {
            $p1 = Read-Host "Enter the path where you want to setup the Test262 repo"
            if (-not (Test-Path($p1)))
            {
                mkdir $p1
            }
            $global:t262Root = $p1

            pushd $global:t262Root
            # Setup the repo
            git clone $t262PublicRepo --depth 1
            npm install -g test262-harness
            npm install test262-harness
            # Create the config file
            $global:t262ConfigFilePath = Create-ConfigFile $global:t262Root
            popd

            break
        }
        elseif ($setupChoice -eq 4)
        {
            Write-Host "Ok! You are on your own!"
            break
        }
        else
        {
            Write-Host "Wrong option $setupChoice"
        }
    }

    if ($setupChoice -ne 4)
    {
        # Create the configuration file to load next time
        $configFileContent = @()
        $configFileContent += "root={0}" -f $global:t262Root
        $configFileContent += "config={0}" -f $global:t262ConfigFilePath
        Set-Content -Encoding Ascii -Path $test262SetupConfigFilePath -Value $configFileContent
    }
}

Function Read-ConfigFile
{
    if (-not (Test-Path($test262SetupConfigFilePath)))
    {
        Setup-ConfigFile
    }

    $lines = Get-Content $test262SetupConfigFilePath
    foreach ($line in $lines)
    {
        if ($line)
        {
            $parts = $line.Split("=")
            if ($parts.Length -eq 2)
            {
                if ($parts[0] -eq "root")
                {
                    $global:t262Root = $parts[1]
                }
                elseif ($parts[0] -eq "config")
                {
                    $global:t262ConfigFilePath = $parts[1]
                }
            }
        }
    }

    Write-Host "Test262 root dir    : " $global:t262Root
    Write-Host "Test262 config file : " $global:t262ConfigFilePath
}

Function Runt-TestCases
{
    Write-Host "Starting the test"
    pushd $global:t262Root

    # Get the latest
    # git pull
    ## TODO: Option to always do a pull before the run
    ## TODO: Options to report merge conflicts
    ## TODO: Option to use a specific branch

    if (-not (Test-Path($global:exePath)))
    {
        Write-Host "Couldn't find the exe at " $global:exePath -ForegroundColor Red
        return $null
    }
    else
    {
        Write-Host "Using the executable from : " $global:exePath
        Write-Host "Using the config : " $global:exeArgs
        if ($global:runFolder)
        {
            Write-Host "Running tests from the folder : " $global:runFolder
        }
    }

    Write-Host "Test262-harness -c $global:t262ConfigFilePath -e '$global:exePath $global:exeArgs' $global:runFolder -R tap"
    $results = (Test262-harness -c $global:t262ConfigFilePath -e "$global:exePath $global:exeArgs" $global:runFolder -R tap)
    # $results = (Test262-harness -c $global:t262ConfigFilePath -e "$global:exePath $global:exeArgs" language -R tap)
    
    Write-Host "Writing entire results to $global:outputFile"
    Set-Content -Encoding Ascii -Path $global:outputFile -Value $results

    $failedTCs = Parse-Failures $results
    popd
    return $failedTCs
}

Function Parse-Failures($results)
{
    $summary = @()
    $failedTCs = @()
    for ($i = 0; $i -lt $results.Length; $i++)
    {
        if ($results[$i].StartsWith("not ok "))
        {
            $failedTC = ""
            # The format is like below
            #not ok 13115 Evaluating LeftHandSideExpression lref returns Reference type; Reference base value is an environment record and environment record kind is declarative environment record. PutValue(lref, v) uses the initially created Reference even if a more local binding is available. Check operator is "x ^= y".
            #   ---
            #   file:         test262/test/language/expressions/compound-assignment/S11.13.2_A6.10_T1.js
            #   errorName:    Test262Error
            #   errorMessage: |
            #   #1: innerX === 2. Actual: 5
            #   ...
            #   OR
            #not ok 9379 function declarations in statement position in strict mode: for ( ;;) Statement  (Strict Mode)
            #   ---
            #   file: test262/test/language/block-scope/syntax/function-declarations/in-statement-position-for-statement.js
            #   ...
            $i += 2
            $parts = $results[$i].Split(":")
            $failedTCs += $parts[1].Trim() # File name
            if (($results[$i + 1]).Trim() -eq "...")
            {
                $global:actualErrors += ""
                $global:actualErrorMessages += ""
            }
            else
            {
                $i++
                $parts = $results[$i].Split(":")
                $global:actualErrors += $parts[1].Trim() # errorName
                $i++
                $errorMessage = ""
                $parts = $results[$i].Split(":")
                $errorMessage += $parts[1].Trim() # errorMessage can be multiple lines
                $i++
                while (($results[$i]).Trim() -ne "...")
                {
                    $errorMessage += ($results[$i]).Trim()
                    $i++
                }
                $global:actualErrorMessages += $errorMessage
            }
        }
        elseif ($results[$i].StartsWith("# "))
        {
            $summary += $results[$i]
        }
    }

    Write-Host "Summary : " 
    Write-Host $summary

    return $failedTCs
}

Function Get-KnownFailureListFile
{
    return "{0}\KnownFailures{1}.txt" -f $global:knownFailuresListFolder, $global:exeArgs
}

Function Summarize-Results ($global:failedTCsArray)
{
    [System.Collections.ArrayList]$failedTCs = $global:failedTCsArray
    [System.Collections.ArrayList]$ae = $global:actualErrors
    [System.Collections.ArrayList]$aem = $global:actualErrorMessages
    $knownFailuresList = Get-KnownFailureListFile
    if (-not (Test-Path($knownFailuresList)))
    {
        Write-Host "Known failure list is not found at $knownFailuresList" -Foreground Red
        return
    }
    else
    {
        Write-Host "Comparing the results with the known failure list at " $knownFailuresList
    }

    $passed = $true
    $knownFailures = Get-Content $knownFailuresList

    Write-Host "Expected count of test cases that fails : " $knownFailures.Length
    Write-Host "Actual number of test cases that failed : " $failedTCs.Count

    for ($i = 0; $i -lt $knownFailures.Length; $i++)
    {
        $found = $false
        $exactMatch = $false
        $failureResultParts = ($knownFailures[$i]) -split "::::"

        $tc = $failureResultParts[0].Trim()
        if (($global:runFolder) -and ($tc.IndexOf($global:runFolder) -lt 0))
        {
            # Do nothing
        }
        else
        {
            for ($j = 0; $j -lt $failedTCs.Count; $j++)
            {
                if ($tc -eq $failedTCs[$j])
                {
                    $found = $true
                    if (($ae[$j] -eq $failureResultParts[1]) -and ($aem[$j] -eq $failureResultParts[2]))
                    {
                        $exactMatch = $true
                    }
                    break
                }
            }

            if (-not $found)
            {
                Write-Host "The test case $tc is expected to fail" -ForegroundColor Red
                $passed = $false
            }
            else
            {
                $failedTCs.Remove($tc)
                if (-not $exactMatch)
                {
                    Write-Host "The output of the test case $tc is not matching its baseline" -ForegroundColor Red
                    $passed = $false
                }
                $ae.RemoveAt($j)
                $aem.RemoveAt($j)
            }
        }
    }

    if ($failedTCs.Count -gt 0)
    {
        Write-Host "Unexpected Failures : " -ForegroundColor Red
        for ($i = 0; $i -lt $failedTCs.Count; $i++)
        {
            Write-Host $failedTCs[$i] -ForegroundColor Red
        }
    }
    elseif ($passed)
    {
        Write-Host "No unknown failures" -ForegroundColor Green
    }
    else
    {
        Write-Host "Found Errors! Please look at the results at $global:t262Root\$global:outputFile" -ForegroundColor Red
    }

    # TODO: Create a build*.log like file so that RunAllRLTests can merge the results
}


########### MAIN #############

if (-not (Check-Environment))
{
    Write-Host "Exiting the Test262 with errors in setup" -ForegroundColor Red
    exit 1
}

if (-not (Parse-Args $args))
{
    Write-Host "Error while parsing the arguments" -ForegroundColor Red
    exit 1
}

Read-ConfigFile
$failedTCs = Runt-TestCases
Summarize-Results $failedTCs

########### MAIN #############
