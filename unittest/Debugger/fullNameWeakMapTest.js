// Tests that the [WeakMap] fake node and any nodes below
// it are not part of the full name.
// Work Item: 769430

function test() {
    var weakMap = new WeakMap();
    var o1 = { a: 1 };
    var o2 = { b: 2 };
    var o3 = { c: 3 };
    weakMap.set(o1, 4);
    weakMap.set(o2, 5);
    weakMap.set(o3, 6);

    /**bp:evaluate('weakMap',1, LOCALS_FULLNAME)**/
}

test();

WScript.Echo("PASSED");