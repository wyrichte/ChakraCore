// Tests that bug 56025 is fixed in the minimal repro case with "with" object property usage.
// [fuzz]:AV@JSCRIPT9TEST!Js::JavascriptLibrary::GetScriptContext+c [d:\7757\inetcore\jscript\lib\runtime\library\javascriptlibrary.h @ 288]
// http://bugcheck/bugs/WindowsBlueBugs/56026

try {
    (function TestFunc() {
        var a;
        (function outer() {
            (function inner() { a; })();
            var o = { p1: 1 }
            with (o) {
                outer();
                p1++;
            }
        })();
    })();
}
catch (ex) {
    if (ex.message == "Out of stack space") {
        WScript.Echo("PASSED");
    }
}
