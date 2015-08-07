// ES6 Function extension tests -- verifies the API shape and basic functionality

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "toMethod() should throw exceptions on illegal arguments",
        body: function() {
            function f() {}

            assert.throws(function () {Function.prototype.toMethod.call(undefined, {}); }, TypeError,
                "toMethod() should not be called from undefined", "Function.prototype.toMethod: 'this' is not a Function object");
            assert.throws(function () {Function.prototype.toMethod.call(null, {}); }, TypeError,
                "toMethod() should not be called from null", "Function.prototype.toMethod: 'this' is not a Function object");
            assert.throws(function () {Function.prototype.toMethod.call(true, {}); }, TypeError,
                "toMethod() should not be called from Boolean", "Function.prototype.toMethod: 'this' is not a Function object");
            assert.throws(function () {Function.prototype.toMethod.call(false, {}); }, TypeError,
                "toMethod() should not be called from Boolean", "Function.prototype.toMethod: 'this' is not a Function object");
            assert.throws(function () {Function.prototype.toMethod.call("string", {}); }, TypeError,
                "toMethod() should not be called from String", "Function.prototype.toMethod: 'this' is not a Function object");
            assert.throws(function () {Function.prototype.toMethod.call(0, {}); }, TypeError,
                "toMethod() should not be called from Number", "Function.prototype.toMethod: 'this' is not a Function object");

            assert.throws(function () {Function.prototype.toMethod.call(f); }, TypeError,
                "toMethod() should have an argument 'newHome'", "Function.prototype.toMethod: argument is not an Object");
            assert.throws(function () {Function.prototype.toMethod.call(f, undefined); }, TypeError,
                "toMethod() argument 'newHome' should not be undefined", "Function.prototype.toMethod: argument is not an Object");
            assert.throws(function () {Function.prototype.toMethod.call(f, null); }, TypeError,
                "toMethod() argument 'newHome' should not be null", "Function.prototype.toMethod: argument is not an Object");
            assert.throws(function () {Function.prototype.toMethod.call(f, true); }, TypeError,
                "toMethod() argument 'newHome' should not be Boolean", "Function.prototype.toMethod: argument is not an Object");
            assert.throws(function () {Function.prototype.toMethod.call(f, false); }, TypeError,
                "toMethod() argument 'newHome' should not be Boolean", "Function.prototype.toMethod: argument is not an Object");
            assert.throws(function () {Function.prototype.toMethod.call(f, "string"); }, TypeError,
                "toMethod() argument 'newHome' should not be String", "Function.prototype.toMethod: argument is not an Object");
            assert.throws(function () {Function.prototype.toMethod.call(f, 0); }, TypeError,
                "toMethod() argument 'newHome' should not be Number", "Function.prototype.toMethod: argument is not an Object");
        }
    },
    {
        name: "toMethod() should be functional on builtin functions",
        body: function() {
            class A {}
            let a=new A();
            
            A.prototype.mathcos_at_prototype=Math.cos.toMethod(A.prototype);
            a.mathcos_at_object=Math.cos.toMethod(a);
            A.mathcos_static=Math.cos.toMethod(A);
            A.prototype.eval_at_prototype=eval.toMethod(A.prototype);
            a.eval_at_object=eval.toMethod(a);
            A.eval_static=eval.toMethod(A);

            // "tag" each object returned by toMethod()
            A.prototype.mathcos_at_prototype.tag="A.prototype.mathcos_at_prototype";
            a.mathcos_at_object.tag="a.mathcos_at_object";
            A.mathcos_static.tag="A.mathcos_static";
            A.prototype.eval_at_prototype.tag="A.prototype.eval_at_prototype";
            a.eval_at_object.tag="a.eval_at_object";
            A.eval_static.tag="A.eval_static";
            
            // check the tags to ensure they are all different objects
            assert.areEqual("A.prototype.mathcos_at_prototype", A.prototype.mathcos_at_prototype.tag, "toMethod() should create a new object A.prototype.mathcos_at_prototype");
            assert.areEqual("a.mathcos_at_object", a.mathcos_at_object.tag, "toMethod() should create a new object a.mathcos_at_object");
            assert.areEqual("A.mathcos_static", A.mathcos_static.tag, "toMethod() should create a new object A.mathcos_static");            
            assert.areEqual("A.prototype.eval_at_prototype", A.prototype.eval_at_prototype.tag, "toMethod() should create a new object A.prototype.eval_at_prototype");
            assert.areEqual("a.eval_at_object", a.eval_at_object.tag, "toMethod() should create a new object a.eval_at_object");
            assert.areEqual("A.eval_static", A.eval_static.tag, "toMethod() should create a new object A.eval_static");

            assert.areEqual(1, a.mathcos_at_prototype(0), "toMethod() applied on builtin function Math.cos & linked to prototype");
            assert.areEqual(1, a.mathcos_at_object(0), "toMethod() applied on builtin function Math.cos & linked to object");
            assert.areEqual(1, A.mathcos_static(0), "toMethod() applied on builtin function Math.cos & linked to class");
            assert.areEqual(2, a.eval_at_prototype("Math.exp(0)*2"), "toMethod() applied on builtin function eval & linked to prototype");
            assert.areEqual(2, a.eval_at_object("Math.exp(0)*2"), "toMethod() applied on builtin function eval & linked to object");
            assert.areEqual(2, A.eval_static("Math.exp(0)*2"), "toMethod() applied on builtin function eval & linked to class");
        }
    },
    {
        name: "toMethod() should be functional on a bound function created by bind",
        body: function() {
            class A {}
            let a=new A();
            
            function f(a,b) { return (++this.val)+a+b; }
            let f_bound=f.bind({val:1},"a");
            
            A.prototype.f_at_prototype = f_bound.toMethod(A.prototype,"b");
            a.f_at_object = f_bound.toMethod(a,"c");
            A.f_static = f_bound.toMethod(A,"d");

            // "tag" each object returned by toMethod()
            A.prototype.f_at_prototype.tag="A.prototype.f_at_prototype";
            a.f_at_object.tag="a.f_at_object";
            A.f_static.tag="A.f_static";
            
            // check the tags to ensure they are all different objects
            assert.areEqual("A.prototype.f_at_prototype", A.prototype.f_at_prototype.tag, "toMethod() should create a new object A.prototype.f_at_prototype");
            assert.areEqual("a.f_at_object", a.f_at_object.tag, "toMethod() should create a new object a.f_at_object");
            assert.areEqual("A.f_static", A.f_static.tag, "toMethod() should create a new object A.f_static");
            
            assert.areEqual("2aundefined", a.f_at_prototype(), "toMethod() applied on bound function & linked to prototype");
            assert.areEqual("3aundefined", a.f_at_object(), "toMethod() applied on bound function & linked to object");
            assert.areEqual("4aundefined", A.f_static(), "toMethod() applied on bound function & linked to class");
        }
    },
    {
        name: "toMethod() should be functional on a cross-thread function",
        body: function() {
            class A {}
            let a=new A();
            
            WScript.RegisterCrossThreadInterfacePS(); 
            let external=WScript.LoadScriptFile("ES6FunctionAPI_helper.js", "crossthread");
            
            A.prototype.f_at_prototype = external.f.toMethod(A.prototype);
            a.f_at_object = external.f.toMethod(a);
            A.f_static = external.f.toMethod(A);

            // "tag" each object returned by toMethod()
            A.prototype.f_at_prototype.tag="A.prototype.f_at_prototype";
            a.f_at_object.tag="a.f_at_object";
            A.f_static.tag="A.f_static";
            
            // check the tags to ensure they are all different objects
            assert.areEqual("A.prototype.f_at_prototype", A.prototype.f_at_prototype.tag, "toMethod() should create a new object A.prototype.f_at_prototype");
            assert.areEqual("a.f_at_object", a.f_at_object.tag, "toMethod() should create a new object a.f_at_object");
            assert.areEqual("A.f_static", A.f_static.tag, "toMethod() should create a new object A.f_static");
            
            assert.areEqual("toMethod xth", a.f_at_prototype("xth"), "toMethod() on cross-thread function & linked to prototype");
            assert.areEqual("toMethod xth", a.f_at_object("xth"), "toMethod() on cross-thread function & linked to object");
            assert.areEqual("toMethod xth", A.f_static("xth"), "toMethod() on cross-thread function & linked to class");
        }
    },
    {
        name: "toMethod() should be functional on a host function",
        body: function() {
            class A {}
            let a=new A();
            
            A.prototype.RegisterCrossThreadInterfacePS = WScript.RegisterCrossThreadInterfacePS.toMethod(A.prototype);
            a.LoadScriptFile = WScript.LoadScriptFile.toMethod(a);            
            
            a.RegisterCrossThreadInterfacePS();
            let external = a.LoadScriptFile("ES6FunctionAPI_helper.js", "crossthread");
            
            assert.areEqual("toMethod host", external.f("host"), "toMethod() on host function");
        }
    },
    {
        name: "toMethod() should expect functionNameId to be either TaggedInt or JavascriptString",
        body: function() {
            // for built-in functions, calling toString() permanently changes this->functionNameId from TaggedInt to JavascriptString
            Math.sin.toString();
            assert.areEqual(0, Math.sin.toMethod({})(0), "toMethod() should function after toString() changes functionNameId to JavascriptString");
        }
    },
    {
        name: "objects returned by toMethod() should show consistent toString() behavior",
        body: function() {
            function testToStringBehavior(f, s)
            {
                // for built-in functions, calling toString() permanently changes this->functionNameId from TaggedInt to JavascriptString
                let newFunc=f.toMethod({});
                assert.areEqual(f.toString(), newFunc.toString(), s+" cloned by toMethod() without prior toString() call should retains its toString() behavior");
                assert.areEqual(f.toString(), f.toMethod({}).toString(), s+" cloned by toMethod() with prior toString() call should retains its toString() behavior");
            }
            testToStringBehavior(Math.sin, "Builtin Math function");
            testToStringBehavior(function(){}, "Script function");
            testToStringBehavior(WScript.Echo, "Host function");
         }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });

