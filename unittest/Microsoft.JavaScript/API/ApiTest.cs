//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

namespace JsrtUnitTests
{
    using System;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.JavaScript;

    [TestClass]
    public class ApiTest
    {
        [AssemblyInitialize]
// We need to give te.exe a hint of what kind of surrogate process we need to run this
// this test, since the assembly is arch-neutral.
#if amd64
        [TestProperty("Architecture", "x64")]
#elif x86
        [TestProperty("Architecture", "x86")]
#endif
        public static void RunModuleSetup()
        {
        }

        JavaScriptRuntime runtime;
        JavaScriptContext context;

        [TestInitialize]
        public void TestMethodSetup()
        {
            runtime = JavaScriptRuntime.New();
            JavaScriptContext newContext = runtime.NewContext();
            newContext.AddRef();
            context = newContext;
        }

        [TestCleanup]
        public void TestCleanup()
        {
            if (context.IsValid)
            {
                context.Release();
                context = new JavaScriptContext();
            }

            if (runtime.IsValid)
            {
                runtime.Dispose();
                runtime = new JavaScriptRuntime();
            }
        }

        [TestMethod]
        public void ObjectsAndPropertiesTest()
        {
            using (new JavaScriptContext.Scope(context))
            {
                // Create a Chakra object, set some properties
                JavaScriptValue obj = JavaScriptValue.NewObject();

                obj.SetProperty(JavaScriptPropertyId.New("stringProperty"), JavaScriptValue.New("stringValue"), true);
                obj.SetProperty(JavaScriptPropertyId.New("doubleProperty"), JavaScriptValue.New(3.141592), true);
                obj.SetProperty(JavaScriptPropertyId.New("boolProperty"), JavaScriptValue.New(true), true);

                JavaScriptValue objProperty = JavaScriptValue.NewObject();
                obj.SetProperty(JavaScriptPropertyId.New("objectProperty"), objProperty, true);

                // Get some properties
                Assert.IsTrue(obj.GetProperty(JavaScriptPropertyId.New("stringProperty")).ToString() == "stringValue");
                Assert.IsTrue(obj.GetProperty(JavaScriptPropertyId.New("doubleProperty")).ToDouble() == 3.141592);
                Assert.IsTrue(obj.GetProperty(JavaScriptPropertyId.New("boolProperty")).ToBoolean() == true);
                Assert.IsTrue(obj.GetProperty(JavaScriptPropertyId.New("objectProperty")).StrictEquals(objProperty));

                // Set the object on global and run script to validate properties look correct from
                // script.
                JavaScriptValue.GlobalObject.SetProperty(JavaScriptPropertyId.New("foo"), obj, true);
                JavaScriptValue.GlobalObject.SetProperty(JavaScriptPropertyId.New("bar"), objProperty, true);

                string script = "(function() { " +
                                    "return foo.stringProperty == \"stringValue\" && " +
                                        "foo.doubleProperty == 3.141592 && " +
                                        "foo.boolProperty == true && " +
                                        "foo.objectProperty === bar; })";

                JavaScriptValue f = JavaScriptContext.RunScript(script);
                JavaScriptValue result = f.CallFunction(new JavaScriptValue[] { JavaScriptValue.Undefined });
                Assert.IsTrue(result.StrictEquals(JavaScriptValue.True));

                // Create an array with 10 slots
                JavaScriptValue arr = JavaScriptValue.NewArray(10);

                // Get length
                Assert.IsTrue(arr.GetProperty(JavaScriptPropertyId.New("length")).ToDouble() == 10);

                // Populate array and verify values
                for (int i = 0; i < 9; i++)
                {
                    arr.SetIndexedProperty(JavaScriptValue.New(i), JavaScriptValue.New(i.ToString()));
                }

                arr.SetIndexedProperty(JavaScriptValue.New(9), obj);

                for (int i = 0; i < 9; i++)
                {
                    JavaScriptValue item = arr.GetIndexedProperty(JavaScriptValue.New(i));
                    Assert.IsTrue(JavaScriptValue.Equals(item, JavaScriptValue.New(i.ToString())));
                }

                Assert.IsTrue(arr.GetIndexedProperty(JavaScriptValue.New(9)).StrictEquals(obj));

                // Remove an item
                arr.DeleteIndexedProperty(JavaScriptValue.New(5));
                Assert.IsTrue(arr.GetIndexedProperty(JavaScriptValue.New(5)).ValueType == JavaScriptValueType.Undefined);
            }
        }

        [TestMethod]
        public void HostCallbackTest()
        {
            using (new JavaScriptContext.Scope(context))
            {
                WeakReference callbackRef = SetupHostCallback();

                GC.Collect();
                Assert.IsTrue(!callbackRef.IsAlive);
                JavaScriptValue r = JavaScriptContext.RunScript("(function(arg1) { return callback(arg1); })");
                r = r.CallFunction(new JavaScriptValue[] { JavaScriptValue.Undefined, JavaScriptValue.New("hello") });
                Assert.IsTrue(r.ToString() == "world");
            }
        }

        WeakReference SetupHostCallback()
        {
            JavaScriptValue callback = JavaScriptValue.NewFunction(delegate(JavaScriptValue callee, bool isConstructCall, JavaScriptValue[] arguments, ushort argumentCount, IntPtr callbackState)
            {
                Assert.IsTrue(argumentCount == 2);
                Assert.IsTrue(arguments[1].ToString() == "hello");
                return JavaScriptValue.New("world");
            }, IntPtr.Zero);

            JavaScriptValue.GlobalObject.SetProperty(JavaScriptPropertyId.New("callback"), callback, true);

            return new WeakReference(callback);
        }

        [TestMethod]
        public void NestedContextsTest()
        {
            int nestedCalls = 0;

            using (new JavaScriptContext.Scope(context))
            {
                JavaScriptValue callback = JavaScriptValue.NewFunction(delegate(JavaScriptValue callee, bool isConstructCall, JavaScriptValue[] arguments, ushort argumentCount, IntPtr callbackState)
                {
                    nestedCalls++;
                    JavaScriptValue name = JavaScriptValue.GlobalObject.GetProperty(JavaScriptPropertyId.New("contextName"));
                    Assert.IsTrue(name.ToString() == "context1");

                    using (new JavaScriptContext.Scope(runtime.NewContext()))
                    {
                        JavaScriptValue callback2 = JavaScriptValue.NewFunction(delegate(JavaScriptValue callee2, bool isConstructCall2, JavaScriptValue[] arguments2, ushort argumentCount2, IntPtr callbackState2)
                        {
                            nestedCalls++;
                            JavaScriptValue name2 = JavaScriptValue.GlobalObject.GetProperty(JavaScriptPropertyId.New("contextName"));
                            Assert.IsTrue(name2.ToString() == "context2");

                            return JavaScriptValue.Undefined;
                        }, IntPtr.Zero);

                        JavaScriptValue.GlobalObject.SetProperty(JavaScriptPropertyId.New("callback"), callback2, true);
                        JavaScriptValue.GlobalObject.SetProperty(JavaScriptPropertyId.New("contextName"), JavaScriptValue.New("context2"), true);
                        JavaScriptContext.RunScript("callback()");
                    }

                    return JavaScriptValue.Undefined;
                }, IntPtr.Zero);

                JavaScriptValue.GlobalObject.SetProperty(JavaScriptPropertyId.New("callback"), callback, true);
                JavaScriptValue.GlobalObject.SetProperty(JavaScriptPropertyId.New("contextName"), JavaScriptValue.New("context1"), true);
                JavaScriptContext.RunScript("callback()");
            }

            Assert.IsTrue(nestedCalls == 2);
        }

        [TestMethod]
        public void ErrorHandling()
        {
            bool gotException = false;
            JavaScriptValue r;

            // No Current Context
            try
            {
                JavaScriptContext.RunScript("");
            }
            catch (JavaScriptUsageException)
            {
                gotException = true;
            }

            Assert.IsTrue(gotException);

            // Script throws JavaScriptException with Error object
            using (new JavaScriptContext.Scope(context))
            {
                try
                {
                    JavaScriptContext.RunScript("(function() { throw new Error(\"ok\"); })()");
                }
                catch (JavaScriptScriptException e)
                {
                    JavaScriptValue value = e.Error.GetProperty(JavaScriptPropertyId.New("message"));
                    Assert.IsTrue(value.ToString() == "ok");
                }

                r = JavaScriptContext.RunScript("(function() { return \"context still good\"; })()");
                Assert.IsTrue(r.ToString() == "context still good");
            }

            // Host throws JavaScriptException with Error object
            using (new JavaScriptContext.Scope(context))
            {
                JavaScriptValue callback = JavaScriptValue.NewFunction(delegate(JavaScriptValue callee, bool isConstructCall, JavaScriptValue[] arguments, ushort argumentCount, IntPtr callbackState)
                {
                    JavaScriptContext.SetException(JavaScriptValue.NewError(JavaScriptValue.New("ok")));
                    return new JavaScriptValue();
                }, IntPtr.Zero);

                JavaScriptValue f = JavaScriptContext.RunScript(
                    "(function(callback) {" +
                    "   try {" +
                    "       callback(); }" +
                    "   catch(e) {" +
                    "       return e.message;" +
                    "   }" +
                    "   return \"wrong\";" +
                    "})");

                r = f.CallFunction(new JavaScriptValue[] { JavaScriptValue.Undefined, callback });
                Assert.IsTrue(r.ToString() == "ok");

                r = JavaScriptContext.RunScript("(function() { return \"context still good\"; })()");
                Assert.IsTrue(r.ToString() == "context still good");
            }
        }

        [TestMethod]
        public void CompileErrorTest()
        {
            using (new JavaScriptContext.Scope(context))
            {
                bool gotException = false;

                try
                {
                    JavaScriptContext.RunScript(
                        "if (a > 1) {\n" +
                        "  a = 1;\n" +
                        "}}");
                }
                catch (JavaScriptScriptException e)
                {
                    gotException = true;
                    JavaScriptValue stringify = JavaScriptContext.RunScript("JSON.stringify");
                    JavaScriptValue str = stringify.CallFunction(new JavaScriptValue[] { JavaScriptValue.Undefined, e.Error });
                    Assert.AreEqual("{\"message\":\"Syntax error\",\"line\":2,\"column\":1,\"length\":1,\"source\":\"}}\"}", str.ToString());
                }

                Assert.IsTrue(gotException);
            }
        }

        [TestMethod]
        public void ByteCodeTest()
        {
            const string script = "function test() { return true; }; test();";
            byte[] byteCodes = new byte[1024];

            using (new JavaScriptContext.Scope(context))
            {
                Assert.IsTrue(JavaScriptContext.SerializeScript(script, byteCodes) <= 1024);
                JavaScriptValue v = JavaScriptContext.RunScript(script, byteCodes);
                Assert.IsTrue(v.ToBoolean());
            }

            JavaScriptRuntime newRuntime = JavaScriptRuntime.New();
            JavaScriptContext newContext = newRuntime.NewContext();

            using (new JavaScriptContext.Scope(newContext))
            {
                JavaScriptValue v = JavaScriptContext.RunScript(script, byteCodes);
                Assert.IsTrue(v.ToBoolean());
            }

            newContext = new JavaScriptContext();
            newRuntime.Dispose();
        }

        [System.Runtime.InteropServices.ComVisible(true)]
        public class Host
        {
            public int foo { get { return 123; } }
            public string bar = "hello";
            public string getMessage() { return "world"; }
            public void verify(bool condition) { Assert.IsTrue(condition); }
        }

        [TestMethod]
        public void ComProjectionTest()
        {
            using (new JavaScriptContext.Scope(context))
            {
                JavaScriptContext.RunScript(@"
                    function thing() {
                        this.getKind = function() {
                            return this.kind;
                        }
                        this.isA = function(a) {
                            return (a == this.getKind());
                        }
                    }

                    function vehicle() {
                        this.kind = 'vehicle',
                        this.getYearMade = function() {
                            return this.yearMade;
                        }
                    }

                    vehicle.prototype = new thing();

                    function newCar(make) {
                        this.yearMade = 2012;
                        this.make = make;
                    }

                    newCar.prototype = new vehicle();
                    function makeNewCar(make) {
                        return new newCar(make);
                    }
                ");

                dynamic global = JavaScriptValue.GlobalObject.ToObject();
                dynamic vehicle = global.makeNewCar("vw");

                Assert.IsTrue(vehicle.getYearMade(null) == 2012);
                Assert.IsTrue(vehicle.isA("vehicle"));
                Assert.IsTrue(vehicle.make == "vw");

                JavaScriptContext.RunScript(@"
                    function testInjection(host) {
                        host.verify(host.foo == 123);
                        host.verify(host.bar == 'hello');
                        host.verify(host.getMessage() == 'world');
                    }
                ");

                JavaScriptValue testInjection = JavaScriptContext.RunScript("testInjection");
                testInjection.CallFunction(new JavaScriptValue[] { JavaScriptValue.Undefined, JavaScriptValue.New(new Host()) });
            }
        }
    }
}
