// Test EnC at top level

/**edit(top)**/
function f1() {
    WScript.Echo("old f1");
}

function f2() {
    WScript.Echo("old f2");
}

// Move f1 to bottom

/// function f2() {
///    WScript.Echo("old f2");
/// }
///
/// function f1() {
///     WScript.Echo("old f1");
/// }
/**endedit(top)**/

var stages = ["=== Before change ===", "=== After change ==="];
var curStage = 0;

function test() {
    WScript.Echo(stages[curStage++]);
    f1();
    f2();
}
test();
WScript.Edit("top", test);
