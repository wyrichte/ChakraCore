// EnC: Adding try/catch causes a function to skip JIT in debug mode. Need to
// update ScriptFunctionType's entryPointInfo.

function foo() {
    var a = 0;
/**edit(test)**/
/// try {
/// } catch(e) {
/// }
/**endedit(test)**/
}

WScript.Edit("test", foo);
