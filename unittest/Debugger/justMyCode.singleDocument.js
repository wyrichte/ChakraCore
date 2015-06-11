function Do() {
    WScript.Echo("DO() - IsInJitMode() == " +  Debug.isInJit());

    var loopCount = 10;
    for (var i = 0; i < loopCount; i++) {
        Print('Do() with i=' + i);
    }

    Print('Done!');
};

function Print(str) {
    WScript.Echo(str);
};

Print('Start!');
/* only one callstack should be reported, as the resume will run to the end */
Do(); /**bp:stack(CALLSTACK_DOCUMENTID);resume('step_document');stack(CALLSTACK_DOCUMENTID)**/
Print('Completed!');
