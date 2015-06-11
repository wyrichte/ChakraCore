#
# Powershell script which lists all bugs associated with the current user
# or dumps the details for a given bug in the console
#

param (
      [string]$titleFilter = $nil,
      [int]$id = -1,
      [Switch]$open,
      [Switch]$team
)

. "$PSScriptRoot\util.ps1"

function displayBugs($items, $printAssignTo) {
   if ($titleFilter -ne $nil) {
     $items = $items | where { $_.Title -match $titleFilter }
   }

   foreach ($item in $items) {           
       $title = $item.Title
       if ($title.length -gt 80) {
           $title = $title.substring(0,77) + "..."
       }

       if ($printAssignTo) {
           $username = getUserNameFromDisplayName($item.Fields['Assigned To'].Value)
           $str = "{0}`t{1,8}  {2}" -f $item.Id, $username, $title
       } else {
           $str = "{0}`t{1}`t{2}" -f $item.Id, $item.State, $title
       }

       Write-Host $str
   }
}

if ($team) {
    $teamBugs = getBugsForGroup "o_edmaurer"

    displayBugs $teamBugs $true
} elseif ($id -eq -1) {
   # Get all bugs
   $yourBugs = getBugs;

   displayBugs $yourBugs $false
} else {
   $workItem = getBug($id)
   if ($workItem -eq $nil) {
      Write-Host Bug $id not found
      Break
   }
   Write-Host Id: $workItem.Id
   Write-Host State: $workItem.State
   Write-Host Title: $workItem.Title
   Write-Host Priority: $workItem.Fields["Priority"].Value
   Write-Host Severity: $workItem.Fields["Severity"].Value
   Write-Host Branch: $workItem.Fields["Found In Branch"].Value
   Write-Host Build: $workItem.Fields["Found In Build"].Value

   $repro = $workItem.Fields["Repro Steps"].Value

   # Do some minor HTML clean up
   $repro = htmlToText $repro $false

   Write-Host Repro steps: $repro

   $url = $tfsServer + "/OS/_workItems/edit/" + $workItem.Id

   if ($open) {
      Write-Host Url is $url
      start $url
   }
}
