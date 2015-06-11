// Test EnC at top level
//
// Add a new function

/**edit(top)**/
/// var f = function g() {
///     WScript.Echo("new f, add new");
/// }
/**endedit(top)**/

var stages = ["=== Before change ===", "=== After change ==="];
var curStage = 0;

function test() {
    WScript.Echo(stages[curStage++]);
    if (this.f) {
        this.f();
    } else {
        WScript.Echo("f == ", this.f);
    }
    if (this.g) {
        this.g();
    } else {
        WScript.Echo("g == ", this.g);
    }
}
test();
WScript.Edit("top", test);
