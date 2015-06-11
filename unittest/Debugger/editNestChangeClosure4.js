// Test EnC of nested function
//
// Add eval

function foo(a) {
    var x = a;

    /**edit(nest)**/
    /// eval("x = 'newnew'");
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
