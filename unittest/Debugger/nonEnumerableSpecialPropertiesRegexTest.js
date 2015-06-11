// Tests that regular expression special non-enumerated
// properties properly display in the locals display
// (lastIndex, global, multiline, ignoreCase, source, options).
// Work Item: 782730
function test() {
    var regex = new RegExp(".\\s?");
    var regex2 = /.\s?/;

    // Pass true so that we can see if read only attributes are set as well.
    /**bp:evaluate('regex', 1, true)**/
    /**bp:evaluate('regex2', 1, true)**/
}

test();
WScript.Echo("PASSED");