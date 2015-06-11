//-ExpirableCollectionTriggerThreshold:0 -ExpirableCollectionGCCount:0 -ForceExpireOnNonCacheCollect -mic:1 -off:simplejit
function test0() {
    var obj0 = {};
    var Value;
	WScript.Echo(foo(1,2) === true)
	foo(1,2)
	foo(1,2)
    var sc1 = WScript.LoadScriptFile("CrossSiteFixedField_blank.js", "samethread");
    sc1.foo = foo;
	CollectGarbage()
	CollectGarbage()
    var sc1_cctx = sc1.Debug.parseFunction("WScript.Echo(foo(4,5) === true)");
    sc1_cctx();
};

function foo(a,b)
{
	WScript.Echo(a + b);
	return true;
	
}
test0();
WScript.Echo("PASSED");
