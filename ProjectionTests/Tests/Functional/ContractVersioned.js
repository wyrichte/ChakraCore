if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    runner.addTest({
        id: 1, 
        desc: 'Test instantiating a contract versioned class',
        pri: '0',
        test: function() {
            var xyzServer = new DevTests.ContractVersioned.Xyz();
            xyzServer.method1(314);
            xyzServer.property1 = 5;
            verify(xyzServer.property1, 5, "Property1 should reflect written value");
        }
    });

    runner.addTest({
        id: 1, 
        desc: 'Test instantiating a Platform versioned class',
        pri: '0',
        test: function() {
            var xyzServer = new DevTests.ContractVersioned.XyzPlatformVersioned();
            xyzServer.method1(628);
        }
    });
    Loader42_FileName = "Contract versioned class test";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
