WScript.InitializeProjection();
function verify(actualValue, expectedValue, valueMsg) {
    if (actualValue === expectedValue) {
        WScript.Echo("Pass: " + valueMsg);
    }
    else {
        WScript.Echo("Fail: " + valueMsg);
        WScript.Echo("    Expected: " + expectedValue);
    }
    WScript.Echo("      Actual: " + actualValue + "\n");
}

var callbackCount = 0;
function cookiesEatenCallback(ev) {
    callbackCount++;
}
function cookiesEatenHandler(ev) {
    callbackCount++;
}
Animals.Pomapoodle.addEventListener("cookieseatenevent", cookiesEatenCallback);
Animals.Pomapoodle.oncookieseatenevent = cookiesEatenHandler;
Animals.Pomapoodle.eatCookies(1);
verify(callbackCount, 2, "callbackCount");
Debug.dumpHeap(Animals.Pomapoodle);
