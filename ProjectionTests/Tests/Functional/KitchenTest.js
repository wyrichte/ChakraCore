if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    /// This tests the Kitchen Sink sample end to end

    /// Create the Kitchen ABIs used through this test
    var myKitchen;
    var name = 'Sen';
    var chef;
    var toaster;
    var oven;

    runner.globalSetup(function () {
        myKitchen = new Fabrikam.Kitchen.Kitchen();
        chef = new Fabrikam.Kitchen.Chef(name, myKitchen);
        toaster = new Fabrikam.Kitchen.Toaster();
        oven = new Fabrikam.Kitchen.Oven();
    });


    /// Create a Chef
    runner.addTest({
        id: 1,
        desc: 'CreateChef',
        pri: '0',
        test: function () {
            verify(chef.name, name, 'chef.name');
        }
    });

    /// Set the role on Chef
    runner.addTest({
        id: 2,
        desc: 'GetSetChefRole',
        pri: '0',
        test: function () {
            verify(chef.role, Fabrikam.Kitchen.ChefRole.assistantChef, 'chef.role');
            chef.role = Fabrikam.Kitchen.ChefRole.headChef;
            verify(chef.role, Fabrikam.Kitchen.ChefRole.headChef, 'chef.role');
        }
    });

    /// Set the capabilities on Chef
    runner.addTest({
        id: 3,
        desc: 'GetSetChefCapabilities',
        pri: '0',
        test: function () {
            verify(chef.capabilities, Fabrikam.Kitchen.ChefCapabilities.canSlice, 'chef.capabilities');
            chef.capabilities = Fabrikam.Kitchen.ChefCapabilities.canDice;
            verify(chef.capabilities, Fabrikam.Kitchen.ChefCapabilities.canDice, 'chef.capabilities');
        }
    });


    /// Make the breakfast toaster
    runner.addTest({
        id: 4,
        desc: 'MakeBreakfastToaster',
        pri: '0',
        test: function () {
            var toasterCost = chef.makeBreakfast(toaster);
            verify(toasterCost, 10, 'cost from MakeBreakfastToaster');
        }
    });

    /// Make the breakfast toaster via MakeBreakfastToastInt
    runner.addTest({
        id: 5,
        desc: 'MakeBreakfastToasterInt',
        pri: '0',
        test: function () {
            var toasterCost = chef.makeBreakfast(toaster, 5);
            verify(toasterCost, 50, 'cost from MakeBreakfastToasterInt');
        }
    });

    /// Make the breakfast toaster via MakeBreakfastToastDouble
    runner.addTest({
        id: 6,
        desc: 'MakeBreakfastToasterDouble',
        pri: '0',
        test: function () {
            var toasterCost = chef.makeBreakfast(toaster, 8);
            verify(toasterCost, 80, 'cost from MakeBreakfastToasterDouble');
        }
    });

    /// Hook up the toast completed handler
    runner.addTest({
        id: 7,
        desc: 'ToastCompleteEvent',
        pri: '0',
        test: function () {
            var eventsHandled = 0;
            var toastMessage = 'Toast';

            var onToastCompleted = function (ev) {
                verify(ev.message, toastMessage + eventsHandled, 'ev.message');
                eventsHandled += 1;
            }

            toaster.addEventListener('toastcompleteevent', onToastCompleted);
            toaster.makeToast(toastMessage + '0');
            toaster.makeToast(toastMessage + '1');
            toaster.removeEventListener('toastcompleteevent', onToastCompleted);
            toaster.makeToast(toastMessage + '2');

            verify(eventsHandled, 2, 'Number of events handled');
        }
    });

    /// Test the methods on oven
    runner.addTest({
        id: 8,
        desc: 'OvenTest',
        pri: '0',
        test: function () {
            var bakeOperation = oven.bakeAsync(15).operation;

            /// Hook up to the async events - these currently won't hit because there is no message pump in jshost.exe
            bakeOperation.Progress = function (async, time) { fail('Should not have handled bakeOperation.Progress'); }
            bakeOperation.Complete = function (async, time) { fail('Should not have handled bakeOperation.Complete'); }

            bakeOperation.cancel();
            bakeOperation.close();

            logger.comment('Finished calling aysnc operations');
        }
    });

    Loader42_FileName = 'TeachRT tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
