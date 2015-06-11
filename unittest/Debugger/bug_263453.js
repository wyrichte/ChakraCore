/**exception(resume_ignore):stack();resume('step_into');**/
var __count = 1;

function foo() {
    __count++;
    if (__count > 5) return;
    this.prop0 = 2;
}

function bar() {
    Object.defineProperty(foo, "prop0", {
        get : function () { return 0 },
        configurable : false,
    });
    var fooInstance = new foo();
    nonExistFunction(fooInstance.nonExistProp);
}

bar();
WScript.Echo("PASS");

// -debuglaunch -targeted -forcenative -force:fieldcopyprop
