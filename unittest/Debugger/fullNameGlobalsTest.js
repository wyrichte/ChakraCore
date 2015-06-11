// Tests that the [Globals] fake node is not
// part of the full name for a node, but is shown
// when it is the only node at the root.
// Work Item: 769430

var g = { a: 1, b: 2 };

function test() {
    /**bp:locals(1, LOCALS_FULLNAME)**/
}

test();

WScript.Echo("PASSED");