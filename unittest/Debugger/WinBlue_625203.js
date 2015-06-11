// This is a issue of setting 2 breakpoints at same line
// and on hitting first breakpoint scriptengine is closed. 
// ProbeList contains 2 breakpoints and runs a loop, 
// on processing the second bp it doesn't check for haltCallbackProbe (script engine) closed state

function test0() {
    var arrObj0 = {};
    var func2 = function () {
        e = arrObj0.prop0;/**bp(loc0):evaluate('WScript.Shutdown()');**/
    }
    arrObj0.length = 1;/**bp(loc1):enableBp('loc0');**/
    func2();
};
test0();