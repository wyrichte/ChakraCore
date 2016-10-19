if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    // Trim absolute paths from stack traces so that we can compare them with baseline.
    function TrimStackTracePath(line) {
        return line && line.replace(/\(.+\\/ig, "(");
    }

    verify.typeOf = function (obj, expected, msg) {
        return verify.equal(typeof obj, expected, "typeof " + (msg || obj));
    };

    verify.members = function verifyMembers(obj, expected, msg) {
        logger.comment("Verifying members of " + msg);
        var expect;
        for (var mem in expected) {
            expect = expected[mem];
            verify.defined(expect, mem);
            verify.typeOf(obj[mem], expect, (mem === "stack" && TrimStackTracePath(obj[mem])) || obj[mem]);
        }
    };

    verify.doesNotContainValue = function doesNotContainValue(obj, val, msg) {
        logger.comment("Verifying " + msg + " does not contain property with value '" + val + "'");
        var found = false;
        for (var p in obj) {
            if (obj[p] == val) { found = true; }
        }
        verify(found, false, "Found property with value '" + val + "'");
    };

    // If we are in a webview host, static members and object construction are not supported
    var isWebViewHost = (typeof TestUtilities !== 'undefined' && TestUtilities.GetHostType() === 3);
    var currentId = 1;
    var capabilities = Winery.WinRTErrorTests.TestSids;
    var errorAccess = isWebViewHost ? TestUtilities.RestrictedErrorAccessInstanceToVar() : Winery.WinRTErrorTests.RestrictedErrorAccess;
    var errors = errorAccess.errorCodes;

    runner.addTest({
        id: currentId++,
        desc: 'CallOriginateErrorAndReturnDifferentHRESULT',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var errorString = "OriginateErrorReturnHr: E_FAIL";
            var errorHr = -2147454974;
            var actualHr = -2147454973;
            var actualString = "Unknown runtime error";

            try {
                logger.comment("Calling RoOriginateError but returning a different HRESULT should raise unknown error (which doesn't contain errorString passed to OriginateError).");
                var returnedHr = errorAccess.originateErrorReturnHr(errorHr, errorString, actualHr);
            } catch (e) {
                verify(e.number, actualHr, "e.number");
                verify(e.message, actualString, 'e.message');
                verify.defined(TrimStackTracePath(e.stack), 'e.stack');
                verify.defined(e.description, 'e.description');
            }
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'CallOriginateErrorAndReturnSuccess',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var errorString = "OriginateErrorReturnHr: E_FAIL";
            var errorHr = -2147454974;
            var actualHr = 0;

            try {
                logger.comment("Calling RoOriginateError but returning S_OK should not raise exception.");
                var returnedHr = errorAccess.originateErrorReturnHr(errorHr, errorString, actualHr);
            } catch (e) {
                logger.comment("Exception raised.");
                verify.undefined(e, 'e');
                verify.undefined(e.number, "e.number");
                verify.undefined(e.message, 'e.message');
                verify.defined(TrimStackTracePath(e.stack), 'e.stack');
                verify.undefined(e.description, 'e.description');
            }
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'CallOriginateErrorWithAndReturnSuccessAnotherCallReturnsSameFailHRButNotOriginateError',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var errorString = "OriginateErrorReturnHr: We shouldn't see this error message returned from RT.";
            var errorHr = -2147454974;
            var actualHr = 0;
            var actualString = "Unknown runtime error";

            try {
                logger.comment("Calling RoOriginateError but returning S_OK should not raise exception.");
                var returnedHr = errorAccess.originateErrorReturnHr(errorHr, errorString, actualHr);
            } catch (e) {
                logger.comment("Exception raised.");
                verify.undefined(e, 'e');
                verify.undefined(e.number, "e.number");
                verify.undefined(e.message, 'e.message');
                verify.undefined(e.stack, 'e.stack');
                verify.undefined(e.description, 'e.description');
            }

            try {
                logger.comment("Calling method which doesn't RoOriginateError but returns previous fail hr should raise exception but not contain errorString.");
                var returnedHr = errorAccess.returnHr(errorHr);
            } catch (e) {
                logger.comment("Exception raised.");
                verify(e.number, errorHr, "e.number");
                verify(e.message, actualString, 'e.message');
                verify.defined(TrimStackTracePath(e.stack), 'e.stack');
                verify.defined(e.description, 'e.description');
            }
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'Originate and catch error -- verify error object var and restricted values',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var errorObjectExpected = {
                description: "string",
                number: "number",
                stack: "string"
            };
            var expectedString;

            for (var err in errors) {
                logger.comment("Testing HRESULT: e_" + err);
                var systemErrorString = TestUtilities.GetSystemStringFromHr(errors[err]);
                var restrictedErrorString = "Originate: e_" + err;
                expectedString = systemErrorString + "\r\n" + restrictedErrorString;

                try {
                    errorAccess.originateError(errors[err], restrictedErrorString);
                } catch (e) {
                    verify(e.number, errors[err], "e.number");
                    verify(e.message, expectedString, 'e.message');
                    verify.members(e, errorObjectExpected, "e");
                    verify(TestUtilities.GetRestrictedStringFromError(e), restrictedErrorString, "TestUtilities.GetRestrictedStringFromError(e)");
                }
            }
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'Delegate originates error',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var expectedErrorString;
            var expectedMessage;
            for (var err in errors) {
                logger.comment("Testing HRESULT: e_" + err);
                expectedErrorString = "Originate: e_" + err;
                expectedMessage = "Called delegate to originate e_" + err;
                try {
                    errorAccess.invokeDelegate(function (sender, msg) {
                        verify(msg, expectedMessage, "Message arg");
                        try {
                            errorAccess.originateError(errors[err], expectedErrorString);
                        } catch (e) {
                            verify(e.number, errors[err], "e.number");
                            verify(TestUtilities.GetRestrictedStringFromError(e), expectedErrorString, "TestUtilities.GetRestrictedStringFromError(e)");
                        }
                    }, expectedMessage);
                } catch (e) {
                    fail("Delegate execution should always pass");
                }
            }
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'Nested delegates',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var expectedErrorString;
            var expectedMessage;
            for (var err in errors) {
                logger.comment("Testing HRESULT: e_" + err);
                expectedErrorString = "Originate: e_" + err;

                var delegate2 = function (sender, msg) {
                    verify(msg, "Called delegate2", "Message arg");
                    try {
                        errorAccess.originateError(errors[err], expectedErrorString);
                    } catch (e) {
                        verify(e.number, errors[err], "e.number");
                        verify(TestUtilities.GetRestrictedStringFromError(e), expectedErrorString, "TestUtilities.GetRestrictedStringFromError(e)");
                    }
                }

                var delegate1 = function (sender, msg) {
                    verify(msg, "Called delegate1", "Message arg");
                    errorAccess.invokeDelegate(delegate2, "Called delegate2");
                }

                try {
                    errorAccess.invokeDelegate(delegate1, "Called delegate1");
                } catch (e) {
                    fail("Delegate execution should always pass");
                }
            }
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'Transform Error',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var originalString;
            var expectedString;
            for (var err in errors) {
                for (var errNew in errors) {
                    if (errors[err] !== errors[errNew]) {
                        logger.comment("Testing HRESULT: e_" + err + " transformed to HRESULT: e_" + errNew);
                        var systemErrorString = TestUtilities.GetSystemStringFromHr(errors[errNew]);
                        var restrictedErrorString = "Transform: e_" + err + " -> e_" + errNew;
                        expectedString = systemErrorString + "\r\n" + restrictedErrorString;
                        try {
                            errorAccess.transformError(errors[err], errors[errNew], systemErrorString, restrictedErrorString);
                        } catch (e) {
                            verify(e.number, errors[errNew], "e.number");
                            verify(e.message, expectedString, 'e.message');
                            verify(TestUtilities.GetRestrictedStringFromError(e), restrictedErrorString, "TestUtilities.GetRestrictedStringFromError(e)");
                        }
                    }
                }
            }
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'IErrorInfo and HRESULT do not match',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var restrictedString;
            for (var err in errors) {
                for (var errNew in errors) {
                    if (errors[err] !== errors[errNew]) {
                        logger.comment("Testing IErrorInfo with HRESULT: e_" + err + " mismatch with returned HRESULT: e_" + errNew);
                        restrictedString = "Originate: e_" + err;
                        try {
                            // Originate error but return different HRESULT
                            errorAccess.originateErrorReturnHr(errors[err], restrictedString, errors[errNew]);
                        } catch (e) {
                            // IErrorInfo should not be used when HRESULT does not match returned HRESULT
                            verify(e.number, errors[errNew], "e.number");
                            verify(e.message, TestUtilities.GetSystemStringFromHr(errors[errNew]), 'e.message');
                            verify(TestUtilities.GetRestrictedStringFromError(e), undefined, "TestUtilities.GetRestrictedStringFromError(e)");
                        }
                    }
                }
            }
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'Stale IErrorInfo left over from previous call',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var animal = isWebViewHost ? TestUtilities.AnimalToVar() : (new Animals.Animal());
            var restrictedString;
            for (var err in errors) {
                for (var errNew in errors) {
                    logger.comment("Testing stale IErrorInfo with HRESULT: e_" + err + " followed by call with returned HRESULT: e_" + errNew);
                    restrictedString = "Originate: e_" + err;
                    try {
                        // Originate an error but return S_OK -- IErrorInfo not used for this call
                        errorAccess.originateErrorReturnHr(errors[err], restrictedString, errorAccess.successCode);
                        // Call method to return failed HRESULT without overwriting ErrorInfo
                        animal.testError(errors[errNew]);
                    } catch (e) {
                        // Stale ErrorInfo should have been cleared before call to testError
                        verify(e.number, errors[errNew], "e.number");
                        verify(e.message, TestUtilities.GetSystemStringFromHr(errors[errNew]), 'e.message');
                        verify(TestUtilities.GetRestrictedStringFromError(e), undefined, "TestUtilities.GetRestrictedStringFromError(e)");
                    }
                }
            }
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'Originate and catch error with capability SID -- verify error object var and restricted values',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var errorObjectExpected = {
                description: "string",
                number: "number",
                stack: "string"
            };
            var expectedString;
            var capabilityString;

            for (var sid in capabilities) {
                logger.comment("Testing Capability: " + sid);
                expectedString = "Originate E_ACCESSDENIED with Capability SID: " + sid;
                capabilityString = errorAccess.getSidString(capabilities[sid]);

                try {
                    errorAccess.originateErrorWithCapabilitySid(errors.accessDenied, expectedString, capabilities[sid]);
                    fail("Expected error to be thrown");
                } catch (e) {
                    verify(e.number, errors.accessDenied, "e.number");
                    verify.members(e, errorObjectExpected, "e");
                    verify.doesNotContainValue(e, capabilityString, "e");
                    verify(TestUtilities.GetRestrictedStringFromError(e), expectedString, "TestUtilities.GetRestrictedStringFromError(e)");
                    verify(TestUtilities.GetCapabilitySidFromError(e), capabilityString, "TestUtilities.GetCapabilitySidFromError(e)");
                }
            }
        }
    });


    Loader42_FileName = "WinRT Error Tests"
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
