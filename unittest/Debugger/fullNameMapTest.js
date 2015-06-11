// Tests that the [Map] fake node and any nodes below
// it are not part of the full name.
// Work Item: 769430

function test() {
    var map = new Map();
    var o = { a: 1 }
    map.set(o, 1);
    map.set(2, 3);

    /**bp:evaluate('map',5, LOCALS_FULLNAME)**/
}

test();

WScript.Echo("PASSED");