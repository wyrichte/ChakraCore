// EnC: A nest function can directly access ActivationObject closure slots.
// Need to compare ActivationObject closure.
//      If there is no change though, we should be able to update.

function f() {
    var x = "ok";
    
    /**edit(test)**/
    function f1() { return x;}
    /// function f1() { return x + x;}
    /**endedit(test)**/
   
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
