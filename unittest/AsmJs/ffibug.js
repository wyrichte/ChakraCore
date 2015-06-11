function AsmModule(stdlib,foreign,buffer) {
    "use asm";
    var bb = foreign.a;
    function f1(){
        bb(-2147483648);
        return;
    }
    
    return { 
        f1 : f1
    };
}

var stdlib = {Math:Math,Int8Array:Int8Array,Int16Array:Int16Array,Int32Array:Int32Array,Uint8Array:Uint8Array,Uint16Array:Uint16Array,Uint32Array:Uint32Array,Float32Array:Float32Array,Float64Array:Float64Array,Infinity:Infinity, NaN:NaN}
var env = {"a":function(x){print(x);}}
var buffer = new ArrayBuffer(1<<20);

var asmModule = AsmModule(stdlib,env,buffer);
asmModule.f1();