// Test EnC of nested function
//
// Change a nested function, no closure impact

function foo(a) {
    var x = a;

    /**edit(nest)**/
    return function f() {
        return x;
    };

    /// return function f() {
    ///     return x + x;
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
