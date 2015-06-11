// EnC: Move + Update
//      Move a top-level function to nested

/**edit(test)**/
function f1(a) {
    return a;
}
function foo() {
    var v0, v1, v2;
    return f1 && f1("def");
}

/// function foo() {
///     function f1(a) {
///         return a + a;
///     }
///     var v0, v1, v2;
///     return f1 && f1("def");
/// }
/**endedit(test)**/

var stages = ["=== Before change ===", "=== After change ==="];
var curStage = 0;

function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo(this.f1 && this.f1("abc"));
    WScript.Echo(foo());
}
test();
WScript.Edit("test", test);
