function Do() {
    WScript.Echo("DO() - IsInJitMode() == " +  Debug.isInJit());
    Print('foo to inner document!');
    WScript.Echo("/DO() - IsInJitMode() == " +  Debug.isInJit());
};

