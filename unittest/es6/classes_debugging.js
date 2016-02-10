if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in
  // jc/jshost
  this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
  {
    name: "BLUE: 546995 Debugger: FQN: ES6: Fully qualified name just shows function name for class methods",
    body: function () {
      class OneClass {
          static Inc(a) {
              a++; /**bp:stack()**/
              return a;
          }
          static ["Dec"](a) {
              a--; /**bp:stack()**/
              return a;
          }
          static get One() {
              return 1; /**bp:stack()**/
          }
          static set One(a) {
              a; /**bp:stack()**/
          }
          static get ["Two"]() {
              return 2; /**bp:stack()**/
          }
          static set ["Two"](a) {
              a; /**bp:stack()**/
          }
          Sum(a, b) {
              return a + b; /**bp:stack()**/
          }
          ["Diff"](a,b) {
              return a - b; /**bp:stack()**/
          }
          get three() {
              return 3; /**bp:stack()**/
          }
          set three(a) {
              a; /**bp:stack()**/          
          }
          get ["four"]() {
              return 4; /**bp:stack()**/
          }
          set ["four"](a) {
              a; /**bp:stack()**/          
          }
      }
      var temp = OneClass.Inc(1);
      var temp2 = OneClass.Dec(1);
      temp = OneClass.One;
      OneClass.One = temp;
      temp = OneClass.Two;
      OneClass.Two = temp;
      var x = new OneClass();
      temp = x.Sum(1, 2);
      temp2 = x.Diff(1, 2);
      temp = x.three;
      x.three = temp;
      temp = x.four;
      x.four = temp;
    }
  },
  {
    name: "BLUE: 554109 [Debugger]  - Class Declaration is shown incorrectly in Debugger",
    body: function () {
      class A {
          run() {}
      }
      WScript.Echo('Check'); /**bp:locals()**/
    }
    },
  {
    name: "BLUE 546955: Debugger: FQN: ES6: Constructor for extended class should show extended class name instead of \"constructor\" in stack if explicit constructor is not defined in extended class",
    body: function () {
      class OneClass {
        constructor(a) {
            a++; /**bp:stack();**/
        }
      }
      class TwoClass extends OneClass {
          TwoClassMethod() {}
      }
      var obj = new TwoClass();
    }
  },
  {
    name: "Super evaluation in eval/lambda contexts",
    body: function () {
      class a {
        method() { return "hello world"; }
        static staticMethod() { return "static"; }
      };
      class b extends a {
        method1() {
          // Should give an error, super was not captured
          /**bp:locals();evaluate('super.method()')**/
          return null;
        }
        method2() {
          // No error
          /**bp:locals();evaluate('super.method()')**/
          return super.method();
        }
        method3() {
          // No error
          /**bp:locals();evaluate("super['method']()")**/
          return super.method();
        }
        method4() {
          // Implicit lambda
          /**bp:locals();evaluate('(() => super.method())()')**/
          return super.method();
        }
        method5() {
          // Parent activation object
          /**bp:locals();evaluate('super.method()')**/
          eval("");
          return super.method();
        }
        method6() {
          // Implicit lambda with parent activation object
          /**bp:locals();evaluate('(() => super.method())()')**/
          eval("");
          return super.method();
        }
        method7() {
          // Super captured inside lambda
          /**bp:locals();evaluate('super.method()')**/
          return (x => super.method())();
        }
        method8() {
          // Super captured inside lambda inside eval in the parent
          return (x => {
            /**bp:locals();evaluate('super.method();')**/
            eval("x => eval('super.method')");
          })();
        }
        get propGet() {
          /**bp:locals();evaluate('super.method();')**/
          return super.method();
        }
        set propSet(x) {
          /**bp:locals();evaluate('super.method();')**/
          return super.method();
        }
        static staticMethod() {
          /**bp:stack();locals();evaluate('super.staticMethod();')**/
          return super.staticMethod();
        }
      }

      let instance = new b();

      instance.method1();
      instance.method2();
      instance.method3();
      instance.method4();
      instance.method5();
      instance.method6();
      instance.method7();
      instance.method8();
      instance.propGet;
      instance.propSet = 1;
      b.staticMethod();
    }
  },
  {
    name: "GetDiagValueString on constructor",
    body: function () {
      class a {
        method() { return "hello world"; }
        static staticMethod() { return "static"; }
      };
      class b extends a {
        method1() {
          return null;
        }
        method2() {
          return super.method();
        }
      }
      1; /**bp:locals()**/
    }
  }
];

testRunner.runTests(tests);
