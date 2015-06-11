// Tests that the [Set] fake node and any nodes below
// it are not part of the full name.
// Work Item: 769430

function test() {
    var set = new Set();
    set.add(1);
    set.add(2);
    set.add(3);

    /**bp:evaluate('set',5, LOCALS_FULLNAME)**/
}

test();

WScript.Echo("PASSED");