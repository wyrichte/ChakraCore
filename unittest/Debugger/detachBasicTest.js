// Tests that detaching works after attach.
function detachBasicTest() {
    var a = 0;
    a; /**bp:locals(1)**/
}

WScript.Attach(detachBasicTest);
WScript.Detach(detachBasicTest);
WScript.Echo("PASSED");