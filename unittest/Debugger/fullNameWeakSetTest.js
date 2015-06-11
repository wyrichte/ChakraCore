// Tests that the [WeakSet] fake node and any nodes below
// it are not part of the full name.

function test() {
    var weakSet = new WeakSet();
    var o1 = { a: 1 };
    var o2 = { b: 2 };
    var o3 = { c: 3 };
    weakSet.add(o1);
    weakSet.add(o2);
    weakSet.add(o3);

    /**bp:evaluate('weakSet',1, LOCALS_FULLNAME)**/
}

test();

WScript.Echo("PASSED");
