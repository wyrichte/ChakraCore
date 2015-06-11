// Tests that heap enumeration of block scope slot array
// variables reports properly.
// Bug #219563.

var g;

function test() {
    {
        let a = { p1: 1, p2: 2 };
        {
            g = function inner() {
                a;
            }
        }
    }
}

test();
Debug.dumpHeap(g, true);
WScript.Echo("PASSED");