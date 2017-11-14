/**ondmbp(resume_ignore):stack();locals();dumpBreak();resume('step_over');dumpBreak();resume('step_over');dumpBreak();resume('step_over');dumpBreak();locals()**/
function bar() {
    var j = 4;
    WScript.ChangeDOMElement();
    j++;
    return j;
}

function foo() {
    var i = 3;
    bar();
    var j = i * i;
    return j;
}
foo();
foo();
foo();
foo();
WScript.Echo("PASSED");

