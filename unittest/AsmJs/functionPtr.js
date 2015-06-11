//
// Test cases below use code modified from Mozilla's asm.js test suite
//

print(function () {"use asm";function f() { return 42 } var tbl=[f]; return f}()());
print(function () {"use asm";function f() {return 0} function g() {return 1} var tbl=[f,g]; return f}()());
print(function () {"use asm";function f() {return 42} function g(i) { i=i|0; return tbl[i&1]()|0 } var tbl=[f,f]; return g}()(0));
print(function () {"use asm";function f() {return 42} function g(i) { i=i|0; return tbl[i&1]()|0 } const tbl=[f,f]; return g}()(0));
print(function () {"use asm";function f() {return 42} function g() {return 13} function h(i) { i=i|0; return tbl[i&1]()|0 } var tbl=[f,g]; return h}()(1));
print(function () {"use asm";function f() {return 42} function g() {return 13} function h(i) { i=i|0; return tbl[i&1]()|0 } const tbl=[f,g]; return h}()(1));
print(function () {"use asm";function f() {return 42} function g() {return 13} function h(i) { i=i|0; return tbl2[i&1]()|0 } var tbl1=[f,g]; var tbl2=[g,f]; return h}()(1));
print(function () {"use asm";function f() {return 42} function g() {return 13} function h(i) { i=i|0; return tbl2[i&1]()|0 } const tbl1=[f,g]; const tbl2=[g,f]; return h}()(1));
print(function () {"use asm";function f() {return 42} function g() {return 13} function h(i) { i=i|0; return tbl2[i&3]()|0 } var tbl1=[f,g]; var tbl2=[g,g,g,f]; return h}()(3));
print(function () {"use asm";function f() {return 42} function g() {return 13} function h(i) { i=i|0; return tbl2[i&3]()|0 } const tbl1=[f,g]; const tbl2=[g,g,g,f]; return h}()(3));
print(function () {"use asm";function f() {return 42} function g() {return 13} function h(i) { i=i|0; return tbl1[i&1]()|0 } var tbl1=[f,g]; var tbl2=[g,g,g,f]; return h}()(1));
print(function () {"use asm";function f() {return 42} function g() {return 13} function h(i) { i=i|0; return tbl1[i&1]()|0 } const tbl1=[f,g]; const tbl2=[g,g,g,f]; return h}()(1));
print(function () {"use asm";var i=0,j=0; function f() {return i|0} function g() {return j|0} function h(x) { x=x|0; i=5;j=10; return tbl2[x&3]()|0 } var tbl1=[f,g]; var tbl2=[g,g,g,f]; return h}()(3));
print(function () {"use asm";var i=0,j=0; function f() {return i|0} function g() {return j|0} function h(x) { x=x|0; i=5;j=10; return tbl2[x&3]()|0 } const tbl1=[f,g]; const tbl2=[g,g,g,f]; return h}()(3));
print(function (glob,imp) {"use asm";var ffi=imp.ffi; function f() {return ffi()|0} function g() {return 13} function h(x) { x=x|0; return tbl2[x&3]()|0 } var tbl2=[g,g,g,f]; return h}(this, {ffi:function(){return 20}})(3));
print(function (glob,imp) {"use asm";const ffi=imp.ffi; function f() {return ffi()|0} function g() {return 13} function h(x) { x=x|0; return tbl2[x&3]()|0 } const tbl2=[g,g,g,f]; return h}(this, {ffi:function(){return 20}})(3));
print(function (glob,imp) {"use asm";var ffi=imp.ffi; var i=0; function f() {return ((ffi()|0)+i)|0} function g() {return 13} function h(x) { x=x|0; i=2; return tbl2[x&3]()|0 } var tbl2=[g,g,g,f]; return h}(this, {ffi:function(){return 20}})(3));
print(function (glob,imp) {"use asm";const ffi=imp.ffi; var i=0; function f() {return ((ffi()|0)+i)|0} function g() {return 13} function h(x) { x=x|0; i=2; return tbl2[x&3]()|0 } const tbl2=[g,g,g,f]; return h}(this, {ffi:function(){return 20}})(3));
print(function () {"use asm";function f(i) {i=i|0; return +((i+1)|0)} function g(d) { d=+d; return +(d+2.5) } function h(i,j) { i=i|0;j=j|0; return +tbl2[i&1](+tbl1[i&1](j)) } var tbl1=[f,f]; var tbl2=[g,g]; return h}()(0,10));
print(function () {"use asm";function f(i) {i=i|0; return +((i+1)|0)} function g(d) { d=+d; return +(d+2.5) } function h(i,j) { i=i|0;j=j|0; return +tbl2[i&1](+tbl1[i&1](j)) } const tbl1=[f,f]; const tbl2=[g,g]; return h}()(0,10));
print(function () {"use asm";function f() {return 42} function g() { return tbl[0&0]()|0 } var tbl=[f]; return g}()());
print(function () {"use asm";function f() {return 42} function g() { return tbl[0&0]()|0 } const tbl=[f]; return g}()());
print(function () {"use asm";function f1() {return 42} function f2() {return 13} function g() { return tbl[1&1]()|0 } var tbl=[f1,f2]; return g}()());
print(function () {"use asm";function f1() {return 42} function f2() {return 13} function g() { return tbl[1&1]()|0 } const tbl=[f1,f2]; return g}()());
var m1 = function () {"use asm";function f1(d) {d=+d; return +(d/2.0)} function f2(d) {d=+d; return +(d+10.0)} function g(i,j) { i=i|0;j=+j; return +tbl[i&1](+tbl[(i+1)&1](j)) } var tbl=[f1,f2]; return g}();
print(m1(0,10.2));
print(m1(1,10.2));
var m2 = function(glob,imp){"use asm";var ffi=imp.ffi; function f(){return 13} function g(){return 42} function h(i) { i=i|0; var j=0; ffi(1); j=TBL[i&7]()|0; ffi(1.5); return j|0 } var TBL=[f,g,f,f,f,f,f,f]; return h}(this,{ffi:function(){}});
for (var i = 0; i < 100; i++)
    print(m2(i));
