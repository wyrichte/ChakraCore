// Tests that negative properties display their
// full name correctly.
// Work Item: 769430

function test() {
    var arr = [1, 2, 3, 4];
    arr[-1] = "negative property";
    arr[-123456] = -123456;
    arr[-987654] = { a: { b: 1, c: 2 } };
    /**bp:locals(1, LOCALS_FULLNAME);evaluate("arr", 3, LOCALS_FULLNAME)**/

}

test();

WScript.Echo("PASSED");