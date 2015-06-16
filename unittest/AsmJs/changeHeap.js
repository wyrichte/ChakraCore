//
// Test cases below use code modified from Mozilla's asm.js test suite
//

const BUF_64KB = new ArrayBuffer(64 * 1024);
(function(glob){"use asm";var I32=glob.Int32Arra; function f() {} return f})(this, {}, BUF_64KB);
(function(glob){"use asm";var I32=glob.Int32Array; function f() {} return f})({});
(function(glob){"use asm";var I32=glob.Int32Array; function f() {} return f})({Int32Array:null});
(function(glob){"use asm";var I32=glob.Int32Array; function f() {} return f})({Int32Array:{}});
(function(glob){"use asm";var I32=glob.Int32Array; function f() {} return f})({Int32Array:Uint32Array});
print((function(glob){"use asm";var I32=glob.Int32Array; function f() {} return f})({Int32Array:Int32Array}));

print((function(glob,ffis,buf){"use asm";var I32=glob.Int32Array; function f() {} return f})(this)());
print((function(glob,ffis,buf){"use asm";var I32=glob.Int32Array; function f() {} return f})(this,null,BUF_64KB)());

(function(glob,ffis,buf){"use asm";var I32=glob.Int32Array; var i32=new I32(buf); function f() {} return f})(this,null,new ArrayBuffer(100));
print((function(glob,ffis,buf){"use asm";var I32=glob.Int32Array; var i32=new I32(buf); function f() {} return f})(this,null,BUF_64KB));

(function(glob,ffis,buf){"use asm";var I32=glob.Int32Array; var i32=new glob.Int32Array(buf); function f() {} return f})(this,null,new ArrayBuffer(100));
print((function(glob,ffis,buf){"use asm";var I32=glob.Int32Array; var i32=new glob.Int32Array(buf); function f() {} return f})(this,null,BUF_64KB));

(function(glob,ffis,buf){"use asm";var F32=glob.Float32Array; var i32=new glob.Int32Array(buf); function f() {} return f})(this,null,new ArrayBuffer(100));
print((function(glob,ffis,buf){"use asm";var F32=glob.Float32Array; var i32=new glob.Int32Array(buf); function f() {} return f})(this,null,BUF_64KB));

(function(glob,ffis,buf){"use asm";var byteLength=glob.byteLength; function f() { return byteLength(1)|0 } return f})(this, {}, BUF_64KB);

print("byteLength" in this);
(function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this);
this['byteLength'] = null;
(function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this);
this['byteLength'] = {};
(function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this);
this['byteLength'] = function(){};
(function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this);
this['byteLength'] = (function(){}).bind(null);
(function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this);
this['byteLength'] = Function.prototype.call.bind();
(function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this);
this['byteLength'] = Function.prototype.call.bind({});
(function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this);
this['byteLength'] = Function.prototype.call.bind(function f() {});
(function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this);
this['byteLength'] = Function.prototype.call.bind(Math.sin);
(function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this);
this['byteLength'] =
  Function.prototype.call.bind(Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength').get);
print((function(glob){"use asm";var byteLength=glob.byteLength; function f() { return 42 } return f})(this)());

print((function(glob){"use asm";var b1=glob.byteLength, b2=glob.byteLength; function f() { return 43 } return f})(this)());

print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function b(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function f(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2,xyz) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(...r) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2,...r) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { ;if((len((b2))) & (0xffffff) || (len((b2)) <= (0xffffff)) || len(b2) > 0x80000000) {;;return false;;} ; i8=new I8(b2);; b=b2;; return true;; } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function ch2(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { 3; if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { b2=b2|0; if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(1) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(1 || 1) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(1 || 1 || 1) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(1 || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(1 & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || 1 || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(i8(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(xyz) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff && len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) | 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) == 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xfffffe || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0x1ffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) < 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xfffffe || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0x1000000 || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || 1) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) < 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || 1 > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0.0) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0xffffff) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x1000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0x1000000 || len(b2) > 0x1000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0x1000000 || len(b2) > 0x1000001) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000001) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) ; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) {} i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) {return false} i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return true; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i7=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; b=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=1; b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new 1; b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I7(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new b(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8; b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(1); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2,1); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); xyz=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=1; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; 1; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return 1 } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return false } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true; 1 } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b); var I32=glob.Int32Array; var i32=new I32(b); var II32=glob.Int32Array;var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b); var I32=glob.Int32Array; var i32=new I32(b); var II32=glob.Int32Array;var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i32=new I32(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b); var I32=glob.Int32Array; var i32=new I32(b); var II32=glob.Int32Array;var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i32=new I32(b2); i8=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b); var I32=glob.Int32Array; var i32=new I32(b); var II32=glob.Int32Array;var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); i32=new I32(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b); var I32=glob.Int32Array; var i32=new I32(b); var II32=glob.Int32Array;var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I32(b2); i32=new I8(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
print(function(glob,ffis,b){"use asm";var I8=glob.Int8Array; var i8=new I8(b); var I32=glob.Int32Array; var i32=new I32(b); var II32=glob.Int32Array;var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); i32=new II32(b2); b=b2; return true } function f() { return 42 } return f}(this, {}, BUF_64KB)());
const SETUP = '"use asm";var I8=glob.Int8Array; var i8=new I8(b); var I32=glob.Int32Array; var i32=new I32(b); var II32=glob.Int32Array;var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i8=new I8(b2); i32=new I32(b2); b=b2; return true } ';
eval("print(function(glob,ffis,b){" + SETUP + "function f() { i32[0] } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { i32[0] = 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[i >> 2] } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[i >> 2] = 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[(g()|0) >> 2] } function g() { return 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[(g()|0) >> 2] = 0 } function g() { return 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[i >> 2] = g()|0 } function g() { return 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[i32[(g()|0)>>2] >> 2] } function g() { return 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[i32[(g()|0)>>2] >> 2] = 0 } function g() { return 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[i >> 2] = i32[(g()|0)>>2] } function g() { return 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[((i32[i>>2]|0) + (g()|0)) >> 2] } function g() { return 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[((i32[i>>2]|0) + (g()|0)) >> 2] = 0 } function g() { return 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + SETUP + "function f() { var i = 0; i32[i >> 2] = (i32[i>>2]|0) + (g()|0) } function g() { return 0 } return f}(this, {}, BUF_64KB)())");

const HEADER = '"use asm";var I8=glob.Int8Array; var i8=new I8(b);var len = glob.byteLength; function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= MIN || len(b2) > 0x80000000) return false; i8=new I8(b2); b=b2; return true } ';
eval("print(function(glob,ffis,b){" + HEADER.replace('MIN', '0xffffff') + "function f() { i8[0x1000000] = 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + HEADER.replace('MIN', '0xffffff') + "function f() { i8[0xffffff] = 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + HEADER.replace('MIN', '0x1000000') + "function f() { i8[0x1000001] = 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + HEADER.replace('MIN', '0x1000000') + "function f() { i8[0x1000000] = 0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + HEADER.replace('MIN', '0xffffff') + "function f() { return i8[0x1000000]|0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + HEADER.replace('MIN', '0xffffff') + "function f() { return i8[0xffffff]|0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + HEADER.replace('MIN', '0x1000000') + "function f() { return i8[0x1000001]|0 } return f}(this, {}, BUF_64KB)())");
eval("print(function(glob,ffis,b){" + HEADER.replace('MIN', '0x1000000') + "function f() { return i8[0x1000000]|0 } return f}(this, {}, BUF_64KB)())");

var m = function(glob,ffis,b){"use asm";var I32=glob.Int32Array; var i32=new I32(b);var len=glob.byteLength;function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i32=new I32(b2); b=b2; return true } function get(i) { i=i|0; return i32[i>>2]|0 } function set(i, v) { i=i|0; v=v|0; i32[i>>2] = v } return {get:get, set:set, changeHeap:ch}};
var buf1 = new ArrayBuffer(0x1000000);
var o = m(this,{},buf1);
print(m.toString());
o.set(0, 42);
o.set(4, 13);
o.set(4, 13);
print(o.get(0));
print(o.get(4));
o.set(0x1000000, 262);
print(o.get(0x1000000));
var buf2 = new ArrayBuffer(0x2000000);
print(o.changeHeap(buf2));
print(o.get(0));
print(o.get(4));
o.set(0x1000000, 262);
print(o.get(0x1000000));
o.set(0x2000000, 262);
print(o.get(0x2000000));
print(o.changeHeap(buf1));
print(o.get(0));
print(o.get(4));
o.set(0x1000000, 262);
print(o.get(0x1000000));


var buf1 = new ArrayBuffer(0x1000000);
new Int32Array(buf1)[0] = 13;
var buf2 = new ArrayBuffer(0x1000000);
new Int32Array(buf2)[0] = 42;

var changeToBuf = null;
var m = function(glob,ffis,b){"use asm";
                    var ffi=ffis.ffi;
                    var I32=glob.Int32Array; var i32=new I32(b);
                    var len=glob.byteLength;
                    function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i32=new I32(b2); b=b2; return true }
                    function test(i) { i=i|0; var sum=0; sum = i32[i>>2]|0; sum = (sum + (ffi()|0))|0; sum = (sum + (i32[i>>2]|0))|0; return sum|0 }
                    return {test:test, changeHeap:ch}};
var ffi = function() { o.changeHeap(changeToBuf); return 1 }
var o = m(this, {ffi:ffi}, buf1);
changeToBuf = buf1;
print(o.test(0));
changeToBuf = buf2;
print(o.test(0));
changeToBuf = buf2;
print(o.test(0));
changeToBuf = buf1;
print(o.test(0));
changeToBuf = buf1;
print(o.test(0));

var buf3 = new ArrayBuffer(0x10000);
var m = function(glob,ffis,b){"use asm";
                    var I32=glob.Int32Array; var i32=new I32(b);
                    var len=glob.byteLength;
                    function ch(b2) { if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000) return false; i32=new I32(b2); b=b2; return true }
                    function test(i) { i=i|0; var a=0xFFFFFF; var b = 0; b = i32[a >> 2]|0; return b|0 }
                    return {test:test, changeHeap:ch}};
var o = m(this, {}, buf3);
print(o.test(0));
