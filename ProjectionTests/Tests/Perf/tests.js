(function() {
    var NUMBER = 42;
    var SHORT_STRING = "ten chars.";
    var LONG_STRING  = "This string is one hundred characters long. It is more representative of larger strings that get use";
    var global = Function("return this")();
    var ITERATIONS;

    if(typeof global.CHAKRA_PROJECTIONS_ITERATION_COUNT === "number") {
        ITERATIONS = global.CHAKRA_PROJECTIONS_ITERATION_COUNT;
    } else {
        ITERATIONS = 100; // PerfBench default
    }

    var GUID_STRING = "21EC2020-3AEA-1069-A2DD-08002B30309D"

    var obj1;
    var obj2;
    var counter = 1;


    // Create an object when we start testing
    runner.globalSetup(function() {
        obj1 = new Windows.RuntimeTest.Example.SampleTestObjectBothInProc();
        obj2 = new Windows.RuntimeTest.Example.SampleTestObject2();
    });

    var COLLECT_GARBAGE_COUNT = 10;

    function collectGarbage() {
        for(var i = 0; i < COLLECT_GARBAGE_COUNT; i++) {
            CollectGarbage();
        }
    }

    if(typeof CollectGarbage !== "undefined") {
        runner.subscribe('testStart', collectGarbage);
        runner.subscribe('testEnd', collectGarbage);
    }





    runner.addTest({
        id: counter++,
        desc: "Void",
        tags: ['innerIterations 50000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0; 
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_VoidArgumentsCall_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.voidArgumentsCall();
                msWriteProfilerMark("JS_VoidArgumentsCall_End");
            }
        }
    });

    // INT 16
    runner.addTest({
        id: counter++,
        desc: "Int16 RT",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalInt16_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    var val = obj1.marshalINT16InOut(NUMBER);
                msWriteProfilerMark("JS_MarshalInt16_End");
            }
        }
    });
       
    runner.addTest({
        id: counter++,
        desc: "Int16 In",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalInt16In_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalINT16In(NUMBER);
                msWriteProfilerMark("JS_MarshalInt16In_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Int16 Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalInt16Out_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalINT16Out();
                msWriteProfilerMark("JS_MarshalInt16Out_End");
            }
        }
    });

    // UInt32
    runner.addTest({
        id: counter++,
        desc: "UInt32 RT",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalUInt32_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.marshalUINT32InOut(NUMBER);

                msWriteProfilerMark("JS_MarshalUInt32_End");
            }
        }
    });
    
    runner.addTest({
        id: counter++,
        desc: "UInt32 In",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalUInt32In_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT32In(NUMBER);

                msWriteProfilerMark("JS_MarshalUInt32In_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt32 Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalUInt32Out_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalUINT32Out();

                msWriteProfilerMark("JS_MarshalUInt32Out_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Int32 In",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalInt32In_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalINT32In(NUMBER);

                msWriteProfilerMark("JS_MarshalInt32In_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Int32 Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalInt32Out_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalINT32Out();

                msWriteProfilerMark("JS_MarshalInt32Out_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt64 In",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalUInt64In_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT64In(NUMBER);

                msWriteProfilerMark("JS_MarshalUInt64In_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt64 Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalUInt64Out_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalUINT64Out();

                msWriteProfilerMark("JS_MarshalUInt64Out_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Float In",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalFloatIn_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalFloatIn(NUMBER);

                msWriteProfilerMark("JS_MarshalFloatIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Float Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalFloatOut_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalFloatOut();

                msWriteProfilerMark("JS_MarshalFloatOut_End");
            }
        }
    });
    runner.addTest({
        id: counter++,
        desc: "Double RT",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalDouble_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.marshalDoubleInOut(NUMBER);
                msWriteProfilerMark("JS_MarshalDouble_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Double In",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalDoubleIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalDoubleIn(NUMBER);
                msWriteProfilerMark("JS_MarshalDoubleIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Double Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalDoubleOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalDoubleOut();
                msWriteProfilerMark("JS_MarshalDoubleOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Boolean In",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalBooleanIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalBooleanIn(NUMBER);
                msWriteProfilerMark("JS_MarshalBooleanIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Boolean Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_MarshalBooleanOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalBooleanOut();
                msWriteProfilerMark("JS_MarshalBooleanOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String RT - Size 10",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_Marshal10String_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.marshalStringInOut(SHORT_STRING);
                msWriteProfilerMark("JS_Marshal10String_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String In - Size 10",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_Marshal10StringIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringIn(SHORT_STRING);
                msWriteProfilerMark("JS_Marshal10StringIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String Out - Size 10",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            obj2.marshalStringIn(SHORT_STRING);

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_Marshal10StringOut_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringOut();

                msWriteProfilerMark("JS_Marshal10StringOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String RT - Size 100",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_Marshal100String_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.marshalStringInOut(LONG_STRING);

                msWriteProfilerMark("JS_Marshal100String_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String In - Size 100",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_Marshal100StringIn_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalStringIn(LONG_STRING);

                msWriteProfilerMark("JS_Marshal100StringIn_End");
            }
        }
    });


    runner.addTest({
        id: counter++,
        desc: "String Out - Size 100",
        tags: ['innerIterations 20000'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_Marshal100StringOut_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringOut();

                msWriteProfilerMark("JS_Marshal100StringOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String RT - Size 10K",
        tags: ['innerIterations 500'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            var str = "";
            for(var i = 0; i < 100; i++) {
                str += LONG_STRING;
            }

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_Marshal10kString_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.marshalStringInOut(str);

                msWriteProfilerMark("JS_Marshal10kString_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String In - Size 10K",
        tags: ['innerIterations 500'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var str = "";
            for(var i = 0; i < 100; i++) {
                str += LONG_STRING;
            }

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_Marshal10kStringIn_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalStringIn(str);

                msWriteProfilerMark("JS_Marshal10kStringIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String Out - Size 10K",
        tags: ['innerIterations 500'],
        test: function() {
            var val;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            var str = "";
            for(var i = 0; i < 100; i++) {
                str += LONG_STRING;
            }
            obj2.marshalStringIn(str);

            for(var i = 0; i < ITERATIONS; i++) {
                msWriteProfilerMark("JS_Marshal10KStringOut_Start");

                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringOut();

                msWriteProfilerMark("JS_Marshal10KStringOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Interface RT",
        tags: ['innerIterations 4000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObjectBothInProc();
            param.uniqueSentinel = 12345;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalObjAsInterface_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.marshalInterfaceInOut(param);
                msWriteProfilerMark("JS_MarshalObjAsInterface_End");
            }
        }
    });


    runner.addTest({
        id: counter++,
        desc: "Interface In",
        tags: ['innerIterations 4000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObject2();
            param.uniqueSentinel = 12345;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalObjAsInterfaceIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalInterfaceIn(param);
                msWriteProfilerMark("JS_MarshalObjAsInterfaceIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Interface Out",
        tags: ['innerIterations 2000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalObjAsInterfaceOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalInterfaceOut();
                msWriteProfilerMark("JS_MarshalObjAsInterfaceOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "RuntimeClass RT",
        tags: ['innerIterations 4000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObjectBothInProc();
            var INNER_ITERATIONS = this.tags[0].split(" ")[1];

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalObjAsRTC_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.marshalRuntimeClassInOutBothInProc(param);
                msWriteProfilerMark("JS_MarshalObjAsRTC_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "RuntimeClass In",
        tags: ['innerIterations 4000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObject2();
            param.uniqueSentinel = 12345;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalRuntimeClassIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalRuntimeClassIn(param);
                msWriteProfilerMark("JS_MarshalRuntimeClassIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "RuntimeClass Out",
        tags: ['innerIterations 2000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalRuntimeClassOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalRuntimeClassOut();
                msWriteProfilerMark("JS_MarshalRuntimeClassOut_End");
            }
        }
    });

    // Possible additional tests:
    // * Marshal any basic types as IInspectable
    // * Marshal an array of basic types or WinRT objects
    //
    runner.addTest({
        id: counter++,
        desc: "IInspectable RT",
        tags: ['innerIterations 4000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObjectBothInProc();
            param.uniqueSentinel = 12345;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalObjAsInspectable_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.marshalIInspectableInOut(param);
                msWriteProfilerMark("JS_MarshalObjAsInspectable_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "IInspectable In",
        tags: ['innerIterations 4000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObject2();
            param.uniqueSentinel = 12345;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalIInspectableIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalIInspectableIn(param);
                msWriteProfilerMark("JS_MarshalIInspectableIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "IInspectable Out",
        tags: ['innerIterations 4000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalIInspectableOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalIInspectableOut();
                msWriteProfilerMark("JS_MarshalIInspectableOut_End");
            }
        }
    });

    
    runner.addTest({
        id: counter++,
        desc: "Two Interfaces In",
        tags: ['innerIterations 2000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObjectBothInProc();
            param.uniqueSentinel = 12345;
            var param2 = new Windows.RuntimeTest.Example.SampleTestObject2();
            param2.uniqueSentinel = 54321;

            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalTwoInterfacesIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalTwoInterfacesIn(param, param2);
                msWriteProfilerMark("JS_MarshalTwoInterfacesIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Two Runtimeclass In",
        tags: ['innerIterations 2000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObjectBothInProc();
            param.uniqueSentinel = 12345;
            var param2 = new Windows.RuntimeTest.Example.SampleTestObject2();
            param2.uniqueSentinel = 54321;

            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalTwoRuntimeClassesIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalTwoRuntimeClassesIn(param, param2);
                msWriteProfilerMark("JS_MarshalTwoRuntimeClassesIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Interface In and Runtimeclass Out",
        tags: ['innerIterations 2000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObject2();
            param.uniqueSentinel = 12345;
            
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalInterfaceInRuntimeClassOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalInterfaceInRuntimeClassOut(param);
                msWriteProfilerMark("JS_MarshalInterfaceInRuntimeClassOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String In and Interface Out - Size 10",
        tags: ['innerIterations 4000'],
        test: function() {                        
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalString10InInterfaceOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringInInterfaceOut(SHORT_STRING);
                msWriteProfilerMark("JS_MarshalString10InInterfaceOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String In and Interface Out - Size 100",
        tags: ['innerIterations 2000'],
        test: function() {                        
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalString100InInterfaceOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringInInterfaceOut(LONG_STRING);
                msWriteProfilerMark("JS_MarshalString100InInterfaceOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String In and Interface Out - Size 10k",
        tags: ['innerIterations 1000'],
        test: function() {
        var str = "";
            for(var i = 0; i < 100; i++) {
                str += LONG_STRING;
            }
                            
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalString10kInInterfaceOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringInInterfaceOut(str);
                msWriteProfilerMark("JS_MarshalString10kInInterfaceOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String 10 In and Runtimeclass Out",
        tags: ['innerIterations 2000'],
        test: function() {                        
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalString10InRuntimeClassOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringInRuntimeClassOut(SHORT_STRING);
                msWriteProfilerMark("JS_MarshalString10InRuntimeClassOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String 100 In and Runtimeclass Out",
        tags: ['innerIterations 2000'],
        test: function() {                        
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalString100InRuntimeClassOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringInRuntimeClassOut(LONG_STRING);
                msWriteProfilerMark("JS_MarshalString100InRuntimeClassOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String 10k In and Runtimeclass Out",
        tags: ['innerIterations 1000'],
        test: function() {
        var str = "";
            for(var i = 0; i < 100; i++) {
                str += LONG_STRING;
            }
                            
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalString10kInRuntimeClassOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalStringInRuntimeClassOut(str);
                msWriteProfilerMark("JS_MarshalString10kInRuntimeClassOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Point In",
        tags: ['innerIterations 4000'],
        test: function() {
            var param = {x: 10.1, y: 100.1};
        var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalPointIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalPointIn(param);
                msWriteProfilerMark("JS_MarshalPointIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Point Out",
        tags: ['innerIterations 2000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalPointOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalPointOut();
                msWriteProfilerMark("JS_MarshalPointOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Rect In",
        tags: ['innerIterations 4000'],
        test: function() {
            var param = {x: 10.1, y: 100.1, width: 20.2, height: 30.3};
        var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalRectIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalRectIn(param);
                msWriteProfilerMark("JS_MarshalRectIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Rect Out",
        tags: ['innerIterations 1000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalRectOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalRectOut();
                msWriteProfilerMark("JS_MarshalRectOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Delegate Set",
        tags: ['innerIterations 4000'],
        test: function() {
            function handler(sender, newValue)
        {
        }

            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_DelegateSet_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.delegate = handler;
                msWriteProfilerMark("JS_DelegateSet_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Delegate Get",
        tags: ['innerIterations 4000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_DelegateGet_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    value = obj2.delegate;
                msWriteProfilerMark("JS_DelegateGet_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Event Handler Add/Remove",
        tags: ['innerIterations 20000'],
        test: function() {
            function callback(ev)
            {
            }

            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_EventAddRemove_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++) {
                   obj2.addEventListener("newsentinelsetcompletedevent", callback);
                   obj2.removeEventListener("newsentinelsetcompletedevent", callback);
                }    
                msWriteProfilerMark("JS_EventAddRemove_End");
            }
        }
    });


    // GUIDs are pretty expensive for Javascript. To get a better picture of struct performance we
    // we will require a struct without a GUID field.
    runner.addTest({
        id: counter++,
        desc: "Struct without string IN",
        tags: ['innerIterations 2000'],
        test: function() {
            var param = {
                doubleValue: NUMBER,
                integerValue: NUMBER,
                guidValue: GUID_STRING
            }
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalObjAsStructNSIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj1.structWithoutString = param;
                msWriteProfilerMark("JS_MarshalObjAsStructNSIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Struct without string OUT",
        tags: ['innerIterations 2000'],
        test: function() {
            var param = {
                doubleValue: NUMBER,
                integerValue: NUMBER,
                guidValue: GUID_STRING
            }
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    

                msWriteProfilerMark("JS_MarshalObjAsStructNSOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.structWithoutString;
                msWriteProfilerMark("JS_MarshalObjAsStructNSOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Struct with string IN",
        tags: ['innerIterations 2000'],
        test: function() {
            var param = {
                doubleValue: NUMBER,
                integerValue: NUMBER,
                guidValue: GUID_STRING,
                stringValue: SHORT_STRING
            }
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalObjAsStructIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj1.structWithString = param;
                msWriteProfilerMark("JS_MarshalObjAsStructIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Struct with string OUT",
        tags: ['innerIterations 2000'],
        test: function() {
            var param = {
                doubleValue: NUMBER,
                integerValue: NUMBER,
                guidValue: GUID_STRING,
                stringValue: SHORT_STRING
            }
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_MarshalObjAsStructOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj1.structWithString;
                msWriteProfilerMark("JS_MarshalObjAsStructOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "String Size 10 In, IAsyncInfo Out",
        tags: ['innerIterations 2000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var promise;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_String10InPromiseOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    promise = obj2.marshalStringInIAsyncInfoOut(SHORT_STRING);
                msWriteProfilerMark("JS_String10InPromiseOut_End");
            }
        }
    });


    runner.addTest({
        id: counter++,
        desc: "String Size 10 In, Boolean Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var bool;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_String10InBooleanOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    bool = obj2.marshalStringInBooleanOut(SHORT_STRING);
                msWriteProfilerMark("JS_String10InBooleanOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Interface In, IAsyncInfo Out",
        tags: ['innerIterations 2000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObject2();
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var promise;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_InterfaceInPromiseOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    promise = obj2.marshalInterfaceInIAsyncInfoOut(param);
                msWriteProfilerMark("JS_InterfaceInPromiseOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "RuntimeClass In, IAsyncInfo Out",
        tags: ['innerIterations 2000'],
        test: function() {
            var param = new Windows.RuntimeTest.Example.SampleTestObject2();
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var promise;

            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_RuntimeClassInPromiseOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    promise = obj2.marshalRuntimeClassInIAsyncInfoOut(param);
                msWriteProfilerMark("JS_RuntimeClassInPromiseOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "IAsyncInfo Out",
        tags: ['innerIterations 2000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var promise;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_PromiseOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    promise = obj2.marshalIAsyncInfoOut();
                msWriteProfilerMark("JS_PromiseOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Int32 In, String 10 Out",
        tags: ['innerIterations 20000'],
        test: function() {
            obj2.marshalStringIn(SHORT_STRING);
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var str;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_Int32InString10Out_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    str = obj2.marshalINT32InStringOut(NUMBER);
                msWriteProfilerMark("JS_Int32InString10Out_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "Int32 In, RuntimeClass Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var obj;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_Int32InRTCOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj = obj2.marshalINT32InRuntimeClassOut(NUMBER);
                msWriteProfilerMark("JS_Int32InRTCOut_End");
            }
        }
    });


    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Interface Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var obj;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InInterfaceOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj = obj2.marshalUINT32InInterfaceOut(NUMBER);
                msWriteProfilerMark("JS_UInt32InInterfaceOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "DateTime Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var date;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_DateTimeOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    date = obj2.marshalOutDateTime();
                msWriteProfilerMark("JS_DateTimeOut_End");
            }
        }
    });


    runner.addTest({
        id: counter++,
        desc: "Guid Out",
        tags: ['innerIterations 2000'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var Guid;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_GuidOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    Guid = obj2.marshalOutGuid();
                msWriteProfilerMark("JS_GuidOut_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "DateTime In, String 10 Out",
        tags: ['innerIterations 20000'],
        test: function() {
            obj2.marshalStringIn(SHORT_STRING);
            var param = new Date();
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var str;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_DateTimeInString10Out_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    str = obj2.marshalDateTimeInStringOut(param);
                msWriteProfilerMark("JS_DateTimeInString10Out_End");
            }
        }
    });
    
    runner.addTest({
        id: counter++,
        desc: "UInt32 In, String 10 Out",
        tags: ['innerIterations 20000'],
        test: function() {
            obj2.marshalStringIn(SHORT_STRING);
            var param = NUMBER;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var str;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InString10Out_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    str = obj2.marshalUINT32InStringOut(param);
                msWriteProfilerMark("JS_UInt32InString10Out_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Int32 Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var val;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InInt32Out_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalUINT32InINT32Out(param);
                msWriteProfilerMark("JS_UInt32InInt32Out_End");
            }
        }
    });
    
    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Boolean Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var val;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InBooleanOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalUINT32InBooleanOut(param);
                msWriteProfilerMark("JS_UInt32InBooleanOut_End");
            }
        }
    });
    
    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Float Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var val;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InFloatOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalUINT32InFloatOut(param);
                msWriteProfilerMark("JS_UInt32InFloatOut_End");
            }
        }
    });
    
    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Double Out",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            var val;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InDoubleOut_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    val = obj2.marshalUINT32InDoubleOut(param);
                msWriteProfilerMark("JS_UInt32InDoubleOut_End");
            }
        }
    });
    
    runner.addTest({
        id: counter++,
        desc: "UInt32 In, String 10 In",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var param2 = SHORT_STRING;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InString10In_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT32InStringIn(param,param2);
                msWriteProfilerMark("JS_UInt32InString10In_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt32 In, UInt32 In",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var param2 = NUMBER;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InUInt32In_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT32InUINT32In(param,param2);
                msWriteProfilerMark("JS_UInt32InUInt32In_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Int32 In",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var param2 = NUMBER;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InInt32In_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT32InINT32In(param,param2);
                msWriteProfilerMark("JS_UInt32InInt32In_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Boolean In",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var param2 = true;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InBooleanIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT32InBooleanIn(param,param2);
                msWriteProfilerMark("JS_UInt32InBooleanIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Float In",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var param2 = NUMBER;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InFloatIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT32InFloatIn(param,param2);
                msWriteProfilerMark("JS_UInt32InFloatIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Double In",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var param2 = NUMBER;
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InDoubleIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT32InDoubleIn(param,param2);
                msWriteProfilerMark("JS_UInt32InDoubleIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt32 In, RuntimeClass In",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var param2 = new Windows.RuntimeTest.Example.SampleTestObject2();
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InRTCIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT32InRuntimeClassIn(param,param2);
                msWriteProfilerMark("JS_UInt32InRTCIn_End");
            }
        }
    });

    runner.addTest({
        id: counter++,
        desc: "UInt32 In, Interface In",
        tags: ['innerIterations 20000'],
        test: function() {
            var param = NUMBER;
            var param2 = new Windows.RuntimeTest.Example.SampleTestObject2();
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            for(var i = 0; i < ITERATIONS; i++) {    
                msWriteProfilerMark("JS_UInt32InInterfaceIn_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++)
                    obj2.marshalUINT32InInterfaceIn(param,param2);
                msWriteProfilerMark("JS_UInt32InInterfaceIn_End");
            }
        }
    });

    // Collections Tests
    
    [10, 1000].forEach(function(size) {

        runner.addTest({
            id: counter++,
            desc: "[]: Double Vector size " + size + " RT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i/10);
                }
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsDoubleVector" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj1.marshalDoubleVectorInOut(param);
                    msWriteProfilerMark("JS_ArrayAsDoubleVector" + size + "_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Double Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i/10);
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsDoubleVector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalDoubleVectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsDoubleVector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Double Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i/10);
                }
                var val;

                obj2.marshalDoubleVectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsDoubleVector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalDoubleVectorOut();
                    msWriteProfilerMark("JS_ArrayAsDoubleVector" + size + "Out_End");
                }
            }
        });


        runner.addTest({
            id: counter++,
            desc: "[]: Int16 Vector size " + size + " RT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i);
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsInt16Vector" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj1.marshalINT16VectorInOut(param);
                    msWriteProfilerMark("JS_ArrayAsInt16Vector" + size + "_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Int16 Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i/10);
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsINT16Vector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalINT16VectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsINT16Vector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Int16 Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i/10);
                }
                var val;

                obj2.marshalINT16VectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsINT16Vector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalINT16VectorOut();
                    msWriteProfilerMark("JS_ArrayAsINT16Vector" + size + "Out_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: UINT32 Vector size " + size + " RT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i);
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsUInt32Vector" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj1.marshalUINT32VectorInOut(param);
                    msWriteProfilerMark("JS_ArrayAsUInt32Vector" + size + "_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: UINT32 Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i);
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsUINT32Vector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalUINT32VectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsUINT32Vector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: UINT32 Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i);
                }
                var val;

                obj2.marshalUINT32VectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsUINT32Vector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalUINT32VectorOut();
                    msWriteProfilerMark("JS_ArrayAsUINT32Vector" + size + "Out_End");
                }
            }
        });


        runner.addTest({
            id: counter++,
            desc: "[]: INT32 Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i);
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsINT32Vector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalINT32VectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsINT32Vector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: INT32 Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i);
                }
                var val;

                obj2.marshalINT32VectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsINT32Vector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalINT32VectorOut();
                    msWriteProfilerMark("JS_ArrayAsINT32Vector" + size + "Out_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: UINT64 Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i);
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsUINT64Vector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalUINT64VectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsUINT64Vector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: UINT64 Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(i);
                }
                var val;

                obj2.marshalUINT64VectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsUINT64Vector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalUINT64VectorOut();
                    msWriteProfilerMark("JS_ArrayAsUINT64Vector" + size + "Out_End");
                }
            }
        });


        runner.addTest({
            id: counter++,
            desc: "[]: Boolean Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(true);
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsBooleanVector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalBooleanVectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsBooleanVector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Boolean Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(true);
                }
                var val;

                obj2.marshalBooleanVectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsBooleanVector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalBooleanVectorOut();
                    msWriteProfilerMark("JS_ArrayAsBooleanVector" + size + "Out_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Interface Vector size " + size + " RT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(new Windows.RuntimeTest.Example.SampleTestObjectBothInProc());
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsInterfaceVector" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj1.marshalInterfaceVectorInOut(param);
                    msWriteProfilerMark("JS_ArrayAsInterfaceVector" + size + "_End");
                }
            }
        });
        
        runner.addTest({
            id: counter++,
            desc: "[]: Interface Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(new Windows.RuntimeTest.Example.SampleTestObject2());
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsInterfaceVector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalInterfaceVectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsInterfaceVector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Interface Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(new Windows.RuntimeTest.Example.SampleTestObject2());
                }
                var val;

                obj2.marshalInterfaceVectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsInterfaceVector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalInterfaceVectorOut();
                    msWriteProfilerMark("JS_ArrayAsInterfaceVector" + size + "Out_End");
                }
            }
        });



        runner.addTest({
            id: counter++,
            desc: "[]: Runtimeclass Vector size " + size + " RT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(new Windows.RuntimeTest.Example.SampleTestObjectBothInProc());
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsRTCVector" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj1.marshalRuntimeClassVectorInOutBothInProc(param);
                    msWriteProfilerMark("JS_ArrayAsRTCVector" + size + "_End");
                }
            }
        });
        

        runner.addTest({
            id: counter++,
            desc: "[]: RuntimeClass Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(new Windows.RuntimeTest.Example.SampleTestObject2());
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsRTCVector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalRuntimeClassVectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsRTCVector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: RuntimeClass Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(new Windows.RuntimeTest.Example.SampleTestObject2());
                }
                var val;

                obj2.marshalRuntimeClassVectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsRTCVector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalRuntimeClassVectorOut();
                    msWriteProfilerMark("JS_ArrayAsRTCVector" + size + "Out_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Strings(10) as String Vector size " + size + " RT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(SHORT_STRING);
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsStringVector" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj1.marshalStringVectorInOut(param);
                    msWriteProfilerMark("JS_ArrayAsStringVector" + size + "_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: String(10) Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
                for(var i = 0; i < size; i++) {
                    param.push(SHORT_STRING);
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsStringVector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalStringVectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsStringVector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: String(10) Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push(SHORT_STRING);
                }
                var val;

                obj2.marshalStringVectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsStringVector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalStringVectorOut();
                    msWriteProfilerMark("JS_ArrayAsStringVector" + size + "Out_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Strings(10K) as String Vector size " + size + " RT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                var str = "";
                for(var i = 0; i < 100; i++)
                    str += LONG_STRING

                for(var i = 0; i < size; i++) {
                    param.push(str);
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsString10KVector" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj1.marshalStringVectorInOut(param);
                    msWriteProfilerMark("JS_ArrayAsString10KVector" + size + "_End");
                }
            }
        });
        runner.addTest({
            id: counter++,
            desc: "[]: String(10k) Vector size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                var str = "";
                for(var i = 0; i < 100; i++)
                    str += LONG_STRING

                for(var i = 0; i < size; i++) {
                    param.push(str);
                }

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsString10KVector" + size + "In_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj2.marshalStringVectorIn(param);
                    msWriteProfilerMark("JS_ArrayAsString10KVector" + size + "In_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: String(10k) Vector size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                var str = "";
                for(var i = 0; i < 100; i++)
                    str += LONG_STRING

                for(var i = 0; i < size; i++) {
                    param.push(str);
                }
                var val;

                obj2.marshalStringVectorIn(param);

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsString10KVector" + size + "Out_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj2.marshalStringVectorOut();
                    msWriteProfilerMark("JS_ArrayAsString10KVector" + size + "Out_End");
                }
            }
        });
        
        
        runner.addTest({
            id: counter++,
            desc: "[]: Struct Vect w/o string size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push({
                        doubleValue: NUMBER,
                        integerValue: NUMBER,
                        guidValue: GUID_STRING,
                    });
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsStructNSVectorIn" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj1.vectorOfStructWithoutString = param;
                    msWriteProfilerMark("JS_ArrayAsStructNSVectorIn" + size + "_End");

                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Struct Vect w/o string size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push({
                        doubleValue: NUMBER,
                        integerValue: NUMBER,
                        guidValue: GUID_STRING,
                    });
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsStructNSVectorOut" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj1.vectorOfStructWithoutString;
                    msWriteProfilerMark("JS_ArrayAsStructNSVectorOut" + size + "_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Struct Vector w/ string size " + size + " IN",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push({
                        doubleValue: NUMBER,
                        integerValue: NUMBER,
                        guidValue: GUID_STRING,
                        stringValue: SHORT_STRING
                    });
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsStructVectorIn" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        obj1.vectorOfStructWithString = param;
                    msWriteProfilerMark("JS_ArrayAsStructVectorIn" + size + "_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: "[]: Struct Vector w/ string size " + size + " OUT",
            tags: ['innerIterations 5000'],
            test: function() {
                var param = [];
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                for(var i = 0; i < size; i++) {
                    param.push({
                        doubleValue: NUMBER,
                        integerValue: NUMBER,
                        guidValue: GUID_STRING,
                        stringValue: SHORT_STRING
                    });
                }
            
                var val;

                for(var i = 0; i < ITERATIONS; i++) {    
                    msWriteProfilerMark("JS_ArrayAsStructVectorOut" + size + "_Start");
                    for(var j = 0; j < INNER_ITERATIONS; j++)
                        val = obj1.vectorOfStructWithString;
                    msWriteProfilerMark("JS_ArrayAsStructVectorOut" + size + "_End");
                }
            }
        });
    });

    var Datatypes = {
      'INT32': 1,
      'String': SHORT_STRING,
      'RuntimeClass': new Windows.RuntimeTest.Example.SampleTestObject2(),
      'Point': {x: 1, y: 2}
    };

    [10, 1000].forEach(function(size) {
        ['INT32', 'String', 'RuntimeClass', 'Point'].forEach(function(type) {
            runner.addTest({
                id: counter++,
                desc: type + " Array In - Size " + size,
                tags: ['innerIterations ' + (100000/size)],
                test: function() {
                    var func = obj2["marshal" + type + "ArrayIn"].bind(obj2);
                    var startEvent = "JS_" + type + "Array" + size + "In_Start";
                    var endEvent = "JS_" + type + "Array" + size + "In_End";
                    var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                    var param = [];
                    for(var i = 0; i < size; i++) {
                        param.push(Datatypes[type]);
                    }
                    
                    for(var i = 0; i < ITERATIONS; i++) {
                        msWriteProfilerMark(startEvent);
                        for(var j = 0; j < INNER_ITERATIONS; j++)
                            func(param);
                            
                        msWriteProfilerMark(endEvent);
                    }
                }
            });
            
            runner.addTest({
                id: counter++,
                desc: type + " Array Out - Size " + size,
                tags: ['innerIterations ' + (100000/size)],
                test: function() {
                    var func = obj2["marshal" + type + "ArrayOut"].bind(obj2);
                    var startEvent = "JS_" + type + "Array" + size + "Out_Start";
                    var endEvent = "JS_" + type + "Array" + size + "Out_End";
                    var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                    var param = [];
                    for(var i = 0; i < size; i++) {
                        param.push(Datatypes[type]);
                    }
                    
                    obj2["marshal" + type + "ArrayIn"](param);
                   
                    var val;
                    
                    for(var i = 0; i < ITERATIONS; i++) {
                        msWriteProfilerMark(startEvent);
                        for(var j = 0; j < INNER_ITERATIONS; j++)
                            val = func();
                        msWriteProfilerMark(endEvent);
                    }
                }
            });
            
            runner.addTest({
                id: counter++,
                desc: type + " Array Fill - Size " + size,
                tags: ['innerIterations ' + (100000/size)],
                test: function() {
                    var func = obj2["marshal" + type + "ArrayFill"].bind(obj2);
                    var startEvent = "JS_" + type + "Array" + size + "Fill_Start";
                    var endEvent = "JS_" + type + "Array" + size + "Fill_End";
                    var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                    var param = [];
                    for(var i = 0; i < size; i++) {
                        param.push(Datatypes[type]);
                    }
                    
                    obj2["marshal" + type + "ArrayIn"](param);
                   
                    var val = new Array(size);
                    
                    for(var i = 0; i < ITERATIONS; i++) {
                        msWriteProfilerMark(startEvent);
                        for(var j = 0; j < INNER_ITERATIONS; j++)
                            func(val);
                            
                        msWriteProfilerMark(endEvent);
                    }
                }
            });
         });
    });
    
    [10, 1000].forEach(function(size) {
        ['INT32', 'String', 'RuntimeClass', 'Point'].forEach(function(type) {
            runner.addTest({
                id: counter++,
                desc: type + " IVector In - Size " + size,
                tags: ['innerIterations 5000'],
                test: function() {
                    var inFunction = obj2["marshal" + type + "RealIVectorIn"].bind(obj2);
                    var startEvent = "JS_" + type + "IVector" + size + "In_Start";
                    var endEvent = "JS_" + type + "IVector" + size + "In_End";
                    var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                    var param = [];
                    
                    for(var i = 0; i < size; i++) {
                        param.push(Datatypes[type]);
                    }
                    
                    
                    for(var i = 0; i < ITERATIONS; i++) {
                        msWriteProfilerMark(startEvent);
                        for(var j = 0; j < INNER_ITERATIONS; j++)
                            inFunction(param);
                            
                        msWriteProfilerMark(endEvent);
                    }
                }
            });
            
            runner.addTest({
                id: counter++,
                desc: type + " IVector Out - Size " + size,
                tags: ['innerIterations 5000'],
                test: function() {
                    var outFunction = obj2["marshal" + type + "RealIVectorOut"].bind(obj2);
                    var startEvent = "JS_" + type + "IVector" + size + "Out_Start";
                    var endEvent = "JS_" + type + "IVector" + size + "Out_End";
                    var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                    var param = [];
                    for(var i = 0; i < size; i++) {
                        param.push(Datatypes[type]);
                    }
                   
                    obj2["marshal" + type + "RealIVectorIn"](param);
                    var val;
                    
                    for(var i = 0; i < ITERATIONS; i++) {
                        msWriteProfilerMark(startEvent);

                        for(var j = 0; j < INNER_ITERATIONS; j++)
                            val = outFunction();
                            
                        msWriteProfilerMark(endEvent);
                    }
                }
            });
        });
    });

    /*
    // This is a benchmark for normal JS array iteration (eg. not projected anything) useful for
    // comparing to the other iteration tests. Commented out because we don't need this comparison
    // for every run but it's nice to have.
    runner.addTest({
        id: counter++,
        desc: "JS Array Iteration - Size 1000",
        tags: ['innerIterations 10'],
        test: function() {
            var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;
            for(var i = 0; i < ITERATIONS; i++) {
                var val = new Array(1000);
                
                msWriteProfilerMark("JS_JSArraySize1000Iter_Start");
                for(var j = 0; j < INNER_ITERATIONS; j++) {
                  for(var k = 0; k < val.length; k++) {
                      res = val[k];
                  }
                }
                    
                msWriteProfilerMark("JS_JSArraySize1000Iter_End");
            }
        }
    });
    */

    ['INT32', 'String', 'RuntimeClass', 'Point'].forEach(function(type) {
        runner.addTest({
            id: counter++,
            desc: type + " IVector Iteration - Size 1000",
            tags: ['innerIterations 10'],
            test: function() {
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                var param = [];
                for(var i = 0; i < 1000; i++) {
                    param.push(Datatypes[type]);
                }
               
                obj2["marshal" + type + "RealIVectorIn"](param);

                var res;
                
                for(var i = 0; i < ITERATIONS; i++) {
                    var val = obj2["marshal" + type + "RealIVectorOut"]();
                    
                    msWriteProfilerMark("JS_" + type + "IVectorIterSize1000_Start");

                    for(var j = 0; j < INNER_ITERATIONS; j++) {
                      for(var k = 0; k < val.length; k++) {
                          res = val[k];
                      }
                    }
                        
                    msWriteProfilerMark("JS_" + type + "IVectorIterSize1000_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: type + " IVector Iteration For Each - Size 1000",
            tags: ['innerIterations 10'],
            test: function() {
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                var param = [];
                for(var i = 0; i < 1000; i++) {
                    param.push(Datatypes[type]);
                }
               
                obj2["marshal" + type + "RealIVectorIn"](param);

                var res;
                
                for(var i = 0; i < ITERATIONS; i++) {
                    var val = obj2["marshal" + type + "RealIVectorOut"]();
                    
                    msWriteProfilerMark("JS_" + type + "IVectorIterFESize1000_Start");

                    for(var j = 0; j < INNER_ITERATIONS; j++) {
                      val.forEach(function(i) {
                          res = i;
                      });
                    }
                        
                    msWriteProfilerMark("JS_" + type + "IVectorIterFESize1000_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: type + " IVector SetAt - Size 1000",
            tags: ['innerIterations 10'],
            test: function() {
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                var val = Datatypes[type];

                var param = [];
                for(var i = 0; i < 1000; i++) {
                    param.push(val);
                }
               
                obj2["marshal" + type + "RealIVectorIn"](param);

                for(var i = 0; i < ITERATIONS; i++) {
                    var res = obj2["marshal" + type + "RealIVectorOut"]();
                    
                    msWriteProfilerMark("JS_" + type + "IVectorSetAtSize1000_Start");

                    for(var j = 0; j < INNER_ITERATIONS; j++) {
                      for(var k = 0; k < res.length; k++) {
                        res[k] = val;
                      }
                    }
                        
                    msWriteProfilerMark("JS_" + type + "IVectorSetAtSize1000_End");
                }
            }
        });

        runner.addTest({
            id: counter++,
            desc: type + " IVector Append - Size 1000",
            tags: ['innerIterations 10'],
            test: function() {
                var INNER_ITERATIONS = this.tags[0].split(" ")[1] >> 0;

                var val = Datatypes[type];

                var param = [];

                obj2["marshal" + type + "RealIVectorIn"](param);

                for(var i = 0; i < ITERATIONS; i++) {
                    var res = obj2["marshal" + type + "RealIVectorOut"]();
                    
                    msWriteProfilerMark("JS_" + type + "IVectorAppendSize1000_Start");

                    for(var j = 0; j < INNER_ITERATIONS; j++) {
                        for(var i = 0; i < 1000; i++) {
                            res.append(val);
                        }
                    }
                        
                    msWriteProfilerMark("JS_" + type + "IVectorAppendSize1000_End");
                }
            }
        });
    });
}());
