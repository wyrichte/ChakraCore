// Test EnC of nested function
//
// closure var changes name

function foo(a) {
    /**edit(nest)**/
    var x = a;
    return function f() {
        return x;
    };

    /// var y = a;
    /// return function f() {
    ///     return y + y;
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
