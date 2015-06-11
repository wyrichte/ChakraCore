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

var msg = "123";

var toaster = new Fabrikam.Kitchen.Toaster();
var toastCompleteCount = 0;
function toastCompleteCallback(ev) {
    verify(ev.message, msg, 'ev.message');
    toastCompleteCount++;
}
Debug.dumpHeap(2, false, true);
toaster.addEventListener("toastcompleteevent", toastCompleteCallback);
Debug.dumpHeap(0, true, true);
var toast = toaster.makeToast(msg);
verify(toastCompleteCount, 1, 'toastCompleteCount');
