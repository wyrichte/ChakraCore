function test0() {
    var obj0 = {};
    var Value;
    Object.defineProperty(obj0, "prop0", {get:function(){return Value}, set:function(x){Value = x;}});
    obj0.prop0 = 2.2;
    var sc1 = WScript.LoadScriptFile("CrossSiteFixedField_blank.js", "samethread");
    sc1.obj0 = obj0;
    var sc1_cctx = sc1.Debug.parseFunction("obj0.prop0");
    sc1_cctx();
};
test0();
WScript.Echo("PASSED");