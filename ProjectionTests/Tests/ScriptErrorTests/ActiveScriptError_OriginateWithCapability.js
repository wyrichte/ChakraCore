WScript.InitializeProjection();

(function () {
    'use strict'

    var errors = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes;
    var errorAccess = Winery.WinRTErrorTests.RestrictedErrorAccess;
    var capabilities = Winery.WinRTErrorTests.TestSids;

    var expectedString = "Originate: E_ACCESSDENIED";
    errorAccess.originateErrorWithCapabilitySid(errors.accessDenied, expectedString, capabilities.winCapabilityInternetClientSid);

    WScript.Echo("SHOULD NOT REACH THIS POINT");
})();
