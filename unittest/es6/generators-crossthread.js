var csObj = WScript.LoadScript('', 'samethread');
var func = function () {
return func.caller;
};
csObj.func = func;
var csObj_cctx = csObj.Debug.parseFunction(`
    function* gf(flag, value) {
        flag ? yield func() : yield (value * value);
    }
    var gen = gf(true).next().value;
    print(gf === gen, "Generator function returned through the caller property should be the same as the original generator function");
    print(100 === gen(false, 10).next().value, "Generator function returned through the caller property should behave the same way as the original generator function");
`);
csObj_cctx();
