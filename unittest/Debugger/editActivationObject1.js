// EnC: A nest function can directly access ActivationObject closure slots.
// Need to compare ActivationObject closure.

function f() {
    /**edit(test)**/
    var dummy = "ERROR!";
    var x = "ok";    
    /// var x = "ok";
    /// var dummy = "ERROR!";
    /**endedit(test)**/
    
    function f1() { return x;}    
    function f2() { return eval(''); }
    return f1;
}

var f1 = f();

var stages = ["=== Before change ===", "=== After change ==="], curStage = 0;
function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo(f1());
}

test();
WScript.Edit("test", test);
