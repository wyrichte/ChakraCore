if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in
  // jc/jshost
  this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
  {
    name: "Classes hybrid debugging sanity checks",
    body: function () {
      class a {
        method() {
          /**bp:stack();locals()**/
          return "hello world";
        }
        
        static staticMethod() {
          /**bp:stack();locals()**/
          return "static";
        }
      };
      
      class b extends a {
        method() {
          /**bp:stack();locals()**/
          return super.method();
        }
        method2() {
          return (() => {
            /**bp:stack();locals()**/
            super.method();
            })();
        }
        method3() {
          eval("");
          /**bp:stack();locals()**/
          return super.method();
        }
        
        static staticMethod() {
          /**bp:stack();locals()**/
          return super.staticMethod();
        }
      };
      
      let classb = new b();
      classb.method();
      classb.method2();
      classb.method3();
      b.staticMethod();
      /**bp:locals()**/
    }
  }
];

testRunner.runTests(tests);