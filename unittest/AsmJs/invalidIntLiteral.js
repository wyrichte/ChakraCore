function AsmModule(stdlib,foreign,buffer) {
    "use asm";
    function f1(){
        var bb = -2147483649;
        return +bb;
    }
    
    return { 
        f1 : f1
    };
}
var stdlib = {}
var env = {}
var buffer = new ArrayBuffer(1<<20);
var asmModule = AsmModule(stdlib,env,buffer);
WScript.Echo(asmModule.f1());
