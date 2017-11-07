if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    var myPropertySetMembers = [
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

    function verifyPropertyContents(myPropertySetString, myPropertySet, arrayContents) {
        logger.comment("Verifying contents of : " + myPropertySetString);
        verify(myPropertySet.size, arrayContents.length, myPropertySetString + ".size");
        var myIterator = myPropertySet.first();
        var index = 0;
        while (myIterator.hasCurrent && index < arrayContents.length) {
            verify(myIterator.current.key, arrayContents[index][0], "Key (" + index + "): " + myPropertySetString);
            verify(myIterator.current.value, arrayContents[index][1], "Value (" + index + "): " + myPropertySetString);
            myIterator.moveNext();
            index++;
        }

        verify(!myIterator.hasCurrent, true, "AllIterator members read");
        verify(index, arrayContents.length, "All expected members found");
    }

    function verifyPropertySetEnumerator(myPropertySetString, myPropertySet, prototypeMembers, propertySetContents) {
        logger.comment("Verifying enumerator of : " + myPropertySetString);
        var indexInPrototypeMembers = 0;
        var indexInPropertySetContent = 0;
        if (propertySetContents === undefined) {
            propertySetContents = [];
        }

        var propertyIndex = 0;
        for (p in myPropertySet) {
            if (indexInPropertySetContent < propertySetContents.length) {
                verify(p, propertySetContents[indexInPropertySetContent][0], myPropertySetString + '\'s property: ' + propertyIndex);
                verify(typeof myPropertySet[p], typeof propertySetContents[indexInPropertySetContent][1], 'typeof ' + myPropertySetString + '["' + p + '"]');
                verify(myPropertySet[p], propertySetContents[indexInPropertySetContent][1], myPropertySetString + '["' + p + '"]');
                indexInPropertySetContent++;
            }
            else {
                // In prototype
                verify(p, prototypeMembers[indexInPrototypeMembers][0], myPropertySetString + '\'s property: ' + propertyIndex);
                verify(typeof myPropertySet[p], prototypeMembers[indexInPrototypeMembers][1], "typeof" + myPropertySetString + '["' + p + '"]');
                indexInPrototypeMembers++;
            }
            propertyIndex++;
        }

        verify(indexInPropertySetContent, propertySetContents.length, "All propertySet contents read?");
        verify(indexInPrototypeMembers, prototypeMembers.length, "All prototype members read?");
    }

    runner.addTest({
        id: 1,
        desc: 'MarshalOut : Empty PropertySet',
        pri: '0',
        test: function () {
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();
            verifyMembers("myPropertySet", myPropertySet, myPropertySetMembers);
            verifyPropertyContents("myPropertySet", myPropertySet, []);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'MarshalOut : Set the properties',
        pri: '0',
        test: function () {
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();

            myPropertySet["name"] = "TRex";
            verify(myPropertySet["name"], "TRex", 'myPropertySet["name"]');
            myPropertySet["Type"] = Animals.Phylum.entoprocta;
            verify(myPropertySet["Type"], Animals.Phylum.entoprocta, 'myPropertySet["Type"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["name", "TRex"], ["Type", Animals.Phylum.entoprocta]]);

            myPropertySet["name"] = "Nemo";
            verify(myPropertySet["name"], "Nemo", 'myPropertySet["name"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["name", "Nemo"], ["Type", Animals.Phylum.entoprocta]]);

            myPropertySet[0] = 70;
            verify(myPropertySet[0], 70, 'myPropertySet[0]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["0", 70], ["name", "Nemo"], ["Type", Animals.Phylum.entoprocta]]);

            myPropertySet["4"] = "AnotherProperty";
            verify(myPropertySet["4"], "AnotherProperty", 'myPropertySet["4"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["0", 70], ["4", "AnotherProperty"], ["name", "Nemo"], ["Type", Animals.Phylum.entoprocta]]);

            myPropertySet["Type"] = null;
            verify(myPropertySet["Type"], null, 'myPropertySet["Type"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["0", 70], ["4", "AnotherProperty"], ["name", "Nemo"], ["Type", null]]);

            verify(myPropertySet["undefinedProperty"], undefined, 'myPropertySet["undefinedProperty"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["0", 70], ["4", "AnotherProperty"], ["name", "Nemo"], ["Type", null]]);

            myPropertySet["Type"] = undefined;
            verify(myPropertySet["Type"], null, 'myPropertySet["Type"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["0", 70], ["4", "AnotherProperty"], ["name", "Nemo"], ["Type", null]]);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'MarshalOut : Delete the properties',
        pri: '0',
        test: function () {
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();

            myPropertySet["name"] = "TRex";
            verify(myPropertySet["name"], "TRex", 'myPropertySet["name"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["name", "TRex"]]);

            delete myPropertySet["name"];
            verify(myPropertySet["name"], undefined, 'myPropertySet["name"]');
            verifyPropertyContents("myPropertySet", myPropertySet, []);

            myPropertySet[0] = 70;
            verify(myPropertySet[0], 70, 'myPropertySet[0]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["0", 70]]);

            delete myPropertySet[0];
            verify(myPropertySet[0], undefined, 'myPropertySet[0]');
            verifyPropertyContents("myPropertySet", myPropertySet, []);

            myPropertySet["4"] = "AnotherProperty";
            verify(myPropertySet["4"], "AnotherProperty", 'myPropertySet["4"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["4", "AnotherProperty"]]);

            delete myPropertySet["4"];
            verify(myPropertySet["4"], undefined, 'myPropertySet["4"]');
            verifyPropertyContents("myPropertySet", myPropertySet, []);

            var n = delete myPropertySet["undefinedProperty"];
            verify(n, false, 'delete myPropertySet["undefinedProperty"]');
            verify(myPropertySet["undefinedProperty"], undefined, 'myPropertySet["undefinedProperty"]');
            verifyPropertyContents("myPropertySet", myPropertySet, []);

            n = delete myPropertySet[75];
            verify(n, false, 'delete myPropertySet[75]');
            verify(myPropertySet[75], undefined, 'myPropertySet[75]');
            verifyPropertyContents("myPropertySet", myPropertySet, []);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'MarshalOut : Enumerator',
        pri: '0',
        test: function () {
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetMembers);

            logger.comment("Add some properties and then verify enumerator");
            myPropertySet["name"] = "Nemo";
            myPropertySet[0] = 70;
            myPropertySet["4"] = "AnotherProperty";
            myPropertySet["Type"] = null;
            var expectedContents = [["0", 70], ["4", "AnotherProperty"], ["name", "Nemo"], ["Type", null]];
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetMembers, expectedContents);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'MarshalOut : Modifying values in Enumerator',
        pri: '0',
        test: function () {
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();
            myPropertySet["Name"] = "nemo"
            myPropertySet["Type"] = null;
            myPropertySet["FearForSwimming"] = false;
            myPropertySet["father"] = "Marlin"
            myPropertySet["Alive"] = true;

            var expectedContents = [["father", "Marlin"], ["Alive", true], ["Name", "nemo"], ["FearForSwimming", false], ["Type", null]];
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetMembers, expectedContents);

            expectedContents = [["father", "Marlin"], ["Alive", true], ["Name", "nemo"], ["Type", "Fish"]];
            var indexInPrototypeMembers = 0;
            var indexInPropertySetContent = 0;
            var propertyIndex = 0;
            var myPropertySetString = "myPropertySet";
            var prototypeMembers = myPropertySetMembers;
            for (p in myPropertySet) {
                if (indexInPropertySetContent < expectedContents.length) {
                    verify(p, expectedContents[indexInPropertySetContent][0], '* ' + myPropertySetString + '\'s property: ' + propertyIndex);
                    verify(typeof myPropertySet[p], typeof expectedContents[indexInPropertySetContent][1], '* typeof ' + myPropertySetString + '["' + p + '"]');
                    verify(myPropertySet[p], expectedContents[indexInPropertySetContent][1], '* ' + myPropertySetString + '["' + p + '"]');
                    indexInPropertySetContent++;
                }
                else {
                    // In prototype
                    verify(p, prototypeMembers[indexInPrototypeMembers][0], '* ' + myPropertySetString + '\'s property: ' + propertyIndex);
                    verify(typeof myPropertySet[p], prototypeMembers[indexInPrototypeMembers][1], "* typeof" + myPropertySetString + '["' + p + '"]');
                    indexInPrototypeMembers++;
                }

                if (myPropertySet[p] == "nemo") {
                    logger.comment("*** Modify the properties while enumerating : Start ***");

                    myPropertySet["Name"] = "Nemo";
                    myPropertySet["Type"] = "Fish";

                    myPropertySet["Adventurous"] = true;
                    myPropertySet["NewToSwimming"] = false;

                    delete myPropertySet["father"];
                    delete myPropertySet["FearForSwimming"];

                    verify(myPropertySet["Name"], "Nemo", 'myPropertySet["Name"]');
                    verify(myPropertySet["Type"], "Fish", 'myPropertySet["Type"]');

                    verify(myPropertySet["Adventurous"], true, 'myPropertySet["Adventurous"]');
                    verify(myPropertySet["NewToSwimming"], false, 'myPropertySet["NewToSwimming"]');

                    verify(myPropertySet["father"], undefined, 'myPropertySet["father"]');
                    verify(myPropertySet["FearForSwimming"], undefined, 'myPropertySet["FearForSwimming"]');
                    logger.comment("*** Modify the properties while enumerating : End ***");
                }

                propertyIndex++;
            }

            verify(indexInPropertySetContent, expectedContents.length, "* All propertySet contents read?");
            verify(indexInPrototypeMembers, prototypeMembers.length, "* All prototype members read?");

            expectedContents = [["Adventurous", true], ["Alive", true], ["Name", "Nemo"], ["NewToSwimming", false], ["Type", "Fish"]];
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetMembers, expectedContents);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'MarshalOut : Name conflicting properties : 0, 32 and insert',
        pri: '0',
        test: function () {
            var myPropertySetWithObjectExpandoMembers = [
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
                ['0', 'string'],
                ['32', 'string'],
            ];

            logger.comment("*** Setup prototype for conflicting properties");
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();
            Object.prototype["0"] = "0 Property";
            Object.prototype["32"] = "32 Property";
            verifyMembers("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers);

            logger.comment("*** Set conflicting properties using index operation");
            myPropertySet["insert"] = "TRex"; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myPropertySet.hasKey("insert"), false, 'myPropertySet.hasKey("insert")');
            verify(typeof myPropertySet["insert"], "function", 'typeof myPropertySet["insert"]');
            verify(myPropertySet["insert"].toString(), "\nfunction insert() {\n    [native code]\n}\n", 'myPropertySet["insert"].toString()');

            myPropertySet["0"] = "TRex0"; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myPropertySet.hasKey("0"), false, 'myPropertySet.hasKey("0")');
            verify(myPropertySet["0"], "0 Property", 'myPropertySet["0"]');

            myPropertySet["32"] = "TRex32"; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myPropertySet.hasKey("32"), false, 'myPropertySet.hasKey("32")');
            verify(myPropertySet["32"], "32 Property", 'myPropertySet["32"]');

            verifyPropertyContents("myPropertySet", myPropertySet, []);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers);

            logger.comment("*** Add propertySet members using .insert method");
            myPropertySet.insert("insert", "TRex");
            verify(myPropertySet.hasKey("insert"), true, 'myPropertySet.hasKey("insert")');
            verify(myPropertySet.lookup("insert"), "TRex", 'myPropertySet.lookup("insert")');
            verify(typeof myPropertySet["insert"], "function", 'typeof myPropertySet["insert"]');
            verify(myPropertySet["insert"].toString(), "\nfunction insert() {\n    [native code]\n}\n", 'myPropertySet["insert"].toString()');

            myPropertySet.insert("0", "TRex0");
            verify(myPropertySet.hasKey("0"), true, 'myPropertySet.hasKey("0")');
            verify(myPropertySet.lookup("0"), "TRex0", 'myPropertySet.lookup("0")');
            verify(myPropertySet["0"], "0 Property", 'myPropertySet["0"]');

            myPropertySet.insert("32", "TRex32");
            verify(myPropertySet.hasKey("32"), true, 'myPropertySet.hasKey("32")');
            verify(myPropertySet.lookup("32"), "TRex32", 'myPropertySet.lookup("32")');
            verify(myPropertySet["32"], "32 Property", 'myPropertySet["32"]');

            verifyPropertyContents("myPropertySet", myPropertySet, [["0", "TRex0"], ["32", "TRex32"], ["insert", "TRex"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers);

            logger.comment("*** Delete the conflicting properties - shouldnt change anything");
            delete myPropertySet["insert"];
            verify(myPropertySet.hasKey("insert"), true, 'myPropertySet.hasKey("insert")');
            verify(myPropertySet.lookup("insert"), "TRex", 'myPropertySet.lookup("insert")');
            verify(typeof myPropertySet["insert"], "function", 'typeof myPropertySet["insert"]');
            verify(myPropertySet["insert"].toString(), "\nfunction insert() {\n    [native code]\n}\n", 'myPropertySet["insert"].toString()');

            delete myPropertySet["0"]
            verify(myPropertySet.hasKey("0"), true, 'myPropertySet.hasKey("0")');
            verify(myPropertySet.lookup("0"), "TRex0", 'myPropertySet.lookup("0")');
            verify(myPropertySet["0"], "0 Property", 'myPropertySet["0"]');

            delete myPropertySet["32"]
            verify(myPropertySet.hasKey("32"), true, 'myPropertySet.hasKey("32")');
            verify(myPropertySet.lookup("32"), "TRex32", 'myPropertySet.lookup("32")');
            verify(myPropertySet["32"], "32 Property", 'myPropertySet["32"]');

            verifyPropertyContents("myPropertySet", myPropertySet, [["0", "TRex0"], ["32", "TRex32"], ["insert", "TRex"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers);

            logger.comment("*** Set floating point properties");
            myPropertySet["0.0"] = "TRex0.0"
            verify(myPropertySet.hasKey("0.0"), true, 'myPropertySet.hasKey("0.0")');
            verify(myPropertySet.lookup("0.0"), "TRex0.0", 'myPropertySet.lookup("0.0")');
            verify(myPropertySet["0.0"], "TRex0.0", 'myPropertySet["0.0"]');

            myPropertySet["32.56"] = "TRex32.56"
            verify(myPropertySet.hasKey("32.56"), true, 'myPropertySet.hasKey("32.56")');
            verify(myPropertySet.lookup("32.56"), "TRex32.56", 'myPropertySet.lookup("32.56")');
            verify(myPropertySet["32.56"], "TRex32.56", 'myPropertySet["32.56"]');

            verifyPropertyContents("myPropertySet", myPropertySet, [["0.0", "TRex0.0"], ["32.56", "TRex32.56"], ["0", "TRex0"], ["32", "TRex32"], ["insert", "TRex"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers, [["0.0", "TRex0.0"], ["32.56", "TRex32.56"]]);

            logger.comment("*** Delete the object.prototype properties");
            delete Object.prototype["0"];
            delete Object.prototype["32"];

            verify(myPropertySet["0"], "TRex0", 'myPropertySet["0"]');
            verify(myPropertySet["32"], "TRex32", 'myPropertySet["32"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["0.0", "TRex0.0"], ["32.56", "TRex32.56"], ["0", "TRex0"], ["32", "TRex32"], ["insert", "TRex"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetMembers, [["0.0", "TRex0.0"], ["32.56", "TRex32.56"], ["0", "TRex0"], ["32", "TRex32"]]);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'MarshalOut : Name conflicting properties : 0.0, 32.14, 38.1, 38',
        pri: '0',
        test: function () {
            var myPropertySetWithObjectExpandoMembers = [
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
                ['38', 'string'],
                ['0.0', 'string'],
                ['32.14', 'string'],
                ['38.1', 'string'],
            ];

            logger.comment("*** Setup prototype for conflicting properties");
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();
            Object.prototype["0.0"] = "0.0 Property";
            Object.prototype["32.14"] = "32.14 Property";
            Object.prototype["38.1"] = "38.1 Property";
            Object.prototype["38"] = "38 Property";
            verifyMembers("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers);

            logger.comment("*** Set conflicting properties using index operation");
            myPropertySet["0.0"] = "TRex0.0"; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myPropertySet.hasKey("0.0"), false, 'myPropertySet.hasKey("0.0")');
            verify(myPropertySet["0.0"], "0.0 Property", 'myPropertySet["0.0"]');

            myPropertySet["32.14"] = "TRex32.14"; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myPropertySet.hasKey("32.14"), false, 'myPropertySet.hasKey("32.14")');
            verify(myPropertySet["32.14"], "32.14 Property", 'myPropertySet["32.14"]');

            myPropertySet["38.1"] = "TRex38.1"; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myPropertySet.hasKey("38.1"), false, 'myPropertySet.hasKey("38.1")');
            verify(myPropertySet["38.1"], "38.1 Property", 'myPropertySet["38.1"]');

            myPropertySet["38"] = "TRex38"; // this should fail since from java script one cannot write property on object if prototype property is non writable
            verify(myPropertySet.hasKey("38"), false, 'myPropertySet.hasKey("38")');
            verify(myPropertySet["38"], "38 Property", 'myPropertySet["38"]');

            verifyPropertyContents("myPropertySet", myPropertySet, []);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers);

            logger.comment("*** Add propertySet members using .insert method");
            myPropertySet.insert("0.0", "TRex0.0");
            verify(myPropertySet.hasKey("0.0"), true, 'myPropertySet.hasKey("0.0")');
            verify(myPropertySet.lookup("0.0"), "TRex0.0", 'myPropertySet.lookup("0.0")');
            verify(myPropertySet["0.0"], "0.0 Property", 'myPropertySet["0.0"]');

            myPropertySet.insert("32.14", "TRex32.14");
            verify(myPropertySet.hasKey("32.14"), true, 'myPropertySet.hasKey("32.14")');
            verify(myPropertySet.lookup("32.14"), "TRex32.14", 'myPropertySet.lookup("32.14")');
            verify(myPropertySet["32.14"], "32.14 Property", 'myPropertySet["32.14"]');

            myPropertySet.insert("38.1", "TRex38.1");
            verify(myPropertySet.hasKey("38.1"), true, 'myPropertySet.hasKey("38.1")');
            verify(myPropertySet.lookup("38.1"), "TRex38.1", 'myPropertySet.lookup("38.1")');
            verify(myPropertySet["38.1"], "38.1 Property", 'myPropertySet["38.1"]');

            myPropertySet.insert("38", "TRex38");
            verify(myPropertySet.hasKey("38"), true, 'myPropertySet.hasKey("38")');
            verify(myPropertySet.lookup("38"), "TRex38", 'myPropertySet.lookup("38")');
            verify(myPropertySet["38"], "38 Property", 'myPropertySet["38"]');

            verifyPropertyContents("myPropertySet", myPropertySet, [["38.1", "TRex38.1"], ["0.0", "TRex0.0"], ["38", "TRex38"], ["32.14", "TRex32.14"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers);

            logger.comment("*** Delete the conflicting properties - shouldnt change anything");
            delete myPropertySet["0.0"]
            verify(myPropertySet.hasKey("0.0"), true, 'myPropertySet.hasKey("0.0")');
            verify(myPropertySet.lookup("0.0"), "TRex0.0", 'myPropertySet.lookup("0.0")');
            verify(myPropertySet["0.0"], "0.0 Property", 'myPropertySet["0.0"]');

            delete myPropertySet["32.14"]
            verify(myPropertySet.hasKey("32.14"), true, 'myPropertySet.hasKey("32.14")');
            verify(myPropertySet.lookup("32.14"), "TRex32.14", 'myPropertySet.lookup("32.14")');
            verify(myPropertySet["32.14"], "32.14 Property", 'myPropertySet["32.14"]');

            delete myPropertySet["38.1"]
            verify(myPropertySet.hasKey("38.1"), true, 'myPropertySet.hasKey("38.1")');
            verify(myPropertySet.lookup("38.1"), "TRex38.1", 'myPropertySet.lookup("38.1")');
            verify(myPropertySet["38.1"], "38.1 Property", 'myPropertySet["38.1"]');

            delete myPropertySet["38"]
            verify(myPropertySet.hasKey("38"), true, 'myPropertySet.hasKey("38")');
            verify(myPropertySet.lookup("38"), "TRex38", 'myPropertySet.lookup("38")');
            verify(myPropertySet["38"], "38 Property", 'myPropertySet["38"]');

            verifyPropertyContents("myPropertySet", myPropertySet, [["38.1", "TRex38.1"], ["0.0", "TRex0.0"], ["38", "TRex38"], ["32.14", "TRex32.14"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers);

            logger.comment("*** Set floating point properties");
            myPropertySet["0"] = "TRex0"
            verify(myPropertySet.hasKey("0"), true, 'myPropertySet.hasKey("0")');
            verify(myPropertySet.lookup("0"), "TRex0", 'myPropertySet.lookup("0")');
            verify(myPropertySet["0"], "TRex0", 'myPropertySet["0"]');

            myPropertySet["32"] = "TRex32"
            verify(myPropertySet.hasKey("32"), true, 'myPropertySet.hasKey("32")');
            verify(myPropertySet.lookup("32"), "TRex32", 'myPropertySet.lookup("32")');
            verify(myPropertySet["32"], "TRex32", 'myPropertySet["32"]');

            verifyPropertyContents("myPropertySet", myPropertySet, [["38.1", "TRex38.1"], ["0.0", "TRex0.0"], ["0", "TRex0"], ["38", "TRex38"], ["32", "TRex32"], ["32.14", "TRex32.14"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetWithObjectExpandoMembers, [["0", "TRex0"], ["32", "TRex32"]]);

            logger.comment("*** Delete the object.prototype properties");
            delete Object.prototype["0.0"];
            delete Object.prototype["32.14"];
            delete Object.prototype["38.1"];
            delete Object.prototype["38"];

            verify(myPropertySet["0.0"], "TRex0.0", 'myPropertySet["0.0"]');
            verify(myPropertySet["32.14"], "TRex32.14", 'myPropertySet["32.14"]');
            verify(myPropertySet["38"], "TRex38", 'myPropertySet["38"]');
            verify(myPropertySet["38.1"], "TRex38.1", 'myPropertySet["38.1"]');
            verifyPropertyContents("myPropertySet", myPropertySet, [["38.1", "TRex38.1"], ["0.0", "TRex0.0"], ["0", "TRex0"], ["38", "TRex38"], ["32", "TRex32"], ["32.14", "TRex32.14"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetMembers, [["38.1", "TRex38.1"], ["0.0", "TRex0.0"], ["0", "TRex0"], ["38", "TRex38"], ["32", "TRex32"], ["32.14", "TRex32.14"]]);
        }
    });

    runner.addTest({
        id: 8,
        desc: 'MarshalOut : Name conflicting properties onmapchanged',
        pri: '0',
        test: function () {
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();

            logger.comment("*** Set conflicting properties using index operation");
            myPropertySet["onmapchanged"] = "TRex"; // this shouldnt change the onmapchanged
            verify(myPropertySet.hasKey("onmapchanged"), false, 'myPropertySet.hasKey("onmapchanged")');
            verify(myPropertySet["onmapchanged"], null, 'myPropertySet["onmapchanged"]');

            verifyPropertyContents("myPropertySet", myPropertySet, []);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetMembers);

            logger.comment("*** Add propertySet members using .insert method");
            myPropertySet.insert("onmapchanged", "TRex");
            verify(myPropertySet.hasKey("onmapchanged"), true, 'myPropertySet.hasKey("onmapchanged")');
            verify(myPropertySet.lookup("onmapchanged"), "TRex", 'myPropertySet.lookup("onmapchanged")');
            verify(myPropertySet["onmapchanged"], null, 'myPropertySet["onmapchanged"]');

            verifyPropertyContents("myPropertySet", myPropertySet, [["onmapchanged", "TRex"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetMembers);

            logger.comment("*** Delete the conflicting properties - shouldnt change anything");
            delete myPropertySet["onmapchanged"];
            verify(myPropertySet.hasKey("onmapchanged"), true, 'myPropertySet.hasKey("onmapchanged")');
            verify(myPropertySet.lookup("onmapchanged"), "TRex", 'myPropertySet.lookup("onmapchanged")');
            verify(myPropertySet["onmapchanged"], null, 'myPropertySet["onmapchanged"]');

            verifyPropertyContents("myPropertySet", myPropertySet, [["onmapchanged", "TRex"]]);
            verifyPropertySetEnumerator("myPropertySet", myPropertySet, myPropertySetMembers);
        }
    });

    runner.addTest({
        id: 9,
        desc: 'MarshalIn : PropertySet',
        pri: '0',
        test: function () {
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();

            var samePropertySet = Animals.Animal.sendBackSamePropertySet(myPropertySet);
            verify(samePropertySet, myPropertySet, "Animals.Animal.SendBackSamePropertySet(myPropertySet)");
            verifyMembers("myPropertySet", myPropertySet, myPropertySetMembers);

            var myObject = {
                "firstProperty": 20,
                "name": "FirstProperty"
            };
            verify.exception(function () {
                var sameObject = Animals.Animal.sendBackSamePropertySet(myObject);
            }, TypeError, "Marshaling object as PropertySet");
        }
    });

    runner.addTest({
        id: 10,
        desc: 'MarshalOut : PropertySet IMap methods and [] mixing',
        pri: '0',
        test: function () {
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();
            logger.comment("*** Set property using [] operation");
            myPropertySet["prop1"] = "Property 1";
            myPropertySet["prop2"] = "Property 2";
            myPropertySet["prop3"] = "Property 3";
            verifyPropertyContents("myPropertySet", myPropertySet, [["prop2", "Property 2"], ["prop1", "Property 1"], ["prop3", "Property 3"]]);

            logger.comment("*** Add property override using insert method");
            var replaced = myPropertySet.insert("prop2", "Override Property 2");
            verify(replaced, true, 'myPropertySet.insert("prop2", "Override property 2")');
            verifyPropertyContents("myPropertySet", myPropertySet, [["prop2", "Override Property 2"], ["prop1", "Property 1"], ["prop3", "Property 3"]]);

            logger.comment("*** Remove property using remove method");
            myPropertySet.remove("prop1");
            verifyPropertyContents("myPropertySet", myPropertySet, [["prop2", "Override Property 2"], ["prop3", "Property 3"]]);

            logger.comment("*** Clear all properties using clear method");
            myPropertySet.clear();
            verifyPropertyContents("myPropertySet", myPropertySet, []);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'MarshalOut : PropertySet IObservable methods and [] mixing',
        pri: '0',
        test: function () {
            var myPropertySet = new Windows.Foundation.Collections.PropertySet();
            var expectedCollectionChange;
            var expectedKey;
            var expectedContents;
            var eventCount = 0;
            function mapChangedEvent(ev) {
                logger.comment("*** mapChanegdEvent : start ***");

                verify(ev.type, "mapchanged", "ev.type");
                verify(ev.target, myPropertySet, "ev.target");

                verify(ev.collectionChange, expectedCollectionChange, "ev.collectionChange");
                verify(ev.key, expectedKey, "ev.key");

                verifyPropertyContents("ev.target", ev.target, expectedContents);

                eventCount++;

                logger.comment("*** mapChanegdEvent : end ***");
            }

            logger.comment("** Set event handler");
            myPropertySet.onmapchanged = mapChangedEvent;
            verify(myPropertySet.onmapchanged, mapChangedEvent, "myPropertySet.onmapchanged");

            logger.comment("** Set property using [] operation");
            expectedKey = "prop1";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemInserted;
            expectedContents = [["prop1", "Property 1"]];
            myPropertySet["prop1"] = "Property 1";
            verify(eventCount, 1, "Event Count");
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);

            logger.comment("** Set property using insert method");
            expectedKey = "prop2";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemInserted;
            expectedContents = [["prop2", "Property 2"], ["prop1", "Property 1"]];
            eventCount = 0;
            var replaced = myPropertySet.insert("prop2", "Property 2");
            verify(replaced, false, 'myPropertySet.insert("prop2", "Property 2")');
            verify(eventCount, 1, "Event Count");
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);

            logger.comment("** Change property using [] operation");
            expectedKey = "prop1";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemChanged;
            expectedContents = [["prop2", "Property 2"], ["prop1", "Changed Property 1"]];
            eventCount = 0;
            myPropertySet["prop1"] = "Changed Property 1";
            verify(eventCount, 1, "Event Count");
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);

            logger.comment("** Change property using insert method");
            expectedKey = "prop2";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemChanged;
            expectedContents = [["prop2", "Changed Property 2"], ["prop1", "Changed Property 1"]];
            eventCount = 0;
            replaced = myPropertySet.insert("prop2", "Changed Property 2");
            verify(replaced, true, 'myPropertySet.insert("prop2", "Changed Property 2")');
            verify(eventCount, 1, "Event Count");
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);

            logger.comment("** Remove property using delete operation");
            expectedKey = "prop2";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemRemoved;
            expectedContents = [["prop1", "Changed Property 1"]];
            eventCount = 0;
            delete myPropertySet["prop2"];
            verify(eventCount, 1, "Event Count");
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);

            logger.comment("** Remove property using remove method");
            expectedKey = "prop1";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemRemoved;
            expectedContents = [];
            eventCount = 0;
            myPropertySet.remove("prop1");
            verify(eventCount, 1, "Event Count");
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);

            logger.comment("** Setting key insert using [] operation");
            eventCount = 0;
            myPropertySet["insert"] = "Insert Property"
            verify(eventCount, 0, "Event Count");
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);

            logger.comment("** Setting key insert using insert method");
            expectedKey = "insert";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.itemInserted;
            expectedContents = [["insert", "Insert Property"]];
            eventCount = 0;
            replaced = myPropertySet.insert("insert", "Insert Property");
            verify(replaced, false, 'myPropertySet.insert("insert", "Insert Property")');
            verify(eventCount, 1, "Event Count");
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);

            logger.comment("** Clear all properties using clear method");
            expectedKey = "";
            expectedCollectionChange = Windows.Foundation.Collections.CollectionChange.reset;
            expectedContents = [];
            eventCount = 0;
            myPropertySet.clear();
            verify(eventCount, 1, "Event Count");
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);

            logger.comment("** remove event handler");
            myPropertySet.onmapchanged = null;
            verify(myPropertySet.onmapchanged, null, "myPropertySet.onmapchanged");

            logger.comment("** Set new property using [] operation");
            eventCount = 0;
            myPropertySet["prop3"] = "Property 3";
            verify(eventCount, 0, "Event Count");
            expectedContents = [["prop3", "Property 3"]];
            verifyPropertyContents("myPropertySet", myPropertySet, expectedContents);
        }
    });

    Loader42_FileName = 'PropertySet Tests'
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
