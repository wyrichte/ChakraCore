//
// Test cases below use code modified from Mozilla's asm.js test suite
//

print(function(glob,i,b){"use asm";var i32=new glob.Int32Array(b); function f(){} return f}(this, {}, new ArrayBuffer(65536))());
print(function(glob,i,b){"use asm";var i32=new glob.Int32Array(b); function f(){} return f}(this, {}, new ArrayBuffer(2*65536))());

print(function(glob,imp,b){"use asm";const i8=new glob.Int8Array(b);var u8=new glob.Uint8Array(b);const i16=new glob.Int16Array(b);var u16=new glob.Uint16Array(b);const i32=new glob.Int32Array(b);var u32=new glob.Uint32Array(b);const f32=new glob.Float32Array(b);var f64=new glob.Float64Array(b);function f(i) {i=i|0; i = i32[i>>2]|0; return i|0}; return f}(this, {}, new ArrayBuffer(65536))());
//var exp = function(){"use asm";return {}}();
//print(Object.keys(exp).length);

var exp = function(){"use asm";function f() { return 3 } return {f:f,f:f}}();
print(exp.f());
print(Object.keys(exp).join());

var exp = function(){"use asm";function internal() { return ((g()|0)+2)|0 } function f() { return 1 } function g() { return 2 } function h() { return internal()|0 } return {f:f,g1:g,h1:h}}();
print(exp.f());
print(exp.g1());
print(exp.h1());
print(Object.keys(exp).join());