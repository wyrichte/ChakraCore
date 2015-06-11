// Tests that function special non-enumerated
// properties properly display in the locals display
// (caller, arguments).
// Work Item: 782730
function test() {
    function inner(a, b, c) {
        a++;
        var f = inner;

        /**bp:evaluate('f', 2, LOCALS_ATTRIBUTES)**/
    }

    inner(1, 2, 3);
}

test();
WScript.Echo("PASSED");