//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// Tests that internal stack frames from Intl are hidden from the debugger user
// and internal code is interlaced.
function testCallback() {
    var array =
    [
        {
            toString: function ()
            {
                var array2 = 
                    [
                        {
                            toString: function ()
                            {
                                return 'bar'; /**bp:stack()**/
                            }
                        },
                        5
                    ]

                var c2 = new Intl.Collator();
                array2.sort(c2.compare);
                return 'foo';
            }
        },
        5
    ]

    var c = new Intl.Collator();
    array.sort(c.compare);
}

testCallback();
WScript.Echo("PASSED");
