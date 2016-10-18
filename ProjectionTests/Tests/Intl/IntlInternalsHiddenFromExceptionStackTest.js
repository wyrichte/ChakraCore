// Tests that internal stack frames from Intl are hidden from the debugger when an exception is thrown.
if (this.WScript) { WScript.LoadScriptFile("TrimStackTracePath.js"); }

function testCallback() {
    var array =
    [
        {
            toString: function ()
            {
                throw new Error('Throwing...');
            }
        },
        5
    ]

    var c = new Intl.Collator();
    array.sort(c.compare);
}

try {
    testCallback();
}
catch (ex) {
    WScript.Echo(TrimStackTracePath(ex.stack));
}