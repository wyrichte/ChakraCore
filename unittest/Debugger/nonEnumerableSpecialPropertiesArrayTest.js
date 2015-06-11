// Tests that array special non-enumerated
// properties properly display in the locals display
// (length).
// Work Item: 782730
function test() {
    var arr = [1, 2, 3, 4, 5];

    // Pass true so that we can see if the length is writeable.
    /**bp:evaluate('arr', 1, true)**/
}

test();
WScript.Echo("PASSED");