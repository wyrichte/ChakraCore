if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    function simpleInterfaceGet() {
        logger.comment("Create myFish");
        var myFish = new Animals.Fish();
        verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");

        logger.comment("Get mySameFish same as myFish");
        var mySameFish = Animals.Animal.sendBackSameFish(myFish);
        verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");
        verify(Animals.Animal.getRefCount(mySameFish), 2, "mySameFish refCount");

        logger.comment("Set the myFish to null and perform gc");
        myFish = null;
        logger.comment("Perform gc");
        CollectGarbage();
        verify(Animals.Animal.getRefCount(mySameFish), 2, "mySameFish refCount");

        logger.comment("Set mySameFish to winrt");
        Animals.Animal.myFish = mySameFish;
        verify(Animals.Animal.getRefCount(mySameFish), 3, "mySameFish refCount");
    }

    function createFish() {
        logger.comment("Create myFish");
        var myFish = new Animals.Fish();
        verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");

        logger.comment("Set Fish into the animal");
        Animals.Animal.myFish = myFish;
        verify(Animals.Animal.getRefCount(myFish), 3, "myFish refCount");
    }

    runner.addTest({
        id: 1,
        desc: 'SimpleInterfaceGet',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            // Do the interface tests
            simpleInterfaceGet();

            logger.comment("Perform gc and check if all the vars of myFish are released");
            CollectGarbage();
            verify(Animals.Animal.myFishRefCount, 1, "Animals.Animal.myFishRefCount");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'MethodInvoke',
        pri: '0',
        test: function () {
            logger.comment("Create myFish");
            var myFish = new Animals.Fish();
            verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");

            logger.comment("Call getNumFins");
            verify(myFish.getNumFins(), 5, "myFish.getNumFins()");
            verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'PropertyGetAndSet',
        pri: '0',
        test: function () {
            logger.comment("Create myFish");
            var myFish = new Animals.Fish();
            verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");

            logger.comment("Set property");
            myFish.name = "Nemo"
            verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");

            logger.comment("Get property");
            verify(myFish.name, "Nemo", "myFish.name");
            verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'DelegateInvoke',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            createFish();

            logger.comment("Perform gc");
            CollectGarbage();
            verify(Animals.Animal.myFishRefCount, 1, "Animals.Animal.myFishRefCount");

            var returnFish = false;
            function myDelegate(delegateMyFish) {
                logger.comment("*** myDelegate: Invoke");

                verify(Animals.Animal.getRefCount(delegateMyFish), 4, "delegateMyFish refCount");

                logger.comment("*** myDelegate: Exit");

                if (returnFish) {
                    return delegateMyFish;
                }
                return null;
            }

            logger.comment("Call myDelegate: return null");
            this.myOutFish = Animals.Animal.callDelegateWithFish(myDelegate);

            logger.comment("Perform gc");
            CollectGarbage();
            verify(Animals.Animal.myFishRefCount, 1, "Animals.Animal.myFishRefCount");

            logger.comment("Call myDelegate: return Fish");
            returnFish = true;
            this.myOutFish = Animals.Animal.callDelegateWithFish(myDelegate);

            logger.comment("Perform gc");
            CollectGarbage();
            verify(Animals.Animal.myFishRefCount, 3, "Animals.Animal.myFishRefCount");

            logger.comment("Set the myOutFish to null and check the myFish of winrt is the only one holding reference");
            this.myOutFish = null;
            logger.comment("Perform gc");
            CollectGarbage();
            verify(Animals.Animal.myFishRefCount, 1, "Animals.Animal.myFishRefCount");
        }
    });

    runner.addTest({
        id: 5,
        desc: 'EventInvoke',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            logger.comment("Create toaster");
            var msg = "Toast Complete Msg";
            var toaster = new Fabrikam.Kitchen.Toaster();
            verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

            function toastCompleteCallback(ev) {
                logger.comment("*** toastCompleteCallback: Enter");
                verify(ev.message, msg, 'ev.message');
                var targetCount = Animals.Animal.getRefCount(ev.target);
                verify(targetCount == 3 || // Fast-sig does one less QI
                       targetCount == 4,   // Slow-sig
                       true, "toaster refCount inside event handler : ev.target");
                verify(Animals.Animal.getRefCount(ev.detail[0]), 7, "toast refCount inside event handler: ev.detail[0]");
                verify(Animals.Animal.getRefCount(ev), 7, "toast refCount inside event handler: ev");

                logger.comment("*** toastCompleteCallback: Exit");
            }

            logger.comment("Add event listner");
            toaster.addEventListener("toastcompleteevent", toastCompleteCallback);
            verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

            var toast = toaster.makeToast(msg);
            verify(toast.message, msg, 'toast.message');

            logger.comment("Perform gc");
            CollectGarbage();

            verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount after event handler invoke");
            verify(Animals.Animal.getRefCount(toast), 2, "toast refCount after event handler invoke");

            logger.comment("Remove event listner");
            toaster.removeEventListener("toastcompleteevent", toastCompleteCallback);

            logger.comment("Perform gc");
            CollectGarbage();

            verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount after event handler invoke");
            verify(Animals.Animal.getRefCount(toast), 2, "toast refCount after event handler invoke");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Inspectable_Out: Get inspectable out that has fails on GRCN',
        pri: '0',
        test: function () {
            logger.comment("Create myFish");
            var myFish = new Animals.Fish();
            verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");

            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testFailingRuntimeClassNameWithAnotherInterface(myFish);
            }, TypeError, "propertyValueTests.testFailingRuntimeClassNameWithAnotherInterface(myFish);");

            verify(Animals.Animal.getRefCount(myFish), 2, "myFish refCount");
        }
    });

    Loader42_FileName = 'Memory Leaks Tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
