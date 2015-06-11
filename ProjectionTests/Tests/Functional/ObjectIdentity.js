if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var myFishMembers = [
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

    var myVectorMembers = [
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

    var myIteratorMembers = [
        ['current', 'number'],
        ['getMany', 'function', 1],
        ['hasCurrent', 'boolean'],
        ['moveNext', 'function', 0],
        ['toString', 'function', 0],
    ];

    var myVectorViewMembers = [
        ['first', 'function', 0],
        ['getAt', 'function', 1],
        ['getMany', 'function', 2],
        ['indexOf', 'function', 1],
        ['size', 'number'],
    ];

    var myDinoMembers = [
        ['Animals.IDino.hasTeeth', 'function', 0],
        ['Animals.IExtinct.hasTeeth', 'function', 0],
        ['canRoar', 'function', 0],
        ['height', 'number'],
        ['isExtinct', 'function', 0],
        ['roar', 'function', 1],
        ['toString', 'function', 0],
    ];

    var myMultipleIVectorMembers = [
        ['Windows.Foundation.Collections.IIterable`1<Animals.IAnimal>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<Int32>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<String>.first', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<Int32>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<String>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.size', 'number'],
        ['toString', 'function', 0],
    ];

    var myToasterMembers = [
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

    var toastMembers = [
        ['message', 'string'],
        ['toString', 'function', 0],
    ];

    var evToastMembers = [
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
        ['message', 'string'],
        ['toString', 'function', 0],
    ];

    var electricityReporterMembers = [
        ['addEventListener', 'function', 2],
        ['applianceName', 'string'],
        ['getSameElectricityReporter', 'function', 0],
        ['onapplianceswitchoffevent', 'object'],
        ['onapplianceswitchonevent', 'object'],
        ['removeEventListener', 'function', 2],
        ['toString', 'function', 0],
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

    function dumpObjectMembers(myObjectString, myObject) {
        var objectDump = easyMembersPrint(myObjectString, myObject);

        logger.comment("typeof " + myObjectString + ": " + typeof myObject);
        logger.comment("Dump of properties : " + objectDump);
    }

    function verifyMembers(myObjectString, myObject, expectedProperties, myArray) {
        dumpObjectMembers(myObjectString, myObject);

        verify(typeof myObject, "object", "typeof " + myObjectString);

        var checkArray = 0;
        var numberOfElements = 0;
        var propertiesIndex = 0;

        for (p in myObject) {
            if (Array.isArray(myArray) && checkArray < myArray.length) {
                verify(p, checkArray.toString(), myObjectString + '\'s property');
                verify(typeof myObject[p], typeof myArray[checkArray], 'typeof ' + myObjectString + '["' + p + '"]');
                verify(myObject[p], myArray[checkArray], myObjectString + '["' + p + '"]');
                checkArray++;
            }
            else {
                // Look in properties
                verify(p, expectedProperties[propertiesIndex][0], myObjectString + '\'s property');
                verify(typeof myObject[p], expectedProperties[propertiesIndex][1], 'typeof ' + myObjectString + '["' + p + '"]');

                if (typeof myObject[p] == 'function') {
                    verify(myObject[p].length, expectedProperties[propertiesIndex][2], myObjectString + '["' + p + '"].length');
                    logger.comment('Setting length of function to be 10');
                    myObject[p].length = 10;
                    verify(myObject[p].length, expectedProperties[propertiesIndex][2], myObjectString + '["' + p + '"].length');
                }
                propertiesIndex++;
            }

            numberOfElements++;
        }

        var exptectedPropertiesLength = expectedProperties.length;
        if (Array.isArray(myArray)) {
            exptectedPropertiesLength = exptectedPropertiesLength + myArray.length;
        }

        verify(numberOfElements, exptectedPropertiesLength, 'number of properties of ' + myObjectString);
    }

    function VerifyArrayAndVectorContents(myVectorString, actualVector, myArrayString, actualArray) {
        verify(actualVector.length, actualArray.length, myVectorString + ".length");

        for (var i = 0; i < actualArray.length; i++) {
            assert(actualVector[i] === actualArray[i], myVectorString + "[" + i + "] === " + myArrayString + "[" + i + "]");
        }
    }

    runner.addTest({
        id: 1,
        desc: 'GetBackSameSimpleRuntimeClass',
        pri: '0',
        test: function () {
            logger.comment("Create myFish");
            var myFish = new Animals.Fish();
            myFish.name = "Nemo";
            verifyMembers("myFish", myFish, myFishMembers);
            verify(myFish.name, "Nemo", "myFish.name");

            logger.comment("Get mySameFish same as myFish");
            var mySameFish = Animals.Animal.sendBackSameFish(myFish);
            verifyMembers("mySameFish", mySameFish, myFishMembers);
            verify(mySameFish.name, "Nemo", "mySameFish.name");

            // Equality tests
            logger.comment("Equality Tests on myFish and mySameFish");
            assert(myFish == mySameFish, "myFish == mySameFish");
            assert(myFish === mySameFish, "myFish === mySameFish");

            logger.comment("Get new fish as mySecondFish");
            var mySecondFish = new Animals.Fish();
            mySecondFish.name = "Dori";
            verify(mySecondFish.name, "Dori", "mySecondFish.name");

            // Equality tests
            logger.comment("Equality Tests on mySecondFish with myFish and mySecondFish");
            assert(myFish != mySecondFish, "myFish != mySecondFish");
            assert(myFish !== mySecondFish, "myFish !== mySecondFish");
            assert(mySameFish != mySecondFish, "mySameFish != mySecondFish");
            assert(mySameFish !== mySecondFish, "mySameFish !== mySecondFish");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Abi as Expandos -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // function expandos -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Accessing abis as part of getter and setter -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Lifetime
            logger.comment("Lifetime tests");

            // Set my Fish to null and check if samefish can still retain the expandos and other values
            logger.comment("Set my Fish to null and check if samefish can still retain the expandos and other values");
            myFish = new Animals.Fish();
            myFish.name = "Marlin"

            logger.comment("Verifying that myFish doesnt have the expandos");
            verify(myFish.name, "Marlin", "myFish.name");
            assert(myFish !== mySameFish, "myFish !== mySameFish");

            // verify that mySameFish still is intact?
            logger.comment("Verifying that mySameFish still has the expandos");
            verify(mySameFish.name, "Nemo", "mySameFish.name");

            // Pass mySameFish to Method that keeps fish into the ABI
            logger.comment("Pass mySameFish to Method that keeps fish into the ABI");
            Animals.Animal.myFish = mySameFish;
            assert(Animals.Animal.myFish == mySameFish, "Animals.Animal.myFish == mySameFish");
            assert(Animals.Animal.myFish === mySameFish, "Animals.Animal.myFish === mySameFish");

            // Set the mySameFish to null so that we dont have myFish instance anywhere in JS land
            logger.comment("Set the mySameFish to null so that we dont have myFish instance anywhere in JS land");
            mySameFish = null;
            thisObject = null;

            logger.comment("Perform gc");
            CollectGarbage();
            CollectGarbage();

            // Get the fish back from winrt and check that expandos exist
            logger.comment("Get the fish back from winrt");
            mySameFish = Animals.Animal.myFish;
            thisObject = mySameFish;

            // verify that mySameFish still is intact?
            logger.comment("Verifying that mySameFish still has the expandos");
            verify(mySameFish.name, "Nemo", "mySameFish.name");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'GetBackSameParameterizedInterface',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            logger.comment("Get first vector as myVector");
            var myAnimal = new Animals.Animal(1);
            var myVector = myAnimal.getVector();
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyMembers("myVector", myVector, myVectorMembers, myArray);

            logger.comment("Get myVector back as mySameVector");
            var mySameVector = myAnimal.sendBackSameVector(myVector);
            verifyMembers("mySameVector", mySameVector, myVectorMembers, myArray);

            // Equality tests
            logger.comment("Equality tests on myVector and mySameVector");
            assert(myVector == mySameVector, "myVector == mySameVector");
            assert(myVector === mySameVector, "myVector === mySameVector");

            logger.comment("Get new vector as mySecondVector");
            var mySecondVector = myAnimal.getVector();
            mySecondVector.length = 4;
            var mySecondArray = [1, 2, 3, 4];
            verifyMembers("mySecondVector", mySecondVector, myVectorMembers, mySecondArray);

            // Equality tests
            logger.comment("Equality tests on mySecondVector with myVector and mySecondVector");
            assert(myVector != mySecondVector, "myVector != mySecondVector");
            assert(myVector !== mySecondVector, "myVector !== mySecondVector");
            assert(mySameVector != mySecondVector, "mySameVector != mySecondVector");
            assert(mySameVector !== mySecondVector, "mySameVector !== mySecondVector");

            // Vector with same contents but different vector
            logger.comment("Create third vector with same contents but different vector and check equality");
            var myThirdVector = myAnimal.getVector();
            myThirdVector.length = 4;
            assert(mySecondVector != myThirdVector, "mySecondVector != myThirdVector");
            assert(mySecondVector !== myThirdVector, "mySecondVector !== myThirdVector");

            // With different interface.
            logger.comment("Create Fish and perform equality tests with vector");
            var myFish = new Animals.Fish();
            assert(myVector != myFish, "myVector != myFish");
            assert(myVector !== myFish, "myVector !== myFish");

            // With different type of vector
            logger.comment("Get the string Vector and check its not equal with any of the vectors we got earlier");
            var myStringVector = myAnimal.getStringVector();
            assert(myVector != myStringVector, "myVector != myStringVector");
            assert(myVector !== myStringVector, "myVector !== myStringVector");
            assert(mySecondVector != myStringVector, "mySecondVector != myStringVector");
            assert(mySecondVector !== myStringVector, "mySecondVector !== myStringVector");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Abi as Expandos -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // function expandos -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Accessing abis as part of getter and setter -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            logger.comment("Check the view and iterables are not identical");
            var myIterator = myVector.first();
            verifyMembers("myIterator", myIterator, myIteratorMembers);

            logger.comment("Equality and Expando tests on : myIterator and myVector");
            assert(myIterator != myVector, "myIterator != myVector");
            assert(myIterator !== myVector, "myIterator !== myVector");

            var myVectorView = myVector.getView();
            verifyMembers("myVectorView", myVectorView, myVectorViewMembers, myArray);

            logger.comment("Equality tests on : myVectorView and myVector");
            assert(myVectorView != myVector, "myVectorView != myVector");
            assert(myVectorView !== myVector, "myVectorView !== myVector");

            // Lifetime
            logger.comment("Lifetime tests");

            // Set myVector to null and check if sameVector can still retain the expandos and other values
            logger.comment("Set myVector to null and check if sameVector can still retain the expandos and other values");
            myVector = myAnimal.getVector();

            logger.comment("Verifying that myVector doesnt have the expandos");
            verify(myVector.length, 9, "myVector.length");
            assert(myVector !== mySameVector, "myVector !== mySameVector");

            // Same vector is still alive with expandos
            logger.comment("Verifying that mySameVector still has the expandos");
            verify(mySameVector.length, 9, "mySameVector.length");

            // Pass mySameVector to Method that keeps vector into the ABI
            logger.comment("Pass mySameVector to Method that keeps vector into the ABI");
            myAnimal.myVector = mySameVector;
            assert(myAnimal.myVector == mySameVector, "myAnimal.myVector == mySameVector");
            assert(myAnimal.myVector === mySameVector, "myAnimal.myVector === mySameVector");

            // Set the mySameVector to null so that we dont have myVector instance anywhere in JS land
            logger.comment("Set the mySameVector to null so that we dont have myVector instance anywhere in JS land");
            mySameVector = null;

            logger.comment("Perform gc");
            CollectGarbage();

            // Get the vector back from winrt and check that expandos exist
            logger.comment("Get the vector back from winrt");
            mySameVector = myAnimal.myVector;

            // verify that mySameVector still is intact?
            logger.comment("Verifying that mySameVector still has the expandos");
            verify(mySameVector.length, 9, "mySameVector.length");
            myArray[6] = 40;
            myArray[7] = 90;
            myArray[8] = 100;
            mySameVector.append(40);
            mySameVector.append(90);
            mySameVector.append(100);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'GetBackSameSimpleInterface',
        pri: '0',
        test: function () {
            logger.comment("Create myFish");
            var myFish = new Animals.Fish();
            myFish.name = "Nemo";
            verifyMembers("myFish", myFish, myFishMembers);
            verify(myFish.name, "Nemo", "myFish.name");

            logger.comment("Get mySameFish same as myFish");
            var mySameFish = Animals.Animal.sendBackSameIFish(myFish);
            verifyMembers("mySameFish", mySameFish, myFishMembers);
            verify(mySameFish.name, "Nemo", "mySameFish.name");

            // Equality tests
            logger.comment("Equality Tests on myFish and mySameFish");
            assert(myFish == mySameFish, "myFish == mySameFish");
            assert(myFish === mySameFish, "myFish === mySameFish");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Lifetime
            logger.comment("Lifetime tests");

            // Set my Fish to null
            logger.comment("Set my Fish to null");
            myFish = null;

            // verify that mySameFish still is intact?
            logger.comment("Verifying that mySameFish still has the expandos");
            verify(mySameFish.name, "Nemo", "mySameFish.name");

            // Pass mySameFish to Method that keeps fish into the ABI
            logger.comment("Pass mySameFish to Method that keeps fish into the ABI");
            Animals.Animal.myIFish = mySameFish;
            assert(Animals.Animal.myIFish == mySameFish, "Animals.Animal.myIFish == mySameFish");
            assert(Animals.Animal.myIFish === mySameFish, "Animals.Animal.myIFish === mySameFish");

            // Set the mySameFish to null so that we dont have myFish instance anywhere in JS land
            logger.comment("Set the mySameFish to null");
            mySameFish = null;

            logger.comment("Perform gc");
            CollectGarbage();
            CollectGarbage();

            // Get the fish back from winrt and check that expandos exist
            logger.comment("Get the fish back from winrt");
            mySameFish = Animals.Animal.myIFish;

            // verify that mySameFish still is intact?
            logger.comment("Verifying that mySameFish still has the expandos");
            verify(mySameFish.name, "Nemo", "mySameFish.name");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'GetBackSameSimpleRequiredInterface',
        pri: '0',
        test: function () {
            logger.comment("Create myFish");
            var myFish = new Animals.Fish();
            myFish.name = "Nemo";
            verifyMembers("myFish", myFish, myFishMembers);
            verify(myFish.name, "Nemo", "myFish.name");

            logger.comment("Get mySameFish same as myFish");
            var mySameFish = Animals.Animal.sendBackSameLikeToSwim(myFish);
            verifyMembers("mySameFish", mySameFish, myFishMembers);
            verify(mySameFish.name, "Nemo", "mySameFish.name");

            // Equality tests
            logger.comment("Equality Tests on myFish and mySameFish");
            assert(myFish == mySameFish, "myFish == mySameFish");
            assert(myFish === mySameFish, "myFish === mySameFish");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Lifetime
            logger.comment("Lifetime tests");

            // Set my Fish to null
            logger.comment("Set my Fish to null");
            myFish = null;

            // verify that mySameFish still is intact?
            logger.comment("Verifying that mySameFish still has the expandos");
            verify(mySameFish.name, "Nemo", "mySameFish.name");

            // Pass mySameFish to Method that keeps fish into the ABI
            logger.comment("Pass mySameFish to Method that keeps fish into the ABI");
            Animals.Animal.myLikeToSwim = mySameFish;
            assert(Animals.Animal.myLikeToSwim == mySameFish, "Animals.Animal.myLikeToSwim == mySameFish");
            assert(Animals.Animal.myLikeToSwim === mySameFish, "Animals.Animal.myLikeToSwim === mySameFish");

            // Set the mySameFish to null so that we dont have myFish instance anywhere in JS land
            logger.comment("Set the mySameFish to null");
            mySameFish = null;

            logger.comment("Perform gc");
            CollectGarbage();
            CollectGarbage();

            // Get the fish back from winrt and check that expandos exist
            logger.comment("Get the fish back from winrt");
            mySameFish = Animals.Animal.myLikeToSwim;

            // verify that mySameFish still is intact?
            logger.comment("Verifying that mySameFish still has the expandos");
            verify(mySameFish.name, "Nemo", "mySameFish.name");
        }
    });

    runner.addTest({
        id: 5,
        desc: 'GetBackSameParameterizedRequiredInterface',
        pri: '0',
        test: function () {
            logger.comment("Get first vector as myVector");
            var myAnimal = new Animals.Animal(1);
            var myVector = myAnimal.getVector();
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyMembers("myVector", myVector, myVectorMembers, myArray);

            logger.comment("Get myVector back as Iterable = mySameVector");
            var mySameVector = myAnimal.sendBackSameIterable(myVector);
            verifyMembers("mySameVector", mySameVector, myVectorMembers, myArray);

            // Equality tests
            logger.comment("Equality tests on myVector and mySameVector");
            assert(myVector == mySameVector, "myVector == mySameVector");
            assert(myVector === mySameVector, "myVector === mySameVector");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Lifetime
            logger.comment("Lifetime tests");

            // Set myVector to null
            logger.comment("Set myVector to null");
            myVector = null;

            // verify that mySameVector still is intact?
            logger.comment("Verifying that mySameVector still has the expandos");
            verify(mySameVector.length, 9, "mySameVector.length");

            // Pass mySameVector to Method that keeps vector into the ABI
            logger.comment("Pass mySameVector to Method that keeps vector into the ABI");
            myAnimal.myIterable = mySameVector;
            assert(myAnimal.myIterable == mySameVector, "myAnimal.myIterable == mySameVector");
            assert(myAnimal.myIterable === mySameVector, "myAnimal.myIterable === mySameVector");

            // Set the mySameVector to null so that we dont have myVector instance anywhere in JS land
            logger.comment("Set the mySameVector to null");
            mySameVector = null;

            logger.comment("Perform gc");
            CollectGarbage();

            // Get the fish back from winrt and check that expandos exist
            logger.comment("Get the iterable back from winrt");
            mySameVector = myAnimal.myIterable;

            // verify that mySameVector still is intact?
            logger.comment("Verifying that mySameVector still has the expandos");
            verify(mySameVector.length, 9, "mySameVector.length");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'DelegateWithSimpleRuntimeClass',
        pri: '0',
        test: function () {
            logger.comment("Create myFish");
            var myFish = new Animals.Fish();
            myFish.name = "Nemo";
            myFish.myExpando = "This is myExpando";
            verifyMembers("myFish", myFish, myFishMembers);
            verify(myFish.name, "Nemo", "myFish.name");

            // Pass mySameFish to Method that keeps fish into the ABI
            logger.comment("Pass myFish to Method that keeps fish into the ABI");
            Animals.Animal.myFish = myFish;

            var returnFish = false;

            function myDelegate(delegateMyFish) {
                logger.comment("*** myDelegate: Invoke");
                verifyMembers("delegateMyFish", delegateMyFish, myFishMembers);
                verify(delegateMyFish.name, "Nemo", "delegateMyFish.name");

                logger.comment("Equality Tests on myFish and delegateMyFish");
                assert(myFish == delegateMyFish, "myFish == delegateMyFish");
                assert(myFish === delegateMyFish, "myFish === delegateMyFish");

                // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

                // Lifetime
                logger.comment("Lifetime tests");

                // Set my Fish to null
                logger.comment("Set my Fish to null");
                myFish = null;

                logger.comment("Perform gc");
                CollectGarbage();
                CollectGarbage();

                // verify that mySameFish still is intact?
                logger.comment("Verifying that delegateMyFish still has the expandos");
                verify(delegateMyFish.name, "Nemo", "mySameFish.name");

                logger.comment("*** myDelegate: Exit");

                if (returnFish) {
                    return delegateMyFish;
                }

                return null;
            }

            logger.comment("Call the delegate with the fish : returning null");
            var mySameFish = Animals.Animal.callDelegateWithFish(myDelegate);
            verify(mySameFish, null, "mySameFish");

            logger.comment("Call the delegate with the fish: returning fish");
            myFish = Animals.Animal.myFish;
            returnFish = true;
            var mySameFish = Animals.Animal.callDelegateWithFish(myDelegate);
            verifyMembers("mySameFish", mySameFish, myFishMembers);
            verify(mySameFish.name, "Nemo", "mySameFish.name");

            // Equality tests
            logger.comment("Equality Tests on myFish and mySameFish");
            assert(Animals.Animal.myFish == mySameFish, "Animals.Animal.myFish == mySameFish");
            assert(Animals.Animal.myFish === mySameFish, "Animals.Animal.myFish === mySameFish");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Lifetime
            logger.comment("Lifetime tests");

            // Set my Fish to null
            logger.comment("Set my Fish to null");
            Animals.Animal.myFish = null;

            logger.comment("Perform gc");
            CollectGarbage();
            CollectGarbage();

            // verify that mySameFish still is intact?
            logger.comment("Verifying that mySameFish still has the expandos");
            verify(mySameFish.name, "Nemo", "mySameFish.name");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'DelegateWithSimpleInterface',
        pri: '0',
        test: function () {
            logger.comment("Create myFish");
            var myFish = new Animals.Fish();
            myFish.name = "Nemo";
            myFish.myExpando = "This is myExpando";
            verifyMembers("myFish", myFish, myFishMembers);
            verify(myFish.name, "Nemo", "myFish.name");

            // Pass mySameFish to Method that keeps fish into the ABI
            logger.comment("Pass myFish to Method that keeps fish into the ABI");
            Animals.Animal.myIFish = myFish;

            var returnFish = false;

            function myDelegate(delegateMyFish) {
                logger.comment("*** myDelegate: Invoke");
                verifyMembers("delegateMyFish", delegateMyFish, myFishMembers);
                verify(delegateMyFish.name, "Nemo", "delegateMyFish.name");

                logger.comment("Equality Tests on myFish and delegateMyFish");
                assert(myFish == delegateMyFish, "myFish == delegateMyFish");
                assert(myFish === delegateMyFish, "myFish === delegateMyFish");

                // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

                // Lifetime
                logger.comment("Lifetime tests");

                // Set my Fish to null
                logger.comment("Set my Fish to null");
                myFish = null;

                logger.comment("Perform gc");
                CollectGarbage();
                CollectGarbage();

                // verify that mySameFish still is intact?
                logger.comment("Verifying that delegateMyFish still has the expandos");
                verify(delegateMyFish.name, "Nemo", "mySameFish.name");

                logger.comment("*** myDelegate: Exit");

                if (returnFish) {
                    return delegateMyFish;
                }

                return null;
            }

            logger.comment("Call the delegate with the fish : returning null");
            var mySameFish = Animals.Animal.callDelegateWithIFish(myDelegate);
            verify(mySameFish, null, "mySameFish");

            logger.comment("Call the delegate with the fish: returning fish");
            myFish = Animals.Animal.myIFish;
            returnFish = true;
            var mySameFish = Animals.Animal.callDelegateWithIFish(myDelegate);
            verifyMembers("mySameFish", mySameFish, myFishMembers);
            verify(mySameFish.name, "Nemo", "mySameFish.name");

            // Equality tests
            logger.comment("Equality Tests on myFish and mySameFish");
            assert(Animals.Animal.myIFish == mySameFish, "Animals.Animal.myIFish == mySameFish");
            assert(Animals.Animal.myIFish === mySameFish, "Animals.Animal.myIFish === mySameFish");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Lifetime
            logger.comment("Lifetime tests");

            // Set my Fish to null
            logger.comment("Set my Fish to null");
            Animals.Animal.myFish = null;

            logger.comment("Perform gc");
            CollectGarbage();
            CollectGarbage();

            // verify that mySameFish still is intact?
            logger.comment("Verifying that mySameFish still has the expandos");
            verify(mySameFish.name, "Nemo", "mySameFish.name");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'DelegateWithSimpleRequiredInterface',
        pri: '0',
        test: function () {
            logger.comment("Create myFish");
            var myFish = new Animals.Fish();
            myFish.name = "Nemo";
            myFish.myExpando = "This is myExpando";
            verifyMembers("myFish", myFish, myFishMembers);
            verify(myFish.name, "Nemo", "myFish.name");

            // Pass mySameFish to Method that keeps fish into the ABI
            logger.comment("Pass myFish to Method that keeps fish into the ABI");
            Animals.Animal.myLikeToSwim = myFish;

            var returnFish = false;

            function myDelegate(delegateMyFish) {
                logger.comment("*** myDelegate: Invoke");
                verifyMembers("delegateMyFish", delegateMyFish, myFishMembers);
                verify(delegateMyFish.name, "Nemo", "delegateMyFish.name");

                logger.comment("Equality Tests on myFish and delegateMyFish");
                assert(myFish == delegateMyFish, "myFish == delegateMyFish");
                assert(myFish === delegateMyFish, "myFish === delegateMyFish");

                // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

                // Lifetime
                logger.comment("Lifetime tests");

                // Set my Fish to null
                logger.comment("Set my Fish to null");
                myFish = null;

                logger.comment("Perform gc");
                CollectGarbage();
                CollectGarbage();

                // verify that mySameFish still is intact?
                logger.comment("Verifying that delegateMyFish still has the expandos");
                verify(delegateMyFish.name, "Nemo", "mySameFish.name");

                logger.comment("*** myDelegate: Exit");

                if (returnFish) {
                    return delegateMyFish;
                }

                return null;
            }

            logger.comment("Call the delegate with the fish : returning null");
            var mySameFish = Animals.Animal.callDelegateWithLikeToSwim(myDelegate);
            verify(mySameFish, null, "mySameFish");

            logger.comment("Call the delegate with the fish: returning fish");
            myFish = Animals.Animal.myLikeToSwim;
            returnFish = true;
            var mySameFish = Animals.Animal.callDelegateWithLikeToSwim(myDelegate);
            verifyMembers("mySameFish", mySameFish, myFishMembers);
            verify(mySameFish.name, "Nemo", "mySameFish.name");

            // Equality tests
            logger.comment("Equality Tests on myFish and mySameFish");
            assert(Animals.Animal.myLikeToSwim == mySameFish, "Animals.Animal.myLikeToSwim == mySameFish");
            assert(Animals.Animal.myLikeToSwim === mySameFish, "Animals.Animal.myLikeToSwim === mySameFish");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Lifetime
            logger.comment("Lifetime tests");

            // Set my Fish to null
            logger.comment("Set my Fish to null");
            Animals.Animal.myFish = null;

            logger.comment("Perform gc");
            CollectGarbage();
            CollectGarbage();

            // verify that mySameFish still is intact?
            logger.comment("Verifying that mySameFish still has the expandos");
            verify(mySameFish.name, "Nemo", "mySameFish.name");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'DelegateWithParameterizedInterface',
        pri: '0',
        test: function () {
            logger.comment("Get first vector as myVector");
            var myAnimal = new Animals.Animal(1);
            var myVector = myAnimal.getVector();
            myVector.myExpando = "This is myExpando";
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyMembers("myVector", myVector, myVectorMembers, myArray);

            // Pass myVector to Method that keeps fish into the ABI
            logger.comment("Pass myVector to Method that keeps vector into the ABI");
            myAnimal.myVector = myVector;

            var returnVector = false;

            function myDelegate(delegateVector) {
                logger.comment("*** myDelegate: Invoke");
                verifyMembers("delegateVector", delegateVector, myVectorMembers, myArray);

                logger.comment("Equality Tests on myVector and delegateVector");
                assert(myVector == delegateVector, "myVector == delegateVector");
                assert(myVector === delegateVector, "myVector === delegateVector");

                // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

                // Lifetime
                logger.comment("Lifetime tests");

                // Set my Fish to null
                logger.comment("Set my Vector to null");
                myVector = null;

                logger.comment("Perform gc");
                CollectGarbage();
                CollectGarbage();

                // verify that mySameFish still is intact?
                logger.comment("Verifying that delegateVector still has the expandos");
                verify(delegateVector.length, 9, "delegateVector.length");

                logger.comment("*** myDelegate: Exit");

                if (returnVector) {
                    return delegateVector;
                }

                return null;
            }

            logger.comment("Call the delegate with the vector : returning null");
            var mySameVector = myAnimal.callDelegateWithVector(myDelegate);
            verify(mySameVector, null, "mySameVector");

            logger.comment("Call the delegate with the vector: returning vector");
            myVector = myAnimal.myVector;
            returnVector = true;
            mySameVector = myAnimal.callDelegateWithVector(myDelegate);
            verifyMembers("mySameVector", mySameVector, myVectorMembers, myArray);

            // Equality tests
            logger.comment("Equality tests on myVector and mySameVector");
            assert(myAnimal.myVector == mySameVector, "myAnimal.myVector == mySameVector");
            assert(myAnimal.myVector === mySameVector, "myAnimal.myVector === mySameVector");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Lifetime
            logger.comment("Lifetime tests");

            // Set myVector to null
            logger.comment("Set myVector to null");
            myAnimal.myVector = null;

            logger.comment("Perform gc");
            CollectGarbage();

            // verify that mySameVector still is intact?
            logger.comment("Verifying that mySameVector still has the expandos");
            verify(mySameVector.length, 9, "mySameVector.length");
        }
    });

    runner.addTest({
        id: 10,
        desc: 'DelegateWithParameterizedRequiredInterface',
        pri: '0',
        test: function () {
            logger.comment("Get first vector as myVector");
            var myAnimal = new Animals.Animal(1);
            var myVector = myAnimal.getVector();
            myVector.myExpando = "This is myExpando";
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyMembers("myVector", myVector, myVectorMembers, myArray);

            // Pass myVector to Method that keeps fish into the ABI
            logger.comment("Pass myVector to Method that keeps vector into the ABI");
            myAnimal.myIterable = myVector;

            var returnVector = false;

            function myDelegate(delegateVector) {
                logger.comment("*** myDelegate: Invoke");
                verifyMembers("delegateVector", delegateVector, myVectorMembers, myArray);

                logger.comment("Equality Tests on myVector and delegateVector");
                assert(myVector == delegateVector, "myVector == delegateVector");
                assert(myVector === delegateVector, "myVector === delegateVector");

                // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

                // Lifetime
                logger.comment("Lifetime tests");

                // Set my Fish to null
                logger.comment("Set my Vector to null");
                myVector = null;

                logger.comment("Perform gc");
                CollectGarbage();
                CollectGarbage();

                // verify that mySameFish still is intact?
                logger.comment("Verifying that delegateVector still has the expandos");
                verify(delegateVector.length, 9, "delegateVector.length");

                logger.comment("*** myDelegate: Exit");

                if (returnVector) {
                    return delegateVector;
                }

                return null;
            }

            logger.comment("Call the delegate with the vector : returning null");
            var mySameVector = myAnimal.callDelegateWithIterable(myDelegate);
            verify(mySameVector, null, "mySameVector");

            logger.comment("Call the delegate with the vector: returning vector");
            myVector = myAnimal.myIterable;
            returnVector = true;
            mySameVector = myAnimal.callDelegateWithIterable(myDelegate);
            verifyMembers("mySameVector", mySameVector, myVectorMembers, myArray);

            // Equality tests
            logger.comment("Equality tests on myVector and mySameVector");
            assert(myAnimal.myIterable == mySameVector, "myAnimal.myIterable == mySameVector");
            assert(myAnimal.myIterable === mySameVector, "myAnimal.myIterable === mySameVector");

            // Expando tests -- REMOVED BECAUSE PROJECTED OBJECTS ARE NO LONGER EXTENSIBLE

            // Lifetime
            logger.comment("Lifetime tests");

            // Set myVector to null
            logger.comment("Set myVector to null");
            myAnimal.myIterable = null;

            logger.comment("Perform gc");
            CollectGarbage();

            // verify that mySameVector still is intact?
            logger.comment("Verifying that mySameVector still has the expandos");
            verify(mySameVector.length, 9, "mySameVector.length");
        }
    });

    runner.addTest({
        id: 11,
        desc: 'TestWithDino',
        pri: '0',
        test: function () {
            logger.comment("Create myDino");
            var myDino = new Animals.Dino();
            verifyMembers("myDino", myDino, myDinoMembers);

            logger.comment("Get back same dino");
            Animals.Animal.myDino = myDino;
            var mySameDino = Animals.Animal.myDino;
            verifyMembers("mySameDino", mySameDino, myDinoMembers);

            logger.comment("Equality tests");
            assert(mySameDino == myDino, "mySameDino == myDino");
            assert(mySameDino === myDino, "mySameDino === myDino");

            logger.comment("Set extinct and check the equality and expandos");
            Animals.Animal.myExtinct = myDino;
            mySameDino = Animals.Animal.myExtinct;
            verifyMembers("mySameDino", mySameDino, myDinoMembers);
            assert(mySameDino == myDino, "mySameDino == myDino");
            assert(mySameDino === myDino, "mySameDino === myDino");

            logger.comment("Keep only extinct and null out rest, perform gc");
            mySameDino = null;
            myDino = null;
            Animals.Animal.myDino = null;
            CollectGarbage();

            function myDelegate(delegateExtinct) {
                logger.comment("*** myDelegate : Invoke");

                verifyMembers("delegateExtinct", delegateExtinct, myDinoMembers);
                assert(delegateExtinct == Animals.Animal.myExtinct, "delegateExtinct == Animals.Animal.myExtinct");
                assert(delegateExtinct === Animals.Animal.myExtinct, "delegateExtinct === Animals.Animal.myExtinct");

                logger.comment("*** myDelegate : Exit");
                return null;
            }

            logger.comment("Call the delegate IExtinct");
            Animals.Animal.callDelegateWithExtinct(myDelegate);
        }
    });

    runner.addTest({
        id: 12,
        desc: 'RuntimeClassWithMultipleVectors',
        pri: '0',
        test: function () {
            function VerifyMultipleVector(vectorName, actualVector, myExpectedMembers) {
                verifyMembers(vectorName, actualVector, myExpectedMembers);

                // Methods invokes:
                function forEachIntMethod(x, idx) {
                    verify(actualVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](idx), x, vectorName + '["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
                }
                var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
                myArray.forEach(forEachIntMethod);

                function forEachStringMethod(x, idx) {
                    verify(actualVector["Windows.Foundation.Collections.IVector`1<String>.getAt"](idx), x, vectorName + '["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
                }
                myArray = ["String1", "String2", "String3", "String4"];
                myArray.forEach(forEachStringMethod);

                function forEachAnimalMethod(x, idx) {
                    verify(actualVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt"](idx).getGreeting(), x, vectorName + '["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt"](' + idx + ')GetGreeting()');
                }
                myArray = ["Animal1", "Animal2", "Animal3"];
                myArray.forEach(forEachAnimalMethod);

                // Property invokes
                verify(actualVector["Windows.Foundation.Collections.IVector`1<Int32>.size"], 9, vectorName + '["Windows.Foundation.Collections.IVector`1<Int32>.size"]');
                verify(actualVector["Windows.Foundation.Collections.IVector`1<String>.size"], 4, vectorName + '["Windows.Foundation.Collections.IVector`1<String>.size"]');
                verify(actualVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size"], 3, vectorName + '["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size"]');
            }

            logger.comment("Create MultipleIVector");
            var myMultipleIVector = new Animals.MultipleIVector();
            var myAnimal = new Animals.Animal(1);
            VerifyMultipleVector("myMultipleIVector", myMultipleIVector, myMultipleIVectorMembers);

            var mySameVector = myAnimal.sendBackSameStringVector(myMultipleIVector);

            logger.comment("Equality tests");
            assert(mySameVector == myMultipleIVector, "mySameVector == myMultipleIVector");
            assert(mySameVector === myMultipleIVector, "mySameVector === myMultipleIVector");

            function myDelegate(delegateVector) {
                logger.comment("*** myDelegate: Invoke");
                VerifyMultipleVector("delegateVector", delegateVector, myMultipleIVectorMembers);
                assert(delegateVector == myMultipleIVector, "delegateVector == myMultipleIVector");
                assert(delegateVector === myMultipleIVector, "delegateVector === myMultipleIVector");
                logger.comment("*** myDelegate: Exit");
                return null;
            }

            logger.comment("Set as iterable and call the delegate");
            myAnimal.myIterable = mySameVector;
            myAnimal.callDelegateWithIterable(myDelegate);
        }
    });

    runner.addTest({
        id: 13,
        desc: 'ArraysIntoCollections',
        pri: '0',
        test: function () {
            var myArray = [1, 2, 3, 4, 5];
            myArray.myArrayExpando = "Array Expando"
            var myAnimal = new Animals.Animal(1);

            logger.comment("Get Back array as IVector");
            var myVector = myAnimal.sendBackSameVector(myArray);
            verifyMembers("myVector", myVector, myVectorMembers, myArray);
            verify(myVector.myArrayExpando, undefined, "myVector.myArrayExpando");

            logger.comment("Get back the same Vector as mySameVector");
            var mySameVector = myAnimal.sendBackSameVector(myVector);
            verifyMembers("mySameVector", mySameVector, myVectorMembers, myArray);

            logger.comment("Equality tests");
            assert(mySameVector == myVector, "mySameVector == myVector");
            assert(mySameVector === myVector, "mySameVector === myVector");
            verify(mySameVector.myArrayExpando, undefined, "mySameVector.myArrayExpando");

            logger.comment("Marshal this Vector into Iterable and see if we get same object");
            myAnimal.myIterable = mySameVector;
            verifyMembers("myAnimal.myIterable", myAnimal.myIterable, myVectorMembers, myArray);

            logger.comment("Test equality of myAnimal.myIterable and myVector");
            assert(myAnimal.myIterable == myVector, "myAnimal.myIterable == myVector");
            assert(myAnimal.myIterable === myVector, "myAnimal.myIterable === myVector");
            verify(myAnimal.myIterable.myArrayExpando, undefined, "myAnimal.myIterable.myArrayExpando");

            function myDelegate(deletateVector) {
                logger.comment("*** myDelegate: Invoke");
                verifyMembers("deletateVector", deletateVector, myVectorMembers, myArray);

                logger.comment("Test equality of myAnimal.myIterable and myVector");
                assert(deletateVector == myVector, "deletateVector == myVector");
                assert(deletateVector === myVector, "deletateVector === myVector");
                verify(deletateVector.myArrayExpando, undefined, "deletateVector.myArrayExpando");

                logger.comment("*** myDelegate: Exit");
                return null;
            }

            logger.comment("Call the delegate and see if we can marshal it correctly");
            myAnimal.callDelegateWithIterable(myDelegate);

            logger.comment("Check the view and iterables are not identical");
            var myIterator = myVector.first();
            verifyMembers("myIterator", myIterator, myIteratorMembers);

            logger.comment("Equality and Expando tests on : myIterator and myVector");
            assert(myIterator != myVector, "myIterator != myVector");
            assert(myIterator !== myVector, "myIterator !== myVector");
            verify(myIterator.myArrayExpando, undefined, "myIterator.myArrayExpando");

            var myVectorView = myVector.getView();
            verifyMembers("myVectorView", myVectorView, myVectorViewMembers, myArray);

            logger.comment("Equality and Expando tests on : myVectorView and myVector");
            assert(myVectorView != myVector, "myVectorView != myVector");
            assert(myVectorView !== myVector, "myVectorView !== myVector");
            verify(myVectorView.myArrayExpando, undefined, "myVectorView.myArrayExpando");
        }
    });

    runner.addTest({
        id: 14,
        desc: 'Events',
        pri: '0',
        test: function () {
            logger.comment("Create toaster");
            var msg = "Toast Complete Msg";
            var toaster = new Fabrikam.Kitchen.Toaster();
            verifyMembers("toaster", toaster, myToasterMembers);

            var myToastInDelegate = null;

            function toastCompleteCallback(ev) {
                logger.comment("*** toastCompleteCallback: Enter");
                verify(ev.message, msg, 'ev.message');
                verifyMembers("ev.target", ev.target, myToasterMembers);

                logger.comment("Test equality of ev.target and toaster");
                assert(ev.target == toaster, "ev.target == toaster");
                assert(ev.target === toaster, "ev.target === toaster");

                logger.comment("Store ev.detail[0] - toast and myToastExpando so that we can verify it outside event");
                myToastInDelegate = ev.detail[0];
                ev.detail[0].myToastExpando = "This is myToastExpando"
                verify(ev.detail[0].myToastExpando, undefined, 'ev.detail[0].myToastExpando');
                verifyMembers("ev.detail[0]", ev.detail[0], toastMembers);

                logger.comment("Equality of ev and ev.detail[0]");
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev != ev.detail[0], "ev != ev.detail[0]");
                verify(ev.myToastExpando, undefined, 'ev.myToastExpando');
                verifyMembers("ev", ev, evToastMembers);

                logger.comment("*** toastCompleteCallback: Exit");
            }

            logger.comment("Add event listner");
            toaster.addEventListener("toastcompleteevent", toastCompleteCallback);

            logger.comment("Call makeToast to see event handler is invoked");
            var toast = toaster.makeToast(msg);
            verify(toast.message, msg, 'toast.message');
            verifyMembers("toast", toast, toastMembers);

            logger.comment("Test equality of toasterInDelegate and toaster");
            assert(toast == myToastInDelegate, "toast == myToastInDelegate");
            assert(toast === myToastInDelegate, "toast === myToastInDelegate");

            logger.comment("Get back same toaster");
            var sameToaster = toaster.getSameToaster();
            assert(sameToaster === toaster, "sameToaster == toaster");

            logger.comment("Remove event listner using sameToaster");
            sameToaster.removeEventListener("toastcompleteevent", toastCompleteCallback);

            logger.comment("Call makeToast to see event is actually removed and eventhandler is not invoked");
            var toast = toaster.makeToast(msg);
        }
    });

    runner.addTest({
        id: 15,
        desc: 'EventsOnInterfaceOnlyWinRTObjects',
        pri: '0',
        test: function () {
            logger.comment("Create toaster");
            var toaster = new Fabrikam.Kitchen.Toaster();
            var msg = "This is my toaster msg"
            var switchOnSender;

            function onSwitchOn(ev) {
                logger.comment('*** OnSwitchOn : Invoke');

                verify(ev.target.applianceName, "Toaster", 'ev.target.applianceName');
                verify(ev, msg, "ev");
                verifyMembers("ev.target", ev.target, electricityReporterMembers);

                logger.comment("Test equality of sender and electricityReporterr");
                assert(ev.target == electricityReporter, "ev.target == electricityReporter");
                assert(ev.target === electricityReporter, "ev.target === electricityReporter");

                logger.comment("Test non equality for ev and event details");
                assert(ev == ev.detail[0], "ev == ev.detail[0]");
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                verify(ev.detail[0], msg, "ev.detail[0]");

                logger.comment("Store sender to check in switchoff event");
                switchOnSender = ev.target;

                logger.comment('*** OnSwitchOn : Exit');
            }

            function onSwitchOff(ev) {
                logger.comment('*** OnSwitchOff : Enter');

                verify(ev.target.applianceName, "Toaster", 'ev.target.applianceName');
                verify(ev.detail[0], msg, "ev.detail[0]");
                verify(ev.detail[1], 5, "ev.detail[1]");
                verifyMembers("ev.target", ev.target, electricityReporterMembers);

                logger.comment("Test equality of sender, switchOnSender and electricityReporterr");
                assert(ev.target == electricityReporter, "ev.target == electricityReporter");
                assert(ev.target === electricityReporter, "ev.target === electricityReporter");

                assert(ev.target == switchOnSender, "ev.target == electricityReporter");
                assert(ev.target === switchOnSender, "ev.target === electricityReporter");

                logger.comment("Test non equality for ev and event details");
                assert(ev == ev.detail[0], "ev == ev.detail[0]");
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                verify(ev.detail[0], msg, "ev.detail[0]");

                logger.comment('*** OnSwitchOff : Exit');
            }

            logger.comment("Get Electricityreporter and set the expando");
            var electricityReporter = toaster.electricityReporter;
            verify(electricityReporter.applianceName, 'Toaster', "electricityReporter.applianceName");
            electricityReporter.myExpando = "This is myExpando"
            verifyMembers("electricityReporter", electricityReporter, electricityReporterMembers);

            logger.comment("Registering switch on and off events");
            electricityReporter.addEventListener("applianceswitchonevent", onSwitchOn);
            electricityReporter.addEventListener("applianceswitchoffevent", onSwitchOff);

            logger.comment('Calling toaster.makeToast');
            toaster.makeToast(msg);

            logger.comment("Get back same electricity reporter");
            var sameElectricityReporter = electricityReporter.getSameElectricityReporter();
            assert(sameElectricityReporter === electricityReporter, "sameElectricityReporter == electricityReporter");

            logger.comment("Unregister switch on and off events using sameElectricityReporter");
            sameElectricityReporter.removeEventListener("applianceswitchonevent", onSwitchOn);
            sameElectricityReporter.removeEventListener("applianceswitchoffevent", onSwitchOff);

            logger.comment("Call makeToast to see event is actually removed and eventhandlers are not invoked");
            toaster.makeToast(msg);
        }
    });

    runner.addTest({
        id: 16,
        desc: 'InspectableInCollections',
        pri: '0',
        test: function () {
            logger.comment("Create Fish");
            var myFish = new Animals.Fish();

            logger.comment("Create Dino");
            var myDino = new Animals.Dino();

            logger.comment("Get Vector");
            var myAnimal = new Animals.Animal();
            var myVector = myAnimal.getVector();

            logger.comment("Create MultipleIVector");
            var myMultipleIVector = new Animals.MultipleIVector();

            logger.comment("Add all these objects into array and pass it as IVector and get back as IVector");
            var myArray = [myFish, myDino, myVector, myMultipleIVector, myFish];
            var myInspectableVector = Animals.Animal.sendBackSameInspectableVector(myArray);

            logger.comment("Verify array contents");
            VerifyArrayAndVectorContents("myInspectableVector", myInspectableVector, "myArray", myArray);

            assert(myInspectableVector[0] === myFish, "myInspectableVector[0] === myFish");
            assert(myInspectableVector[1] === myDino, "myInspectableVector[1] === myDino");
            assert(myInspectableVector[2] === myVector, "myInspectableVector[2] === myVector");
            assert(myInspectableVector[3] === myMultipleIVector, "myInspectableVector[3] === myMultipleIVector");
            assert(myInspectableVector[4] === myFish, "myInspectableVector[4] === myFish");
            assert(myInspectableVector[4] === myInspectableVector[0], "myInspectableVector[4] === myInspectableVector[0]");
        }
    });

    Loader42_FileName = 'Object Identity Tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }