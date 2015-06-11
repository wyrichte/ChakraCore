WScript.InitializeProjection();

(function () {
    'use strict'

    var errors = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes;
    var errorAccess = Winery.WinRTErrorTests.RestrictedErrorAccess;

    var originalErrorString = "Originate: E_HANDLE";
    var expectedErrorString = "Transform: E_HANDLE -> E_OUTOFMEMORY";
    var expectedMessage = "Called delegate to originate E_HANDLE";

    var delegate = function (sender, msg) {
        WScript.Echo(msg);
        try {
            errorAccess.originateError(errors.handle, originalErrorString);
            WScript.Echo("SHOULD NOT REACH THIS POINT");
        } catch (e) {
            WScript.Echo(TestUtilities.GetRestrictedStringFromError(e));
            throw e;
        }
        WScript.Echo("SHOULD NOT REACH THIS POINT");
    }
    errorAccess.transformDelegateError(delegate, expectedMessage, errors.outOfMemory, expectedErrorString);
    WScript.Echo("transformDelegateError call SUCCEEDED");
})();
