if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile('..\\projectionsglue.js'); }
(function () {

    // This is essentially the same as InspectableToVar.js, except that due to the Animal constructor being blocked the
    // baseline file is slightly different when we verify the constructor
    var animalMembersExpected = [
        ['acceptKiloStruct', 'function', 1],
        ['addEventListener', 'function', 2],
        ['addInts', 'function', 2],
        ['areDimensionPointersEqual', 'function', 2],
        ['callDelegateFillArray', 'function', 1],
        ['callDelegateFillArrayHSTRING', 'function', 1],
        ['callDelegateFillArrayWithInLength', 'function', 1],
        ['callDelegateFillArrayWithInLengthHSTRING', 'function', 1],
        ['callDelegateFillArrayWithOutLength', 'function', 1],
        ['callDelegateFillArrayWithOutLengthHSTRING', 'function', 1],
        ['callDelegateFillArrayWithOutLengthWithRetValLength', 'function', 1],
        ['callDelegateFillArrayWithOutLengthWithRetValLengthHSTRING', 'function', 1],
        ['callDelegateFillArrayWithOutLengthWithRetValRandomParam', 'function', 1],
        ['callDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING', 'function', 1],
        ['callDelegatePassArray', 'function', 1],
        ['callDelegatePassArrayHSTRING', 'function', 1],
        ['callDelegatePassArrayWithInLength', 'function', 1],
        ['callDelegatePassArrayWithInLengthHSTRING', 'function', 1],
        ['callDelegatePassArrayWithOutLength', 'function', 1],
        ['callDelegatePassArrayWithOutLengthHSTRING', 'function', 1],
        ['callDelegatePassArrayWithOutLengthWithRetValLength', 'function', 1],
        ['callDelegatePassArrayWithOutLengthWithRetValLengthHSTRING', 'function', 1],
        ['callDelegatePassArrayWithOutLengthWithRetValRandomParam', 'function', 1],
        ['callDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING', 'function', 1],
        ['callDelegateReceiveArray', 'function', 1],
        ['callDelegateReceiveArrayHSTRING', 'function', 1],
        ['callDelegateReceiveArrayWithInLength', 'function', 1],
        ['callDelegateReceiveArrayWithInLengthHSTRING', 'function', 1],
        ['callDelegateReceiveArrayWithOutLength', 'function', 1],
        ['callDelegateReceiveArrayWithOutLengthHSTRING', 'function', 1],
        ['callDelegateReceiveArrayWithOutLengthWithRetValLength', 'function', 1],
        ['callDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING', 'function', 1],
        ['callDelegateReceiveArrayWithOutLengthWithRetValRandomParam', 'function', 1],
        ['callDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING', 'function', 1],
        ['callDelegateWithIterable', 'function', 1],
        ['callDelegateWithMultipleOutParams', 'function', 2],
        ['callDelegateWithOutParam_HSTRING', 'function', 1],
        ['callDelegateWithOutParam_InOutMixed', 'function', 2],
        ['callDelegateWithOutParam_Interface', 'function', 1],
        ['callDelegateWithOutParam_Struct', 'function', 1],
        ['callDelegateWithOutParam_int', 'function', 1],
        ['callDelegateWithVector', 'function', 1],
        ['checkByRefStruct', 'function', 2],
        ['checkMother', 'function', 1],
        ['checkMotherConcrete', 'function', 1],
        ['copyStringVector', 'function', 1],
        ['copyVector', 'function', 1],
        ['delIn_BooleanOut2', 'function', 1],
        ['delegateByRef_Class', 'function', 2],
        ['delegateByRef_Delegate', 'function', 3],
        ['delegateByRef_GUID', 'function', 2],
        ['delegateByRef_Interface', 'function', 2],
        ['delegateByRef_Struct', 'function', 2],
        ['doubleOffset2Int', 'function', 3],
        ['doubleOffsetByte', 'function', 2],
        ['doubleOffsetChar', 'function', 2],
        ['doubleOffsetInt', 'function', 2],
        ['doubleOffsetInt64', 'function', 2],
        ['doubleOffsetStruct', 'function', 2],
        ['duplicateIterable', 'function', 1],
        ['duplicateIterator', 'function', 1],
        ['duplicateStringIterable', 'function', 1],
        ['duplicateStringIterator', 'function', 1],
        ['duplicateStringVector', 'function', 1],
        ['duplicateStringVectorView', 'function', 1],
        ['duplicateVector', 'function', 1],
        ['duplicateVectorView', 'function', 1],
        ['errorCode', 'number'],
        ['fastPath', 'function', 0],
        ['fastPathIn', 'function', 1],
        ['fastPathInIn', 'function', 2],
        ['fastPathInOut', 'function', 1],
        ['fastPathOut', 'function', 0],
        ['fillArray', 'function', 1],
        ['fillArrayHSTRING', 'function', 1],
        ['fillArrayWithInLength', 'function', 2],
        ['fillArrayWithInLengthHSTRING', 'function', 2],
        ['fillArrayWithOutLength', 'function', 1],
        ['fillArrayWithOutLengthHSTRING', 'function', 1],
        ['fillArrayWithOutLengthWithRetValLength', 'function', 1],
        ['fillArrayWithOutLengthWithRetValLengthHSTRING', 'function', 1],
        ['fillArrayWithOutLengthWithRetValRandomParam', 'function', 1],
        ['fillArrayWithOutLengthWithRetValRandomParamHSTRING', 'function', 1],
        ['fillDimensions', 'function', 1],
        ['floatOffset2Int', 'function', 3],
        ['floatOffsetByte', 'function', 2],
        ['floatOffsetChar', 'function', 2],
        ['floatOffsetInt', 'function', 2],
        ['floatOffsetInt64', 'function', 2],
        ['floatOffsetStruct', 'function', 2],
        ['getDimensions', 'function', 0],
        ['getGreeting', 'function', 0],
        ['getMap', 'function', 1],
        ['getNULLHSTRING', 'function', 0],
        ['getNames', 'function', 0],
        ['getNativeDelegateAsOutParam', 'function', 0],
        ['getNumLegs', 'function', 0],
        ['getObservableStringVector', 'function', 0],
        ['getObservableVector', 'function', 0],
        ['getOuterStruct', 'function', 0],
        ['getReadOnlyVector', 'function', 1],
        ['getStringVector', 'function', 0],
        ['getVector', 'function', 0],
        ['id', 'string'],
        ['interspersedInOutBool', 'function', 2],
        ['interspersedInOutChar16', 'function', 2],
        ['interspersedInOutDimensions', 'function', 2],
        ['interspersedInOutDouble', 'function', 2],
        ['interspersedInOutFish', 'function', 2],
        ['interspersedInOutHSTRING', 'function', 2],
        ['interspersedInOutIFish', 'function', 2],
        ['interspersedInOutInt32', 'function', 2],
        ['interspersedInOutInt64', 'function', 2],
        ['interspersedInOutPhylum', 'function', 2],
        ['interspersedInOutSingle', 'function', 2],
        ['interspersedInOutUInt32', 'function', 2],
        ['interspersedInOutUInt64', 'function', 2],
        ['interspersedInOutUInt8', 'function', 2],
        ['isHungry', 'function', 0],
        ['isSleepy', 'function', 0],
        ['isStructModified', 'function', 2],
        ['layoutBasicWithStructs', 'function', 9],
        ['layoutOfManyMembers', 'function', 9],
        ['layoutStructs', 'function', 5],
        ['likesChef', 'function', 0],
        ['marshalBool', 'function', 1],
        ['marshalChar16', 'function', 1],
        ['marshalDimensions', 'function', 1],
        ['marshalDouble', 'function', 1],
        ['marshalGUID', 'function', 1],
        ['marshalHRESULT', 'function', 1],
        ['marshalHSTRING', 'function', 1],
        ['marshalInt16', 'function', 1],
        ['marshalInt32', 'function', 1],
        ['marshalInt64', 'function', 1],
        ['marshalNames', 'function', 1],
        ['marshalNullAsDelegate', 'function', 1],
        ['marshalOuterStruct', 'function', 1],
        ['marshalPhylum', 'function', 1],
        ['marshalPhylumChange', 'function', 1],
        ['marshalSingle', 'function', 1],
        ['marshalStudyInfo', 'function', 1],
        ['marshalUInt16', 'function', 1],
        ['marshalUInt32', 'function', 1],
        ['marshalUInt64', 'function', 1],
        ['marshalUInt8', 'function', 1],
        ['methodDelegateAsOutParam', 'function', 1],
        ['mother', 'object'],
        ['multiDouble3', 'function', 3],
        ['multiDouble4', 'function', 4],
        ['multiFloat3', 'function', 3],
        ['multiFloat4', 'function', 4],
        ['multipleOutBool', 'function', 2],
        ['multipleOutChar16', 'function', 2],
        ['multipleOutDimensions', 'function', 2],
        ['multipleOutDouble', 'function', 2],
        ['multipleOutFish', 'function', 2],
        ['multipleOutHSTRING', 'function', 2],
        ['multipleOutIFish', 'function', 2],
        ['multipleOutInt32', 'function', 2],
        ['multipleOutInt64', 'function', 2],
        ['multipleOutPhylum', 'function', 2],
        ['multipleOutSingle', 'function', 2],
        ['multipleOutUInt32', 'function', 2],
        ['multipleOutUInt64', 'function', 2],
        ['multipleOutUInt8', 'function', 2],
        ['myArrayProp', 'object'],
        ['myArrayPropHSTRING', 'object'],
        ['myDimensions', 'object'],
        ['myIterable', 'object'],
        ['myPhylum', 'number'],
        ['myVector', 'object'],
        ['oneventhandler', 'object'],
        ['passArray', 'function', 1],
        ['passArrayHSTRING', 'function', 1],
        ['passArrayWithInLength', 'function', 2],
        ['passArrayWithInLengthHSTRING', 'function', 2],
        ['passArrayWithOutLength', 'function', 1],
        ['passArrayWithOutLengthHSTRING', 'function', 1],
        ['passArrayWithOutLengthWithRetValLength', 'function', 1],
        ['passArrayWithOutLengthWithRetValLengthHSTRING', 'function', 1],
        ['passArrayWithOutLengthWithRetValRandomParam', 'function', 1],
        ['passArrayWithOutLengthWithRetValRandomParamHSTRING', 'function', 1],
        ['passIDByRef', 'function', 1],
        ['pureFillArray', 'function', 1],
        ['purePassArray', 'function', 1],
        ['pureReceiveArray', 'function', 0],
        ['receiveArray', 'function', 0],
        ['receiveArrayHSTRING', 'function', 0],
        ['receiveArrayWithInLength', 'function', 1],
        ['receiveArrayWithInLengthHSTRING', 'function', 1],
        ['receiveArrayWithOutLength', 'function', 0],
        ['receiveArrayWithOutLengthHSTRING', 'function', 0],
        ['receiveArrayWithOutLengthWithRetValLength', 'function', 0],
        ['receiveArrayWithOutLengthWithRetValLengthHSTRING', 'function', 0],
        ['receiveArrayWithOutLengthWithRetValRandomParam', 'function', 0],
        ['receiveArrayWithOutLengthWithRetValRandomParamHSTRING', 'function', 0],
        ['removeEventListener', 'function', 2],
        ['sendAndGetIVectorStructs', 'function', 1],
        ['sendBackSameIterable', 'function', 1],
        ['sendBackSameIterator', 'function', 1],
        ['sendBackSameStringIterable', 'function', 1],
        ['sendBackSameStringIterator', 'function', 1],
        ['sendBackSameStringVector', 'function', 1],
        ['sendBackSameStringVectorView', 'function', 1],
        ['sendBackSameVector', 'function', 1],
        ['sendBackSameVectorView', 'function', 1],
        ['setGreeting', 'function', 1],
        ['setNumLegs', 'function', 1],
        ['slowPath', 'function', 6],
        ['testBug202724_GetInt64', 'function', 0],
        ['testBug202724_GetUInt64', 'function', 0],
        ['testBug8327782_StackArguments', 'function', 9],
        ['testError', 'function', 1],
        ['testInAllIRefStruct', 'function', 1],
        ['testInMixIRefStruct', 'function', 1],
        ['testInNestedIRefNestedStruct', 'function', 1],
        ['testInNestedIRefStruct', 'function', 1],
        ['testInSimpleIRefStruct', 'function', 1],
        ['testOutAllIRefStruct', 'function', 1],
        ['testOutBug258665_HttpProgress', 'function', 1],
        ['testOutBug258665_HttpProgressAsOptEmpty', 'function', 0],
        ['testOutBug258665_HttpProgressAsOptIntEmpty', 'function', 0],
        ['testOutMixIRefStruct', 'function', 1],
        ['testOutNestedIRefNestedStruct', 'function', 1],
        ['testOutNestedIRefStruct', 'function', 1],
        ['testOutSimpleIRefStruct', 'function', 1],
        ['testPackedBoolean1', 'function', 1],
        ['testPackedByte12', 'function', 1],
        ['verifyMarshalGUID', 'function', 2],
        ['weight', 'number'],
        ['toString', 'function', 0]
    ];

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
        desc: 'Verify that instance members of an object of a class with [AllowForWeb] are accessible',
        pri: '0',
        test: function () {
            var animal = getAnimal();
            verifyMembers(animal, animalMembersExpected, 'object');
        }
    });

    Loader42_FileName = 'AllowForWeb - Instance Members tests';

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
