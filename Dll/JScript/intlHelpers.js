// JSLS mock-up of Intl EngineInterfaceObject

var Intl = 
{
    Collator : function(locales, options){},
    NumberFormat: function (locales, options) {},
    DateTimeFormat: function (locales, options) {}
};
Intl.Collator.supportedLocalesOf = function(locales, options) {};
Intl.NumberFormat.supportedLocalesOf = function(locales, options) {};
Intl.DateTimeFormat.supportedLocalesOf = function(locales, options) {};
Intl.Collator.prototype.compare = function(a, b) {};
Intl.Collator.prototype.resolvedOptions = function() {};
Intl.NumberFormat.prototype.format = function(n) {};
Intl.NumberFormat.prototype.resolvedOptions = function() {};
Intl.DateTimeFormat.prototype.format = function(date) {};
Intl.DateTimeFormat.prototype.resolvedOptions = function() {};
(function (Intl) {
    Intl.EngineInterface = {};
    Intl.EngineInterface.Intl = {
        "Boolean": Boolean,
        "Object": Object,
        "Number": Number,
        "RegExp": RegExp,
        "String": String,
        "Date": Date,
        "Error": Error,

        builtInMathAbs: Math.abs,
        builtInMathFloor: Math.floor,
        builtInMathMax: Math.max,
        builtInMathPow: Math.pow,

        builtInJavascriptObjectEntryDefineProperty: Object.defineProperty,
        builtInJavascriptObjectEntryGetPrototypeOf: Object.getPrototypeOf,
        builtInJavascriptObjectEntryIsExtensible: Object.isExtensible,
        builtInJavascriptObjectEntryGetOwnPropertyNames: Object.getOwnPropertyNames,
        builtInJavascriptObjectEntryHasOwnProperty: Object.hasOwnProperty,

        builtInJavascriptArrayEntryForEach: Array.prototype.forEach,
        builtInJavascriptArrayEntryIndexOf: Array.prototype.indexOf,
        builtInJavascriptArrayEntryPush: Array.prototype.push,
        builtInJavascriptArrayEntryJoin: Array.prototype.join,

        builtInJavascriptFunctionEntryBind: Function.prototype.bind,
        builtInJavascriptRegExpEntryTest: RegExp.prototype.test,
        builtInJavascriptDateEntryGetDate: Date.prototype.getDate,
        builtInJavascriptDateEntryNow: Date.now,

        builtInJavascriptStringEntryMatch: String.prototype.match,
        builtInJavascriptStringEntryReplace: String.prototype.replace,
        builtInJavascriptStringEntryToLowerCase: String.prototype.toLowerCase,
        builtInJavascriptStringEntryToUpperCase: String.prototype.toUpperCase,

        builtInGlobalObjectEntryIsFinite: isFinite,
        builtInGlobalObjectEntryIsNaN: isNaN,

        raiseNeedObject: function () { throw new TypeError(); },
        raiseObjectIsAlreadyInitialized: function () { throw new TypeError(); },
        raiseOptionValueOutOfRange: function () { throw new RangeError(); },
        raiseOptionValueOutOfRange_3: function () { throw new RangeError(); },
        raiseNeedObjectOrString: function () { throw new TypeError(); },
        raiseLocaleNotWellFormed: function () { throw new RangeError(); },
        raiseThis_NullOrUndefined: function () { throw new TypeError(); },
        raiseNotAConstructor: function () { throw new TypeError(); },
        raiseObjectIsNonExtensible: function () { throw new TypeError(); },
        raiseNeedObjectOfType: function () { throw new TypeError(); },
        raiseInvalidCurrencyCode: function () { throw new RangeError(); },
        raiseMissingCurrencyCode: function () { throw new TypeError(); },
        raiseInvalidDate: function () { throw new RangeError(); }
    }

    Intl.EngineInterface.Intl.hidePlatform = function () {
        delete Intl.EngineInterface;
    }

    Intl.EngineInterface.Intl.builtInSetPrototype = function (obj, proto) {
        obj.__proto__ = proto;
        return obj;
    }

    Intl.EngineInterface.Intl.builtInGetArrayLength = function (arr) {
        return arr.length;
    }

    Intl.EngineInterface.Intl.builtInCallInstanceFunction = function (func, instance) {
        return func.apply(instance, Array.prototype.slice.call(arguments, 2));
    }

    Intl.EngineInterface.Intl.raiseAssert = function () { }

    Intl.EngineInterface.Intl.isWellFormedLanguageTag = function () {
        return true;
    }

    Intl.EngineInterface.Intl.normalizeLanguageTag = function (tag) {
        return tag;
    }

    Intl.EngineInterface.Intl.compareString = function () {
        return 0;
    }

    Intl.EngineInterface.Intl.resolveLocaleLookup = function (locale) {
        return locale;
    }

    Intl.EngineInterface.Intl.resolveLocaleBestFit = function (locale) {
        return locale;
    }

    Intl.EngineInterface.Intl.getDefaultLocale = function () {
        return "";
    }

    Intl.EngineInterface.Intl.getExtensions = function () {
        return [];
    }

    Intl.EngineInterface.Intl.formatNumber = function (number) {
        return number.toString();
    }

    Intl.EngineInterface.Intl.cacheNumberFormat = function (numberFormat) {
        numberFormat.__numberingSystem = "";
        numberFormat.__locale = "";
    }

    Intl.EngineInterface.Intl.createDateTimeFormat = function (dateTimeFormat) {
        dateTimeFormat.__windowsCalendar = "";
        dateTimeFormat.__windowsClock = "";
        dateTimeFormat.__numberingSystem = "";
        dateTimeFormat.__patternStrings = [""];
    }

    Intl.EngineInterface.Intl.currencyDigits = function () {
        return 0;
    }

    Intl.EngineInterface.Intl.formatDateTime = function (date) {
        return date.toString();
    }

    Intl.EngineInterface.Intl.registerBuiltInFunction = function () { }

    var hiddenObjectMap = new WeakMap();

    Intl.EngineInterface.Intl.getHiddenObject = function (obj) {
        return hiddenObjectMap.get(obj);
    };

    Intl.EngineInterface.Intl.setHiddenObject = function (obj, hiddenObject) {
        hiddenObjectMap.set(obj, hiddenObject);
        return true;
    };
})(Intl);
