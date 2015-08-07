var a = eval(`
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
        
        function ch(b2)
        {
            if(len(b2) & 0xffffff || len(b2) <= 0xffffff || len(b2) > 0x80000000)
                return false;
            HEAP32=new I32(b2);
            i8=new I8(b2);
            HEAPU8=new U8(b2);
            heap=b2; /**bp:locals(1);stack()**/
            return true
        }
        
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
        return {fExp:f, gExp:g, ch:ch};
    }
    return foo;
}
var moduleFunc = outer();
this['byteLength'] = Function.prototype.call.bind(Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength').get);
var buffer = new ArrayBuffer(1<<24);
var module = moduleFunc(this, {bar: function f(c){print("import func, val " + c)}, c: 4.5, d: 12}, buffer);
print(Math.round(module.gExp(2)));
print(Math.round(module.fExp(1)));
`);

function debugCallback()
{
    print("debugCallback");
    print(Math.round(module.fExp(1)));
    print(Math.round(module.gExp(2)));
    print(module.ch(new ArrayBuffer(1<<25)));
    print(Math.round(module.gExp(2)));
}
WScript.Attach(debugCallback);
WScript.Detach(debugCallback);