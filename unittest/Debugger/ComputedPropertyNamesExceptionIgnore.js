/**exception(resume_ignore)**/
function f() { return "f"; }
function g() { return "g"; }

var o = {
    a: 1,
    [f()]: 2,
    // This instanceof will throw an error in the middle of our object initializer.
    // The debugger should recover at the next statement, leaving o undefined since
    // the store to o is skipped.
    [1 instanceof null]: 3,
    [g()]: 4,
    b: 5
};

WScript.Echo(o === undefined ? "passed" : "failed");
