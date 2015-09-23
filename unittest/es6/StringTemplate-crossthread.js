// ES6 Cross-thread String Template tests -- verifies the API shape and basic functionality

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

function GetCallsite(callsite) {
    return callsite;
}

var tests = [
    {
        name: "Callsite objects are not shared cross-Realm",
        body: function() {
            var callsite = GetCallsite`uniquestringforrealmcachetest\n`;

            assert.areEqual('uniquestringforrealmcachetest\\n', callsite.raw[0], 'String template callsite has correct raw value');

            var crossRealmCrossThread = WScript.LoadScriptFile("StringTemplate-crossthread-child.js","crossthread");
            var crossRealmSameThread = WScript.LoadScriptFile("StringTemplate-crossthread-child.js","samethread");
            var sameRealm = WScript.LoadScriptFile("StringTemplate-crossthread-child.js","self");

            assert.areEqual('uniquestringforrealmcachetest\\n', crossRealmCrossThread.callsite.raw[0], "Cross thread, cross Realm string template callsite has correct value");
            assert.areEqual('uniquestringforrealmcachetest\\n', crossRealmSameThread.callsite.raw[0], "Same thread, cross Realm string template callsite has correct value");
            assert.areEqual('uniquestringforrealmcachetest\\n', sameRealm.callsite.raw[0], "Same Realm string template callsite has correct value");

            assert.isFalse(callsite === crossRealmCrossThread.callsite, "Cross thread, cross Realm does not share string template callsite object cache");
            assert.isFalse(callsite === crossRealmSameThread.callsite, "Same thread, cross Realm does not share string template callsite object cache");
            assert.isTrue(callsite === sameRealm.callsite, "Same Realm (cross-frame) does share string template callsite object cache");
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
