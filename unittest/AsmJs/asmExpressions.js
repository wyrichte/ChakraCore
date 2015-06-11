//
// Test cases below use code modified from Mozilla's asm.js test suite
//

const UINT32_MAX = Math.pow(2,32)-1;
const INT32_MIN = -Math.pow(2,31);
const INT32_MAX = Math.pow(2,31)-1;

function AsmModule(glob, imp, b) {
    "use asm";
    
    const i8=new glob.Int8Array(b);
    var u8=new glob.Uint8Array(b);
    const i16=new glob.Int16Array(b);
    var u16=new glob.Uint16Array(b);
    const i32=new glob.Int32Array(b);
    var u32=new glob.Uint32Array(b);
    const f32=new glob.Float32Array(b);
    var f64=new glob.Float64Array(b);
    
    var sqrt=glob.Math.sqrt;
    var imul=glob.Math.imul; 
    var x1=0; 
    
    function f1(i) { i=i|0; return (i*2)|0 }
    function f2(i) { i=i|0; return (2*i)|0 }
    function f3(i) { i=i|0; return (i*1048575)|0 }
    function f4(i) { i=i|0; return (1048575*i)|0 }
    function f5(i) { i=+i; var j=0; j=~~i; return j|0 }
    function f6(i) { i=i|0; var j=0.0; j=+~~i; return +j }
    function f7(i) { i=i|0; var j=0.1; j=+(i>>>0); return +j }
    function f8(i) { i=i|0; return (-i)|0 }
    function f9(i) { i=+i; return +(-i) }
    function f10(i,j) { i=i|0;j=j|0; return ((i|0) < (j|0))|0 }
    function f11(i,j) { i=i|0;j=j|0; return ((i>>>0) < (j>>>0))|0 }
    function f12(i,j) { i=i|0;j=j|0; var k=0; k=(i|0)==(j|0); return k|0 }
    function f13(i,j) { i=i|0;j=j|0; var k=0; k=(i|0)!=(j|0); return k|0 }
    function f14(i,j) { i=i|0;j=j|0; var k=0; k=(i|0)<(j|0); return k|0 }
    function f15(i,j) { i=i|0;j=j|0; var k=0; k=(i|0)>(j|0); return k|0 } 
    function f16(i,j) { i=i|0;j=j|0; var k=0; k=(i|0)<=(j|0); return k|0 }
    function f17(i,j) { i=i|0;j=j|0; var k=0; k=(i|0)>=(j|0); return k|0 }
    
    function f18(i,j) { i=i|0;j=j|0; return ((i|0)/(j|0))|0 }
    function f19(i,j) { i=i|0;j=j|0; return ((i>>>0)/(j>>>0))|0 }
    function f20(i,j) { i=i|0;j=j|0; var k = 0; k = (i|0)%(j|0)|0; return k|0 }
    function f21(i,j) { i=i|0;j=j|0; var k = 0; k = (i|0)%4|0; return k|0 }
    function f22(i,j) { i=i|0;j=j|0; var k = 0; k = (i>>>0)%(j>>>0)|0; return k|0 }
    
    function f23() { return (4 / 2)|0 }
    function f24() { return (3 / 2)|0 }
    function f25() { return (4 % 2)|0 }
    function f26() { return (3 % 2)|0 }
    
    function f27() { var i=42,j=1.1; return +(i?+(i|0):j) }
    function f28() { var i=42,j=1; return (i?i:j)|0 }
    
    function f29(i,j) { i=i|0;j=j|0; return ((i|0)>(j|0)?(i+10)|0:(j+100)|0)|0 }
    
    function f30(i,j,k) { i=i|0;j=j|0;k=k|0; return ((i|0)>(j|0) ? (i|0)>(k|0) ? i : k : (j|0)>(k|0) ? j : k)|0 }
    
    function f31(i,j) { i=i|0;j=j|0; var a=0,b=0; a=i>>>0 < 4294967292; b=(i|0) < -4; return (j ? a : b)|0 }
    
    function fn32() { return +(f64[0] + 2.0) }
    function f33() { return +(f64[0] - 2.0) }
    function f34() { return +(f64[0] * 2.0) }
    function f35() { return +(f64[0] / 2.0) }
    function f36() { return +(f64[0] % 2.0) } 
    function f37() { return +-f64[0] }
    function f38() { return +sqrt(f64[0]) }
    function f39() { return imul(i32[0], 2)|0 } 
    
    function f40(i) { i=i|0; if (i) { i = ((i|0) == 2); } else { i=(i-1)|0 } return i|0; }
    function f41(i) { i=i|0; if (i) { i = !i } else { i=(i-1)|0 } return i|0; }
    
    function f42() { return (4 | (2 == 2))|0 }
    function f43() { return (4 | (!2))|0 }
    
    function f44() { i32[((b1()|0) & 0x3) >> 2] = a1()|0 }
        function a1() { return x1|0 }
        function b1() { x1=42; return 0 }
    
    function f45() { var a=0,i=0; for (; ~~i!=4; i=(i+1)|0) { a = (a*5)|0; if (+(a>>>0) != 0.0) return 1; } return 0; }
    
    function f46(i) { i=i|0; return ((i|0)/1)|0; }
    function f47(i) { i=i|0; return ((i|0)/2)|0; }
    function f48(i) { i=i|0; return ((i|0)/4)|0; }
    function f49(i) { i=i|0; return ((i|0)/1073741824)|0; }
    function f50(i) { i=i|0; return ((((i|0)/1)|0)+i)|0; }
    
    function f51(i) { i=i|0; i=(i&2147483647)|0; return ((i|0)/1)|0; }
    function f52(i) { i=i|0; i=(i&2147483647)|0; return ((i|0)/2)|0; }
    function f53(i) { i=i|0; i=(i&2147483647)|0; return ((i|0)/4)|0; }
    function f54(i) { i=i|0; i=(i&2147483647)|0; return ((i|0)/1073741824)|0; }
    function f55(i) { i=i|0; i=(i&2147483647)|0; return ((((i|0)/1)|0)+i)|0; }
    
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
        f31:f31,
        fn32:fn32,
        f33:f33,
        f34:f34,
        f35:f35,
        f36:f36,
        f37:f37,
        f38:f38,
        f39:f39,
        f40:f40,
        f41:f41,
        f42:f42,
        f43:f43,
        f44:f44,
        f45:f45,
        f46:f46,
        f47:f47,
        f48:f48,
        f49:f49,
        f50:f50,
        f51:f51,
        f52:f52,
        f53:f53,
        f54:f54,
        f55:f55,
    }
}

var global = {Math:Math,Int8Array:Int8Array,Int16Array:Int16Array,Int32Array:Int32Array,Uint8Array:Uint8Array,Uint16Array:Uint16Array,Uint32Array:Uint32Array,Float32Array:Float32Array,Float64Array:Float64Array,Infinity:Infinity, NaN:NaN}
var env = {}
var heap = new ArrayBuffer(1<<20);
var asmModule = AsmModule(global, env, heap);

print(asmModule.f1(0));
print(asmModule.f1(INT32_MIN));
print(asmModule.f1(INT32_MAX));

print(asmModule.f2(0));
print(asmModule.f2(INT32_MIN));
print(asmModule.f2(INT32_MAX));

print(asmModule.f3(0));
print(asmModule.f3(2));
print(asmModule.f3(-1));
print(asmModule.f3(INT32_MIN));
print(asmModule.f3(INT32_MAX));

print(asmModule.f4(0));
print(asmModule.f4(2));
print(asmModule.f4(-1));
print(asmModule.f4(INT32_MIN));
print(asmModule.f4(INT32_MAX));

print(asmModule.f5(0));
print(asmModule.f5(3.5));
print(asmModule.f5(-3.5));
print(asmModule.f5(INT32_MIN));
print(asmModule.f5(INT32_MAX));
print(asmModule.f5(UINT32_MAX));

print(asmModule.f6(0));
print(asmModule.f6(INT32_MIN));
print(asmModule.f6(INT32_MAX));

print(asmModule.f7(0));
print(asmModule.f7(INT32_MIN));
print(asmModule.f7(INT32_MAX));

print(asmModule.f8(0));
print(asmModule.f8(-0));
print(asmModule.f8(-1));
print(asmModule.f8(INT32_MIN));
print(asmModule.f8(INT32_MAX));

print(asmModule.f9(0));
print(asmModule.f9(-0));
print(asmModule.f9(-1));
print(asmModule.f9(1));
print(asmModule.f9(Math.pow(2,50)));
print(asmModule.f9(1.54e20));

print(asmModule.f10(0, 1));
print(asmModule.f10(1, 0));
print(asmModule.f10(1, 1));
print(asmModule.f10(INT32_MIN, INT32_MAX));
print(asmModule.f10(INT32_MAX, INT32_MIN));
print(asmModule.f10(0, INT32_MAX));
print(asmModule.f10(INT32_MAX, 0));
print(asmModule.f10(INT32_MIN, 0));
print(asmModule.f10(0, INT32_MIN));
print(asmModule.f10(UINT32_MAX, 0));
print(asmModule.f10(0, UINT32_MAX));

print(asmModule.f11(0, 1));
print(asmModule.f11(1, 0));
print(asmModule.f11(1, 1));
print(asmModule.f11(INT32_MIN, INT32_MAX));
print(asmModule.f11(INT32_MAX, INT32_MIN));
print(asmModule.f11(0, INT32_MAX));
print(asmModule.f11(INT32_MAX, 0));
print(asmModule.f11(INT32_MIN, 0));
print(asmModule.f11(0, INT32_MIN));
print(asmModule.f11(UINT32_MAX, 0));
print(asmModule.f11(0, UINT32_MAX));

print(asmModule.f12(1, 2));
print(asmModule.f13(1, 2));
print(asmModule.f14(1, 2));
print(asmModule.f15(1, 2));
print(asmModule.f16(1, 2));
print(asmModule.f17(1, 2));

print(asmModule.f18(4, 2));
print(asmModule.f18(3, 2));
print(asmModule.f18(3,-2));
print(asmModule.f18(-3,-2));
print(asmModule.f18(0,-1));
print(asmModule.f18(0, INT32_MAX));
print(asmModule.f18(0, INT32_MIN));
print(asmModule.f18(INT32_MAX, 0));
print(asmModule.f18(INT32_MIN, 0));
print(asmModule.f18(-1, INT32_MAX));
print(asmModule.f18(-1, INT32_MIN));
print(asmModule.f18(INT32_MAX, -1));
print(asmModule.f18(INT32_MIN, -1));
print(asmModule.f18(INT32_MAX, INT32_MAX));
print(asmModule.f18(INT32_MAX, INT32_MIN));
print(asmModule.f18(INT32_MIN, INT32_MAX));
print(asmModule.f18(INT32_MIN, INT32_MIN));

print(asmModule.f19(4, 2));
print(asmModule.f19(3, 2));
print(asmModule.f19(3,-2));
print(asmModule.f19(-3,-2));
print(asmModule.f19(0,-1));
print(asmModule.f19(0, INT32_MAX));
print(asmModule.f19(0, INT32_MIN));
print(asmModule.f19(0, UINT32_MAX));
print(asmModule.f19(INT32_MAX, 0));
print(asmModule.f19(INT32_MIN, 0));
print(asmModule.f19(UINT32_MAX, 0));
print(asmModule.f18(-1, INT32_MAX));
print(asmModule.f18(-1, INT32_MIN));
print(asmModule.f18(-1, UINT32_MAX));
print(asmModule.f19(INT32_MAX, -1));
print(asmModule.f19(INT32_MIN, -1));
print(asmModule.f19(UINT32_MAX, -1));
print(asmModule.f19(INT32_MAX, INT32_MAX));
print(asmModule.f19(INT32_MAX, INT32_MIN));
print(asmModule.f19(UINT32_MAX, INT32_MIN));
print(asmModule.f19(INT32_MAX, UINT32_MAX));
print(asmModule.f19(UINT32_MAX, UINT32_MAX));
print(asmModule.f19(INT32_MIN, INT32_MAX));
print(asmModule.f19(INT32_MIN, UINT32_MAX));
print(asmModule.f19(INT32_MIN, INT32_MIN));

print(asmModule.f20(4, 2));
print(asmModule.f20(3, 2));
print(asmModule.f20(3,-2));
print(asmModule.f20(-3,-2));
print(asmModule.f20(0,-1));
print(asmModule.f20(0, INT32_MAX));
print(asmModule.f20(0, INT32_MIN));
print(asmModule.f20(INT32_MAX, 0));
print(asmModule.f20(INT32_MIN, 0));
print(asmModule.f20(-1, INT32_MAX));
print(asmModule.f20(-1, INT32_MIN));
print(asmModule.f20(INT32_MAX, -1));
print(asmModule.f20(INT32_MIN, -1));
print(asmModule.f20(INT32_MAX, INT32_MAX));
print(asmModule.f20(INT32_MAX, INT32_MIN));
print(asmModule.f20(INT32_MIN, INT32_MAX));
print(asmModule.f20(INT32_MIN, INT32_MIN));

print(asmModule.f21(0));
print(asmModule.f21(-1));
print(asmModule.f21(-3));
print(asmModule.f21(-4));
print(asmModule.f21(INT32_MIN));
print(asmModule.f21(3));
print(asmModule.f21(4));
print(asmModule.f21(INT32_MAX));

print(asmModule.f22(4, 2));
print(asmModule.f22(3, 2));
print(asmModule.f22(3,-2));
print(asmModule.f22(-3,-2));
print(asmModule.f22(0,-1));
print(asmModule.f22(0, INT32_MAX));
print(asmModule.f22(0, INT32_MIN));
print(asmModule.f22(0, UINT32_MAX));
print(asmModule.f22(INT32_MAX, 0));
print(asmModule.f22(INT32_MIN, 0));
print(asmModule.f22(UINT32_MAX, 0));
print(asmModule.f22(-1, INT32_MAX));
print(asmModule.f22(-1, INT32_MIN));
print(asmModule.f22(-1, UINT32_MAX));
print(asmModule.f22(INT32_MAX, -1));
print(asmModule.f22(INT32_MIN, -1));
print(asmModule.f22(UINT32_MAX, -1));
print(asmModule.f22(INT32_MAX, INT32_MAX));
print(asmModule.f22(INT32_MAX, INT32_MIN));
print(asmModule.f22(UINT32_MAX, INT32_MIN));
print(asmModule.f22(INT32_MAX, UINT32_MAX));
print(asmModule.f22(UINT32_MAX, UINT32_MAX));
print(asmModule.f22(INT32_MIN, INT32_MAX));
print(asmModule.f22(INT32_MIN, UINT32_MAX));
print(asmModule.f22(INT32_MIN, INT32_MIN));

print(asmModule.f23());
print(asmModule.f24());
print(asmModule.f25());
print(asmModule.f26());
print(asmModule.f27());
print(asmModule.f28());

print(asmModule.f29(2, 4));
print(asmModule.f29(-2, -4));

print(asmModule.f30(1,2,3));
print(asmModule.f30(1,3,2));
print(asmModule.f30(2,1,3));
print(asmModule.f30(2,3,1));
print(asmModule.f30(3,1,2));
print(asmModule.f30(3,2,1));

print(asmModule.f31(1,true));
print(asmModule.f31(-1,true));
print(asmModule.f31(-5,true));
print(asmModule.f31(1,false));
print(asmModule.f31(-1,false));
print(asmModule.f31(-5,false));

new Float64Array(heap)[0] = 2.3;
print(asmModule.fn32());
print(asmModule.f33());
print(asmModule.f34());
print(asmModule.f35());
print(asmModule.f36());
print(asmModule.f37());
print(asmModule.f38());
new Int32Array(heap)[0] = 42;
print(asmModule.f39());

print(asmModule.f40(0));
print(asmModule.f40(1));
print(asmModule.f40(2));
print(asmModule.f41(0));
print(asmModule.f41(1));
print(asmModule.f41(2));

print(asmModule.f42());
print(asmModule.f43());

asmModule.f44();
print(new Int32Array(heap)[0]);

print(asmModule.f45());

for (let i = 0; i < 31; i++) {
    print(asmModule.f46(Math.pow(2,i)));
    print(asmModule.f46(Math.pow(2,i)-1));
    print(asmModule.f46(-Math.pow(2,i)));
    print(asmModule.f46(-Math.pow(2,i)-1));
}
print(asmModule.f46(INT32_MIN));
print(asmModule.f46(INT32_MAX));

for (let i = 0; i < 31; i++) {
    print(asmModule.f47(Math.pow(2,i)));
    print(asmModule.f47(Math.pow(2,i)-1));
    print(asmModule.f47(-Math.pow(2,i)));
    print(asmModule.f47(-Math.pow(2,i)-1));
}
print(asmModule.f47(INT32_MIN));
print(asmModule.f47(INT32_MAX));

for (let i = 0; i < 31; i++) {
    print(asmModule.f48(Math.pow(2,i)));
    print(asmModule.f48(Math.pow(2,i)-1));
    print(asmModule.f48(-Math.pow(2,i)));
    print(asmModule.f48(-Math.pow(2,i)-1));
}
print(asmModule.f48(INT32_MIN));
print(asmModule.f48(INT32_MAX));

for (let i = 0; i < 31; i++) {
    print(asmModule.f49(Math.pow(2,i)));
    print(asmModule.f49(Math.pow(2,i)-1));
    print(asmModule.f49(-Math.pow(2,i)));
    print(asmModule.f49(-Math.pow(2,i)-1));
}
print(asmModule.f49(INT32_MIN));
print(asmModule.f49(INT32_MAX));

for (let i = 0; i < 31; i++) {
    print(asmModule.f50(Math.pow(2,i)));
    print(asmModule.f50(Math.pow(2,i)-1));
    print(asmModule.f50(-Math.pow(2,i)));
    print(asmModule.f50(-Math.pow(2,i)-1));
}
print(asmModule.f50(INT32_MIN));
print(asmModule.f50(INT32_MAX));

for (let i = 0; i < 31; i++) {
    print(asmModule.f51(Math.pow(2,i)));
    print(asmModule.f51(Math.pow(2,i+1)-1));
}
for (let i = 0; i < 31; i++) {
    print(asmModule.f52(Math.pow(2,i)));
    print(asmModule.f52(Math.pow(2,i+1)-1));
}
for (let i = 0; i < 31; i++) {
    print(asmModule.f53(Math.pow(2,i)));
    print(asmModule.f53(Math.pow(2,i+1)-1));
}
for (let i = 0; i < 31; i++) {
    print(asmModule.f54(Math.pow(2,i)));
    print(asmModule.f54(Math.pow(2,i+1)-1));
}
for (let i = 0; i < 31; i++) {
    print(asmModule.f55(Math.pow(2,i)));
    print(asmModule.f55(Math.pow(2,i+1)-1));
}