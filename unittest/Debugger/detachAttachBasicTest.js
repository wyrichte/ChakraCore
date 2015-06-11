// Tests that attaching and detaching works over and over again.
function detachBasicTest() {
    /* This test runs in -auto mode which generates a lot of internal evals from dbgcontroller.js which are
       registered in sourceList and needs to reparsed on attach. To collect those strings calling forcing GC */
    CollectGarbage();
    var a = 0;
    a;
}

for (var i = 0; i < 100; ++i) {
    WScript.Attach(detachBasicTest);
    WScript.Detach(detachBasicTest);
}

WScript.Echo("PASSED");