WScript.InitializeProjection();

(function () {
    'use strict'

    var errors = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes;
    var errorAccess = Winery.WinRTErrorTests.RestrictedErrorAccess;

    var expectedString = "Originate: E_INVALIDARG";
    errorAccess.originateError(errors.invalidArg, expectedString);

    WScript.Echo("SHOULD NOT REACH THIS POINT");
})();
