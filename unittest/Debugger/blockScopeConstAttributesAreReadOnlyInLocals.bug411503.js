// Tests that const variables have the proper
// read-only attribute when run through locals
// window dumping.
// Bug #411503.

function test() {
    const c = 5;
    let a = c; /**bp:locals(0, LOCALS_ATTRIBUTES)**/
    {
        const d = [1, 2, 3];
        d; /**bp:locals(1, LOCALS_ATTRIBUTES)**/

        {
            const e = {p:1, q:2}
            e; /**bp:locals(1, LOCALS_ATTRIBUTES)**/
        }
    }
    a++;
}

test();
WScript.Echo("PASSED");