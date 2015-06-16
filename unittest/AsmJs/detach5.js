function AsmModule(stdlib,foreign,buffer) {
    "use asm";
    //views
    var floor = stdlib.Math.floor;
    var H64=stdlib.Float64Array;
    var HEAPF64 =new stdlib.Float64Array(buffer);
    var detach = foreign.detachBuffer;
    var len = stdlib.byteLength;
    function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; HEAPF64=new H64(b2); buffer=b2; return true }
    function read8  (x){
        x = x|0; 
        var y = 0;
        var z = 0;
        var k = 1.1;     
        detach();
        k = +HEAPF64[x>>3];
        return +k;
    }
    return read8;
}
this['byteLength'] =
  Function.prototype.call.bind(Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength').get);
var stdlib = this;
var buffer = new ArrayBuffer(0x1000000);
var env = {detachBuffer:function(){Debug.detachAndFreeObject(buffer); print("success");}}
var asmModule = AsmModule(stdlib,env,buffer);
try {
    print(asmModule(2));
} catch (e) {
    print(e.stack);
}
