// Smoke test to verify that generator functions successfully execute when
// they are deferred parsed and deferred deserialized.
var passed = true;

function validateValue(ir, ev) {
    if (ir.value !== ev) {
        WScript.Echo("FAILED: Expected '" + ev + "' but got '" + ir.value + "'");
        passed = false;
    }
}

function test() {
    function* gf() {
        yield 1;
        yield 2;
        yield 3;
        return null;
    }

    var g = gf();

    validateValue(g.next(), 1);
    validateValue(g.next(), 2);
    validateValue(g.next(), 3);
    validateValue(g.next(), null);
}

test();

if (passed) {
    WScript.Echo("PASSED");
}
