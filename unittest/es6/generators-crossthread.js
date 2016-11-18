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
    if (gf === gen) {
        print("PASSED");
    } else {
        print("FAILED");
    }
    if (100 === gen(false, 10).next().value) {
        print("PASSED");
    } else {
        print("FAILED");
    }
`);
csObj_cctx();
