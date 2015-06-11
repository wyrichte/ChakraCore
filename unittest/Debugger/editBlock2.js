// EnC: new function body accesses block scope vars that didn't exist before change.
//

function g() {
    if (true) {
        /**edit(test)**/
        function foo() { }

        /// function Timer(){};
        /// function foo() {
        ///     return Timer;
        /// }
        /**endedit(test)**/

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