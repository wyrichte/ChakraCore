if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    /// This tests the Winery sample end to end

    /// Create the winery ABI used through the test
    var winery;

    runner.globalSetup(function () {
        winery = new Winery.RWinery(1);
    });

    /// Test the IGeneralShop interface

    runner.addTest({
        id: 1,
        desc: 'IGeneralShop.shopArea',
        pri: '0',
        test: function () {
            verify(("shopArea" in winery), true, "Winery has property shopArea");
            winery.shopArea = { width: 100, length: 120 };
            verify(winery.shopArea.width, 100, 'winery.shopArea.width');
            verify(winery.shopArea.length, 120, 'winery.shopArea.length');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'IGeneralShop.shopDimension',
        pri: '0',
        test: function () {
            verify(("shopDimension" in winery), true, "Winery has property shopDimension");
            winery.shopDimension = { baseArea: { width: 100, length: 120 }, height: 1.1 };
            verify(winery.shopDimension.baseArea.width, 100, 'winery.shopDimension.baseArea.width');
            verify(winery.shopDimension.baseArea.length, 120, 'winery.shopDimension.baseArea.length');
            verify(winery.shopDimension.height, 1.1, 'winery.shopDimension.height');
        }
    });

    runner.addTest({
        id: 3,
        desc: 'IGeneralShop.shopName',
        pri: '0',
        test: function () {
            verify(("shopName" in winery), true, "Winery has property shopName");
            var shopName = 'Delille Cellars';
            winery.shopName = shopName;
            verify(winery.shopName, shopName, 'winery.shopName');
        }
    });

    /// Test the GWineFactory interface group

    /// Test the IWarehouse interface

    runner.addTest({
        id: 4,
        desc: 'IWarehouse',
        pri: '0',
        test: function () {
            winery.clearWarehouse();
            winery.storeAgedWine();
            verify(winery.wineInStorage, 1, 'winery.wineInStorage');
            winery.clearWarehouse();
            verify(winery.wineInStorage, 0, 'winery.wineInStorage');
        }
    });


    /// Test the IProductionLine interface

    runner.addTest({
        id: 5,
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


    /// Test the IWineRetail interface
    runner.addTest({
        id: 6,
        desc: 'IWineRetail',
        pri: '0',
        test: function () {
            winery.initDatabase();
            verify(("welcomeMessage" in winery), true, "Winery has property welcomeMessage");
            var message = "Welcome to Delille Cellars in Woodinville!";
            winery.welcomeMessage = message
            verify(winery.welcomeMessage, message, 'winery.welcomeMessage');

            winery.sellReds(Winery.reds.pinotNoir, 20);
            winery.sellReds(Winery.reds.merlot, 1);
            verify(winery.getBestSellingRed(), Winery.reds.pinotNoir, 'winery.getBestSellingRed()');

            winery.sellWhites(Winery.whites.sauvignonBlanc, 35);
            verify(winery.getBestSellingWhite(), Winery.whites.sauvignonBlanc, 'winery.getBestSellingWhite()');

            winery.sellSweets(Winery.sweets.riesling, 1);
            verify(winery.getBestSellingSweet(), Winery.sweets.riesling, 'winery.getBestSellingSweet()');
        }
    });

    Loader42_FileName = 'Winery tests';

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
