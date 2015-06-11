// include the 'Do()' method
WScript.LoadScriptFile('justMyCode.try.twoDocuments.do.js');

function Print(str, i) {
    WScript.Echo("Print() - IsInJitMode() == " +  Debug.isInJit());

    /* CALLSTACK entry point #2, #4 */
    WScript.Echo(str);

    if (i === 0) {
        throw "throw at 1st iteration!";
    }
};

Print('Start!');
/* CALLSTACK entry point #0 */
Do(2); /**bp:logJson("CS#0: entry");stack(CALLSTACK_DOCUMENTID | CALLSTACK_FLAGS);resume('step_document');logJson("CS#1: 2nd level - i=0");stack(CALLSTACK_DOCUMENTID | CALLSTACK_FLAGS);resume('step_document');logJson("CS#2: back to main, print i==0, will throw - in a try block");stack(CALLSTACK_DOCUMENTID | CALLSTACK_FLAGS);resume('step_document')**/
/* CALLSTACK entry point #6 */
Print('Completed!');

