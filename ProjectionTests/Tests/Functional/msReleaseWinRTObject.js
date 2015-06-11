if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var root = this;

    runner.addTest({
        id: 1,
        desc: 'Verify msReleaseWinRTObject function exists on the global object',
        pri: '0',
        test: function () {
            verify.defined(msReleaseWinRTObject, "msReleaseWinRTObject");
            verify.typeOf(msReleaseWinRTObject, 'function');

            var desc = Object.getOwnPropertyDescriptor(root, "msReleaseWinRTObject");
            var attributesExpected = {
                writable: true,
                enumerable: false,
                configurable: true
            };
            logger.comment("Verify attributes of msReleaseWinRTObject");
            for (var attrib in attributesExpected) {
                verify(desc[attrib], attributesExpected[attrib], attrib);
            }
            verify(msReleaseWinRTObject.length, 1, "msReleaseWinRTObject.length");
            verify(msReleaseWinRTObject(), undefined, "Call to msReleaseWinRTObject with 0 arguments should succeed");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Verify ref count after release is decreased',
        pri: '0',
        test: function () {
            // CLR manage it's own refcount for the m_fish.
            var refCountBefore = typeof Animals._CLROnly === 'undefined' ? 3 : 2;
            var refCountAfter  = typeof Animals._CLROnly === 'undefined' ? 1 : 0;
            var myFish = new Animals.Fish();
            Animals.Animal.myFish = myFish;
            verify(Animals.Animal.myFishRefCount, refCountBefore, "Animals.Animal.myFishRefCount");

            msReleaseWinRTObject(myFish);

            verify(Animals.Animal.myFishRefCount, refCountAfter, "Animals.Animal.myFishRefCount");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Releasing already released object',
        pri: '0',
        test: function () {
            // CLR manage it's own refcount for the m_fish.
            var refCountBefore = typeof Animals._CLROnly === 'undefined' ? 3 : 2;
            var refCountAfter = typeof Animals._CLROnly === 'undefined' ? 1 : 0;
            var myFish = new Animals.Fish();
            Animals.Animal.myFish = myFish;
            verify(Animals.Animal.myFishRefCount, refCountBefore, "Animals.Animal.myFishRefCount");

            msReleaseWinRTObject(myFish);

            verify(Animals.Animal.myFishRefCount, refCountAfter, "Animals.Animal.myFishRefCount");

            verify.exception(function () {
                msReleaseWinRTObject(myFish);
            }, ReferenceError, "Release call after calling release once");

            verify(Animals.Animal.myFishRefCount, refCountAfter, "Animals.Animal.myFishRefCount");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Verify method call after release fails',
        pri: '0',
        test: function () {
            var animal = new Animals.Animal(4);
            verify(animal.addInts(10, 30), 40, "animal.addInts(10, 30)");

            msReleaseWinRTObject(animal);

            verify.exception(function () {
                animal.addInts(10, 30);
            }, ReferenceError, "Method call after release");
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Verify property get after release fails',
        pri: '0',
        test: function () {
            var animal = new Animals.Animal(4);
            verify(animal.weight, 50, "animal.weight");

            msReleaseWinRTObject(animal);

            verify.exception(function () {
                var propGetVal = animal.weight;
            }, ReferenceError, "Property get after release");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Verify property set after release fails',
        pri: '0',
        test: function () {
            var animal = new Animals.Animal(4);
            verify(animal.weight, 50, "animal.weight");
            animal.weight = 40;
            verify(animal.weight, 40, "animal.weight");

            msReleaseWinRTObject(animal);

            verify.exception(function () {
                animal.weight = 60;
            }, ReferenceError, "Property set after release");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Verify adding event handler after release fails',
        pri: '0',
        test: function () {
            var toaster = new Fabrikam.Kitchen.Toaster();

            var eventCount = 0;
            function callback() {
                logger.comment("Callback : Invoke");
                eventCount++;
                logger.comment("Callback : Exit");
            }

            toaster.addEventListener("toastcompleteevent", callback);
            toaster.makeToast("myToast");
            verify(eventCount, 1, "eventCount");

            msReleaseWinRTObject(toaster);

            eventCount = 0;
            var eventCount2 = 0;
            function callback2() {
                logger.comment("Callback2 : Invoke");
                eventCount2++;
                logger.comment("Callback2 : Exit");
            }

            verify.exception(function () {
                toaster.addEventListener("toastcompleteevent", callback2);
            }, ReferenceError, "addEventListener after release");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Verify remove event handler after release fails',
        pri: '0',
        test: function () {
            var toaster = new Fabrikam.Kitchen.Toaster();

            var eventCount = 0;
            function callback() {
                logger.comment("Callback : Invoke");
                eventCount++;
                logger.comment("Callback : Exit");
            }

            toaster.addEventListener("toastcompleteevent", callback);
            toaster.makeToast("myToast1");
            verify(eventCount, 1, "eventCount");

            eventCount = 0;
            toaster.removeEventListener("toastcompleteevent", callback);
            toaster.makeToast("myToast2");
            verify(eventCount, 0, "eventCount");

            toaster.addEventListener("toastcompleteevent", callback);
            toaster.makeToast("myToast3");
            verify(eventCount, 1, "eventCount");

            msReleaseWinRTObject(toaster);

            verify.exception(function () {
                toaster.removeEventListener("toastcompleteevent", callback);
            }, ReferenceError, "removeEventListener after release");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Verify setting event handler after release fails',
        pri: '0',
        test: function () {
            var toaster = new Fabrikam.Kitchen.Toaster();

            var eventCount = 0;
            function callback() {
                logger.comment("Callback : Invoke");
                eventCount++;
                logger.comment("Callback : Exit");
            }

            toaster.ontoastcompleteevent = callback;
            toaster.makeToast("myToast1");
            verify(eventCount, 1, "eventCount");

            eventCount = 0;
            var eventCount2 = 0;
            function callback2() {
                logger.comment("Callback2 : Invoke");
                eventCount2++;
                logger.comment("Callback2 : Exit");
            }

            msReleaseWinRTObject(toaster);

            verify.exception(function () {
                toaster.ontoastcompleteevent = callback2;
            }, ReferenceError, "Setting event handler after release");
        }
    });

    runner.addTest({
        id: 10,
        desc: 'Verify setting event handler to null after release fails',
        pri: '0',
        test: function () {
            var toaster = new Fabrikam.Kitchen.Toaster();

            var eventCount = 0;
            function callback() {
                logger.comment("Callback : Invoke");
                eventCount++;
                logger.comment("Callback : Exit");
            }

            toaster.ontoastcompleteevent = callback;
            toaster.makeToast("myToast1");
            verify(eventCount, 1, "eventCount");

            eventCount = 0;
            toaster.ontoastcompleteevent = null;
            toaster.makeToast("myToast2");
            verify(eventCount, 0, "eventCount");

            toaster.ontoastcompleteevent = callback;
            toaster.makeToast("myToast3");
            verify(eventCount, 1, "eventCount");

            msReleaseWinRTObject(toaster);

            verify.exception(function () {
                toaster.ontoastcompleteevent = null;
            }, ReferenceError, "Setting event handler to null after release");
        }
    });

    runner.addTest({
        id: 11,
        desc: 'Verify getting event handler after release fails',
        pri: '0',
        test: function () {
            var toaster = new Fabrikam.Kitchen.Toaster();

            var eventCount = 0;
            function callback() {
                logger.comment("Callback : Invoke");
                eventCount++;
                logger.comment("Callback : Exit");
            }

            toaster.ontoastcompleteevent = callback;
            toaster.makeToast("myToast1");
            verify(eventCount, 1, "eventCount");
            verify(toaster.ontoastcompleteevent, callback, "toaster.ontoastcompleteevent");

            msReleaseWinRTObject(toaster);

            verify.exception(function () {
                var toastCompleteEventHandler = toaster.ontoastcompleteevent;
            }, ReferenceError, "Getting event handler after release");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Verify getting null event handler after release fails',
        pri: '0',
        test: function () {
            var toaster = new Fabrikam.Kitchen.Toaster();
            verify(toaster.ontoastcompleteevent, null, "toaster.ontoastcompleteevent");

            msReleaseWinRTObject(toaster);

            verify.exception(function () {
                var toastCompleteEventHandler = toaster.ontoastcompleteevent;
            }, ReferenceError, "Getting event handler after release");
        }
    });

    runner.addTest({
        id: 13,
        desc: 'Verify released object as in parameter fails',
        pri: '0',
        test: function () {
            var oneFish = new Animals.Fish();
            var twoFish = new Animals.Fish();
            var result = oneFish.marshalIFish(twoFish);
            verify(result, twoFish, "oneFish.marshalIFish(twoFish)");

            msReleaseWinRTObject(twoFish);

            verify.exception(function () {
                result = oneFish.marshalIFish(twoFish);
            }, ReferenceError, "Passing released object as in parameter");
        }
    });

    runner.addTest({
        id: 14,
        desc: 'Identity and released objects',
        pri: '0',
        test: function () {
            var a = new Animals.Fish();
            Animals.Animal.myFish = a;

            logger.comment("Verify identity of round-tripped runtimeclass");
            var b = Animals.Animal.myFish;
            verify.strictEqual(a, b, "a === b");
            verify(a.getNumFins(), 5, "a.getNumFins()");
            logger.comment("a.setNumFins(22)");
            a.setNumFins(22);
            verify(b.getNumFins(), 22, "b.getNumFins()");

            logger.comment("Release original object");
            msReleaseWinRTObject(a);
            CollectGarbage();

            verify.exception(function () {
                a.getNumFins();
            }, ReferenceError, "Method call after release");

            verify.exception(function () {
                b.getNumFins();
            }, ReferenceError, "Method call after release");

            logger.comment("Verify round-tripped runtimeclass");
            var c = Animals.Animal.myFish;
            
            try {
                verify(a !== c, true, "a !== c");
                verify.notStrictEqual(a, c, "a !== c");
            } catch(e) {
                logger.comment("Expected exception caught: " + e.message);
            }
            
            verify(c.getNumFins(), 22, "c.getNumFins()");
        }
    });

    Loader42_FileName = 'Release Tests'
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
