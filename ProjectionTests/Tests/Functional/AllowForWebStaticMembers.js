if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile('..\\projectionsglue.js'); } 
(function () {

    // These fields are similar to the definitions in Statics.js, except here they are all expected to be undefined
    var staticAnimalMembers = [
        ['animalObjectSize'],
        ['callDelegateWithExtinct'],
        ['callDelegateWithFish'],
        ['callDelegateWithIFish'],
        ['callDelegateWithInFloat'],
        ['callDelegateWithInOutBigComplexStruct'],
        ['callDelegateWithInOutFloat'],
        ['callDelegateWithInOutOddSizedStruct'],
        ['callDelegateWithInOutPackedBoolean'],
        ['callDelegateWithInOutPackedByte'],
        ['callDelegateWithInOutSmallComplexStruct'],
        ['callDelegateWithInParam_BigStruct'],
        ['callDelegateWithLikeToSwim'],
        ['callDelegateWithOutFloat'],
        ['callDelegateWithOutParam_BigStruct'],
        ['callMyFishMethod'],
        ['dinoDefault'],
        ['dinoDefaultVector'],
        ['dinoMarshalAs'],
        ['fillUInt8Array'],
        ['getAnswer'],
        ['getBigComplexStructArray'],
        ['getCLSID'],
        ['getDoubleObservableMap'],
        ['getNullAsAnimal'],
        ['getNullAsMap'],
        ['getNullAsObservableVector'],
        ['getNullAsPropertyValue'],
        ['getNullAsVector'],
        ['getObservableStringIntegerMap'],
        ['getOddSizedStructArray'],
        ['getOneAnimal'],
        ['getOneEmptyGRCNFail'],
        ['getOneEmptyGRCNInterface'],
        ['getOneEmptyGRCNNull'],
        ['getOneMap'],
        ['getOneObservableVector'],
        ['getOnePropertyValue'],
        ['getOneVector'],
        ['getPackedBooleanArray'],
        ['getPackedByteArray'],
        ['getRefCount'],
        ['getSmallComplexStructArray'],
        ['getStaticAnimalAsInspectable'],
        ['getStaticAnimalAsStaticInterface'],
        ['getStringHiddenTypeMap'],
        ['getStringIntegerMap'],
        ['isLovable'],
        ['marshalInAndOutBigComplexStruct'],
        ['marshalInAndOutOddSizedStruct'],
        ['marshalInAndOutPackedBoolean'],
        ['marshalInAndOutPackedByte'],
        ['marshalInAndOutSmallComplexStruct'],
        ['methodWithInParam_BigStruct'],
        ['methodWithOutParam_BigStruct'],
        ['multiplyNumbers'],
        ['myDino'],
        ['myExtinct'],
        ['myFish'],
        ['myFishRefCount'],
        ['myIFish'],
        ['myLikeToSwim'],
        ['myStaticArrayProp'],
        ['myStaticArrayPropHSTRING'],
        ['myToaster'],
        ['myToasterRefCount'],
        ['passUInt8Array'],
        ['sendBackSameDino'],
        ['sendBackSameExtinct'],
        ['sendBackSameFish'],
        ['sendBackSameIDino'],
        ['sendBackSameIFish'],
        ['sendBackSameInspectableVector'],
        ['sendBackSameLikeToSwim'],
        ['sendBackSamePropertySet'],
        ['takeANap'],
        ['testDefaultAnimal'],
        ['testDefaultDino'],
        ['testDefaultFish'],
        ['testDefaultMultipleIVector'],
    ];

    var staticAnimal2Members = [
        ['animalObjectSize'],
        ['callDelegateWithFish'],
        ['callDelegateWithIFish'],
        ['callDelegateWithLikeToSwim'],
        ['callMyFishMethod'],
        ['getCLSID'],
        ['getDoubleObservableMap'],
        ['getObservableStringIntegerMap'],
        ['getRefCount'],
        ['getStaticAnimalAsInspectable'],
        ['getStaticAnimalAsStaticInterface'],
        ['getStringHiddenTypeMap'],
        ['getStringIntegerMap'],
        ['multiplyNumbers'],
        ['myFish'],
        ['myFishRefCount'],
        ['myIFish'],
        ['myLikeToSwim'],
        ['myStaticArrayProp'],
        ['myStaticArrayPropHSTRING'],
        ['myToaster'],
        ['myToasterRefCount'],
        ['sendBackSameFish'],
        ['sendBackSameIFish'],
        ['sendBackSameLikeToSwim'],
        ['sendBackSamePropertySet'],
        ['testDefaultAnimal'],
        ['testDefaultDino'],
        ['testDefaultFish'],
        ['testDefaultMultipleIVector'],
    ];

    function verifyMembersDoNotExist(actual, expected, expectedType) {
        verify(typeof actual, expectedType, 'type');
        var i = 0;
        for (p in expected) {
            verify(actual[p[0]] + '', 'undefined', expected[p][0]);
            i++;
        }
    }

    function getAnimal() {
        // Get an instance of an Animal from TestUtilities (since we can't 'new' one in a Web View host)
        var animalInstance = TestUtilities.AnimalToVar();
        verify(!!animalInstance, true, '!!animalInstance');
        return animalInstance;
    }

    runner.globalSetup(function () {
    });

    runner.addTest({
        id: 1,
        desc: 'Verify that static members of a class with [AllowForWeb] aren\'t accessible through object instance',
        pri: '0',
        test: function () {
            var animal = getAnimal();
            verifyMembersDoNotExist(animal, staticAnimalMembers, 'object');
            verifyMembersDoNotExist(animal, staticAnimal2Members, 'object');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Verify that static members of a class with [AllowForWeb] aren\'t accessible from constructor',
        pri: '0',
        test: function () {
            // Validate factory (constructor)
            verifyMembersDoNotExist(Animals.Animal, staticAnimalMembers, 'function');
            verifyMembersDoNotExist(Animals.Animal, staticAnimal2Members, 'function');
        }
    });

    Loader42_FileName = 'AllowForWeb - Static Members tests';

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
