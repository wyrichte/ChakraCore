// The location to either read or write. We have a constraint that  
// we must also be able to allocate this much memory. We cannot 
// use a value too big to allocate.  
let pointer_value = 0x21212121; 
 
let ab = new ArrayBuffer( pointer_value + 1 ); 
 
var dv = new DataView( ab ); 
 
let offset = new Number(); 
  
Object.defineProperty(  
    offset,  
    Symbol.toPrimitive,  
    {  
        value: function( hint )  
        { 
            ArrayBuffer.detach( ab );            
            return pointer_value; 
        }  
    }  
); 
 
// This triggers the memory write issue. 
var passed = true;

try {
    dv.setUint8( offset, 0x42 );  
} catch (e) {
    if(!(e instanceof TypeError))
    {
        WScript.Echo("FAIL 1: " + e.message);
        passed = false;
    }
}

let ab2 = new ArrayBuffer( pointer_value + 1 ); 
 
var dv2 = new DataView( ab2 ); 
 
let offset2 = new Number(); 

Object.defineProperty(  
    offset2,  
    Symbol.toPrimitive,  
    {  
        value: function( hint )  
        { 
            ArrayBuffer.detach( ab2 );            
            return pointer_value; 
        }  
    }  
); 
 
// This triggers the memory read issue. 
try {
    dv2.getUint8( offset2 ); 
} catch (e) {
    if(!(e instanceof TypeError))
    {
        WScript.Echo("FAIL 2: " + e.message);
        passed = false;
    }
}

if(passed)
{
    WScript.Echo("PASS");
}

