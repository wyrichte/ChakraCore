function Do() {
    WScript.Echo("DO() - IsInJitMode() == " +  Debug.isInJit());
    eval("eval(\"Print('foo callback to main document!');\");");
};

