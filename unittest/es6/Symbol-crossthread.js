// ES6 Cross-thread Symbol tests -- verifies the API shape and basic functionality

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: 'Symbol registration cross-thread',
        body: function() {
            var parent_sym = Symbol.for('parent symbol');

            var child = WScript.LoadScriptFile("ES6Symbol_cross_context_registration_child.js", "crossthread");

            var child_sym = Symbol.for('child symbol');

            assert.isFalse(child.child_sym === child_sym, "Symbol registered in child is a different symbol from the one registered in parent");
            assert.isFalse(child.parent_sym === parent_sym, "Symbol registered in parent is a different symbol from the one registered in child");
            assert.isTrue(child.parent_string === Symbol.keyFor(parent_sym), "Symbol registered in parent is returned correctly in child");
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
