// Tests that shadowing a let/const declaration doesn't AV.
// Bug 346122.

const c = 5;
function c(){}

WScript.Echo("PASSED");