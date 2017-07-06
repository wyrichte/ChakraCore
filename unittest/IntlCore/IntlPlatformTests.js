//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

var platform = Debug.EngineInterface.Intl;

// WinTH: Changed to use non-breaking space in some output

function Assert(func, arguments, expected, thisArg, altExpected) {
    var result = func.apply(thisArg, arguments);
    var funcName = func.toString().match(/function (.*)\(/)[1];

    if (String(result) !== String(expected) && String(result) !== String(altExpected)) {
        WScript.Echo(funcName + ": Expected result was '" + expected + "', actual '" + result + "'.");
    }
}

function AssertRegex(func, arguments, expectedRegex, thisArg) {
    var result = func.apply(thisArg, arguments);
    var funcName = func.toString().match(/function (.*)\(/)[1];

    if (!expectedRegex.test(String(result))) {
        WScript.Echo(funcName + ": Expected result Regex was '" + expectedRegex + "', actual '" + String(result) + "'.");
    }
}

function AssertTypeOf(func, arguments, expected, thisArg) {
    var result = func.apply(thisArg, arguments);
    var funcName = func.toString().match(/function (.*)\(/)[1];

    if ((typeof result) !== (typeof expected)) {
        WScript.Echo(funcName + ": Expected result was '" + (typeof expected) + "', actual '" + (typeof result) + "'.");
    }
}

function AssertError(func, arguments, expectedType) {
    var funcName = func.toString().match(/function (.*)\(/)[1];
    try {
        func.apply(thisArg, arguments);
        WScript.Echo(funcName + ": Expected result was an error of type '" + (typeof e) + "', but no error was thrown.");
    }
    catch (e) {
        if ((typeof e) != (typeof expectedType)) {
            WScript.Echo(funcName + ": Expected result was an error of type '" + (typeof e) + "', actual '" + (typeof expectedType) + "'.");
        }
    }
}

function AssertObjectTypeOf(obj, objname, expected) {
    var result = typeof obj;
    
    if (result !== expected) {
        WScript.Echo("typeof " + objname + " = '" + result + "', expected '" + expected + "'.");
    }
}

AssertObjectTypeOf(Intl, 'Intl', 'object');
AssertObjectTypeOf(Intl.Collator, 'Intl.Collator', 'function');

Assert(platform.isWellFormedLanguageTag, ["en-US"], true);
Assert(platform.isWellFormedLanguageTag, ["1"], false);
Assert(platform.normalizeLanguageTag, ["en-us"], "en-US");
Assert(platform.compareString, ["A", "a", "en-US", undefined, undefined, undefined], 1);
Assert(platform.resolveLocaleLookup, [platform.getDefaultLocale()], platform.getDefaultLocale());
Assert(platform.resolveLocaleBestFit, [platform.getDefaultLocale()], platform.getDefaultLocale());
Assert(platform.getExtensions, ["en-us-u-kf-true"], ["kf-true"]);
Assert(platform.getExtensions, ["en-us-kf-true"], undefined);

var currencyFormatterStateObject = { __locale: "en-US", __formatterToUse: 2, __currency: "USD" };
var numberFormatterStateObject = { __locale: "en-US", __formatterToUse: 0 };
var percentFormatterStateObject = { __locale: "en-US", __formatterToUse: 1 };
Assert(platform.cacheNumberFormat, [currencyFormatterStateObject], undefined);
Assert(platform.cacheNumberFormat, [numberFormatterStateObject], undefined);
Assert(platform.cacheNumberFormat, [percentFormatterStateObject], undefined);

Assert(platform.formatNumber, [1, currencyFormatterStateObject], "$1");
Assert(platform.formatNumber, [1, numberFormatterStateObject], "1");
AssertRegex(platform.formatNumber, [0.5, percentFormatterStateObject], new RegExp("50[\x20\u00a0]?%"), /*thisArg*/undefined);

Assert(platform.currencyDigits, ["USD"], 2);

var dateTimeFormatterStateObject = { __locale: "en-US", __templateString: "hour minute", __windowsClock: "24HourClock", __timeZone:"UTC" };
Assert(platform.createDateTimeFormat, [dateTimeFormatterStateObject, true], undefined);
Assert(platform.formatDateTime, [Number(new Date(2000, 1, 1, 13, 13, 13, 13)), dateTimeFormatterStateObject], "\u200e21\u200e:\u200e13");

var hiddenObject = { val: 5 };
var someObject = { other: 5 };
Assert(platform.getHiddenObject, [{}], undefined);
Assert(platform.setHiddenObject, [someObject, hiddenObject], true);
if (Object.getOwnPropertyNames(someObject).length > 1) {
    WScript.Echo("Hidden object appeared in the properties.");
}
if (platform.getHiddenObject(someObject) !== hiddenObject) {
    WScript.Echo("Hidden object was stored incorrectly.");
}

Assert(platform.registerBuiltInFunction, [function () { return "Pass0"; }, 0], undefined);
Assert(platform.registerBuiltInFunction, [function () { return "Pass1"; }, 1], undefined);
Assert(platform.registerBuiltInFunction, [function () { return "Pass2"; }, 2], undefined);
Assert(platform.registerBuiltInFunction, [function () { return "Pass3"; }, 3], undefined);
Assert(platform.registerBuiltInFunction, [function () { return "Pass4"; }, 4], undefined);
Assert(Date.prototype.toLocaleString, [], "Pass0", new Date());
Assert(Date.prototype.toLocaleDateString, [], "Pass1", new Date());
Assert(Date.prototype.toLocaleTimeString, [], "Pass2", new Date());
Assert(Number.prototype.toLocaleString, [], "Pass3", 5);
Assert(String.prototype.localeCompare, [], "Pass4", "");
Assert(platform.builtInGetArrayLength, [["", ""]], 2);
Assert(platform.builtInSetPrototype, [someObject, hiddenObject], {});
if (!hiddenObject.isPrototypeOf(someObject)) {
    WScript.Echo("Failed to set the prototype.");
}
Assert(platform.builtInCallInstanceFunction, [function () { return this; }, "test"], "test");
AssertTypeOf(platform.Boolean, [5], Boolean(5));
AssertTypeOf(platform.String, [5], String(5));
AssertTypeOf(platform.Number, [5], Number(5));
AssertTypeOf(platform.Date, [5], Date(5));
AssertTypeOf(platform.RegExp, [5], RegExp(5));
AssertTypeOf(platform.Object, [5], Object(5));


if ((typeof new platform.Boolean(5)) !== (typeof new Boolean(5))) {
    WScript.Echo("platform.Boolean: Not a constructor.");
}
if ((typeof new platform.Number(5)) !== (typeof new Number(5))) {
    WScript.Echo("platform.Number: Not a constructor.");
}
if ((typeof new platform.Date(5)) !== (typeof new Date(5))) {
    WScript.Echo("platform.Date: Not a constructor.");
}
if ((typeof new platform.RegExp(5)) !== (typeof new RegExp(5))) {
    WScript.Echo("platform.Boolean: Not a constructor.");
}
if ((typeof new platform.Object(5)) !== (typeof new Object(5))) {
    WScript.Echo("platform.Object: Not a constructor.");
}
if ((typeof new platform.String(5)) !== (typeof new String(5))) {
    WScript.Echo("platform.String: Not a constructor.");
}

Assert(platform.builtInMathAbs, [-5], "5");
Assert(platform.builtInMathFloor, [5.5], "5");
Assert(platform.builtInMathMax, [4, 5], "5");
Assert(platform.builtInMathPow, [2, 2], "4");

Assert(platform.builtInJavascriptObjectEntryDefineProperty, [someObject, "test", { value: 5 }], {});
if (someObject.test !== 5) {
    WScript.Echo("Define property was incorrect.");
}
platform.builtInSetPrototype(someObject, null);
Assert(platform.builtInJavascriptObjectEntryGetPrototypeOf, [someObject], null);
Assert(platform.builtInJavascriptObjectEntryIsExtensible, [{}], true);
Assert(platform.builtInJavascriptObjectEntryGetOwnPropertyNames, [someObject], ["other", "test"]);
Assert(platform.builtInJavascriptObjectEntryHasOwnProperty, ["test"], true, someObject);
var pass = false;
Assert(platform.builtInJavascriptArrayEntryForEach, [function (val) { pass = val; }], undefined, [1]);
if (pass !== 1) {
    WScript.Echo("Failed Array.prototype.forEach");
}
Assert(platform.builtInJavascriptArrayEntryIndexOf, [1], 2, [3, 4, 1]);
Assert(platform.builtInJavascriptArrayEntryPush, [1], [1], []);
Assert(platform.builtInJavascriptArrayEntryJoin, ["."], "1.1", [1, 1]);

Assert(platform.builtInJavascriptDateEntryGetDate, [], 1, new Date(2000, 1, 1));

Assert(platform.builtInJavascriptStringEntryReplace, ["5", "4"], "a4", "a5");
Assert(platform.builtInJavascriptStringEntryToLowerCase, [], "ab", "aB");
Assert(platform.builtInJavascriptStringEntryToUpperCase, [], "AB", "aB");

Assert(platform.builtInGlobalObjectEntryIsFinite, [NaN], false);
Assert(platform.builtInGlobalObjectEntryIsNaN, [NaN], true);

AssertError(platform.raiseNeedObject, [], new TypeError());
AssertError(platform.raiseObjectIsAlreadyInitialized, ["", ""], new TypeError());
AssertError(platform.raiseOptionValueOutOfRange, ["", "", ""], new RangeError());
AssertError(platform.raiseNeedObjectOrString, [""], new TypeError());
AssertError(platform.raiseLocaleNotWellFormed, [""], new RangeError());
AssertError(platform.raiseThis_NullOrUndefined, [""], new TypeError());
AssertError(platform.raiseNotAConstructor, [""], new TypeError());
AssertError(platform.raiseObjectIsNonExtensible, [""], new TypeError());
AssertError(platform.raiseNeedObjectOfType, ["", ""], new TypeError());
AssertError(platform.raiseInvalidCurrencyCode, [""], new RangeError());
AssertError(platform.raiseMissingCurrencyCode, [], new TypeError());
AssertError(platform.raiseInvalidDate, [], new RangeError());

//Tests on platform's call to Regex to bypass the constructor updates.
"a".match(/(a)/);
var before = {};
Object.getOwnPropertyNames(RegExp).forEach(function (key) { before[key] = RegExp[key]; });
var builtInRegex = JSON.stringify(platform.builtInRegexMatch("test", /(t)e(s(t))/));
Object.getOwnPropertyNames(RegExp).forEach(function (key) { if (RegExp[key] !== before[key]) WScript.Echo("Built-In regex implementation overwrote the global constructor's value."); });

var normalRegexResult = JSON.stringify("test".match(/(t)e(s(t))/));
if (builtInRegex !== normalRegexResult) {
    WScript.Echo("Built-in result doesn't equal regex result. Built In: " + builtInRegex + "; using normal regex: " + normalRegexResult);
}

//Blue 249001
var localesList = [
    { defaultLocale: "de-DE_phoneb", parsedLocale: "de-DE", collation: "phonebk" },
    { defaultLocale: "es-ES_tradnl", parsedLocale: "es-ES", collation: "trad" },
    { defaultLocale: "lv-LV_tradnl", parsedLocale: "lv", collation: "trad" },
    { defaultLocale: "ja-JP_radstr", parsedLocale: "ja", collation: "unihan" },
// These locales behave different on win7
//    { defaultLocale: "zh-TW_pronun", parsedLocale: "zh-Hant-TW", collation: "phonetic" },
//    { defaultLocale: "zh-TW_radstr", parsedLocale: "zh-Hant-TW", collation: "unihan" },
    { defaultLocale: "zh-HK_radstr", parsedLocale: "zh-Hant-HK", collation: "unihan" },
    { defaultLocale: "zh-MO_radstr", parsedLocale: "zh-Hant-MO", collation: "unihan" },
    { defaultLocale: "zh-CN_stroke", parsedLocale: "zh-Hans-CN", collation: "stroke" },
    { defaultLocale: "zh-SG_stroke", parsedLocale: "zh-Hans-SG", collation: "stroke" },
    { defaultLocale: "hu-HU_technl", parsedLocale: "hu", collation: "default" },
    { defaultLocale: "ka-GE_modern", parsedLocale: "ka", collation: "default" }];

localesList.forEach(function (localeInfo) {
    platform.getDefaultLocale = function () { return localeInfo.defaultLocale; }
    var resolvedOptions = new Intl.Collator().resolvedOptions();
    if (resolvedOptions.locale !== localeInfo.parsedLocale || resolvedOptions.collation !== localeInfo.collation) {
        WScript.Echo("Failed to parse default locale of '" + JSON.stringify(localeInfo) + "'. Options: " + JSON.stringify(resolvedOptions));
    }
});

WScript.Echo("Pass");
