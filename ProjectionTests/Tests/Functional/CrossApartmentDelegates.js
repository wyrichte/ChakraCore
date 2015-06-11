if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    runner.addTest({
        id: 1,
        desc: 'Basic',
        pri: '0',
        test: function () {
            var callbackCalled = false;
            function preheatCompleteCallback() {
                callbackCalled = true;
            }

            var toaster = new Fabrikam.Kitchen.Toaster();
            logger.comment('Fabrikam.Kitchen.Toaster created successfully');
            logger.comment('Calling toaster.preheatInBackground');
            toaster.preheatInBackground(preheatCompleteCallback);
            logger.comment('toaster.preheatInBackground done');
            verify(callbackCalled, true, 'Callback was called');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Calling Delegate on wrong thread',
        pri: '0',
        test: function () {
            var callbackCalled = false;
            function preheatCompleteCallback() {
                callbackCalled = true;
            }

            var toaster = new Fabrikam.Kitchen.Toaster();
            logger.comment('Fabrikam.Kitchen.Toaster created successfully');
            logger.comment('Calling toaster.preheatInBackgroundWithSmuggledDelegate');
            verify.exception(function () {
                toaster.preheatInBackgroundWithSmuggledDelegate(preheatCompleteCallback);
            }, WinRTError, "Calling Delegate on wrong thread should throw exception");
            logger.comment('toaster.preheatInBackground done');
            verify(callbackCalled, false, 'Callback was called');
        }
    });

    Loader42_FileName = 'Cross apartment delegate test';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
