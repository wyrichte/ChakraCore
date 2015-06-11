// Tests that bug 56025 is fixed.
// [fuzz]:AV@JSCRIPT9TEST!Js::JavascriptLibrary::GetScriptContext+c [d:\7757\inetcore\jscript\lib\runtime\library\javascriptlibrary.h @ 288]
// http://bugcheck/bugs/WindowsBlueBugs/56026

try {
    (function () {
        with ({}) with ({}) with ({}) {
            for (eval("a = 0");
              a < 1;
              function x() {
                  with ({}) with ({}) with ({}) (function y() { new Function })();
                  with ({}) x();
              } ()
            ) {
            }
        }
    })();
}
catch (ex) {
    if (ex.message == "Out of stack space") {
        WScript.Echo("PASSED");
    }
}
