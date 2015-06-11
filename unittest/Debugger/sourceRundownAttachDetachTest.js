// Tests basic attach/detach/attach scenario after a source rundown.
function sourceRundownAttachDetachTest() {
    var a = 0;
    function innerFunc() {
        var b = a;
        b++;    /**bp:locals()**/
    }

    innerFunc();
}

WScript.PerformSourceRundown();
WScript.Attach(sourceRundownAttachDetachTest);
WScript.Detach(sourceRundownAttachDetachTest);
WScript.Attach(sourceRundownAttachDetachTest);

WScript.Echo("PASSED");