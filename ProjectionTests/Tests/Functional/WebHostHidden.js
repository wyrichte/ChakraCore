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

    function dumpObjectMembers(myObjectString, myObject) {
        var objectDump = easyMembersPrint(myObjectString, myObject);

        logger.comment("typeof " + myObjectString + ": " + typeof myObject);
        logger.comment("Dump of properties : " + objectDump);
    }

    function verifyRuntimeClass(actual, expected, expectedType) {
        dumpObjectMembers("myObject", actual);
        verify(typeof actual, expectedType, 'type');
        var i = 0;
        for (p in actual) {
            verify(p, expected[i][0], 'name');
            verify(typeof actual[p], expected[i][1], 'typeof ' + actual + '[' + p + ']');
            i++;
        }

        verify(i, expected.length, 'number of members');
    }


    // name, type, length
    var VisibleClassWithDefaultVisibleInterfaceExpected = [
    ['call_DelegateUsing_HiddenClass_In', 'function', 1],
    ['call_DelegateUsing_HiddenClass_Out', 'function', 1],
    ['call_DelegateUsing_HiddenInterface_In', 'function', 1],
    ['call_DelegateUsing_HiddenInterface_Out', 'function', 1],
    ['call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_In', 'function', 1],
    ['call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out', 'function', 1],
    ['call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In', 'function', 1],
    ['call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out', 'function', 1],
    ['call_HiddenDelegate', 'function', 1],
    ['fillArray_HiddenClass', 'function', 1],
    ['fillArray_HiddenInterface', 'function', 1],
    ['fillArray_VisibleClassWithDefaultHiddenInterface', 'function', 1],
    ['fillArray_VisibleClassWithDefaultVisibleInterface', 'function', 1],
    ['fillArray_VisibleClassWithHiddenInterfaceOnly', 'function', 1],
    ['get_HiddenDelegate', 'function', 0],
    ['hiddenEnum_In', 'function', 1],
    ['hiddenEnum_Out', 'function', 0],
    ['hiddenOverload', 'function', 1],
    ['hiddenStruct_In', 'function', 1],
    ['hiddenStruct_Out', 'function', 0],
    ['methodUsing_HiddenClass_In', 'function', 1],
    ['methodUsing_HiddenClass_Out', 'function', 0],
    ['methodUsing_HiddenInterface_In', 'function', 1],
    ['methodUsing_HiddenInterface_Out', 'function', 0],
    ['methodUsing_VisibleClassWithDefaultHiddenInterface_In', 'function', 1],
    ['methodUsing_VisibleClassWithDefaultHiddenInterface_Out', 'function', 0],
    ['methodUsing_VisibleClassWithDefaultVisibleInterface_In', 'function', 1],
    ['methodUsing_VisibleClassWithDefaultVisibleInterface_Out', 'function', 0],
    ['methodUsing_VisibleClassWithHiddenInterfaceOnly_In', 'function', 1],
    ['methodUsing_VisibleClassWithHiddenInterfaceOnly_Out', 'function', 0],
    ['passArray_HiddenClass', 'function', 1],
    ['passArray_HiddenInterface', 'function', 1],
    ['passArray_VisibleClassWithDefaultHiddenInterface', 'function', 1],
    ['passArray_VisibleClassWithDefaultVisibleInterface', 'function', 1],
    ['passArray_VisibleClassWithHiddenInterfaceOnly', 'function', 1],
    ['property_HiddenClass', 'undefined', 0],
    ['property_HiddenInterface', 'undefined', 0],
    ['property_VisibleClassWithHiddenInterfaceOnly', 'undefined', 0],
    ['property__VisibleClassWithDefaultHiddenInterface', 'undefined', 0],
    ['property__VisibleClassWithDefaultVisibleInterface', 'object', 0],
    ['receiveArray_HiddenClass', 'function', 0],
    ['receiveArray_HiddenInterface', 'function', 0],
    ['receiveArray_VisibleClassWithDefaultHiddenInterface', 'function', 0],
    ['receiveArray_VisibleClassWithDefaultVisibleInterface', 'function', 0],
    ['receiveArray_VisibleClassWithHiddenInterfaceOnly', 'function', 0],
    ['structWithHiddenInnerStruct_In', 'function', 1],
    ['structWithHiddenInnerStruct_Out', 'function', 0],
    ['vector_HiddenClass_In', 'function', 0],
    ['vector_HiddenClass_Out', 'function', 0],
    ['vector_HiddenInterface_In', 'function', 0],
    ['vector_HiddenInterface_Out', 'function', 0],
    ['vector_VisibleClassWithDefaultHiddenInterface_In', 'function', 0],
    ['vector_VisibleClassWithDefaultHiddenInterface_Out', 'function', 0],
    ['vector_VisibleClassWithDefaultVisibleInterface_In', 'function', 0],
    ['vector_VisibleClassWithDefaultVisibleInterface_Out', 'function', 0],
    ['vector_VisibleClassWithHiddenInterfaceOnly_In', 'function', 0],
    ['vector_VisibleClassWithHiddenInterfaceOnly_Out', 'function', 0],
    ['visibleMethod', 'function', 0],
	['toString', 'function', 0],
];

    /// Create the ABIs used throughout this test
    var myVisibleClassWithDefaultVisibleInterface;

    runner.globalSetup(function () {
    });

    runner.addTest({
        id: 1,
        desc: 'Verify hidden class isnt visible',
        pri: '0',
        test: function () {
            verify(Animals.HiddenClass, undefined, "Animals.HiddenClass");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Try to create a class which implements only a hidden interface',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myVisibleClassWithHiddenInterfaceOnly = new Animals.VisibleClassWithHiddenInterfaceOnly();
            } catch (e) {
                verify.instanceOf(e, Error);
                verify(e.message, "Animals.VisibleClassWithHiddenInterfaceOnly: type is not constructible", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught trying to create Animals.VisibleClassWithHiddenInterfaceOnly');

        }
    });

    runner.addTest({
        id: 3,
        desc: 'Try to create a class which implements a hidden interface and visible interface, but the hidden interface is the default interface',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myVisibleClassWithDefaultHiddenInterface = new Animals.VisibleClassWithDefaultHiddenInterface();
            } catch (e) {
                verify.instanceOf(e, Error);
                verify(e.message, "Animals.VisibleClassWithDefaultHiddenInterface: type is not constructible", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught trying to create a Animals.VisibleClassWithDefaultHiddenInterface');

        }
    });

    runner.addTest({
        id: 4,
        desc: 'Try to create a class which implements a hidden interface and visible interface, and the visible interface is the default interface',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface = new Animals.VisibleClassWithDefaultVisibleInterface();
            } catch (e) {
                exceptionCaught = true;
            }

            verify(exceptionCaught, false, 'Able to successfully create Animals.VisibleClassWithDefaultVisibleInterface');

        }
    });

    runner.addTest({
        id: 5,
        desc: 'Verify the members of Animals.VisibleClassWithDefaultVisibleInterface',
        pri: '0',
        test: function () {
            verifyRuntimeClass(myVisibleClassWithDefaultVisibleInterface, VisibleClassWithDefaultVisibleInterfaceExpected, 'object');

        }
    });

    runner.addTest({
        id: 6,
        desc: 'Verify that calling a method that returns an explicitly hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hiddenClass = myVisibleClassWithDefaultVisibleInterface.methodUsing_HiddenClass_Out();
            } catch (e) {
                verify(e.message, "The function 'methodUsing_HiddenClass_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_HiddenClass_Out');

        }
    });

    runner.addTest({
        id: 7,
        desc: 'Verify that calling a method that requires an explicitly hidden class input parameter fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hiddenClass;
                myVisibleClassWithDefaultVisibleInterface.methodUsing_HiddenClass_In(hiddenClass);
            } catch (e) {
                verify(e.message, "The function 'methodUsing_HiddenClass_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_HiddenClass_In');

        }
    });

    runner.addTest({
        id: 8,
        desc: 'Verify that calling a method that returns a hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden = myVisibleClassWithDefaultVisibleInterface.methodUsing_HiddenInterface_Out();
            } catch (e) {
                verify(e.message, "The function 'methodUsing_HiddenInterface_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_HiddenInterface_Out');

        }
    });

    runner.addTest({
        id: 9,
        desc: 'Verify that calling a method that requires a hidden interface input parameter fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden;
                myVisibleClassWithDefaultVisibleInterface.methodUsing_HiddenInterface_In(hidden);
            } catch (e) {
                verify(e.message, "The function 'methodUsing_HiddenInterface_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_HiddenInterface_In');

        }
    });

    runner.addTest({
        id: 10,
        desc: 'Verify that calling a method that returns a class with a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden = myVisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithDefaultHiddenInterface_Out();
            } catch (e) {
                verify(e.message, "The function 'methodUsing_VisibleClassWithDefaultHiddenInterface_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithDefaultHiddenInterface_Out');

        }
    });

    runner.addTest({
        id: 11,
        desc: 'Verify that calling a method that requires a class with a default hidden interface input parameter fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden;
                myVisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithDefaultHiddenInterface_In(hidden);
            } catch (e) {
                verify(e.message, "The function 'methodUsing_VisibleClassWithDefaultHiddenInterface_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithDefaultHiddenInterface_In');

        }
    });

    runner.addTest({
        id: 12,
        desc: 'Verify that calling a method that returns a class implementing a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden = myVisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithHiddenInterfaceOnly_Out();
            } catch (e) {
                verify(e.message, "The function 'methodUsing_VisibleClassWithHiddenInterfaceOnly_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithHiddenInterfaceOnly_Out');

        }
    });

    runner.addTest({
        id: 13,
        desc: 'Verify that calling a method that requires a class implementing a default hidden interface only input parameter fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden;
                myVisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithHiddenInterfaceOnly_In(hidden);
            } catch (e) {
                verify(e.message, "The function 'methodUsing_VisibleClassWithHiddenInterfaceOnly_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithHiddenInterfaceOnly_In');

        }
    });

    runner.addTest({
        id: 14,
        desc: 'Verify that calling a method that returns a class implementing a default visisble interface succeeds',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var visible = myVisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithDefaultVisibleInterface_Out();
            } catch (e) {
                exceptionCaught = true;
            }

            verify(exceptionCaught, false, 'Able to successfully call Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithDefaultVisibleInterface_Out');

        }
    });

    runner.addTest({
        id: 15,
        desc: 'Verify that calling a method that requires a class implementing a default visible interface input parameter succeeds',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var visible = new Animals.VisibleClassWithDefaultVisibleInterface();
                myVisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithDefaultVisibleInterface_In(visible);
            } catch (e) {
                exceptionCaught = true;
            }

            verify(exceptionCaught, false, 'Able to successfully call Animals.VisibleClassWithDefaultVisibleInterface.methodUsing_VisibleClassWithDefaultVisibleInterface_In');

        }
    });

    runner.addTest({
        id: 16,
        desc: 'Verify that getting a property which is an explicit hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden = myVisibleClassWithDefaultVisibleInterface.property_HiddenClass;
            } catch (e) {
                verify(e.message, "The function 'get_Property_HiddenClass' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Unable to successfully get Animals.VisibleClassWithDefaultVisibleInterface.property_HiddenClass');

        }
    });

    runner.addTest({
        id: 17,
        desc: 'Verify that setting a property which is an explicit hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.property_HiddenClass = null;
            } catch (e) {
                verify(e.message, "The function 'put_Property_HiddenClass' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Unable to successfully set Animals.VisibleClassWithDefaultVisibleInterface.property_HiddenClass');

        }
    });

    runner.addTest({
        id: 18,
        desc: 'Verify that getting a property which is an explicit hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden = myVisibleClassWithDefaultVisibleInterface.property_HiddenInterface;
            } catch (e) {
                verify(e.message, "The function 'get_Property_HiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Unable to successfully get Animals.VisibleClassWithDefaultVisibleInterface.property_HiddenInterface');

        }
    });

    runner.addTest({
        id: 19,
        desc: 'Verify that setting a property which is an explicit hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.property_HiddenInterface = null;
            } catch (e) {
                verify(e.message, "The function 'put_Property_HiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Unable to successfully set Animals.VisibleClassWithDefaultVisibleInterface.property_HiddenInterface');

        }
    });

    runner.addTest({
        id: 20,
        desc: 'Verify that getting a property which is a class implementing a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden = myVisibleClassWithDefaultVisibleInterface.property_VisibleClassWithHiddenInterfaceOnly;
            } catch (e) {
                verify(e.message, "The function 'get_Property_VisibleClassWithHiddenInterfaceOnly' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Unable to successfully get Animals.VisibleClassWithDefaultVisibleInterface.property_VisibleClassWithHiddenInterfaceOnly');

        }
    });

    runner.addTest({
        id: 21,
        desc: 'Verify that setting a property which is a class implementing a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.property_VisibleClassWithHiddenInterfaceOnly = null;
            } catch (e) {
                verify(e.message, "The function 'put_Property_VisibleClassWithHiddenInterfaceOnly' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Unable to successfully set Animals.VisibleClassWithDefaultVisibleInterface.property_VisibleClassWithHiddenInterfaceOnly');

        }
    });

    runner.addTest({
        id: 22,
        desc: 'Verify that getting a property which is a class implementing a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var hidden = myVisibleClassWithDefaultVisibleInterface.property__VisibleClassWithDefaultHiddenInterface;
            } catch (e) {
                verify(e.message, "The function 'get_Property__VisibleClassWithDefaultHiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Unable to successfully get Animals.VisibleClassWithDefaultVisibleInterface.property__VisibleClassWithDefaultHiddenInterface');

        }
    });

    runner.addTest({
        id: 23,
        desc: 'Verify that setting a property which is a class implementing a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.property__VisibleClassWithDefaultHiddenInterface = null;
            } catch (e) {
                verify(e.message, "The function 'put_Property__VisibleClassWithDefaultHiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Unable to successfully set Animals.VisibleClassWithDefaultVisibleInterface.property__VisibleClassWithDefaultHiddenInterface');

        }
    });

    runner.addTest({
        id: 24,
        desc: 'Verify that getting a property which is a class implementing a default visible interface succeeds',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var visible = myVisibleClassWithDefaultVisibleInterface.property__VisibleClassWithDefaultVisibleInterface;
            } catch (e) {
                exceptionCaught = true;
            }

            verify(exceptionCaught, false, 'Able to successfully get Animals.VisibleClassWithDefaultVisibleInterface.property__VisibleClassWithDefaultVisibleInterface');

        }
    });

    runner.addTest({
        id: 25,
        desc: 'Verify that setting a property which is a class implementing a default visible interface succeeds',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.property__VisibleClassWithDefaultVisibleInterface = new Animals.VisibleClassWithDefaultVisibleInterface();
            } catch (e) {
                exceptionCaught = true;
            }

            verify(exceptionCaught, false, 'Able to successfully set Animals.VisibleClassWithDefaultVisibleInterface.property__VisibleClassWithDefaultHiddenInterface');

        }
    });

    runner.addTest({
        id: 26,
        desc: 'Verify that passing an array of a hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.passArray_HiddenInterface(0, []);

            } catch (e) {
                verify(e.message, "The function 'passArray_HiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.passArray_HiddenInterface');

        }
    });

    runner.addTest({
        id: 27,
        desc: 'Verify that filling an array of a hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myArray = myVisibleClassWithDefaultVisibleInterface.fillArray_HiddenInterface(2);

            } catch (e) {
                verify(e.message, "The function 'fillArray_HiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.fillArray_HiddenInterface');

        }
    });

    runner.addTest({
        id: 28,
        desc: 'Verify that receiving an array of a hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myReceiveArray = myVisibleClassWithDefaultVisibleInterface.receiveArray_HiddenInterface();

            } catch (e) {
                verify(e.message, "The function 'receiveArray_HiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.receiveArray_HiddenInterface');

        }
    });

    runner.addTest({
        id: 29,
        desc: 'Verify that passing an array of an explicitly hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.passArray_HiddenClass(0, []);

            } catch (e) {
                verify(e.message, "The function 'passArray_HiddenClass' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.passArray_HiddenClass');

        }
    });

    runner.addTest({
        id: 30,
        desc: 'Verify that filling an array of an explicitly hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myArray = myVisibleClassWithDefaultVisibleInterface.fillArray_HiddenClass(2);

            } catch (e) {
                verify(e.message, "The function 'fillArray_HiddenClass' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.fillArray_HiddenClass');

        }
    });

    runner.addTest({
        id: 31,
        desc: 'Verify that receiving an array of an explicitly hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myReceiveArray = myVisibleClassWithDefaultVisibleInterface.receiveArray_HiddenClass();

            } catch (e) {
                verify(e.message, "The function 'receiveArray_HiddenClass' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.receiveArray_HiddenClass');

        }
    });

    runner.addTest({
        id: 32,
        desc: 'Verify that passing an array of a class implementing a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.passArray_VisibleClassWithHiddenInterfaceOnly(0, []);

            } catch (e) {
                verify(e.message, "The function 'passArray_VisibleClassWithHiddenInterfaceOnly' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.passArray_VisibleClassWithHiddenInterfaceOnly');

        }
    });

    runner.addTest({
        id: 33,
        desc: 'Verify that filling an array of a class implementing a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myArray = myVisibleClassWithDefaultVisibleInterface.fillArray_VisibleClassWithHiddenInterfaceOnly(2);

            } catch (e) {
                verify(e.message, "The function 'fillArray_VisibleClassWithHiddenInterfaceOnly' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.fillArray_VisibleClassWithHiddenInterfaceOnly');

        }
    });

    runner.addTest({
        id: 34,
        desc: 'Verify that receiving an array of a class implementing a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myReceiveArray = myVisibleClassWithDefaultVisibleInterface.receiveArray_VisibleClassWithHiddenInterfaceOnly();

            } catch (e) {
                verify(e.message, "The function 'receiveArray_VisibleClassWithHiddenInterfaceOnly' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.receiveArray_VisibleClassWithHiddenInterfaceOnly');

        }
    });

    runner.addTest({
        id: 35,
        desc: 'Verify that passing an array of a class with a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.passArray_VisibleClassWithDefaultHiddenInterface(0, []);

            } catch (e) {
                verify(e.message, "The function 'passArray_VisibleClassWithDefaultHiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.passArray_VisibleClassWithDefaultHiddenInterface');

        }
    });

    runner.addTest({
        id: 36,
        desc: 'Verify that filling an array of a class with a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myArray = myVisibleClassWithDefaultVisibleInterface.fillArray_VisibleClassWithDefaultHiddenInterface(2);

            } catch (e) {
                verify(e.message, "The function 'fillArray_VisibleClassWithDefaultHiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.fillArray_VisibleClassWithDefaultHiddenInterface');

        }
    });

    runner.addTest({
        id: 37,
        desc: 'Verify that receiving an array of a class with a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myReceiveArray = myVisibleClassWithDefaultVisibleInterface.receiveArray_VisibleClassWithDefaultHiddenInterface();

            } catch (e) {
                verify(e.message, "The function 'receiveArray_VisibleClassWithDefaultHiddenInterface' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.receiveArray_VisibleClassWithDefaultHiddenInterface');

        }
    });



    runner.addTest({
        id: 38,
        desc: 'Verify that receiving an array of a class with a default hidden interface succeeds',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myReceiveArray = myVisibleClassWithDefaultVisibleInterface.receiveArray_VisibleClassWithDefaultVisibleInterface();

            } catch (e) {
                exceptionCaught = true;
            }

            verify(exceptionCaught, false, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.receiveArray_VisibleClassWithDefaultVisibleInterface');

        }
    });

    runner.addTest({
        id: 39,
        desc: 'Verify that calling a method that requires an IVector of an explicitly hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var vector;
                myVisibleClassWithDefaultVisibleInterface.vector_HiddenClass_In(vector);
            } catch (e) {
                verify(e.message, "The function 'vector_HiddenClass_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.vector_HiddenClass_In');

        }
    });

    runner.addTest({
        id: 40,
        desc: 'Verify that calling a method that returns an IVector of an explicitly hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var vector = myVisibleClassWithDefaultVisibleInterface.vector_HiddenClass_Out();
            } catch (e) {
                verify(e.message, "The function 'vector_HiddenClass_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.vector_HiddenClass_In');

        }
    });

    runner.addTest({
        id: 41,
        desc: 'Verify that calling a method that requires an IVector of a hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var vector;
                myVisibleClassWithDefaultVisibleInterface.vector_HiddenInterface_In(vector);
            } catch (e) {
                verify(e.message, "The function 'vector_HiddenInterface_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.vector_HiddenInterface_In');

        }
    });

    runner.addTest({
        id: 42,
        desc: 'Verify that calling a method that returns an IVector of a hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var vector = myVisibleClassWithDefaultVisibleInterface.vector_HiddenInterface_Out();
            } catch (e) {
                verify(e.message, "The function 'vector_HiddenInterface_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.vector_HiddenInterface_Out');

        }
    });

    runner.addTest({
        id: 43,
        desc: 'Verify that calling a method that requires an IVector of a class with a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var vector;
                myVisibleClassWithDefaultVisibleInterface.vector_VisibleClassWithDefaultHiddenInterface_In(vector);
            } catch (e) {
                verify(e.message, "The function 'vector_VisibleClassWithDefaultHiddenInterface_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.vector_VisibleClassWithDefaultHiddenInterface_In');

        }
    });

    runner.addTest({
        id: 44,
        desc: 'Verify that calling a method that returns an IVector of a class with a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var vector = myVisibleClassWithDefaultVisibleInterface.vector_VisibleClassWithDefaultHiddenInterface_Out(vector);
            } catch (e) {
                verify(e.message, "The function 'vector_VisibleClassWithDefaultHiddenInterface_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.vector_VisibleClassWithDefaultHiddenInterface_Out');

        }
    });

    runner.addTest({
        id: 45,
        desc: 'Verify that calling a method that requires an IVector of a class implementing a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var vector;
                myVisibleClassWithDefaultVisibleInterface.vector_VisibleClassWithHiddenInterfaceOnly_In(vector);
            } catch (e) {
                verify(e.message, "The function 'vector_VisibleClassWithHiddenInterfaceOnly_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.vector_VisibleClassWithHiddenInterfaceOnly_In');

        }
    });

    runner.addTest({
        id: 46,
        desc: 'Verify that calling a method that returns an IVector of a class implementing a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var vector = myVisibleClassWithDefaultVisibleInterface.vector_VisibleClassWithHiddenInterfaceOnly_Out();
            } catch (e) {
                verify(e.message, "The function 'vector_VisibleClassWithHiddenInterfaceOnly_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.vector_VisibleClassWithHiddenInterfaceOnly_Out');

        }
    });

    runner.addTest({
        id: 47,
        desc: 'Verify that calling a method that returns a hidden struct fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var struct = myVisibleClassWithDefaultVisibleInterface.hiddenStruct_Out();
            } catch (e) {
                verify(e.message, "The function 'hiddenStruct_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.hiddenStruct_Out');

        }
    });


    runner.addTest({
        id: 48,
        desc: 'Verify that calling a method that requires a hidden struct fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var struct;
                myVisibleClassWithDefaultVisibleInterface.hiddenStruct_In(struct);
            } catch (e) {
                verify(e.message, "The function 'hiddenStruct_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.hiddenStruct_In');

        }
    });

    runner.addTest({
        id: 49,
        desc: 'Verify that calling a method that returns a struct with a hidden field fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var struct = myVisibleClassWithDefaultVisibleInterface.structWithHiddenInnerStruct_Out();
            } catch (e) {
                verify(e.message, "The function 'structWithHiddenInnerStruct_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.structWithHiddenInnerStruct_Out');

        }
    });


    runner.addTest({
        id: 50,
        desc: 'Verify that calling a method that requires a struct with a hidden field fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var struct;
                myVisibleClassWithDefaultVisibleInterface.structWithHiddenInnerStruct_In(struct);
            } catch (e) {
                verify(e.message, "The function 'structWithHiddenInnerStruct_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.structWithHiddenInnerStruct_In');

        }
    });

    runner.addTest({
        id: 51,
        desc: 'Verify that calling a method returning a hidden delegate fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var delegate = myVisibleClassWithDefaultVisibleInterface.get_HiddenDelegate();
            } catch (e) {
                verify(e.message, "The function 'get_HiddenDelegate' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.get_HiddenDelegate');

        }
    });

    runner.addTest({
        id: 52,
        desc: 'Verify that calling a method requiring a hidden delegate fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var delegateCalled = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.call_HiddenDelegate(function (x) { delegateCalled = true; });
            } catch (e) {
                verify(e.message, "The function 'call_HiddenDelegate' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.call_HiddenDelegate');
            verify(delegateCalled, false, 'A Hidden delegate was verified to not have been called');

        }
    });

    runner.addTest({
        id: 53,
        desc: 'Verify that calling a delegate requiring a hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var delegateCalled = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.call_DelegateUsing_HiddenClass_In(function (x) { delegateCalled = true; });
            } catch (e) {
                verify(e.message, "The function 'call_DelegateUsing_HiddenClass_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.call_DelegateUsing_HiddenClass_In');
            verify(delegateCalled, false, 'A delegate using a hidden class was verified to not have been called');
        }
    });

    runner.addTest({
        id: 54,
        desc: 'Verify that method that takes delegate returning a hidden class fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var delegateCalled = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.call_DelegateUsing_HiddenClass_Out(function () { delegateCalled = true; return null; });
            } catch (e) {
                verify(e.message, "The function 'call_DelegateUsing_HiddenClass_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.call_DelegateUsing_HiddenClass_Out');
            verify(delegateCalled, false, 'A delegate returning a hidden class was verified to have been called');

        }
    });

    runner.addTest({
        id: 55,
        desc: 'Verify that calling a delegate requiring a hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var delegateCalled = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.call_DelegateUsing_HiddenInterface_In(function (x) { delegateCalled = true; });
            } catch (e) {
                verify(e.message, "The function 'call_DelegateUsing_HiddenInterface_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.call_DelegateUsing_HiddenInterface_In');
            verify(delegateCalled, false, 'A delegate using a hidden interface was verified to not have been called');
        }
    });

    runner.addTest({
        id: 56,
        desc: 'Verify that calling a delegate returning a hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var delegateCalled = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.call_DelegateUsing_HiddenInterface_Out(function () { delegateCalled = true; return null; });
            } catch (e) {
                verify(e.message, "The function 'call_DelegateUsing_HiddenInterface_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.call_DelegateUsing_HiddenInterface_In');
            verify(delegateCalled, false, 'A delegate returning a hidden interface was verified to have been called');

        }
    });


    runner.addTest({
        id: 57,
        desc: 'Verify that calling a delegate requiring a class with a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var delegateCalled = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_In(function (x) { delegateCalled = true; });
            } catch (e) {
                verify(e.message, "The function 'call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_In');
            verify(delegateCalled, false, 'A delegate using a class with a default hidden interface was verified to not have been called');

        }
    });

    runner.addTest({
        id: 58,
        desc: 'Verify that calling a delegate returning a class with a default hidden interface fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var delegateCalled = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out(function () { delegateCalled = true; return null; });
            } catch (e) {
                verify(e.message, "The function 'call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out');
            verify(delegateCalled, false, 'A delegate returning a class with a default hidden interface was verified to have been called');

        }
    });

    runner.addTest({
        id: 59,
        desc: 'Verify that calling a delegate requiring a class with a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var delegateCalled = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In(function (x) { delegateCalled = true; });
            } catch (e) {
                verify(e.message, "The function 'call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In');
            verify(delegateCalled, false, 'A delegate using a class with a hidden interface only was verified to not have been called');

        }
    });

    runner.addTest({
        id: 60,
        desc: 'Verify that calling a delegate returning a class with a hidden interface only fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var delegateCalled = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out(function () { delegateCalled = true; return null; });
            } catch (e) {
                verify(e.message, "The function 'call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out');
            verify(delegateCalled, false, 'A delegate returning a class with a hidden interface only was verified to have been called');

        }
    });

    runner.addTest({
        id: 61,
        desc: 'Verify that calling a method requiring a hidden enum fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                myVisibleClassWithDefaultVisibleInterface.hiddenEnum_In(0);
            } catch (e) {
                verify(e.message, "The function 'hiddenEnum_In' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.hiddenEnum_In');
        }
    });

    runner.addTest({
        id: 62,
        desc: 'Verify that calling a method returning a hidden enum fails',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myEnum = myVisibleClassWithDefaultVisibleInterface.hiddenEnum_Out();
            } catch (e) {
                verify(e.message, "The function 'hiddenEnum_Out' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.hiddenEnum_Out');
        }
    });

    runner.addTest({
        id: 63,
        desc: 'Verify that hidden enums are not defined',
        pri: '0',
        test: function () {
            verify(Animals.HiddenEnum, undefined, 'Animals.HiddenEnum');
        }
    });

    runner.addTest({
        id: 64,
        desc: 'Verify that hidden enums are not defined',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myEnum = Animals.HiddenEnum.first;
            } catch (e) {
                verify(e.message, "Unable to get property 'first' of undefined or null reference");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.HiddenEnum');
        }
    });

    runner.addTest({
        id: 65,
        desc: 'Verify that an overloaded method using a hidden param type is not callable',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            try {
                myVisibleClassWithDefaultVisibleInterface.hiddenOverload();
            } catch (e) {
                verify(e.message, "The function 'hiddenOverload' has an invalid signature and cannot be called", "Exception message");
                exceptionCaught = true;
            }

            verify(exceptionCaught, true, 'Exception caught from Animals.VisibleClassWithDefaultVisibleInterface.hiddenOverload');
        }
    });

    runner.addTest({
        id: 66,
        desc: 'Verify that an overloaded method using visible types is callable',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var value = 0;

            try {
                value = myVisibleClassWithDefaultVisibleInterface.hiddenOverload(5);
            } catch (e) {
                exceptionCaught = true;
            }

            verify(value, 5, 'Called Animals.VisibleClassWithDefaultVisibleInterface.hiddenOverload(5) successfully');
            verify(exceptionCaught, false, 'Exception not caught from Animals.VisibleClassWithDefaultVisibleInterface.hiddenOverload(5)');
        }
    });

    runner.addTest({
        id: 66,
        desc: 'Verify that an static hidden interface methods and properties are hidden',
        pri: '0',
        preReq: function () {
            // CLR don't support static interface
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            verifyRuntimeClass(Animals.VisibleClassWithVisibleInterfaceAndHiddenStaticInterface, [], "function");
            verifyRuntimeClass(new Animals.VisibleClassWithVisibleInterfaceAndHiddenStaticInterface(), VisibleClassWithDefaultVisibleInterfaceExpected, "object");
        }
    });

    Loader42_FileName = 'WebHostHidden tests';

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
