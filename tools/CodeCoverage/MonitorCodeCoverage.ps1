Import-Module "\\bptstorage3\StressSuite\Setup\IEandWWA\ChakraStress.psm1"
Add-Type -Path ".\MongoDB.Bson.dll"
Add-Type -Path ".\MongoDB.Driver.dll"

$global:tag             = "CCRun"
$global:runHours        = ""
$global:type            = "BuddyRun"
$global:tool            = "CC"
$global:branch          = Get-Branch
$global:build           = Get-BuildNumber
$global:arch            = $Env:PROCESSOR_ARCHITECTURE
$global:flav            = "fre"
$global:copyShare       = $null
$global:publishFormat   = "html"
$global:mailTo          = "aneeshd@microsoft.com"

$global:gumshoeDir      = "{0}\Program Files\Gumshoe" -f $global:sysDrive
$global:gumshoeExe      = "$global:gumshoeDir\gumshoe.exe"
$global:copyShareBase   = "\\chakrafs01\CodeCoverageLogs"

$global:db              = [MongoDB.Driver.MongoDatabase]::Create('mongodb://MongoDB/ReportingDB');
$global:collection      = $global:db['CodeCoverage']

# Parse command line args
for ($i = 0; $i -lt $args.Length; $i++)
{
    if ($args[$i] -eq "-tag")
    {
        $i++
        $global:tag = $args[$i]
    }
    elseif ($args[$i] -eq "-duration")
    {
            $i++
            $global:runHours = $args[$i]
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
        Write-Host "Usage powershell MonitorCodeCoverage.ps1 -tag <run name> [-duration <Duration of the run>] -type <OfficialRun|BuddyRun> -tool <tool name> [-branch <branch>] [-build <build number>] [-arch <architecture>] -flav <fre|chk> [-copyto <Share to copy the results to>] [-publish [<html|xml|csv>]] [-mailto <alias>]"
        Write-Host "tag - Used to identify the results folder under $global:copyShareBase\type\tool\branch\build\arch\flav\tag"
        Write-Host "duration - If specified after this duration the session will be stopped and the user will be notified about the result"
        Write-Host "type - Either OfficialRun or BuddyRun. Used to create the folder"
        Write-Host "tool - Name of the tool running the coverage"
        Write-Host "branch - Branch name. If not specified then the corresponding value from the machine this script is running will be used."
        Write-Host "build - Build number. If not specified then the corresponding value from the machine this script is running will be used."
        Write-Host "arch - Architecture. If not specified then the corresponding value from the machine this script is running will be used."
        Write-Host "flav - Either fre or chk"
        Write-Host "copyto - Location to copy the results to"
        Write-Host "publish - Format you want the results to be published on. html format has only the topmost number. Both xml and csv files have all the data but will take more time to prepare."
        Write-Host "mailto - Alias to send the mails about issues or results"

        exit 1
    }
}

Function Publish-Results
{
    param(
            [string]$copyTo,
            [string]$format,
            [string]$location,
            [switch]$sendMail
    )

    $output = cmd /c "$global:gumshoeExe" publish $location $format 2>&1
    Write-Host $output

    if ($copyTo)
    {
        $resultsFile = "{0}\CoverageResults*.{1}" -f $location, $format
        Write-Host "Copying the result file from $resultsFile $copyTo"
        xcopy /C /Y "$resultsFile" "$copyTo"
    }

    if ($sendMail)
    {
        $subject = "Code Coverage result for $global:tag"
        $body = "Please find the coverage results at $copyTo"
        Send-Mail $global:mailTo $subject $body
    }
}

Function InsertTo-MongoDB ($resultFile)
{
    $contents       = Get-Content $resultFile
    $names          = @()
    $totalBlocks    = @()
    $blocksCovered  = @()
    $percCov        = @()

    # Expected format of each line
    # Binary,Filtered,Static Library,Function Name,Total Blocks,Blocks Covered,Percentage Covered,Total Arcs,Arcs Covered,Percenage Covered2
    for ($i = 0; $i -lt $contents.Length; $i++)
    {
        $parts = $contents[$i].Split(",")
        if ($parts[2].Contains("chakra."))
        {
            $name = $null
            $nameParts = $parts[2].Split(".")
            if ($nameParts.Length -gt 3)
            {
                $name = $nameParts[2].Trim()
            }
            else
            {
                $name = $nameParts[1].Trim()
            }
            if ($names.Contains($name))
            {
                $index = $names.IndexOf($name)
                $totalBlocks[$index] = ($totalBlocks[$index] + [int]($parts[4].Trim()))
                $blocksCovered[$index] = ($blocksCovered[$index] + [int]($parts[5].Trim()))
            }
            else
            {
                $names += $name
                $totalBlocks += [int]($parts[4].Trim())
                $blocksCovered += [int]($parts[5].Trim())
            }
        }
    }

    $totalBlocksCount = 0
    $totalBlocksCovered = 0
    $totalPercCovered = 0.0
    for ($i = 0; $i -lt $names.Length; $i++)
    {
        $totalBlocksCount += $totalBlocks[$i]
        $totalBlocksCovered += $blocksCovered[$i]
        $percCov += ((($blocksCovered[$i]) / ($totalBlocks[$i])) * 100)
    }
    $totalPercCovered = (($totalBlocksCovered / $totalBlocksCount) * 100)

    Write-Host "Uploading to MongoDB"
        Write-Host "Name`t`tTotal Blocks`tBlocked Covered`tPerc Covered"
    [MongoDB.Bson.BsonDocument] $resultDoc = @{}
    for ($i = 0; $i -lt $names.Length; $i++)
    {
        [MongoDB.Bson.BsonDocument] $resultObj = @{
            'TotalBlocks' = $totalBlocks[$i];
            'CoveredBlocks' = $blocksCovered[$i];
            'PercCovered' = $percCov[$i];
        }
        $resultDoc[$names[$i]] = $resultObj

        Write-Host ("{0}`t{1}`t{2}`t{3}" -f $names[$i], $totalBlocks[$i], $blocksCovered[$i], $percCov[$i])
    }
    Write-Host ("Total`t{0}`t{1}`t{2}" -f $totalBlocksCount, $totalBlocksCovered, $totalPercCovered)
    [MongoDB.Bson.BsonDocument] $resultObj = @{
        'TotalBlocks' = $totalBlocksCount;
        'CoveredBlocks' = $totalBlocksCovered;
        'PercCovered' = $totalPercCovered;
    }
    $resultDoc['Total'] = $resultObj

    # Build format 10151.0.150704-1800
    $shortBuildNumber = $global:build.Split(".")[2]
    [MongoDB.Bson.BsonDocument] $doc = @{
        "Tag"           = $global:tag;
        "RunHours"      = $global:runHours;
        "Type"          = $global:type;
        "RunID"         = "";
        "ToolName"      = $global:tool;
        "Branch"        = $global:branch;
        "Build"         = $shortBuildNumber;
        "FullBuild"     = $global:build;
        "Architecture"  = $global:arch;
        "Flavor"        = $global:flav;
        "CopyShare"     = $global:copyShare;
        "Result"        = $resultDoc;
    };

    # TODO: Query whether it exists before inserting
    [MongoDB.Driver.QueryDocument] $query = @{
        "Tag"           = $global:tag;
        "RunHours"      = $global:runHours;
        "Type"          = $global:type;
        "RunID"         = "";
        "ToolName"      = $global:tool;
        "Branch"        = $global:branch;
        "Build"         = $shortBuildNumber;
        "FullBuild"     = $global:build;
        "Architecture"  = $global:arch;
        "Flavor"        = $global:flav;
        "CopyShare"     = $global:copyShare;
    };
    $exists = ($global:collection).FindOne($query)
    if ($exists)
    {
        ($global:collection).Update($query, [MongoDb.Driver.UpdateDocument]$doc)
    }
    else
    {
        ($global:collection).Insert($doc)
    }

}

$copyTo = ""
if ($global:copyShare)
{
    $copyTo = $global:copyShare
}
else
{
    $copyTo = "{0}\{1}\{2}\{3}\{4}\{5}\{6}\{7}" -f $global:copyShareBase, $global:type, $global:tool, $global:branch, $global:build, $global:arch, $global:flav, $global:tag
}

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

    Publish-Results $null "." "csv"
    InsertTo-MongoDB ".\CoverageResults_Libraries.csv"

    $duration = $(get-date) - $startTime
    if ($global:runHours -and ($global:runHours -lt $duration.hours))
    {
            break
    }
            
    Start-Sleep -s 300
}

$output = cmd /c "$global:gumshoeExe" stop 2>&1
Write-Host $output

$temp = "{0}\temp" -f $Env:systemDrive
if (-not (Test-Path $temp))
{
    mkdir $temp
}
Publish-Results $copyTo $temp $global:publishFormat
