#
# Run a command, output result to an xml file. (Used by submit.js)
#
Param(
    [string]$cmdline,
    [string]$outputFile,
    [int]$timeout = -1 # seconds. Default -1 for infinity
)

# Save a hashtable content into a xml file.
function simpleXml($h) {
    $doc = New-Object -ComObject "Microsoft.XMLDOM"

    $root = $doc.CreateElement("root")
    $doc.AppendChild($root) | Out-Null

    foreach ($p in $h.Keys) {
        $e = $doc.CreateElement($p)
        $e.Text = $h[$p]
        $root.appendChild($e) | Out-Null
    }
    
    $doc.Save($outputFile) | Out-Null
}

$job = Start-Job -ScriptBlock { Invoke-Expression $using:cmdline }
Wait-Job $job -Timeout $timeout
Receive-Job $job -OutVariable outVar -ErrorVariable errVar | Out-Null

simpleXml @{
    state = $job.State;
    stdout = $outVar -join "`n";
    stderr = $errVar -join "`n";
}
