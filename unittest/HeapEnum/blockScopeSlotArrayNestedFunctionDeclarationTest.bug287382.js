// Tests that heap enumeration of a slot array nested
// block scope function declaration doesn't cause an
// AV.
// Bug #287382.

var g;

function test() {
    {
        function outer() { };
        {
            g = function inner() {
                outer();
            }
        }
    }
}

test();
Debug.dumpHeap(g, true);
WScript.Echo("PASSED");