// EnC: Reorder + Update
//

/**edit(test)**/
function f1(a) {
    return a;
}
function f2() {}

/// function f2() {}
/// function f1(a) {
///     return a + a;
/// }
/**endedit(test)**/

var stages = ["=== Before change ===", "=== After change ==="];
var curStage = 0;

function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo(f1("abc"));
}
test();
WScript.Edit("test", test);
