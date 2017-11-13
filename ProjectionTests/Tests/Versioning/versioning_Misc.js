if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    runner.addTest({
        id: "Win8: 837548",
        desc: 'Read-only property versioned to read-write property -- Read-only under target version Win8',
        pri: '0',
        test: function () {
            var obj = new DevTests.Repros.VersionedProperties.ReadOnlyVersionedProperty();
            verify.defined(obj.testProperty, "obj.testProperty");

            var prototype = Object.getPrototypeOf(obj);
            var desc = Object.getOwnPropertyDescriptor(prototype, "testProperty");
            verify.defined(desc, "obj.testProperty property descriptor");
            verify.typeOf(desc.get, "function");
            verify.typeOf(desc.set, "undefined");

            verify(obj.testProperty, 0, "obj.testProperty");
            obj.testProperty = 42;
            verify(obj.testProperty, 0, "obj.testProperty");
        }
    });

    Loader42_FileName = 'Miscellaneous Versioning Scenarios';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
