// Test EnC at top level
//
// Add a new function

/**edit(top)**/
/// function f() {
///     WScript.Echo("new f, add new");
/// }
/**endedit(top)**/

var f = "Something else already exist!";
var stages = ["=== Before change ===", "=== After change ==="];
var curStage = 0;

function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo("f == ", this.f);
}
test();
WScript.Edit("top", test);
