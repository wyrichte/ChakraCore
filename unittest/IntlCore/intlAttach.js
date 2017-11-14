//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

var i=0;
function dateToString() {
    var d = new Date();
    var t = d.toLocaleTimeString(); /**bp:stack()**/
    i++;
    if(i == 4)
    {
        WScript.Echo("PASSED");
    }
}
dateToString();
dateToString();
// toLocaleTimeString calls into intl and maxinterpretCount:1 ensures those calls within intl are jitted before attach.
WScript.Attach(dateToString);
WScript.Detach(dateToString);
