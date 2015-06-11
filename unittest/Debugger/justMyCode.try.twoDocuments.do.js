function Do(loopCount) {
    WScript.Echo("DO() - IsInJitMode() == " +  Debug.isInJit());

    /* CALLSTACK entry point #1 */
    loopCount = loopCount || 10;

    for (var i = 0; i < loopCount; i++) {
        try
        {
            Print('Do() with i=' + i, i);
            /* CALLSTACK entry point #5 */
        }
        catch (e) {
            /* CALLSTACK entry point #3 */
            /**bp:logJson('CS#3: exceptionBlock');stack(CALLSTACK_DOCUMENTID | CALLSTACK_FLAGS);resume('step_document');logJson('CS#4: print with 2nd i');stack(CALLSTACK_DOCUMENTID | CALLSTACK_FLAGS);resume('step_document');logJson('CS#5: end do()');stack(CALLSTACK_DOCUMENTID | CALLSTACK_FLAGS);resume('step_document');logJson('CS#6: back to main');stack(CALLSTACK_DOCUMENTID | CALLSTACK_FLAGS);resume('step_document');logJson('end!')**/
            WScript.Echo("Caught error: " + e);
        }
    }
};
