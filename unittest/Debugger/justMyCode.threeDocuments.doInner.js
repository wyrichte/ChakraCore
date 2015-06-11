function Print(str) {
    WScript.Echo("Print() - IsInJitMode() == " +  Debug.isInJit());
    PrintInner(str);
    WScript.Echo("/Print() - IsInJitMode() == " +  Debug.isInJit());
};

function PrintInner(str) {
    WScript.Echo("PrintInner() - IsInJitMode() == " +  Debug.isInJit());
    /**bp:logJson('InInner()');stack(CALLSTACK_DOCUMENTID);resume('step_document');
          logJson('InOuter()');stack(CALLSTACK_DOCUMENTID)**/
    WScript.Echo(str);
    WScript.Echo("/PrintInner() - IsInJitMode() == " +  Debug.isInJit());
};
