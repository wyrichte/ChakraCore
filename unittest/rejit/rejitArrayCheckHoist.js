function wrongArray(bailout, changeProfileData) {
    wrongArray_run(bailout ? "01" : [0, 1], 2);

    function wrongArray_run(a, n) {
        var sum = 0;
        for(var i = 0; i < n; ++i)
            if(!bailout || changeProfileData)
                sum += a[i];
        return sum;
    }
}

function changeToEs5Array(miscellaenousBailout, implicitCallBailout) {
    var a = [0, 1];
    Object.defineProperty(
        Array.prototype,
        "changeToEs5Array",
        {
            configurable: true,
            enumerable: true,
            set: function (v) {
                Object.defineProperty(this, 0, { configurable: true, writable: true, enumerable: false });
            }
        });
    changeToEs5Array_run([0, 1], 2, miscellaenousBailout, implicitCallBailout);

    function changeToEs5Array_run(a, n, miscellaenousBailout, implicitCallBailout) {
        if(miscellaenousBailout) {
            n = -n;
            n *= n + 2; // bail on negative zero
        }

        var sum = 0;
        for(var i = 0; i < n; ++i) {
            sum += a[i];
            if(implicitCallBailout && i === 1)
                a.changeToEs5Array = 0;
            sum += a[i];
        }
        return sum;
    }
}

WScript.Echo("--------------------------");
WScript.Echo("Bailout due to wrong array");
WScript.Echo("--------------------------");
WScript.Echo("");

WScript.Echo("--- Interpret ---");
WScript.Echo("");
wrongArray(false, false);
wrongArray(false, false);
wrongArray(false, false);

WScript.Echo("");
WScript.Echo("--- JIT, run jitted code ---");
WScript.Echo("");
wrongArray(false, false);

WScript.Echo("");
WScript.Echo("--- Bailout, don't change to profile data ---");
WScript.Echo("");
wrongArray(true, false);

WScript.Echo("");
WScript.Echo("--- Bailout, don't change to profile data, schedule rejit with runtime stats enabled ---");
WScript.Echo("");
wrongArray(true, false);

WScript.Echo("");
WScript.Echo("--- Rejit with runtime stats enabled, run rejitted code, bailout, change profile data ---");
WScript.Echo("");
wrongArray(true, true);
wrongArray(true, true);
wrongArray(true, true);
wrongArray(true, true);
wrongArray(true, true);

WScript.Echo("");
WScript.Echo("--- Bailout, schedule rejit with array check hoisting disabled ---");
WScript.Echo("");
wrongArray(true, true);

WScript.Echo("");
WScript.Echo("--- Rejit with array check hoisting disabled, run rejitted code, no bailout ---");
WScript.Echo("");
wrongArray(true, true);
wrongArray(false, false);

WScript.Echo("");
WScript.Echo("--------------------------------------------------------------------");
WScript.Echo("Bailout due to an implicit call changing the array into an ES5 array");
WScript.Echo("--------------------------------------------------------------------");
WScript.Echo("");

WScript.Echo("--- Interpret ---");
WScript.Echo("");
changeToEs5Array(false, false);
changeToEs5Array(false, false);
changeToEs5Array(false, false);

WScript.Echo("");
WScript.Echo("--- JIT, run jitted code ---");
WScript.Echo("");
changeToEs5Array(false, false);

WScript.Echo("");
WScript.Echo("--- Miscellaneous bailout, don't change to profile data ---");
WScript.Echo("");
changeToEs5Array(true, false);

WScript.Echo("");
WScript.Echo("--- Miscellaneous bailout, don't change to profile data, schedule rejit with runtime stats enabled ---");
WScript.Echo("");
changeToEs5Array(true, false);

WScript.Echo("");
WScript.Echo("--- Rejit with runtime stats enabled, run rejitted code, bailout on implicit call, change profile data ---");
WScript.Echo("");
changeToEs5Array(false, true);
changeToEs5Array(false, true);
changeToEs5Array(false, true);
changeToEs5Array(false, true);
changeToEs5Array(false, true);

WScript.Echo("");
WScript.Echo("--- Bailout on implicit call, schedule rejit with updated implicit call flags ---");
WScript.Echo("");
changeToEs5Array(false, true);

WScript.Echo("");
WScript.Echo("--- Rejit with updated implicit call flags, run rejitted code, no bailout ---");
WScript.Echo("");
changeToEs5Array(false, true);
changeToEs5Array(false, false);
