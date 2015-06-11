function AsmModule(stdlib,foreign,buffer) {
    "use asm";
    //views
    var floor = stdlib.Math.floor;
    var H64=stdlib.Float64Array;
    var HEAPF64 =new stdlib.Float64Array(buffer);
    var detach = foreign.detachBuffer;
    var len = stdlib.byteLength;
    function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; HEAPF64=new H64(b2); buffer=b2; return true }
    function fun  (x){
        x = x|0; 
        var y = 0;
        var z = 0;
        var k = 1.1;     
        detach();
        k = +HEAPF64[x>>3];
        return +k;
    }
    return {fun:fun, ch:ch}
}
this['byteLength'] =
  Function.prototype.call.bind(Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength').get);
var stdlib = this;
var buffer = new ArrayBuffer(1<<20);
var env = {detachBuffer:function(){Debug.detachAndFreeObject(buffer); buffer =new ArrayBuffer(1<<24); print(asmModule.ch(buffer)); print("success");}}
var asmModule = AsmModule(stdlib,env,buffer);
print(asmModule.fun(2));
print(asmModule.fun(2));
Debug.detachAndFreeObject(buffer);
asmModule.ch(new ArrayBuffer(1<<24));
print(asmModule.fun(4096));