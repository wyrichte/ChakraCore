// Tests that the [Methods] fake node is not
// part of the full name for a node.
// Work Item: 769430

function test() {
    var o = {
        a: function () {
        }
    };

    /**bp:evaluate('o', 2, LOCALS_FULLNAME)**/
}

test();

WScript.Echo("PASSED");