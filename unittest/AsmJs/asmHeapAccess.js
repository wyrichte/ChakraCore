//
// Test cases below use code modified from Mozilla's asm.js test suite
//

function AsmModule(glob, imp, b) {
    "use asm";
    
    var id=imp.id;
    const i1=4294967295;
    const i2=-1; 
    const i3=0x80000000;
    const i4=-2147483648;
    const i5=4294967295; 
    const i6=-1; 
    const i7=0x80000000; 
    const c8=-2147483648; 
    const i12=-2147483648;
    const i13=4294967295;     
    const i14=-1; 
    const i15=0x80000000;
    const c16=-2147483648; 
    const i17=4294967295; 
    const i18=-1; 
    const i19=0x80000000; 
    const i20=-2147483648; 
    var i25=0; 
    var i26=0; 
    
    const i8=new glob.Int8Array(b);
    var u8=new glob.Uint8Array(b);
    const i16=new glob.Int16Array(b);
    var u16=new glob.Uint16Array(b);
    const i32=new glob.Int32Array(b);
    var u32=new glob.Uint32Array(b);
    const f32=new glob.Float32Array(b);
    var f64=new glob.Float64Array(b);
    
    var toFloat32 = glob.Math.fround; 
    
    function f01() { return i32[0]|0 }
    function f02(i) {i=i|0; i = i32[i>>2]|0; return i|0}
    function f03(i) {i=i|0; i32[0] = i; return i8[0]|0};
    function f04(i) {i=i|0; i32[0] = i; return u8[0]|0};
    function f05(i) {i=i|0; i32[0] = i; return i16[0]|0};
    function f06(i) {i=i|0; i32[0] = i; return u16[0]|0};
    function f07(i) {i=i|0; i32[0] = i; return i32[0]|0};
    function f08(i) {i=i|0; i32[0] = i; return u32[0]|0};
    function f09(i) {i=i|0; i32[0] = i; return i8[0]|0};
    function f10(i) {i=i|0; i32[0] = i; return u8[0]|0};
    function f11(i) {i=i|0; i8[0] = i; return i8[0]|0};
    function f12(i) {i=i|0; i8[0] = i; return u8[0]|0};
    function f13(i,j) {i=i|0;j=+j; f64[i>>3] = j; return (~~+f64[i>>3])|0};
    function f14(i,j) {i=i|0;j=+j; f64[i>>3] = j; return (~~f64[i>>3])|0};
    function f15(i,j) {i=i|0;j=+j; f64[i>>3] = j; return +f64[i>>3]};
    function f16() { i32[1] = i32[0] };
    function f17() { f64[1] = f64[0] };
    function f18(i) {i=i|0; return i32[((i<<2)+1)>>2]|0 };
    function f19(i) {i=i|0; return i32[((i<<2)+2)>>2]|0 };
    function f20(i) {i=i|0; return i32[(i<<1)>>2]|0 };
    function f21(i) {i=i|0; return i32[(i<<2)>>2]|0 };
    function f22(i) {i=i|0; return i32[((i<<2)+4)>>2]|0 };
    function f23() { u8[7&0xffff] = 41 }
    function f24() { i8[7&0xffff] = -41 }
    function f25() { u16[(6&0xffff)>>1] = 0xabc }
    function f26() { i16[(6&0xffff)>>1] = -0xabc }
    function f27() { u32[(4&0xffff)>>2] = 0xabcde }
    function f28() { i32[(4&0xffff)>>2] = -0xabcde } 
    function f29() { f32[(4&0xffff)>>2] = 1.0 } 
    function f30() { f64[(8&0xffff)>>3] = 1.3 }
    function f31() { return +f32[(4&0xffff)>>2] }
    function fn32() { return toFloat32(f32[(4&0xffff)>>2]) }
    function f33() { return +f64[(8&0xffff)>>3] }
    function f34(i) { i=i|0; u32[64] }
    function f35(i) { i=i|0; u32[12] = i }
    function f36() { return u32[12]|0 }
    function f37() { u8[1] = -1 }
    function f38() { u8[4095] = -1 }
    function f39() { u8[4096] = -1 }
    function f40() { u8[258048] = -1 }
    function f41() { i8[1] = -1 }
    function f42() { i8[4095] = -1 }
    function f43() { i8[4096] = -1 }
    function f44() { i8[258048] = -1 }
    function f45() { u16[1] = -1 }
    function f46() { u16[2047] = -1 }
    function f47() { u16[2048] = -1 }
    function f48() { u16[126976] = -1 }
    function f49() { i16[1] = -1 }
    function f50() { i16[2047] = -1 }
    function f51() { i16[2048] = -1 }
    function f52() { i16[126976] = -1 }
    function f53() { u32[1] = -1 }
    function f54() { u32[1023] = -1 }
    function f55() { u32[1024] = -1 }
    function f56() { u32[61440] = -1 }
    function f57() { i32[1] = -1 }
    function f58() { i32[1023] = -1 }
    function f59() { i32[1024] = -1 }
    function f60() { i32[61440] = -1 }
    function f61() { f32[1] = -1.0 }
    function f62() { f32[1023] = -1.0 }
    function f63() { f32[1024] = -1.0 }
    function fn64() { f32[61440] = -1.0 }
    function f65() { f64[1] = -1.0 }
    function f66() { f64[511] = -1.0 }
    function f67() { f64[512] = -1.0 }
    function f68() { f64[28672] = -1.0 }
    
    function f69() { return u8[1]|0; }
    function f70() { return i8[1]|0; }
    function f71() { return u8[126976]|0; }
    function f72() { return i8[126976]|0; }
    function f73() { return u16[1]|0; }
    function f74() { return i16[1]|0; }
    function f75() { return u16[126976]|0; }
    function f76() { return i16[126976]|0; }
    
    function f77() { return u32[1]|0; }
    function f78() { return i32[1]|0; }
    function f79() { return i32[61440]|0; }
    
    function f80() { return +f32[1]; }
    function f81() { return +f32[1023]; }
    function f82() { return +f32[1024]; }
    function f83() { return +f32[61440]; }
    
    function f84() { return +f64[1]; } 
    function f85() { return +f64[511]; }
    function f86() { return +f64[512]; } 
    function f87() { return +f64[28672]; }
    
    function f88() { return u8[8191&8191]|0; }
    function f89() { return u8[(8191&8191)>>0]|0; }
    function f90() { u8[8192&8191] = -1; u8[0] = 0; return u8[8192&8191]|0; }
    function f91() { u8[(8192&8191)>>0] = -1; u8[0] = 0; return u8[(8192&8191)>>0]|0; }
    function f92() { return i8[8191&8191]|0; }
    function f93() { return i8[(8191&8191)>>0]|0; }
    function f94() { i8[8192&8191] = -1; i8[0] = 0; return i8[8192&8191]|0; }
    function f95() { i8[(8192&8191)>>0] = -1; i8[0] = 0; return i8[(8192&8191)>>0]|0; }
    
    function f96() { return u16[(8190&8191)>>1]|0; }
    function f97() { return u16[(8191&8191)>>1]|0; }
    function f98() { u16[(8192&8191)>>1] = -1; u16[0] = 0; return u16[(8192&8191)>>1]|0; }
    function f99() { return i16[(8190&8191)>>1]|0; }
    function f100() { return i16[(8191&8191)>>1]|0; }
    function f101() { i16[(8192&8191)>>1] = -1; i16[0] = 0; return i16[(8192&8191)>>1]|0; }
    
    function f102() { return u32[(8188&8191)>>2]|0; }
    function f103() { return u32[(8191&8191)>>2]|0; }
    function f104() { u32[(8192&8191)>>2] = -1; u32[0] = 0; return u32[(8192&8191)>>2]|0; }
    function f105() { return i32[(8188&8191)>>2]|0; }
    function f106() { return i32[(8191&8191)>>2]|0; }
    function f107() { i32[(8192&8191)>>2] = -1; i32[0] = 0; return i32[(8192&8191)>>2]|0; } 
    
    function f108() { return +f32[(8188&8191)>>2]; }
    function f109() { return +f32[(8191&8191)>>2]; }
    function f110() { f32[(8192&8191)>>2] = -1.0; f32[0] = 0.0; return +f32[(8192&8191)>>2]; }
    function f111() { return +f64[(8184&8191)>>3]; }
    function f112() { return +f64[(8191&8191)>>3]; }
    function f113() { f64[(8192&8191)>>3] = -1.0; f64[0] = 0.0; return +f64[(8192&8191)>>3]; }
    
    function f114() { return i32[(0&0)>>2]|0; }
    function f115() { return i32[(4&0)>>2]|0; }
    function f116() { return i32[((h1()|0)&0)>>2]|0; }
        function h1() { i32[0] = 1; return 8; };
    
    function f117(){f64[0]=+id(2.0);return +f64[0];}
    
    function f118() { return u32[(0x5A&4294967295)>>2]|0; }
    function f119() { return u32[(0x5A&i1)>>2]|0; }
    function f120() { return u32[(0x5A&-1)>>2]|0; }
    function f121() { return u32[(0x5A&i2)>>2]|0; }
    function f122() { return u32[(0x5A&0x80000000)>>2]|0; }
    function f123() { return u32[(0x5A&i3)>>2]|0; }
    function f124() { return u32[(0x5A&-2147483648)>>2]|0; }
    function f125() { return u32[(0x5A&i4)>>2]|0; }
    
    function f126() { return u32[(4294967295&0x5A)>>2]|0; }
    function f127() { return u32[(i5&0x5A)>>2]|0; } 
    function f128() { return u32[(-1&0x5A)>>2]|0; }
    function f129() { return u32[(i6&0x5A)>>2]|0; }
    function f130() { return u32[(0x80000000&0x5A)>>2]|0; }
    function f131() { return u32[(i7&0x5A)>>2]|0; } 
    function f132() { return u32[(-2147483648&0x5A)>>2]|0; }
    function f133() { return u32[(-2147483648&0x5A)>>2]|0; }
    
    function f142() { return u8[0x5A&4294967295]|0; }
    function f143() { return u8[0x5A&i13]|0; }
    function f144() { return u8[0x5A&-1]|0; }
    function f145() { return u8[0x5A&i14]|0; } 
    function f146() { return u8[0x5A&0x80000000]|0; }
    function f147() { return u8[0x5A&i15]|0; }
    function f148() { return u8[0x5A&-2147483648]|0; }
    function f149() { return u8[0x5A&c16]|0; }
    
    function f150() { return u8[4294967295&0x5A]|0; }
    function f151() { return u8[i17&0x5A]|0; }
    function f152() { return u8[-1&0x5A]|0; }
    function f153() { return u8[i18&0x5A]|0; }
    function f154() { return u8[0x80000000&0x5A]|0; } 
    function f155() { return u8[i19&0x5A]|0; }
    function f156() { return u8[-2147483648&0x5A]|0; }
    function f157() { return u8[i20&0x5A]|0; }
    
    function f166() { var x = 0, y = 0; u8[0] = 1; u8[1] = 2; x = 0|u8[i25]; i25 = x; y = 0|u8[i25]; return y|0;}
    function f167() { var x = 0, y = 0; u8[0] = 1; u8[1] = 2; x = 0|u8[i26]; y = 0|u8[i26]; return (x+y)|0;} 
    
    
    return {
        f01:f01,
        f02:f02,
        f03:f03,
        f04:f04,
        f05:f05,
        f06:f06,
        f07:f07,
        f08:f08,
        f09:f09,
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
        f56:f56,
        f57:f57,
        f58:f58,
        f59:f59,
        f60:f60,
        f61:f61,
        f62:f62,
        f63:f63,
        fn64:fn64,
        f65:f65,
        f66:f66,
        f67:f67,
        f68:f68,
        f69:f69,
        f70:f70,
        f71:f71,
        f72:f72,
        f73:f73,
        f74:f74,
        f75:f75,
        f76:f76,
        f77:f77,
        f78:f78,
        f79:f79,
        f80:f80,
        f81:f81,
        f82:f82,
        f83:f83,
        f84:f84,
        f85:f85,
        f86:f86,
        f87:f87,
        f88:f88,
        f89:f89,
        f90:f90,
        f91:f91,
        f92:f92,
        f93:f93,
        f94:f94,
        f95:f95,
        f96:f96,
        f97:f97,
        f98:f98,
        f99:f99,
        f100:f100,
        f101:f101,
        f102:f102,
        f103:f103,
        f104:f104,
        f105:f105,
        f106:f106,
        f107:f107,
        f108:f108,
        f109:f109,
        f110:f110,
        f111:f111,
        f112:f112,
        f113:f113,
        f114:f114,
        f115:f115,
        f116:f116,
        f117:f117,
        f118:f118,
        f119:f119,
        f120:f120,
        f121:f121,
        f122:f122,
        f123:f123,
        f124:f124,
        f125:f125,
        f126:f126,
        f127:f127,
        f128:f128,
        f129:f129,
        f130:f130,
        f131:f131,
        f132:f132,
        f133:f133,
        f142:f142,
        f143:f143,
        f144:f144,
        f145:f145,
        f146:f146,
        f147:f147,
        f148:f148,
        f149:f149,
        f150:f150,
        f151:f151,
        f152:f152,
        f153:f153,
        f154:f154,
        f155:f155,
        f156:f156,
        f157:f157,
        f166:f166,
        f167:f167,
    }
}
var global = {Math:Math,Int8Array:Int8Array,Int16Array:Int16Array,Int32Array:Int32Array,Uint8Array:Uint8Array,Uint16Array:Uint16Array,Uint32Array:Uint32Array,Float32Array:Float32Array,Float64Array:Float64Array,Infinity:Infinity, NaN:NaN}
var env = {id: function(x){return x;}}
var heap = new ArrayBuffer(1<<20);
var asmModule = AsmModule(global, env, heap);

print(asmModule.f01());
print(asmModule.f02());
print(asmModule.f03(0));
print(asmModule.f03(0xf7));
print(asmModule.f03(0xff));
print(asmModule.f03(0x100));
print(asmModule.f04(0));
print(asmModule.f04(0xf7));
print(asmModule.f04(0xff));
print(asmModule.f04(0x100));
print(asmModule.f05(0));
print(asmModule.f05(0x7fff));
print(asmModule.f05(0xffff));
print(asmModule.f05(0x10000));
print(asmModule.f06(0));
print(asmModule.f06(0x7fff));
print(asmModule.f06(0xffff));
print(asmModule.f06(0x10000));
print(asmModule.f07(0));
print(asmModule.f07(0x7fffffff));
print(asmModule.f07(0xffffffff));
print(asmModule.f07(0x100000000));
print(asmModule.f08(0));
print(asmModule.f08(0x7fffffff));
print(asmModule.f08(0xffffffff));
print(asmModule.f08(0x100000000));
print(asmModule.f09(0));
print(asmModule.f09(0xf7));
print(asmModule.f09(0xff));
print(asmModule.f09(0x100));
print(asmModule.f10(0));
print(asmModule.f10(0xf7));
print(asmModule.f10(0xff));
print(asmModule.f10(0x100));
print(asmModule.f11(0));
print(asmModule.f11(0xf7));
print(asmModule.f11(0xff));
print(asmModule.f11(0x100));
print(asmModule.f12(0));
print(asmModule.f12(0xf7));
print(asmModule.f12(0xff));
print(asmModule.f12(0x100));
print(asmModule.f13(0, 1.3));
print(asmModule.f13(4088, 2.5));
print(asmModule.f13(4096, 3.8));
print(asmModule.f14(0, 1.3));
print(asmModule.f14(4088, 2.5));
print(asmModule.f14(4096, 3.8));
print(asmModule.f15(0, 1.3));
print(asmModule.f15(4088, 2.5));
print(asmModule.f15(4096, 3.8));
var i32=new Int32Array(heap);
i32[0] = 42;
asmModule.f16();
print(i32[1]);
var f64=new Float64Array(heap);
f64[0] = 52;
asmModule.f17();
print(f64[1]);
i32[0] = 13;
i32[1] = 0xfffeeee;
print(asmModule.f18(0));
print(asmModule.f18(1));
print(asmModule.f19(1));
print(asmModule.f19(1));
print(asmModule.f20(0));
print(asmModule.f20(1));
print(asmModule.f20(2));
print(asmModule.f20(3));
print(asmModule.f21(0));
print(asmModule.f21(1));
print(asmModule.f22(0));
asmModule.f23();
print(new Uint8Array(heap)[7]);
asmModule.f24();
print(new Int8Array(heap)[7]);
asmModule.f25();
print(new Uint16Array(heap)[3]);
asmModule.f26();
print(new Int16Array(heap)[3]);
asmModule.f27();
print(new Uint32Array(heap)[1]);
asmModule.f28();
print(new Int32Array(heap)[1]);
asmModule.f29();
print(new Float32Array(heap)[1]);
asmModule.f30();
print(new Float64Array(heap)[1]);
new Float32Array(heap)[1] = 1.0;
print(asmModule.f31());
print(asmModule.fn32());
new Float64Array(heap)[1] = 1.3;
print(asmModule.f33());
asmModule.f34(11);
print(new Int32Array(heap)[12]);
print(asmModule.f35());
new Float64Array(heap)[0] = 3.5;
print(asmModule.f36());

new Uint8Array(heap)[0] = 0;
new Uint8Array(heap)[1] = 0;
new Uint8Array(heap)[2] = 0;
asmModule.f37();
print(new Uint8Array(heap)[0]);
print(new Uint8Array(heap)[1]);
print(new Uint8Array(heap)[2]);
new Uint8Array(heap)[4094] = 0;
new Uint8Array(heap)[4095] = 0;
new Uint8Array(heap)[4096] = 0;
asmModule.f38();
print(new Uint8Array(heap)[4094]);
print(new Uint8Array(heap)[4095]);
print(new Uint8Array(heap)[4096]);
new Uint8Array(heap)[4095] = 0;
new Uint8Array(heap)[4096] = 0;
new Uint8Array(heap)[4097] = 0;
asmModule.f39();
print(new Uint8Array(heap)[4095]);
print(new Uint8Array(heap)[4096]);
print(new Uint8Array(heap)[4097]);
new Uint8Array(heap)[258047] = 0;
new Uint8Array(heap)[258048] = 0;
new Uint8Array(heap)[258049] = 0;
asmModule.f40();
print(new Uint8Array(heap)[258047]);
print(new Uint8Array(heap)[258048]);
print(new Uint8Array(heap)[258049]);


new Int8Array(heap)[0] = 0;
new Int8Array(heap)[1] = 0;
new Int8Array(heap)[2] = 0;
asmModule.f41();
print(new Int8Array(heap)[0]);
print(new Int8Array(heap)[1]);
print(new Int8Array(heap)[2]);
new Int8Array(heap)[4094] = 0;
new Int8Array(heap)[4095] = 0;
new Int8Array(heap)[4096] = 0;
asmModule.f42();
print(new Int8Array(heap)[4094]);
print(new Int8Array(heap)[4095]);
print(new Int8Array(heap)[4096]);
new Int8Array(heap)[4095] = 0;
new Int8Array(heap)[4096] = 0;
new Int8Array(heap)[4097] = 0;
asmModule.f43();
print(new Int8Array(heap)[4095]);
print(new Int8Array(heap)[4096]);
print(new Int8Array(heap)[4097]);
new Int8Array(heap)[258047] = 0;
new Int8Array(heap)[258048] = 0;
new Int8Array(heap)[258049] = 0;
asmModule.f44();
print(new Int8Array(heap)[258047]);
print(new Int8Array(heap)[258048]);
print(new Int8Array(heap)[258049]);

new Uint16Array(heap)[0] = 0;
new Uint16Array(heap)[1] = 0;
new Uint16Array(heap)[2] = 0;
asmModule.f45();
print(new Uint16Array(heap)[0]);
print(new Uint16Array(heap)[1]);
print(new Uint16Array(heap)[2]);
new Uint16Array(heap)[2046] = 0;
new Uint16Array(heap)[2047] = 0;
new Uint16Array(heap)[2048] = 0;
asmModule.f46();
print(new Uint16Array(heap)[2046]);
print(new Uint16Array(heap)[2047]);
print(new Uint16Array(heap)[2048]);
new Uint16Array(heap)[2047] = 0;
new Uint16Array(heap)[2048] = 0;
new Uint16Array(heap)[2049] = 0;
asmModule.f47();
print(new Uint16Array(heap)[2047]);
print(new Uint16Array(heap)[2048]);
print(new Uint16Array(heap)[2049]);
new Uint16Array(heap)[126975] = 0;
new Uint16Array(heap)[126976] = 0;
new Uint16Array(heap)[126977] = 0;
asmModule.f48();
print(new Uint16Array(heap)[126975]);
print(new Uint16Array(heap)[126976]);
print(new Uint16Array(heap)[126977]);

new Int16Array(heap)[0] = 0;
new Int16Array(heap)[1] = 0;
new Int16Array(heap)[2] = 0;
asmModule.f49();
print(new Int16Array(heap)[0]);
print(new Int16Array(heap)[1]);
print(new Int16Array(heap)[2]);
new Int16Array(heap)[2046] = 0;
new Int16Array(heap)[2047] = 0;
new Int16Array(heap)[2048] = 0;
asmModule.f50();
print(new Int16Array(heap)[2046]);
print(new Int16Array(heap)[2047]);
print(new Int16Array(heap)[2048]);
new Int16Array(heap)[2047] = 0;
new Int16Array(heap)[2048] = 0;
new Int16Array(heap)[2049] = 0;
asmModule.f51();
print(new Int16Array(heap)[2047]);
print(new Int16Array(heap)[2048]);
print(new Int16Array(heap)[2049]);
new Int16Array(heap)[126975] = 0;
new Int16Array(heap)[126976] = 0;
new Int16Array(heap)[126977] = 0;
asmModule.f52();
print(new Int16Array(heap)[126975]);
print(new Int16Array(heap)[126976]);
print(new Int16Array(heap)[126977]);

new Uint32Array(heap)[0] = 0;
new Uint32Array(heap)[1] = 0;
new Uint32Array(heap)[2] = 0;
asmModule.f53();
print(new Uint32Array(heap)[0]);
print(new Uint32Array(heap)[1]);
print(new Uint32Array(heap)[2]);
new Uint32Array(heap)[1022] = 0;
new Uint32Array(heap)[1023] = 0;
new Uint32Array(heap)[1024] = 0;
asmModule.f54();
print(new Uint32Array(heap)[1022]);
print(new Uint32Array(heap)[1023]);
print(new Uint32Array(heap)[1024]);
new Uint32Array(heap)[1023] = 0;
new Uint32Array(heap)[1024] = 0;
new Uint32Array(heap)[1025] = 0;
asmModule.f55();
print(new Uint32Array(heap)[1023]);
print(new Uint32Array(heap)[1024]);
print(new Uint32Array(heap)[1025]);
new Uint32Array(heap)[61439] = 0;
new Uint32Array(heap)[61440] = 0;
new Uint32Array(heap)[61441] = 0;
asmModule.f56();
print(new Uint32Array(heap)[61439]);
print(new Uint32Array(heap)[61440]);
print(new Uint32Array(heap)[61441]);

new Int32Array(heap)[0] = 0;
new Int32Array(heap)[1] = 0;
new Int32Array(heap)[2] = 0;
asmModule.f57();
print(new Int32Array(heap)[0]);
print(new Int32Array(heap)[1]);
print(new Int32Array(heap)[2]);
new Int32Array(heap)[1022] = 0;
new Int32Array(heap)[1023] = 0;
new Int32Array(heap)[1024] = 0;
asmModule.f58();
print(new Int32Array(heap)[1022]);
print(new Int32Array(heap)[1023]);
print(new Int32Array(heap)[1024]);
new Int32Array(heap)[1023] = 0;
new Int32Array(heap)[1024] = 0;
new Int32Array(heap)[1025] = 0;
asmModule.f59();
print(new Int32Array(heap)[1023]);
print(new Int32Array(heap)[1024]);
print(new Int32Array(heap)[1025]);
new Int32Array(heap)[61439] = 0;
new Int32Array(heap)[61440] = 0;
new Int32Array(heap)[61441] = 0;
asmModule.f60();
print(new Int32Array(heap)[61439]);
print(new Int32Array(heap)[61440]);
print(new Int32Array(heap)[61441]);

new Float32Array(heap)[0] = 0;
new Float32Array(heap)[1] = 0;
new Float32Array(heap)[2] = 0;
asmModule.f61();
print(new Float32Array(heap)[0]);
print(new Float32Array(heap)[1]);
print(new Float32Array(heap)[2]);
new Float32Array(heap)[1022] = 0;
new Float32Array(heap)[1023] = 0;
new Float32Array(heap)[1024] = 0;
asmModule.f62();
print(new Float32Array(heap)[1022]);
print(new Float32Array(heap)[1023]);
print(new Float32Array(heap)[1024]);
new Float32Array(heap)[1023] = 0;
new Float32Array(heap)[1024] = 0;
new Float32Array(heap)[1025] = 0;
asmModule.f63();
print(new Float32Array(heap)[1023]);
print(new Float32Array(heap)[1024]);
print(new Float32Array(heap)[1025]);
new Float32Array(heap)[61439] = 0;
new Float32Array(heap)[61440] = 0;
new Float32Array(heap)[61441] = 0;
asmModule.fn64();
print(new Float32Array(heap)[61439]);
print(new Float32Array(heap)[61440]);
print(new Float32Array(heap)[61441]);

new Float64Array(heap)[0] = 0;
new Float64Array(heap)[1] = 0;
new Float64Array(heap)[2] = 0;
asmModule.f65();
print(new Float64Array(heap)[0]);
print(new Float64Array(heap)[1]);
print(new Float64Array(heap)[2]);
new Float64Array(heap)[510] = 0;
new Float64Array(heap)[511] = 0;
new Float64Array(heap)[512] = 0;
asmModule.f66();
print(new Float64Array(heap)[510]);
print(new Float64Array(heap)[511]);
print(new Float64Array(heap)[512]);
new Float64Array(heap)[511] = 0;
new Float64Array(heap)[512] = 0;
new Float64Array(heap)[513] = 0;
asmModule.f67();
print(new Float64Array(heap)[511]);
print(new Float64Array(heap)[512]);
print(new Float64Array(heap)[513]);
new Float64Array(heap)[28671] = 0;
new Float64Array(heap)[28672] = 0;
new Float64Array(heap)[28673] = 0;
asmModule.f68();
print(new Float64Array(heap)[28671]);
print(new Float64Array(heap)[28672]);
print(new Float64Array(heap)[28673]);

new Uint8Array(heap)[1] = -1;
print(asmModule.f69());
new Int8Array(heap)[1] = -1;
print(asmModule.f70());
new Uint8Array(heap)[126976] = -1;
print(asmModule.f71());
new Int8Array(heap)[126976] = -1;
print(asmModule.f72());

new Uint16Array(heap)[1] = -1;
print(asmModule.f73());
new Int16Array(heap)[1] = -1;
print(asmModule.f74());
new Uint16Array(heap)[126976] = -1;
print(asmModule.f75());
new Int16Array(heap)[126976] = -1;
print(asmModule.f76());

new Uint32Array(heap)[1] = -1;
print(asmModule.f77());
new Int32Array(heap)[1] = -1;
print(asmModule.f78());
new Int32Array(heap)[61440] = -1;
print(asmModule.f79());

new Float32Array(heap)[1] = -1.0;
print(asmModule.f80());
new Float32Array(heap)[1023] = -1.0;
print(asmModule.f81());
new Float32Array(heap)[1024] = -1.0;
print(asmModule.f82());
new Float32Array(heap)[61440] = -1.0;
print(asmModule.f83());

new Float64Array(heap)[1] = -1.0;
print(asmModule.f84());
new Float64Array(heap)[511] = -1.0;
print(asmModule.f85());
new Float64Array(heap)[512] = -1.0;
print(asmModule.f86());
new Float64Array(heap)[28672] = -1.0;
print(asmModule.f87());

new Uint8Array(heap)[8191] = -1;
print(asmModule.f88());
print(asmModule.f89());
print(asmModule.f90());
print(asmModule.f91());
new Int8Array(heap)[8191] = -1;
print(asmModule.f92());
print(asmModule.f93());
print(asmModule.f94());
print(asmModule.f95());

new Uint16Array(heap)[4095] = -1;
print(asmModule.f96());
print(asmModule.f97());
print(asmModule.f98());
new Int16Array(heap)[4095] = -1;
print(asmModule.f99());
print(asmModule.f100());
print(asmModule.f101());

new Uint32Array(heap)[2047] = -1;
print(asmModule.f102());
print(asmModule.f103());
print(asmModule.f104());
new Int32Array(heap)[2047] = -1;
print(asmModule.f105());
print(asmModule.f106());
print(asmModule.f107());

new Float32Array(heap)[2047] = -1.0;
print(asmModule.f108());
print(asmModule.f109());
print(asmModule.f110());
new Float64Array(heap)[1023] = -1.0;
print(asmModule.f111());
print(asmModule.f112());
print(asmModule.f113());

new Int32Array(heap)[0] = 0x55aa5a5a;
print(asmModule.f114());
print(asmModule.f115());
print(asmModule.f116());
print(new Int32Array(heap)[0]);

print(asmModule.f117());

new Uint32Array(heap)[0] = 0xAA;
new Uint32Array(heap)[0x5A>>2] = 0xA5;
print(asmModule.f118());
print(asmModule.f119());
print(asmModule.f120());
print(asmModule.f121());
print(asmModule.f122());
print(asmModule.f123());
print(asmModule.f124());
print(asmModule.f125());
print(asmModule.f126());
print(asmModule.f127());
print(asmModule.f128());
print(asmModule.f129());
print(asmModule.f130());
print(asmModule.f131());
print(asmModule.f132());
print(asmModule.f133());

new Uint8Array(heap)[0] = 0xAA;
new Uint8Array(heap)[0x5A] = 0xA5;
print(asmModule.f142());
print(asmModule.f143());
print(asmModule.f144());
print(asmModule.f145());
print(asmModule.f146());
print(asmModule.f147());
print(asmModule.f148());
print(asmModule.f149());
print(asmModule.f150());
print(asmModule.f151());
print(asmModule.f152());
print(asmModule.f153());
print(asmModule.f154());
print(asmModule.f155());
print(asmModule.f156());
print(asmModule.f157());
print(asmModule.f166());
print(asmModule.f167());
 
print(function(glob,i,b){"use asm";var u32=new glob.Uint32Array(b); function f134() {return u32[4294967295>>2]|0; } return f134}(this, null, heap)());
print(function(glob,i,b){"use asm";const i9=4294967295; var u32=new glob.Uint32Array(b); function f135() { return u32[i9>>2]|0; } return f135}(this, null, heap)());
print(function(glob,i,b){"use asm";var u32=new glob.Uint32Array(b); function f136() { return u32[-1>>2]|0; } return f136}(this, null, heap)());
print(function(glob,i,b){"use asm";const i10=-1; var u32=new glob.Uint32Array(b); function f137() { return u32[i10>>2]|0; } return f137}(this, null, heap)());
print(function(glob,i,b){"use asm";var u32=new glob.Uint32Array(b); function f138() { return u32[0x80000000>>2]|0; } return f138}(this, null, heap)());
print(function(glob,i,b){"use asm";const i11=0x80000000; var u32=new glob.Uint32Array(b); function f139() { return u32[i11>>2]|0; } return f139}(this, null, heap)());
print(function(glob,i,b){"use asm";var u32=new glob.Uint32Array(b); function f140() { return u32[-2147483648>>2]|0; } return f140}(this, null, heap)());
print(function(glob,i,b){"use asm";var u8=new glob.Uint8Array(b); function f158() { return u8[4294967295>>0]|0; } return f158}(this, null, heap)());
print(function(glob,i,b){"use asm";const i21=4294967295;var u8=new glob.Uint8Array(b); function f159() { return u8[i21>>0]|0; } return f159}(this, null, heap)());
print(function(glob,i,b){"use asm";var u8=new glob.Uint8Array(b); function f160() { return u8[-1>>0]|0; } return f160}(this, null, heap)());
print(function(glob,i,b){"use asm";const i22=-1;var u8=new glob.Uint8Array(b); function f161() { return u8[i22>>0]|0; } return f161}(this, null, heap)());
print(function(glob,i,b){"use asm";var u8=new glob.Uint8Array(b); function f162() { return u8[0x80000000>>0]|0; } return f162}(this, null, heap)());
print(function(glob,i,b){"use asm";const i23=0x80000000;var u8=new glob.Uint8Array(b); function f163() { return u8[i23>>0]|0; }  return f163}(this, null, heap)());
print(function(glob,i,b){"use asm";var u8=new glob.Uint8Array(b); function f164() { return u8[-2147483648>>0]|0; } return f164}(this, null, heap)());
print(function(glob,i,b){"use asm";const i24=-2147483648;var u8=new glob.Uint8Array(b); function f165() { return u8[i24>>0]|0; } return f165}(this, null, heap)());