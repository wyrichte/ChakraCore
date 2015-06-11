// Test EnC at top level
//
// Change a function

/**edit(top)**/
function f() {
    WScript.Echo("old f");
}

/// function f() {
///     WScript.Echo("new f");
/// }
/**endedit(top)**/

var stages = ["=== Before change ===", "=== After change ==="];
var curStage = 0;

function test() {
    WScript.Echo(stages[curStage++]);
    f();
}
test();
WScript.Edit("top", test);
