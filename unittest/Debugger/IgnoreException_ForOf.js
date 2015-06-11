/**exception(resume_ignore):stack();**/

// verify in function
var fncPass = false;
function test0() {
    for (var i of nonExistVar) { // throw right before iterator temporary register is initialized.
        WScript.Echo("Fail function in loop");
        return; // debugger won't enter the loop body
    }
    // debugger should resume here (that is, it should not give up and bail to the caller)
    fncPass = true;
};
test0();


// verify at global scope
for (var i of nonExistVar) {
    WScript.Echo("Fail global in loop");
}

WScript.Echo(fncPass ? "PASS" : "Failed function test");
