WScript.InitializeProjection();

(function () {
    'use strict'

    var errors = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes;
    var errorAccess = Winery.WinRTErrorTests.RestrictedErrorAccess;

    var originalString = "Originate: E_FAIL";
    var expectedString = "Transform: E_FAIL -> E_ACCESSDENIED";
    errorAccess.transformError(errors.fail, errors.accessDenied, originalString, expectedString);

    WScript.Echo("SHOULD NOT REACH THIS POINT");
})();
