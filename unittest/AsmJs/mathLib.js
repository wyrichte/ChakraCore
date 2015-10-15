//
// Test cases below use code modified from Mozilla's asm.js test suite
//

function AsmModule(glob, imp, heap) {
    "use asm";
    const si=glob.Math.sin;
    var co=glob.Math.cos;
    var ta=glob.Math.tan; 
    var as=glob.Math.asin; 
    var ac=glob.Math.acos;
    var at=glob.Math.atan;
    var ce=glob.Math.ceil;
    var fl=glob.Math.floor;
    var exq=glob.Math.exp;
    var lo=glob.Math.log;
    var sq=glob.Math.sqrt;
    var abs=glob.Math.abs;
    var po=glob.Math.pow;
    var at2=glob.Math.atan2;
    var min=glob.Math.min;
    var max=glob.Math.max;
    var e=glob.Math.E;
    var ln10=glob.Math.LN10;
    var ln2=glob.Math.LN2;
    var log2e=glob.Math.LOG2E;
    var log10e=glob.Math.LOG10E;
    var pi=glob.Math.PI;
    var sqrt1_2=glob.Math.SQRT1_2;
    var sqrt2=glob.Math.SQRT2;
    
    function f1(d) { d=+d; return +si(d) }
    function f2(d) { d=+d; return +si(d) }
    function f3(d) { d=+d; return +co(d) }
    function f4(d) { d=+d; return +ta(d) }
    function f5(d) { d=+d; return +as(d) } 
    function f6(d) { d=+d; return +ac(d) }
    function f7(d) { d=+d; return +at(d) }
    function f8(d) { d=+d; return +ce(d) }
    function f9(d) { d=+d; return +fl(d) }
    function f10(d) { d=+d; return +exq(d) }
    function f11(d) { d=+d; return +lo(d) }
    function f12(d) { d=+d; return +sq(d) }
    function f13(d) { d=+d; return +abs(d) }
    function f14(i) { i=i|0; return abs(i|0)|0 }
    
    function f15(d,e) { d=+d;e=+e; return +po(d,e) }
    function f16(d,e) { d=+d;e=+e; return +at2(d,e) }
    function f17(d,e) { d=+d;e=+e; return +min(d,e) }
    function f18(d,e) { d=d|0;e=e|0; return min(d|0,e|0)|0    }
    function f19(d,e) { d=+d;e=+e; return +max(d,e) }
    function f20(d,e) { d=d|0;e=e|0; return max(d|0,e|0)|0 }
    function f21(d,e,g) { d=d|0;e=e|0;g=g|0; return max(d|0,e|0,g|0)|0 }
    function f22(d,e,g) { d=+d;e=+e;g=+g; return +max(d,e,g) }
    function f23(d,e,g) { d=d|0;e=e|0;g=g|0; return min(d|0,e|0,g|0)|0 }
    function f24(d,e,g) { d=+d;e=+e;g=+g; return +min(d,e,g) }
    
    function f25() { return +e }
    function f26() { return +ln10 }
    function f27() { return +ln2}
    function f28() { return +log2e }
    function f29() { return +log10e }
    function f30() { return +pi}
    function f31() { return +sqrt1_2 }
    function f32() { return +sqrt2 }
    
    function f33(d) { d=+d; return si(d) }
    function f34(d) { d=+d; return si(d) }
    function f35(d) { d=+d; return co(d) }
    function f36(d) { d=+d; return ta(d) }
    function f37(d) { d=+d; return as(d) } 
    function f38(d) { d=+d; return ac(d) }
    function f39(d) { d=+d; return at(d) }
    function f40(d) { d=+d; return ce(d) }
    function f41(d) { d=+d; return fl(d) }
    function f42(d) { d=+d; return exq(d) }
    function f43(d) { d=+d; return lo(d) }
    function f44(d) { d=+d; return sq(d) }
    function f45(d) { d=+d; return abs(d) }
    function f46(i) { i=i|0; var a = 0; a = abs(i|0); return a|0 }
    
    function f47(d,e) { d=+d;e=+e; return po(d,e) }
    function f48(d,e) { d=+d;e=+e; return at2(d,e) }
    function f49(d,e) { d=+d;e=+e; return min(d,e) }
    function f50(d,e) { d=d|0;e=e|0; return min(d|0,e|0)   }
    function f51(d,e) { d=+d;e=+e; return max(d,e) }
    function f52(d,e) { d=d|0;e=e|0; return max(d|0,e|0) }
    function f53(d,e,g) { d=d|0;e=e|0;g=g|0; return max(d|0,e|0,g|0) }
    function f54(d,e,g) { d=+d;e=+e;g=+g; return max(d,e,g) }
    function f55(d,e,g) { d=d|0;e=e|0;g=g|0; return min(d|0,e|0,g|0) }
    function f56(d,e,g) { d=+d;e=+e;g=+g; return min(d,e,g) }
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
        f32:f32,
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
        f56:f56
    }
}

var global = this;
var env = {}
var heap = new ArrayBuffer(1<<20);
var asmModule = AsmModule(global, env, heap);

function testUnary(f) {
    var numbers = [NaN, Infinity, -Infinity, -10000, -3.4, -0, 0, 3.4, 10000];
    for (n of numbers)
        print(f(n));
}
testUnary(asmModule.f1);
testUnary(asmModule.f2);
testUnary(asmModule.f3);
testUnary(asmModule.f4);
testUnary(asmModule.f5);
testUnary(asmModule.f6);
testUnary(asmModule.f7);
testUnary(asmModule.f8);
testUnary(asmModule.f9);
testUnary(asmModule.f10);
testUnary(asmModule.f11);
testUnary(asmModule.f12);
testUnary(asmModule.f13);
for (n of [-Math.pow(2,31)-1, -Math.pow(2,31), -Math.pow(2,31)+1, -1, 0, 1, Math.pow(2,31)-2, Math.pow(2,31)-1, Math.pow(2,31)])
    print(asmModule.f14());

var doubleNumbers = [NaN, Infinity, -Infinity, -10000, -3.4, -0, 0, 3.4, 10000];
var intNumbers = [-10000, -3, -1, 0, 3, 10000];
function testBinary(f, numbers) {
    for (n of numbers)
        for (o of numbers)
            print(f(n,o));
}

testBinary(asmModule.f15,doubleNumbers);
testBinary(asmModule.f16,doubleNumbers);
testBinary(asmModule.f17,doubleNumbers);
testBinary(asmModule.f18,intNumbers);
testBinary(asmModule.f19,doubleNumbers);
testBinary(asmModule.f20,intNumbers);

function testTernary(f, numbers) {
    for (n of numbers)
        for (o of numbers)
            for (p of numbers)
                print(f(n,o,p));
}

testTernary(asmModule.f21,intNumbers);
testTernary(asmModule.f22,doubleNumbers);
testTernary(asmModule.f23,intNumbers);
testTernary(asmModule.f24,doubleNumbers);

print(asmModule.f25());
print(asmModule.f26());
print(asmModule.f27());
print(asmModule.f28());
print(asmModule.f29());
print(asmModule.f30());
print(asmModule.f31());
print(asmModule.f32());


testUnary(asmModule.f33);
testUnary(asmModule.f34);
testUnary(asmModule.f35);
testUnary(asmModule.f36);
testUnary(asmModule.f37);
testUnary(asmModule.f38);
testUnary(asmModule.f39);
testUnary(asmModule.f40);
testUnary(asmModule.f41);
testUnary(asmModule.f42);
testUnary(asmModule.f43);
testUnary(asmModule.f44);
testUnary(asmModule.f45);
for (n of [-Math.pow(2,31)-1, -Math.pow(2,31), -Math.pow(2,31)+1, -1, 0, 1, Math.pow(2,31)-2, Math.pow(2,31)-1, Math.pow(2,31)])
    print(asmModule.f46());

testBinary(asmModule.f47,doubleNumbers);
testBinary(asmModule.f48,doubleNumbers);
testBinary(asmModule.f49,doubleNumbers);
testBinary(asmModule.f50,intNumbers);
testBinary(asmModule.f51,doubleNumbers);
testBinary(asmModule.f52,intNumbers);

testTernary(asmModule.f53,intNumbers);
testTernary(asmModule.f54,doubleNumbers);
testTernary(asmModule.f55,intNumbers);
testTernary(asmModule.f56,doubleNumbers);
