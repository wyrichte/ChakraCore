if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    function easyMembersPrint(myObjectString, myObject) {
        var objectDump = "\n    var " + myObjectString + "Members = [";
        for (p in myObject) {
            if (typeof myObject[p] == 'function') {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myObject[p] + '\', ' + myObject[p].length + '],'
            }
            else {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myObject[p] + '\'],';
            }
        }
        objectDump = objectDump + "\n    ];";

        return objectDump;
    }

    function verifyMembers(actual, expected, expectedType) {
        logger.comment("Object dump:" + easyMembersPrint("objectDump", actual));
        verify(typeof actual, expectedType, 'type');
        var i = 0;
        for (p in actual) {
            verify(p, expected[i][0], 'name');
            verify(typeof actual[p], expected[i][1], 'typeof ' + actual + '[' + p + ']');

            if (typeof actual[p] == 'function') {
                verify(actual[p].length, expected[i][2], p + ".length");
                logger.comment('Setting length of function to be 10');
                actual[p].length = 10;
                verify(actual[p].length, expected[i][2], p + ".length");
            }

            i++;
        }

        verify(i, expected.length, 'number of members');
    }

    // name, type, length if function
    var toasterMembers = [
        ['addEventListener', 'function', 2],
        ['electricityReporter', 'object'],
        ['getSameToaster', 'function', 0],
        ['indirectMakeToast', 'function', 1],
        ['indirectToaster', 'object'],
        ['invokePreheatCompleteBackgroundEvents', 'function', 1],
        ['invokeRootedHandler', 'function', 2],
        ['makeToast', 'function', 1],
        ['onindirecttoastcompleteevent', 'object'],
        ['onpreheatcompletebackground', 'object'],
        ['onpreheatstart', 'object'],
        ['onrootedtoastcompleteevent', 'object'],
        ['ontoastcompleteevent', 'object'],
        ['ontoaststartevent', 'object'],
        ['preheatInBackground', 'function', 1],
        ['preheatInBackgroundWithSmuggledDelegate', 'function', 1],
        ['removeEventListener', 'function', 2],
        ['rootedHandler', 'object'],
        ['size', 'object'],
    ];

    var kitchenMembers = [
    ];

    // name, type, length if function
    var chefMembers = [
        ['addEventListener', 'function', 2],
        ['capabilities', 'number'],
        ['makeBreakfast', 'function', 1],
        ['name', 'string'],
        ['onmaketoastroundoff', 'object'],
        ['onmultipletoastcompletearray', 'object'],
        ['onmultipletoastcompletecollection', 'object'],
        ['removeEventListener', 'function', 2],
        ['role', 'number'],
    ];

    // name, type, length if function
    var ovenMembers = [
    ['bakeAsync', 'function', 1],
    ['electricityReporter', 'object'],
    ['size', 'object'],
    ['timerAsync', 'function', 1],
    ];

    // name, type, length if function
    var bakeOperationMembers = [
    ['cancel', 'function', 0],
    ['close', 'function', 0],
    ['completed', 'object'],
    ['errorCode', 'number'],
    ['getResults', 'function', 0],
    ['id', 'number'],
    ['progress', 'object'],
    ['status', 'number']
    ];


    var shopAreaExpected = [
    ['width', 'number', 100],
    ['length', 'number', 120]
    ];

    function verifyShopArea(actual, expected) {
        var iIndex = 0;

        for (p in actual) {
            verify(p, expected[iIndex][0], p);
            verify(typeof actual[p], expected[iIndex][1], 'type');
            verify(actual[p], expected[iIndex][2], 'value');
            iIndex++;
        }
    }

    function verifyShopDimension(actual, expected) {
        var iIndex = 0;

        for (p in actual) {
            verify(p, expected[iIndex][0], p);
            verify(typeof actual[p], expected[iIndex][1], 'type');

            if (typeof actual[p] === 'object') {
                verifyShopArea(actual[p], shopAreaExpected);
            }
            else {
                verify(actual[p], expected[iIndex][2], 'value');
            }

            iIndex++;
        }
    }

    // name, type, length
    var fishExpected = [
        ['getNullAsAnimal', 'function', 0],
        ['getNullAsMap', 'function', 0],
        ['getNullAsObservableVector', 'function', 0],
        ['getNullAsPropertyValue', 'function', 0],
        ['getNullAsVector', 'function', 0],
        ['getNumFins', 'function', 0],
        ['getOneAnimal', 'function', 0],
        ['getOneEmptyGRCNFail', 'function', 0],
        ['getOneEmptyGRCNInterface', 'function', 0],
        ['getOneEmptyGRCNNull', 'function', 0],
        ['getOneMap', 'function', 0],
        ['getOneObservableVector', 'function', 0],
        ['getOnePropertyValue', 'function', 0],
        ['getOneVector', 'function', 0],
        ['marshalIFish', 'function', 1],
        ['marshalIFishToFish', 'function', 1],
        ['marshalILikeToSwim', 'function', 1],
        ['marshalILikeToSwimToFish', 'function', 1],
        ['name', 'string'],
        ['setNumFins', 'function', 1],
        ['singTheSwimmingSong', 'function', 0],
    ];

    var songExpected = "I feed from the bottom, you feed from the top \nI live upon morsels you happen to drop \nAnd coffee that somehow leaks out of your cup \nIf nothing comes down then I'm forced to swim up \n";

    var fish;
    var factory;
    var winery;

    runner.globalSetup(function () {
        fish = new Animals.Fish();
        factory = Winery.RWinery;
        winery = new factory(1);
    });

    runner.addTest({
        id: 1,
        desc: 'Kitchen Walkthrough',
        pri: '0',
        test: function () {
            var toaster = new Fabrikam.Kitchen.Toaster()
            verifyMembers(toaster, toasterMembers, 'object')
            var kitchen = new Fabrikam.Kitchen.Kitchen()
            verifyMembers(kitchen, kitchenMembers, 'object')
            var chef = new Fabrikam.Kitchen.Chef("Swedish", kitchen)
            verifyMembers(chef, chefMembers, 'object')
            var oven = new Fabrikam.Kitchen.Oven()
            verifyMembers(oven, ovenMembers, 'object')
            var bakeOperation = oven.bakeAsync(15).operation;
            verifyMembers(bakeOperation, bakeOperationMembers, 'object')

        }
    });

    runner.addTest({
        id: 2,
        desc: 'Winery.shopName',
        pri: '0',
        test: function () {
            var wineShopName = "Wine Shop";
            verify(("shopName" in winery), true, "Winery has property shopName");
            winery.shopName = wineShopName;
            verify(winery.shopName, wineShopName, 'winery.shopName');
            verify(winery["shopName"], wineShopName, 'winery["shopName"]');

        }
    });

    runner.addTest({
        id: 3,
        desc: 'Calling a method on a required interface',
        pri: '0',
        test: function () {
            verify(fish.singTheSwimmingSong(), songExpected, 'fish.singTheSwimmingSong()');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'marshalIFish',
        pri: '0',
        test: function () {
            var fish2 = fish.marshalIFish(fish)
            verifyMembers(fish2, fishExpected, 'object');
            verify(fish2.getNumFins(), 5, 'fish2.getNumFins()');
        }
    });

    Loader42_FileName = 'Recycler Stress scenarios from interfaces.js';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
