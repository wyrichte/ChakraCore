//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

var passed = true;

function testDateTimeFormatOptions(opts, date, expected) {
    try {
        var options = { ca: "gregory", hour12: true, timeZone: "UTC" };
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

function arrayEqual(arr1, arr2) {
    if (arr1.length !== arr2.length) return false;
    for (var i = 0; i < arr1.length; i++) {
        if (arr1[i] !== arr2[i]) return false;
    }
    return true;
}

function testSupportedLocales(locales, expectedResult) {
    try {
        var actual = Intl.DateTimeFormat.supportedLocalesOf(locales, { localeMatcher: "best fit" });
        if (!arrayEqual(actual, expectedResult)) {
            throw new Error("Calling SupportedLocalesOf on '[" + locales.join(",") + "]' doesn't match expected result '[" + expectedResult.join(",") + "]' when using best fit. Actual:[" + actual.join(",") + "]");
        }
        actual = Intl.DateTimeFormat.supportedLocalesOf(locales, { localeMatcher: "best fit" });
        if (!arrayEqual(actual, expectedResult)) {
            throw new Error("Calling SupportedLocalesOf on '[" + locales.join(",") + "]' doesn't match expected result '[" + expectedResult.join(",") + "]' when using lookup. Actual: [" + actual.join(",") + "]");
        }
    }
    catch (ex) {
        passed = false;
        WScript.Echo("Error testSupportedLocales: " + ex.message);
    }
}
testSupportedLocales(undefined, []);
testSupportedLocales(["en-US"], ["en-US"]);
testSupportedLocales([], []);
testSupportedLocales(["xxx"], []);

testDateTimeFormatOptions({ hour: "numeric" }, new Date(2000, 1, 1), "8 AM");
testDateTimeFormatOptions({ hour: "numeric", minute: "numeric", second: "numeric", hour12: true }, new Date(2000, 1, 1, 1, 1, 1), "9:01:01 AM");
testDateTimeFormatOptions({ hour: "numeric", minute: "numeric", second: "numeric", hour12: false }, new Date(2000, 1, 1, 1, 1, 1), "9:01:01");

testDateTimeFormatOptions({ month: "numeric" }, new Date(2000, 1, 1), "2");
testDateTimeFormatOptions({ month: "short" }, new Date(2000, 1, 1), "Feb");
testDateTimeFormatOptions({ month: "long" }, new Date(2000, 1, 1), "February");
//This test is simply checking if it throws an error. It shouldn't, as it was fixed for bug 315623.
//Otherwise 2 baselines are needed for this test, as Win7 adds a comma, while 8 doesn't.
new Intl.DateTimeFormat("en-US", { month: "numeric", year: "numeric" }).format(new Date());

testDateTimeFormatOptions({ year: "numeric" }, new Date(2000, 1, 1), "2000");
testDateTimeFormatOptions({ year: "2-digit" }, new Date(2000, 1, 1), "00");

testDateTimeFormatOptions({ day: "numeric" }, new Date(2000, 1, 1), "1");

testDateTimeFormatOptions({ weekday: "short" }, new Date(2000, 1, 1), "Tue");
testDateTimeFormatOptions({ weekday: "long" }, new Date(2000, 1, 1), "Tuesday");

testDateTimeFormatOptions({ day: "2-digit", month: "2-digit", year: "2-digit" }, new Date(-59958100000000), "01/01/70");

testDateTimeFormatOptions({ day: "2-digit", month: "2-digit", timeZone: "America/New_York", timeZoneName: "short" }, new Date(-59958100000000), "1/1/70 7:13:20 AM EST");

testDateTimeFormatOptions({ day: "2-digit", month: "2-digit", timeZone: "America/Denver", timeZoneName: "long" }, new Date(-59958100000000), "1/1/70 5:13:20 AM Mountain Standard Time");

//Expect no error here, Blue: 448060
try {
    var test = new Intl.DateTimeFormat(undefined, { minute: "numeric" });
}
catch (e) {
    passed = false;
}

// Read default time zone
try {
    var test = new Intl.DateTimeFormat('en-US', { timeZone: "america/New_york" });
    var options = test.resolvedOptions();
    if (options.timeZone !== "America/New_York") {
        WScript.Echo('Expected : America/New_York but got :' + options.timeZone);
        passed = false;
    }

} catch (e) {
    WScript.Echo(e.message);
    passed = false;
}

// Expected to fail
try {
    var test = new Intl.DateTimeFormat("en-US", { timeZone: "ABC" });
    WScript.Echo('Should have thrown exception.');
    passed = false;
} catch (e) {
}

try {
    var test = new Intl.DateTimeFormat("en-US", { timeZone: "" });
    WScript.Echo('Should have thrown exception.');
    passed = false;
} catch (e) {
}

//Round Tripping of Date and Time
var originalDate = new Date();
var roundTrippedDate = new Date(Date.parse(originalDate.toLocaleString("en-us")));
if (originalDate.toString() !== roundTrippedDate.toString()) {
    WScript.Echo("Date and Time roundtripping doesn't work.");
    passed = false
}

if (passed === true) {
    WScript.Echo("Pass");
}

/*
    locale: this.__locale,
    calendar: this.__calendar, // ca unicode extension
    numberingSystem: this.__numberingSystem, // nu unicode extension
    timeZone: this.__timeZone,
    hour12: this.__hour12,
    weekday: this.__weekday,
    era: this.__era,
    year: this.__year,
    month: this.__month,
    day: this.__day,
    hour: this.__hour,
    minute: this.__minute,
    second: this.__second,
    timeZoneName: this.__timeZoneName
*/
