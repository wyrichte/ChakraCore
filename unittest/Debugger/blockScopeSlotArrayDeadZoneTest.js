// Tests that let/const variables display properly when leaving a dead zone in the slot array case.
function blockScopeSlotArrayDeadZoneTest() {
    var a = 0; /**bp:locals()**/
    let b = 1; /**bp:locals()**/
    const c = 2; /**bp:locals()**/
    a; /**bp:locals()**/
    function inner() {
        b;
        c; /**bp:locals()**/
    }
    inner();
}

blockScopeSlotArrayDeadZoneTest();
WScript.Echo("PASSED");