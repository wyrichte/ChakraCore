# based on Register.psm1
#  from
#  \\winbuilds\release\fbl_ie_script_dev\9278.0.121206-1815\x86fre\bin\idw\WinRTFirstPartyABIReg\Register.psm1

$ChakraABITestRegistrationIdentifier = "ChakraABIUT"
$ChakraABITestRegistrationKeyCreatedIdentifier = "NewKey"
$ChakraABITestRegistrationEnableVerboseOutput = $false

function set-registryAcl {
param (
	[String]
	$keyname,

    [String]
    $ownershiptool = ".\TakeRegistryAdminOwnership.exe"
	)

    if(!(test-path $ownershiptool)) {
		throw "cannot find $($ownershiptool)"
    }

	# make the keyName properly formatted, just in case
	$keyname = $keyname.Replace("HKEY_LOCAL_MACHINE\","MACHINE\")
    $keyname = $keyname.Replace("HKLM:\", "MACHINE\")
    invoke-expression "$ownershiptool '$keyname'"

    $keyname = $keyname.Replace("MACHINE\","HKLM:\")
	$acl = get-acl $keyname
	$username = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name
	$rule = new-object System.Security.AccessControl.RegistryAccessRule ($username, "FullControl","ContainerInherit,ObjectInherit", "InheritOnly", "Allow")
	$acl.SetAccessRule($rule)
	$acl | set-acl -path $keyname
}

function setup-registry {
<#
.Synopsis
    Ensure the required keys exist and are properly permissioned
.Description
    Requires TakeRegistryAdminOwnership in the target directory (defaults to current directory), or it uses the tool passed in.
.Example
    setup-registry
    
    creates and permissions the keys.
.Example
    setup-registry \path\to\TakeRegistryAdminOwnership.exe
    
    Creates and permissions the kesy with the specified tool
.Notes
 NAME:      setup-registry
 AUTHOR:    REDMOND\jdeville;redmond\fcantonn
 LASTEDIT:  10/21/2010;12/07/2012
#Requires -Version 2.0
#>
param(
    #tool to use
    [String]
    $ownershiptool = ".\TakeRegistryAdminOwnership.exe"
    )
    
    if(!(test-path $ownershiptool)) {
		throw "cannot find $($ownershiptool)"
    }

	# expand all levels
	$keys = @("MACHINE\Software\Microsoft\WindowsRuntime",
    "MACHINE\Software\Microsoft\WindowsRuntime\ActivatableClassId",
    "MACHINE\Software\Microsoft\WindowsRuntime\CLSID",
    "MACHINE\Software\Microsoft\WindowsRuntime\Server")

    $keys | foreach-object -begin { write-debug "Creating REG folders" } -process {
        $keyname = $_.Replace("MACHINE\","HKLM:\")
        if(!(test-path $keyname)) {
            New-Item -type Directory $keyname
        }
		
		# set acl
		set-registryAcl -keyname $keyname -ownershiptool $ownershiptool
	}
}

function register-man {
<#
.Synopsis
    Registers given man files
.Description
    register-man can be used to register ABIs with the system. Requires 
    TakeRegistryAdminOwnership in the target directory (defaults to current directory)
.Example
    register-man foo.man
    
    Registers the foo.man manifest from the current directory
.Example
    register-man foo.man,bar.man
    
    Registers foo and bar in the current directory
.Example
    ls *.man | register-man
    
    Registers all man files in the current directory
.Example
    register-man -targetdir \some\path -src \some\other\path -dest \some\destination -filenames list,of,files
    
    Registers the manifest files in \some\path matching list,of,files. Assumes TakeRegistryAdminOwnership is 
    in \some\path and the binaries to be registered are in \some\other\path. Places the files at \some\destination
.Notes
 NAME:      register-man
 AUTHOR:    REDMOND\jdeville;redmond\fcantonn
 LASTEDIT:  10/21/2010;12/07/2012
#Requires -Version 2.0
#>
param(

    
    #man files to register
    [Parameter(Mandatory=$true, 
        Position=0,
        ValueFromPipeline=$true,
        ValueFromPipelineByPropertyName=$true)]
    [String[]]
    $filenames,
    
    #target location for man files
    [String]
    $targetdir = (get-location),
    
    #location of dll's and winmd's
    [String]
    $src  = $targetdir,

    #where to place the files
    [String]
#    $dest = "$env:windir\system32",
    $dest = $targetdir, #"$env:windir\system32",
    
    [Switch]
    $skipSetup,
    
    [Switch]
    $testOnly
)
    begin{
        # write-host 'Changing to ' $targetdir

        pushd $targetdir
        if(!$skipSetup -and !$testOnly) {
            setup-registry
        }
    }

    process{
        foreach($file in $filenames) {
            $manifest = [xml](gc $file)

            if ($src -ne $dest)
            {
                write-host -foregroundcolor yellow "Copying manifests to $($dest)"
                if( $testOnly ) {
                    # require admin access
                    return $true
                }

                foreach($node in $manifest.assembly.file) {
                    $src = (join-path $src $node.sourceName)
                    $dst = (join-path $dest $node.name)
                    write-host "copy-item from $($src) to $($dst)"
                    copy-item -force $src $dst
               }
           }

            foreach ($keynode in $manifest.assembly.registryKeys.registryKey) {
            
                $filter = "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\WindowsRuntime\"
                $keyname = $keynode.keyName
                
                if ($keyname.Contains($filter)) {
                    $keyname = $keyname.Replace("HKEY_LOCAL_MACHINE", "HKLM:")
                    $key = Get-Item $keyname -ErrorAction:SilentlyContinue

                    if (!$key) 
                    {
						write-host -foregroundColor yellow 'NEW KEY: ' $keyname
                        
                        if( $testOnly ) {
                            # require admin access
                            return $true
                        }
                        
                        New-Item -type Directory $keyname | out-null
                        if (!$?)
                        {
                            write-host -foregroundColor red "Failed to perform New-Item -type Directory $($keyname)"
                            exit 10
                        }
                        
						set-registryAcl -keyname $keyname
						
                        # tag it as 'Charkra projection ABI unit test'
                        Set-ItemProperty -path $keyname -name $ChakraABITestRegistrationIdentifier -value $ChakraABITestRegistrationKeyCreatedIdentifier
                        if (!$?)
                        {
                            write-host -foregroundColor red "Failed to perform Set-ItemProperty $($keyname) $ChakraABITestRegistrationIdentifier=$ChakraABITestRegistrationKeyCreatedIdentifier"
                            exit 11
                        }
                    } else {
                        $itemProperty = Get-ItemProperty -path $keyname -name $ChakraABITestRegistrationIdentifier -errorAction:silentlyContinue
                        if ($itemProperty -ne $null -and
                            $itemProperty.$ChakraABITestRegistrationIdentifier -ne $ChakraABITestRegistrationKeyCreatedIdentifier)
                        {
                            write-host -foregroundColor white 'KEY ' $keyname ' already exists'
                        
                            write-host '    if you have removed values from manifest, use regedit to remove from registry'
                            write-host -foregroundColor red "This is not supported - please clean registry manually"
                            exit 20
                        }
                    }
                
                    foreach($valuenodes in $keynode.registryValue) {

                        foreach($value in $valuenodes)
                        {
                            if ($value.valueType)
                            {
                                $value.valueType = $value.valueType.replace("REG_DWORD", "Dword");
                                $value.valueType = $value.valueType.replace("REG_SZ", "string");
                            }

                            if ($value.value)
                            {
                                $value.value = $value.value.Replace('$(runtime.system32)', $dest)
                            }

                            $currentRegistryKey = Get-Item $keyname -ErrorAction:SilentlyContinue
                            $isCurrentRegistryValueExists = Get-ItemProperty $keyname $value.name -ErrorAction:SilentlyContinue
                            
                            $noop = 0
                            if ($isCurrentRegistryValueExists)
                            {
                                # exists -- either NO-OP or MOVE
                                $currentRegistryValue = $currentRegistryKey.GetValue($value.name)
                                $currentRegistryValueType = $currentRegistryKey.GetValueKind($value.name)

                                if (($currentRegistryValueType -ieq $value.valueType) -AND
                                    ($currentRegistryValue -ieq $value.value))
                                {
                                    $noop = 1
                                }
                                else
                                {
									if ($ChakraABITestRegistrationEnableVerboseOutput)
									{
										write-host -foregroundColor white 'KEY ' $keyname ' already exists'
										write-host 'current:  ' $currentRegistryValueType ' ' $currentRegistryValue
										write-host -NoNewline '    new:  ' $value.valueType ' ' $value.value 

										write-host -foregroundColor yellow  " Rename: into $($value.name).renamed"
									}
									
                                    if( $testOnly ) {
                                        # require admin access
                                        return $true
                                    }
                                    
                                    Rename-ItemProperty $keyname -Name $value.name -NewName "$($value.name).renamed"
                                }
                            }
                            
                            if ($noop -eq 0)
                            {
								if ($ChakraABITestRegistrationEnableVerboseOutput)
								{
									write-host -foregroundColor gray "KEY: $($keyname)"
									write-host -NoNewline '    new:  ' $value.valueType ' ' $value.value 
									write-host -foregroundColor yellow  " NEW"
								}

                                if( $testOnly ) {
                                    # require admin access
                                    return $true
                                }
                                
                                $newvalue = new-ItemProperty -path $keyname -name $value.name -value $value.value -propertyType $value.valueType
                                if (!$newvalue) 
                                {
                                    exit 30
                                }
                            }
                        }
                    }
                } else  {
                    write-host -foregroundColor white '... ignored non-WinRT key ' $keyname
                }
            }
        }
    }
    end {
        if(!$skipSetup -and !$testOnly) {
            # do setup-registry twice, to set ownership/permissions before (for vanilla) and after (for new test) registry keys
            setup-registry
        }

        popd
    }
}
function unregister-man {
<#
.Synopsis
    UnRegisters given man files
.Description
    unregister-man can be used to unregister ABIs with the system.
.Example
    unregister-man foo.man
    
    unegisters the foo.man manifest from the current directory
.Example
    unregister-man foo.man,bar.man
    
    unregisters foo and bar in the current directory
.Example 
    ls *.man | unregister-man
    
    unregisters all man files in the current directory.
.Example
    unregister-man -targetdir \some\path -dest \some\destination -filenames list,of,files
    
    unegisters the manifest files in \some\path matching list,of,files. Assumes manifests are in 
    in \some\path. Removes binaries from \some\destination
.Notes
 NAME:      unregister-man
 AUTHOR:    REDMOND\jdeville
 LASTEDIT:  10/21/2010
#Requires -Version 2.0
#>
param(
    #man files to unregister
    [Parameter(Mandatory=$true, 
        Position=0,
        ValueFromPipeline=$true,
        ValueFromPipelineByPropertyName=$true)]
    [String[]]
    $filenames,
    
    #target location for man files
    [String]
    $targetdir = (get-location),
    

    #where to remove the files from
    [String]
    $dest = "$env:windir\system32"
)
    begin{
        $filter = "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\WindowsRuntime\"
        write-host 'Changing to ' $targetdir

        pushd $targetdir
    }

    process{
        foreach($file in $filenames) {
            $manifest = [xml](gc $file)
            $manifest.assembly.file | 
                foreach-object { join-path $dest $_.name} | 
                remove-item -force      
            
            $manifest.assembly.registryKeys.registryKey | 
                where-object { $_.keyName.Contains($filter) } | 
                foreach-object { $_.keyName.Replace("HKEY_LOCAL_MACHINE", "HKLM:")} |
                where-object { $_ -ne $null} |
                where-object { test-path $_ } |
                resolve-path |
                remove-item -force
        }
    }
    end {
        popd
    }
    
}
function unregister-chakraman {
<#
.Synopsis
    UnRegisters given (chakra registered) man files
.Description
    unregister-man can be used to unregister Chakra ABIs with the system.
.Example
    unregister-chakraman
.Notes
 NAME:      unregister-chakraman
 AUTHOR:    REDMOND\fcantonn
 LASTEDIT:  12/10/2012
#Requires -Version 2.0
#>
param(
    [Switch]
    $testOnly
)
    begin{
        $filter = "HKLM:\SOFTWARE\Microsoft\WindowsRuntime\"
        pushd $filter
    }

    process{
        $currentPath = (get-location).Path
        write-host "Current path: " $currentPath

        if ($testOnly)
        {
            # require admin access
            return $true
        }
    
        # pre-count
        $count = (
            (gci -Recurse) | 
                where-object { ($_ -ne $null) -and 
                    ($_.GetValueNames() -contains $ChakraABITestRegistrationIdentifier) -and 
                    ($_.GetValue($ChakraABITestRegistrationIdentifier) -eq $ChakraABITestRegistrationKeyCreatedIdentifier)  }).Count
        
        write-host -foregroundColor gray "Removing $($count) Chakra test manifest entries in registry"
    
        # delete
        (gci -Recurse) | 
            where-object { ($_ -ne $null) -and 
                ($_.GetValueNames() -contains $ChakraABITestRegistrationIdentifier) -and 
                ($_.GetValue($ChakraABITestRegistrationIdentifier) -eq $ChakraABITestRegistrationKeyCreatedIdentifier)  } | 
            remove-item -force

        # post-count
        $count = (
            (gci -Recurse) | 
                where-object { ($_ -ne $null) -and 
                    ($_.GetValueNames() -contains $ChakraABITestRegistrationIdentifier) -and 
                    ($_.GetValue($ChakraABITestRegistrationIdentifier) -eq $ChakraABITestRegistrationKeyCreatedIdentifier)  }).Count
        
        write-host -foregroundColor yellow "Removed all Chakra test manifest entries in registry - now count=$($count)"
    }
    end {
        popd
    }
    
}
Set-Alias register    register-man        -Scope Global -Option AllScope -Force
