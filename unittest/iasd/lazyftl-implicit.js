if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "LazyFTL Getter/Setters do not call trampoline when implicit calls are disabled",
        body: function () {
            // Set up lazy FTL slots and trigger the trampolines
            var obj = Debug.createTypedObject(2000, "mytype", 32);
            var proto = Debug.createTypedObject(2000, "mytype", 32);
            obj.__proto__ = proto;
            var b = Debug.addLazyFTLProperty(obj, "getFld", 1);
            var c = Debug.addLazyFTLProperty(obj, "setFld", 2);

            Debug.disableImplicitCalls();
            obj.setFld = 3;
            var getterResult = obj.getFld
            var setterResult =  obj.setFld;
            Debug.enableImplicitCalls();
            assert.areEqual(undefined, getterResult, "LazyFTL getter does not call trampoline with implicit calls disabled");
            assert.areEqual(undefined, setterResult, "LazyFTL setter does not call trampoline with implicit calls disabled");
        }
    }
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });