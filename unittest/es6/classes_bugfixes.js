if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in
  // jc/jshost
  this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
  {
    name: "BLUE: 516448 Deleting properties which references to super should be a ReferenceError",
    body: function () {
      class a {
        static b(){ }
      };

      class b extends a {
        static test(){
          super.b();
        }
      };

      assert.throws(function () { delete b.test; }, ReferenceError);
    }
  },
  {
    name: "BLUE 540289: AV on deferred parse of first class method",
    body: function () {
      assert.throws(function() { eval("function f() { var o = { \"a\": class { \"b\""); }, SyntaxError);
    }
  },
  {
    name: "BLUE 558906: [ES6][Class] get and set should be valid method names",
    body: function () {
      class foo {
        set(key) { } // No error
        get() { }    // No error
      }
    }
  },
  {
    name: "BLUE 573391: Classes extending null with a non-default constructor crash",
    body: function () {
      class A { constructor() { } };
      var test1 = class { constructor(args) { } };
      var test2 = class extends null { constructor(args) { } };
      var test3 = class extends A { constructor(args) { } };
      var test4 = class extends A { constructor(args) { super(args) } };
    }
  },
  {
    name: "BLUE 603997: Method formals redeclaration error",
    body: function() {
      assert.throws(function() { eval("class { method(a) { var a; }; }"); },           SyntaxError, "Method formal parameters cannot be redeclared.");
      assert.throws(function() { eval("class { method(a) { let a; }; }"); },           SyntaxError, "Method formal parameters cannot be redeclared.");
      assert.throws(function() { eval("class { method(a) { const a; }; }"); },         SyntaxError, "Method formal parameters cannot be redeclared.");
      
      assert.throws(function() { eval("class { method(a,b,c) { var b; }; }"); },       SyntaxError, "Method formal parameters cannot be redeclared.");
      assert.throws(function() { eval("class { method(a,b,c) { let b; }; }"); },       SyntaxError, "Method formal parameters cannot be redeclared.");
      assert.throws(function() { eval("class { method(a,b,c) { const b; }; }"); },     SyntaxError, "Method formal parameters cannot be redeclared.");
      
      assert.throws(function() { eval("class { set method(a) { var a; }; }"); },       SyntaxError, "Method formal parameters cannot be redeclared.");
      assert.throws(function() { eval("class { set method(a) { let a; }; }"); },       SyntaxError, "Method formal parameters cannot be redeclared.");
      assert.throws(function() { eval("class { set method(a) { const a; }; }"); },     SyntaxError, "Method formal parameters cannot be redeclared.");
      
      assert.throws(function() { eval("class { set method(a,b,c) { var b; }; }"); },   SyntaxError, "Method formal parameters cannot be redeclared.");
      assert.throws(function() { eval("class { set method(a,b,c) { let b; }; }"); },   SyntaxError, "Method formal parameters cannot be redeclared.");
      assert.throws(function() { eval("class { set method(a,b,c) { const b; }; }"); }, SyntaxError, "Method formal parameters cannot be redeclared.");
      
      assert.throws(function() { eval("class { method(a,a,c) { }; }"); },              SyntaxError, "Method formal parameters cannot be redeclared.");
      assert.throws(function() { eval("class { set method(a,a,c) { }; }"); },          SyntaxError, "Method formal parameters cannot be redeclared.");
    }
  },
  {
    name: "BLUE 629214: Class methods with a prefix crash in deferred parse",
    body: function () {
      function test1() { class a { static "a"() { } } }
      function test2() { class a { static get "a"() { } } }
      function test3() { class a { static set "a"(x) { } } }
      function test4() { class a { get "a"() { } } }
      function test5() { class a { set "a"(x) { } } }
      function test6() { class a { *"a"(x) { } } }
      function test7() { class a { method() {} "a"() {} } }
      function test8() { class a { method() {} static "a"() { } } }
      function test9() { class a { method() {} static get "a"() { } } }
      function test10() { class a { method() {} static set "a"(x) { } } }
      function test11() { class a { method() {} get "a"() { } } }
      function test12() { class a { method() {} set "a"(x) { } } }
      function test13() { class a { method() {} *"a"(x) { } } }
    }
  },
  {
    name: "OS 102456: Assert when deleting a non-method property from a class",
    body: function () {
      u3056 = function() {};
      class eval extends u3056 {};
      eval.y = "str";
      delete eval.x;
      delete eval.y;
    }
  },
  {
    name: "OS 112054, 206284: A class named arguments allows methods to create an arguments object",
    body: function () {
      function foo() {
        class arguments { };
        return arguments;
      }

      function bar() {
        let arguments = 5;
        return arguments;
      }
      
      for (let d in [(Math.atan()), arguments.caller, (Math.atan())]) {
        class arguments {}
      }

      assert.areEqual("constructor() {}", foo(1,2)+"", "A class named arguments overrides the arguments object")
      assert.areEqual(5,                    bar(1,2,3),  "A let binding named arguments overrides the arguments object");
    }
  },
  {
    name: "OS 112921: Nested evals attempt to load super into a scope slot",
    body: function () {
      class z{window(){((eval("")))((this))}};
      eval("class z{window(){((eval(\"\")))((this))}};");
    }
  },
  {
    name: "OS 101184: Class methods without separators inside an array break deferred parsing heuristics",
    body: function () {
      Function("[class z{\u3056(){}functional(){}}]");
    }
  },
  {
    name: "OS 182090: Class method after a semicolon terminated method does not force PID",
    body: function () {
      z = (class {
              if (shouldBailout) { /*bLoop*/ };
              "" (x) {}
              })
    }
  },
  {
    name: "OS 257621: Class expressions should not have trailing call parens",
    body: function () {
      assert.throws(function () { eval('class{}();'); }, SyntaxError, "Class expressions cannot be called without parens", "Expected identifier");
      assert.doesNotThrow(function () { eval('new (class {})();'); }, "Parenthesized class expressions can be called");
    }
  },
  {
    name: "OS 863801: Class expression named arguments incorrectly overrides arguments object",
    body: function () {
      function test() {
        assert.areEqual([1, 2, undefined], [arguments[0], arguments[1], arguments[2]], "Arguments object is available");
        var x = class arguments {};
      }
      test(1, 2);
    }
  },
  {
    name: "OS 1114090: Home object should remain unchanged with 'super' call",
    body: function () {
        class Person {
            getFullName() {
                return this.firstName + " " + this.lastName;
            }
            get fullName() {
                return this.firstName + " " + this.lastName;
            }
        }
        class MedicalWorker extends Person { } // to show it works through inheritance chain
        class Doctor extends MedicalWorker {
            constructor(firstName,lastName) {
                super();
                this.firstName = firstName;
                this.lastName = lastName;
            }
            getFullNameExplicit() { return "Dr. " + super.getFullName(); }
            getFullNameProperty() { return "Dr. " + super.fullName; }
            getFullNameEval() { return "Dr. " + super.getFullNameEval(); }
            getFullNameEvalCall() { return "Dr. " + eval('super.getFullName()'); }
            getFullNameEvalProperty() { return "Dr. " + eval('super.fullName'); }
            getFullNameLambdaCall() { return "Dr. " + (()=>super.getFullName()) (); }
            getFullNameLambdaProperty() { return "Dr. " + (()=>super.fullName) (); }
        }
        let x = new Doctor("John","Smith");
        assert.areEqual("Dr. John Smith", x.getFullNameExplicit(), "explicit super call should use subclass as home object");
        assert.areEqual("Dr. John Smith", x.getFullNameProperty(), "property accessor in superclass should use subclass as home object");
        assert.areEqual("Dr. John Smith", x.getFullNameEvalCall(), "super called from within eval should have same behavior as outside of eval");
        assert.areEqual("Dr. John Smith", x.getFullNameEvalProperty(), "super object property access from within eval should have same behavior as outside of eval");
        assert.areEqual("Dr. John Smith", x.getFullNameLambdaCall(), "super called from within lambda should have same behavior as outside of lambda");
        assert.areEqual("Dr. John Smith", x.getFullNameLambdaProperty(), "super object property access from within lambda should have same behavior as outside of lambda");
    }
  },
  {
    name: "OS 1001915: Function built-in properties \'length\', \'caller\', \'arguments\' should not shadow class members",
    body: function () {
        class A {
            static length() { }
            static caller() { }
            static arguments() { }
        };
        assert.areEqual("length() { }", A.length.toString(), "Accessing static method \'length\'");
        assert.areEqual("caller() { }", A.caller.toString(), "Accessing static method \'caller\'");
        assert.areEqual("arguments() { }", A.arguments.toString(), "Accessing static method \'arguments\'");
        for (var p in A) {
            assert.areEqual(p+"() { }", A[p].toString(), "PropertyString for \'"+p+"\' should have a matching cached value");
        }

        class B {
            set length(a) { this._length=a; }
            get length() { return this._length; }            
            set caller(a) { this._caller=a; }
            get caller() { return this._caller; }          
            set arguments(a) { this._arguments=a; }
            get arguments() { return this._arguments; }            
        };
        var b=new B();
        b.length=100;
        b.caller="Caller";
        b.arguments=function() { };
        assert.areEqual(100, b.length, "Get/set accessor \'length\'");
        assert.areEqual("Caller", b.caller, "Get/set accessor \'caller\'");
        assert.areEqual("function () { }", b.arguments.toString(), "Get/set accessor \'arguments\'");
    }
  },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });

// BLUE 516429 at global scope
class a {};
a = null; // No error

// OS 257621 at global scope
assert.doesNotThrow(function () { eval('new (class {})();'); }, "Parenthesized class expressions can be new'd");
