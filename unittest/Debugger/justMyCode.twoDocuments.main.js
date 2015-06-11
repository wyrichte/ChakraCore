// include the 'Do()' method
WScript.LoadScriptFile('justMyCode.twoDocuments.do.js');

function Print(str) {
    WScript.Echo("Print() - IsInJitMode() == " +  Debug.isInJit());
    WScript.Echo(str);
};

Print('Start!');
Do(); /**bp:stack(CALLSTACK_DOCUMENTID);resume('step_document');logJson('Do()');stack(CALLSTACK_DOCUMENTID);resume('step_over');logJson('skip the main document Print call');stack(CALLSTACK_DOCUMENTID);resume('step_document');logJson('back to main()');stack(CALLSTACK_DOCUMENTID)**/
Print('Completed!');
