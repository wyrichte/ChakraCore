Import-Module "\\bptstorage3\StressSuite\Setup\IEandWWA\ChakraStress.psm1"

$global:tag                 = "CCRun"
$global:binaryPath          = ""
$global:binaries            = @()
$global:runHours            = ""
$global:monitorRun          = $false
$global:dpkPath             = ""
$global:fileFilter          = ""
$global:type                = "BuddyRun"
$global:tool                = "CC"
$global:branch              = Get-Branch
$global:build               = Get-BuildNumber
$global:arch                = $Env:PROCESSOR_ARCHITECTURE
$global:flav                = "fre"
$global:copyShare           = $null
$global:publishFormat       = "html"
$global:mailTo              = "aneeshd@microsoft.com"

$global:sysDrive            = $Env:SystemDrive
$global:gumshoeDir          = "{0}\Program Files\Gumshoe" -f $global:sysDrive
$global:gumshoeExe          = "$global:gumshoeDir\gumshoe.exe"
$global:magellanDir         = "{0}\Chakramagellan" -f $global:sysDrive
$global:bbCoverExe          = "{0}\Magellan\BBCover.exe" -f $global:magellanDir
$global:GumshoeSetupPath    = "\\ocentral\products\Gumshoe\latest\setup\x64\gumshoe.msi"
$global:MagellanSetupPath   = "\\codecovfs01\Magellan_pre_Release\Latest\amd64fre"
$global:copyShareBase       = "\\chakrafs01\CodeCoverageLogs"
$global:fileFilterTemplate  = "\\chakrafs\fs\Tools\CodeCoverage\FilterFileTemplate.xml"

if (-not ($global:arch -eq "AMD64"))
{
    $global:GumshoeSetupPath = "\\ocentral\products\Gumshoe\latest\setup\x86\gumshoe.msi"
    $global:MagellanSetupPath = "\\codecovfs01\Magellan_pre_Release\Latest\x86fre"
}

Function Setup-Gumshoe
{
    #Check the Gumshoe installation
    if (-not (Test-Path("$global:gumshoeExe")))
    {
            Write-Host "Gumshoe not installed. Installing now ..."
            msiexec.exe /q /i "$global:GumshoeSetupPath" /log GumshoeSetup.log 
    }
    else
    {
            Write-Host "Gumshoe is already installed."
    }

    return $true
}

Function Setup-Magellan
{
    # Check magellan is already installed or not
    if (Test-Path($global:bbCoverExe))
    {
            Write-Host "magellan bits are already installed"
            return $true
    }

    Write-Host "Installing Magellan bits"
    # Copy magellan bits
    if (-not (Test-Path $global:magellanDir))
    {
        mkdir $global:magellanDir
    }
    xcopy /S /C /Q /Y $global:MagellanSetupPath $global:magellanDir

    if (-not (Test-Path($global:bbCoverExe)))
    {
            Write-Host "magellan installation failed" -ForegroundColor Red
            return $false
    }

    return $true
}

Function Send-Mail
{
    Send-MailMessage -to $emailTo -from "aneeshd@microsoft.com" -subject $subject -body $body -SmtpServer "smtphost.redmond.corp.microsoft.com"
}

Function Setup-CodeCoverage
{
    param(
            [string]$binaryList
    )

    if (-not (Setup-Gumshoe))
    {
            return $false
    }

    if (-not (Setup-Magellan))
    {
            return $false
    }

    # Stop in case there is already a session going on
    Write-Host "Stop active sessions if any"
    $output = cmd /c "$global:gumshoeExe" stop
    Write-Host $output

    if ($binaryList.Contains(","))
    {
        $parts = $binaryList.Split(",")
        foreach ($part in $parts)
        {
            if (Test-Path($part))
            {
                $global:binaries += $part
            }
            else
            {
                Write-Host "Couldn't find the binary at $part" -ForegroundColor Red
            }
        }
    }
    else
    {
        if (Test-Path($binaryList))
        {
            $global:binaries += $binaryList
        }
        else
        {
            Write-Host "Couldn't find the binary at $binaryList" -ForegroundColor Red
        }
    }

    if (($global:binaries) -and ($global:binaries.Length -gt 0))
    {
        foreach ($binary in $global:binaries)
        {
            # Instrumenting the binaries
            Write-Host "Instrumenting the binary $binary"
            $bbtfFile = "{0}.bbtf" -f $binary
            if (Test-Path($bbtfFile))
            {
                $output = cmd /c $global:bbCoverExe /block /CovSym /I $binary /Cmd $bbtfFile 2>&1
            }
            else
            {
                $output = cmd /c $global:bbCoverExe /block /CovSym /I $binary 2>&1	
            }
            Write-Host $output

            $binOldName = "{0}.old" -f $binary
            $binInstrName = "{0}.block.instr" -f $binary
            if (-not (Test-Path $binInstrName))
            {
                Write-Host "Instrumentation failed. No instrumented binary at " $binInstrName -ForegroundColor Red
            }
            else
            {
                Write-Host "Renaming the binary $binary to $binOldName"
                Move-Item -Force -Path $binary -Destination $binOldName
                Write-Host "Renaming the instrumented binary $binInstrName to $binary"
                Move-Item -Force -Path $binInstrName -Destination $binary
            }
        }
    }

    # Create the filter file if specified
    if ($global:fileFilter)
    {
        Write-Host "Creating the file filter at .\fileFilter.xml using the template $global:fileFilterTemplate and the filter $global:fileFilter"
        $filterSchema = Get-Content $global:fileFilterTemplate
        $filter = $filterSchema -f $global:fileFilter
        Set-Content -Encoding Ascii -Path ".\fileFilter.xml" -Value $filter
    }

    # Starting the Code Coverage session
    $output = cmd /c "$global:gumshoeExe" start 2>&1 
    Write-Host $output

    if (($global:binaries) -and ($global:binaries.Length -gt 0))
    {
        foreach ($binary in $global:binaries)
        {
            $covSymFile = "{0}.covsym" -f $binary
            $output = cmd /c "$global:gumshoeExe" add covsym:$covSymFile 2>&1
            Write-Host $output
        }
    }

    if ($global:dpkPath)
    {
            $output = cmd /c "$global:gumshoeExe" add dpk:$global:dpkPath 2>&1
        Write-Host $output
    }
    
    if ($global:fileFilter)
    {
        $output = cmd /c "$global:gumshoeExe" add filter:$global:fileFilter.xml 2>&1
        Write-Host $output
    }
    
    $output = cmd /c "$global:gumshoeExe" status 2>&1
    Write-Host $output

    # Start this script with no-wait and monitor flag, passing in the runHours
    # Start-MonitorSession

    return $true
}

Function Publish-Results
{
    param(
            [string]$copyTo
    )

    $temp = "{0}\temp" -f $Env:systemDrive
    if (-not (Test-Path $temp))
    {
        mkdir $temp
    }
    $output = cmd /c "$global:gumshoeExe" publish $temp $global:publishFormat 2>&1
    Write-Host $output

    $resultsFile = "{0}\CoverageResults*.{1}" -f $temp, $global:publishFormat
    Write-Host "Copying the result file from $resultsFile $copyTo"
    xcopy /C /Y "$resultsFile" "$copyTo"

    $subject = "Code Coverage result for $global:tag"
    $body = "Please find the coverage results at $copyTo"
    Send-Mail $global:mailTo $subject $body
}

Function Start-MonitorSession
{
    Enable-PSRemoting -Force | Out-Null
    $s = new-pssession -computername '.' -Name 'Write and Sleep'
    
    Write-Host "Starting the monitoring job in a separate session"
    $Job = Invoke-Command -Session $s -AsJob -ScriptBlock {
        param(
                [string]$copyTo
        )

        $copyTo = $global:copyShare
        if (-not (Test-Path $copyTo))
        {
            mkdir $copyTo
        }

        Write-Host "Results will be copied to $copyTo"
        $copyFrom = "{0}\ProgramData\Gumshoe" -f $Env:systemDrive
        $startTime = $(get-date)

        while ($true)
        {
            xcopy /S /C /Q /Y $copyFrom $copyTo
        
            $duration = $(get-date) - $startTime
            if ($global:runHours -and ($global:runHours -lt $duration.seconds))
            {
                    break
            }
            
            Start-Sleep -s 10
        }

        $output = cmd /c "$global:gumshoeExe" stop 2>&1
        Write-Host $output

        Publish-Results $copyTo
    }
}

Function Print-Help
{
    Write-Host "Usage powershell SetupCodeCoverage.ps1 -tag <run name> -bin <full path to the binary> [-duration <Duration of the run>] [-dpk <dpk to be used as filter>] [-filter <source file filter path>] [-type <OfficialRun|BuddyRun>] -tool <tool name> [-branch <branch>] [-build <build number>] [-arch <architecture>] [-flav <fre|chk>] [-copyto <Share to copy the results to>] [-publish [<html|xml|csv>]] [-mailto <alias>]"
    Write-Host "tag - Used to identify the results folder under $global:copyShareBase\type\tool\branch\build\arch\flav\tag"
    Write-Host "bin - Full path to the binary to be instrumented, inclusing the file name"
    Write-Host "duration - If specified after this duration the session will be stopped and the user will be notified about the result"
    Write-Host "dpk - Full path to a dpk file which will be added as a filter to the session. Useful for buddy test sceanrios."
    Write-Host "filter - Full path to a specific source folder or a file for which you specifically want the coverage numbers"
    Write-Host "type - Either OfficialRun or BuddyRun. Used to create the folder. BuddyRun by default"
    Write-Host "tool - Name of the tool running the coverage"
    Write-Host "branch - Branch name. If not specified then the corresponding value from the machine this script is running will be used."
    Write-Host "build - Build number. If not specified then the corresponding value from the machine this script is running will be used."
    Write-Host "arch - Architecture. If not specified then the corresponding value from the machine this script is running will be used."
    Write-Host "flav - Either fre or chk. fre by default"
    Write-Host "copyto - Location to copy the results to"
    Write-Host "publish - Format you want the results to be published on. html format has only the topmost number. Both xml and csv files have all the data but will take more time to prepare."
    Write-Host "mailto - Alias to send the mails about issues or results"
    exit 1
}

Function Print-Settings
{
    Write-Host "Starting with the below params"

    $global:dpkPath = $args[$i]
    if (-not ($global:tag))
    {
        Write-Host "No tag specified" -ForegroundColor Red
        exit 1
    }
    else
    {
        Write-Host "tag : " $global:tag
    }
    if (-not ($global:binaryPath))
    {
        Write-Host "Binary path not specified" -ForegroundColor Red
        exit 1
    }
    else
    {
        Write-Host "bin : " $global:binaryPath
    }
    Write-Host "dpk : " $global:dpkPath
    Write-Host "duration : " $global:runHours
    Write-Host "filter : " $global:fileFilter
    Write-Host "run type : " $global:type
    if (-not ($global:tool))
    {
        Write-Host "No tool name specified" -ForegroundColor Red
        exit 1
    }
    else
    {
        Write-Host "tool : " $global:tool
    }
    Write-Host "branch : " $global:branch
    Write-Host "build : " $global:build
    Write-Host "architecture : " $global:arch
    Write-Host "flavor : " $global:flav
    Write-Host "copy to : " $global:copyShare
    Write-Host "publish format : " $global:publishFormat
    Write-Host "mail to : " $global:mailto
}

# Parse command line args
if ($args.Length -lt 1)
{
    Print-Help
}
for ($i = 0; $i -lt $args.Length; $i++)
{
    if ($args[$i] -eq "-tag")
    {
        $i++
        $global:tag = $args[$i]
    }
    elseif ($args[$i] -eq "-bin")
    {
        $i++
            $global:binaryPath = $args[$i]
    }
    elseif ($args[$i] -eq "-duration")
    {
            $i++
            $global:runHours = $args[$i]
    }
    elseif ($args[$i] -eq "-monitor")
    {
            $global:monitorRun = $true
    }
    elseif ($args[$i] -eq "-dpk")
    {
            $i++
            $global:dpkPath = $args[$i]
    }
    elseif ($args[$i] -eq "-filter")
    {
            $i++
            $global:fileFilter = $args[$i]
    }
    elseif ($args[$i] -eq "-type")
    {
            $i++
            $global:type = $args[$i]
    }
    elseif ($args[$i] -eq "-tool")
    {
            $i++
            $global:tool = $args[$i]
    }
    elseif ($args[$i] -eq "-branch")
    {
            $i++
            $global:branch = $args[$i]
    }
    elseif ($args[$i] -eq "-build")
    {
            $i++
            $global:build = $args[$i]
    }
    elseif ($args[$i] -eq "-arch")
    {
            $i++
            $global:arch = $args[$i]
    }
    elseif ($args[$i] -eq "-flav")
    {
            $i++
            $global:flav = $args[$i]
    }
    elseif ($args[$i] -eq "-copyto")
    {
            $i++
            $global:copyShare = $args[$i]
    }
    elseif ($args[$i] -eq "-publish")
    {
        if (-not ($args[$i+1].StartsWith("-")))
        {
            $i++
            $global:publishFormat = $args[$i]
        }
    }
    elseif ($args[$i] -eq "-mailto")
    {
        $i++
            $global:mailTo = "{0}@microsoft.com" -f $args[$i]
    }
    else
    {
        $error = "ERROR: wrong param {0}" -f $args[$i]
        Write-Host $error -ForegroundColor Red
        Print-Help
    }
}

Write-Host "Make sure you have the pdb file in the same folder as " $global:binaryPath -ForegroundColor Yellow
if (-not ($global:monitorRun))
{
    if (-not $global:copyShare)
    {
        $global:copyShare = "{0}\{1}\{2}\{3}\{4}\{5}\{6}\{7}" -f $global:copyShareBase, $global:type, $global:tool, $global:branch, $global:build, $global:arch, $global:flav, $global:tag
    }
    Print-Settings
    if (-not (Setup-CodeCoverage -binary $global:binaryPath))
    {
        Write-Host "Code coverage set up failed" -ForegroundColor Red
        exit 1
    }
}
else
{
    Monitor-CodeCoverage
}
