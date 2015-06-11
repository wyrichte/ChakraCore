if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    function isNumber(o) { return !isNaN(o - 0); }

    function verifyRuntimeObjectAgainstLanguageService(runtimeObject, languageServiceObject, level) {
        if (!level) level = 0;
        if (runtimeObject || languageServiceObject) {
            var runtimeObjectPropertyNames = Object.getOwnPropertyNames(runtimeObject)
            var languageServicePropertyNames = Object.getOwnPropertyNames(languageServiceObject)

            // Loop the language service prototype members and see that each exists
            for (i in languageServicePropertyNames) {
                var propertyName = languageServicePropertyNames[i];
                if (propertyName != "_type" && propertyName != "length") {
                    var languageServiceProperty = Object.getOwnPropertyDescriptor(languageServiceObject, propertyName);
                    if (!languageServiceProperty) {
                        fail("L+" + level + ":Expected language service object to have a property named " + propertyName);
                    }
                    var runtimeObjectProperty = Object.getOwnPropertyDescriptor(runtimeObject, propertyName);
                    if (!runtimeObjectProperty) {
                        fail("L+" + level + ":Expected runtime object to have a property named " + propertyName);
                    }
                }
            }

            // Loop the runtime object prototype members and see that each exists
            for (i in runtimeObjectPropertyNames) {
                var propertyName = runtimeObjectPropertyNames[i];
                if (!isNumber(propertyName)) { // Skip integer valued properties for the array case.
                    var languageServiceProperty = Object.getOwnPropertyDescriptor(languageServiceObject, propertyName);
                    if (!languageServiceProperty) {
                        fail("R+" + level + ":Expected language service object to have a property named " + propertyName);
                    }
                    var runtimeObjectProperty = Object.getOwnPropertyDescriptor(runtimeObject, propertyName);
                    if (!runtimeObjectProperty) {
                        fail("R+" + level + ":Expected runtime object to have a property named " + propertyName);
                    }
                }
            }

            verifyRuntimeObjectAgainstLanguageService(Object.getPrototypeOf(runtimeObject), Object.getPrototypeOf(languageServiceObject), level + 1);
        }
    }

    function walkUp(obj, level) {
        if (obj) {
            if (!level) level = 0;
            var names = Object.getOwnPropertyNames(obj);
            for (i in names) {
                logger.comment(level + " : " + names[i]);
            }
            walkUp(Object.getPrototypeOf(obj), level + 1);
        }
    }


    runner.addTest({
        id: 1,
        desc: 'Enum',
        pri: '0',

        test: function () {
            verifyRuntimeObjectAgainstLanguageService(Animals.Phylum, reflect.Animals.Phylum);
        }

    });

    runner.addTest({
        id: 2,
        desc: 'Array',
        pri: '0',

        test: function () {
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            var realAnimal = new Animals.Animal(1);
            var realVector = realAnimal.passArray(myArray);
            var reflectAnimal = new reflect.Animals.Animal(1);
            verifyRuntimeObjectAgainstLanguageService(realAnimal, reflectAnimal);
            var reflectVector = reflectAnimal.passArray(myArray);
            verifyRuntimeObjectAgainstLanguageService(realVector, reflectVector);
        }
    });

// Broken by FI -- 10/26/2011
//    runner.addTest({
//        id: 3,
//        desc: 'Runtime Class Prototype',
//        pri: '0',

//        test: function () {
//            var realPrototype = Windows.ApplicationModel.DataTransfer.DataPackage.prototype;
//            var reflectPrototype = reflect.Windows.ApplicationModel.DataTransfer.DataPackage.prototype;
//            verifyRuntimeObjectAgainstLanguageService(realPrototype, reflectPrototype);
//        }
//    });

    runner.addTest({
        id: 4,
        desc: 'Struct',
        pri: '0',

        test: function () {
            var realAnimal = new Animals.Animal(1);
            var reflectAnimal = new reflect.Animals.Animal(1);
            var realStruct = realAnimal.getOuterStruct();
            var reflectStruct = reflectAnimal.getOuterStruct();
            verifyRuntimeObjectAgainstLanguageService(realStruct, reflectStruct);
        }
    });

    runner.addTest({
        id: '369696',
        desc: 'Multi-out',
        pri: '0',

        test: function () {
            var reflectAnimal = new reflect.Animals.Animal(1);
            var reflectResult = reflectAnimal.doubleOffset2Int(1, 2, 3);
            verify(typeof reflectResult.reta, 'number', 'Expected reta');
            verify(typeof reflectResult.retb, 'number', 'Expected retb');
            verify(typeof reflectResult.retc, 'number', 'Expected retc');
        }
    });

    runner.addTest({
        id: 'promise-1',
        desc: 'Basic async',
        pri: '0',
        test: function () {
            var realWinery = new Winery.RWinery(1);
            var realAsync = realWinery.asyncOperationOut();
            var reflectWinery = new reflect.Winery.RWinery(1);
            var reflectAsync = reflectWinery.asyncOperationOut();
            verifyRuntimeObjectAgainstLanguageService(realAsync, reflectAsync);
        }
    });

    runner.addTest({
        id: 'promise-2',
        desc: 'CompleteSuccessfully',
        pri: '0',
        test: function () {
            // real
            var realWinery = new Winery.RWinery(1);
            var realCompleteCalled = false;
            var realResultWas = 0;
            var realPromise = realWinery.asyncOperationOut();
            realPromise
            .then(function (result) {
                realCompleteCalled = true;
                realResultWas = result;
            });
            realPromise.operation.moveToCompleted(192); // Simulate asynchronous completion

            // reflect
            var reflectWinery = new reflect.Winery.RWinery(1);
            var reflectCompleteCalled = false;
            var reflectResultWas = 0;
            var reflectPromise = reflectWinery.asyncOperationOut();
            reflectPromise
            .then(function (result) {
                reflectCompleteCalled = true;
                reflectResultWas = result;
            });
            reflectPromise.operation.moveToCompleted(192); // Simulate asynchronous completion
            if (!reflectResultWas) {
                throw "Expected a reflect result";
            }
            verify(typeof reflectResultWas, "object", "Result type of then");
            verify("number", typeof realResultWas, "Result type of then");
        }
    });

    runner.addTest({
        id: 'promise-3',
        desc: 'CompleteWithCancel',
        pri: '0',
        test: function () {
            // real
            var realWinery = new Winery.RWinery(1);
            var realCompleteCalled = false;
            var realErrorCalled = false;
            var realErrorResult = null;
            var realPromise = realWinery.asyncOperationOut();
            realPromise
            .then(function (result) { realCompleteCalled = true; },
                function (error) {
                    realErrorCalled = true;
                    realErrorResult = error;
                }
            );
            realPromise.operation.cancel(); // Simulate asynchronous cancel

            // reflect
            var reflectWinery = new reflect.Winery.RWinery(1);
            var reflectCompleteCalled = false;
            var reflectErrorCalled = false;
            var reflectErrorResult = null;
            var reflectAsyncOperationOut = reflectWinery.asyncOperationOut();
            var reflectPromise = reflectAsyncOperationOut;
            reflectPromise
            .then(function (result) { reflectCompleteCalled = true; },
                function (error) {
                    reflectErrorCalled = true;
                    reflectErrorResult = error;
                }
            );
            reflectPromise.operation.cancel(); // Simulate asynchronous cancel
            verify(typeof reflectErrorResult, typeof realErrorResult, "Result type of error");
        }
    });

    runner.addTest({
        id: 'promise-5',
        desc: 'TwoThensFollowedByCompleted',
        pri: '0',
        test: function () {
            var winery = new reflect.Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) {
                completeCalled = completeCalled + "first ";
            })
            .then(function (result) {
                completeCalled = completeCalled + "second";
            });

            if (completeCalled != "first second") {
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: 'promise-12',
        desc: 'ThenCancelProgress',
        pri: '0',
        test: function () {
            var winery = new reflect.Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var progressCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .then(
                    function (result) { completeCalled = completeCalled + "onComplete(" + result + ")"; },
                    function (result) { errorCalled = errorCalled + "onError(" + result + ")"; },
                    function (percent) { progressCalled = progressCalled + "onProgress(" + percent + ")"; }
                );

            if (completeCalled != "onComplete([object Object])") {
                logger.comment(completeCalled);
                fail("expected a call to on onComplete");
            }
            if (errorCalled != "onError(Error)") {
                logger.comment(errorCalled);
                fail("expected onError be called");
            }
            if (progressCalled != "onProgress(100)") {
                logger.comment(progressCalled);
                fail("expected onProgress to be called");
            }
        }
    });


    runner.addTest({
        id: 142,
        desc: 'IntelliDoc',
        pri: '0',

        test: function () {
            verify(reflect.Windows.Foundation.Collections["IMap`2"].prototype._type.clear.signatures[0].externalid, "M:Windows.Foundation.Collections.IMap`2.Clear", "Method of generic interface");
            verify(reflect.Windows.Foundation.Collections["IMap`2"].prototype._type.clear.signatures[0].externalFile, "Windows.xml", "Method of generic interface (xml)");
            verify(reflect.Windows.Foundation.Collections["IMap`2"].prototype._type.clear.signatures[0].helpKeyword, "Windows.Foundation.Collections.IMap`2.Clear", "Method of generic interface (help)");
            verify(reflect.Windows.Foundation.Collections["IMap`2"].prototype._type.hasKey.signatures[0].externalid, "M:Windows.Foundation.Collections.IMap`2.HasKey(`0)", "Method of generic interface");
            verify(reflect.Windows.Foundation.Collections["IMap`2"].prototype._type.hasKey.signatures[0].externalFile, "Windows.xml", "Method of generic interface (xml)");
            verify(reflect.Windows.Foundation.Collections["IMap`2"].prototype._type.hasKey.signatures[0].helpKeyword, "Windows.Foundation.Collections.IMap`2.HasKey", "Method of generic interface (help)");
            verify(reflect.Windows.Foundation.Collections._type.PropertySet.signatures[0].externalid, "M:Windows.Foundation.Collections.PropertySet.#ctor", "Parameterless constructor");
            verify(reflect.Windows.Foundation.Collections._type.PropertySet.signatures[0].externalFile, "Windows.xml", "Parameterless constructor (xml)");
            verify(reflect.Windows.Foundation.Collections._type.PropertySet.signatures[0].helpKeyword, "Windows.Foundation.Collections.PropertySet", "Parameterless constructor (help)");
            verify(reflect.Windows.Foundation.Collections["IMap`2"].prototype._type.size.externalid, "P:Windows.Foundation.Collections.IMap`2.Size", "Property of generic interface");
            verify(reflect.Windows.Foundation.Collections["IMap`2"].prototype._type.size.externalFile, "Windows.xml", "Property of generic interface (xml)");
            verify(reflect.Windows.Foundation.Collections["IMap`2"].prototype._type.size.helpKeyword, "Windows.Foundation.Collections.IMap`2.Size", "Property of generic interface (help)");
            verify(reflect.Windows.Foundation.Collections["IObservableMap`2"].prototype._type.onmapchanged.externalid, "E:Windows.Foundation.Collections.IObservableMap`2.MapChanged", "Event of generic interface");
            verify(reflect.Windows.Foundation.Collections["IObservableMap`2"].prototype._type.onmapchanged.externalFile, "Windows.xml", "Event of generic interface (xml)");
            verify(reflect.Windows.Foundation.Collections["IObservableMap`2"].prototype._type.onmapchanged.helpKeyword, "Windows.Foundation.Collections.IObservableMap`2.MapChanged", "Event of generic interface (help)");
            verify(reflect.Animals._type.BigComplexStruct.signatures[0].externalid, "T:Animals.BigComplexStruct", "Struct");
            verify(reflect.Animals._type.BigComplexStruct.signatures[0].externalFile, "animals.xml", "Struct (xml)");
            verify(reflect.Animals._type.BigComplexStruct.signatures[0].helpKeyword, "Animals.BigComplexStruct", "Struct (help)");
            // Broken by FI -- 10/26/2011
            //            verify(reflect.Windows.ApplicationModel.Resources.Core.IResourceCandidate.prototype._type.toFile.signatures[0].returnTypeExternalFile, "Windows.Storage.xml");
        }
    });

    runner.addTest({
        id: '354819-1',
        desc: 'addEventListener is called',
        pri: '0',
        test: function () {
            // Reflect version
            var reflectToaster = new reflect.Fabrikam.Kitchen.Toaster();
            var reflectListenerCalled = "";
            var reflectp0 = null;
            reflectToaster.addEventListener("toastcompleteevent", function (a, b, c) { reflectp0 = a; reflectListenerCalled = reflectListenerCalled + "toastcompleteevent(" + a + "," + b + "," + c + ")" });
            reflectToaster.makeToast("123");

            // Real version
            var realToaster = new Fabrikam.Kitchen.Toaster();
            var realListenerCalled = "";
            var realp0 = null;
            realToaster.addEventListener("toastcompleteevent", function (a, b, c) { realp0 = a; realListenerCalled = realListenerCalled + "toastcompleteevent(" + a + "," + b + "," + c + ")" });
            realToaster.makeToast("123");

            verify(typeof reflectp0.type == 'string', true, "Expected reflectToaster param to have 'type'");
            verify(!!reflectp0.target, true, "Expected reflectToaster param to have 'target'");
            verify(typeof reflectp0.message == 'string', true, "Expected reflectToaster param to have 'message'");
            verify(Array.isArray(realp0.detail), true, "Expected realToaster param .detail to be an array"); // If not, need to change jsgen to match
            verify(Array.isArray(reflectp0.detail), true, "Expected reflectToaster param .detail to be an array");
            verify(typeof reflectp0.target['makeToast'] == 'function', true, "Expected target to be an IToaster");
            verify(typeof reflectp0.type == 'string', true, "Expected type to be a string");
            verify(typeof reflectp0.detail[0].message == 'string', true, "Expected reflectp0.detail[0] to have 'message'");
            verify(!!reflectp0.detail[0].target, false, "Expected reflectp0.detail[0] to NOT have 'target'");
            verify(!!reflectp0.detail[0].detail, false, "Expected reflectp0.detail[0] to NOT have 'detail'");
            verify(!!reflectp0.detail[0].type, false, "Expected reflectp0.detail[0] to NOT have 'type'");
        }
    });

    runner.addTest({
        id: '354819-2',
        desc: 'ontoaststartevent is called',
        pri: '0',
        test: function () {
            // Reflect version
            var reflectToaster = new reflect.Fabrikam.Kitchen.Toaster();
            var reflectListenerCalled = "";
            var reflectp0 = null;
            reflectToaster.ontoaststartevent = (function (a, b, c) { reflectp0 = a; reflectListenerCalled = reflectListenerCalled + "ontoaststartevent(" + a + "," + b + "," + c + ")" });
            reflectToaster.makeToast("123");

            // Real version
            var realToaster = new Fabrikam.Kitchen.Toaster();
            var realListenerCalled = "";
            var realp0 = null;
            realToaster.ontoaststartevent = (function (a, b, c) { realp0 = a; realListenerCalled = realListenerCalled + "ontoaststartevent(" + a + "," + b + "," + c + ")" });
            realToaster.makeToast("123");

            logger.comment(reflectListenerCalled);
            for (p in reflectp0) logger.comment('--' + p + ":" + reflectp0[p]);
            logger.comment(reflectListenerCalled);
            for (p in realp0) logger.comment('--' + p + ":" + realp0[p]);

            verify(typeof reflectp0.type == 'string', true, "Expected reflectToaster param to have 'type'");
            verify(!!reflectp0.target, true, "Expected reflectToaster param to have 'target'");
            verify(Array.isArray(realp0.detail), true, "Expected realToaster param .detail to be an array"); // If not, need to change jsgen to match
            verify(Array.isArray(reflectp0.detail), true, "Expected reflectToaster param .detail to be an array");
            verify(typeof reflectp0.target['makeToast'] == 'function', true, "Expected target to be an IToaster");
            verify(typeof reflectp0.type == 'string', true, "Expected type to be a string");
        }
    });

    // Broken by FI -- 10/26/2011
//    runner.addTest({
//        id: '399763',
//        desc: 'Generic parameter should not be "string"',
//        pri: '0',
//        test: function () {
//            var result = "value to be replaced";
//            var picker = new reflect.Windows.Storage.Pickers.FileOpenPicker();
//            picker.fileTypeFilter.replaceAll([".png", ".jpg"]);
//            picker.pickMultipleFilesAsync().then(function (files) {
//                result = files[0];
//            });
//            verify(typeof result != 'string', true, "Type should not be string");
//            verify(typeof result == 'object', true, "Type should be object");
//            // Verify the type is consistant with Windows.Storage.StorageFile
//            var realPrototype = Windows.Storage.StorageFile.prototype;
//            var reflectPrototype = Object.getPrototypeOf(result);
//            verifyRuntimeObjectAgainstLanguageService(realPrototype, reflectPrototype);
//        }
//    });

    runner.addTest({
        id: '450740',
        desc: 'Max enum',
        pri: '0',
        test: function () {
            var realValue = Windows.Foundation.Metadata.AttributeTargets.all;
            var lsValue = reflect.Windows.Foundation.Metadata.AttributeTargets.all;
            logger.comment(realValue);
            logger.comment(lsValue);
            verify(realValue == lsValue, true, "Values should be the same");
        }
    });

    runner.addTest({
        id: '432654-1',
        desc: 'IVector of varying types',
        pri: '0',
        test: function () {
            verify(reflect.Windows.Foundation.Collections['IVector`1'].length, 1, "reflect.Windows.Foundation.Collections['IVector`1'].length");
            var reflectPVTests = new reflect.Animals.PropertyValueTests();
            var realPVTests = new Animals.PropertyValueTests();

            logger.comment("IVector<Dimensions>");
            var reflectVector = reflectPVTests.receiveVectorOfStruct();
            verify.instanceOf(reflectVector, Array);
            var realVector = realPVTests.receiveVectorOfStruct();
            verifyRuntimeObjectAgainstLanguageService(realVector, reflectVector);
            var reflectItem = reflectVector.getAt(0);
            verify.instanceOf(reflectItem, Object);
            var realItem = realVector.getAt(0);
            verifyRuntimeObjectAgainstLanguageService(realItem, reflectItem);

            // Instances of enum types are constructed in jsgen even though enums are not generated as functions.
            // Re-enable this case when enum instances are fixed.
            /*
            logger.comment("IVector<Phylum>");
            reflectVector = reflectPVTests.receiveVectorOfEnum();
            verify.instanceOf(reflectVector, Array);
            realVector = realPVTests.receiveVectorOfEnum();
            verifyRuntimeObjectAgainstLanguageService(realVector, reflectVector);
            reflectItem = reflectVector.getAt(0);
            verify.typeOf(reflectItem, "number");
            */

            logger.comment("IVector<DelegateWithOutParam_HSTRING>");
            reflectVector = reflectPVTests.receiveVectorOfDelegate();
            verify.instanceOf(reflectVector, Array);
            realVector = realPVTests.receiveVectorOfDelegate();
            verifyRuntimeObjectAgainstLanguageService(realVector, reflectVector);
            reflectItem = reflectVector.getAt(0);
            verify.typeOf(reflectItem, "object");

            logger.comment("IVector<IVector<int>>");
            reflectVector = reflectPVTests.receiveVectorOfVector();
            verify.instanceOf(reflectVector, Array);
            realVector = realPVTests.receiveVectorOfVector();
            verifyRuntimeObjectAgainstLanguageService(realVector, reflectVector);
            reflectVector = reflectVector.getAt(0);
            verify.instanceOf(reflectVector, Array);
            realVector = realVector.getAt(0);
            verifyRuntimeObjectAgainstLanguageService(realVector, reflectVector);
            reflectItem = reflectVector.getAt(0);
            verify.typeOf(reflectItem, "number");
        }
    });

    runner.addTest({
        id: '432654-2',
        desc: 'Prototype of generic type',
        pri: '0',
        test: function () {
            verify(reflect.Windows.Foundation.Collections['IMap`2'].length, 2, "reflect.Windows.Foundation.Collections['IMap`2'].length");
            var mapInstance = new reflect.Windows.Foundation.Collections['IMap`2'](Number, String)();
            var instancePrototype = Object.getPrototypeOf(mapInstance);
            var staticPrototype = reflect.Windows.Foundation.Collections['IMap`2'].prototype;

            verify.typeOf(mapInstance.lookup(), "string");
            verify.typeOf(instancePrototype.lookup(), "string");
            verify.typeOf(staticPrototype.lookup(), "object");

            var keyValuePairInstance = mapInstance.first().current;
            instancePrototype = Object.getPrototypeOf(keyValuePairInstance);
            staticPrototype = reflect.Windows.Foundation.Collections['IKeyValuePair`2'].prototype;

            verify.typeOf(keyValuePairInstance.key, "number");
            verify.typeOf(instancePrototype.key, "number");
            verify.typeOf(staticPrototype.key, "object");
            verify.typeOf(keyValuePairInstance.value, "string");
            verify.typeOf(instancePrototype.value, "string");
            verify.typeOf(staticPrototype.value, "object");
        }
    });

    // Broken by FI -- 10/26/2011
//    runner.addTest({
//        id: '432654-3',
//        desc: 'Generic async operation',
//        pri: '0',
//        test: function () {
//            var result = "value to be replaced";
//            var picker = reflect.Windows.Storage.Pickers.FileOpenPicker();
//            picker.pickSingleFileAsync().then(function (file) {
//                result = file;
//            });
//            verify(typeof result == 'object', true, "Type should be object");
//            // Verify the type is consistant with Windows.Storage.StorageFile
//            var realPrototype = Windows.Storage.StorageFile.prototype;
//            var reflectPrototype = Object.getPrototypeOf(result);
//            verifyRuntimeObjectAgainstLanguageService(realPrototype, reflectPrototype);
//        }
//    });

    // Broken by FI -- 10/26/2011
//    runner.addTest({
//        id: '481268',
//        desc: 'Chained promises',
//        pri: '0',
//        test: function () {
//            var fileResult = "value to be replaced";
//            var streamResult = "value to be replaced";
//            var errorResult = "value to be replaced";
//            reflect.Windows.Storage.ApplicationData.current.temporaryFolder.createFileAsync("arg1").then(function (file) {
//                fileResult = file;
//                return file.openAsync(Windows.Storage.FileAccessMode.readWrite);
//            }).then(function (stream) {
//                streamResult = stream;
//            }, function (error) {
//                errorResult = error;
//            });
//            logger.comment("Verify the file result is consistant with type Windows.Storage.StorageFile");
//            verify.typeOf(fileResult, "object");
//            var realPrototype = Windows.Storage.StorageFile.prototype;
//            var reflectPrototype = Object.getPrototypeOf(fileResult);
//            verifyRuntimeObjectAgainstLanguageService(realPrototype, reflectPrototype);

//            logger.comment("Verify the stream result is consistant with type Windows.Storage.Streams.IRandomAccessStream");
//            verify((streamResult instanceof Error), false, "Result of openAsync should not be an error object");
//            verify.typeOf(streamResult, "object");
//            realPrototype = reflect.Windows.Storage.Streams.IRandomAccessStream.prototype;
//            reflectPrototype = Object.getPrototypeOf(streamResult);
//            verifyRuntimeObjectAgainstLanguageService(realPrototype, reflectPrototype);

//            logger.comment("Verify the argument to the error handler is an error object");
//            verify((errorResult instanceof Error), true, "onError argument should be instance of Error");
//            realPrototype = Error.prototype;
//            reflectPrototype = Object.getPrototypeOf(errorResult);
//            verifyRuntimeObjectAgainstLanguageService(realPrototype, reflectPrototype);
//        }
//    });

    runner.addTest({
        id: '452197',
        desc: 'Returned delegate is a function',
        pri: '0',

        test: function () {
            var realAnimal = new Animals.Animal(1);
            var reflectAnimal = new reflect.Animals.Animal(1);
            var realDelegate = realAnimal.getNativeDelegateAsOutParam();
            var reflectDelegate = reflectAnimal.getNativeDelegateAsOutParam();
            verify((reflectDelegate instanceof Function), true, "expected a functions");
            logger.comment('Reflect length: ' + reflectDelegate.length);
            logger.comment('Real length: ' + realDelegate.length);
            verify(reflectDelegate.length === realDelegate.length, true, "expected length of delegate to be the number of parameters");
        }
    });


    Loader42_FileName = "Language Service tests";

})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
