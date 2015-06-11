WScript.InitializeProjection();
function verify(actualValue, expectedValue, valueMsg) {
    if (actualValue === expectedValue) {
        WScript.Echo("Pass: " + valueMsg);
    }
    else {
        WScript.Echo("Fail: " + valueMsg);
        WScript.Echo("    Expected: " + expectedValue);
        runner.currentTestSucceeded = false;
    }
    WScript.Echo("      Actual: " + actualValue + "\n");
}

var callbackCount = 0;
function cookiesEatenCallback(ev) {
    callbackCount++;
}
Animals.Pomapoodle.oncookieseatenevent = cookiesEatenCallback;
Animals.Pomapoodle.eatCookies(1);
verify(callbackCount, 1, "callbackCount");
Debug.dumpHeap(Animals.Pomapoodle);
