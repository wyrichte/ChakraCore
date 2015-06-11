// Tests that bug 56025 is fixed in the nested funcexprscope case.
// [fuzz]:AV@JSCRIPT9TEST!Js::JavascriptLibrary::GetScriptContext+c [d:\7757\inetcore\jscript\lib\runtime\library\javascriptlibrary.h @ 288]
// http://bugcheck/bugs/WindowsBlueBugs/56026

try {
    (function TestFunc() {
        var a;
        (function outer() {
            (function inner() { a; })();
            with ({}) {
                (function innerOuter() {
                    (function innerInner() { a; })();
                    with ({}) {
                        outer();
                    }
                })();
            }
        })();
    })();
}
catch (ex) {
    if (ex.message == "Out of stack space") {
        WScript.Echo("PASSED");
    }
}
