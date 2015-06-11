// Tests that array buffer special non-enumerated
// properties properly display in the locals display
// (byteLength).
// Work Item: 782730
function test() {
    var arrBuffer = new ArrayBuffer(5);

    // Pass true so that we can see if read only attributes are set as well.
    /**bp:evaluate('arrBuffer', 1, true)**/
}

test();
WScript.Echo("PASSED");