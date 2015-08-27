//
// Test cases below use code modified from Mozilla's asm.js test suite
//

function ffi(a,b,c,d) {
    return a+b+c+d;
}

print(function(global,imp){"use asm";var ffi=imp.ffi; function g() { return 1 } function f() { var i=0; i=g()|0; return ((ffi(4,5,6,7)|0)+i)|0 } return f}(this,{ffi:ffi})(1));

var counter = 0;
function inc() { return counter++ }
function add1(x) { return x+1 }
function add2(x,y) { return x+y }
function add3(x,y,z) { return x+y+z }
function addN() {
    var sum = 0;
    for (var i = 0; i < arguments.length; i++)
        sum += arguments[i];
    return sum;
}
var imp = { inc:inc, add1:add1, add2:add2, add3:add3, addN:addN, identity: x => x };

print(function(global,imp){"use asm";var inc=imp.inc; function g() { inc() } return g}(this,imp)());
print(counter);

var f = function(global,imp){"use asm";var inc=imp.inc; function g() { return inc()|0 } return g}(this,imp);
print(f());
print(counter);
print(f());
print(counter);

print(function(global,imp){"use asm";var add1=imp.add1; function g(i) { i=i|0; return add1(i|0)|0 } return g}(this,imp)(9));
print(function(global,imp){"use asm";const add1=imp.add1; function g(i) { i=i|0; return add1(i|0)|0 } return g}(this,imp)(9));
print(function(global,imp){"use asm";var add3=imp.add3; function g() { var i=1,j=3,k=9; return add3(i|0,j|0,k|0)|0 } return g}(this,imp)());
print(function(global,imp){"use asm";const add3=imp.add3; function g() { var i=1,j=3,k=9; return add3(i|0,j|0,k|0)|0 } return g}(this,imp)());
print(function(global,imp){"use asm";var add3=imp.add3; function g() { var i=1.4,j=2.3,k=32.1; return +add3(i,j,k) } return g}(this,imp)());
print(function(global,imp){"use asm";const add3=imp.add3; function g() { var i=1.4,j=2.3,k=32.1; return +add3(i,j,k) } return g}(this,imp)());
print(function(global,imp){"use asm";var add3=imp.add3; function f(i,j,k) { i=i|0;j=+j;k=k|0; return add3(i|0,j,k|0)|0 } return f}(this,imp)(1, 2.5, 3));
print(function(global,imp){"use asm";var addN=imp.addN; function f() { return +addN(1,2,3,4.1,5,6.1,7,8.1,9.1,10,11.1,12,13,14.1,15.1,16.1,17.1,18.1) } return f}(this,imp)());
print(function(global,imp){"use asm";var add2=imp.add2; function f(i,j) { i=i|0;j=+j; return +(+(add2(i|0,1)|0) + +add2(j,1) + +add2(+~~i,j)) } return f}(this,imp)(2, 5.5));
print(function(global,imp){"use asm";var addN=imp.addN; function f(i,j) { i=i|0;j=+j; return +(+addN(i|0,j,3,j,i|0) + +addN() + +addN(j,j,j)) } return f}(this,imp)(1, 2.2));

counter = 0;
print(function(global,imp){"use asm";var addN=imp.addN,inc=imp.inc; function f() { return ((addN(inc()|0,inc(3.3)|0,inc()|0)|0) + (addN(inc(0)|0)|0))|0 } return f}(this,imp)());
print(counter);

var recurse = function(i,j) { if (i == 0) return j; return f(i-1,j+1)+j }
imp.recurse = recurse;

var f = function(global,imp){"use asm";var r=imp.recurse; function f(i,j) { i=i|0;j=+j; return +r(i|0,j) } return f}(this,imp);
print(f(0,3.3));
print(f(1,3.3));
print(f(2,3.3));

function maybeThrow(i, j) {
    if (i == 0)
        throw j;
    try {
        return f(i-1, j);
    } catch(e) {
        print(typeof e);
        return e;
    }
}

var f = function(global,imp){"use asm";var ffi=imp.ffi; function f(i, j) { i=i|0;j=j|0; return ffi(i|0, (j+1)|0)|0 } return f}(this,{ffi:maybeThrow});
try{
    f(0,0);
}catch(e){
    print(e);
}
try{
    f(0,Math.pow(2,31)-1);
}catch(e){
    print(e);
}
print(f(1,0));
print(f(2,0));
print(f(3,0));
print(f(4,5));

this.import1 = function(a){ return a + 1.5;};
print(function(global,imp){"use asm"; var g = imp.import1; function f(x) { x=+x; return g(x)|0;} return f}(this,this)(13.37));
print(function(global,imp){"use asm";function f(x) { x=+x; return +x } return f}(this,null)(13.37));
print(function(global,imp){"use asm";function f(x) { x=+x; return +x } return f}(this,1)(13.37));

var recurse = function(i,j) { if (i == 0) throw j; f(i-1,j) }
var f = function(global,imp){"use asm";var ffi=imp.ffi; function g(i,j,k) { i=i|0;j=+j;k=k|0; if (!(k|0)) ffi(i|0,j)|0; else g(i, j+1.0, (k-1)|0) } function f(i,j) { i=i|0;j=+j; g(i,j,4) } return f}(this,{ffi:recurse});
try{
    f(0,2.4);
}catch(e){
    print(e);
}
try{
    f(1,2.4);
}catch(e){
    print(e);
}
try{
    f(8,2.4);
}catch(e){
    print(e);
}

print(function(global,imp){"use asm";var identity=imp.identity; function g(x) { x=+x; return +identity(x) } return g}(this,imp)(13.37));

this.importUndef = function(a){ return undefined;};
print(function(global,imp){"use asm"; var g = imp.importUndef; function f(x) { x=+x; return +g(x);} return f}(this,this)(13.37));
print(function(global,imp){"use asm"; var g = imp.importUndef; function f(x) { x=+x; return g(x)|0;} return f}(this,this)(13.37));

// asm.js link errors
print(function(global,imp){"use asm"; var g = imp.doesntExist; function f(x) { x=+x; return +x;} return f}(this,{})(13.37));
print(function(global,imp){"use asm"; var g = imp.a; function f(x) { x=+x; return +x;} return f}(this,{a:1})(13.37));
var implicit={get:function(){print("implicit get");}, valueOf:function(){print("implicit valueOf");}}
print(function(global,imp){"use asm"; var g = +imp.a; function f(x) { x=+x; return +x;} return f}(this,{a:implicit})(13.37));

