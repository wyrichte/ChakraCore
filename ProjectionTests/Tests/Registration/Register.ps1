$requiredArgs = 1

# Changes directory to $args[0]
write-host 'Changing to ' $args[0]

cd $args[0]

if ($args[1])
{
	$filename = $args[1]
}
else 
{
	write-host "Missing Argument: Manifest file required."
	write-host "usage: .\register.bat [manifest.man] [source-dir] [dest-dir]"
	exit
}

$src = "."
$dest = "."

if ($args[2])
{
	$src = $args[2]
}
if ($args[3])
{
	$dest = $args[3]
}

write-host '### configuring basic registry keys ###'

$admins = [System.Security.Principal.NTAccount]"BUILTIN\Administrators"
$wrtkeynames = "HKLM:\SOFTWARE\Microsoft", "HKLM:\SOFTWARE\Microsoft\WindowsRuntime", "HKLM:\SOFTWARE\Microsoft\WindowsRuntime\CoClassID", "HKLM:\SOFTWARE\Microsoft\WindowsRuntime\ActivatableClassID", "HKLM:\SOFTWARE\Microsoft\WindowsRuntime\CLSID"

#subinacl.exe /noverbose /nostatistic /keyreg HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\WindowsRuntime /owner=BUILTIN\Administrators

foreach ($wrtkeyname in $wrtkeynames)
{
  $wrtkey = Get-Item $wrtkeyname -ErrorAction:SilentlyContinue

  if (!$wrtkey)
  {
    write-host '... creating ' $wrtkeyname
    $wrtkey = New-Item -type Directory $wrtkeyname
  }

  # TODO: Ensure full control as well. CoClassID has that limitation.

  if ($wrtkey)
  {
    write-host '...taking ownership of ' $wrtkey
	.\subinacl.exe /noverbose /nostatistic /keyreg $wrtkey /owner=BUILTIN\Administrators
	write-host '...Setting permissions of ' $wrtkey
	.\subinacl.exe /noverbose /nostatistic /keyreg $wrtkey /grant=BUILTIN\Administrators=F
  }
  else
  {
  write-host "Key does not exist, and needs to be created."
  }
}

write-host '### Loading Manifest file' $filename ' ###'
$manifest = new-object System.Xml.XmlDocument
$ns = New-Object Xml.XmlNamespaceManager $manifest.NameTable
$ns.AddNamespace( "m", "urn:schemas-microsoft-com:asm.v3" )

$file = resolve-path($filename)
$manifest.load($file)
$keynodes = $manifest.selectnodes("/m:assembly/m:registryKeys/m:registryKey", $ns)
$filenodes = $manifest.selectnodes("/m:assembly/m:file", $ns)

write-host '... found ' $filenodes.count ' file(s)'

ForEach($filenode in $filenodes) {
	$filesrc = $src + "\" + $filenode.sourcename
	$filedest = $dest + "\" + $filenode.name
	write-host 'copying from: ' $filesrc ' to: ' $filedest
	#cp $filesrc $filedest
	write-host
}

write-host '... found ' $keynodes.count ' key(s)'
$filter = "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\WindowsRuntime\"

ForEach($keynode in $keynodes) {

  $keyname = $keynode.keyName

  if ($keyname.Contains($filter))
  {
    $keyname = $keyname.Replace("HKEY_LOCAL_MACHINE", "HKLM:")
    $key = Get-Item $keyname -ErrorAction:SilentlyContinue

    if (!$key) 
    {
      write-host '... creating key:' $keyname
      $key = New-Item -type Directory $keyname
    }
    else
    {
      write-host '... key exists:' $keyname
      write-host '    if you have removed values from manifest, use regedit to remove from registry'
   }
    
    $valuenodes = $keynode.selectnodes("m:registryValue", $ns)
    write-host '..... found ' $valuenodes.count ' values'

    foreach($value in $valuenodes)
    {
	  if ($value.valueType)
	  {
		$value.valueType = $value.valueType.replace("REG_DWORD", "Dword");
		$value.valueTYpe = $value.valueType.replace("REG_SZ", "string");
	  }

	  if ($value.value)
	  {
		$value.value = $value.value.Replace('$(runtime.system32)', $dest)
      }

      $newvalue = Get-ItemProperty $keyname $value.name -ErrorAction:SilentlyContinue

      if ($newvalue -AND $value.name)
      {
         Remove-ItemProperty $keyname $value.name
      }

	  $newvalue = new-ItemProperty -path $keyname -name $value.name -value $value.value -propertyType $value.valueType
    }
  }
  else
  {
    write-host '... ignored non-WinRT key ' $keyname
  }
}
