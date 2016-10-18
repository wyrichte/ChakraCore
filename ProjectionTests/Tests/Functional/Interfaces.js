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
        ['toString', 'function', 0],
    ];

    var kitchenMembers = [
        ['toString', 'function', 0],
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
        ['toString', 'function', 0],
    ];

    // name, type, length if function
    var ovenMembers = [
    ['bakeAsync', 'function', 1],
    ['electricityReporter', 'object'],
    ['size', 'object'],
    ['timerAsync', 'function', 1],
    ['toString', 'function', 0],
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
    ['status', 'number'],
    ['toString', 'function', 0],
    ];


    var shopAreaExpected = [
    ['width', 'number', 100],
    ['length', 'number', 120]
    ];

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
        ['toString', 'function', 0],
    ];

    var mapMembers = [
        ['by', 'number'],
        ['Hundred', 'number'],
        ['Hundred And Fifty', 'number'],
        ['clear', 'function', 0],
        ['first', 'function', 0],
        ['getView', 'function', 0],
        ['hasKey', 'function', 1],
        ['insert', 'function', 2],
        ['lookup', 'function', 1],
        ['remove', 'function', 1],
        ['size', 'number'],
        ['toString', 'function', 0],
    ];

    var emptyGRCNMembers = [
        ['getMyClassName', 'function', 0],
        ['toString', 'function', 0],
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

    var shopDimensionExpected = [
    ['baseArea', 'object', shopAreaExpected],
    ['height', 'number', 1.1]
    ];

    // name, type, length
    var wineryExpected = [
        ['addEventListener', 'function', 2],
        ['allowForWebAsyncOperationOut', 'function', 0],
        ['asyncOperationOut', 'function', 0],
        ['asyncOperationOutAfterExecuteDelegate', 'function', 1],
        ['asyncOperationOutAfterExecuteDelegateWithAsyncInParameter', 'function', 1],
        ['asyncOperationOutAfterExecuteDelegateWithAsyncInParameterUseSameAsyncObject', 'function', 1],
        ['asyncOperationViaDelegate', 'function', 1],
        ['asyncOperationWithMultipleAsyncOutParameters', 'function', 0],
        ['asyncOperationWithMultipleOutParameters', 'function', 0],
        ['clearWarehouse', 'function', 0],
        ['getBestSellingRed', 'function', 0],
        ['getBestSellingSweet', 'function', 0],
        ['getBestSellingWhite', 'function', 0],
        ['getDeprecatedAttributes', 'function', 0],
        ['getDiamond', 'function', 0],
        ['getDiamondAsInterface', 'function', 0],
        ['getEnumerableOfDefaultInterface', 'function', 0],
        ['getEnumerableOfDefaultInterfaceWithMultipleSameName', 'function', 0],
        ['getEnumerableOfItself', 'function', 0],
        ['getEnumerableOfItselfAsRTC', 'function', 0],
        ['getInheritedConflict', 'function', 0],
        ['getInheritedConflictAsInterface', 'function', 0],
        ['getNameConflictingWithOverloadSet', 'function', 0],
        ['getNameConflictingWithOverloadSetAsInterface', 'function', 0],
        ['getSimpleConflict', 'function', 0],
        ['getSimpleConflictAsInterface', 'function', 0],
        ['getSimpleDefaultOverloadSet', 'function', 0],
        ['getSimpleDefaultOverloadSetAsInterface', 'function', 0],
        ['getSimpleOverloadSet', 'function', 0],
        ['getSimpleOverloadSetAsInterface', 'function', 0],
        ['initDatabase', 'function', 0],
        ['marshalIGeneralShop', 'function', 1],
        ['marshalIProductionLine', 'function', 1],
        ['marshalIRetail', 'function', 1],
        ['marshalIWarehouse', 'function', 1],
        ['marshalIWineRetail', 'function', 1],
        ['onagecompleteevent', 'object'],
        ['produce', 'function', 0],
        ['removeEventListener', 'function', 2],
        ['sellReds', 'function', 2],
        ['sellSweets', 'function', 2],
        ['sellWhites', 'function', 2],
        ['sendToWarehouse', 'function', 1],
        ['shopArea', 'object'],
        ['shopDimension', 'object'],
        ['shopName', 'string'],
        ['storeAgedWine', 'function', 0],
        ['throwVinegar', 'function', 1],
        ['welcomeMessage', 'string'],
        ['wineInStorage', 'number'],
        ['toString', 'function', 0],
    ];

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
        ['toString', 'function', 0],
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
        desc: 'Winery.shopArea',
        pri: '0',
        test: function () {
            verify(("shopArea" in winery), true, "Winery has property shopArea");
            winery.shopArea = { width: 100, length: 120 };
            verifyShopArea(winery.shopArea, shopAreaExpected);
            verifyShopArea(winery['shopArea'], shopAreaExpected);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Winery.shopDimension',
        pri: '0',
        test: function () {
            verify(("shopDimension" in winery), true, "Winery has property shopDimension");
            winery.shopDimension = { baseArea: { width: 100, length: 120 }, height: 1.1 };
            verifyShopDimension(winery.shopDimension, shopDimensionExpected);
            verifyShopDimension(winery['shopDimension'], shopDimensionExpected);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Winery.welcomeMessage',
        pri: '0',
        test: function () {
            verify(("welcomeMessage" in winery), true, "Winery has property welcomeMessage");
            winery.welcomeMessage = "hello world";
            verifyMembers(winery, wineryExpected, 'object');
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Calling a method on a required interface',
        pri: '0',
        test: function () {
            verify(fish.singTheSwimmingSong(), songExpected, 'fish.singTheSwimmingSong()');
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Verify Fish',
        pri: '0',
        test: function () {
            verifyMembers(fish, fishExpected, 'object');
        }
    });

    runner.addTest({
        id: 7,
        desc: 'marshalIFish',
        pri: '0',
        test: function () {
            var fish2 = fish.marshalIFish(fish)
            verifyMembers(fish2, fishExpected, 'object');
            verify(fish2.getNumFins(), 5, 'fish2.getNumFins()');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'marshalILikeToSwim',
        pri: '0',
        test: function () {
            var swimmer = fish.marshalILikeToSwim(fish)
            verifyMembers(swimmer, fishExpected, 'object');
            verify(swimmer.singTheSwimmingSong(), songExpected, 'swimmer.singTheSwimmingSong()');
            verify(swimmer.getNumFins(), 5, 'swimmer.getNumFins()');
        }
    });


    runner.addTest({
        id: 9,
        desc: 'marshalIFishToFish',
        pri: '0',
        test: function () {
            var fish2 = fish.marshalIFishToFish(fish);
            verifyMembers(fish2, fishExpected, 'object');
            verify(fish2.singTheSwimmingSong(), songExpected, 'fish2.singTheSwimmingSong()');
        }
    });

    runner.addTest({
        id: 10,
        desc: 'marshalILikeToSwimToFish',
        pri: '0',
        test: function () {
            var fish2 = fish.marshalILikeToSwimToFish(fish)
            verifyMembers(fish2, fishExpected, 'object');
            verify(fish2.getNumFins(), 5, 'fish2.getNumFins()');
        }
    });

    runner.addTest({
        id: "Win8: 195355",
        desc: 'Interface with struct properties',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            verifyMembers(myAnimal, animalMembersExpected, 'object');
        }
    });

    runner.addTest({
        id: 13,
        desc: 'IVector out on instance',
        pri: '0',
        test: function () {
            var vectorMembers = [
                ['0', 'number'],
                ['1', 'number'],
                ['2', 'number'],
                ['append', 'function', 1],
                ['clear', 'function', 0],
                ['first', 'function', 0],
                ['getAt', 'function', 1],
                ['getMany', 'function', 2],
                ['getView', 'function', 0],
                ['indexOf', 'function', 1],
                ['insertAt', 'function', 2],
                ['removeAt', 'function', 1],
                ['removeAtEnd', 'function', 0],
                ['replaceAll', 'function', 1],
                ['setAt', 'function', 2],
                ['size', 'number'],
            ];
            var vector = fish.getOneVector();
            verifyMembers(vector, vectorMembers, 'object');

            // Verify vector is specialized
            verify(vector.length, vector.size, "vector.length");
            for (var i = 0; i < vector.length; i++) {
                verify(vector[i], i + 1, "vector[" + i + "]");
            }

            vector = fish.getNullAsVector();
            verify(vector, null, "Vector as Null");
        }
    });

    runner.addTest({
        id: 14,
        desc: 'IObservableVector out on instance',
        pri: '0',
        test: function () {
            var vectorMembers = [
                ['0', 'number'],
                ['1', 'number'],
                ['2', 'number'],
                ['3', 'number'],
                ['addEventListener', 'function', 2],
                ['append', 'function', 1],
                ['clear', 'function', 0],
                ['first', 'function', 0],
                ['getAt', 'function', 1],
                ['getMany', 'function', 2],
                ['getView', 'function', 0],
                ['indexOf', 'function', 1],
                ['insertAt', 'function', 2],
                ['onvectorchanged', 'object'],
                ['removeAt', 'function', 1],
                ['removeAtEnd', 'function', 0],
                ['removeEventListener', 'function', 2],
                ['replaceAll', 'function', 1],
                ['setAt', 'function', 2],
                ['size', 'number'],
            ];
            var vector = fish.getOneObservableVector();
            verifyMembers(vector, vectorMembers, 'object');

            // Verify vector is specialized
            verify(vector.length, vector.size, "vector.length");
            for (var i = 0; i < vector.length; i++) {
                verify(vector[i], i + 1, "vector[" + i + "]");
            }

            // Verify events work
            var invokeCount = 0;
            vector.onvectorchanged = function (ev) {
                logger.comment("onvectorchanged : Invoked");
                verify(ev.target, vector, "ev.target");
                verify(ev.index, 2, "ev.index");
                verify(ev.collectionChange, 3, "ev.collectionChange");
                verify(ev.type, "vectorchanged", "ev.type");
                logger.comment("onvectorchanged : Exit");
                invokeCount++;
            }
            vector[2] = 33;
            verify(invokeCount, 1, "InvokeCount");
            verify(vector[2], 33, "vector[2]");

            vector.onvectorchanged = null;
            vector[2] = 44;
            verify(invokeCount, 1, "InvokeCount");
            verify(vector[2], 44, "vector[2]");

            function vectorChangedHanler(ev) {
                logger.comment("vectorchanged : Invoked");
                verify(ev.target, vector, "ev.target");
                verify(ev.index, 1, "ev.index");
                verify(ev.collectionChange, 3, "ev.collectionChange");
                verify(ev.type, "vectorchanged", "ev.type");
                logger.comment("vectorchanged : Exit");
                invokeCount++;
            }
            vector.addEventListener("vectorchanged", vectorChangedHanler);
            vector[1] = 33;
            verify(invokeCount, 2, "InvokeCount");
            verify(vector[1], 33, "vector[1]");

            vector.removeEventListener("vectorchanged", vectorChangedHanler);
            vector[1] = 44;
            verify(invokeCount, 2, "InvokeCount");
            verify(vector[1], 44, "vector[1]");



            vector = fish.getNullAsObservableVector();
            verify(vector, null, "ObservableVector as Null");
        }
    });

    runner.addTest({
        id: 15,
        desc: 'Interface out with RuntimeClass',
        pri: '0',
        test: function () {
            var oneAnimal = fish.getOneAnimal();
            verifyMembers(oneAnimal, animalMembersExpected, 'object');

            var mother = new Animals.Animal(2);
            oneAnimal.mother = mother;
            verify(oneAnimal.mother, mother, "oneAnimal.mother");
            verify(oneAnimal.getGreeting(), "Hello", "oneAnimal.getGreeting()");

            oneAnimal = fish.getNullAsAnimal();
            verify(oneAnimal, null, "Null as animal");
        }
    });

    runner.addTest({
        id: 16,
        desc: 'Interface out with PropertyValue specialization',
        pri: '0',
        test: function () {
            var pvVal = fish.getOnePropertyValue();
            verify(pvVal, 10.5, "pvVal");

            pvVal = fish.getNullAsPropertyValue();
            verify(pvVal, null, "Null as PropertyValue");
        }
    });

    runner.addTest({
        id: 17,
        desc: 'Interface out with Map specialization',
        pri: '0',
        test: function () {
            var oneMap = fish.getOneMap();
            verifyMembers(oneMap, mapMembers, "object");
            verify(oneMap["by"], oneMap.lookup("by"), 'oneMap["by"]');
            verify(oneMap["Hundred"], oneMap.lookup("Hundred"), 'oneMap["Hundred"]');
            verify(oneMap["Hundred And Fifty"], oneMap.lookup("Hundred And Fifty"), 'oneMap["Hundred And Fifty"]');

            oneMap["Map"] = 3;
            verify(oneMap["Map"], 3, 'oneMap["Map"]');
            verify(oneMap["Map"], oneMap.lookup("Map"), 'oneMap["Map"]');

            oneMap = fish.getNullAsMap();
            verify(oneMap, null, "Null as Map");
        }
    });

    runner.addTest({
        id: 18,
        desc: 'Interface out with Interface ClassName',
        pri: '0',
        test: function () {
            var emptyGRCN = fish.getOneEmptyGRCNInterface();
            verifyMembers(emptyGRCN, emptyGRCNMembers, "object");
            verify(emptyGRCN.getMyClassName(), "CEmptyGRCNInterface", "getMyClassName()");
        }
    });

    runner.addTest({
        id: 19,
        desc: 'Interface out with "" as classname',
        pri: '0',
        test: function () {
            var emptyGRCN = fish.getOneEmptyGRCNNull();
            verifyMembers(emptyGRCN, emptyGRCNMembers, "object");
            verify(emptyGRCN.getMyClassName(), "CEmptyGRCN", "getMyClassName()");
        }
    });

    runner.addTest({
        id: 20,
        desc: 'Interface out with failing GRCN',
        pri: '0',
        test: function () {
            var emptyGRCN = fish.getOneEmptyGRCNFail();
            verifyMembers(emptyGRCN, emptyGRCNMembers, "object");
            verify(emptyGRCN.getMyClassName(), "CEmptyFailingGRCNString", "getMyClassName()");
        }
    });


    runner.addTest({
        id: 21,
        desc: 'Static : IVector out',
        pri: '0',
        test: function () {
            var vectorMembers = [
                ['0', 'number'],
                ['1', 'number'],
                ['2', 'number'],
                ['append', 'function', 1],
                ['clear', 'function', 0],
                ['first', 'function', 0],
                ['getAt', 'function', 1],
                ['getMany', 'function', 2],
                ['getView', 'function', 0],
                ['indexOf', 'function', 1],
                ['insertAt', 'function', 2],
                ['removeAt', 'function', 1],
                ['removeAtEnd', 'function', 0],
                ['replaceAll', 'function', 1],
                ['setAt', 'function', 2],
                ['size', 'number'],
            ];
            var vector = Animals.Animal.getOneVector();
            verifyMembers(vector, vectorMembers, 'object');

            // Verify vector is specialized
            verify(vector.length, vector.size, "vector.length");
            for (var i = 0; i < vector.length; i++) {
                verify(vector[i], i + 1, "vector[" + i + "]");
            }

            vector = Animals.Animal.getNullAsVector();
            verify(vector, null, "Vector as Null");
        }
    });

    runner.addTest({
        id: 22,
        desc: 'Static : IObservableVector out',
        pri: '0',
        test: function () {
            var vectorMembers = [
                ['0', 'number'],
                ['1', 'number'],
                ['2', 'number'],
                ['3', 'number'],
                ['addEventListener', 'function', 2],
                ['append', 'function', 1],
                ['clear', 'function', 0],
                ['first', 'function', 0],
                ['getAt', 'function', 1],
                ['getMany', 'function', 2],
                ['getView', 'function', 0],
                ['indexOf', 'function', 1],
                ['insertAt', 'function', 2],
                ['onvectorchanged', 'object'],
                ['removeAt', 'function', 1],
                ['removeAtEnd', 'function', 0],
                ['removeEventListener', 'function', 2],
                ['replaceAll', 'function', 1],
                ['setAt', 'function', 2],
                ['size', 'number'],
            ];
            var vector = Animals.Animal.getOneObservableVector();
            verifyMembers(vector, vectorMembers, 'object');

            // Verify vector is specialized
            verify(vector.length, vector.size, "vector.length");
            for (var i = 0; i < vector.length; i++) {
                verify(vector[i], i + 1, "vector[" + i + "]");
            }

            // Verify events work
            var invokeCount = 0;
            vector.onvectorchanged = function (ev) {
                logger.comment("onvectorchanged : Invoked");
                verify(ev.target, vector, "ev.target");
                verify(ev.index, 2, "ev.index");
                verify(ev.collectionChange, 3, "ev.collectionChange");
                verify(ev.type, "vectorchanged", "ev.type");
                logger.comment("onvectorchanged : Exit");
                invokeCount++;
            }
            vector[2] = 33;
            verify(invokeCount, 1, "InvokeCount");
            verify(vector[2], 33, "vector[2]");

            vector.onvectorchanged = null;
            vector[2] = 44;
            verify(invokeCount, 1, "InvokeCount");
            verify(vector[2], 44, "vector[2]");

            function vectorChangedHanler(ev) {
                logger.comment("vectorchanged : Invoked");
                verify(ev.target, vector, "ev.target");
                verify(ev.index, 1, "ev.index");
                verify(ev.collectionChange, 3, "ev.collectionChange");
                verify(ev.type, "vectorchanged", "ev.type");
                logger.comment("vectorchanged : Exit");
                invokeCount++;
            }
            vector.addEventListener("vectorchanged", vectorChangedHanler);
            vector[1] = 33;
            verify(invokeCount, 2, "InvokeCount");
            verify(vector[1], 33, "vector[1]");

            vector.removeEventListener("vectorchanged", vectorChangedHanler);
            vector[1] = 44;
            verify(invokeCount, 2, "InvokeCount");
            verify(vector[1], 44, "vector[1]");



            vector = Animals.Animal.getNullAsObservableVector();
            verify(vector, null, "ObservableVector as Null");
        }
    });

    runner.addTest({
        id: 23,
        desc: 'Static : Interface out with RuntimeClass',
        pri: '0',
        test: function () {
            var oneAnimal = Animals.Animal.getOneAnimal();
            verifyMembers(oneAnimal, animalMembersExpected, 'object');

            var mother = new Animals.Animal(2);
            oneAnimal.mother = mother;
            verify(oneAnimal.mother, mother, "oneAnimal.mother");
            verify(oneAnimal.getGreeting(), "Hello", "oneAnimal.getGreeting()");

            oneAnimal = Animals.Animal.getNullAsAnimal();
            verify(oneAnimal, null, "Null as animal");
        }
    });

    runner.addTest({
        id: 24,
        desc: 'Static : Interface out with PropertyValue specialization',
        pri: '0',
        test: function () {
            var pvVal = Animals.Animal.getOnePropertyValue();
            verify(pvVal, 10.5, "pvVal");

            pvVal = Animals.Animal.getNullAsPropertyValue();
            verify(pvVal, null, "Null as PropertyValue");
        }
    });

    runner.addTest({
        id: 25,
        desc: 'Static : Interface out with Map specialization',
        pri: '0',
        test: function () {
            var oneMap = Animals.Animal.getOneMap();
            verifyMembers(oneMap, mapMembers, "object");
            verify(oneMap["by"], oneMap.lookup("by"), 'oneMap["by"]');
            verify(oneMap["Hundred"], oneMap.lookup("Hundred"), 'oneMap["Hundred"]');
            verify(oneMap["Hundred And Fifty"], oneMap.lookup("Hundred And Fifty"), 'oneMap["Hundred And Fifty"]');

            oneMap["Map"] = 3;
            verify(oneMap["Map"], 3, 'oneMap["Map"]');
            verify(oneMap["Map"], oneMap.lookup("Map"), 'oneMap["Map"]');

            oneMap = Animals.Animal.getNullAsMap();
            verify(oneMap, null, "Null as Map");
        }
    });

    runner.addTest({
        id: 26,
        desc: 'Static : Interface out with Interface ClassName',
        pri: '0',
        test: function () {
            var emptyGRCN = Animals.Animal.getOneEmptyGRCNInterface();
            verifyMembers(emptyGRCN, emptyGRCNMembers, "object");
            verify(emptyGRCN.getMyClassName(), "CEmptyGRCNInterface", "getMyClassName()");
        }
    });

    runner.addTest({
        id: 27,
        desc: 'Static : Interface out with "" as classname',
        pri: '0',
        test: function () {
            var emptyGRCN = Animals.Animal.getOneEmptyGRCNNull();
            verifyMembers(emptyGRCN, emptyGRCNMembers, "object");
            verify(emptyGRCN.getMyClassName(), "CEmptyGRCN", "getMyClassName()");
        }
    });

    runner.addTest({
        id: 28,
        desc: 'Static : Interface out with failing GRCN',
        pri: '0',
        test: function () {
            var emptyGRCN = Animals.Animal.getOneEmptyGRCNFail();
            verifyMembers(emptyGRCN, emptyGRCNMembers, "object");
            verify(emptyGRCN.getMyClassName(), "CEmptyFailingGRCNString", "getMyClassName()");
        }
    });

    runner.addTest({
        id: 29,
        desc: 'Default Interfaces',
        pri: '0',
        test: function () {
            var dino = new Animals.Dino();
            var isSame = Animals.Animal.testDefaultDino(dino);
            verify(isSame, true, "Animals.Animal.testDefaultDino(dino)");

            var fish = new Animals.Fish();
            var isSame = Animals.Animal.testDefaultFish(fish);
            verify(isSame, true, "Animals.Animal.testDefaultFish(fish)");

            var animal = new Animals.Animal(1);
            var isSame = Animals.Animal.testDefaultAnimal(animal);
            verify(isSame, true, "Animals.Animal.testDefaultAnimal(animal)");

            var multiVector = new Animals.MultipleIVector();
            var isSame = Animals.Animal.testDefaultMultipleIVector(multiVector);
            verify(isSame, true, "Animals.Animal.testDefaultMultipleIVector(multiVector)");
        }
    });
    Loader42_FileName = 'Interfaces test';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
