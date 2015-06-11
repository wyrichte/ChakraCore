// Tests that let/const variables disappear when a breakpoint is
// set the in the dead zone before the variable is initialized.
function blockScopeDeadZoneTestFunc() {
    var a = 0;
    a; /**bp:locals()**/
    let b = 1;
    const cConst = 2;
    a; /**bp:locals()**/
    {
        a; /**bp:locals()**/
        const dConst = 3;
        let e = 4;
        dConst;
        e;
        b; /**bp:locals()**/
    }
    
    return 0; /**bp:locals()**/
}
blockScopeDeadZoneTestFunc();
WScript.Echo("PASSED");