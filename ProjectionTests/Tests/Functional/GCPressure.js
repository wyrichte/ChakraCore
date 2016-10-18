if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    var GCPressure = {
        none: -1,
        low: 0,
        medium: 1,
        high: 2
    };

    runner.addTest({
        id: 1,
        desc: 'Associated GC pressure of class without attribute',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var animal = new Animals.Animal();

            verify(TestUtilities.GetMemoryFootprintOfRC("Animals.Animal"), GCPressure.none, "Memory Footprint of Animals.Animal");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Associated GC pressure of class with attribute value "Low"',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var smallClass = new DevTests.GCPressure.SmallClass();

            verify(TestUtilities.GetMemoryFootprintOfRC("DevTests.GCPressure.SmallClass"), GCPressure.low, "Memory Footprint of DevTests.GCPressure.SmallClass");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Associated GC pressure of class with attribute value "Medium"',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var smallClass = new DevTests.GCPressure.MediumClass();

            verify(TestUtilities.GetMemoryFootprintOfRC("DevTests.GCPressure.MediumClass"), GCPressure.medium, "Memory Footprint of DevTests.GCPressure.MediumClass");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Associated GC pressure of class with attribute value "High"',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var smallClass = new DevTests.GCPressure.LargeClass();

            verify(TestUtilities.GetMemoryFootprintOfRC("DevTests.GCPressure.LargeClass"), GCPressure.high, "Memory Footprint of DevTests.GCPressure.LargeClass");
        }
    });

    Loader42_FileName = "GCPressureAttribute consumption tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
