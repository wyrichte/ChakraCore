// EnC: block scope changed. "Timer" is at a different slot after change.
//

function g() {
    if (true) {
        /**edit(test)**/
        /// function dummy(){};
        /// function dummy2() { return dummy; }
        /**endedit(test)**/

        function Timer() { };
        function foo() {
            return Timer;
        }

        return foo;
    }
}

var f = g();

var stages = ["=== Before change ===", "=== After change ==="], curStage = 0;
function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo(f());
}

test();
WScript.Edit("test", test);