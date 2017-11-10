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

    function verifyMembersExist(actual, expected, expectedType) {
        logger.comment("Objectdump: " + easyMembersPrint("objectDump", actual));
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

    function verifyMembersDoNotExist(actual, expected, expectedType) {
        verify(typeof actual, expectedType, 'type');
        var i = 0;
        for (p in expected) {
            verify(actual[p[0]] + '', 'undefined', expected[p][0]);
            i++;
        }
    }

    // name, type, length if function
    var staticAnimalMembers = [
        ['animalObjectSize', 'number'],
        ['callDelegateWithExtinct', 'function', 1],
        ['callDelegateWithFish', 'function', 1],
        ['callDelegateWithIFish', 'function', 1],
        ['callDelegateWithInFloat', 'function', 2],
        ['callDelegateWithInOutBigComplexStruct', 'function', 2],
        ['callDelegateWithInOutFloat', 'function', 6],
        ['callDelegateWithInOutOddSizedStruct', 'function', 2],
        ['callDelegateWithInOutPackedBoolean', 'function', 2],
        ['callDelegateWithInOutPackedByte', 'function', 2],
        ['callDelegateWithInOutSmallComplexStruct', 'function', 2],
        ['callDelegateWithInParam_BigStruct', 'function', 5],
        ['callDelegateWithLikeToSwim', 'function', 1],
        ['callDelegateWithOutFloat', 'function', 1],
        ['callDelegateWithOutParam_BigStruct', 'function', 1],
        ['callMyFishMethod', 'function', 1],
        ['dinoDefault', 'function', 0],
        ['dinoDefaultVector', 'function', 0],
        ['dinoMarshalAs', 'function', 0],
        ['fillUInt8Array', 'function', 2],
        ['getAnswer', 'function', 0],
        ['getBigComplexStructArray', 'function', 0],
        ['getCLSID', 'function', 0],
        ['getDoubleObservableMap', 'function', 0],
        ['getNullAsAnimal', 'function', 0],
        ['getNullAsMap', 'function', 0],
        ['getNullAsObservableVector', 'function', 0],
        ['getNullAsPropertyValue', 'function', 0],
        ['getNullAsVector', 'function', 0],
        ['getObservableStringIntegerMap', 'function', 0],
        ['getOddSizedStructArray', 'function', 0],
        ['getOneAnimal', 'function', 0],
        ['getOneEmptyGRCNFail', 'function', 0],
        ['getOneEmptyGRCNInterface', 'function', 0],
        ['getOneEmptyGRCNNull', 'function', 0],
        ['getOneMap', 'function', 0],
        ['getOneObservableVector', 'function', 0],
        ['getOnePropertyValue', 'function', 0],
        ['getOneVector', 'function', 0],
        ['getPackedBooleanArray', 'function', 0],
        ['getPackedByteArray', 'function', 0],
        ['getRefCount', 'function', 1],
        ['getSmallComplexStructArray', 'function', 0],
        ['getStaticAnimalAsInspectable', 'function', 0],
        ['getStaticAnimalAsStaticInterface', 'function', 0],
        ['getStringHiddenTypeMap', 'function', 0],
        ['getStringIntegerMap', 'function', 0],
        ['isLovable', 'boolean'],
        ['marshalInAndOutBigComplexStruct', 'function', 1],
        ['marshalInAndOutOddSizedStruct', 'function', 1],
        ['marshalInAndOutPackedBoolean', 'function', 1],
        ['marshalInAndOutPackedByte', 'function', 1],
        ['marshalInAndOutSmallComplexStruct', 'function', 1],
        ['methodWithInParam_BigStruct', 'function', 1],
        ['methodWithOutParam_BigStruct', 'function', 4],
        ['multiplyNumbers', 'function', 2],
        ['myDino', 'object'],
        ['myExtinct', 'object'],
        ['myFish', 'object'],
        ['myFishRefCount', 'number'],
        ['myIFish', 'object'],
        ['myLikeToSwim', 'object'],
        ['myStaticArrayProp', 'object'],
        ['myStaticArrayPropHSTRING', 'object'],
        ['myToaster', 'object'],
        ['myToasterRefCount', 'number'],
        ['passUInt8Array', 'function', 1],
        ['sendBackSameDino', 'function', 1],
        ['sendBackSameExtinct', 'function', 1],
        ['sendBackSameFish', 'function', 1],
        ['sendBackSameIDino', 'function', 1],
        ['sendBackSameIFish', 'function', 1],
        ['sendBackSameInspectableVector', 'function', 1],
        ['sendBackSameLikeToSwim', 'function', 1],
        ['sendBackSamePropertySet', 'function', 1],
        ['takeANap', 'function', 1],
        ['testDefaultAnimal', 'function', 1],
        ['testDefaultDino', 'function', 1],
        ['testDefaultFish', 'function', 1],
        ['testDefaultMultipleIVector', 'function', 1],
    ];

    var staticAnimal2Members = [
        ['animalObjectSize', 'number'],
        ['callDelegateWithFish', 'function', 1],
        ['callDelegateWithIFish', 'function', 1],
        ['callDelegateWithLikeToSwim', 'function', 1],
        ['callMyFishMethod', 'function', 1],
        ['getCLSID', 'function', 0],
        ['getDoubleObservableMap', 'function', 0],
        ['getObservableStringIntegerMap', 'function', 0],
        ['getRefCount', 'function', 1],
        ['getStaticAnimalAsInspectable', 'function', 0],
        ['getStaticAnimalAsStaticInterface', 'function', 0],
        ['getStringHiddenTypeMap', 'function', 0],
        ['getStringIntegerMap', 'function', 0],
        ['multiplyNumbers', 'function', 2],
        ['myFish', 'object'],
        ['myFishRefCount', 'number'],
        ['myIFish', 'object'],
        ['myLikeToSwim', 'object'],
        ['myStaticArrayProp', 'object'],
        ['myStaticArrayPropHSTRING', 'object'],
        ['myToaster', 'object'],
        ['myToasterRefCount', 'number'],
        ['sendBackSameFish', 'function', 1],
        ['sendBackSameIFish', 'function', 1],
        ['sendBackSameLikeToSwim', 'function', 1],
        ['sendBackSamePropertySet', 'function', 1],
        ['testDefaultAnimal', 'function', 1],
        ['testDefaultDino', 'function', 1],
        ['testDefaultFish', 'function', 1],
        ['testDefaultMultipleIVector', 'function', 1],
    ];

    var animalFactory;
    var animal;

    var staticDinoMembers = [
        ['addEventListener', 'function', 2],
        ['inspectDino', 'function', 1],
        ['isScary', 'boolean'],
        ['lookForFossils', 'function', 1],
        ['onfossilsfoundevent', 'object'],
        ['removeEventListener', 'function', 2],
    ];

    var dinoFactory;
    var dino;
    var puppy;

    runner.globalSetup(function () {
        animalFactory = Animals.Animal;
        animal = new animalFactory();
        puppy = Animals.Pomapoodle;
        dinoFactory = Animals.Dino;
        dino = new dinoFactory;
    });

    runner.addTest({
        id: 0,
        desc: 'Verify that members appear the same before and after a method call',
        pri: '0',
        test: function () {

            verifyMembersExist(animalFactory, staticAnimalMembers, 'function')
            Animals.Animal.animalObjectSize;
            Animals.Animal.dinoDefault;
            Animals.Animal.myDino;
            Animals.Animal.dinoDefault.length;
            if (Animals.Animal.methodWithOutParam_BigStruct.length != 4) {
                throw "Wrong length for methodWithOutParam_BigStruct";
            }
            verifyMembersExist(animalFactory, staticAnimalMembers, 'function')
        }
    });

    runner.addTest({
        id: 1,
        desc: 'Verify Static Methods on ABI factory',
        pri: '0',
        test: function () {

            verifyMembersExist(animalFactory, staticAnimalMembers, 'function')

        }
    });

    runner.addTest({
        id: 2,
        desc: 'Verify Static Methods do not exist on ABI instance',
        pri: '0',
        test: function () {

            verifyMembersDoNotExist(animal, staticAnimalMembers, 'object')

        }
    });


    runner.addTest({
        id: 3,
        desc: 'Make static method calls',
        pri: '0',
        test: function () {
            verify(animalFactory.multiplyNumbers(8, 9), 72, 'animalFactory.multiplyNumbers(8, 9)');
            verify(animalFactory.getAnswer(), 42, 'animalFactory.getAnswer()');
            animalFactory.isLovable = false;
            verify(animalFactory.isLovable, false, 'animalFactory.isLovable');
            animalFactory.isLovable = true;
            verify(animalFactory.isLovable, true, 'animalFactory.isLovable');
            verify(animalFactory.takeANap(5), false, 'animalFactory.takeANap(5)');
            verify(animalFactory.takeANap(20), true, 'animalFactory.takeANap(5)');
            var exceptionCaught = false;
            try {
                animalFactory.takeANap(-1);
            } catch (e) {
                exceptionCaught = true;
                verify(e.number, Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes.invalidArg, 'Exception caught while calling animalFactory.takeANap(-1)');
            }
            assert(exceptionCaught, 'Exception was caught while calling animalFactory.takeANap(-1)');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Statics on non-activatable class',
        pri: '0',
        test: function () {
            var puppy = Animals.Pomapoodle;

            verify(puppy.eatCookies(5), 4, 'puppy.eatCookies(5)');
            var exceptionCaught = false;
            try {
                puppy.wagTail(5);
            } catch (e) {
                exceptionCaught = true;
                verify(e.toString(), "TypeError: Object doesn't support property or method 'wagTail'", 'Exception caught while calling puppy.wagTail(5)');
            }
            assert(exceptionCaught, 'Exception was caught while calling puppy.wagTail(5)');
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Verify Static Methods on dino factory',
        pri: '0',
        test: function () {

            verifyMembersExist(dinoFactory, staticDinoMembers, 'function')

        }
    });

    runner.addTest({
        id: 6,
        desc: 'Verify Static Methods do not exist on dino instance',
        pri: '0',
        test: function () {

            verifyMembersDoNotExist(dino, staticDinoMembers, 'object')

        }
    });

    runner.addTest({
        id: 7,
        desc: 'Call static methods on dino',
        pri: '0',
        test: function () {
            verify(dinoFactory.lookForFossils(13), 3, 'dinoFactory.lookForFossils(13)');
            verify(dinoFactory.inspectDino(dino), 'Height: 5\nHasTeeth: true', 'dinoFactory.inspectDino(dino)');
            dinoFactory.isScary = false;
            verify(dinoFactory.isScary, false, 'dinoFactory.isScary');
            dinoFactory.isScary = true;
            verify(dinoFactory.isScary, true, 'dinoFactory.isScary');
        }
    });

    /// Hook up the cookies eaten handler
    runner.addTest({
        id: 7,
        desc: 'CookiesEatenEvent',
        pri: '0',
        test: function () {
            var cookieCount = 0;
            var cookiesGiven;

            var onCookiesEaten = function (ev) {
                verify(ev, cookiesGiven - 1, 'cookiesEaten');
                cookieCount += ev;
            }

            puppy.addEventListener('cookieseatenevent', onCookiesEaten);
            cookiesGiven = 5;
            puppy.eatCookies(cookiesGiven);
            cookiesGiven = 6;
            puppy.eatCookies(cookiesGiven);
            puppy.removeEventListener('cookieseatenevent', onCookiesEaten);
            cookiesGiven = 2;
            puppy.eatCookies(cookiesGiven);

            verify(cookieCount, 9, 'Total number of cookies eaten');
        }
    });

    /// Hook up the fossils found handler
    runner.addTest({
        id: 8,
        desc: 'FossilsFoundEvent',
        pri: '0',
        test: function () {
            var fossilCount = 0;
            var timeSpent;

            var onFossilsFound = function (ev) {
                verify(ev, timeSpent / 4, 'fossilsFound');
                fossilCount += ev;
            }

            dinoFactory.addEventListener('fossilsfoundevent', onFossilsFound);
            timeSpent = 24;
            dinoFactory.lookForFossils(timeSpent);
            timeSpent = 16;
            dinoFactory.lookForFossils(timeSpent);
            dinoFactory.removeEventListener('fossilsfoundevent', onFossilsFound);
            timeSpent = 7;
            dinoFactory.lookForFossils(timeSpent);

            verify(fossilCount, 10, 'Total number of fossils found');
        }
    });


    // Make static calls which use the default attribute
    runner.addTest({
        id: 9,
        desc: 'dinoDefault',
        pri: '0',
        test: function () {
            var dino = Animals.Animal.dinoDefault();
            verify(dino.canRoar(), false, 'dino.canRoar()');
            verify(dino.isExtinct(), true, 'dino.canRoar()');
        }
    });

    // Make static calls which use the default attribute
    runner.addTest({
        id: 10,
        desc: 'dinoDefaultVector',
        pri: '0',
        test: function () {
            var dino = Animals.Animal.dinoDefaultVector();
            verify(dino.size, 2, 'dino.size');
            verify(dino[0].canRoar(), false, 'dino.canRoar()');
            verify(dino[1].canRoar(), false, 'dino.canRoar()');
        }
    });


    // Make static calls which use the marshalas attribute
    runner.addTest({
        id: 11,
        desc: 'dinoMarshalAs',
        pri: '0',
        test: function () {
            var dino = Animals.Animal.dinoMarshalAs();
            verify(dino.canRoar(), false, 'dino.canRoar()');
            verify(dino.isExtinct(), true, 'dino.canRoar()');
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Get static interface as Inspectable',
        pri: '0',
        test: function () {
            // GetRuntimeClassName should fail for static interfaces so we should throw error
            verify.exception(function () {
                var staticInspectable = Animals.Animal.getStaticAnimalAsInspectable();
            }, Error, "IStaticAnimal2 returned as IInspectable should throw error because of failing GetRuntimeClassName");
        }
    });

    runner.addTest({
        id: 13,
        desc: 'Get static interface as IStaticInterface',
        pri: '0',
        preReq: function () {
            // CLR don't support this
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            // GetRuntimeClassName should fail but since the parameter is IStaticAnimal2 we would project it as IStaticAnimal2
            var staticAnimal2 = Animals.Animal.getStaticAnimalAsStaticInterface();
            verify(staticAnimal2 != null, true, "staticAnimal2 != null");
            verify(staticAnimal2 instanceof Animals.Animal, false, "staticAnimal2 instanceof Animals.Animal");

            verifyMembersExist(staticAnimal2, staticAnimal2Members, 'object')
        }
    });

    Loader42_FileName = 'Statics test';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
