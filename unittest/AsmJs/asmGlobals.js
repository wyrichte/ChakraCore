//
// Test cases below use code modified from Mozilla's asm.js test suite
//

function AsmModule(global, imp, heap) {
    "use asm";
    var i1=42;
    const i2=42; 
    var i3=4.2;
    const i4=4.2;
    var i5=13;
    var d1=.1;
    var i6=13;
    var i7=13;
    var i8=13;
    var i9=global.Infinity;
    var i10=global.NaN;
    const i11=13;
    const i12=13.37;
    var f32 = global.Math.fround;
    const i13=f32(13.37); 
    const i14=global.Infinity; 
    const i15=global.NaN;
    var i16=imp.i1|0;
    const i17=imp.i2|0;
    var i18=imp.i3|0;
    const i19=imp.i4|0; 
    var i20=+imp.i5; 
    const i21=+imp.i6;
    var i22=+imp.i7;
    const i23=+imp.i8;
    var g1=0; 
    var i32 = new global.Int32Array(heap);
    var g2=0;
    var g3=0;
    var g4=0;
    var h1=0;
    function f1(){ return i1|0 }
    function f2(){ return i2|0 } 
    function f3(){ return +i3 }
    function f4(){ return +i4 }
    function f5(j) { j=j|0; i5=j; return i5|0 }
    function f6(e) { e=+e; d1=e; return +e }
    function f7(i6, j) { i6=i6|0; j=j|0; i6=j; return i6|0 }
    function f8(j) { j=j|0; var i7=0; i7=j; return i7|0 }
    function f9(j) { j=j|0; if ((j|0) != -1) { i8=j } else { return i8|0 } return 0 }
    function f10() { var j=i9; return +j }
    function f11() { var j=i10; return +j }
    function f12() { var j=i11; return j|0 }
    function f13() { var j=i12; return +j }
    function f14() { var j=i13; return f32(j) }
    function f15() { return +i9 }
    function f16() { return +i14 }
    function f17() { return +i15 }
    function f18() { return i16|0 }
    function f19() { return i17|0 }
    function f20() { return i18|0 }
    function f21() { return i19|0 }
    function f22() { return +i20 }
    function f23() { return +i21 }
    function f24() { return +i22 }
    function f25() { return +i23 }
    function f26() { var i=42; while (1) { break; } g1 = i; return g1|0 }
    function f27() { return i32[4]|0 }
    function f28() { var x = 0; g2 = 1; x = g2; return (x+g2)|0 }
    function f29() { var x = 0; g3 = 1; x = g3; g3 = 2; return (x+g3)|0 }
    function f30() { var x = 0; g4 = 1; x = g4; h1 = 3; return (x+g4)|0 }
    return {
        f1:f1,
        f2:f2,
        f3:f3,
        f4:f4,
        f5:f5,
        f6:f6,
        f7:f7,
        f8:f8,
        f9:f9,
        f10:f10,
        f11:f11,
        f12:f12,
        f13:f13,
        f14:f14,
        f15:f15,
        f16:f16,
        f17:f17,
        f18:f18,
        f19:f19,
        f20:f20,
        f21:f21,
        f22:f22,
        f23:f23,
        f24:f24,
        f25:f25,
        f26:f26,
        f27:f27,
        f28:f28,
        f29:f29,
        f30:f30,
    }
}
var global = {Math:Math,Int8Array:Int8Array,Int16Array:Int16Array,Int32Array:Int32Array,Uint8Array:Uint8Array,Uint16Array:Uint16Array,Uint32Array:Uint32Array,Float32Array:Float32Array,Float64Array:Float64Array,Infinity:Infinity, NaN:NaN}
var env = {id: function(x){return x;},i1:42,i2:42,i3:1.4,i4:1.4,i5:42,i6:42,i7:1.4,i8:1.4}
var heap = new ArrayBuffer(1<<20);
var asmModule = AsmModule(global, env, heap);

print(asmModule.f1());
print(asmModule.f2());
print(asmModule.f3());
print(asmModule.f4());
print(asmModule.f5(42));
print(asmModule.f6(42.1));
print(asmModule.f7(42, 43));
print(asmModule.f8(42));
print(asmModule.f9(-1));
print(asmModule.f9(42));
print(asmModule.f9(-1));
print(asmModule.f10());
print(asmModule.f11());
print(asmModule.f12());
print(asmModule.f13());
print(asmModule.f14());
print(asmModule.f15());
print(asmModule.f16());
print(asmModule.f17());
print(asmModule.f18());
print(asmModule.f19());
print(asmModule.f20());
print(asmModule.f21());
print(asmModule.f22());
print(asmModule.f23());
print(asmModule.f24());
print(asmModule.f25());
print(asmModule.f26());
new Int32Array(heap)[4] = 42;
print(asmModule.f27());
print(asmModule.f28());
print(asmModule.f29());
print(asmModule.f30());