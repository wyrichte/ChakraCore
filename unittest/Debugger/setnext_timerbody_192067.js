// Validating 192067, setnext statement on the timer body.

function refreshTimer() {
}
WScript.SetTimeout("refreshTimer();/**bp:stack();setnext(1,0)**/\n", 100);

WScript.Echo("Pass");
