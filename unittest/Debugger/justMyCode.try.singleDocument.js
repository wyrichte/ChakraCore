function Print(str) {
    WScript.Echo("Print() - IsInJitMode() == " +  Debug.isInJit());

    try {
        /**bp:logJson('tryBlock');stack(CALLSTACK_FLAGS)**/
        WScript.Echo(str);

        throw "bad but expected!";
    }
    catch (e) {
        /**bp:logJson('exceptionBlock');stack(CALLSTACK_FLAGS)**/
        WScript.Echo("Caught error: " + e);
    }

    /**bp:logJson('normalBlock');stack(CALLSTACK_FLAGS)**/
};

/**bp:logJson('entry');stack(CALLSTACK_FLAGS)**/
Print('Hello world!');
