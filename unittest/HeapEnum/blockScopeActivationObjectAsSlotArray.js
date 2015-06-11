var g;

function test() {
    function nestedEval() { eval(""); } // eval here makes test's scope a non-dynamic object scope

    let a = { p1: 1, p2: 2 };
    {
        g = function inner() {
            a;
        }
    }
}

test();
Debug.dumpHeap(g, true);
WScript.Echo("PASSED");

