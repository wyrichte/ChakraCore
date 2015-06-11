WScript.InitializeProjection();

(function () {
    'use strict'

    var errors = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes;
    var errorAccess = Winery.WinRTErrorTests.RestrictedErrorAccess;

    var expectedErrorString = "Originate: E_NOTIMPL";

    var delegate2 = function (sender, msg) {
        WScript.Echo(msg);
        errorAccess.originateError(errors.notImpl, expectedErrorString);
        WScript.Echo("SHOULD NOT REACH THIS POINT");
    }

    var delegate1 = function (sender, msg) {
        WScript.Echo(msg);
        errorAccess.invokeDelegate(delegate2, "Called delegate2");
        WScript.Echo("invokeDelegate on delegate2 SUCCEEDED");
    }

    errorAccess.invokeDelegate(delegate1, "Called delegate1");

    WScript.Echo("invokeDelegate on delegate1 SUCCEEDED");
})();
