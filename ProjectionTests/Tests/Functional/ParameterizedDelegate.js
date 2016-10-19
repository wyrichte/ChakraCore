if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var setTimeOutCalls = 0;
    var savedSetTimeout = undefined;
    if (typeof setTimeout !== 'undefined')
        savedSetTimeout = setTimeout;
    runner.globalSetup(function () {
        setTimeout = function () { setTimeOutCalls = setTimeOutCalls + 1; }
    });
    runner.globalTeardown(function () {
        setTimeout = savedSetTimeout;
    });

    runner.addTest({
        id: '0',
        desc: 'BLUE: 178785 - Unprojected parameterized delegate marks parent delegate as unprojected',
        pri: '0',
        test: function () {
            var eventhandler;

            logger.comment("First trigger Expr construction for the hidden parameterized delegate...");

            try {
                eventhandler = Winery.RWinery.getHiddenHandler();
            } catch(e) {
                logger.comment("Failed to get hidden parameterized delegate - expected. " + e.message);
            }

            logger.comment("Now trigger Expr construction for the non-hidden parameterized delegate...");

            eventhandler = Winery.RWinery.getVisibleHandler();
        }
    });

    Loader42_FileName = "Parameterized delegate tests";
})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
