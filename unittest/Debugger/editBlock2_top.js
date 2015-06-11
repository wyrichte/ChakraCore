// EnC: new function body accesses block scope vars that didn't exist before change.
//
//  Test block scope at top level.

if (true) {
    /**edit(test)**/
    function foo() {}
    
    /// function Timer(){};
    /// function foo() {
    ///     return Timer;
    /// }
    /**endedit(test)**/

    f = foo;
}

var f;

var stages = ["=== Before change ===", "=== After change ==="], curStage = 0;
function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo(f());
}

test();
WScript.Edit("test", test);