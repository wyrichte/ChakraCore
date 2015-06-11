// Tests that the [Scope] fake node is not
// part of the full name for a node.
// Work Item: 769430

function test() {
    var a = 0;
    (function test2() {
        var b = 1;
        (function test3() {
            var c = 2;
            a++;
            b++;
            c++; /**bp:locals(1, LOCALS_FULLNAME)**/
        })();
    })();
}

test();

WScript.Echo("PASSED");