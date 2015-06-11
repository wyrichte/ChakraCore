// EnC: block scope changed. "Timer" is at a different slot after change.
//
//  Test block scope at top level.

if (true) {
    /**edit(test)**/
    /// function dummy(){};
    /// function dummy2() { return dummy; }
    /**endedit(test)**/
    
    function Timer(){};
    function foo() {
        return Timer;
    }

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