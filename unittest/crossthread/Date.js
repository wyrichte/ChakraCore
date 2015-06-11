WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");
WScript.LoadScriptFile("testObjects.js");

WScript.Echo(x.testDate.getDate());
WScript.Echo(x.testDate.getDay());
WScript.Echo(x.testDate.getFullYear());
WScript.Echo(x.testDate.getHours());
WScript.Echo(x.testDate.getMilliseconds());
WScript.Echo(x.testDate.getMinutes());
WScript.Echo(x.testDate.getMonth());
WScript.Echo(x.testDate.getSeconds());
WScript.Echo(x.testDate.getTime());
WScript.Echo(x.testDate.getTimezoneOffset());
WScript.Echo(x.testDate.getUTCDate());
WScript.Echo(x.testDate.getUTCDay());
WScript.Echo(x.testDate.getUTCFullYear());
WScript.Echo(x.testDate.getUTCHours());
WScript.Echo(x.testDate.getUTCMilliseconds());
WScript.Echo(x.testDate.getUTCMinutes());
WScript.Echo(x.testDate.getUTCMonth());
WScript.Echo(x.testDate.getUTCSeconds());
WScript.Echo(x.testDate.getVarDate());
WScript.Echo(x.testDate.getYear());
WScript.Echo(x.testDate.toDateString());
WScript.Echo(x.testDate.toISOString());
WScript.Echo(x.testDate.toJSON());
WScript.Echo(x.testDate.toLocaleDateString());
WScript.Echo(x.testDate.toLocaleString());
WScript.Echo(x.testDate.toLocaleTimeString());
WScript.Echo(x.testDate.toString());
WScript.Echo(x.testDate.toTimeString());
WScript.Echo(x.testDate.toUTCString());
WScript.Echo(x.testDate.valueOf());
WScript.Echo(x.testDate.valueOf() == x.testDate.getTime());

WScript.Echo();

WScript.Echo(Date.prototype.getDate.call(x.testDate));
WScript.Echo(Date.prototype.getDay.call(x.testDate));
WScript.Echo(Date.prototype.getFullYear.call(x.testDate));
WScript.Echo(Date.prototype.getHours.call(x.testDate));
WScript.Echo(Date.prototype.getMilliseconds.call(x.testDate));
WScript.Echo(Date.prototype.getMinutes.call(x.testDate));
WScript.Echo(Date.prototype.getMonth.call(x.testDate));
WScript.Echo(Date.prototype.getSeconds.call(x.testDate));
WScript.Echo(Date.prototype.getTime.call(x.testDate));
WScript.Echo(Date.prototype.getTimezoneOffset.call(x.testDate));
WScript.Echo(Date.prototype.getUTCDate.call(x.testDate));
WScript.Echo(Date.prototype.getUTCDay.call(x.testDate));
WScript.Echo(Date.prototype.getUTCFullYear.call(x.testDate));
WScript.Echo(Date.prototype.getUTCHours.call(x.testDate));
WScript.Echo(Date.prototype.getUTCMilliseconds.call(x.testDate));
WScript.Echo(Date.prototype.getUTCMinutes.call(x.testDate));
WScript.Echo(Date.prototype.getUTCMonth.call(x.testDate));
WScript.Echo(Date.prototype.getUTCSeconds.call(x.testDate));
WScript.Echo(Date.prototype.getVarDate.call(x.testDate));
WScript.Echo(Date.prototype.getYear.call(x.testDate));
WScript.Echo(Date.prototype.toDateString.call(x.testDate));
WScript.Echo(Date.prototype.toISOString.call(x.testDate));
WScript.Echo(Date.prototype.toJSON.call(x.testDate));
WScript.Echo(Date.prototype.toLocaleDateString.call(x.testDate));
WScript.Echo(Date.prototype.toLocaleString.call(x.testDate));
WScript.Echo(Date.prototype.toLocaleTimeString.call(x.testDate));
WScript.Echo(Date.prototype.toString.call(x.testDate));
WScript.Echo(Date.prototype.toTimeString.call(x.testDate));
WScript.Echo(Date.prototype.toUTCString.call(x.testDate));
WScript.Echo(Date.prototype.valueOf.call(x.testDate));
WScript.Echo(Date.prototype.valueOf.call(x.testDate) == x.testDate.getTime());

WScript.Echo();

WScript.Echo(x.Date.prototype.getDate.call(testDate));
WScript.Echo(x.Date.prototype.getDay.call(testDate));
WScript.Echo(x.Date.prototype.getFullYear.call(testDate));
WScript.Echo(x.Date.prototype.getHours.call(testDate));
WScript.Echo(x.Date.prototype.getMilliseconds.call(testDate));
WScript.Echo(x.Date.prototype.getMinutes.call(testDate));
WScript.Echo(x.Date.prototype.getMonth.call(testDate));
WScript.Echo(x.Date.prototype.getSeconds.call(testDate));
WScript.Echo(x.Date.prototype.getTime.call(testDate));
WScript.Echo(x.Date.prototype.getTimezoneOffset.call(testDate));
WScript.Echo(x.Date.prototype.getUTCDate.call(testDate));
WScript.Echo(x.Date.prototype.getUTCDay.call(testDate));
WScript.Echo(x.Date.prototype.getUTCFullYear.call(testDate));
WScript.Echo(x.Date.prototype.getUTCHours.call(testDate));
WScript.Echo(x.Date.prototype.getUTCMilliseconds.call(testDate));
WScript.Echo(x.Date.prototype.getUTCMinutes.call(testDate));
WScript.Echo(x.Date.prototype.getUTCMonth.call(testDate));
WScript.Echo(x.Date.prototype.getUTCSeconds.call(testDate));
WScript.Echo(x.Date.prototype.getVarDate.call(testDate));
WScript.Echo(x.Date.prototype.getYear.call(testDate));
WScript.Echo(x.Date.prototype.toDateString.call(testDate));
WScript.Echo(x.Date.prototype.toISOString.call(testDate));
WScript.Echo(x.Date.prototype.toJSON.call(testDate));
WScript.Echo(x.Date.prototype.toLocaleDateString.call(testDate));
WScript.Echo(x.Date.prototype.toLocaleString.call(testDate));
WScript.Echo(x.Date.prototype.toLocaleTimeString.call(testDate));
WScript.Echo(x.Date.prototype.toString.call(testDate));
WScript.Echo(x.Date.prototype.toTimeString.call(testDate));
WScript.Echo(x.Date.prototype.toUTCString.call(testDate));
WScript.Echo(x.Date.prototype.valueOf.call(testDate));
WScript.Echo(x.Date.prototype.valueOf.call(testDate) == testDate.getTime());

WScript.Echo();

x.testDate.setDate(17); WScript.Echo(x.testDate);
x.testDate.setFullYear(2012); WScript.Echo(x.testDate);
x.testDate.setHours(15); WScript.Echo(x.testDate);
x.testDate.setMilliseconds(982); WScript.Echo(x.testDate.getMilliseconds());
x.testDate.setMinutes(44); WScript.Echo(x.testDate);
x.testDate.setMonth(7); WScript.Echo(x.testDate);
x.testDate.setSeconds(29); WScript.Echo(x.testDate);
x.testDate.setTime(1345241466982); WScript.Echo(x.testDate);
x.testDate.setUTCDate(18); WScript.Echo(x.testDate);
x.testDate.setUTCFullYear(2013); WScript.Echo(x.testDate);
x.testDate.setUTCHours(15); WScript.Echo(x.testDate);
x.testDate.setUTCMilliseconds(852); WScript.Echo(x.testDate);
x.testDate.setUTCMinutes(45); WScript.Echo(x.testDate);
x.testDate.setUTCMonth(8); WScript.Echo(x.testDate);
x.testDate.setUTCSeconds(30); WScript.Echo(x.testDate);
x.testDate.setYear(97); WScript.Echo(x.testDate);

WScript.Echo();

Date.prototype.setDate.call(x.testDate, 17); WScript.Echo(x.testDate);
Date.prototype.setFullYear.call(x.testDate, 2012); WScript.Echo(x.testDate);
Date.prototype.setHours.call(x.testDate, 15); WScript.Echo(x.testDate);
Date.prototype.setMilliseconds.call(x.testDate, 982); WScript.Echo(x.testDate.getMilliseconds());
Date.prototype.setMinutes.call(x.testDate, 44); WScript.Echo(x.testDate);
Date.prototype.setMonth.call(x.testDate, 7); WScript.Echo(x.testDate);
Date.prototype.setSeconds.call(x.testDate, 29); WScript.Echo(x.testDate);
Date.prototype.setTime.call(x.testDate, 1345241466982); WScript.Echo(x.testDate);
Date.prototype.setUTCDate.call(x.testDate, 18); WScript.Echo(x.testDate);
Date.prototype.setUTCFullYear.call(x.testDate, 2013); WScript.Echo(x.testDate);
Date.prototype.setUTCHours.call(x.testDate, 15); WScript.Echo(x.testDate);
Date.prototype.setUTCMilliseconds.call(x.testDate, 852); WScript.Echo(x.testDate);
Date.prototype.setUTCMinutes.call(x.testDate, 45); WScript.Echo(x.testDate);
Date.prototype.setUTCMonth.call(x.testDate, 8); WScript.Echo(x.testDate);
Date.prototype.setUTCSeconds.call(x.testDate, 30); WScript.Echo(x.testDate);
Date.prototype.setYear.call(x.testDate, 97); WScript.Echo(x.testDate);

WScript.Echo();

x.Date.prototype.setDate.call(testDate, 17); WScript.Echo(testDate);
x.Date.prototype.setFullYear.call(testDate, 2012); WScript.Echo(testDate);
x.Date.prototype.setHours.call(testDate, 15); WScript.Echo(testDate);
x.Date.prototype.setMilliseconds.call(testDate, 982); WScript.Echo(testDate.getMilliseconds());
x.Date.prototype.setMinutes.call(testDate, 44); WScript.Echo(testDate);
x.Date.prototype.setMonth.call(testDate, 7); WScript.Echo(testDate);
x.Date.prototype.setSeconds.call(testDate, 29); WScript.Echo(testDate);
x.Date.prototype.setTime.call(testDate, 1345241466982); WScript.Echo(testDate);
x.Date.prototype.setUTCDate.call(testDate, 18); WScript.Echo(testDate);
x.Date.prototype.setUTCFullYear.call(testDate, 2013); WScript.Echo(testDate);
x.Date.prototype.setUTCHours.call(testDate, 15); WScript.Echo(testDate);
x.Date.prototype.setUTCMilliseconds.call(testDate, 852); WScript.Echo(testDate);
x.Date.prototype.setUTCMinutes.call(testDate, 45); WScript.Echo(testDate);
x.Date.prototype.setUTCMonth.call(testDate, 8); WScript.Echo(testDate);
x.Date.prototype.setUTCSeconds.call(testDate, 30); WScript.Echo(testDate);
x.Date.prototype.setYear.call(testDate, 97); WScript.Echo(testDate);
x.Date.prototype.setYear.call(testDate, -12345); WScript.Echo(testDate);

// BLUE 295070 - Date.prototype.toString : The output string reports incorrect DST status
WScript.Echo();
WScript.Echo("Daylight savings section");
WScript.Echo();
// PDT vs PST

for (var i = 119; i < 121; i++) {
    var testDate = new Date(2012, 10, 4, 0, i, 0);
    WScript.Echo("");
    WScript.Echo(testDate);
    WScript.Echo(testDate.toString());
    WScript.Echo(testDate.toTimeString());
    WScript.Echo(testDate.toUTCString());
}