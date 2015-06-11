function AsmModule(stdlib,foreign,buffer) {
    "use asm";
    //views
    var floor = stdlib.Math.floor;
    var HEAP8  =new stdlib.Int8Array(buffer);
    var HEAPU8  =new stdlib.Uint8Array(buffer);
    var HEAP16  =new stdlib.Int16Array(buffer);
    var HEAPU16  =new stdlib.Uint16Array(buffer);
    var HEAP32 =new stdlib.Int32Array(buffer);
    var HEAPF64 =new stdlib.Float64Array(buffer);
    var HEAPF32 =new stdlib.Float32Array(buffer);
        
    function read8  (x){
        x = x|0; 
        var y = 0;
        var z = 0;
        var k = 1.1;     
        k = +HEAPF64[x>>3];          
        return +k
    }
    return read8
}
var stdlib = {Math:Math,Int32Array:Int32Array,Int8Array:Int8Array,Int16Array:Int16Array,Uint16Array:Uint16Array,Uint8Array:Uint8Array,Float64Array:Float64Array,Float32Array:Float32Array}
var env = {}
var buffer = new ArrayBuffer(1<<20);
var asmModule = AsmModule(stdlib,env,buffer);
print(asmModule(2));
print(asmModule(2));
WScript.Echo(Debug.detachAndFreeObject(buffer));
print(asmModule(4096));