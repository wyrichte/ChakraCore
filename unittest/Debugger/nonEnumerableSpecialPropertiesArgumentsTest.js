// Tests that arguments special non-enumerated
// properties properly display in the locals display
// (caller).
// Work Item: 782730
function test(a, b, c) {
    /**bp:evaluate('arguments', 1, LOCALS_ATTRIBUTES)**/
}

test(1, 2, 3);
WScript.Echo("PASSED");
