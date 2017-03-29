//
// Test host throw stack
//
WScript.LoadScriptFile("../../core/test/UnitTestFramework/TrimStackTracePath.js");

try {
    (function foo() {
        SCA.serialize(function(){});
    })();
} catch(e) {
    WScript.Echo(TrimStackTracePath(e.stack));
}
