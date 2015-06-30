start-transcript FileContent.log
$results = Get-Content $args[0]
$failuresCount = 0
for ($i = 0; $i -lt $results.Length; $i++)
{
    if ($results[$i].StartsWith("not ok "))
    {
        $line = ""
        $failuresCount++
        $i += 2
        $parts = $results[$i].Split(":")
        $line += $parts[1].Trim()

        if (($results[$i + 1]).Trim() -eq "...")
        {
            $line += "::::::::"
        }
        else
        {
            $i++
            $parts = $results[$i].Split(":")
            $line += "::::" + $parts[1].Trim() # errorName
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
            $line += "::::" + $errorMessage
        }
        Write-Host $line
    }
}
Write-Host "Total failures : " $failuresCount
stop-transcript
