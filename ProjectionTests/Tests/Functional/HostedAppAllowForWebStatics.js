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
        ['myDino', 'undefined'],  // member is there, but it returns undefined as Dino is not marked with allowForWeb
        ['myExtinct', 'object'],
        ['myFish', 'undefined'],  // member is there, but it returns undefined as Dino is not marked with allowForWeb
        ['myFishRefCount', 'number'],
        ['myIFish', 'object'],
        ['myLikeToSwim', 'object'],
        ['myStaticArrayProp', 'object'],
        ['myStaticArrayPropHSTRING', 'object'],
        ['myToaster', 'undefined'],  // member is there, but it returns undefined as Dino is not marked with allowForWeb
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
        ['staticFastPath', 'function', 0],
        ['staticFastPathIn', 'function', 1],
        ['staticFastPathInIn', 'function', 2],
        ['staticFastPathInOut', 'function', 1],
        ['staticFastPathOut', 'function', 0],
        ['staticSlowPath', 'function', 6],
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
        ['myFish', 'undefined'],  // member is there, but it returns undefined as Dino is not marked with allowForWeb
        ['myFishRefCount', 'number'],
        ['myIFish', 'object'],
        ['myLikeToSwim', 'object'],
        ['myStaticArrayProp', 'object'],
        ['myStaticArrayPropHSTRING', 'object'],
        ['myToaster', 'undefined'],  // member is there, but it returns undefined as Dino is not marked with allowForWeb
        ['myToasterRefCount', 'number'],
        ['sendBackSameFish', 'function', 1],
        ['sendBackSameIFish', 'function', 1],
        ['sendBackSameLikeToSwim', 'function', 1],
        ['sendBackSamePropertySet', 'function', 1],
        ['testDefaultAnimal', 'function', 1],
        ['testDefaultDino', 'function', 1],
        ['testDefaultFish', 'function', 1],
        ['testDefaultMultipleIVector', 'function', 1],
        ['toString', 'function', 0],
    ];

    var animalFactory;
    var animal;

    runner.globalSetup(function () {
        animalFactory = Animals.Animal;
        animal = new animalFactory();
    });

    runner.addTest({
        id: 0,
        desc: 'Verify that members appear the same before and after a method call',
        pri: '0',
        test: function () {
            verifyMembersExist(animalFactory, staticAnimalMembers, 'function')
            Animals.Animal.animalObjectSize;
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
            verify(animalFactory.takeANap(20), true, 'animalFactory.takeANap(20)');
            var exceptionCaught = false;
            try {
                animalFactory.takeANap(-1);
            } catch (e) {
                exceptionCaught = true;
            }
            assert(exceptionCaught, 'Exception was caught while calling animalFactory.takeANap(-1)');
        }
    });

    runner.addTest({
        id: 4,
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
        id: 5,
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

    Loader42_FileName = 'Statics test for allowForWeb hosted app';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
