function outer()
{
    function foo(stdlib, imports, heap)
    {
        "use asm";
        var fr = stdlib.Math.fround;
        var a = 3;
        var impa = +imports.c;
        var I32 =stdlib.Int32Array;
        var HEAP32 = new stdlib.Int32Array(heap);
        const b = 3.5;
        var inf = stdlib.Infinity;
        var I8=stdlib.Int8Array;
        var len = stdlib.byteLength;
        var ln2 = stdlib.Math.LN2;
        var i8= new I8(heap);
        var c = fr(4);
        var sn = stdlib.Math.sin;
        var impFunc = imports.bar;
        var impb = imports.d|0;
        var U8 =stdlib.Uint8Array;
        var HEAPU8 =new stdlib.Uint8Array(heap);
        
        function f(param)
        {
            param = param|0;
            var e = 0.;
            a = (a+param)|0;
            a = (impb+a)|0;
            impa = +(impa+b);
            c = fr(c+c);
            impFunc(+c);
            e = +sn(+impa);
            a = (a + (HEAPU8[a|0]|0))|0; /**bp:locals(1);stack()**/
            return +e;
        }
        function g(b)
        {
            b = b|0;
            impa = +table1[b&3](b);
            i8[a|0] = impb|0;
            impa = +(impa + ln2); /**bp:locals(1);stack()**/
            return +impa;
        }
        var table1 = [f, g, f, g];
        return {fExp:f, gExp:g};
    }
    return foo;
}
var moduleFunc = outer();


var m1 = function namedExpr() {
    "use asm";
    function o() {}
    return o;
};
var m1i = m1();
var m2 = function () {
    "use asm";
    function o() {}
    return o;
};
var m2i = m2();

this['byteLength'] = Function.prototype.call.bind(Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength').get);
var buffer = new ArrayBuffer(1<<24);
var module1 = moduleFunc(this, {bar: function f(c){print("import func, val " + c)}, c: 4.5, d: 12}, buffer);
var module2 = moduleFunc(this, {bar: function f(c){print("import2 func, val " + c)}, c: 5.5, d: 13}, new ArrayBuffer(1<<25));
print(Math.round(module1.gExp(2)));
print(Math.round(module1.fExp(1)));
function debugCallback()
{
    print("debugCallback");
    print(Math.round(module2.fExp(1)));
    print(Math.round(module2.gExp(2)));
    print(Math.round(module1.fExp(1)));
    print(Math.round(module1.gExp(2)));
}
WScript.Attach(debugCallback);
WScript.Detach(debugCallback);
WScript.Attach(debugCallback);
