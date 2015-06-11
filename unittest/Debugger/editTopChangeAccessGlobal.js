// Test EnC at top level
//
// Change a function and access global property (regression test for missing inline cache entry in ScriptFunctionWithInlineCache)

/**edit(top)**/
function f() {
    WScript.Echo("old f");
}
/// function f() {
///     WScript.Echo(foo());
/// }
/// function foo()
/// {
///     return "new f"
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
