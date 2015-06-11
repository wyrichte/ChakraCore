//
// test global func
//

var g0 = 10;
/**bp:locals(1)**/

(function foo() {
    eval('var x0 = 1;\n  x0;foo;/**bp:locals(1)**/');
}).apply({});

(function foo() {
    "use strict";
    eval('var x0 = 2;\n  x0;foo;/**bp:locals(1)**/');
})();

WScript.Echo("pass");
