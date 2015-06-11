#
# OpenDump.ps1
#
# Powershell script which takes a bug number and opens a dump in that bugs
# repro steps
#
param (
      [Parameter(Mandatory=$True)]
      [int]$id,
      [Switch]$noDbg,
      [Switch]$returnAllDumps,
      [int]$dumpId = -1,
      [Switch]$debugMode
)

. "$PSScriptRoot\util.ps1"

Write-Host Looking up bug $id
function getSymbolPathFromLogFile($logFile)
{
    Write-Host Getting symbol path from $logFile
    $symbolPath = $nil
    $contents = Get-Content -LiteralPath $logFile
    $contents | ForEach-Object {
        $line = $_
            
        if ($symbolPath -eq $nil) {
            if ($line -match "^Symbol search path is[:]?[\s]*(.*)")
            {
                $symbolPath = $matches[1]
                Write-Host Search path is $symbolPath
                return
            }
            if ($line -match "^Symbol Path[:]?[\s]*(.*)")
            {
                $symbolPath = $matches[1]
                Write-Host Symbol path is $symbolPath
                return
            }
        }
    }
    
    return $symbolPath
}

$workItem = getBug($id)
$reproSteps = $workItem.Fields["Repro Steps"].Value
$reproStepsText = htmlToText($reproSteps, $debugMode)
$lines = $reproStepsText.split("`n")

$dumps = @()

$invalidFsChars = "[{0}]" -f ([Regex]::Escape([String] [System.IO.Path]::GetInvalidPathChars()))

foreach ($line in $lines) {
    # Write-Host Checking $line

    if ($line -match ".*Dump([\s]*Location|[\s]*):[\s]*(.*)") {
       $current = @{}
       $current.dump = [Regex]::Replace($matches[2].trim(), $invalidFsChars, '')

       if ($returnAllDumps -eq $false) {
           Write-Host Dump is $current.dump
       }
    }
    if ($line -match ".*Symbol([\s]*|s[\s]*)[:;][\s]*(.*)") {
       if ($current -ne $nil) {
          if ($current.symbol -eq $nil) {
              $current.symbol = $matches[2]      
              $dumps += $current
              $current = $nil
          }
       }
    }

    if ($line -match ".*Log([\s]*|s[\s]*)[:;][\s]*(.*)") {
       if ($current -ne $nil) {
          $logPath = [Regex]::Replace($matches[2].trim(), $invalidFsChars, '')
          $symPath = getSymbolPathFromLogFile $logPath
          if ($symPath -ne $nil) {
              $current.symbol = $symPath
          }

          $dumps += $current
          $current = $nil
       }
    }
}

if (($dumps.length -gt 1) -or $noDbg)
{

   if ($returnAllDumps -eq $false) {
     for ($i = 0; $i -lt $dumps.length; $i++) 
     {
         Write-Host $i`. $dumps[$i].dump
     }
   }
   if ($dumpId -lt 0)
   {
        $dumpId = $dumps.length + $dumpId;
   }

   if (($dumpId -eq 0) -and ($noDbg -eq $false) -and ($returnAllDumps -eq $false))
   {
      Write-Host Multiple dumps found, so opening the last one. Use -dumpId to open a different dump
   } 
   elseif ($dumpId -gt $dumps.length) 
   {
      Write-Host Invalid dump number
      Break
   } 
}
elseif ($dumps.length -eq 0) 
{
   Write-Host Error: No dumps found in repro steps
   Write-Host Lines: $lines
   Break
}

if ($returnAllDumps -eq $true) {
   return $dumps
}

if ($noDbg -eq $false)
{
    $contents = ".load jd; .srcfix+; .ecxr"
    $dumpFile = $dumps[$dumpId].dump
    $symbolPath = $dumps[$dumpId].symbol
    Write-Host windbg -z `"$dumpFile`" -c `"$contents`" -y `"$symbolPath`"
    Start-Process -FilePath windbg -ArgumentList ("-z", "`"$dumpFile`"", "-c", "`"$contents`"", "-y", "`"$symbolPath`"")
}
