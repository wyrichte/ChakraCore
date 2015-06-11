if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
    var console = {warn : WScript.Echo };
(function () {
    var winery;
    runner.globalSetup(function () {
        winery = new Winery.RWinery(1);
    });
    runner.addTest({
        id:1,
        desc: "Deprecated attributes",
        pri:0,
        test: function () {
            WScript.Echo("calling getDeprecatedAttribute");
            var c = winery.getDeprecatedAttributes();
            WScript.Echo("calling amazingMethods");
            c.amazingMethod(1);
            WScript.Echo("calling exceptionalMethod");
            c.exceptionalMethod("hello");
            WScript.Echo("get exceptionProp");
            var data = c.exceptionalProp;
            c.exceptionalProp = data;
        }
    });

    runner.addTest({
        id:2,
        desc: "Deprecated runtimeclass",
        pri:0,
        test: function () {
            var d  = new Winery.RWinery(2);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'IProductionLine',
        pri: '0',
        test: function () {
            var eventsHandled = 0;
            var onAgeComplete = function (sender, warehouse) { eventsHandled += 1; }
            winery.addEventListener("agecompleteevent", onAgeComplete);
            winery.produce();
            winery.sendToWarehouse(winery); // We should get this event
            winery.removeEventListener("agecompleteevent", onAgeComplete);
            winery.sendToWarehouse(winery); // We should not get this event
            verify(eventsHandled, 1, 'Number of events handled');
        }
    });
    runner.addTest({
        id:4,
        desc: "Deprecated propget",
        pri:0,
        test: function () {
            var d  = DevTests.Delegates.StaticTestClass.exceptionalPropStatic;
            WScript.Echo(d);
        }
    });

    runner.addTest({
        id:5,
        desc: "Deprecated propset",
        pri:0,
        test: function () {
            DevTests.Delegates.StaticTestClass.exceptionalPropStatic = 'hello';
        }
    });

    runner.addTest({
        id:6,
        desc: "Deprecated static",
        pri:0,
        test: function () {
            var d  = DevTests.Delegates.StaticTestClass.operationOutStatic();
            WScript.Echo(d);
        }
    });

    Loader42_FileName = 'deprecated tests';

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
