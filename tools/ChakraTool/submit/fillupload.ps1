#
# This script fills IE "Choose File to Upload" dialog.
#

Param(
    [string]$path,
    [switch]$Fail,  # Throws an error if such a dialog exists. Run this beforehand to avoid filling to an existing dialog incorrectly.
    [string]$windowTitle = "Choose File to Upload",
    [int]$timeout = 20 #seconds
)

function Main
{
    if ($Fail -and (FindWindow $windowTitle) -ne 0) {
        throw "ERROR: found an existing `"Choose File to Upload`" dialog. Please close it."
    }

    if (!(Test-Path $path)) {
        throw "ERROR: file does not exist: $path"
    }

    $startTime = [DateTime]::Now
    while ($true) {
        $dlg = FindWindow $windowTitle
        if ($dlg -ne 0) {
            Start-Sleep -Milliseconds 300 # Wait for the new window to warm up...
            if (SetForegroundWindow $dlg) {
                $hWnd = FindWindowEx $dlg -windowClass "ComboBoxEx32"
                $hWnd = FindWindowEx $hWnd -windowClass "ComboBox"
                $hWnd = FindWindowEx $hWnd -windowClass "Edit"
                $fill = Invoke-Win32 "user32.dll" ([long]) "SendMessage" @([IntPtr],[int],[int],[String]) @($hWnd, 0xc, 0, $path)
                if ($fill) {
                    $hWnd = FindWindowEx $dlg -windowClass "Button" -windowTitle "&Open"
                    if (SetForegroundWindow $hWnd) {
                        $click = Invoke-Win32 "user32.dll" ([long]) "SendMessage" @([IntPtr],[int],[int],[long]) @($hWnd, 0xF5, 0, 0)
                    }
                    return
                }
            }
        }

        if (([DateTime]::Now - $startTime).TotalSeconds -gt $timeout) {
            throw "ERROR: timeout"
        }
        Start-Sleep -Milliseconds 200
    }
}

function FindWindow($name, $windowClass) {
    Invoke-Win32 "user32.dll" ([IntPtr]) "FindWindow" @([String],[String]) @($windowClass, $name)
}

function FindWindowEx($hwnd, $hAfterChild, $windowClass, $windowTitle) {
    Invoke-Win32 "user32.dll" ([IntPtr]) "FindWindowEx" @([IntPtr],[IntPtr],[String],[String]) @($hWnd, $hAfterChild, $windowClass, $windowTitle)
}

function SetForegroundWindow($hwnd) {
    Invoke-Win32 "user32.dll" ([bool]) "SetForegroundWindow" @([IntPtr]) @($hwnd)
}

#
# http://www.leeholmes.com/blog/2006/07/21/get-the-owner-of-a-process-in-powershell-%E2%80%93-pinvoke-and-refout-parameters/
#
## Invoke a Win32 P/Invoke call. 
function Invoke-Win32([string] $dllName, [Type] $returnType,  
   [string] $methodName, [Type[]] $parameterTypes, [Object[]] $parameters) 
{ 
   ## Begin to build the dynamic assembly 
   $domain = [AppDomain]::CurrentDomain 
   $name = New-Object Reflection.AssemblyName ‘PInvokeAssembly’ 
   $assembly = $domain.DefineDynamicAssembly($name, ‘Run’) 
   $module = $assembly.DefineDynamicModule(‘PInvokeModule’) 
   $type = $module.DefineType(‘PInvokeType’, “Public,BeforeFieldInit”) 

   ## Go through all of the parameters passed to us.  As we do this, 
   ## we clone the user’s inputs into another array that we will use for 
   ## the P/Invoke call.   
   $inputParameters = @() 
   $refParameters = @() 
   
   for($counter = 1; $counter -le $parameterTypes.Length; $counter++) 
   { 
      ## If an item is a PSReference, then the user  
      ## wants an [out] parameter. 
      if($parameterTypes[$counter - 1] -eq [Ref]) 
      { 
         ## Remember which parameters are used for [Out] parameters 
         $refParameters += $counter 

         ## On the cloned array, we replace the PSReference type with the  
         ## .Net reference type that represents the value of the PSReference,  
         ## and the value with the value held by the PSReference. 
         $parameterTypes[$counter - 1] =  
            $parameters[$counter - 1].Value.GetType().MakeByRefType() 
         $inputParameters += $parameters[$counter - 1].Value 
      } 
      else 
      { 
         ## Otherwise, just add their actual parameter to the 
         ## input array. 
         $inputParameters += $parameters[$counter - 1] 
      } 
   } 

   ## Define the actual P/Invoke method, adding the [Out] 
   ## attribute for any parameters that were originally [Ref]  
   ## parameters. 
   $method = $type.DefineMethod($methodName, ‘Public,HideBySig,Static,PinvokeImpl’,  
      $returnType, $parameterTypes) 
   foreach($refParameter in $refParameters) 
   { 
      $method.DefineParameter($refParameter, “Out”, $null) 
   } 

   ## Apply the P/Invoke constructor 
   $ctor = [Runtime.InteropServices.DllImportAttribute].GetConstructor([string]) 
   $attr = New-Object Reflection.Emit.CustomAttributeBuilder $ctor, $dllName 
   $method.SetCustomAttribute($attr) 

   ## Create the temporary type, and invoke the method. 
   $realType = $type.CreateType() 
   $realType.InvokeMember($methodName, ‘Public,Static,InvokeMethod’, $null, $null,  
      $inputParameters) 

   ## Finally, go through all of the reference parameters, and update the 
   ## values of the PSReference objects that the user passed in. 
   foreach($refParameter in $refParameters) 
   { 
      $parameters[$refParameter - 1].Value = $inputParameters[$refParameter - 1] 
   } 
} 

. Main
