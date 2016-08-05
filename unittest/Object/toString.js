if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "DOM objects revert to class name",
        body: function () {
            // NOTE: This test will no longer be relevant when @@toStringTag is standard in WebIDL spec and implemented in Edge.
            //       New tests will be needed pending the consensus in WebIDL. First implementation is expected RS2/RS3.
            var o = new Image();

            assert.areEqual(undefined,                            Object.getPrototypeOf(Image.prototype)[Symbol.toStringTag], "DOM Image prototype does not have @@toStringTag");
            assert.areEqual("[object HTMLImageElement]" ,         Object.prototype.toString.call(o),                          "DOM Image instance uses class name in absence of @@toStringTag");
            assert.areEqual("[object HTMLImageElementPrototype]", Object.prototype.toString.call(o.__proto__),                "DOM Image prototype uses class name in absence of @@toStringTag");
        }
    }
];

testRunner.runTests(tests);