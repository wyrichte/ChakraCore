if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var TWO_POW_8 = 256;
    var TWO_POW_32 = 4294967296;

    var myAnimal;
    var oneFish;
    var twoFish;

    function verifyObject(parentName, obj, expectedOutput) {
        var match = true;
        verify(typeof expectedOutput, 'object', 'typeof expectedOutput');

        if ("NumFins" in expectedOutput) {
            verify(obj.getNumFins(), expectedOutput.NumFins, 'obj.getNumFins()');
        } else {
            for (var field in obj) {
                if (typeof obj[field] == "object") {
                    verifyObject((parentName + "." + field), obj[field], expectedOutput[field]);
                }
                verify(obj[field], expectedOutput[field], 'obj[field]');
            }
        }

    }

    function verifyOutput(output, expectedOutput) {
        if (typeof expectedOutput == 'object') {
            for (var x in output) {
                if (typeof output[x] == 'object') {
                    verifyObject(x, output[x], expectedOutput[x]);
                } else {
                    verify(output[x], expectedOutput[x], x);
                }
            }
        } else {
            verify(output, expectedOutput, 'output');
        }
    }


    runner.globalSetup(function () {
        myAnimal = new Animals.Animal(1);
        oneFish = new Animals.Fish();
        twoFish = new Animals.Fish();
        twoFish.setNumFins(12);
    });

    runner.addTest({
        id: 1,
        desc: 'multiOutBool Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutBool(true, false);
            var expectedOutput = { reta: true, retb: false };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'multiOutUInt8 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutUInt8(0, TWO_POW_8 - 1);
            var expectedOutput = { reta: 0, retb: 255 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'multiOutInt32 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutInt32(TWO_POW_32 - 1, 1);
            var expectedOutput = { reta: -1, retb: 1 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'multiOutUInt32 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutUInt32(TWO_POW_32 - 1, TWO_POW_32);
            var expectedOutput = { reta: TWO_POW_32 - 1, retb: 0 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'multiOutInt64 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutInt64(-1, TWO_POW_32 + 1);
            var expectedOutput = { reta: -1, retb: TWO_POW_32 + 1 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'multiOutUInt64 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutUInt64(0, TWO_POW_32 + 1);
            var expectedOutput = { reta: 0, retb: TWO_POW_32 + 1 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'multiOutSingle Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutSingle(24, -56);
            var expectedOutput = { reta: 24, retb: -56 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 8,
        desc: 'multiOutDouble Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutDouble(2.4, -7.8);
            var expectedOutput = { reta: 2.4, retb: -7.8 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 9,
        desc: 'multiOutChar16 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutChar16('a', ' ');
            var expectedOutput = { reta: 'a', retb: ' ' };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 10,
        desc: 'mixedInOutBool Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutBool(true, false);
            var expectedOutput = { reta: true, retb: false };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'mixedInOutUInt8 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutUInt8(0, TWO_POW_8 - 1);
            var expectedOutput = { reta: 0, retb: 255 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 12,
        desc: 'mixedInOutInt32 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutInt32(TWO_POW_32 - 1, 1);
            var expectedOutput = { reta: -1, retb: 1 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 13,
        desc: 'mixedInOutUInt32 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutUInt32(TWO_POW_32 - 1, TWO_POW_32);
            var expectedOutput = { reta: TWO_POW_32 - 1, retb: 0 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 14,
        desc: 'mixedInOutInt64 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutInt64(-1, TWO_POW_32 + 1);
            var expectedOutput = { reta: -1, retb: TWO_POW_32 + 1 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 15,
        desc: 'mixedInOutInt64 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutUInt64(0, TWO_POW_32 + 1);
            var expectedOutput = { reta: 0, retb: TWO_POW_32 + 1 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 16,
        desc: 'mixedInOutInt64 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutSingle(24, -56);
            var expectedOutput = { reta: 24, retb: -56 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 17,
        desc: 'mixedInOutDouble Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutDouble(2.4, -7.8);
            var expectedOutput = { reta: 2.4, retb: -7.8 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 18,
        desc: 'mixedInOutChar16 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutChar16('a', ' ');
            var expectedOutput = { reta: 'a', retb: ' ' };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 19,
        desc: 'layoutMany Test',
        pri: '0',
        test: function () {
            var output = myAnimal.layoutOfManyMembers(0, -1, TWO_POW_8 - 1, 5.6, 128, 73, -12.45, TWO_POW_32 / 2 - 1, 0);
            var expectedOutput = { reta: 0, retb: -1, retc: 255, retd: 5.6, rete: 128, retf: 73, retg: -12.45, reth: TWO_POW_32 / 2 - 1, reti: 0 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 20,
        desc: 'multiFloat3 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multiFloat3(15, -3, 76);
            var expectedOutput = { reta: 15, retb: -3, retc: 76 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 21,
        desc: 'multiFloat4 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multiFloat4(-107, 23, -9, 91);
            var expectedOutput = { reta: -107, retb: 23, retc: -9, retd: 91 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 22,
        desc: 'multiDouble3 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multiDouble3(1.5, -12, 7.6);
            var expectedOutput = { reta: 1.5, retb: -12, retc: 7.6 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 23,
        desc: 'multiDouble4 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multiDouble4(-10.7, 2.3, -0.9, 9.1);
            var expectedOutput = { reta: -10.7, retb: 2.3, retc: -0.9, retd: 9.1 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 24,
        desc: 'floatOffsetChar Test',
        pri: '0',
        test: function () {
            var output = myAnimal.floatOffsetChar('a', 22);
            var expectedOutput = { reta: 'a', retb: 22 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 25,
        desc: 'floatOffsetByte Test',
        pri: '0',
        test: function () {
            var output = myAnimal.floatOffsetByte(127, -45);
            var expectedOutput = { reta: 127, retb: -45 }
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 26,
        desc: 'floatOffsetInt Test',
        pri: '0',
        test: function () {
            var output = myAnimal.floatOffsetInt(378, 1);
            var expectedOutput = { reta: 378, retb: 1 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 27,
        desc: 'floatOffsetInt64 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.floatOffsetInt64(TWO_POW_32 + 1, -2);
            var expectedOutput = { reta: TWO_POW_32 + 1, retb: -2 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 28,
        desc: 'floatOffset2Int Test',
        pri: '0',
        test: function () {
            var output = myAnimal.floatOffset2Int(0, -3, 40);
            var expectedOutput = { reta: 0, retb: -3, retc: 40 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 29,
        desc: 'doubleOffsetChar Test',
        pri: '0',
        test: function () {
            var output = myAnimal.doubleOffsetChar('a', 2.2);
            var expectedOutput = { reta: 'a', retb: 2.2 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 30,
        desc: 'doubleOffsetByte Test',
        pri: '0',
        test: function () {
            var output = myAnimal.doubleOffsetByte(127, -4.5);
            var expectedOutput = { reta: 127, retb: -4.5 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 31,
        desc: 'doubleOffsetInt Test',
        pri: '0',
        test: function () {
            var output = myAnimal.doubleOffsetInt(378, 1);
            var expectedOutput = { reta: 378, retb: 1 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 32,
        desc: 'doubleOffsetInt64 Test',
        pri: '0',
        test: function () {
            var output = myAnimal.doubleOffsetInt64(TWO_POW_32 + 1, -2);
            var expectedOutput = { reta: TWO_POW_32 + 1, retb: -2 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 33,
        desc: 'doubleOffset2Int Test',
        pri: '0',
        test: function () {
            var output = myAnimal.doubleOffset2Int(0, -3, 4.0);
            var expectedOutput = { reta: 0, retb: -3, retc: 4.0 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 34,
        desc: 'multiOutHSTRING Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutHSTRING("parameter a", "value of b");
            var expectedOutput = { reta: "parameter a", retb: "value of b" };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 35,
        desc: 'multiOutPhylum Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutPhylum(Animals.Phylum.first, Animals.Phylum.last);
            var expectedOutput = { reta: Animals.Phylum.acanthocephala, retb: Animals.Phylum.xenoturbellid };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 36,
        desc: 'multiOutDimensions Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutDimensions({ length: 10, width: 14 }, { length: 42, width: 121 });
            var expectedOutput = { reta: { length: 10, width: 14 }, retb: { length: 42, width: 121} };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 37,
        desc: 'multiOutIFish Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutIFish(oneFish, twoFish);
            var expectedOutput = { reta: { NumFins: 5 }, retb: { NumFins: 12} };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 39,
        desc: 'multiOutFish Test',
        pri: '0',
        test: function () {
            var output = myAnimal.multipleOutFish(oneFish, twoFish);
            var expectedOutput = { reta: { NumFins: 5 }, retb: { NumFins: 12} };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 40,
        desc: 'mixedInOutHSTRING Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutHSTRING("parameter a", "value of b");
            var expectedOutput = { reta: "parameter a", retb: "value of b" };
            verifyOutput(output, expectedOutput);
        }
    });


    runner.addTest({
        id: 41,
        desc: 'mixedInOutPhylum Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutPhylum(Animals.Phylum.first, Animals.Phylum.last);
            var expectedOutput = { reta: Animals.Phylum.acanthocephala, retb: Animals.Phylum.xenoturbellid };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 42,
        desc: 'mixedInOutDimensions Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutDimensions({ length: 10, width: 14 }, { length: 42, width: 121 });
            var expectedOutput = { reta: { length: 10, width: 14 }, retb: { length: 42, width: 121} };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 43,
        desc: 'mixedInOutIFish Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutIFish(oneFish, twoFish);
            var expectedOutput = { reta: { NumFins: 5 }, retb: { NumFins: 12} };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 45,
        desc: 'mixedInOutFish Test',
        pri: '0',
        test: function () {
            var output = myAnimal.interspersedInOutFish(oneFish, twoFish);
            var expectedOutput = { reta: { NumFins: 5 }, retb: { NumFins: 12} };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 46,
        desc: 'layoutStructs Test',
        pri: '0',
        test: function () {
            var output = myAnimal.layoutStructs({ a: TWO_POW_32 - 1 }, { length: 100, width: 365 }, { inner: { a: TWO_POW_32} }, { common: "Common Wombat", scientific: "Vombatus ursinis", alsoKnownAs: "Coarse-haired Wombat" }, { current: Animals.Phylum.chordata, original: Animals.Phylum.first });
            var expectedOutput = { reta: { a: -1 }, retb: { length: 100, width: 365 }, retc: { inner: { a: 0} }, retd: { common: "Common Wombat", scientific: "Vombatus ursinis", alsoKnownAs: "Coarse-haired Wombat" }, rete: { current: Animals.Phylum.chordata, original: Animals.Phylum.first} };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 47,
        desc: 'layoutBasicWithStructs Test',
        pri: '0',
        test: function () {
            var output = myAnimal.layoutBasicWithStructs(TWO_POW_8 - 1, { a: 42 }, -100, 12.34, { common: "Long-tailed Chinchilla", scientific: "Chinchilla lanigera", alsoKnownAs: "Chilean, coastal, or lesser chinchilla" }, 13, TWO_POW_8, { length: -3, width: 52 }, -512);
            var expectedOutput = { reta: 255, retb: { a: 42 }, retc: -100, retd: 12.34, rete: { common: "Long-tailed Chinchilla", scientific: "Chinchilla lanigera", alsoKnownAs: "Chilean, coastal, or lesser chinchilla" }, retf: 13, retg: 0, reth: { length: -3, width: 52 }, reti: -512 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 48,
        desc: 'floatOffsetStruct Test',
        pri: '0',
        test: function () {
            var output = myAnimal.floatOffsetStruct({ common: "Naked Mole Rat", scientific: "Heterocephalus glaber", alsoKnownAs: "sand puppy or desert mole rat" }, 82);
            var expectedOutput = { reta: { common: "Naked Mole Rat", scientific: "Heterocephalus glaber", alsoKnownAs: "sand puppy or desert mole rat" }, retb: 82 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 49,
        desc: 'doubleOffsetStruct Test',
        pri: '0',
        test: function () {
            var output = myAnimal.doubleOffsetStruct({ common: "Pygmy Hippopotamus", scientific: "Choeropsis liberiensis", alsoKnownAs: "Mini Hippo" }, 0.82);
            var expectedOutput = { reta: { common: "Pygmy Hippopotamus", scientific: "Choeropsis liberiensis", alsoKnownAs: "Mini Hippo" }, retb: .82 };
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 50,
        desc: 'SetAndGetNumLegs Test',
        pri: '0',
        test: function () {
            var output = myAnimal.setNumLegs(42); return myAnimal.getNumLegs();
            var expectedOutput = 42;
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 51,
        desc: 'SetAndGetGreeting Test',
        pri: '0',
        test: function () {
            var output = myAnimal.setGreeting("Guten Tag!"); return myAnimal.getGreeting();
            var expectedOutput = 'Guten Tag!';
            verifyOutput(output, expectedOutput);
        }
    });

    runner.addTest({
        id: 52,
        desc: 'Pass in the Big struct',
        pri: '0',
        test: function () {
            var myCollectionChangedEventArgs = {
                eType: Animals.CollectionChangeType.itemRemoved,
                index: 38,
                previousIndex: 49,
                objectId: "delegateWithOutParamTestCase"
            };
            var result = Animals.Animal.methodWithInParam_BigStruct(myCollectionChangedEventArgs);
            verify(result.eType, myCollectionChangedEventArgs.eType, "result.eType");
            verify(result.index, myCollectionChangedEventArgs.index, "result.index");
            verify(result.previousIndex, myCollectionChangedEventArgs.previousIndex, "result.previousIndex");
            verify(result.objectId, myCollectionChangedEventArgs.objectId, "result.objectId");
        }
    });

    runner.addTest({
        id: 53,
        desc: 'Receive big struct as out param',
        pri: '0',
        test: function () {
            var myEventArgsType = Animals.CollectionChangeType.itemRemoved;
            var myObjectId = "DelegateWithInParamTestCase";
            var myIndex = 38;
            var myPreviousIndex = 49;
            var result = Animals.Animal.methodWithOutParam_BigStruct(myObjectId, myEventArgsType, myIndex, myPreviousIndex);
            verify(result.eType, myEventArgsType, "result.eType");
            verify(result.index, myIndex, "result.index");
            verify(result.previousIndex, myPreviousIndex, "result.previousIndex");
            verify(result.objectId, myObjectId, "result.objectId");
        }
    });

    runner.addTest({
        id: 54,
        desc: 'Marshal in and out Packed Byte',
        pri: '0',
        test: function () {
            var inValue = { field0: 80 };
            var outValue = Animals.Animal.marshalInAndOutPackedByte(inValue);
            verify(outValue.toString(), "[object Animals.PackedByte]", "outValue.toString()");
            verify(outValue.field0, 80, "outValue.field0");
            assert(outValue.field0 === inValue.field0, "outValue.field0 == inValue.field0");
        }
    });

    runner.addTest({
        id: 55,
        desc: 'Marshal in and out Packed Booleans',
        pri: '0',
        test: function () {
            var inValue = {
                field0: false,
                field1: true,
                field2: false,
                field3: false
            }
            var outValue = Animals.Animal.marshalInAndOutPackedBoolean(inValue);
            verify(outValue.toString(), "[object Animals.PackedBoolean4]", "outValue.toString()");
            verify(outValue.field0, false, "outValue.field0");
            verify(outValue.field1, true, "outValue.field1");
            verify(outValue.field2, false, "outValue.field2");
            verify(outValue.field3, false, "outValue.field3");

            assert(outValue.field0 === inValue.field0, "outValue.field0 === inValue.field0");
            assert(outValue.field1 === inValue.field1, "outValue.field1 === inValue.field1");
            assert(outValue.field2 === inValue.field2, "outValue.field2 === inValue.field2");
            assert(outValue.field3 === inValue.field3, "outValue.field3 === inValue.field3");
        }
    });

    runner.addTest({
        id: 56,
        desc: 'Marshal in and out odd sized struct',
        pri: '0',
        test: function () {
            var inValue = {
                field0: 50,
                field1: 100,
                field2: 150
            }
            var outValue = Animals.Animal.marshalInAndOutOddSizedStruct(inValue);
            verify(outValue.toString(), "[object Animals.OddSizedStruct]", "outValue.toString()");
            verify(outValue.field0, 50, "outValue.field0");
            verify(outValue.field1, 100, "outValue.field1");
            verify(outValue.field2, 150, "outValue.field2");

            assert(outValue.field0 === inValue.field0, "outValue.field0 === inValue.field0");
            assert(outValue.field1 === inValue.field1, "outValue.field1 === inValue.field1");
            assert(outValue.field2 === inValue.field2, "outValue.field2 === inValue.field2");
        }
    });

    runner.addTest({
        id: 57,
        desc: 'Marshal in and out small complex struct',
        pri: '0',
        test: function () {
            var inValue = {
                field0: 50,
                field1: {
                    field0: 100
                },
                field2: 150
            }
            var outValue = Animals.Animal.marshalInAndOutSmallComplexStruct(inValue);
            verify(outValue.toString(), "[object Animals.SmallComplexStruct]", "outValue.toString()");
            verify(outValue.field0, 50, "outValue.field0");
            verify(outValue.field1.toString(), "[object Animals.PackedByte]", "outValue.field1.toString()");
            verify(outValue.field1.field0, 100, "outValue.field1.field0");
            verify(outValue.field2, 150, "outValue.field2");

            assert(outValue.field0 === inValue.field0, "outValue.field0 === inValue.field0");
            assert(outValue.field1.field0 === inValue.field1.field0, "outValue.field1.field0 === inValue.field1.field0");
            assert(outValue.field2 === inValue.field2, "outValue.field2 === inValue.field2");
        }
    });


    runner.addTest({
        id: 58,
        desc: 'Marshal in and out big complex struct',
        pri: '0',
        test: function () {
            var inValue = {
                field0: 5,
                field1: {
                    field0: 10
                },
                field2: 15,
                field3: {
                    field0: true,
                    field1: true,
                    field2: false,
                    field3: false
                },
                field4: {
                    field0: 20,
                    field1: {
                        field0: 25
                    },
                    field2: 30
                },
                field5: {
                    field0: 35,
                    field1: {
                        field0: 40
                    },
                    field2: 45
                },
                field6: 50,
                field7: 55
            }
            var outValue = Animals.Animal.marshalInAndOutBigComplexStruct(inValue);
            verify(outValue.toString(), "[object Animals.BigComplexStruct]", "outValue.toString()");
            verify(outValue.field0, 5, "outValue.field0");
            verify(outValue.field1.toString(), "[object Animals.PackedByte]", "outValue.field1.toString()");
            verify(outValue.field1.field0, 10, "outValue.field1.field0");
            verify(outValue.field2, 15, "outValue.field2");
            verify(outValue.field3.toString(), "[object Animals.PackedBoolean4]", "outValue.field3.toString()");
            verify(outValue.field3.field0, true, "outValue.field3.field0");
            verify(outValue.field3.field1, true, "outValue.field3.field1");
            verify(outValue.field3.field2, false, "outValue.field3.field2");
            verify(outValue.field3.field3, false, "outValue.field3.field3");
            verify(outValue.field4.toString(), "[object Animals.SmallComplexStruct]", "outValue.field3.toString()");
            verify(outValue.field4.field0, 20, "outValue.field4.field0");
            verify(outValue.field4.field1.toString(), "[object Animals.PackedByte]", "outValue.field4.field1.toString()");
            verify(outValue.field4.field1.field0, 25, "outValue.field4.field1.field0");
            verify(outValue.field4.field2, 30, "outValue.field4.field2");
            verify(outValue.field5.toString(), "[object Animals.SmallComplexStruct]", "outValue.field5.toString()");
            verify(outValue.field5.field0, 35, "outValue.field5.field0");
            verify(outValue.field5.field1.toString(), "[object Animals.PackedByte]", "outValue.field5.field1.toString()");
            verify(outValue.field5.field1.field0, 40, "outValue.field5.field1.field0");
            verify(outValue.field5.field2, 45, "outValue.field5.field2");
            verify(outValue.field6, 50, "outValue.field6");
            verify(outValue.field7, 55, "outValue.field7");

            assert(outValue.field0 === inValue.field0, "outValue.field0 === inValue.field0");
            assert(outValue.field1.field0 === inValue.field1.field0, "outValue.field1.field0 === inValue.field1.field0");
            assert(outValue.field2 === inValue.field2, "outValue.field2 === inValue.field2");
            assert(outValue.field3.field0 === inValue.field3.field0, "outValue.field3.field0 === inValue.field3.field0");
            assert(outValue.field3.field1 === inValue.field3.field1, "outValue.field3.field1 === inValue.field3.field1");
            assert(outValue.field3.field2 === inValue.field3.field2, "outValue.field3.field2 === inValue.field3.field2");
            assert(outValue.field3.field3 === inValue.field3.field3, "outValue.field3.field3 === inValue.field3.field3");
            assert(outValue.field4.field0 === inValue.field4.field0, "outValue.field4.field0 === inValue.field4.field0");
            assert(outValue.field4.field1.field0 === inValue.field4.field1.field0, "outValue.field4.field1.field0 === inValue.field4.field1.field0");
            assert(outValue.field4.field2 === inValue.field4.field2, "outValue.field4.field2 === inValue.field4.field2");
            assert(outValue.field5.field0 === inValue.field5.field0, "outValue.field5.field0 === inValue.field5.field0");
            assert(outValue.field5.field1.field0 === inValue.field5.field1.field0, "outValue.field5.field1.field0 === inValue.field5.field1.field0");
            assert(outValue.field5.field2 === inValue.field5.field2, "outValue.field5.field2 === inValue.field5.field2");
            assert(outValue.field6 === inValue.field6, "outValue.field6 === inValue.field6");
            assert(outValue.field7 === inValue.field7, "outValue.field7 === inValue.field7");
        }
    });


    Loader42_FileName = "Parameter tests";

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
