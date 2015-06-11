// Test EnC of nested function
//
// Change a nested function, add closure var

function foo(a) {
    /**edit(nest)**/
    return function f() {
        return "old";
    };

    /// var x = a;
    /// return function f() {
    ///     return x;
    /// };
    /**endedit(nest)**/
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
