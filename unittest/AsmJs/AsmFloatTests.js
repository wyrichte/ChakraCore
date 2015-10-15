//
// Test cases below use code modified from Mozilla's asm.js test suite
//

function AsmModule(glob, env, heap) {
    "use asm";
    
    var toF = glob.Math.fround;
    var inf = glob.Infinity;
    var ffi = env.ffi;
    var ffi2 = env.ffi2;
    var g = 4;
    var x = toF(3.5);
    var y = toF(3);
    var z = toF(env.x);
    var w = toF(env.y);
    var special = glob.NaN;
    var special2 = glob.Infinity;
    var abs = glob.Math.abs;
    var sqrt = glob.Math.sqrt;
    var floor = glob.Math.floor;
    var ceil = glob.Math.ceil;
    var cos = glob.Math.cos;
    var pow = glob.Math.pow;
    
    var f32 = new glob.Float32Array(heap);
    var f64 = new glob.Float64Array(heap); 
    
    function f1(i) { i = toF(i); }
    function f2() { var i = toF(5); }
    function f3() { var i = toF(5.); }
    function f4() { return toF(42); }
    function f5() { return toF(0.); }
    function f6() { return toF(-0.); }
    function f7() { return toF(inf); }
    function f8() { return toF(13.37); }
    function f9() { return +toF(4.); }
    function f0() { return +~~toF(4.5); }
    function fa() { return toF(w); }
    function fb() { var i = toF(5.); return toF(i); }
    function fc() { var i = toF(5.); i = toF(42); return toF(i); }
    function fd() { var i = toF(5.); i = toF(6.); return toF(i); } 
    function fe() { var i = toF(5.); f32[0] = toF(6.); i = toF(f32[0]); return toF(i); }
    function ff() { var i = 5.; f32[0] = i; return toF(f32[0]); } 
    function fg() { var i = toF(5.); f64[0] = i; return +f64[0]; } 
    function fh() { f32[0] = 1.5; return +f32[0]; }
    function fi() { f64[0] = 1.5; return toF(f64[0]); }
    function fj(x) { x = toF(x); var n = 0.; n = +x; return +n; }
    function fk(x) { x = toF(x); var n = 0; n = ~~x; return n | 0; }
    function fl(x) { x = toF(x); var n = 0; n = ~~x >>> 0; return n | 0; }
    function fm(x) { x = toF(x); f32[0] = x; return +f32[0]; }
    function fn(x) { x = toF(x); f32[0] = x; return toF(f32[0]); }
    function fo(x) { x = toF(x); y = x; return toF(y); }
    function fq(x) { x = x|0; return toF(~~x); } 
    function fr(x) { x = x|0; return toF(x >> 0); }
    function fs(x) { x = +x; return toF(x); }
    function ft(x) { x = x|0; return toF(x >>> 0); }
    function fu(x) { x = x|0; return toF(x>>>0) }
    function fv() {var g=toF(0); g=toF(special); return toF(g);}
    function fw() {var g=toF(0); g=toF(special2); return toF(g);}
    function fx() {var x = toF(1.5); return toF(abs(x))}
    function fy() {var x = toF(-1.5); return toF(abs(x))}
    function fz() {var x = toF(2.25); return toF(sqrt(x))}
    function fA() {var x = toF(-1.); return toF(sqrt(x))}
    function fB() {var x = toF(0.); x = toF(inf); return toF(sqrt(x))}
    function fC(x) { x = toF(x); f32[0] = x; return toF(sqrt(f32[0])) }
    function fD(x) {x = toF(x); return toF(floor(x))}
    function fE(x) { x = toF(x); f32[0] = x; return toF(floor(f32[0])) }
    function fF(x) {x = toF(x); return toF(ceil(x))}
    function fG(x) { x = toF(x); f32[0] = x; return toF(ceil(f32[0])) }
    function fH(x) { x = toF(x); return toF(+cos(+x)) }
    function fI(x) {x = toF(x); return +pow(+x, 2.)}
    function fJ(x) {x = toF(x); return toF(+pow(+x, 2.))}
    function fK() {var x=toF(4.); var y=toF(0.); var z = 0.; y = toF(g1(x)); z = +toF(g1(x)); return toF(z); }
        function g1(x){x=toF(x); return toF(sqrt(x));} 
    function fL(x) {x = toF(x); return toF(+ffi(+x));}
    function fM(x) {x = toF(x); return toF(+ffi2(+x, 1., 2., 3., 4., 5., -5., -4., 1.));}
    function fN(x) {x = toF(x); return toF(g2(x));}
        function g2(x){x=toF(x);return toF(+x + 1.);}
    
    function fO(x) {x = toF(x); return toF(g3(x, toF(1.)));}
        function g3(x,y){x=toF(x);y=toF(y);return toF(+x + +y);}
    function fP(x) { x = +x; toF(s1(x)); return g|0}
        function s1(x) { x = +x; g = (g + ~~x)|0; return toF(g|0);}
    function fQ(x) { x = toF(x); return (toF(s2(x)), g)|0}
        function s2(x) { x = toF(x); g = (g + ~~x)|0; return toF(g|0);}
    function fR(x) { x = toF(x); return toF(+s3(x))}
        function s3(x) { x = toF(x); g = (g + ~~x)|0; return +(g|0);} 
    function fS(x) { x = toF(x); return toF(s4(x)|0)}
        function s4(x) { x = toF(x); g = (g + ~~x)|0; return g|0;}
    function fT(x) { x = toF(x); return toF(toF(s5(x)))}
        function s5(x) { x = toF(x); g = (g + ~~x)|0; return toF(g|0);}
    function fU(x, n) {x=toF(x);n=n|0;return toF(t[n&1](x));}
        function h1(x){x=toF(x);return toF(+x + .5);}
        function h2(x){x=toF(x);return toF(+x - .5);}
    function fV() { return toF(toF(3.) * toF(4.)); }
    function fW() { f32[0] = 4.; return toF(toF(3.) * f32[0]);}
    function fX() { return toF(toF(3.5) - toF(4.)); }
    function fY() { return toF(toF(toF(3.5) - toF(4.)) - toF(4.5)); }
    function fZ() { return toF(toF(toF(3.5) + toF(4.)) - toF(4.5)); }
    function f10() { return toF(toF(toF(3.5) + toF(4.)) - toF(4.5)); }
    function f11() { return toF(toF(toF(3.5) - toF(4.)) + toF(4.5)); }
    function f12() { f32[0] = 4.; return toF(toF(3) - f32[0]);}
    function f13() { return toF(toF(12.) / toF(4.)); }
    function f14() { f32[0] = 2.; return toF(toF(4) / f32[0]);}
    
    var t=[h1,h2];
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
        f0:f0,
        fa:fa,
        fb:fb,
        fc:fc,
        fd:fd,
        fe:fe,
        ff:ff,
        fg:fg,
        fh:fh,
        fi:fi,
        fj:fj,
        fk:fk,
        fl:fl,
        fm:fm,
        fn:fn,
        fo:fo,
        fq:fq,
        fr:fr,
        fs:fs,
        ft:ft,
        fu:fu,
        fv:fv,
        fw:fw,
        fx:fx,
        fy:fy,
        fz:fz,
        fA:fA,
        fB:fB,
        fC:fC,
        fD:fD,
        fE:fE,
        fF:fF,
        fG:fG,
        fH:fH,
        fI:fI,
        fJ:fJ,
        fK:fK,
        fL:fL,
        fM:fM,
        fN:fN,
        fO:fO,
        fP:fP,
        fQ:fQ,
        fR:fR,
        fS:fS,
        fT:fT,
        fU:fU,
        fV:fV,
        fW:fW,
        fX:fX,
        fY:fY,
        fZ:fZ,
        f10:f10,
        f11:f11,
        f12:f12,
        f13:f13,
        f14:f14,
    }
}
var global = {Math:Math,Int8Array:Int8Array,Int16Array:Int16Array,Int32Array:Int32Array,Uint8Array:Uint8Array,Uint16Array:Uint16Array,Uint32Array:Uint32Array,Float32Array:Float32Array,Float64Array:Float64Array,Infinity:Infinity, NaN:NaN}
var env = {x:3,y:3.5,ffi: function(x) { return x+1; }, ffi2: function(a,b,c,d,e,f,g,h,i) { return a+b+c+d+e+f+g+h+i; }}
var heap = new ArrayBuffer(1<<20);
var asmModule = AsmModule(global, env, heap);
print(asmModule.f1());
print(asmModule.f2());
print(asmModule.f3());
print(asmModule.f4());
print(asmModule.f5());
print(asmModule.f6());
print(asmModule.f7());
print(asmModule.f8());
print(asmModule.f9());
print(asmModule.f0());
print(asmModule.fa());
print(asmModule.fb());
print(asmModule.fc());
print(asmModule.fd());
print(asmModule.fe());
print(asmModule.ff());
print(asmModule.fg());
print(asmModule.fh());
print(asmModule.fi());
print(asmModule.fj(16.64));
print(asmModule.fk(16.64));
print(asmModule.fl(16.64));
print(asmModule.fm(16.64));
print(asmModule.fn(16.64));
print(asmModule.fo(16.64));
print(asmModule.fq(16.64));
print(asmModule.fr(16.64));
print(asmModule.fs(16.64));
print(asmModule.ft(16.64));
print(asmModule.fu(16.64));
print(asmModule.fv());
print(asmModule.fw());
print(asmModule.fx());
print(asmModule.fy());
print(asmModule.fz());
print(asmModule.fA());
print(asmModule.fB());
print(asmModule.fC(64));
for (v of [-10.5, -1.2345, -1, 0, 1, 3.141592653, 13.37, Math.Infinity, NaN]) {
    print(asmModule.fD(v));
}
print(asmModule.fE(13.37));
for (v of [-10.5, -1.2345, -1, 0, 1, 3.141592653, 13.37, Math.Infinity, NaN]) {
    print(asmModule.fF(v));
}
print(asmModule.fG(13.37));
print(asmModule.fH(3.14159265358));
print(asmModule.fI(3));
print(asmModule.fJ(3));
print(asmModule.fK());
print(asmModule.fL(5));
print(asmModule.fM(5));
print(asmModule.fN(5));
print(asmModule.fO(5));
print(asmModule.fP(3));
print(asmModule.fQ(3));
print(asmModule.fR(3));
print(asmModule.fS(3));
print(asmModule.fT(3));
print(asmModule.fU(0, 0));
print(asmModule.fU(0, 1));
print(asmModule.fU(13.37, 0));
print(asmModule.fU(13.37, 1));
print(asmModule.fV());
print(asmModule.fW());
print(asmModule.fX());
print(asmModule.fY());
print(asmModule.fZ());
print(asmModule.f10());
print(asmModule.f11());
print(asmModule.f12());
print(asmModule.f13());
print(asmModule.f14());

// --> test pressure on registers in internal calls when there are |numArgs| arguments
for (numArgs of [5, 9, 17/* fix bug where max args is now fixed at 32 , 33, 65, 129*/]) {
    let code = (function(n) {
        let args = "", coercions = "", sum = "", call="x";
        for (let i = 0; i < n; i++) {
            let name = 'a' + i;
            args += name + ((i == n-1)?'':',');
            coercions += name + '=toF(' + name + ');';
            sum += ((i>0)?'+':'') + ' +' + name;
            call += (i==0)?'':',toF(' + i + '.)'
        }
        return "function g(" + args + "){" + coercions + "return toF(" + sum + ");}"
               +"function f(x) { x = toF(x); return toF(g(" + call + "))}";
    })(numArgs);
    eval("print((function AsmModule(glob) {'use asm'; var toF = glob.Math.fround;" + code + "return f}(global))(5))");
}

// Comparisons
for (op of ['==', '!=', '<', '>', '<=', '>=']) {
    let code = "(function AsmModule(glob) {'use asm';  var toF = glob.Math.fround; function f(x) { x = toF(x); if( x " + op + " toF(3.) ) return 1; else return 0; return -1; } return f}(global))";
    let ternary = "(function AsmModule(glob) {'use asm';  var toF = glob.Math.fround; function f(x) { x = toF(x); return ((x " + op + " toF(3.)) ? 1 : 0)|0 } return f}(global))";
    for (v of [-5, 0, 2.5, 3, 13.37, NaN, Infinity]) {
        eval("print(" + code + "(" + v + "));");
        eval("print(" + ternary + "(" + v + "));");
    }
}