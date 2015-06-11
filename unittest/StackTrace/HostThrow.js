//
// Test host throw stack
//
WScript.LoadScriptFile("TrimStackTracePath.js");

try {
    (function foo() {
        SCA.serialize(function(){});
    })();
} catch(e) {
    WScript.Echo(TrimStackTracePath(e.stack));
}
