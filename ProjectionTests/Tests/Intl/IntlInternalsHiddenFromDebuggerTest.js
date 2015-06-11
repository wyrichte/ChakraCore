// Tests that internal stack frames from Intl are hidden from the debugger when the callstack is shown.
function testCallback() {
    var array =
    [
        {
            toString: function ()
            {
                return 'foo'; /**bp:stack()**/
            }
        },
        5
    ]

    var c = new Intl.Collator();
    array.sort(c.compare);
}

testCallback();
WScript.Echo("PASSED");