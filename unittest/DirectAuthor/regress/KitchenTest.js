(function () {
    'use strict'

    /// This tests the Kitchen Sink sample end to end

    /// Create the Kitchen ABIs used through this test
    var myKitchen;
    var name = 'Sen';
    var chef;
    var toaster;
    var oven;!R:!+:function!!!R:

    runner.globalSetup(function () {
        myKitchen = new Fabrikam.Kitchen.Kitchen();
        chef = Fabrikam.Kitchen.Chef.CreateChef(name, myKitchen);
        toaster = new Fabrikam.Kitchen.Toaster();
        oven = new Fabrikam.Kitchen.Oven();
    });


    /// Create a Chef
    runner.addTest({
        id: 1,
        desc: 'CreateChef',
        pri: '0',
        test: function () {
            verify(chef.Name, name, 'chef.Name');
        }
    });

    /// Set the role on Chef
    runner.addTest({
        id: 2,
        desc: 'GetSetChefRole',
        pri: '0',
        test: function () {
            verify(chef.Role, Fabrikam.Kitchen.ChefRole.AssistantChef, 'chef.Role');
            chef.Role = Fabrikam.Kitchen.ChefRole.HeadChef;
            verify(chef.Role, Fabrikam.Kitchen.ChefRole.HeadChef, 'chef.Role');
        }
    });

    /// Set the capabilities on Chef
    runner.addTest({
        id: 3,
        desc: 'GetSetChefCapabilities',
        pri: '0',
        test: function () {
            verify(chef.Capabilities, Fabrikam.Kitchen.ChefCapabilities.CanSlice, 'chef.Capabilities');
            chef.Capabilities = Fabrikam.Kitchen.ChefCapabilities.CanDice;
            verify(chef.Capabilities, Fabrikam.Kitchen.ChefCapabilities.CanDice, 'chef.Capabilities');
        }
    });


    /// Make the breakfast toaster
    runner.addTest({
        id: 4,
        desc: 'MakeBreakfastToaster',
        pri: '0',
        test: function () {
            var toasterCost = chef.MakeBreakfastToaster(toaster);
            verify(toasterCost, 10, 'cost from MakeBreakfastToaster');
        }
    });

    /// Make the breakfast toaster via MakeBreakfastToastInt
    runner.addTest({
        id: 5,
        desc: 'MakeBreakfastToasterInt',
        pri: '0',
        test: function () {
            var toasterCost = chef.MakeBreakfastToasterInt(toaster, 5);
            verify(toasterCost, 50, 'cost from MakeBreakfastToasterInt');
        }
    });

    /// Make the breakfast toaster via MakeBreakfastToastDouble
    runner.addTest({
        id: 6,
        desc: 'MakeBreakfastToasterDouble',
        pri: '0',
        test: function () {
            var toasterCost = chef.MakeBreakfastToasterInt(toaster, 8);
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

            var onToastCompleted = function (sender, toast) {
                verify(toast.Message, toastMessage + eventsHandled, 'toast.Message');
                eventsHandled += 1;
            }

            toaster.addEventListener('ToastCompleteEvent', onToastCompleted);
            toaster.MakeToast(toastMessage + '0');
            toaster.MakeToast(toastMessage + '1');
            toaster.removeEventListener('ToastCompleteEvent', onToastCompleted);
            toaster.MakeToast(toastMessage + '2');

            verify(eventsHandled, 2, 'Number of events handled');
        }
    });

    /// Test the methods on oven
    runner.addTest({
        id: 8,
        desc: 'OvenTest',
        pri: '0',
        test: function () {
            var bakeOperation = oven.BakeAsync(15);

            /// Hook up to the async events - these currently won't hit because there is no message pump in jshost.exe
            bakeOperation.OnProgress = function (async, time) { fail('Should not have handled bakeOperation.OnProgress'); }
            bakeOperation.OnComplete = function (async, time) { fail('Should not have handled bakeOperation.OnComplete'); }

            bakeOperation.Start();
            bakeOperation.Cancel();
            bakeOperation.Close();

            logger.comment('Finished calling aysnc operations');
        }
    });

    Run('TeachRT tests');
})();