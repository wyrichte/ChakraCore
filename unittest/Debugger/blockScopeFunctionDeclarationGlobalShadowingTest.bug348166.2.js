// Tests that shadowing a let/const declaration doesn't AV.
// Bug 346122.

function test(){
    const a =  0;
    function a(){ }
}
test();

WScript.Echo("PASSED");