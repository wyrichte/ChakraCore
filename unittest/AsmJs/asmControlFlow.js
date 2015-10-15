//
// Test cases below use code modified from Mozilla's asm.js test suite
//

function AsmModule(glob, imp, heap) {
    "use asm";
    
    var x1=0;
    var ffi=imp.ffi1;
    var func=imp.func; 
    
    function f1(i,j) { i=i|0;j=+j; if (i) return +j; return +~~i }
    function f2(i,j) { i=i|0;j=j|0; if (i) return j^0; return i|0 }
    function f3(i) { i=i|0; if ((i|0) == 0) return 10; else if ((i|0) == 1) return 12; else if ((i|0) == 2) return 14; return 0}
    function f4(i) { i=i|0; if ((i|0) == 0) return 10; else if ((i|0) == 1) return 12; else if ((i|0) == 2) return 14; else return 16; return 0}
    function f5(i) { i=i|0; if ((i|0) == 0) i = 10; else if ((i|0) == 1) return 12; return (i|0) }
    function f6() { while (0) {} return 0} 
    function f7() { for (;0;) {} return 0}
    function f8() { do {} while(0); return 0}
    function f9() { while (0) ; return 0}
    function f10() { for (;0;) ; return 0}
    function f11() { do ; while(0); return 0}
    function f12(j) {j=j|0; var i=0; while ((i|0) < (j|0)) i=(i+4)|0; return i|0} 
    function f13(j) {j=j|0; var i=0; for (;(i|0) < (j|0);) i=(i+4)|0; return i|0}
    function f14(j) {j=j|0; var i=0; do { i=(i+4)|0; } while ((i|0) < (j|0)); return i|0}
    function f15() { while(1) return 42; return 0 }
    function f16() { for(;1;) return 42; return 0 }
    function f17() { do return 42; while(1); return 0 }
    function f18() { var i=0; while(1) { if (i) return 13; return 42 } return 0 }
    function f19() { var i=0; for(;1;) { if (i) return 13; return 42 } return 0 }
    function f20() { var i=0; do { if (i) return 13; return 42 } while(1); return 0 }
    function f21() { var i=0; while(1) { break; while(1) {} } return 42 }
    function f22() { var i=0; for(;;) { break; for(;;) {} } return 42 }
    function f23() { var i=0; do { break; do {} while(1) {} } while(1); return 42 }
    function f24() { var i=1; while(1) { if (i) return 42; return 13 } return 0 }
    function f25() { var i=1; for(;1;) { if (i) return 42; return 13 } return 0 }
    function f26() { var i=1; do { if (i) return 42; return 13 } while(1); return 0 }
    function f27() { var i=0; while(1) { if (i) return 13; else return 42; return 13 } return 0 }
    function f28() { var i=0; for(;;) { if (i) return 13; else return 42; return 13 } return 0 }
    function f29(i) { i=i|0; do { if (i) { break; } else { return i|0 } i = 1 } while (0); return i|0 }
    function f30() { var i=0; do { if (i) return 13; else return 42; return 13 } while(1); return 0 }
    function f31() { var i=0; while((i|0) < 3) { if (i) return 42; i=(i+1)|0 } return 0 }
    function f32() { var i=0; for(;(i|0) < 3;) { if (i) return 42; i=(i+1)|0 } return 0 }
    function f33() { var i=0; do { if (i) return 42; i=(i+1)|0 } while((i|0) < 3); return 0 }
    function f34() { var i=0; while((i|0) < 3) { if (!i) i=(i+1)|0; return 42 } return 0 }
    function f35() { var i=0; for(;(i|0) < 3;) { if (!i) i=(i+1)|0; return 42 } return 0 }
    function f36() { var i=0; do { if (!i) i=(i+1)|0; return 42 } while((i|0) < 3); return 0 }
    function f37() { var i=42; return i|0; while(1) {} return 0 }
    function f38() { var i=42; return i|0; for(;1;) {} return 0 }
    function f39() { var i=42; return i|0; do {} while(1); return 0 }
    function f40() { var i=0; while((i|0) < 10) if ((i|0) == 4) break; else i=(i+1)|0; return i|0 } 
    function f41() { var i=0; for(; (i|0) < 10;) if ((i|0) == 4) break; else i=(i+1)|0; return i|0 }
    function f42() { var i=0; do if ((i|0) == 4) break; else i=(i+1)|0; while((i|0) < 10); return i|0 }
    function f43() { var i=0,sum=0; while ((i=(i+1)|0)<2) { sum=(sum+1)|0; if ((i&1)==0) continue; sum=(sum+100)|0 } return sum|0 }
    function f44() { var i=0,sum=0; for (;(i=(i+1)|0)<2;) { sum=(sum+1)|0; if ((i&1)==0) continue; sum=(sum+100)|0 } return sum|0 }
    function f45() { var i=0,sum=0; do { sum=(sum+1)|0; if ((i&1)==0) continue; sum=(sum+100)|0 } while((i=(i+1)|0)<2); return sum|0 }
    function f46() { var i=0; x:a:y:while(1) { i=1; while(1) { i=2; break a; } i=3; } return i|0 }
    function f47() { var i=0; x:a:y:for(;;) { i=1; while(1) { i=2; break a; } i=3; } return i|0 } 
    function f48() { var i=0; x:a:y:do { i=1; while(1) { i=2; break a; } i=3; } while(1); return i|0 }
    function f49() { var i=0; a:b:while((i|0) < 5) { i=(i+1)|0; while(1) continue b; } return i|0 }
    function f50() { var i=0; a:b:for(;(i|0) < 5;) { i=(i+1)|0; while(1) continue b; } return i|0 }
    function f51() { var i=0; a:b:do { i=(i+1)|0; while(1) continue b; } while((i|0) < 5); return i|0 }
    function f52() { var i=0; return 0; a:b:while((i|0) < 5) { i=(i+1)|0; while(1) continue b; } return i|0 }
    function f53() { var i=0; return 0; a:b:for(;(i|0) < 5;) { i=(i+1)|0; while(1) continue b; } return i|0 }
    function f54() { var i=0; return 0; a:b:do { i=(i+1)|0; while(1) continue b; } while((i|0) < 5); return i|0 }
    //function f55() { var i=42; a:{ break a; i=2; } b:{ c:{ break b; i=3 } i=4 } return i|0 } block labels don't work
    function f56() { var i=0; a:b:for(;(i|0) < 5;i=(i+1)|0) { while(1) continue b; } return i|0 }
    function f57() { var i=42,sum=0; for(i=1;(i|0)<4;i=(i+1)|0) sum=(sum+i)|0; return sum|0 }
    function f58() { var i=42,sum=0; for(i=1;(i|0)<8;i=(i+1)|0) { if ((i&1) == 0) continue; sum=(sum+i)|0; } return sum|0 } 
    function f59() { var i=0; while(1) { i=(i+1)|0; if ((i|0) > 10) break; } return i|0 }
    function f60() { var i=0; for(;1;i=(i+1)|0) { if ((i|0) > 10) break; } return i|0 }
    function f61() { var i=0; do { if ((i|0) > 10) break; i=(i+1)|0 } while(1); return i|0 }
    function f62() { var i=0; while(1){ if ((i|0)>0) break; while (1) { i=i+1|0; if ((i|0)==1) break; } } return i|0; }
    function f63() { var i=0; for(;;){ if ((i|0)>0) break; while (1) { i=i+1|0; if ((i|0)==1) break; } } return i|0; }
    function f64() { var i=0; do{ if ((i|0)>0) break; while (1) { i=i+1|0; if ((i|0)==1) break; } }while(1); return i|0; }
    function f65() { var i=0,sum=0; for(;;){ if ((i|0)>5) break; while (1) { i=i+1|0; sum=(sum+i)|0; if ((i|0)>3) break; } } return sum|0; }
    function f66() { var i=0,sum=0; while(1){ if ((i|0)>5) break; while (1) { i=i+1|0; sum=(sum+i)|0; if ((i|0)>3) break; } } return sum|0; }
    function f67() { var i=0,sum=0; do{ if ((i|0)>5) break; while (1) { i=i+1|0; sum=(sum+i)|0; if ((i|0)>3) break; } }while(1); return sum|0; }
    function f68(i) { i=i|0; while(1) { if (i) { break; } else { return i|0 } i = 1 } return i|0 }
    function f69(i) { i=i|0; for(;1;) { if (i) { break; } else { return i|0 } i = 1 } return i|0 }
    function f70(i) { i=i|0; while(1) { if (i) { return i|0 } else { return i|0 } i = 1 } return i|0 }
    function f71(i) { i=i|0; for(;;) { if (i) { return i|0 } else { return i|0 } i = 1 } return i|0 }
    function f72(i) { i=i|0; do { if (i) { return i|0 } else { return i|0 } i = 1 } while (0); return i|0 }
    function f73() {var j=1,i=0; while(j){ if(0) continue; j=i } return j|0 }
    function f74() {var j=1,i=0; for(;j;){ if(0) continue; j=i } return j|0 }
    function f75() {var j=1,i=0; do{ if(0) continue; j=i } while(j) return j|0 }
    function f76(i) { i=i|0; for(;;) { return i|0 } return 0 }
    function f77(n) { n=n|0; var i=0,s=0; for(;;i=(i+1)|0) { if (~~i==~~n) return s|0; s=(s+i)|0 } return 0 }
    function f78(n,m) { n=n|0;m=m|0; var i=0,sum=0; while((n|0)>(m|0) ? ((i|0)<(n|0))|0 : ((i|0)<(m|0))|0) { sum = (sum+i)|0; i=(i+1)|0 } return sum|0 } 
    function f79(n,m) { n=n|0;m=m|0; var i=0,sum=0; for(; (n|0)>(m|0) ? ((i|0)<(n|0))|0 : ((i|0)<(m|0))|0; i=(i+1)|0) { sum = (sum+i)|0 } return sum|0 }
    function f80(n,m) { n=n|0;m=m|0; var i=0,sum=0; do { sum = (sum+i)|0; i=(i+1)|0 } while((n|0)>(m|0) ? ((i|0)<(n|0))|0 : ((i|0)<(m|0))|0); return sum|0 }
    function f81() { var i=0; switch(i|0) {}; return i|0 }
    function f82() { var i=0; switch(i|0) { default: i=42 } return i|0 }
    function f83() { var i=0; switch(i|0) { default: i=42; break } return i|0 } 
    function f84() { var i=0; switch(i|0) { case 0: i=42 } return i|0 }
    function f85() { var i=0; switch(i|0) { case 0: i=42; break } return i|0 }
    function f86() { var i=0; switch(i|0) { case 0: default: i=42 } return i|0 }
    function f87() { var i=0; switch(i|0) { case 0: default: i=42; break } return i|0 }
    function f88() { var i=1; switch(i|0) { case 0: case 2: break; default: i=42 } return i|0 }
    function f89() { var i=1; switch(i|0) { case 0: case 2: break; default: i=42; break } return i|0 }
    function f90() { return 42; switch(1) { case 1: return 13 } return 14 } 
    
    function f91() { return x1|0 }
    function f92(i) { i=i|0; x1=i }
    function f93(i) { i=i|0; if (i) f92(i); }
    
    function f94(i) { i=i|0; switch(i|0) { case 1: i=-1; break; case 133742: i=2; break; default: i=42; break } return i|0 }
    function f95(i) { i=i|0; switch(i|0) { case 1: i=42; break; default: i=13 } return i|0 }
    function f96(i) { i=i|0; switch(i|0) { case -1: i=42; break; default: i=13 } return i|0 }
    function f97(i) { i=i|0; var sum=0; switch(i|0) { case -1: sum=(sum+1)|0; case 1: sum=(sum+1)|0; case 3: sum=(sum+1)|0; default: sum=(sum+100)|0; } return sum|0 }
    function f98(i) { i=i|0; var sum=0; switch(i|0) { case -1: sum=10; break; case 1: sum=11; break; case 3: sum=12; break; default: sum=13; } return sum|0 } 
    
    function f99() { var i=8,sum=0; a:for(; (i|0)<20; i=(i+1)|0) { switch(i&3) { case 0:case 1:sum=(sum+i)|0;break;case 2:sum=(sum+100)|0;continue;default:break a} sum=(sum+10)|0; } sum=(sum+1000)|0; return sum|0 }
    function f100() { g:{ return 42 } return 13 }
    
    function f101() { var i=0; return (i+1)|0; return ffi(i|0)|0 }
    
    function f102() { return 0; if (1) return -1; return -2}
    function f103(x) { x=x|0; var a=2;if(x?1:0)a=1;else a=0; return a|0 } 
    
    function f104(x) { x=x|0; var a=2;if(x?1:1)a=1; else {func();a=0}return a|0 }
    function f105(x) { x=x|0; var a=2;if(x?0:0){a=1; func()}else a=0;return a|0 }
    
    function f106(x,y) { x=x|0;y=y|0; var a=2;if(x?0:y)a=1;else a=0; return a|0 }
    function f107(x,y) { x=x|0;y=y|0; var a=2;if(x?y:1)a=1;else a=0; return a|0 } 
    
    function f108(x,y,z) { x=x|0;y=y|0;z=z|0; var a=2;if(x?y:z)a=1;else a=0; return a|0 }
    
    function f109(x,y) { x=x|0;y=y|0; var z=0; if((x|0) > 2 ? (y|0) < 5 : 0) z=1; return z|0;}
    function f110(x,y) { x=x|0;y=y|0; var z=2; if((x|0) > 2 ? (y|0) < 5 : 0) z=1; else z=0; return z|0;}
    function f111(x,y,z) { x=x|0;y=y|0;z=z|0; var a=0; if((x|0) > 2 ? ((y|0) < 5 ? (z|0) > -1 : 0) : 0) a=1; return a|0;}
    function f112(x,y,z) { x=x|0;y=y|0;z=z|0; var a=2; if((x|0) > 2 ? ((y|0) < 5 ? (z|0) > -1 : 0) : 0) a=1; else a=0; return a|0;} 
    function f113(x,y,z) { x=x|0;y=y|0;z=z|0; var a=0; if((x|0) > 2 ? (y|0) < 5 : 0) {if ((z|0) > -1) a=1}; return a|0;}
    function f114(x,y,z) { x=x|0;y=y|0;z=z|0; var a=2; if((x|0) > 2 ? (y|0) < 5 : 0) {if ((z|0) > -1) a=1; else a=0;} else a=0; return a|0;} 
    function f115(x,y,z) { x=x|0;y=y|0;z=z|0; var a=0; if(((x|0) == 3 ? 1 : ((x|0) > 3)) ? ((y|0) < 5 ? (z|0) > -1 : 0) : 0) a=1; return a|0;}
    function f116(x,y,z) { x=x|0;y=y|0;z=z|0; var a=2; if(((x|0) == 3 ? 1 : ((x|0) > 3)) ? ((y|0) < 5 ? (z|0) > -1 : 0) : 0) a=1; else a=0; return a|0;} 
    function f117(x,y,z) { x=x|0;y=y|0;z=z|0; var a=0; if((x|0) == 3 ? 1 : (x|0) > 3) {if ((y|0) < 5 ? (z|0) > -1 : 0) a=1;} return a|0;}
    function f118(x,y,z) { x=x|0;y=y|0;z=z|0; var a=2; if((x|0) == 3 ? 1 : (x|0) > 3) {if ((y|0) < 5 ? (z|0) > -1 : 0) a=1; else a=0;} else a=0; return a|0;}
    
    function f119(x,y) { x=x|0;y=y|0; var z=1; if((x|0) <= 2 ? 1 : (y|0) >= 5) z=0; return z|0;}
    function f120(x,y) { x=x|0;y=y|0; var z=2; if((x|0) <= 2 ? 1 : (y|0) >= 5) z=0; else z=1; return z|0;}
    function f121(x,y,z) { x=x|0;y=y|0;z=z|0; var a=1; if((x|0) <= 2 ? 1 : ((y|0) >= 5 ? 1 : (z|0) <= -1)) a=0; return a|0;} 
    function f122(x,y,z) { x=x|0;y=y|0;z=z|0; var a=2; if((x|0) <= 2 ? 1 : ((y|0) >= 5 ? 1 : (z|0) <= -1)) a=0; else a=1; return a|0;} 
    function f123(x,y,z) { x=x|0;y=y|0;z=z|0; var a=1; if((x|0) <= 2 ? 1 : (y|0) >= 5) a=0; else if ((z|0) <= -1) a=0; return a|0;} 
    function f124(x,y,z) { x=x|0;y=y|0;z=z|0; var a=2; if((x|0) <= 2 ? 1 : (y|0) >= 5) a=0; else if ((z|0) <= -1) a=0; else a=1; return a|0;} 
    function f125(x,y,z) { x=x|0;y=y|0;z=z|0; var a=1; if(((x|0) != 3 ? ((x|0) <= 3) : 0) ? 1 : ((y|0) >= 5 ? 1 : (z|0) <= -1)) a=0; return a|0;}
    function f126(x,y,z) { x=x|0;y=y|0;z=z|0; var a=2; if(((x|0) != 3 ? ((x|0) <= 3) : 0) ? 1 : ((y|0) >= 5 ? 1 : (z|0) <= -1)) a=0; else a=1; return a|0;}
    
    
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
        //f55:f55,
        f56:f56,
        f57:f57,
        f58:f58,
        f59:f59,
        f60:f60,
        f61:f61,
        f62:f62,
        f63:f63,
        f64:f64,
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
    }
}

var guard = (function() {
    var called_ = false;
    return {
        called: function(){ return called_ },
        call: function(){ called_ = true }
    }
})();

var global = {Math:Math,Int8Array:Int8Array,Int16Array:Int16Array,Int32Array:Int32Array,Uint8Array:Uint8Array,Uint16Array:Uint16Array,Uint32Array:Uint32Array,Float32Array:Float32Array,Float64Array:Float64Array,Infinity:Infinity, NaN:NaN}
var env = {ffi1:function() { throw "Wrong" },func: guard.call}
var heap = new ArrayBuffer(1<<20);
var asmModule = AsmModule(global, env, heap);


print(asmModule.f1(1,1.4));
print(asmModule.f2(1,8));
print(asmModule.f3(2));
print(asmModule.f4(3));
print(asmModule.f5(0));
print(asmModule.f6());
print(asmModule.f7());
print(asmModule.f8());
print(asmModule.f9());
print(asmModule.f10());
print(asmModule.f11());
print(asmModule.f12(6));
print(asmModule.f13(6));
print(asmModule.f14(6));
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
print(asmModule.f27());
print(asmModule.f28());
print(asmModule.f29(3));
print(asmModule.f30());
print(asmModule.f31());
print(asmModule.f32());
print(asmModule.f33());
print(asmModule.f34());
print(asmModule.f35());
print(asmModule.f36());
print(asmModule.f37());
print(asmModule.f38());
print(asmModule.f39());
print(asmModule.f40());
print(asmModule.f41());
print(asmModule.f42());
print(asmModule.f43());
print(asmModule.f44());
print(asmModule.f45());
print(asmModule.f46());
print(asmModule.f47());
print(asmModule.f48());
print(asmModule.f49());
print(asmModule.f50());
print(asmModule.f51());
print(asmModule.f52());
print(asmModule.f53());
print(asmModule.f54());
//print(asmModule.f55());
print(asmModule.f56());
print(asmModule.f57());
print(asmModule.f58());
print(asmModule.f59());
print(asmModule.f60());
print(asmModule.f61());
print(asmModule.f62());
print(asmModule.f63());
print(asmModule.f64());
print(asmModule.f65());
print(asmModule.f66());
print(asmModule.f67());
print(asmModule.f68(3));
print(asmModule.f69(3));
print(asmModule.f70(3));
print(asmModule.f71(3));
print(asmModule.f72(3));
print(asmModule.f73());
print(asmModule.f74());
print(asmModule.f75());
print(asmModule.f76(42));
print(asmModule.f77(8));
print(asmModule.f78(1,5));
print(asmModule.f78(6,5));
print(asmModule.f79(1,5));
print(asmModule.f79(6,5));
print(asmModule.f80(1,5));
print(asmModule.f80(6,5));
print(asmModule.f81());
print(asmModule.f82());
print(asmModule.f83());
print(asmModule.f84());
print(asmModule.f85());
print(asmModule.f86());
print(asmModule.f87());
print(asmModule.f88());
print(asmModule.f89());
print(asmModule.f90());
print(asmModule.f93(10));
print(asmModule.f91());
print(asmModule.f94(1));
print(asmModule.f94(2));
print(asmModule.f94(133742));
print(asmModule.f94(133743));
print(asmModule.f95(-1));
print(asmModule.f95(0));
print(asmModule.f95(1));
print(asmModule.f96(-1));
print(asmModule.f96(0));
print(asmModule.f96(1));
print(asmModule.f96(0xffffffff));
print(asmModule.f97(-1));
print(asmModule.f97(0));
print(asmModule.f97(1));
print(asmModule.f97(2));
print(asmModule.f97(3));
print(asmModule.f98(-1));
print(asmModule.f98(0));
print(asmModule.f98(1));
print(asmModule.f98(2));
print(asmModule.f98(3));
print(asmModule.f99());
print(asmModule.f100());
print(asmModule.f101());
print(asmModule.f102(5));
print(asmModule.f103(1));
print(asmModule.f103(0));

print(asmModule.f104(1));
print(asmModule.f104(0));
print(guard.called());

print(asmModule.f105(1));
print(asmModule.f105(0));
print(guard.called());

print(asmModule.f106(1,1));
print(asmModule.f106(1,0));
print(asmModule.f106(0,0));
print(asmModule.f106(0,1));

print(asmModule.f107(1,1));
print(asmModule.f107(1,0));
print(asmModule.f107(0,0));
print(asmModule.f107(0,1));

for (var i = 0; i < 2; ++i)
    for (var j = 0; j < 2; ++j)
        for (var k = 0; k < 2; ++k)
            print(asmModule.f108(i,j,k));

function CheckTwoArgsTwoOptions(f) {
    function check(x,y) {
        return (x > 2 && y < 5) | 0;
    }
    for (var a = -10; a < 10; a++)
        for (var b = -10; b < 10; b++)
                print(f(a,b));
}

function CheckThreeArgsTwoOptions(f) {
    function check(x,y,z) {
        return (x > 2 && y < 5 && z > -1) | 0;
    }
    for (var a = -10; a < 10; a++)
        for (var b = -10; b < 10; b++)
            for (var c = -10; c < 10; c++)
                print(f(a,b,c));
}
CheckTwoArgsTwoOptions(asmModule.f109);
CheckTwoArgsTwoOptions(asmModule.f110);
CheckThreeArgsTwoOptions(asmModule.f111);
CheckThreeArgsTwoOptions(asmModule.f112);
CheckThreeArgsTwoOptions(asmModule.f113);
CheckThreeArgsTwoOptions(asmModule.f114);
CheckThreeArgsTwoOptions(asmModule.f115);
CheckThreeArgsTwoOptions(asmModule.f116);
CheckThreeArgsTwoOptions(asmModule.f117);
CheckThreeArgsTwoOptions(asmModule.f118);

CheckTwoArgsTwoOptions(asmModule.f119);
CheckTwoArgsTwoOptions(asmModule.f120);
CheckThreeArgsTwoOptions(asmModule.f121);
CheckThreeArgsTwoOptions(asmModule.f122);
CheckThreeArgsTwoOptions(asmModule.f123);
CheckThreeArgsTwoOptions(asmModule.f124);
CheckThreeArgsTwoOptions(asmModule.f125);
CheckThreeArgsTwoOptions(asmModule.f126);

var code = 'function AsmModule() {"use asm";\
    function g(x,y) {\
        x=x|0;\
        y=y|0;\
        var z = 0;\
        if ((y|0) == 1337) {\
            z = 1;\
        } else if ((x|0) == 1 ? 1 : ((x|0) < 0 ? (y|0) == 1 : 0)) {\
            z = 2;\
        } else if ((x|0) == 2) {\
            z = 3;\
        } else if ((x|0) == 3 ? 1 : (x|0) == 4) {\
            z = 4;\
        } else if ((x|0) == 5 ? (y|0) > 5 : 0) {\
            z = 5;\
        } else {\
            z = 6;\
        }\
        return z|0;\
    }\
    return g;}()';

eval("print(" + code + "(0, 1337))");
eval("print(" + code + "(0, 1338))");
eval("print(" + code + "(0, 0))");
eval("print(" + code + "(0, 1))");
eval("print(" + code + "(0, 1336))");
eval("print(" + code + "(1, 1337))");
eval("print(" + code + "(2, 1337))");
eval("print(" + code + "(3, 1337))");
eval("print(" + code + "(4, 1337))");
eval("print(" + code + "(5, 1337))");

eval("print(" + code + "(1, 10))");
eval("print(" + code + "(1, 1336))");
eval("print(" + code + "(-1, 10))");
eval("print(" + code + "(-1, 2))");
eval("print(" + code + "(-1, -1))");
eval("print(" + code + "(-1, 1))");
eval("print(" + code + "(-9, 1))");

eval("print(" + code + "(2, 1))");
eval("print(" + code + "(2, 0))");
eval("print(" + code + "(2, 6))");

eval("print(" + code + "(3, 1))");
eval("print(" + code + "(3, 0))");
eval("print(" + code + "(3, 6))");
eval("print(" + code + "(3, 3))");
eval("print(" + code + "(4, 1))");
eval("print(" + code + "(4, 0))");
eval("print(" + code + "(4, 6))");
eval("print(" + code + "(4, 3))");

eval("print(" + code + "(5, -1))");
eval("print(" + code + "(5, 4))");
eval("print(" + code + "(5, 5))");
eval("print(" + code + "(5, 6))");
eval("print(" + code + "(5, 10))");


