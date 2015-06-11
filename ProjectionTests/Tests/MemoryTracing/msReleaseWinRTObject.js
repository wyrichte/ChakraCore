if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var root = this;

    function garbageCollection() {
        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }

    function releaseAndVerifyRefCount() {
        var myFish = new Animals.Fish();
        Animals.Animal.myFish = myFish;
        verify(Animals.Animal.myFishRefCount, 3, "Animals.Animal.myFishRefCount");

        msReleaseWinRTObject(myFish);

        verify(Animals.Animal.myFishRefCount, 1, "Animals.Animal.myFishRefCount");
    }

    runner.addTest({
        id: 1,
        desc: 'Verify ref count after release is decreased',
        pri: '0',
        test: function () {
            releaseAndVerifyRefCount();

            garbageCollection();
        }
    });

    Loader42_FileName = 'Release Memory Tracing Tests'
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
