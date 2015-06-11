// Tests that let/const variables appear properly as part of a for loop.
function blockScopeForTestFunc() {
    ; /**bp:locals()**/
    for (let i = 0; i < 1; ++i)
    {
        const innerConst = 2;
        i; /**bp:locals()**/
    }
    return 0; /**bp:locals()**/
}
blockScopeForTestFunc();
WScript.Echo("PASSED");