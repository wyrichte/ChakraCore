// Tests that string special non-enumerated
// properties properly display in the locals display
// (length).
// Work Item: 782730
function test() {
    var str = new String("123456");

    // Pass true so that we can see if read only attributes are set as well.
    /**bp:evaluate('str', 1, true)**/
}

test();
WScript.Echo("PASSED");