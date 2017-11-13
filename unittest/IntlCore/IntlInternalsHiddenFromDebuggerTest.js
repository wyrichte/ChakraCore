//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

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
