if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    verify.members = function verifyMembers(obj, expected, msg) {
        logger.comment("Verifying members of " + msg);

        var expect;
        for (var mem in obj) {
            expect = expected[mem];
            verify.defined(expect, mem);
            verify.typeOf(obj[mem], expect, mem);
        }
    };

    verify.fields = function verifyEnum(obj, expectedFields, unexpectedFields, msg) {
        logger.comment("Verifying fields of " + msg);
        for (var field in expectedFields) {
            verify.defined(obj[field], field);
            verify(obj[field], expectedFields[field], field);
        }

        for (var field in unexpectedFields) {
            verify.typeOf(obj[field], unexpectedFields[field]);
        }
    }

    function verifyAndWrite(actual, expected, newValue, msg) {
        verify(actual, expected, msg);
        actual = newValue;
        verify(actual, newValue, msg);
    }

    runner.addTest({
        id: 1,
        desc: 'String variations of Runtime Class members',
        pri: '0',
        test: function () {
            var stringVariations = new DevTests.CamelCasing.StringVariations();
            var stringVariationsExpected = {
                addEventListener: 'function',
                removeEventListener: 'function',
                fireEvent: 'function',
                struct: 'object',
                pascalProperty: 'string',
                pascalPropertyWritable: 'number',
                alluppercaseproperty: 'string',
                iinterfacePropertyWritable: 'number',
                camelProperty: 'string',
                pascalNotationMethod: 'function',
                alluppercasemethod: 'function',
                iinterfaceMethod: 'function',
                camelCaseMethod: 'function',
                _PrivatePascalProperty: 'string',
                _PRIVATEUPPERCASEPROPERTYWRITABLE: 'number',
                _IInterfacePrivateProperty: 'string',
                _privateCamelPropertyWritable: 'number',
                _PrivatePascalMethod: 'function',
                _privateCamelMethod: 'function',
                _PRIVATEUPPERCASEMETHOD: 'function',
                _IInterfacePrivateMethod: 'function',
                f5: 'function',
                hd720p: 'string',
                sp80056aConcatMd5: 'function',
                uint8Array: 'string',
                noncased_CHAR: 'number',
                on_iinterfaceprivateevent: 'object',
                on_privatecamelevent: 'object',
                on_privatepascalevent: 'object',
                on_privateuppercaseevent: 'object',
                oncamelevent: 'object',
                oniinterfaceevent: 'object',
                onpascalevent: 'object',
                onuppercaseevent: 'object',
                onf8event: 'object',
                onecdh521event: 'object',
                onuint16event: 'object',
                onnoncased_event: 'object',
                onuitwoletteracronymevent: 'object',
                aiTwoLetterAcronymMethod: 'function',
                toString: 'function'
            };
            verify.members(stringVariations, stringVariationsExpected, stringVariations.toString());

            verify(stringVariations.pascalProperty, "DevTests.CamelCasing.ICasing.PascalProperty", "stringVariations.pascalProperty");
            verifyAndWrite(stringVariations.pascalPropertyWritable, 0, 21, "stringVariations.pascalPropertyWritable");
            verify(stringVariations.alluppercaseproperty, "DevTests.CamelCasing.ICasing.ALLUPPERCASEPROPERTY", "stringVariations.alluppercaseproperty");
            verifyAndWrite(stringVariations.iinterfacePropertyWritable, 0, 3, "stringVariations.iinterfacePropertyWritable");
            verify(stringVariations.camelProperty, "DevTests.CamelCasing.ICasing.camelProperty", "stringVariations.camelProperty");

            verify(stringVariations._PrivatePascalProperty, "DevTests.CamelCasing.IPrivate._PrivatePascalProperty", "stringVariations._PrivatePascalProperty");
            verifyAndWrite(stringVariations._PRIVATEUPPERCASEPROPERTYWRITABLE, 0, 1024, "stringVariations._PRIVATEUPPERCASEPROPERTYWRITABLE");
            verify(stringVariations._IInterfacePrivateProperty, "DevTests.CamelCasing.IPrivate._IInterfacePrivateProperty", "stringVariations._IInterfacePrivateProperty");
            verifyAndWrite(stringVariations._privateCamelPropertyWritable, 0, 121, "stringVariations._privateCamelPropertyWritable");

            verify(stringVariations.pascalNotationMethod(), "DevTests.CamelCasing.ICasing.PascalNotationMethod() Called", "stringVariations.pascalNotationMethod()");
            verify(stringVariations.alluppercasemethod(), "DevTests.CamelCasing.ICasing.ALLUPPERCASEMETHOD() Called", "stringVariations.alluppercasemethod()");
            verify(stringVariations.iinterfaceMethod(), "DevTests.CamelCasing.ICasing.IInterfaceMethod() Called", "stringVariations.iinterfaceMethod()");
            verify(stringVariations.camelCaseMethod(), "DevTests.CamelCasing.ICasing.camelCaseMethod() Called", "stringVariations.camelCaseMethod()");

            verify(stringVariations._PrivatePascalMethod(), "DevTests.CamelCasing.IPrivate._PrivatePascalMethod() Called", "stringVariations._PrivatePascalMethod()");
            verify(stringVariations._privateCamelMethod(), "DevTests.CamelCasing.IPrivate._privateCamelMethod() Called", "stringVariations._privateCamelMethod()");
            verify(stringVariations._PRIVATEUPPERCASEMETHOD(), "DevTests.CamelCasing.IPrivate._PRIVATEUPPERCASEMETHOD() Called", "stringVariations._PRIVATEUPPERCASEMETHOD()");
            verify(stringVariations._IInterfacePrivateMethod(), "DevTests.CamelCasing.IPrivate._IInterfacePrivateMethod() Called", "stringVariations._IInterfacePrivateMethod()");

            verify(stringVariations.f5(), "DevTests.CamelCasing.ICasing.F5() Called", "stringVariations.f5()");
            verify(stringVariations.hd720p, "DevTests.CamelCasing.ICasing.HD720p", "stringVariations.hd720p");
            verify(stringVariations.sp80056aConcatMd5(), "DevTests.CamelCasing.ICasing.SP80056aConcatMd5() Called", "stringVariations.sp80056aConcatMd5()");
            verify(stringVariations.uint8Array, "DevTests.CamelCasing.ICasing.UInt8Array", "stringVariations.uint8Array");
            verifyAndWrite(stringVariations.noncased_CHAR, 0, 976, "stringVariations.noncased_CHAR");
            verify(stringVariations.aiTwoLetterAcronymMethod(), "DevTests.CamelCasing.ICasing.AITwoLetterAcronymMethod() Called", "stringVariations.aiTwoLetterAcronymMethod()");

            var structStringVariationsExpected = {
                pascalField: "StructStringVariations.PascalField",
                uppercasefield: "StructStringVariations.UPPERCASEFIELD",
                iinterfaceField: "StructStringVariations.IInterfaceField",
                camelField: "StructStringVariations.camelField",
                _PrivatePascalField: "StructStringVariations._PrivatePascalField",
                _PRIVATEUPPERCASEFIELD: "StructStringVariations._PRIVATEUPPERCASEFIELD",
                _IInterfacePrivateField: "StructStringVariations._IInterfacePrivateField",
                _privateCamelField: "StructStringVariations._privateCamelField",
                f12: "StructStringVariations.F12",
                hd1080p: "StructStringVariations.HD1080p",
                sp800108CtrHmacMd5: "StructStringVariations.SP800108CtrHmacMd5",
                uint32: "StructStringVariations.UInt32",
                noncased_CHAR: "StructStringVariations.NONCASED_CHAR",
                ipTwoLetterAcronym: "StructStringVariations.IPTwoLetterAcronym"
            };

            var structStringVariationsUnexpected = {
                PascalField: 'undefined',
                UPPERCASEFIELD: 'undefined',
                IInterfaceField: 'undefined',
                _privatePascalField: 'undefined',
                F12: 'undefined',
                HD1080p: 'undefined',
                SP800108CtrHmacMd5: 'undefined',
                UInt32: 'undefined',
                NONCASED_CHAR: 'undefined',
                IPTwoLetterAcronym: 'undefined',
                iptwoLetterAcronym: 'undefined'
            };

            verify.fields(stringVariations.struct, structStringVariationsExpected, structStringVariationsUnexpected, "stringVariations.struct");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'String variations of Static Runtime Class members',
        pri: '0',
        test: function () {
            var stringVariationsStatic = DevTests.CamelCasing.StringVariations;
            var stringVariationsStaticExpected = {
                addEventListener: 'function',
                removeEventListener: 'function',
                staticFireEvent: 'function',
                pascalStaticProperty: 'string',
                pascalStaticPropertyWritable: 'number',
                alluppercasestaticpropertywritable: 'number',
                iinterfaceStaticProperty: 'string',
                camelStaticPropertyWritable: 'number',
                pascalStaticMethod: 'function',
                alluppercasestaticmethod: 'function',
                iinterfaceStaticMethod: 'function',
                camelStaticMethod: 'function',
                _PrivatePascalStaticPropertyWritable: 'number',
                _PRIVATEUPPERCASESTATICPROPERTY: 'string',
                _IInterfacePrivateStaticPropertyWritable: 'number',
                _privateCamelStaticProperty: 'string',
                _PrivatePascalStaticMethod: 'function',
                _privateCamelStaticMethod: 'function',
                _PRIVATEUPPERCASESTATICMETHOD: 'function',
                _IInterfacePrivateStaticMethod: 'function',
                f11: 'function',
                smsreceived: 'string',
                y4Cb2Cr0: 'function',
                utf8: 'string',
                noncased_CHARSTATIC: 'number',
                on_iinterfaceprivatestaticevent: 'object',
                on_privatecamelstaticevent: 'object',
                on_privatepascalstaticevent: 'object',
                on_privateuppercasestaticevent: 'object',
                oncamelstaticevent: 'object',
                oniinterfacestaticevent: 'object',
                onpascalstaticevent: 'object',
                onuppercasestaticevent: 'object',
                msTwoLetterAcronymMethod: 'function'
            };
            verify.members(stringVariationsStatic, stringVariationsStaticExpected, "DevTests.CamelCasing.StringVariations");

            verify(stringVariationsStatic.pascalStaticProperty, "DevTests.CamelCasing.ICasingStatic.PascalStaticProperty", "stringVariationsStatic.pascalStaticProperty");
            verifyAndWrite(stringVariationsStatic.pascalStaticPropertyWritable, 0, 21, "stringVariationsStatic.pascalStaticPropertyWritable");
            verifyAndWrite(stringVariationsStatic.alluppercasestaticpropertywritable, 0, 56, "stringVariationsStatic.alluppercasestaticpropertywritable");
            verify(stringVariationsStatic.iinterfaceStaticProperty, "DevTests.CamelCasing.ICasingStatic.IInterfaceStaticProperty", "stringVariationsStatic.iinterfaceStaticProperty");
            verifyAndWrite(stringVariationsStatic.camelStaticPropertyWritable, 0, 43, "stringVariationsStatic.camelStaticPropertyWritable");

            verifyAndWrite(stringVariationsStatic._PrivatePascalStaticPropertyWritable, 0, 1, "stringVariationsStatic._PrivatePascalStaticPropertyWritable");
            verify(stringVariationsStatic._PRIVATEUPPERCASESTATICPROPERTY, "DevTests.CamelCasing.IPrivateStatic._PRIVATEUPPERCASESTATICPROPERTY", "stringVariationsStatic._PRIVATEUPPERCASEPROPERTYWRITABLE");
            verifyAndWrite(stringVariationsStatic._IInterfacePrivateStaticPropertyWritable, 0, 77, "stringVariationsStatic._IInterfacePrivateStaticPropertyWritable");
            verify(stringVariationsStatic._privateCamelStaticProperty, "DevTests.CamelCasing.IPrivateStatic._privateCamelStaticProperty", "stringVariationsStatic._privateCamelStaticProperty");

            verify(stringVariationsStatic.pascalStaticMethod(), "DevTests.CamelCasing.ICasingStatic.PascalStaticMethod() Called", "stringVariationsStatic.pascalStaticMethod()");
            verify(stringVariationsStatic.alluppercasestaticmethod(), "DevTests.CamelCasing.ICasingStatic.ALLUPPERCASESTATICMETHOD() Called", "stringVariationsStatic.alluppercasestaticmethod()");
            verify(stringVariationsStatic.iinterfaceStaticMethod(), "DevTests.CamelCasing.ICasingStatic.IInterfaceStaticMethod() Called", "stringVariationsStatic.iinterfaceStaticMethod()");
            verify(stringVariationsStatic.camelStaticMethod(), "DevTests.CamelCasing.ICasingStatic.camelStaticMethod() Called", "stringVariationsStatic.camelStaticMethod()");

            verify(stringVariationsStatic._PrivatePascalStaticMethod(), "DevTests.CamelCasing.IPrivateStatic._PrivatePascalStaticMethod() Called", "stringVariationsStatic._PrivatePascalStaticMethod()");
            verify(stringVariationsStatic._privateCamelStaticMethod(), "DevTests.CamelCasing.IPrivateStatic._privateCamelStaticMethod() Called", "stringVariationsStatic._privateCamelStaticMethod()");
            verify(stringVariationsStatic._PRIVATEUPPERCASESTATICMETHOD(), "DevTests.CamelCasing.IPrivateStatic._PRIVATEUPPERCASESTATICMETHOD() Called", "stringVariationsStatic._PRIVATEUPPERCASESTATICMETHOD()");
            verify(stringVariationsStatic._IInterfacePrivateStaticMethod(), "DevTests.CamelCasing.IPrivateStatic._IInterfacePrivateStaticMethod() Called", "stringVariationsStatic._IInterfacePrivateStaticMethod()");

            verify(stringVariationsStatic.f11(), "DevTests.CamelCasing.ICasingStatic.F11() Called", "stringVariationsStatic.f11()");
            verify(stringVariationsStatic.smsreceived, "DevTests.CamelCasing.ICasingStatic.SMSReceived", "stringVariationsStatic.smsreceived");
            verify(stringVariationsStatic.y4Cb2Cr0(), "DevTests.CamelCasing.ICasingStatic.Y4Cb2Cr0() Called", "stringVariationsStatic.y4Cb2Cr0()");
            verify(stringVariationsStatic.utf8, "DevTests.CamelCasing.ICasingStatic.UTF8", "stringVariationsStatic.utf8");
            verifyAndWrite(stringVariationsStatic.noncased_CHARSTATIC, 0, 2016, "stringVariationsStatic.noncased_CHARSTATIC");
            verify(stringVariationsStatic.msTwoLetterAcronymMethod(), "DevTests.CamelCasing.ICasingStatic.MSTwoLetterAcronymMethod() Called", "stringVariationsStatic.msTwoLetterAcronymMethod()");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'String variations of Overload names',
        pri: '0',
        test: function () {
            var overloadVariations = new DevTests.CamelCasing.OverloadStringVariations();
            var overloadVariationsExpected = {
                pascalNotationOverload: 'function',
                alluppercaseoverload: 'function',
                iinterfaceOverload: 'function',
                camelCaseOverload: 'function',
                _PrivatePascalOverload: 'function',
                _privateCamelOverload: 'function',
                _PRIVATEUPPERCASEOVERLOAD: 'function',
                _IInterfacePrivateOverload: 'function',
                toString: 'function'
            };
            verify.members(overloadVariations, overloadVariationsExpected, overloadVariations.toString());

            verify(overloadVariations.pascalNotationOverload(), "DevTests.CamelCasing.IOverloadCasing.PascalNotationOverload() Called", "overloadVariations.pascalNotationOverload()");
            verify(overloadVariations.alluppercaseoverload(), "DevTests.CamelCasing.IOverloadCasing.ALLUPPERCASEOVERLOAD() Called", "overloadVariations.alluppercaseoverload()");
            verify(overloadVariations.iinterfaceOverload(), "DevTests.CamelCasing.IOverloadCasing.IInterfaceOverload() Called", "overloadVariations.iinterfaceOverload()");
            verify(overloadVariations.camelCaseOverload(), "DevTests.CamelCasing.IOverloadCasing.camelCaseOverload() Called", "overloadVariations.camelCaseOverload()");

            verify(overloadVariations._PrivatePascalOverload(), "DevTests.CamelCasing.IPrivateOverloads._PrivatePascalOverload() Called", "overloadVariations._PrivatePascalOverload()");
            verify(overloadVariations._privateCamelOverload(), "DevTests.CamelCasing.IPrivateOverloads._privateCamelOverload() Called", "overloadVariations._privateCamelOverload()");
            verify(overloadVariations._PRIVATEUPPERCASEOVERLOAD(), "DevTests.CamelCasing.IPrivateOverloads._PRIVATEUPPERCASEOVERLOAD() Called", "overloadVariations._PRIVATEUPPERCASEOVERLOAD()");
            verify(overloadVariations._IInterfacePrivateOverload(), "DevTests.CamelCasing.IPrivateOverloads._IInterfacePrivateOverload() Called", "overloadVariations._IInterfacePrivateOverload()");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'String variations of Overload names on static interfaces',
        pri: '0',
        test: function () {
            var overloadVariationsStatic = DevTests.CamelCasing.OverloadStringVariations;
            var overloadVariationsStaticExpected = {
                pascalStaticOverload: 'function',
                uppercasestaticoverload: 'function',
                iinterfaceStaticOverload: 'function',
                camelStaticOverload: 'function',
                _PrivatePascalStaticOverload: 'function',
                _privateCamelStaticOverload: 'function',
                _PRIVATEUPPERCASESTATICOVERLOAD: 'function',
                _IInterfacePrivateStaticOverload: 'function'
            };
            verify.members(overloadVariationsStatic, overloadVariationsStaticExpected, "DevTests.CamelCasing.OverloadStringVariations");

            verify(overloadVariationsStatic.pascalStaticOverload(), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called", "overloadVariationsStatic.pascalStaticOverload()");
            verify(overloadVariationsStatic.uppercasestaticoverload(), "DevTests.CamelCasing.IOverloadCasingStatic.UPPERCASESTATICOVERLOAD() Called", "overloadVariationsStatic.uppercasestaticoverload()");
            verify(overloadVariationsStatic.iinterfaceStaticOverload(), "DevTests.CamelCasing.IOverloadCasingStatic.IInterfaceStaticOverload() Called", "overloadVariationsStatic.iinterfaceStaticOverload()");
            verify(overloadVariationsStatic.camelStaticOverload(), "DevTests.CamelCasing.IOverloadCasingStatic.camelStaticOverload() Called", "overloadVariationsStatic.camelStaticOverload()");

            verify(overloadVariationsStatic._PrivatePascalStaticOverload(), "DevTests.CamelCasing.IPrivateOverloadsStatic._PrivatePascalStaticOverload() Called", "overloadVariationsStatic._PrivatePascalStaticOverload()");
            verify(overloadVariationsStatic._privateCamelStaticOverload(), "DevTests.CamelCasing.IPrivateOverloadsStatic._privateCamelStaticOverload() Called", "overloadVariationsStatic._privateCamelStaticOverload()");
            verify(overloadVariationsStatic._PRIVATEUPPERCASESTATICOVERLOAD(), "DevTests.CamelCasing.IPrivateOverloadsStatic._PRIVATEUPPERCASESTATICOVERLOAD() Called", "overloadVariationsStatic._PRIVATEUPPERCASESTATICOVERLOAD()");
            verify(overloadVariationsStatic._IInterfacePrivateStaticOverload(), "DevTests.CamelCasing.IPrivateOverloadsStatic._IInterfacePrivateStaticOverload() Called", "overloadVariationsStatic._IInterfacePrivateStaticOverload()");
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Simple Name Collision Cases',
        pri: '0',
        test: function () {
            var nameCollisions = DevTests.CamelCasing.SimpleNameCollisions;
            var events = nameCollisions.ConflictingEvents;

            var eventsExpected = {
                internalPascalEvent: 0,
                internalCamelEvent: 1,
                externalEvent: 2,
                externalCamelEvent: 3
            };

            var eventsUnexpected = {
                InternalPascalEvent: 'undefined',
                InternalCamelEvent: 'undefined',
                ExternalEvent: 'undefined',
                ExternalCamelEvent: 'undefined'
            };

            verify.fields(events, eventsExpected, eventsUnexpected, "DevTests.CamelCasing.SimpleNameCollisions.ConflictingEvents");

            var enumConflictExpected = {
                conflictingField: 0
            };
            verify.fields(nameCollisions.EnumInternalConflict, enumConflictExpected, null, "DevTests.CamelCasing.SimpleNameCollisions.EnumInternalConflict");

            var internalConflict = new nameCollisions.InternalConflict();
            var internalConflictExpected = {
                addEventListener: 'function',
                removeEventListener: 'function',
                fireEvent: 'function',
                conflictingProperty: 'number',
                conflictingMethod: 'function',
                structConflict: 'object',
                'DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener': 'function',
                RemoveEventListener: 'function',
                onconflictingevent: 'object',
                toString: 'function'
            };

            verify.members(internalConflict, internalConflictExpected, internalConflict.toString());
            verify(internalConflict.conflictingMethod(1),
                "DevTests.CamelCasing.SimpleNameCollisions.IInternalConflict.ConflictingMethod(int) Called",
                "internalConflict.conflictingMethod(1)");
            verify(internalConflict['DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener'](),
                "DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener() called",
                "internalConflict['DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener']()");
            verify(internalConflict.RemoveEventListener(),
                "DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.RemoveEventListener() called",
                "internalConflict.RemoveEventListener()");

            var structConflict = internalConflict.structConflict;
            var structConflictExpected = {
                conflictingField: 'number',
                'toString': 'function'
            };
            verify.members(structConflict, structConflictExpected, structConflict.toString());
            verify(structConflict.conflictingField, 256, "structConflict.conflictingField");
            logger.comment("Set structConflict = { conflictingField: 3 }");
            internalConflict.structConflict = { conflictingField: 3 };
            structConflict = internalConflict.structConflict;
            verify.members(structConflict, structConflictExpected, structConflict.toString());
            verify(structConflict.conflictingField, 3, "structConflict.conflictingField");

            var expectedStr;
            var timesCalled = 0;
            function callback(ev) {
                verify(ev.detail[0], expectedStr, "Event fired");
                timesCalled++;
            }
            internalConflict.addEventListener('conflictingevent', callback);

            expectedStr = "DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.ConflictingEvent";
            internalConflict.fireEvent(events.internalPascalEvent);
            internalConflict.removeEventListener('conflictingevent', callback);

            expectedStr = "";
            internalConflict.fireEvent(events.internalPascalEvent);
            verify(timesCalled, 1, "Total number of callbacks");
            timesCalled = 0;

            var externalSameCase = new nameCollisions.ExternalConflictSameCase();
            var externalSameCaseExpected = {
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.conflictingProperty': 'string',
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalEventConflict.onconflictingevent': 'object',
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalPascalConflict.conflictingProperty': 'number',
                'toString': 'function',
                'conflictingMethod': 'function' // This was turned into an overload by rtc metadata change
            };
            verify.members(externalSameCase, externalSameCaseExpected, externalSameCase.toString());
            verify(externalSameCase.conflictingMethod('a', 'b'),
                "DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingMethod(HSTRING, HSTRING) Called",
                "externalSameCase.conflictingMethod('a', 'b')");
            verify(externalSameCase.conflictingMethod(1),
                "DevTests.CamelCasing.SimpleNameCollisions.IExternalPascalConflict.ConflictingMethod(int) Called",
                "externalSameCase.conflictingMethod(1)");

            var externalDiffCase = new nameCollisions.ExternalConflictDifferentCase();

            var externalDiffCaseExpected = {
                addEventListener: 'function',
                removeEventListener: 'function',
                fireEvent: 'function',
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.conflictingProperty': 'string',
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelConflict.conflictingProperty': 'number',
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.conflictingMethod': 'function',
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelConflict.conflictingMethod': 'function',
                'AddEventListener': 'function', // From DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict
                'DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener': 'string',
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelEventConflict.onconflictingevent': 'object',
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalEventConflict.onconflictingevent': 'object',
                'toString': 'function'
            };

            verify.members(externalDiffCase, externalDiffCaseExpected, externalDiffCase.toString());
            verify(externalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.conflictingMethod']('a', 'b'),
                "DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingMethod(HSTRING, HSTRING) Called",
                "externalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.conflictingMethod']('a', 'b')");
            verify(externalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelConflict.conflictingMethod'](1),
                "DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelConflict.ConflictingMethod(int) Called",
                "externalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelConflict.conflictingMethod'](1)");

            verify(externalDiffCase.AddEventListener(),
                "DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.AddEventListener() called",
                "externalDiffCase.AddEventListener()");
            verify(externalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener'],
                "DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener",
                "externalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener']");

            externalDiffCase.addEventListener("DevTests.CamelCasing.SimpleNameCollisions.IExternalEventConflict.conflictingevent", callback);
            externalDiffCase.addEventListener("DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelEventConflict.conflictingevent", callback);

            expectedStr = "DevTests.CamelCasing.SimpleNameCollisions.IExternalEventConflict.ConflictingEvent";
            externalDiffCase.fireEvent(events.externalEvent);
            expectedStr = "DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelEventConflict.conflictingEvent";
            externalDiffCase.fireEvent(events.externalCamelEvent);

            externalDiffCase.removeEventListener("DevTests.CamelCasing.SimpleNameCollisions.IExternalEventConflict.conflictingevent", callback);
            externalDiffCase.removeEventListener("DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelEventConflict.conflictingevent", callback);

            expectedStr = "";
            externalDiffCase.fireEvent(events.externalEvent);
            externalDiffCase.fireEvent(events.externalCamelEvent);
            verify(timesCalled, 2, "Total number of callbacks");
            timesCalled = 0;

            var internalAndExternal = new nameCollisions.InternalConflictWithExternalConflict();
            var internalAndExternalExpected = {
                'DevTests.CamelCasing.SimpleNameCollisions.IInternalConflict.conflictingProperty': 'number',
                'DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.conflictingProperty': 'string',
                'conflictingMethod': 'function', // This became an overload by the runtime class metadata switchover
                'toString': 'function'
            };
            verify.members(internalAndExternal, internalAndExternalExpected, internalAndExternal.toString());
            verify(internalAndExternal.conflictingMethod(1),
                "DevTests.CamelCasing.SimpleNameCollisions.IInternalConflict.ConflictingMethod(int) Called",
                "internalAndExternal.conflictingMethod(1)");
            verify(internalAndExternal.conflictingMethod('a', 'b'),
                "DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingMethod(HSTRING, HSTRING) Called",
                "internalAndExternal.conflictingMethod('a', 'b')");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Static Name Collision Cases',
        pri: '0',
        test: function () {
            /// Note: These cases may fail in the future. MIDL may be blocking case insensitive conflicts across the static
            /// interfaces of a runtime class. Currently, they only block case sensitive conflicts.
            var nameCollisions = DevTests.CamelCasing.SimpleNameCollisions;
            events = nameCollisions.ConflictingEvents;

            var eventsExpected = {
                internalPascalEvent: 0,
                internalCamelEvent: 1,
                externalEvent: 2,
                externalCamelEvent: 3
            };

            var eventsUnexpected = {
                InternalPascalEvent: 'undefined',
                InternalCamelEvent: 'undefined',
                ExternalEvent: 'undefined',
                ExternalCamelEvent: 'undefined'
            };

            verify.fields(events, eventsExpected, eventsUnexpected, "DevTests.CamelCasing.SimpleNameCollisions.ConflictingEvents");

            var staticInternalConflict = nameCollisions.StaticInternalConflict;
            var staticInternalConflictExpected = {
                addEventListener: 'function',
                removeEventListener: 'function',
                fireEvent: 'function',
                conflictingProperty: 'number',
                conflictingMethod: 'function',
                'DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener': 'function',
                RemoveEventListener: 'function',
                onconflictingevent: 'object'
            };
            verify.members(staticInternalConflict, staticInternalConflictExpected, "DevTests.CamelCasing.SimpleNameCollisions.StaticInternalConflict");
            verify(staticInternalConflict.conflictingMethod(1),
                "DevTests.CamelCasing.SimpleNameCollisions.IInternalConflict.ConflictingMethod(int) Called",
                "staticInternalConflict.conflictingMethod(1)");
            verify(staticInternalConflict['DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener'](),
                "DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener() called",
                "staticInternalConflict['DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener']()");
            verify(staticInternalConflict.RemoveEventListener(),
                "DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.RemoveEventListener() called",
                "staticInternalConflict.RemoveEventListener()");

            var expectedStr;
            var timesCalled = 0;
            function callback(ev) {
                verify(ev.detail[0], expectedStr, "Event fired");
                timesCalled++;
            }
            staticInternalConflict.addEventListener('conflictingevent', callback);

            expectedStr = "DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.ConflictingEvent";
            staticInternalConflict.fireEvent(events.internalPascalEvent);
            staticInternalConflict.removeEventListener('conflictingevent', callback);

            expectedStr = "";
            staticInternalConflict.fireEvent(events.internalPascalEvent);
            verify(timesCalled, 1, "Total number of callbacks");
            timesCalled = 0;

            var staticExternalDiffCase = nameCollisions.StaticExternalConflictDifferentCase;
            var staticExternalDiffCaseExpected = {
                addEventListener: 'function',
                removeEventListener: 'function',
                fireEvent: 'function',
                conflictingProperty: 'string',
                onconflictingevent: 'object',
                conflictingMethod: 'function',
                'DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.addEventListener': 'function',
                'DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener': 'string'
            };
            verify.members(staticExternalDiffCase, staticExternalDiffCaseExpected, "DevTests.CamelCasing.SimpleNameCollisions.StaticExternalConflictDifferentCase");
            verify(staticExternalDiffCase['conflictingMethod']('a', 'b'),
                "DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingMethod(HSTRING, HSTRING) Called",
                "staticExternalDiffCase['conflictingMethod']('a', 'b')");

            verify(staticExternalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.addEventListener'](),
                "DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.AddEventListener() called",
                "staticExternalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.addEventListener']()");
            verify(staticExternalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener'],
                "DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener",
                "staticExternalDiffCase['DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener']");

            staticExternalDiffCase.addEventListener("conflictingevent", callback);

            expectedStr = "DevTests.CamelCasing.SimpleNameCollisions.IExternalEventConflict.ConflictingEvent";
            staticExternalDiffCase.fireEvent(events.externalEvent);

            staticExternalDiffCase.removeEventListener("conflictingevent", callback);

            expectedStr = "";
            staticExternalDiffCase.fireEvent(events.externalEvent);
            verify(timesCalled, 1, "Total number of callbacks");
            timesCalled = 0;
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Event name string variations',
        pri: '0',
        test: function () {
            var stringVariations = new DevTests.CamelCasing.StringVariations();
            verify.defined(stringVariations.addEventListener, 'stringVariations.addEventListener');
            verify.defined(stringVariations.removeEventListener, 'stringVariations.removeEventListener');

            var events = DevTests.CamelCasing.StringVariationsEvent;

            var eventsExpected = {
                pascalEvent: 0,
                uppercaseevent: 1,
                iinterfaceEvent: 2,
                camelEvent: 3,
                _PrivatePascalEvent: 4,
                _PRIVATEUPPERCASEEVENT: 5,
                _IInterfacePrivateEvent: 6,
                _privateCamelEvent: 7,
                f8Event: 8,
                ecdh521Event: 9,
                uint16Event: 10,
                noncased_EVENT: 11,
                uiTwoLetterAcronymEvent: 12
            };

            var eventsUnexpected = {
                PascalEvent: 'undefined',
                UPPERCASEEVENT: 'undefined',
                IInterfaceEvent: 'undefined',
                _privatePascalEvent: 'undefined',
                F8Event: 'undefined',
                ECDH521Event: 'undefined',
                UInt16Event: 'undefined',
                NONCASED_EVENT: 'undefined',
                UITwoLetterAcronymEvent: 'undefined',
                uitwoLetterAcronymEvent: 'undefined'
            };

            verify.fields(events, eventsExpected, eventsUnexpected, "DevTests.CamelCasing.StringVariationsEvent");

            var timesCalled = 0;
            var expectedEvent;
            function callback(ev) {
                verify(ev.detail[0], expectedEvent, "Event fired");
                timesCalled++;
            }

            // add listener for all events
            stringVariations.addEventListener('pascalevent', callback);
            stringVariations.addEventListener('uppercaseevent', callback);
            stringVariations.addEventListener('iinterfaceevent', callback);
            stringVariations.addEventListener('camelevent', callback);
            stringVariations.addEventListener('_privatepascalevent', callback);
            stringVariations.addEventListener('_privateuppercaseevent', callback);
            stringVariations.addEventListener('_iinterfaceprivateevent', callback);
            stringVariations.addEventListener('_privatecamelevent', callback);
            stringVariations.addEventListener('f8event', callback);
            stringVariations.addEventListener('ecdh521event', callback);
            stringVariations.addEventListener('uint16event', callback);
            stringVariations.addEventListener('noncased_event', callback);
            stringVariations.addEventListener('uitwoletteracronymevent', callback);

            // ensure expected result for all events fired
            expectedEvent = "PascalEvent";
            stringVariations.fireEvent(events.pascalEvent);
            expectedEvent = "UPPERCASEEVENT";
            stringVariations.fireEvent(events.uppercaseevent);
            expectedEvent = "IInterfaceEvent";
            stringVariations.fireEvent(events.iinterfaceEvent);
            expectedEvent = "camelEvent";
            stringVariations.fireEvent(events.camelEvent);
            expectedEvent = "_PrivatePascalEvent";
            stringVariations.fireEvent(events._PrivatePascalEvent);
            expectedEvent = "_PRIVATEUPPERCASEEVENT";
            stringVariations.fireEvent(events._PRIVATEUPPERCASEEVENT);
            expectedEvent = "_IInterfacePrivateEvent";
            stringVariations.fireEvent(events._IInterfacePrivateEvent);
            expectedEvent = "_privateCamelEvent";
            stringVariations.fireEvent(events._privateCamelEvent);
            expectedEvent = "F8Event";
            stringVariations.fireEvent(events.f8Event);
            expectedEvent = "ECDH521Event";
            stringVariations.fireEvent(events.ecdh521Event);
            expectedEvent = "UInt16Event";
            stringVariations.fireEvent(events.uint16Event);
            expectedEvent = "NONCASED_EVENT";
            stringVariations.fireEvent(events.noncased_EVENT);
            expectedEvent = "UITwoLetterAcronymEvent";
            stringVariations.fireEvent(events.uiTwoLetterAcronymEvent);

            // remove event listeners
            stringVariations.removeEventListener('pascalevent', callback);
            stringVariations.removeEventListener('uppercaseevent', callback);
            stringVariations.removeEventListener('iinterfaceevent', callback);
            stringVariations.removeEventListener('camelevent', callback);
            stringVariations.removeEventListener('_privatepascalevent', callback);
            stringVariations.removeEventListener('_privateuppercaseevent', callback);
            stringVariations.removeEventListener('_iinterfaceprivateevent', callback);
            stringVariations.removeEventListener('_privatecamelevent', callback);
            stringVariations.removeEventListener('f8event', callback);
            stringVariations.removeEventListener('ecdh521event', callback);
            stringVariations.removeEventListener('uint16event', callback);
            stringVariations.removeEventListener('noncased_event', callback);
            stringVariations.removeEventListener('uitwoletteracronymevent', callback);

            // ensure callback not called for any events fired
            expectedEvent = "";
            stringVariations.fireEvent(events.pascalEvent);
            stringVariations.fireEvent(events.uppercaseevent);
            stringVariations.fireEvent(events.iinterfaceEvent);
            stringVariations.fireEvent(events.camelEvent);
            stringVariations.fireEvent(events._PrivatePascalEvent);
            stringVariations.fireEvent(events._PRIVATEUPPERCASEEVENT);
            stringVariations.fireEvent(events._IInterfacePrivateEvent);
            stringVariations.fireEvent(events._privateCamelEvent);
            stringVariations.fireEvent(events.f8Event);
            stringVariations.fireEvent(events.ecdh521Event);
            stringVariations.fireEvent(events.uint16Event);
            stringVariations.fireEvent(events.noncased_EVENT);
            stringVariations.fireEvent(events.uiTwoLetterAcronymEvent);

            verify(timesCalled, 13, "Total number of callbacks");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Static event name string variations',
        pri: '0',
        test: function () {
            var stringVariationsStatic = DevTests.CamelCasing.StringVariations;
            verify.defined(stringVariationsStatic.addEventListener, 'stringVariationsStatic.addEventListener');
            verify.defined(stringVariationsStatic.removeEventListener, 'stringVariationsStatic.removeEventListener');

            var events = DevTests.CamelCasing.StaticStringVariationsEvent;

            var eventsExpected = {
                pascalStaticEvent: 0,
                uppercasestaticevent: 1,
                iinterfaceStaticEvent: 2,
                camelStaticEvent: 3,
                _PrivatePascalStaticEvent: 4,
                _PRIVATEUPPERCASESTATICEVENT: 5,
                _IInterfacePrivateStaticEvent: 6,
                _privateCamelStaticEvent: 7
            };

            var eventsUnexpected = {
                PascalStaticEvent: 'undefined',
                UPPERCASESTATICEVENT: 'undefined',
                IInterfaceStaticEvent: 'undefined',
                _privatePascalStaticEvent: 'undefined'
            };

            verify.fields(events, eventsExpected, eventsUnexpected, "DevTests.CamelCasing.StaticStringVariationsEvent");

            var timesCalled = 0;
            var expectedEvent;
            function callback(ev) {
                verify(ev.detail[0], expectedEvent, "Event fired");
                timesCalled++;
            }

            // add listener for all events
            stringVariationsStatic.addEventListener('pascalstaticevent', callback);
            stringVariationsStatic.addEventListener('uppercasestaticevent', callback);
            stringVariationsStatic.addEventListener('iinterfacestaticevent', callback);
            stringVariationsStatic.addEventListener('camelstaticevent', callback);
            stringVariationsStatic.addEventListener('_privatepascalstaticevent', callback);
            stringVariationsStatic.addEventListener('_privateuppercasestaticevent', callback);
            stringVariationsStatic.addEventListener('_iinterfaceprivatestaticevent', callback);
            stringVariationsStatic.addEventListener('_privatecamelstaticevent', callback);

            // ensure expected result for all events fired
            expectedEvent = "PascalStaticEvent";
            stringVariationsStatic.staticFireEvent(events.pascalStaticEvent);
            expectedEvent = "UPPERCASESTATICEVENT";
            stringVariationsStatic.staticFireEvent(events.uppercasestaticevent);
            expectedEvent = "IInterfaceStaticEvent";
            stringVariationsStatic.staticFireEvent(events.iinterfaceStaticEvent);
            expectedEvent = "camelStaticEvent";
            stringVariationsStatic.staticFireEvent(events.camelStaticEvent);
            expectedEvent = "_PrivatePascalStaticEvent";
            stringVariationsStatic.staticFireEvent(events._PrivatePascalStaticEvent);
            expectedEvent = "_PRIVATEUPPERCASESTATICEVENT";
            stringVariationsStatic.staticFireEvent(events._PRIVATEUPPERCASESTATICEVENT);
            expectedEvent = "_IInterfacePrivateStaticEvent";
            stringVariationsStatic.staticFireEvent(events._IInterfacePrivateStaticEvent);
            expectedEvent = "_privateCamelStaticEvent";
            stringVariationsStatic.staticFireEvent(events._privateCamelStaticEvent);

            // remove event listeners
            stringVariationsStatic.removeEventListener('pascalstaticevent', callback);
            stringVariationsStatic.removeEventListener('uppercasestaticevent', callback);
            stringVariationsStatic.removeEventListener('iinterfacestaticevent', callback);
            stringVariationsStatic.removeEventListener('camelstaticevent', callback);
            stringVariationsStatic.removeEventListener('_privatepascalstaticevent', callback);
            stringVariationsStatic.removeEventListener('_privateuppercasestaticevent', callback);
            stringVariationsStatic.removeEventListener('_iinterfaceprivatestaticevent', callback);
            stringVariationsStatic.removeEventListener('_privatecamelstaticevent', callback);

            // ensure callback not called for any events fired
            expectedEvent = "";
            stringVariationsStatic.staticFireEvent(events.pascalStaticEvent);
            stringVariationsStatic.staticFireEvent(events.uppercasestaticevent);
            stringVariationsStatic.staticFireEvent(events.iinterfaceStaticEvent);
            stringVariationsStatic.staticFireEvent(events.camelStaticEvent);
            stringVariationsStatic.staticFireEvent(events._PrivatePascalStaticEvent);
            stringVariationsStatic.staticFireEvent(events._PRIVATEUPPERCASESTATICEVENT);
            stringVariationsStatic.staticFireEvent(events._IInterfacePrivateStaticEvent);
            stringVariationsStatic.staticFireEvent(events._privateCamelStaticEvent);

            verify(timesCalled, 8, "Total number of callbacks");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Collisions between member types',
        pri: '0',
        test: function () {
            var cmCollisions = DevTests.CamelCasing.CrossMemberCollisions;

            var internalCMConflict = new cmCollisions.InternalCrossMemberConflict();
            var internalCMConflictExpected = {
                conflicting: 'string',
                conflictingMethod: 'function',
                'toString': 'function'
            };
            verify.members(internalCMConflict, internalCMConflictExpected, internalCMConflict.toString());

            verify(internalCMConflict.conflicting,
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting",
                "internalCMCollison.conflicting");
            verify(internalCMConflict.conflictingMethod(1),
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.ConflictingMethod(int) Called",
                "internalCMCollison.conflictingMethod(1)");

            var externalPPConflict = new cmCollisions.ExternalPropPropConflict();
            var externalPPConflictExpected = {
                conflictingMethod: 'function',
                'DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting': 'string',
                'DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting': 'number',
                'toString': 'function'
            };
            verify.members(externalPPConflict, externalPPConflictExpected, externalPPConflict.toString());

            verify(externalPPConflict.conflictingMethod(1),
                 "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.ConflictingMethod(int) Called",
                 "externalPPConflict.conflictingMethod(1)");
            verify(externalPPConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting'],
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting",
                "externalPPConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting']");
            verify(externalPPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting'], 0,
                "externalPPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting']");
            externalPPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting'] = 42;
            verify(externalPPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting'], 42,
                "externalPPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting']");

            var externalPMConflict = new cmCollisions.ExternalPropMethodConflict();
            var externalPMConflictExpected = {
                conflictingMethod: 'function',
                'DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting': 'string',
                'DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.conflicting': 'function',
                'toString': 'function'
            };
            verify.members(externalPMConflict, externalPMConflictExpected, externalPMConflict.toString());

            verify(externalPMConflict.conflictingMethod(1),
                 "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.ConflictingMethod(int) Called",
                 "externalPMConflict.conflictingMethod(1)");
            verify(externalPMConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting'],
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting",
                "externalPMConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting']");
            verify(externalPMConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.conflicting']('a', 'b'),
                "DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.Conflicting(HSTRING,HSTRING) Called",
                "externalPMConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.conflicting']('a', 'b')");

            var externalMMConflict = new cmCollisions.ExternalMethodMethodConflict();
            var externalMMConflictExpected = {
                conflictingProp: 'string',
                'DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting': 'function',
                'DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.conflicting': 'function',
                'toString': 'function'
            };
            verify.members(externalMMConflict, externalMMConflictExpected, externalMMConflict.toString());

            verify(externalMMConflict.conflictingProp,
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.ConflictingProp",
                "externalMMConflict.conflictingProp");
            verify(externalMMConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting'](1),
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting(int) Called",
                "externalMMConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting'](1)");
            verify(externalMMConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.conflicting']('a', 'b'),
                "DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.Conflicting(HSTRING,HSTRING) Called",
                "externalMMConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.conflicting']('a', 'b')");

            var externalMPConflict = new cmCollisions.ExternalMethodPropConflict();
            var externalMPConflictExpected = {
                conflictingProp: 'string',
                'DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting': 'function',
                'DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting': 'number',
                'toString': 'function'
            };
            verify.members(externalMPConflict, externalMPConflictExpected, externalMPConflict.toString());

            verify(externalMPConflict.conflictingProp,
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.ConflictingProp",
                "externalMPConflict.conflictingProp");
            verify(externalMPConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting'](1),
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting(int) Called",
                "externalMPConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting'](1)");
            verify(externalMPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting'], 0,
                "externalMPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting']");
            externalMPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting'] = 42;
            verify(externalMPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting'], 42,
                "externalMPConflict['DevTests.CamelCasing.CrossMemberCollisions.IExternalPropertyConflict.conflicting']");

            var doubleCMConflict = new cmCollisions.DoubleCrossMemberConflict();
            var doubleCMConflictExpected = {
                'DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting': 'string',
                conflictingMethod: 'function',
                'DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting': 'function',
                conflictingProp: 'string',
                'toString': 'function'
            };
            verify.members(doubleCMConflict, doubleCMConflictExpected, doubleCMConflict.toString());

            verify(doubleCMConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting'],
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting",
                "doubleCMConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting']");
            verify(doubleCMConflict['conflictingMethod'](1),
                 "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.ConflictingMethod(int) Called",
                 "doubleCMConflict['conflictingMethod'](1)");
            verify(doubleCMConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting'](1),
                "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting(int) Called",
                "doubleCMConflict['DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting'](1)");
            verify(doubleCMConflict['conflictingProp'],
                 "DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.ConflictingProp",
                 "doubleCMConflict['conflictingProp']");
        }
    });

    runner.addTest({
        id: 10,
        desc: 'Collisions with built in properties',
        pri: '0',
        test: function () {
            var cmCollisions = DevTests.CamelCasing.CrossMemberCollisions;

            var enumFieldsUnexpected = {
                Apply: 'undefined',
                'DevTests.CamelCasing.CrossMemberCollisions.ConflictsWithBuiltIns.call': 'undefined',
                'DevTests.CamelCasing.CrossMemberCollisions.ConflictsWithBuiltIns.hasOwnProperty': 'undefined',
                IsPrototypeOf: 'undefined',
                'DevTests.CamelCasing.CrossMemberCollisions.ConflictsWithBuiltIns.propertyIsEnumerable': 'undefined',
                prototype: 'undefined',
                'DevTests.CamelCasing.CrossMemberCollisions.ConflictsWithBuiltIns.toLocalString': 'undefined',
                ToString: 'undefined',
                'DevTests.CamelCasing.CrossMemberCollisions.ConflictsWithBuiltIns.valueOf': 'undefined'
            };

            var enumFieldsExpected = {
                apply: 0,
                call: 1,
                Constructor: 2,
                hasOwnProperty: 3,
                isPrototypeOf: 4,
                propertyIsEnumerable: 5,
                'DevTests.CamelCasing.CrossMemberCollisions.ConflictsWithBuiltIns.prototype': 6,
                toLocalString: 7,
                toString: 8,
                valueOf: 9
            };

            verify.fields(cmCollisions.ConflictsWithBuiltIns, enumFieldsExpected, enumFieldsUnexpected, "DevTests.CamelCasing.CrossMemberCollisions.ConflictsWithBuiltIns");

            verify.exception(function () {
                "" + cmCollisions.ConflictsWithBuiltIns;
            }, TypeError, "toString of object not a function");

            var builtInConflicts = new cmCollisions.BuiltInConflicts();
            var builtInConflictsExpected = {
                apply: 'string',
                'DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor': 'function',
                isPrototypeOf: 'string',
                'Prototype': 'string',
                toString: 'number',
                call: 'function',
                hasOwnProperty: 'number',
                propertyIsEnumerable: 'function',
                toLocalString: 'function',
                valueOf: 'function',
                length: 'function',
                structBuiltInConflict: 'object'
            };
            verify.members(builtInConflicts, builtInConflictsExpected, Object.prototype.toString.call(builtInConflicts));

            verify(builtInConflicts.apply, "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Apply", "builtInConflicts.apply");
            verifyAndWrite(builtInConflicts['hasOwnProperty'], 0, 125, "builtInConflicts['hasOwnProperty']");
            verify(builtInConflicts.isPrototypeOf, "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.isPrototypeOf", "builtInConflicts.isPrototypeOf");
            verify(builtInConflicts.Prototype,
                "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Prototype",
                "builtInConflicts.Prototype");
            verifyAndWrite(builtInConflicts.toString, 0, 24, "builtInConflicts.toString");
            verify(builtInConflicts.call(1), "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Call(int) Called", "builtInConflicts.call(1)");
            verify(builtInConflicts['DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor'](1),
                "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor(int) Called",
                "builtInConflicts['DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor'](1)");
            verify(builtInConflicts.propertyIsEnumerable(1), "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.PropertyIsEnumerable(int) Called", "builtInConflicts.propertyIsEnumerable(1)");
            verify(builtInConflicts['toLocalString'](1),
                "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.toLocalString(int) Called",
                "builtInConflicts['toLocalString'](1)");
            verify(builtInConflicts.valueOf(1), "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.ValueOf(int) Called", "builtInConflicts.valueOf(1)");
            verify(builtInConflicts.length(1), "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Length(int) Called", "builtInConflicts.length(1)");

            var structConflict = builtInConflicts.structBuiltInConflict;
            var structConflictExpected = {
                apply: 'number',
                'DevTests.CamelCasing.CrossMemberCollisions.StructBuiltInConflict.constructor': 'number',
                isPrototypeOf: 'number',
                Prototype: 'number',
                //toString: 'number',
                call: 'number',
                hasOwnProperty: 'number',
                propertyIsEnumerable: 'number',
                toLocalString: 'number',
                valueOf: 'number'
            };
            verify.members(structConflict, structConflictExpected, Object.prototype.toString.call(structConflict));
            verify(structConflict['apply'], 0, "structConflict['apply']");
            verify(structConflict['DevTests.CamelCasing.CrossMemberCollisions.StructBuiltInConflict.constructor'], 0, "structConflict['DevTests.CamelCasing.CrossMemberCollisions.StructBuiltInConflict.constructor']");
            verify(structConflict['isPrototypeOf'], 0, "structConflict['isPrototypeOf']");
            verify(structConflict['Prototype'], 0, "structConflict['Prototype']");
            //verify(structConflict['toString'], 0, "structConflict['toString']");
            verify(structConflict.call, 0, "structConflict.call");
            verify(structConflict.hasOwnProperty, 0, "structConflict.hasOwnProperty");
            verify(structConflict.propertyIsEnumerable, 0, "structConflict.propertyIsEnumerable");
            verify(structConflict.toLocalString, 0, "structConflict.toLocalString");
            verify(structConflict.valueOf, 0, "structConflict.valueOf");

            builtInConflicts.structBuiltInConflict = {
                'apply': 42,
                'DevTests.CamelCasing.CrossMemberCollisions.StructBuiltInConflict.constructor': 23,
                'isPrototypeOf': 156,
                'Prototype': 74,
                //'toString': 81,
                call: 99,
                hasOwnProperty: 3,
                propertyIsEnumerable: 1000,
                toLocalString: 51,
                valueOf: 1
            };
            structConflict = builtInConflicts.structBuiltInConflict;

            verify(structConflict['apply'], 42, "structConflict['apply']");
            verify(structConflict['DevTests.CamelCasing.CrossMemberCollisions.StructBuiltInConflict.constructor'], 23, "structConflict['DevTests.CamelCasing.CrossMemberCollisions.StructBuiltInConflict.constructor']");
            verify(structConflict['isPrototypeOf'], 156, "structConflict['isPrototypeOf']");
            verify(structConflict['Prototype'], 74, "structConflict['Prototype']");
            //verify(structConflict['toString'], 81, "structConflict['toString']");
            verify(structConflict.call, 99, "structConflict.call");
            verify(structConflict.hasOwnProperty, 3, "structConflict.hasOwnProperty");
            verify(structConflict.propertyIsEnumerable, 1000, "structConflict.propertyIsEnumerable");
            verify(structConflict.toLocalString, 51, "structConflict.toLocalString");
            verify(structConflict.valueOf, 1, "structConflict.valueOf");
        }
    });

    runner.addTest({
        id: 11,
        desc: 'Static collisions with built in properties',
        pri: '0',
        test: function () {
            var cmCollisions = DevTests.CamelCasing.CrossMemberCollisions;
            var staticBuiltInConflicts = cmCollisions.BuiltInConflictsStatic;
            var staticBuiltInConflictsExpected = {
                apply: 'string',
                'DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor': 'function',
                isPrototypeOf: 'string',
                'Prototype': 'string',
                toString: 'number',
                call: 'function',
                hasOwnProperty: 'number',
                propertyIsEnumerable: 'function',
                toLocalString: 'function',
                valueOf: 'function',
                'Length': 'function'
            };
            verify.members(staticBuiltInConflicts, staticBuiltInConflictsExpected, "DevTests.CamelCasing.CrossMemberCollisions.BuiltInConflicts()");

            verify(staticBuiltInConflicts.apply, "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Apply", "staticBuiltInConflicts.apply");
            verifyAndWrite(staticBuiltInConflicts['hasOwnProperty'], 0, 125, "staticBuiltInConflicts['hasOwnProperty']");
            verify(staticBuiltInConflicts.isPrototypeOf, "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.isPrototypeOf", "staticBuiltInConflicts.isPrototypeOf");
            verify(staticBuiltInConflicts.Prototype,
                "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Prototype",
                "staticBuiltInConflicts.Prototype");
            verifyAndWrite(staticBuiltInConflicts.toString, 0, 24, "staticBuiltInConflicts.toString");
            verify(staticBuiltInConflicts.call(1), "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Call(int) Called", "staticBuiltInConflicts.call(1)");
            verify(staticBuiltInConflicts['DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor'](1),
                "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor(int) Called",
                "staticBuiltInConflicts['DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor'](1)");
            verify(staticBuiltInConflicts.propertyIsEnumerable(1), "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.PropertyIsEnumerable(int) Called", "staticBuiltInConflicts.propertyIsEnumerable(1)");
            verify(staticBuiltInConflicts['toLocalString'](1),
                "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.toLocalString(int) Called",
                "staticBuiltInConflicts['toLocalString'](1)");
            verify(staticBuiltInConflicts.valueOf(1), "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.ValueOf(int) Called", "staticBuiltInConflicts.valueOf(1)");
            verify(staticBuiltInConflicts.Length(1), "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Length(int) Called", "staticBuiltInConflicts.Length(1)");
            verify(staticBuiltInConflicts.length, 0, "staticBuiltInConflicts.length");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Length property conflicts',
        pri: '0',
        test: function () {
            var cmCollisions = DevTests.CamelCasing.CrossMemberCollisions;

            logger.comment("'Length' prototype member conflicting with vector length")
            var vectorLengthConflict = new cmCollisions.VectorLengthConflict();
            var vectorLengthConflictExpected = {
                apply: 'string',
                'DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor': 'function',
                isPrototypeOf: 'string',
                'Prototype': 'string',
                toString: 'number',
                call: 'function',
                hasOwnProperty: 'number',
                propertyIsEnumerable: 'function',
                toLocalString: 'function',
                valueOf: 'function',
                first: 'function',
                getAt: 'function',
                size: 'number',
                indexOf: 'function',
                getMany: 'function',
                getView: 'function',
                setAt: 'function',
                insertAt: 'function',
                removeAt: 'function',
                replaceAll: 'function',
                append: 'function',
                removeAtEnd: 'function',
                clear: 'function',
                length: 'number',
                '0': 'number',
                '1': 'number',
                '2': 'number',
                '3': 'number',
                '4': 'number',
                '5': 'number',
                '6': 'number',
                '7': 'number',
                '8': 'number',
                '9': 'number'
            };
            verify.members(vectorLengthConflict, vectorLengthConflictExpected, Object.prototype.toString.call(vectorLengthConflict));

            var vectorLengthConflictPrototype = Object.getPrototypeOf(vectorLengthConflict);
            verify(vectorLengthConflictPrototype.length.call(vectorLengthConflict, 1), "DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Length(int) Called", "vectorLengthConflictPrototype.length.call(vectorLengthConflict, 1)");
            verify(vectorLengthConflict.length, 9, "vectorLengthConflict.length");

            logger.comment("'length' static member conflicting with constructor length");
            var camelLengthConflictStatic = cmCollisions.CamelLengthConflict;
            var camelLengthConflictStaticExpected = {
                'DevTests.CamelCasing.CrossMemberCollisions.ICamelLengthConflict.length': 'function'
            };
            verify.members(camelLengthConflictStatic, camelLengthConflictStaticExpected, "DevTests.CamelCasing.CrossMemberCollisions.CamelLengthConflict()");

            verify(camelLengthConflictStatic['DevTests.CamelCasing.CrossMemberCollisions.ICamelLengthConflict.length'](1),
                "DevTests.CamelCasing.CrossMemberCollisions.ICamelLengthConflict.length(int) Called",
                "camelLengthConflictStatic['DevTests.CamelCasing.CrossMemberCollisions.ICamelLengthConflict.length'](1)");

            verify.typeOf(camelLengthConflictStatic.length, "number");
            verify(camelLengthConflictStatic.length, 0, "camelLengthConflictStatic.length");

            logger.comment("'Length' static member conflicting with constructor length");
            var pascalLengthConflictStatic = cmCollisions.PascalLengthConflict;
            var pascalLengthConflictStaticExpected = {
                Length: 'string'
            };
            verify.members(pascalLengthConflictStatic, pascalLengthConflictStaticExpected, "DevTests.CamelCasing.CrossMemberCollisions.PascalLengthConflict()");

            verify(pascalLengthConflictStatic.Length,
                "DevTests.CamelCasing.CrossMemberCollisions.IPascalLengthConflict.Length",
                "pascalLengthConflictStatic.Length");

            verify.typeOf(pascalLengthConflictStatic.length, "number");
            verify(pascalLengthConflictStatic.length, 0, "pascalLengthConflictStatic.length");
        }
    });

    Loader42_FileName = "Camel Casing tests"
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
