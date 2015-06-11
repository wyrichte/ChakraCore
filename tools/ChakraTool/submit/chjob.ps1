#
# This script updates snap job xml with some additional info. Use powershell because it handles unicode well.
#

Param(
    [string]$JobXmlPath,
    [string]$Reviewer
)

function updateJob() {
    $ENDMAIL = 0
    $ENDROOT = 1
    $state = -1

    foreach($line in (gc $JobXmlPath)) {
        if ($state -lt $ENDMAIL) {
            if ($line -match "^\s*<CodeReviewer>") {
                throw "Snap job xml already has CodeReviewer"
            }
            if ($line -match "^\s*<CC>") {
                Write-Output ("        <CodeReviewer>{0}</CodeReviewer>" -f $Reviewer)
                $line = "        <CC>chakrachkin</CC>"
            } elseif ($line -match "</CC>") {
                throw "Snap job xml format unexpected. <CC> and </CC> are not at the same line."
            } elseif ($line -match "^\s*</Mail>") {
                $state = $ENDMAIL
            }
        } elseif ($state -lt $ENDROOT) {
            if ($line -match "^\s*</root>") {
                $state = $ENDROOT
            }
        }

        Write-Output $line
    }

    if ($state -ne $ENDROOT) {
        throw "Snap job xml not ready? </root> not seen."
    }
}

$c = updateJob
Out-File $JobXmlPath -InputObject $c
