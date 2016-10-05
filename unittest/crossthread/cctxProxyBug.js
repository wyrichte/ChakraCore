function test0() {
    var func1 = function (...argArr0) {
        argArr0.join(',');
    };
    var func3 = function () {
        var sc0 = WScript.LoadScript('', 'samethread');
        sc0.func1 = func1;
        sc0.ary = ary;
        var sc0_cctx = sc0.Debug.parseFunction('func1.call(1 , ary);\nary = new Proxy(ary,  {});\n    ');
        sc0_cctx();
        sc0_cctx();
    };
    var ary = Array();
    func3();
}
test0();
print('pass');