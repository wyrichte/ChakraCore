var a=()=>super.base();
function b() { return eval("super.base()") }
try {a()} catch(e) { WScript.Echo(e) } 
try {b()} catch(e) { WScript.Echo(e) } 

