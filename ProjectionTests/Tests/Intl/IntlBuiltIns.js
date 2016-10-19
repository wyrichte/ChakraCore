
function testBuiltInFunction(options, builtInConstructor, builtInName, builtInFunc, intlConstructor, intlFunc, args) {
    try{
        var builtInValue = args.length === 1 ? new builtInConstructor(args[0])[builtInFunc]("en-US", options) : new builtInConstructor(args[0])[builtInFunc](args[1], "en-US", options);
        var intlValue = new Intl[intlConstructor]("en-US", options)[intlFunc](args[0], args[1]);
        
        if (builtInValue !== intlValue) {
            WScript.Echo("ERROR: Result from built in function 'new " + builtInName+"()." + builtInFunc + "' doesn't match Intl." + intlConstructor + "'s function '" + intlFunc + "'!");
        }
        else {
            WScript.Echo(builtInValue);
        }
    }
    catch (ex) {
        WScript.Echo(ex.message);
    }
}
testBuiltInFunction({ minimumFractionDigits: 3 }, Number, "Number", "toLocaleString", "NumberFormat", "format", [5]);
testBuiltInFunction({ sensitivity: "base" }, String, "String", "localeCompare", "Collator", "compare", ["A", "a"]);
testBuiltInFunction({ hour: "numeric", timeZone:"UTC" }, Date, "Date", "toLocaleString", "DateTimeFormat", "format", [new Date(2000, 1, 1)]);
testBuiltInFunction({ hour: "numeric", timeZone:"UTC" }, Date, "Date", "toLocaleTimeString", "DateTimeFormat", "format", [new Date(2000, 1, 1)]);
testBuiltInFunction({ month: "numeric", timeZone:"UTC" }, Date, "Date", "toLocaleDateString", "DateTimeFormat", "format", [new Date(2000, 1, 1)]);