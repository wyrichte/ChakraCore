if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    var myMapMembers = [
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

    var myMapWithPropertiesMembers = [
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

    var myMapViewMembers = [
        ['first', 'function', 0],
        ['hasKey', 'function', 1],
        ['lookup', 'function', 1],
        ['size', 'number'],
        ['split', 'function', 0],
        ['toString', 'function', 0],
    ];

    var myMapViewWithPropertiesMembers = [
        ['by', 'number'],
        ['Hundred', 'number'],
        ['Hundred And Fifty', 'number'],
        ['first', 'function', 0],
        ['hasKey', 'function', 1],
        ['lookup', 'function', 1],
        ['size', 'number'],
        ['split', 'function', 0],
        ['toString', 'function', 0],
    ];

    var myObservableMapMembers = [
        ['addEventListener', 'function', 2],
        ['clear', 'function', 0],
        ['first', 'function', 0],
        ['getView', 'function', 0],
        ['hasKey', 'function', 1],
        ['insert', 'function', 2],
        ['lookup', 'function', 1],
        ['onmapchanged', 'object'],
        ['remove', 'function', 1],
        ['removeEventListener', 'function', 2],
        ['size', 'number'],
        ['toString', 'function', 0],
    ];

    var myObservableMapWithPropertiesMembers = [
        ['Hundred', 'number'],
        ['Twenty', 'number'],
        ['Five', 'number'],
        ['addEventListener', 'function', 2],
        ['clear', 'function', 0],
        ['first', 'function', 0],
        ['getView', 'function', 0],
        ['hasKey', 'function', 1],
        ['insert', 'function', 2],
        ['lookup', 'function', 1],
        ['onmapchanged', 'object'],
        ['remove', 'function', 1],
        ['removeEventListener', 'function', 2],
        ['size', 'number'],
        ['toString', 'function', 0],
    ];

    var myRCStringMapWithIterableWithPropertiesMembers = [
        ['by', 'number'],
        ['Hundred', 'number'],
        ['Hundred And Fifty', 'number'],
        ['Windows.Foundation.Collections.IIterable`1<String>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<Windows.Foundation.Collections.IKeyValuePair`2<String,Int32>>.first', 'function', 0],
        ['clear', 'function', 0],
        ['getView', 'function', 0],
        ['hasKey', 'function', 1],
        ['insert', 'function', 2],
        ['lookup', 'function', 1],
        ['remove', 'function', 1],
        ['size', 'number'],
        ['toString', 'function', 0],
    ];

    var myRCStringMapWithIterableMembers = [
        ['Windows.Foundation.Collections.IIterable`1<String>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<Windows.Foundation.Collections.IKeyValuePair`2<String,Int32>>.first', 'function', 0],
        ['clear', 'function', 0],
        ['getView', 'function', 0],
        ['hasKey', 'function', 1],
        ['insert', 'function', 2],
        ['lookup', 'function', 1],
        ['remove', 'function', 1],
        ['size', 'number'],
        ['toString', 'function', 0],
    ];

    var myRCIDoubleObservableMapMembers = [
        ['Windows.Foundation.Collections.IIterable`1<Windows.Foundation.Collections.IKeyValuePair`2<String,Int32>>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<Windows.Foundation.Collections.IKeyValuePair`2<System.Guid,Object>>.first', 'function', 0],
        ['Windows.Foundation.Collections.IMap`2<String,Int32>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IMap`2<String,Int32>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IMap`2<String,Int32>.hasKey', 'function', 1],
        ['Windows.Foundation.Collections.IMap`2<String,Int32>.insert', 'function', 2],
        ['Windows.Foundation.Collections.IMap`2<String,Int32>.lookup', 'function', 1],
        ['Windows.Foundation.Collections.IMap`2<String,Int32>.remove', 'function', 1],
        ['Windows.Foundation.Collections.IMap`2<String,Int32>.size', 'number'],
        ['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.hasKey', 'function', 1],
        ['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.insert', 'function', 2],
        ['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.lookup', 'function', 1],
        ['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.remove', 'function', 1],
        ['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.size', 'number'],
        ['Windows.Foundation.Collections.IObservableMap`2<String,Int32>.onmapchanged', 'object'],
        ['Windows.Foundation.Collections.IObservableMap`2<System.Guid,Object>.onmapchanged', 'object'],
        ['addEventListener', 'function', 2],
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

    function verifyMapContents(myMapString, myMap, arrayContents, firstPropertyName) {
        // map content order is determined by hash, the order is not guaranteed.
        if (typeof Animals._CLROnly !== 'undefined')
            return verifyMapContents_CLR(myMapString, myMap, arrayContents, firstPropertyName);

        logger.comment("Verifying contents of : " + myMapString);
        verify(myMap.size, arrayContents.length, myMapString + ".size");

        if (firstPropertyName === undefined) {
            firstPropertyName = "first";
        }

        var myIterator = myMap[firstPropertyName]();
        var index = 0;
        while (myIterator.hasCurrent && index < arrayContents.length) {
            verify(myIterator.current.key, arrayContents[index][0], "Key (" + index + "): " + myMapString);
            verify(myIterator.current.value, arrayContents[index][1], "Value (" + index + "): " + myMapString);
            myIterator.moveNext();
            index++;
        }

        verify(!myIterator.hasCurrent, true, "AllIterator members read");
        verify(index, arrayContents.length, "All expected members found");
    }

    function verifyMapContents_CLR(myMapString, myMap, arrayContents, firstPropertyName) {
        logger.comment("Verifying contents of : " + myMapString);
        verify(myMap.size, arrayContents.length, myMapString + ".size");

        if (firstPropertyName === undefined) {
            firstPropertyName = "first";
        }

        var actualMap = {};
        var myIterator = myMap[firstPropertyName]();
        var index = 0;
        while (myIterator.hasCurrent && index < arrayContents.length) {
            actualMap[myIterator.current.key] = myIterator.current.value;
            myIterator.moveNext();
            index++;
        }
        arrayContents.forEach(function (e) {
            verify(e[0], e[0], "Key (" + index + "): " + myMapString);
            verify(actualMap[e[0]], e[1], "Value (" + index + "): " + myMapString);

        });

        verify(!myIterator.hasCurrent, true, "AllIterator members read");
        verify(index, arrayContents.length, "All expected members found");
    }


    function verifyMapEnumerator(myMapString, myMap, prototypeMembers, mapContents) {
        // map content order is determined by hash, the order is not guaranteed.
        if (typeof Animals._CLROnly !== 'undefined')
            return verifyMapEnumerator_CLR(myMapString, myMap, prototypeMembers, mapContents);
        logger.comment("Verifying enumerator of : " + myMapString);
        var indexInPrototypeMembers = 0;
        var indexInPropertySetContent = 0;
        if (mapContents === undefined) {
            mapContents = [];
        }

        var propertyIndex = 0;
        for (p in myMap) {
            if (indexInPropertySetContent < mapContents.length) {
                verify(p, mapContents[indexInPropertySetContent][0], myMapString + '\'s property: ' + propertyIndex);
                verify(typeof myMap[p], typeof mapContents[indexInPropertySetContent][1], 'typeof ' + myMapString + '["' + p + '"]');
                verify(myMap[p], mapContents[indexInPropertySetContent][1], myMapString + '["' + p + '"]');
                indexInPropertySetContent++;
            }
            else {
                // In prototype
                verify(p, prototypeMembers[indexInPrototypeMembers][0], myMapString + '\'s property: ' + propertyIndex);
                verify(typeof myMap[p], prototypeMembers[indexInPrototypeMembers][1], "typeof" + myMapString + '["' + p + '"]');
                indexInPrototypeMembers++;
            }
            propertyIndex++;
        }

        verify(indexInPropertySetContent, mapContents.length, "All propertySet contents read?");
        verify(indexInPrototypeMembers, prototypeMembers.length, "All prototype members read?");
    }

    function verifyMapEnumerator_CLR(myMapString, myMap, prototypeMembers, mapContents) {
        logger.comment("Verifying enumerator of : " + myMapString);
        var indexInPrototypeMembers = 0;
        var indexInPropertySetContent = 0;
        if (mapContents === undefined) {
            mapContents = [];
        }

        var actual = {};

        var propertyIndex = 0;
        for (p in myMap) {
            if (indexInPropertySetContent < mapContents.length) {
                actual[p] = { name:p, type: typeof myMap[p], value: myMap[p] };
                indexInPropertySetContent++;
            }
            else {
                // In prototype
                verify(p, prototypeMembers[indexInPrototypeMembers][0], myMapString + '\'s property: ' + propertyIndex);
                verify(typeof myMap[p], prototypeMembers[indexInPrototypeMembers][1], "typeof" + myMapString + '["' + p + '"]');
                indexInPrototypeMembers++;
            }
            propertyIndex++;
        }
        for (var i = 0; i < mapContents.length; i++) {
            var p = mapContents[i][0];
            verify(actual[p].name, mapContents[i][0], myMapString + '\'s property: ' + i);
            verify(actual[p].type, typeof mapContents[i][1], 'typeof ' + myMapString + '["' + p + '"]');
            verify(actual[p].value, mapContents[i][1], myMapString + '["' + p + '"]');
        }


        verify(indexInPropertySetContent, mapContents.length, "All propertySet contents read?");
        verify(indexInPrototypeMembers, prototypeMembers.length, "All prototype members read?");
    }

    runner.addTest({
        id: 1,
        desc: 'IMap : IMap of String and Integer',
        pri: '0',
        test: function () {
            var myMap = Animals.Animal.getStringIntegerMap();
            verifyMembers("myMap", myMap, myMapWithPropertiesMembers);
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]]);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'IMap : Set the properties',
        pri: '0',
        test: function () {
            var myMap = Animals.Animal.getStringIntegerMap();

            myMap["name"] = 4;
            verify(myMap["name"], 4, 'myMap["name"]');
            myMap["Twenty"] = 6;
            verify(myMap["Twenty"], 6, 'myMap["Twenty"]');
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17], ["name", 4]]);

            myMap["name"] = 2;
            verify(myMap["name"], 2, 'myMap["name"]');
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17], ["name", 2]]);

            myMap[0] = 0;
            verify(myMap[0], 0, 'myMap[0]');
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["0", 0], ["Twenty", 6], ["Hundred And Fifty", 17], ["name", 2]]);

            myMap["4"] = 4;
            verify(myMap["4"], 4, 'myMap["4"]');
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["0", 0], ["Twenty", 6], ["Hundred And Fifty", 17], ["4", 4], ["name", 2]]);

            myMap["Twenty"] = null;
            verify(myMap["Twenty"], 0, 'myMap["Twenty"]');
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["0", 0], ["Twenty", 0], ["Hundred And Fifty", 17], ["4", 4], ["name", 2]]);

            verify(myMap["undefinedProperty"], undefined, 'myMap["undefinedProperty"]');
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["0", 0], ["Twenty", 0], ["Hundred And Fifty", 17], ["4", 4], ["name", 2]]);

            myMap["Twenty"] = undefined;
            verify(myMap["Twenty"], 0, 'myMap["Twenty"]');
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["0", 0], ["Twenty", 0], ["Hundred And Fifty", 17], ["4", 4], ["name", 2]]);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'IMap : Delete the properties',
        pri: '0',
        test: function () {
            var myMap = Animals.Animal.getStringIntegerMap();

            delete myMap["by"];
            verify(myMap["by"], undefined, 'myMap["by"]');
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["Hundred And Fifty", 17]]);

            myMap[0] = 70;
            verify(myMap[0], 70, 'myMap[0]');
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["0", 70], ["Hundred And Fifty", 17]]);

            delete myMap[0];
            verify(myMap[0], undefined, 'myMap[0]');
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["Hundred And Fifty", 17]]);

            myMap["4"] = 40;
            verify(myMap["4"], 40, 'myMap["4"]');
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["Hundred And Fifty", 17], ["4", 40]]);

            delete myMap["4"];
            verify(myMap["4"], undefined, 'myMap["4"]');
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["Hundred And Fifty", 17]]);

            var n = delete myMap["undefinedProperty"];
            verify(n, false, 'delete myMap["undefinedProperty"]');
            verify(myMap["undefinedProperty"], undefined, 'myMap["undefinedProperty"]');
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["Hundred And Fifty", 17]]);

            n = delete myMap[75];
            verify(n, false, 'delete myMap[75]');
            verify(myMap[75], undefined, 'myMap[75]');
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["Hundred And Fifty", 17]]);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'IMap : Enumerator',
        pri: '0',
        test: function () {
            var myMap = Animals.Animal.getStringIntegerMap();
            myMap[20] = 20;
            var expectedContents = [["by", 2], ["Hundred", 7], ["20", 20], ["Hundred And Fifty", 17]];

            verifyMapContents("myMap", myMap, expectedContents);
            verifyMapEnumerator("myMap", myMap, myMapMembers, expectedContents);

            logger.comment("Remove all properties and verify the enumerator");
            delete myMap["by"];
            delete myMap["Hundred"];
            delete myMap["Hundred And Fifty"];
            delete myMap[20];
            verifyMapContents("myMap", myMap, []);
            verifyMapEnumerator("myMap", myMap, myMapMembers);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'IMap : Modifying values in Enumerator',
        pri: '0',
        test: function () {
            var myMap = Animals.Animal.getStringIntegerMap();

            myMap[20] = 20;
            var expectedContents = [["by", 2], ["Hundred", 7], ["20", 20], ["Hundred And Fifty", 17]];
            verifyMapContents("myMap", myMap, expectedContents);
            verifyMapEnumerator("myMap", myMap, myMapMembers, expectedContents);

            expectedContents = [["by", 2], ["Hundred And Fifty", 150]];
            var indexInPrototypeMembers = 0;
            var indexInMapContent = 0;
            var propertyIndex = 0;
            var myMapString = "myMap";
            var prototypeMembers = myMapMembers;
            for (p in myMap) {
                if (indexInMapContent < expectedContents.length) {
                    verify(p, expectedContents[indexInMapContent][0], '* ' + myMapString + '\'s property: ' + propertyIndex);
                    verify(typeof myMap[p], typeof expectedContents[indexInMapContent][1], '* typeof ' + myMapString + '["' + p + '"]');
                    verify(myMap[p], expectedContents[indexInMapContent][1], '* ' + myMapString + '["' + p + '"]');
                    indexInMapContent++;
                }
                else {
                    // In prototype
                    verify(p, prototypeMembers[indexInPrototypeMembers][0], '* ' + myMapString + '\'s property: ' + propertyIndex);
                    verify(typeof myMap[p], prototypeMembers[indexInPrototypeMembers][1], "* typeof" + myMapString + '["' + p + '"]');
                    indexInPrototypeMembers++;
                }

                if (myMap[p] == 2) {
                    logger.comment("*** Modify the properties while enumerating : Start ***");

                    myMap["by"] = 20;
                    myMap["Hundred And Fifty"] = 150;

                    myMap["Adventurous"] = 1;
                    myMap["NewToSwimming"] = 0;

                    delete myMap[20];
                    delete myMap["Hundred"];

                    verify(myMap["by"], 20, 'myMap["by"]');
                    verify(myMap["Hundred And Fifty"], 150, 'myMap["Hundred And Fifty"]');

                    verify(myMap["Adventurous"], 1, 'myMap["Adventurous"]');
                    verify(myMap["NewToSwimming"], 0, 'myMap["NewToSwimming"]');

                    verify(myMap["20"], undefined, 'myMap["20"]');
                    verify(myMap["Hundred"], undefined, 'myMap["Hundred"]');
                    logger.comment("*** Modify the properties while enumerating : End ***");
                }

                propertyIndex++;
            }

            verify(indexInMapContent, expectedContents.length, "* All Map contents read?");
            verify(indexInPrototypeMembers, prototypeMembers.length, "* All prototype members read?");

            expectedContents = [["Adventurous", 1], ["by", 20], ["Hundred And Fifty", 150], ["NewToSwimming", 0]];
            verifyMapContents("myMap", myMap, expectedContents);
            verifyMapEnumerator("myMap", myMap, myMapMembers, expectedContents);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'IMap : Name conflicting properties : 0, 32 and insert',
        pri: '0',
        test: function () {
            var myMapWithPropertiesAndExpandosMembers = [
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
                ['0', 'string'],
                ['32', 'string'],
            ];

            var myMapWithExpandosMembers = [
                ['clear', 'function', 0],
                ['first', 'function', 0],
                ['getView', 'function', 0],
                ['hasKey', 'function', 1],
                ['insert', 'function', 2],
                ['lookup', 'function', 1],
                ['remove', 'function', 1],
                ['size', 'number'],
                ['toString', 'function', 0],
                ['0', 'string'],
                ['32', 'string'],
            ];

            logger.comment("*** Setup prototype for conflicting properties");
            var myMap = Animals.Animal.getStringIntegerMap();
            Object.prototype["0"] = "0 Property";
            Object.prototype["32"] = "32 Property";
            verifyMembers("myMap", myMap, myMapWithPropertiesAndExpandosMembers);

            logger.comment("*** Set conflicting properties using index operation");
            myMap["insert"] = 55; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myMap.hasKey("insert"), false, 'myMap.hasKey("insert")');
            verify(typeof myMap["insert"], "function", 'typeof myMap["insert"]');
            verify(myMap["insert"].toString(), "function insert() { [native code] }", 'myMap["insert"].toString()');

            myMap["0"] = 66; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myMap.hasKey("0"), false, 'myMap.hasKey("0")');
            verify(myMap["0"], "0 Property", 'myMap["0"]');

            myMap["32"] = 77; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myMap.hasKey("32"), false, 'myMap.hasKey("32")');
            verify(myMap["32"], "32 Property", 'myMap["32"]');

            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]]);
            verifyMapEnumerator("myMap", myMap, myMapWithExpandosMembers, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]]);

            logger.comment("*** Add Map members using .insert method");
            myMap.insert("insert", 55);
            verify(myMap.hasKey("insert"), true, 'myMap.hasKey("insert")');
            verify(myMap.lookup("insert"), 55, 'myMap.lookup("insert")');
            verify(typeof myMap["insert"], "function", 'typeof myMap["insert"]');
            verify(myMap["insert"].toString(), "function insert() { [native code] }", 'myMap["insert"].toString()');

            myMap.insert("0", 66);
            verify(myMap.hasKey("0"), true, 'myMap.hasKey("0")');
            verify(myMap.lookup("0"), 66, 'myMap.lookup("0")');
            verify(myMap["0"], "0 Property", 'myMap["0"]');

            myMap.insert("32", 77);
            verify(myMap.hasKey("32"), true, 'myMap.hasKey("32")');
            verify(myMap.lookup("32"), 77, 'myMap.lookup("32")');
            verify(myMap["32"], "32 Property", 'myMap["32"]');

            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["0", 66], ["32", 77], ["Hundred And Fifty", 17], ["insert", 55]]);
            verifyMapEnumerator("myMap", myMap, myMapWithExpandosMembers, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]]);

            logger.comment("*** Delete the conflicting properties - shouldnt change anything");
            delete myMap["insert"];
            verify(myMap.hasKey("insert"), true, 'myMap.hasKey("insert")');
            verify(myMap.lookup("insert"), 55, 'myMap.lookup("insert")');
            verify(typeof myMap["insert"], "function", 'typeof myMap["insert"]');
            verify(myMap["insert"].toString(), "function insert() { [native code] }", 'myMap["insert"].toString()');

            delete myMap["0"]
            verify(myMap.hasKey("0"), true, 'myMap.hasKey("0")');
            verify(myMap.lookup("0"), 66, 'myMap.lookup("0")');
            verify(myMap["0"], "0 Property", 'myMap["0"]');

            delete myMap["32"]
            verify(myMap.hasKey("32"), true, 'myMap.hasKey("32")');
            verify(myMap.lookup("32"), 77, 'myMap.lookup("32")');
            verify(myMap["32"], "32 Property", 'myMap["32"]');

            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["0", 66], ["32", 77], ["Hundred And Fifty", 17], ["insert", 55]]);
            verifyMapEnumerator("myMap", myMap, myMapWithExpandosMembers, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]]);

            logger.comment("*** Set floating point properties");
            myMap["0.0"] = 11
            verify(myMap.hasKey("0.0"), true, 'myMap.hasKey("0.0")');
            verify(myMap.lookup("0.0"), 11, 'myMap.lookup("0.0")');
            verify(myMap["0.0"], 11, 'myMap["0.0"]');

            myMap["32.56"] = 22
            verify(myMap.hasKey("32.56"), true, 'myMap.hasKey("32.56")');
            verify(myMap.lookup("32.56"), 22, 'myMap.lookup("32.56")');
            verify(myMap["32.56"], 22, 'myMap["32.56"]');

            verifyMapContents("myMap", myMap, [["by", 2], ["0.0", 11], ["Hundred", 7], ["32.56", 22], ["0", 66], ["32", 77], ["Hundred And Fifty", 17], ["insert", 55]]);
            verifyMapEnumerator("myMap", myMap, myMapWithExpandosMembers, [["by", 2], ["0.0", 11], ["Hundred", 7], ["32.56", 22], ["Hundred And Fifty", 17]]);

            logger.comment("*** Delete the object.prototype properties");
            delete Object.prototype["0"];
            delete Object.prototype["32"];

            verify(myMap["0"], 66, 'myMap["0"]');
            verify(myMap["32"], 77, 'myMap["32"]');
            verifyMapContents("myMap", myMap, [["by", 2], ["0.0", 11], ["Hundred", 7], ["32.56", 22], ["0", 66], ["32", 77], ["Hundred And Fifty", 17], ["insert", 55]]);
            verifyMapEnumerator("myMap", myMap, myMapMembers, [["by", 2], ["0.0", 11], ["Hundred", 7], ["32.56", 22], ["0", 66], ["32", 77], ["Hundred And Fifty", 17]]);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'IMap : IMap methods and [] mixing',
        pri: '0',
        test: function () {
            var myMap = Animals.Animal.getStringIntegerMap();
            logger.comment("*** Set property using [] operation");
            myMap["prop1"] = 1;
            myMap["prop2"] = 2;
            myMap["prop3"] = 3;
            verifyMapContents("myMap", myMap, [["prop2", 2], ["by", 2], ["Hundred", 7], ["prop1", 1], ["Hundred And Fifty", 17], ["prop3", 3]]);

            logger.comment("*** Add property override using insert method");
            var replaced = myMap.insert("prop2", 22);
            verify(replaced, true, 'myMap.insert("prop2", 22)');
            verifyMapContents("myMap", myMap, [["prop2", 22], ["by", 2], ["Hundred", 7], ["prop1", 1], ["Hundred And Fifty", 17], ["prop3", 3]]);

            logger.comment("*** Remove property using remove method");
            myMap.remove("prop1");
            verifyMapContents("myMap", myMap, [["prop2", 22], ["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17], ["prop3", 3]]);

            logger.comment("*** Clear all properties using clear method");
            myMap.clear();
            verifyMapContents("myMap", myMap, []);
        }
    });

    runner.addTest({
        id: 8,
        desc: 'IMap : Non specialized IMap',
        pri: '0',
        test: function () {
            function verifyMapOfStructAndVectorContents(myMapString, myMap, arrayContents) {
                // map content order is determined by hash, the order is not guaranteed.
                if (typeof Animals._CLROnly !== 'undefined')
                    return verifyMapOfStructAndVectorContents_CLR(myMapString, myMap, arrayContents);
                logger.comment("Verifying contents of : " + myMapString);
                verify(myMap.size, arrayContents.length, myMapString + ".size");
                var myIterator = myMap.first();
                var index = 0;
                while (myIterator.hasCurrent && index < arrayContents.length) {
                    logger.comment("* Item: " + index);
                    logger.comment("  -Key: " + index);
                    verify(myIterator.current.key.length, arrayContents[index][0].length, "Key (" + index + ").length: " + myMapString);
                    verify(myIterator.current.key.width, arrayContents[index][0].width, "Key (" + index + ").width: " + myMapString);
                    logger.comment("  -Value: " + index);
                    verify(myIterator.current.value.length, arrayContents[index][1].length, "Value (" + index + ").length: " + myMapString);

                    var arrayIndex = 0;
                    while (arrayIndex < myIterator.current.value.length && arrayIndex < arrayContents[index][1].lenth) {
                        verify(myIterator.current.value[arrayIndex], arrayContents[index][1][arrayIndex], "Value (" + index + ")[arrayIndex]: " + myMapString);
                    }
                    myIterator.moveNext();
                    index++;
                }

                verify(!myIterator.hasCurrent, true, "AllIterator members read");
                verify(index, arrayContents.length, "All expected members found");
            }

            function verifyMapOfStructAndVectorContents_CLR(myMapString, myMap, arrayContents) {
                logger.comment("Verifying contents of : " + myMapString);
                verify(myMap.size, arrayContents.length, myMapString + ".size");
                var myIterator = myMap.first();
                var index = 0;

                var actual = {};

                while (myIterator.hasCurrent && index < arrayContents.length) {
                    var key=JSON.stringify(myIterator.current.key);
                    actual[key] = myIterator.current;
                    myIterator.moveNext();
                    index++;
                }

                index = 0;
                arrayContents.forEach(function (e) {
                    var key = JSON.stringify(e[0]);

                    logger.comment("* Item: " + index);
                    logger.comment("  -Key: " + index);
                    verify(actual[key].key.length, arrayContents[index][0].length, "Key (" + index + ").length: " + myMapString);
                    verify(actual[key].key.width, arrayContents[index][0].width, "Key (" + index + ").width: " + myMapString);
                    logger.comment("  -Value: " + index);
                    verify(actual[key].value.length, arrayContents[index][1].length, "Value (" + index + ").length: " + myMapString);

                    var arrayIndex = 0;
                    while (arrayIndex < actual[key].value.length && arrayIndex < arrayContents[index][1].lenth) {
                        verify(actual[key].value[arrayIndex], arrayContents[index][1][arrayIndex], "Value (" + index + ")[arrayIndex]: " + myMapString);
                    }
                    index++;

                });


                verify(!myIterator.hasCurrent, true, "AllIterator members read");
                verify(index, arrayContents.length, "All expected members found");
            }


            var propertyValueTests = new Animals.PropertyValueTests();
            var myMapOfStructAndVector = propertyValueTests.receiveMapOfStructAndVector();
            verifyMembers("myMapOfStructAndVector", myMapOfStructAndVector, myMapMembers);
            verifyMapOfStructAndVectorContents("myMapOfStructAndVector", myMapOfStructAndVector, [[{ length: 150, width: 100 }, ["Hundred And Fifty", "by", "Hundred"]], [{ length: 100, width: 100 }, ["Hundred", "by", "Hundred"]]]);

            logger.comment("*** Set/Get property using [] operation");
            myMapOfStructAndVector["{length: 20, width: 30}"] = ["Twenty", "Thirty"];
            verify(myMapOfStructAndVector["{length: 20, width: 30}"], undefined, 'myMapOfStructAndVector["{length: 20, width: 30}"]');
            verifyMapOfStructAndVectorContents("myMapOfStructAndVector", myMapOfStructAndVector, [[{ length: 150, width: 100 }, ["Hundred And Fifty", "by", "Hundred"]], [{ length: 100, width: 100 }, ["Hundred", "by", "Hundred"]]]);

            myMapOfStructAndVector[{ length: 20, width: 30}] = ["Twenty", "Thirty"];
            verify(myMapOfStructAndVector[{ length: 20, width: 30}], undefined, 'myMapOfStructAndVector[{length: 20, width: 30}]');
            verifyMapOfStructAndVectorContents("myMapOfStructAndVector", myMapOfStructAndVector, [[{ length: 150, width: 100 }, ["Hundred And Fifty", "by", "Hundred"]], [{ length: 100, width: 100 }, ["Hundred", "by", "Hundred"]]]);

            logger.comment("*** Add property override using insert method");
            var replaced = myMapOfStructAndVector.insert({ length: 20, width: 30 }, ["Twenty", "Thirty"]);
            verify(replaced, false, 'myMapOfStructAndVector.insert({length: 20, width: 30}, ["Twenty", "Thirty"])');
            verifyMapOfStructAndVectorContents("myMapOfStructAndVector", myMapOfStructAndVector, [[{ length: 150, width: 100 }, ["Hundred And Fifty", "by", "Hundred"]], [{ length: 20, width: 30 }, ["Twenty", "Thirty"]], [{ length: 100, width: 100 }, ["Hundred", "by", "Hundred"]]]);

            logger.comment("*** Remove property using remove method");
            myMapOfStructAndVector.remove({ length: 150, width: 100 });
            verifyMapOfStructAndVectorContents("myMapOfStructAndVector", myMapOfStructAndVector, [[{ length: 20, width: 30 }, ["Twenty", "Thirty"]], [{ length: 100, width: 100 }, ["Hundred", "by", "Hundred"]]]);

            logger.comment("*** Clear all properties using clear method");
            myMapOfStructAndVector.clear();
            verifyMapOfStructAndVectorContents("myMapOfStructAndVector", myMapOfStructAndVector, []);
        }
    });

    runner.addTest({
        id: 9,
        desc: 'IMapView : IMapView of String and Integer',
        pri: '0',
        preReq: function () {
            // CLR map.getView not returning IMapView, it returns IMap with IMapVIew member functions
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myMap = Animals.Animal.getStringIntegerMap();
            var myMapView = myMap.getView();
            verifyMembers("myMapView", myMapView, myMapViewWithPropertiesMembers);
            verifyMapContents("myMapView", myMapView, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]]);
        }
    });

    runner.addTest({
        id: 10,
        desc: 'IMapView : Set the properties',
        pri: '0',
        preReq: function () {
            // CLR map.getView not returning IMapView, it returns IMap with IMapVIew member functions
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myMapView = Animals.Animal.getStringIntegerMap().getView();

            verify(myMapView["undefinedProperty"], undefined, 'myMapView["undefinedProperty"]');
            myMapView["name"] = 4;
            verify(myMapView["name"], undefined, 'myMapView["name"]');
            myMapView[0] = 0;
            verify(myMapView[0], undefined, 'myMapView[0]');
            myMapView["4"] = 4;
            verify(myMapView["4"], undefined, 'myMapView["4"]');
            verifyMapContents("myMapView", myMapView, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]]);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'IMapView : Delete the properties',
        pri: '0',
        preReq: function () {
            // CLR map.getView not returning IMapView, it returns IMap with IMapVIew member functions
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myOriginalMap = Animals.Animal.getStringIntegerMap();
            myOriginalMap[0] = 70;
            myOriginalMap["4"] = 40;

            var myMapView = myOriginalMap.getView();

            delete myMapView["by"];
            verify(myMapView["by"], 2, 'myMapView["by"]');
            delete myMapView[0];
            verify(myMapView[0], 70, 'myMapView[0]');
            delete myMapView["4"];
            verify(myMapView["4"], 40, 'myMapView["4"]');
            verifyMapContents("myMapView", myMapView, [["by", 2], ["Hundred", 7], ["0", 70], ["Hundred And Fifty", 17], ["4", 40]]);

            var n = delete myMapView["undefinedProperty"];
            verify(n, false, 'delete myMapView["undefinedProperty"]');
            verify(myMapView["undefinedProperty"], undefined, 'myMapView["undefinedProperty"]');
            verifyMapContents("myMapView", myMapView, [["by", 2], ["Hundred", 7], ["0", 70], ["Hundred And Fifty", 17], ["4", 40]]);

            n = delete myMapView[75];
            verify(n, false, 'delete myMapView[75]');
            verify(myMapView[75], undefined, 'myMapView[75]');
            verifyMapContents("myMapView", myMapView, [["by", 2], ["Hundred", 7], ["0", 70], ["Hundred And Fifty", 17], ["4", 40]]);
        }
    });

    runner.addTest({
        id: 12,
        desc: 'IMapView : Enumerator',
        pri: '0',
        preReq: function () {
            // CLR map.getView not returning IMapView, it returns IMap with IMapVIew member functions
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myOriginalMap = Animals.Animal.getStringIntegerMap();
            myOriginalMap[20] = 20;
            var myMapView = myOriginalMap.getView();
            var expectedContents = [["by", 2], ["Hundred", 7], ["20", 20], ["Hundred And Fifty", 17]];

            verifyMapContents("myMapView", myMapView, expectedContents);
            verifyMapEnumerator("myMapView", myMapView, myMapViewMembers, expectedContents);

            logger.comment("Remove all properties and verify the enumerator");
            delete myOriginalMap["by"];
            delete myOriginalMap["Hundred"];
            delete myOriginalMap["Hundred And Fifty"];
            delete myOriginalMap[20];
            myMapView = myOriginalMap.getView();
            verifyMapContents("myMapView", myMapView, []);
            verifyMapEnumerator("myMapView", myMapView, myMapViewMembers);
        }
    });

    runner.addTest({
        id: 13,
        desc: 'IMapView : Name conflicting properties : 0, 32 and lookup',
        pri: '0',
        preReq: function () {
            // CLR map.getView not returning IMapView, it returns IMap with IMapVIew member functions
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myMapViewWithPropertiesAndExpandosMembers = [
                ['by', 'number'],
                ['Hundred', 'number'],
                ['Hundred And Fifty', 'number'],
                ['first', 'function', 0],
                ['hasKey', 'function', 1],
                ['lookup', 'function', 1],
                ['size', 'number'],
                ['split', 'function', 0],
                ['toString', 'function', 0],
                ['0', 'string'],
                ['32', 'string'],
            ];

            var myMapViewWithExpandosMembers = [
                ['first', 'function', 0],
                ['hasKey', 'function', 1],
                ['lookup', 'function', 1],
                ['size', 'number'],
                ['split', 'function', 0],
                ['toString', 'function', 0],
                ['0', 'string'],
                ['32', 'string'],
            ];

            logger.comment("*** Setup prototype for conflicting properties");
            var myMap = Animals.Animal.getStringIntegerMap();
            var myMapView = myMap.getView();
            Object.prototype["0"] = "0 Property";
            Object.prototype["32"] = "32 Property";
            verifyMembers("myMapView", myMapView, myMapViewWithPropertiesAndExpandosMembers);

            logger.comment("*** Add Map members using .insert and verify the get operation using [] method");
            myMap.insert("lookup", 55);
            myMapView = myMap.getView();
            verify(myMapView.hasKey("lookup"), true, 'myMapView.hasKey("insert")');
            verify(myMapView.lookup("lookup"), 55, 'myMapView.lookup("insert")');
            verify(typeof myMapView["lookup"], "function", 'typeof myMapView["lookup"]');
            verify(myMapView["lookup"].toString(), "function lookup() { [native code] }", 'myMapView["lookup"].toString()');

            myMap.insert("0", 66);
            myMapView = myMap.getView();
            verify(myMapView.hasKey("0"), true, 'myMapView.hasKey("0")');
            verify(myMapView.lookup("0"), 66, 'myMapView.lookup("0")');
            verify(myMapView["0"], "0 Property", 'myMapView["0"]');

            myMap.insert("32", 77);
            myMapView = myMap.getView();
            verify(myMapView.hasKey("32"), true, 'myMapView.hasKey("32")');
            verify(myMapView.lookup("32"), 77, 'myMapView.lookup("32")');
            verify(myMapView["32"], "32 Property", 'myMapView["32"]');

            verifyMapContents("myMapView", myMapView, [["lookup", 55], ["by", 2], ["Hundred", 7], ["0", 66], ["32", 77], ["Hundred And Fifty", 17]]);
            verifyMapEnumerator("myMapView", myMapView, myMapViewWithExpandosMembers, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]]);

            logger.comment("*** Delete the object.prototype properties");
            delete Object.prototype["0"];
            delete Object.prototype["32"];

            verify(myMapView["0"], 66, 'myMapView["0"]');
            verify(myMapView["32"], 77, 'myMapView["32"]');
            verifyMapContents("myMapView", myMapView, [["lookup", 55], ["by", 2], ["Hundred", 7], ["0", 66], ["32", 77], ["Hundred And Fifty", 17]]);
            verifyMapEnumerator("myMapView", myMapView, myMapViewMembers, [["by", 2], ["Hundred", 7], ["0", 66], ["32", 77], ["Hundred And Fifty", 17]]);
        }
    });

    runner.addTest({
        id: 14,
        desc: 'IObservableMap : IObservableMap of String and Integer',
        pri: '0',
        test: function () {
            var myObservableMap = Animals.Animal.getObservableStringIntegerMap();
            verifyMembers("myObservableMap", myObservableMap, myObservableMapWithPropertiesMembers);
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["Twenty", 20], ["Five", 5]]);
        }
    });

    runner.addTest({
        id: 15,
        desc: 'IObservableMap : Set the properties',
        pri: '0',
        test: function () {
            var myObservableMap = Animals.Animal.getObservableStringIntegerMap();

            myObservableMap["Thirty"] = 3;
            verify(myObservableMap["Thirty"], 3, 'myObservableMap["Thirty"]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["Twenty", 20], ["Thirty", 3], ["Five", 5]]);

            myObservableMap["Thirty"] = 30;
            verify(myObservableMap["Thirty"], 30, 'myObservableMap["Thirty"]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["Twenty", 20], ["Thirty", 30], ["Five", 5]]);

            myObservableMap[0] = 10;
            verify(myObservableMap[0], 10, 'myObservableMap[0]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["0", 10], ["Twenty", 20], ["Thirty", 30], ["Five", 5]]);

            myObservableMap[0] = 0;
            verify(myObservableMap[0], 0, 'myObservableMap[0]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["0", 0], ["Twenty", 20], ["Thirty", 30], ["Five", 5]]);

            myObservableMap["Twenty"] = null;
            verify(myObservableMap["Twenty"], 0, 'myObservableMap["Twenty"]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["0", 0], ["Twenty", 0], ["Thirty", 30], ["Five", 5]]);

            verify(myObservableMap["undefinedProperty"], undefined, 'myObservableMap["undefinedProperty"]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["0", 0], ["Twenty", 0], ["Thirty", 30], ["Five", 5]]);

            myObservableMap["Twenty"] = undefined;
            verify(myObservableMap["Twenty"], 0, 'myObservableMap["Twenty"]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["0", 0], ["Twenty", 0], ["Thirty", 30], ["Five", 5]]);
        }
    });

    runner.addTest({
        id: 16,
        desc: 'IObservableMap : Delete the properties',
        pri: '0',
        test: function () {
            var myObservableMap = Animals.Animal.getObservableStringIntegerMap();

            delete myObservableMap["Twenty"];
            verify(myObservableMap["Twenty"], undefined, 'myObservableMap["Twenty"]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["Five", 5]]);

            myObservableMap[0] = 0;
            verify(myObservableMap[0], 0, 'myObservableMap[0]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["0", 0], ["Five", 5]]);

            delete myObservableMap[0];
            verify(myObservableMap[0], undefined, 'myObservableMap[0]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["Five", 5]]);

            var n = delete myObservableMap["undefinedProperty"];
            verify(n, false, 'delete myObservableMap["undefinedProperty"]');
            verify(myObservableMap["undefinedProperty"], undefined, 'myObservableMap["undefinedProperty"]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["Five", 5]]);

            n = delete myObservableMap[75];
            verify(n, false, 'delete myObservableMap[75]');
            verify(myObservableMap[75], undefined, 'myObservableMap[75]');
            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["Five", 5]]);
        }
    });

    runner.addTest({
        id: 17,
        desc: 'IObservableMap : Enumerator',
        pri: '0',
        test: function () {
            var myObservableMap = Animals.Animal.getObservableStringIntegerMap();
            myObservableMap[20] = 20;
            var expectedContents = [["Hundred", 100], ["20", 20], ["Twenty", 20], ["Five", 5]];

            verifyMapContents("myObservableMap", myObservableMap, expectedContents);
            verifyMapEnumerator("myObservableMap", myObservableMap, myObservableMapMembers, expectedContents);

            logger.comment("Remove all properties and verify the enumerator");
            delete myObservableMap[20];
            delete myObservableMap["Hundred"];
            delete myObservableMap["Twenty"];
            delete myObservableMap["Five"];
            verifyMapContents("myObservableMap", myObservableMap, []);
            verifyMapEnumerator("myObservableMap", myObservableMap, myObservableMapMembers);
        }
    });

    runner.addTest({
        id: 18,
        desc: 'IObservableMap : Name conflicting properties : addEventListener and onmapchanged',
        pri: '0',
        test: function () {
            var myObservableMap = Animals.Animal.getObservableStringIntegerMap();

            logger.comment("*** Set conflicting properties using index operation");
            myObservableMap["addEventListener"] = 55; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myObservableMap.hasKey("addEventListener"), false, 'myObservableMap.hasKey("addEventListener")');
            verify(typeof myObservableMap["addEventListener"], "function", 'typeof myObservableMap["addEventListener"]');
            verify(myObservableMap["addEventListener"].toString(), "function addEventListener() { [native code] }", 'myObservableMap["addEventListener"].toString()');

            myObservableMap["onmapchanged"] = 66; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myObservableMap.hasKey("onmapchanged"), false, 'myObservableMap.hasKey("onmapchanged")');
            verify(typeof myObservableMap["onmapchanged"], "object", 'typeof myObservableMap["onmapchanged"]');
            verify(myObservableMap["onmapchanged"], null, 'myObservableMap["onmapchanged"]');

            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["Twenty", 20], ["Five", 5]]);
            verifyMapEnumerator("myObservableMap", myObservableMap, myObservableMapMembers, [["Hundred", 100], ["Twenty", 20], ["Five", 5]]);

            logger.comment("*** Add Map members using .insert method");
            myObservableMap.insert("addEventListener", 55);
            verify(myObservableMap.hasKey("addEventListener"), true, 'myObservableMap.hasKey("addEventListener")');
            verify(myObservableMap.lookup("addEventListener"), 55, 'myObservableMap.lookup("addEventListener")');
            verify(typeof myObservableMap["addEventListener"], "function", 'typeof myObservableMap["addEventListener"]');
            verify(myObservableMap["addEventListener"].toString(), "function addEventListener() { [native code] }", 'myObservableMap["addEventListener"].toString()');

            myObservableMap.insert("onmapchanged", 66);
            verify(myObservableMap.hasKey("onmapchanged"), true, 'myObservableMap.hasKey("onmapchanged")');
            verify(myObservableMap.lookup("onmapchanged"), 66, 'myObservableMap.lookup("onmapchanged")');
            verify(typeof myObservableMap["onmapchanged"], "object", 'typeof myObservableMap["onmapchanged"]');
            verify(myObservableMap["onmapchanged"], null, 'myObservableMap["onmapchanged"]');

            verifyMapContents("myObservableMap", myObservableMap, [["addEventListener", 55], ["Hundred", 100], ["Twenty", 20], ["Five", 5], ["onmapchanged", 66]]);
            verifyMapEnumerator("myObservableMap", myObservableMap, myObservableMapMembers, [["Hundred", 100], ["Twenty", 20], ["Five", 5]]);

            logger.comment("*** Delete the conflicting properties - shouldnt change anything");
            delete myObservableMap["addEventListener"];
            verify(myObservableMap.hasKey("addEventListener"), true, 'myObservableMap.hasKey("addEventListener")');
            verify(myObservableMap.lookup("addEventListener"), 55, 'myObservableMap.lookup("addEventListener")');
            verify(typeof myObservableMap["addEventListener"], "function", 'typeof myObservableMap["addEventListener"]');
            verify(myObservableMap["addEventListener"].toString(), "function addEventListener() { [native code] }", 'myObservableMap["addEventListener"].toString()');

            delete myObservableMap["onmapchanged"];
            verify(myObservableMap.hasKey("onmapchanged"), true, 'myObservableMap.hasKey("onmapchanged")');
            verify(myObservableMap.lookup("onmapchanged"), 66, 'myObservableMap.lookup("onmapchanged")');
            verify(typeof myObservableMap["onmapchanged"], "object", 'typeof myObservableMap["onmapchanged"]');
            verify(myObservableMap["onmapchanged"], null, 'myObservableMap["onmapchanged"]');

            verifyMapContents("myObservableMap", myObservableMap, [["addEventListener", 55], ["Hundred", 100], ["Twenty", 20], ["Five", 5], ["onmapchanged", 66]]);
            verifyMapEnumerator("myObservableMap", myObservableMap, myObservableMapMembers, [["Hundred", 100], ["Twenty", 20], ["Five", 5]]);

            logger.comment("*** Delete the conflicting properties using .remove method");
            myObservableMap.remove("addEventListener");
            verify(myObservableMap.hasKey("addEventListener"), false, 'myObservableMap.hasKey("addEventListener")');
            verify(typeof myObservableMap["addEventListener"], "function", 'typeof myObservableMap["addEventListener"]');
            verify(myObservableMap["addEventListener"].toString(), "function addEventListener() { [native code] }", 'myObservableMap["addEventListener"].toString()');

            myObservableMap.remove("onmapchanged");
            verify(myObservableMap.hasKey("onmapchanged"), false, 'myObservableMap.hasKey("onmapchanged")');
            verify(typeof myObservableMap["onmapchanged"], "object", 'typeof myObservableMap["onmapchanged"]');
            verify(myObservableMap["onmapchanged"], null, 'myObservableMap["onmapchanged"]');

            verifyMapContents("myObservableMap", myObservableMap, [["Hundred", 100], ["Twenty", 20], ["Five", 5]]);
            verifyMapEnumerator("myObservableMap", myObservableMap, myObservableMapMembers, [["Hundred", 100], ["Twenty", 20], ["Five", 5]]);
        }
    });

    runner.addTest({
        id: 19,
        desc: 'IObservableMap : IObservable methods and [] mixing',
        pri: '0',
        test: function () {
            var myObservableMap = Animals.Animal.getObservableStringIntegerMap();

            var expectedCollectionChange;
            var expectedKey;
            var expectedContents;
            var eventCount = 0;
            function mapChangedEvent(ev) {
                logger.comment("*** mapChanegdEvent : start ***");

                verify(ev.type, "mapchanged", "ev.type");
                verify(ev.target, myObservableMap, "ev.target");

                verify(ev.collectionChange, expectedCollectionChange, "ev.collectionChange");
                verify(ev.key, expectedKey, "ev.key");

                verifyMapContents("ev.target", ev.target, expectedContents);

                eventCount++;

                logger.comment("*** mapChanegdEvent : end ***");
            }

            logger.comment("** Set event handler");
            myObservableMap.onmapchanged = mapChangedEvent;
            verify(myObservableMap.onmapchanged, mapChangedEvent, "myObservableMap.onmapchanged");

            logger.comment("** Set property using [] operation");
            expectedKey = "Thirty";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemInserted;
            expectedContents = [["Hundred", 100], ["Twenty", 20], ["Thirty", 330], ["Five", 5]];
            myObservableMap["Thirty"] = 330;
            verify(eventCount, 1, "Event Count");
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);

            logger.comment("** Set property using insert method");
            expectedKey = "One";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemInserted;
            expectedContents = [["Hundred", 100], ["One", 11], ["Twenty", 20], ["Thirty", 330], ["Five", 5]];
            eventCount = 0;
            var replaced = myObservableMap.insert("One", 11);
            verify(replaced, false, 'myObservableMap.insert("One", 11)');
            verify(eventCount, 1, "Event Count");
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);

            logger.comment("** Change property using [] operation");
            expectedKey = "One";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemChanged;
            expectedContents = [["Hundred", 100], ["One", 1], ["Twenty", 20], ["Thirty", 330], ["Five", 5]];
            eventCount = 0;
            myObservableMap["One"] = 1;
            verify(eventCount, 1, "Event Count");
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);

            logger.comment("** Change property using insert method");
            expectedKey = "Thirty";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemChanged;
            expectedContents = [["Hundred", 100], ["One", 1], ["Twenty", 20], ["Thirty", 30], ["Five", 5]];
            eventCount = 0;
            replaced = myObservableMap.insert("Thirty", 30);
            verify(replaced, true, 'myObservableMap.insert("Thirty", 30)');
            verify(eventCount, 1, "Event Count");
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);

            logger.comment("** Remove property using delete operation");
            expectedKey = "Thirty";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemRemoved;
            expectedContents = [["Hundred", 100], ["One", 1], ["Twenty", 20], ["Five", 5]];
            eventCount = 0;
            delete myObservableMap["Thirty"];
            verify(eventCount, 1, "Event Count");
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);

            logger.comment("** Remove property using remove method");
            expectedKey = "One";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemRemoved;
            expectedContents = [["Hundred", 100], ["Twenty", 20], ["Five", 5]];
            eventCount = 0;
            myObservableMap.remove("One");
            verify(eventCount, 1, "Event Count");
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);

            logger.comment("** Setting key insert using [] operation");
            eventCount = 0;
            myObservableMap["insert"] = "Insert Property"
            verify(eventCount, 0, "Event Count");
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);

            logger.comment("** Setting key insert using insert method");
            expectedKey = "insert";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemInserted;
            expectedContents = [["Hundred", 100], ["Twenty", 20], ["Five", 5], ["insert", 55]];
            eventCount = 0;
            replaced = myObservableMap.insert("insert", 55);
            verify(replaced, false, 'myObservableMap.insert("insert", 55)');
            verify(eventCount, 1, "Event Count");
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);

            logger.comment("** Clear all properties using clear method");
            expectedKey = "";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.reset;
            expectedContents = [];
            eventCount = 0;
            myObservableMap.clear();
            verify(eventCount, 1, "Event Count");
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);

            logger.comment("** remove event handler");
            myObservableMap.onmapchanged = null;
            verify(myObservableMap.onmapchanged, null, "myObservableMap.onmapchanged");

            logger.comment("** Set new property using [] operation");
            eventCount = 0;
            myObservableMap["Two"] = 2;
            verify(eventCount, 0, "Event Count");
            expectedContents = [["Two", 2]];
            verifyMapContents("myObservableMap", myObservableMap, expectedContents);
        }
    });

    runner.addTest({
        id: 20,
        desc: 'IMap : HString Key and WebHostHidden Value',
        pri: '0',
        test: function () {
            verify.exception(function () {
                var result = Animals.Animal.getStringHiddenTypeMap();
            }, TypeError, "Out param with IMap<HSTRING, HiddenType>");
        }
    });

    runner.addTest({
        id: 21,
        desc: 'RCStringMap : Runtimeclass with IMap<HSTRING, int>',
        pri: '0',
        test: function () {
            var myMap = new Animals.RCStringMap();
            verifyMembers("myMap", myMap, myMapWithPropertiesMembers);
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]]);

            logger.comment("*** Setting property using []");
            myMap["Twenty"] = 6;
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17]]);

            logger.comment("*** Deleting the property using delete operator");
            delete myMap["by"];
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17]]);

            logger.comment("*** Enumerator using for");
            verifyMapEnumerator("myMap", myMap, myMapMembers, [["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17]]);
        }
    });

    runner.addTest({
        id: 22,
        desc: 'RCStringMapWithIterable : Runtimeclass with IMap<HSTRING, int> and Iterable<HSTRING>',
        pri: '0',
        test: function () {
            var myMap = new Animals.RCStringMapWithIterable();
            verifyMembers("myMap", myMap, myRCStringMapWithIterableWithPropertiesMembers);
            var firstPropertyName = "Windows.Foundation.Collections.IIterable`1<Windows.Foundation.Collections.IKeyValuePair`2<String,Int32>>.first";
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]], firstPropertyName);

            logger.comment("*** Setting property using []");
            myMap["Twenty"] = 6;
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17]], firstPropertyName);

            logger.comment("*** Deleting the property using delete operator");
            delete myMap["by"];
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17]], firstPropertyName);

            logger.comment("*** Enumerator using for");
            verifyMapEnumerator("myMap", myMap, myRCStringMapWithIterableMembers, [["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17]]);

            logger.comment("*** verify iterable First method can be called too");
            var myIterator = myMap['Windows.Foundation.Collections.IIterable`1<String>.first']();
            var days = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"];
            var index = 0;
            while (myIterator.hasCurrent && index < days.length) {
                verify(myIterator.current, days[index], "(" + index + "): myIterator");
                myIterator.moveNext();
                index++;
            }

            verify(!myIterator.hasCurrent, true, "AllIterator members read");
            verify(index, days.length, "All expected members found");
        }
    });

    runner.addTest({
        id: 23,
        desc: 'RCStringMapWithDefaultIterable : Runtimeclass with IMap<HSTRING, int> and default interface as Iterable<HSTRING>',
        pri: '0',
        test: function () {
            var myMap = new Animals.RCStringMapWithDefaultIterable();
            verifyMembers("myMap", myMap, myRCStringMapWithIterableWithPropertiesMembers);
            var firstPropertyName = "Windows.Foundation.Collections.IIterable`1<Windows.Foundation.Collections.IKeyValuePair`2<String,Int32>>.first";
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Hundred And Fifty", 17]], firstPropertyName);

            logger.comment("*** Setting property using []");
            myMap["Twenty"] = 6;
            verifyMapContents("myMap", myMap, [["by", 2], ["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17]], firstPropertyName);

            logger.comment("*** Deleting the property using delete operator");
            delete myMap["by"];
            verifyMapContents("myMap", myMap, [["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17]], firstPropertyName);

            logger.comment("*** Enumerator using for");
            verifyMapEnumerator("myMap", myMap, myRCStringMapWithIterableMembers, [["Hundred", 7], ["Twenty", 6], ["Hundred And Fifty", 17]]);

            logger.comment("*** verify iterable First method can be called too");
            var myIterator = myMap['Windows.Foundation.Collections.IIterable`1<String>.first']();
            var days = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"];
            var index = 0;
            while (myIterator.hasCurrent && index < days.length) {
                verify(myIterator.current, days[index], "(" + index + "): myIterator");
                myIterator.moveNext();
                index++;
            }

            verify(!myIterator.hasCurrent, true, "AllIterator members read");
            verify(index, days.length, "All expected members found");
        }
    });

    runner.addTest({
        id: 24,
        desc: 'RCIDoubleObservableMap',
        pri: '0',
        test: function () {
            var myDoubleObservableMap = new Animals.RCIDoubleObservableMap();
            verifyMembers("myDoubleObservableMap", myDoubleObservableMap, myRCIDoubleObservableMapMembers);

            // Verify Contents
            function forEachGUIDAnimalEntryMethod(x) {
                verify(myDoubleObservableMap['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.lookup'](x.key).getGreeting(), x.value, "myDoubleObservableMap['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.lookup'](" + x.key + ").getGreeting()");
            }
            var myArray = [
                { key: "8eb82cb5-03d6-49d2-80ee-8583e949b5bf", value: "Animal1" },
                { key: "b960a7ac-f275-43fc-b154-91e7adeee7aa", value: "Animal2" },
                { key: "9f1af037-baf9-473b-b8c3-183ab3f5b3ce", value: "Animal3" }
                ];
            myArray.forEach(forEachGUIDAnimalEntryMethod);

            function forEachStringIntEntryMethod(x) {
                verify(myDoubleObservableMap['Windows.Foundation.Collections.IMap`2<String,Int32>.lookup'](x.key), x.value, "myDoubleObservableMap['Windows.Foundation.Collections.IMap`2<String,Int32>.lookup'](" + x.key + ")");
            }
            myArray = [
                 { key: "by", value: 2 },
                 { key: "Hundred", value: 7 },
                 { key: "Hundred And Fifty", value: 17 }
                 ];
            myArray.forEach(forEachStringIntEntryMethod);
        }
    });

    runner.addTest({
        id: 25,
        desc: 'IDoubleIObservableMap - projected as interface type',
        pri: '0',
        test: function () {
            var myDoubleObservableMap = Animals.Animal.getDoubleObservableMap();
            verifyMembers("myDoubleObservableMap", myDoubleObservableMap, myRCIDoubleObservableMapMembers);

            // Verify Contents
            function forEachGUIDAnimalEntryMethod(x) {
                verify(myDoubleObservableMap['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.lookup'](x.key).getGreeting(), x.value, "myDoubleObservableMap['Windows.Foundation.Collections.IMap`2<System.Guid,Object>.lookup'](" + x.key + ").getGreeting()");
            }
            var myArray = [
                { key: "8eb82cb5-03d6-49d2-80ee-8583e949b5bf", value: "Animal1" },
                { key: "b960a7ac-f275-43fc-b154-91e7adeee7aa", value: "Animal2" },
                { key: "9f1af037-baf9-473b-b8c3-183ab3f5b3ce", value: "Animal3" }
                ];
            myArray.forEach(forEachGUIDAnimalEntryMethod);

            function forEachStringIntEntryMethod(x) {
                verify(myDoubleObservableMap['Windows.Foundation.Collections.IMap`2<String,Int32>.lookup'](x.key), x.value, "myDoubleObservableMap['Windows.Foundation.Collections.IMap`2<String,Int32>.lookup'](" + x.key + ")");
            }
            myArray = [
                 { key: "by", value: 2 },
                 { key: "Hundred", value: 7 },
                 { key: "Hundred And Fifty", value: 17 }
                 ];
            myArray.forEach(forEachStringIntEntryMethod);
        }
    });

    Loader42_FileName = 'IMap Tests'
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
