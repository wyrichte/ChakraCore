// include the 'Do()' method
WScript.LoadScriptFile('justMyCode.twoDocuments.simple.do.js');

function Print(str) {
    WScript.Echo("Print() - IsInJitMode() == " +  Debug.isInJit());
    WScript.Echo(str);
};

/**bp:stack(CALLSTACK_DOCUMENTID);resume('step_document');logJson('Do()');stack(CALLSTACK_DOCUMENTID);resume('step_document');logJson('resumed');stack(CALLSTACK_DOCUMENTID)**/
Print('Start!');
Do();
Print('Completed!');
