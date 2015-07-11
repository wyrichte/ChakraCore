#
# Utility helpers for the TFS based powershell scripts
#
# Exposes following APIs:
#  getBugs:    Get's a list of all the bugs associated with the current user
#  getBug:     Returns a particular bug 
#  htmlToText: Cleans up a particular snippet of HTML and returns the plaintext
#

function TryLoadTeamFoundationAssembly($assemblyName)
{
   $assembly = [System.Reflection.Assembly]::LoadWithPartialName($assemblyName)

   if ($assembly -eq $nil) {
      try {      
         $assembly = [System.Reflection.Assembly]::LoadFrom("$env:CommonProgramW6432\Microsoft Shared\Team Foundation Server\14.0\$assemblyName.dll")      
      } catch {      
      }
   }

   if ($assembly -eq $nil) {
      try   {      
         $assembly = [System.Reflection.Assembly]::LoadFrom("$env:CommonProgramFiles\Microsoft Shared\Team Foundation Server\14.0\$assemblyName.dll")
      } catch {      
      }
   }

   if ($assembly -eq $nil)   {
      Write-Host Unable to load $assemblyName
      return $false
   }
}

if ((TryLoadTeamFoundationAssembly("Microsoft.TeamFoundation.Client") -eq $false) -or 
   (TryLoadTeamFoundationAssembly("Microsoft.TeamFoundation.WorkItemTracking.Client") -eq $false))
{ 
   exit; 
}

$tfsServer = "https://microsoft.visualstudio.com/DefaultCollection"
[psobject] $tfs = [Microsoft.TeamFoundation.Client.TeamFoundationServerFactory]::GetServer($tfsServer)
$workItemStore = $tfs.GetService("Microsoft.TeamFoundation.WorkItemTracking.Client.WorkItemStore")
$idService = $tfs.GetService("Microsoft.TeamFoundation.Framework.Client.IIdentityManagementService2")

function getBugs() {
   $query = "Select [System.Id], [System.State], [System.AssignedTo], [System.Title], [System.AreaPath] From WorkItems where [System.AssignedTo] = @Me"

   $items = $workItemStore.Query($query)

   return $items;
}

function getUserNameFromDisplayName($displayName)
{
    $id = $idService.ReadIdentity([Microsoft.TeamFoundation.Framework.Common.IdentitySearchFactor]::DisplayName,
             $displayName,
             [Microsoft.TeamFoundation.Framework.Common.MembershipQuery]::Direct,
             [Microsoft.TeamFoundation.Framework.Common.ReadIdentityOptions]::ExtendedProperties)

    $username = $id.UniqueName -replace "`@.*",""
    return $username;
}

function getBugsForGroup($groupName) {
   $query = "Select [System.Id], [System.State], [System.AssignedTo], [System.Title] From WorkItems where [System.AreaPath] under 'OS\CORE-OS Core\DEP-Developer Ecosystem Platform\IEP-Internet Explorer Platform\Chakra Javascript Engine' AND [System.State] = 'Active' AND [System.AssignedTo] in group '[DefaultCollection]\" + $groupName + "' order by [System.AssignedTo]"

   $items = $workItemStore.Query($query)

   return $items;
}

function getBug($id) {
   return $workItemStore.GetWorkItem($id);
}

function htmlToText($html, $debugMode) {
   $ie = New-Object -COM InternetExplorer.Application

   $capture = $ie.navigate2("about:blank")
   $ie.width = 600
   $ie.height = 400
   $ie.AddressBar = $False
   $ie.Toolbar = $False
   $ie.MenuBar = $False
   $ie.StatusBar = $False
   $ie.Resizable = $True

   $ie.document.body.innerHTML = $html

   if ($debugMode -eq $True)
   {
     $ie.Visible = $True
   }

   $text = $ie.document.body.innerText
   
   if ($debugMode -eq $True)
   {
     Write-Host Raw HTML is $text
   }      
   
   return $text;
}