// Test EnC of nested function
//
// Add a closure var

var x = "old";

function foo(a) {
    /**edit(nest)**/
    /// var x = a;
    /**endedit(nest)**/
        
    return function f() {
        return x;
    };
}

var f0 = foo("old");


var stages = ["=== Before change ===", "=== After change ==="];
var curStage = 0;

function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo(f0(), foo("new")());
}
test();
WScript.Edit("nest", test);
