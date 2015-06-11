// include the 'Do()' method
WScript.LoadScriptFile('justMyCode.twoDocuments.eval.do.js');

function Print(str) {
    WScript.Echo("Print() - IsInJitMode() == " +  Debug.isInJit());
    WScript.Echo(str);
};

/**bp:stack(CALLSTACK_DOCUMENTID);resume('step_document');logJson('Do()');stack(CALLSTACK_DOCUMENTID);resume('step_document');logJson('resumed_eval');stack(CALLSTACK_DOCUMENTID);resume('step_document');logJson('resumed_eval2');stack(CALLSTACK_DOCUMENTID);resume('step_document');logJson('resumed_print');stack(CALLSTACK_DOCUMENTID)**/
Print('Start!');
Do();
Print('Completed!');
