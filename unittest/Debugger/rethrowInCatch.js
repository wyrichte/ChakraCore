
/**exception(resume_ignore):stack();locals()**/
function foo() {
    try {
        throw 1;
    } catch (e) {
        throw e;
    }
}

WScript.Echo("Pass");
foo();

