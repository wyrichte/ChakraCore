// Tests that bug 56025 is fixed in the try/catch/with funcexprscope case.
// [fuzz]:AV@JSCRIPT9TEST!Js::JavascriptLibrary::GetScriptContext+c [d:\7757\inetcore\jscript\lib\runtime\library\javascriptlibrary.h @ 288]
// http://bugcheck/bugs/WindowsBlueBugs/56026

try {
    (function TestFunc() {
        var a;
        (function outer() {
            (function inner() { a; })();
            try {
                throw "Exception";
            }
            catch (ex) {
                with ({}) { outer(); }
            }
        })();
    })();
}
catch (ex) {
    if (ex.message == "Out of stack space") {
        WScript.Echo("PASSED");
    }
}
