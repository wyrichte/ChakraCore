// Test EnC at top level
//
// Delete a function

/**edit(top)**/
function f() {
    WScript.Echo("old f, delete");
}

///  // f deleted
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
}
test();
WScript.Edit("top", test);
