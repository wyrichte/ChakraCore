// Tests that canvas pixel array special non-enumerated
// properties properly display in the locals display
// (input, $_, lastMatch, $Ampersand, lastParen, $Plus, leftContext
// $BackTick, rightContext, $Tick, $1, $2, $3, $4, $5, $6, $7, $8, $9
// index, lastIndex).
// Work Item: 782730
function test() {
    var regex = /.*/;

    // Pass true so that we can see if read only attributes are set as well.
    /**bp:evaluate('regex.constructor', 1, true)**/
}

test();
WScript.Echo("PASSED");