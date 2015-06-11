// Tests that const variables have the proper
// read-only attribute when run through expression
// evaluation (same code path as datatip and watch
// window).
// Bug #411503.

function test() {
    const c = 5;
    let a = c; /**bp:evaluate('c', 0, LOCALS_ATTRIBUTES)**/
    {
        const d = [1, 2, 3];
        d; /**bp:evaluate('d', 1, LOCALS_ATTRIBUTES)**/

        {
            const e = {p:1, q:2}
            e; /**bp:evaluate('e', 1, LOCALS_ATTRIBUTES)**/
        }
    }
    a++;
}

test();
WScript.Echo("PASSED");