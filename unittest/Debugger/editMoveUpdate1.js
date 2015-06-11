// EnC: Move + Update
//      Move a nested function to top-level

/**edit(test)**/
function foo() {
    function f1(a) {
        return a;
    }
    function f2() {}
    var v0, v1, v2;
}

/// function f1(a) {
///     return a + a;
/// }
/// function foo() {
///     function f2() { return v0;}
///     var v0, v1, v2;
/// }
/**endedit(test)**/

var stages = ["=== Before change ===", "=== After change ==="];
var curStage = 0;

function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo(this.f1 && this.f1("abc"));
}
test();
WScript.Edit("test", test);
