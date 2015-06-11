WScript.InitializeProjection();

(function () {
    'use strict'

    var errors = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes;
    var errorAccess = Winery.WinRTErrorTests.RestrictedErrorAccess;

    var expectedMessage = "Called delegate to originate E_NOINTERFACE";
    var expectedErrorString = "Originate: E_NOINTERFACE";

    errorAccess.invokeDelegate(function (sender, msg) {
        WScript.Echo(msg);
        errorAccess.originateError(errors.noInterface, expectedErrorString);
    }, expectedMessage);

    WScript.Echo("invokeDelegate call SUCCEEDED");
})();
