if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    function easyMembersPrint(myObjectString, myObject) {
        var objectDump = "\n    var " + myObjectString + "Members = [";
        for (p in myObject) {
            if (typeof myObject[p] == 'function') {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myObject[p] + '\', getFunctionString(\'' + p + '\'), ' + myObject[p].length + '],'
            }
            else {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myObject[p] + '\', getFunctionString(\'' + p + '\'), 0 ],';
            }
        }
        objectDump = objectDump + "\n    ];";

        return objectDump;
    }

    function dumpObjectMembers(myObjectString, myObject) {
        var objectDump = easyMembersPrint(myObjectString, myObject);

        logger.comment("typeof " + myObjectString + ": " + typeof myObject);
        logger.comment("Dump of properties : " + objectDump);
    }

    function verifyRuntimeClass(actual, expected, expectedType) {
        dumpObjectMembers("object", actual);
        verify(typeof actual, expectedType, 'type');
        var i = 0;
        for (p in actual) {
            verify(p, expected[i][0], 'name');
            verify(typeof actual[p], expected[i][1], 'typeof ' + actual + '[' + p + ']');
            var desc = Object.getOwnPropertyDescriptor(actual, p);
            var result;
            if (desc !== undefined) {
                if (desc.get !== undefined) {
                    result = desc.get;
                } else if (desc.set !== undefined) {
                    result = desc.set;
                } else {
                    result = actual[p];
                }
            } else {
                result = actual[p];
            }
            if (typeof result == 'function') {
                verify(result + '', expected[i][2], p);
                verify(result.length, expected[i][3], p + '.length');
                logger.comment('Setting length of function to be 10');
                result.length = 10;
                verify(result.length, expected[i][3], p + '.length');
            }

            i++;
        }

        verify(i, expected.length, 'number of members');
    }

    function getFunctionString(name) {
        if (name === '') {
            return 'function ' + name + '() { [native code] }';
        } else {
            return 'function ' + name + '() { [native code] }';
        }
    }

    function getErrorString(name) {
        return 'Error: ' + name + ': function called with too few arguments';
    }

    // name, type,string, length
    var fishExpected = [
        ['getNullAsAnimal', 'function', getFunctionString('getNullAsAnimal'), 0],
        ['getNullAsMap', 'function', getFunctionString('getNullAsMap'), 0],
        ['getNullAsObservableVector', 'function', getFunctionString('getNullAsObservableVector'), 0],
        ['getNullAsPropertyValue', 'function', getFunctionString('getNullAsPropertyValue'), 0],
        ['getNullAsVector', 'function', getFunctionString('getNullAsVector'), 0],
        ['getNumFins', 'function', getFunctionString('getNumFins'), 0],
        ['getOneAnimal', 'function', getFunctionString('getOneAnimal'), 0],
        ['getOneEmptyGRCNFail', 'function', getFunctionString('getOneEmptyGRCNFail'), 0],
        ['getOneEmptyGRCNInterface', 'function', getFunctionString('getOneEmptyGRCNInterface'), 0],
        ['getOneEmptyGRCNNull', 'function', getFunctionString('getOneEmptyGRCNNull'), 0],
        ['getOneMap', 'function', getFunctionString('getOneMap'), 0],
        ['getOneObservableVector', 'function', getFunctionString('getOneObservableVector'), 0],
        ['getOnePropertyValue', 'function', getFunctionString('getOnePropertyValue'), 0],
        ['getOneVector', 'function', getFunctionString('getOneVector'), 0],
        ['marshalIFish', 'function', getFunctionString('marshalIFish'), 1],
        ['marshalIFishToFish', 'function', getFunctionString('marshalIFishToFish'), 1],
        ['marshalILikeToSwim', 'function', getFunctionString('marshalILikeToSwim'), 1],
        ['marshalILikeToSwimToFish', 'function', getFunctionString('marshalILikeToSwimToFish'), 1],
        ['name', 'string', getFunctionString('name'), 0],
        ['setNumFins', 'function', getFunctionString('setNumFins'), 1],
        ['singTheSwimmingSong', 'function', getFunctionString('singTheSwimmingSong'), 0],
        ['toString', 'function', getFunctionString('toString'), 0],
    ];


    // name, type,string, length
    var fishPrototypeExpected = [
        ['getNullAsAnimal', 'function', getFunctionString('getNullAsAnimal'), 0],
        ['getNullAsMap', 'function', getFunctionString('getNullAsMap'), 0],
        ['getNullAsObservableVector', 'function', getFunctionString('getNullAsObservableVector'), 0],
        ['getNullAsPropertyValue', 'function', getFunctionString('getNullAsPropertyValue'), 0],
        ['getNullAsVector', 'function', getFunctionString('getNullAsVector'), 0],
        ['getNumFins', 'function', getFunctionString('getNumFins'), 0],
        ['getOneAnimal', 'function', getFunctionString('getOneAnimal'), 0],
        ['getOneEmptyGRCNFail', 'function', getFunctionString('getOneEmptyGRCNFail'), 0],
        ['getOneEmptyGRCNInterface', 'function', getFunctionString('getOneEmptyGRCNInterface'), 0],
        ['getOneEmptyGRCNNull', 'function', getFunctionString('getOneEmptyGRCNNull'), 0],
        ['getOneMap', 'function', getFunctionString('getOneMap'), 0],
        ['getOneObservableVector', 'function', getFunctionString('getOneObservableVector'), 0],
        ['getOnePropertyValue', 'function', getFunctionString('getOnePropertyValue'), 0],
        ['getOneVector', 'function', getFunctionString('getOneVector'), 0],
        ['marshalIFish', 'function', getFunctionString('marshalIFish'), 1],
        ['marshalIFishToFish', 'function', getFunctionString('marshalIFishToFish'), 1],
        ['marshalILikeToSwim', 'function', getFunctionString('marshalILikeToSwim'), 1],
        ['marshalILikeToSwimToFish', 'function', getFunctionString('marshalILikeToSwimToFish'), 1],
        ['name', 'undefined', getFunctionString('get_Name'), 0],
        ['setNumFins', 'function', getFunctionString('setNumFins'), 1],
        ['singTheSwimmingSong', 'function', getFunctionString('singTheSwimmingSong'), 0],
        ['toString', 'function', getFunctionString('toString'), 0],
];

    var fishResultsExpected = [
        ['getNullAsAnimal', null],
        ['getNullAsMap', null],
        ['getNullAsObservableVector', null],
        ['getNullAsPropertyValue', null],
        ['getNullAsVector', null],
        ['getNumFins', '5'],
        ['getOneAnimal', '[object Animals.Animal]'],
        ['getOneEmptyGRCNFail', '[object Animals.IEmptyGRCN]'],
        ['getOneEmptyGRCNInterface', '[object Animals.IEmptyGRCN]'],
        ['getOneEmptyGRCNNull', '[object Animals.IEmptyGRCN]'],
        ['getOneMap', '[object Windows.Foundation.Collections.IMap`2<String,Int32>]'],
        ['getOneObservableVector', '1,2,3,4'],
        ['getOnePropertyValue', '10.5'],
        ['getOneVector', '1,2,3'],
        ['marshalIFish', getErrorString('marshalIFish')],
        ['marshalIFishToFish', getErrorString('marshalIFishToFish')],
        ['marshalILikeToSwim', getErrorString('marshalILikeToSwim')],
        ['marshalILikeToSwimToFish', getErrorString('marshalILikeToSwimToFish')],
        ['name', "TypeError: The value of the property 'name' is not a Function object"],
        ['setNumFins', getErrorString('setNumFins')],
        ['singTheSwimmingSong', "I feed from the bottom, you feed from the top \nI live upon morsels you happen to drop \nAnd coffee that somehow leaks out of your cup \nIf nothing comes down then I'm forced to swim up \n"],
        ['toString', '[object Animals.Fish]'],
    ];


    // name, type,string, length
    var dinoExpected = [
    ['Animals.IDino.hasTeeth', 'function', getFunctionString('Animals.IDino.hasTeeth'), 0],
    ['Animals.IExtinct.hasTeeth', 'function', getFunctionString('Animals.IExtinct.hasTeeth'), 0],
    ['canRoar', 'function', getFunctionString('canRoar'), 0],
    ['height', 'number', getFunctionString('get_Height'), 0],
    ['isExtinct', 'function', getFunctionString('isExtinct'), 0],
    ['roar', 'function', getFunctionString('roar'), 1],
    ['toString', 'function', getFunctionString('toString'), 0],
];


    // name, type,string, length
    var dinoPrototypeExpected = [
    ['Animals.IDino.hasTeeth', 'function', getFunctionString('Animals.IDino.hasTeeth'), 0],
    ['Animals.IExtinct.hasTeeth', 'function', getFunctionString('Animals.IExtinct.hasTeeth'), 0],
    ['canRoar', 'function', getFunctionString('canRoar'), 0],
    ['height', 'undefined', getFunctionString('get_Height'), 0],
    ['isExtinct', 'function', getFunctionString('isExtinct'), 0],
    ['roar', 'function', getFunctionString('roar'), 1],
    ['toString', 'function', getFunctionString('toString'), 0],
];

    var dinoResultsExpected = [
    ['Animals.IDino.hasTeeth', true],
    ['Animals.IExtinct.hasTeeth', false],
    ['canRoar', false],
    ['height', "TypeError: The value of the property 'height' is not a Function object"],
    ['isExtinct', true],
    ['roar', getErrorString('roar')],
    ['toString', "[object Animals.Dino]"],
];

    /// Create the ABIs used throughout this test
    var myFish;
    var myDino;

    runner.globalSetup(function () {
        myFish = new Animals.Fish();
        myDino = new Animals.Dino();
    });

    runner.addTest({
        id: 1,
        desc: 'Fish',
        pri: '0',
        test: function () {
            verifyRuntimeClass(myFish, fishExpected, 'object');

        }
    });

    runner.addTest({
        id: 2,
        desc: 'Dino',
        pri: '0',
        test: function () {
            verifyRuntimeClass(myDino, dinoExpected, 'object');

        }
    });


    runner.addTest({
        id: 3,
        desc: 'DinoPrototype',
        pri: '0',
        test: function () {
            verifyRuntimeClass(Animals.Dino.prototype, dinoPrototypeExpected, 'object');

        }
    });

    runner.addTest({
        id: 4,
        desc: 'FishPrototype',
        pri: '0',
        test: function () {
            verifyRuntimeClass(Animals.Fish.prototype, fishPrototypeExpected, 'object');

        }
    });

    runner.addTest({
        id: 5,
        desc: 'InvokeFishMethods',
        pri: '0',
        test: function () {
            var iIndex = 0;
            for (var i in myFish) {
                try {
                    var result = myFish[i]();
                    verify(result === null ? null : result.toString(), fishResultsExpected[iIndex][1], i);
                }
                catch (e) {
                    verify(e + '', fishResultsExpected[iIndex][1], i);
                }
                iIndex++;
            }

        }
    });


    runner.addTest({
        id: 6,
        desc: 'InvokeDinoMethods',
        pri: '0',
        test: function () {

            var iIndex = 0;
            for (var i in myDino) {
                try {
                    verify(myDino[i](), dinoResultsExpected[iIndex][1], i);
                }
                catch (e) {
                    verify(e + '', dinoResultsExpected[iIndex][1], i);
                }

                iIndex++;
            }

        }
    });


    runner.addTest({
        id: 7,
        desc: 'RuntimeClass as in parameter',
        pri: '0',
        test: function () {
            var myFish = new Animals.Fish();
            myFish.name = "Nemo";

            var mySameFish = Animals.Animal.sendBackSameFish(myFish);
            verify(mySameFish, myFish, "mySameFish");

            var mySameNullFish = Animals.Animal.sendBackSameFish(null);
            verify(mySameNullFish, null, "mySameNullFish");

            var mySameUndefinedFish = Animals.Animal.sendBackSameFish(undefined);
            verify(mySameUndefinedFish, null, "mySameUndefinedFish");

            verify.exception(function () {
                var myMismatchFish = Animals.Animal.sendBackSameFish(Animals.Animal);
            }, TypeError, "var myMismatchFish = Animals.Animal.sendBackSameFish(Animals.Animal);");

            verify.exception(function () {
                var myMismatchFish = Animals.Animal.sendBackSameFish(new Animals.Animal(10));
            }, TypeError, "var myMismatchFish = Animals.Animal.sendBackSameFish(new Animals.Animal(10));");

        }
    });

    Loader42_FileName = 'RunTimeClass tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
