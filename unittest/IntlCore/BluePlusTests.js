//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

var passed = true;

function testDateTimeFormatOptions(opts, date, expected) {
    try {
        var options = { ca: "gregory", hour12: true, timeZone:"UTC" };
        for (option in opts) {
            options[option] = opts[option];
        }
        var actual = new Intl.DateTimeFormat("en-US", options).format(date);
        if (String(actual).localeCompare(expected)) {//This isn't the Intl localeCompare as flag is disable by default.
            passed = false;
            WScript.Echo("ERROR: Formatting '" + date + "' with options '" + JSON.stringify(options) + "' resulted in: '" + actual + "'; expected '" + expected + "'!");
        }
    }
    catch (ex) {
        passed = false;
        WScript.Echo(ex.message);
    }
}

testDateTimeFormatOptions({ timeZoneName: "short" }, new Date(2000, 1, 1), "2/1/2000 8:00:00 AM GMT");
testDateTimeFormatOptions({ timeZoneName: "long" }, new Date(2000, 1, 1), "2/1/2000 8:00:00 AM Greenwich Mean Time");

if (passed === true) {
    WScript.Echo("Pass");
}
