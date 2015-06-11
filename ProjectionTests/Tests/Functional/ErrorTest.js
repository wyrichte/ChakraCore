if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
// HRESULT values covered by this test:
//  S_OK            Operation successful                0x00000000
//  E_ABORT         Operation aborted                   0x80004004 
//  E_ACCESSDENIED  General access denied error         0x80070005 
//  E_FAIL          Unspecified failure                 0x80004005 
//  E_HANDLE        Handle that is not valid            0x80070006 
//  E_INVALIDARG    One or more arguments are not valid 0x80070057 
//  E_NOINTERFACE   No such interface supported         0x80004002 
//  E_NOTIMPL       Not implemented                     0x80004001 
//  E_OUTOFMEMORY   Failed to allocate necessary memory 0x8007000E 
//  E_POINTER       Pointer that is not valid           0x80004003 
//  E_UNEXPECTED    Unexpected failure                  0x8000FFFF 


(function () {

    runner.addTest({
        id: 1,
        desc: 'Error test',
        pri: '0',
        test: function () {

            var hresults = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes;

            var errorMessages = {
                abort: 'Operation aborted\n',
                accessDenied: 'Access is denied.\n',
                fail: 'Unspecified error\n',
                handle: 'The handle is invalid.\n',
                invalidArg: 'The parameter is incorrect.\n',
                noInterface: 'No such interface supported\n',
                notImpl: 'Not implemented\n',
                outOfMemory: 'Not enough storage is available to complete this operation.\n',
                pointer: 'Invalid pointer\n',
                unexpected: 'Catastrophic failure\n'
            };

            var myAnimal = new Animals.Animal(1);
            for (var error in hresults) {
                logger.comment("Test marshaling of HRESULT: e_" + error);
                try {
                    myAnimal.testError(hresults[error]);
                }
                catch (e) {
                    verify.instanceOf(e, WinRTError);
                    verify(e.number, hresults[error], 'Error number');
                    verify(e.name, 'WinRTError', 'Error name');
                    if (typeof TestUtilities !== 'undefined') {
                        verify(e.message, TestUtilities.GetSystemStringFromHr(hresults[error]), 'Error message');
                    }
                }

            }
        }
    });

    runner.addTest({
        id: 2,
        desc: 'WinRTError constructor test',
        pri: '0',
        test: function () {
            var message = "This is a WinRT error";

            try {
                throw new WinRTError(message);
            } catch (e) {
                verify.instanceOf(e, WinRTError);
                verify(e.name, 'WinRTError', 'Error name');
                verify(e.message, message, 'Error message');
            }
        }
    });
    Loader42_FileName = "Error test";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
