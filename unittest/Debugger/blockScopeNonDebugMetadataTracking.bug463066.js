// Tests that updating of debugger metadata bytecode offsets still occurs
// and doesn't assert when specifying the -Debug jshost flag.

function test() {
    let a = 0;
    function inner() {
        a++;
    }
    inner();
}

test();

WScript.Echo("PASSED");