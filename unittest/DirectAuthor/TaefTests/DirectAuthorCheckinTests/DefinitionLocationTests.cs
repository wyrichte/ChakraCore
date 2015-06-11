using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class DefinitionLocationTests : DirectAuthorTest
    {
        [TestInitialize]
        new public void Initialize()
        {
            base.Initialize();
        }

        [TestCleanup]
        new public void Cleanup()
        {
            base.Cleanup();
        }

        [TestMethod]
        [WorkItem(id: 748444)]
        public void When_a_var_decl_is_redefined_gotodef_should_goto_the_appropriate_var_definition()
        {
            TestDefinition("var |define:a1|a = function () { }; |reference:a1|a(); var |define:a2|a = function() { }; |reference:a2|a();");
        }

        [TestMethod]
        public void GlobalWithThisAssignment()
        {
            TestDefinition("var |define:a|a = 0; function b(a) {this.|reference:a|a = a; } b(a);");
        } 

        [TestMethod]
        public void GlobalInPrimary()
        {
            TestDefinition("var |define:a|a; |reference:a|a");
        }

        [TestMethod]
        public void GlobalInContext()
        {
            TestDefinition("|reference:a|a", "var |define:a|a;");
        }

        [TestMethod]
        [TestCategory("Let")]
        public void GlobalLetReferencedInFunction()
        {
            TestDefinition("let |define:a|a = 0; function b() { let c = |reference:a|a; } b();");
        }

        [TestMethod]
        [TestCategory("Let")]
        public void GlobalLetInPrimary()
        {
            TestDefinition("let |define:a|a; |reference:a|a");
        }

        [TestMethod]
        [TestCategory("Let")]
        public void GlobalLetInContext()
        {
            TestDefinition("|reference:a|a", "let |define:a|a;");
        }

        [TestMethod]
        [TestCategory("Const")]
        public void GlobalConstReferencedInFunction()
        {
            TestDefinition("const |define:a|a = 0; function b() { let c = |reference:a|a; } b();");
        }

        [TestMethod]
        [TestCategory("Const")]
        public void GlobalConstInPrimary()
        {
            TestDefinition("const |define:a|a = 0; |reference:a|a");
        }

        [TestMethod]
        [TestCategory("Const")]
        public void GlobalConstInContext()
        {
            TestDefinition("|reference:a|a", "const |define:a|a = 0;");
        }

        [TestMethod]
        public void ObjectLiteralInPrimary()
        {
            TestDefinition("var a = {|define:b|b: 1}; a.|reference:b|b");
        }

        [TestMethod]
        public void ObjectLiteralInContext()
        {
            TestDefinition("a.|reference:b|b", "var a = {|define:b|b: 1}; ");
        }

        [TestMethod]
        public void Parameter()
        {
            TestDefinition("function foo(|define:a|a, |define:b|b) { var c = |reference:a|a; c = |reference:b|b; }");
        }

        [TestMethod]
        public void ParentParameter()
        {
            TestDefinition("function foo(|define:a|a, |define:b|b) { function bar() { var c = |reference:a|a; c = |reference:b|b; } }");
        }

        [TestMethod]
        public void LocalVariable()
        {
            TestDefinition("function foo() { var |define:a|a = 1; var |define:b|b = 2; var c = |reference:a|a; var d = |reference:b|b }");
        }

        [TestMethod]
        public void ParentLocalVariable()
        {
            TestDefinition("function foo() { var |define:a|a = 1; var |define:b|b = 2; function bar() { var c = |reference:a|a; var d = |reference:b|b } }");
        }

        [TestMethod]
        [WorkItem(203471)]
        public void ConstructorField()
        {
            TestDefinition("function Foo() { this.|define:a|a = 10; } var b = new Foo(); b.|reference:a|a;");
        }

        [TestMethod]
        public void GlobalFunction()
        {
            TestDefinition("function |define:foo|foo() { } |reference:foo|foo()");
        }

        [TestMethod]
        public void FunctionInLiteral()
        {
            TestDefinition("var a = { |define:foo|foo: function() { } }; a.|reference:foo|foo();");
        }

        [TestMethod]
        public void FunctionInPrototype()
        {
            TestDefinition("function Foo() { } Foo.prototype.foo = |define:foo|function() { }; var a = new Foo(); a.|reference:foo|foo()");
        }

        [TestMethod]
        public void FileLocationOfBuiltIn()
        {
            TestDefinition("new |reference:none|Object()");
        }

        [TestMethod]
        public void CatchVariable()
        {
            TestDefinition("try { } catch(|define:e||reference:e|e) { }");
        }

        [TestMethod]
        public void VariableList()
        {
            TestDefinition("var |define:a|a, b; |reference:a|a");
        }

        [TestMethod]
        public void ForInVariable()
        {
            TestDefinition("var a = [2]; for (var |define:b|b in a) { |reference:b|b = 0; }");
        }

        [TestMethod]
        public void Labels()
        {
            TestDefinition("b: |define:b|for (var i = 0; i < 1; ++i) if (i == 0) break |reference:b|b;");
        }

        [TestMethod]
        public void FunctionInvocation()
        {
            TestDefinition("var |define:a|a = function () { }; |reference:a|a();");
        }


        [TestMethod]
        public void FileId()
        {
            var file1 = _session.FileFromText("function foo() { } foo|()");
            var file2 = _session.FileFromText("function bar() { } bar|()");

            var handle1 = file1.GetHandle();
            var handle2 = file2.GetHandle();

            Assert.AreEqual(handle1.FileId, handle1.FileId);
            Assert.AreEqual(handle2.FileId, handle2.FileId);
            Assert.AreNotEqual(handle1.FileId, handle2.FileId);
        }

        [TestMethod]
        [WorkItem(195769)]
        public void SelfReferentFunctionExpression()
        {
            TestDefinition("var f = function |define:fact|fact(x) { if (x <= 1) return 1; else return x * |reference:fact|fact(x - 1); };");
        }

        [TestMethod]
        [WorkItem(195772)]
        public void CatchVariableInThrow()
        {
            TestDefinition("try { } catch (|define:e|e) { throw |reference:e|e; }");
        }

        [TestMethod]
        public void AnnotatedFunction()
        {
            // Ensure annotations do not affect the definition location of a function
            TestDefinition("function |define:foo|foo() { } intellisense.annotate(foo, function foo() { }); foo|reference:foo|();");
        }

        [TestMethod]
        public void AnnotatedFunctions()
        {
            // Ensure annotations do not affect the definition location of a function
            TestDefinition("var a = { |define:foo|foo: function foo() { }, |define:bar|bar: function () { } } intellisense.annotate(a, { foo: function fooAnn() { }, bar: function barAnn() { } }); a.foo|reference:foo|(); a.bar|reference:bar|()");
        }

        [TestMethod]
        public void AnnotatedFields()
        {
            // Ensure annotations do not affect the definition location of a field
            TestDefinition(AnnotatedFieldsText);
        }
        #region Test data
        const string AnnotatedFieldsText = @"
                    var someObject = {
                       |define:foo|foo: 1,
                       |define:bar|bar: 2
                    };

                    intellisense.annotate(someObject, {
                       /// <field name='foo' type='number'>Some field</field>
                       foo: 1,

                       /// <field name='bar' type='number'>Some field</field>
                       bar: 2
                    });


someObject.|reference:foo|foo;
someObject.|reference:bar|bar;
";
        #endregion

        [TestMethod]
        public void FileInMultipleContexts()
        {
            var commonText = "var a = 1; a;";
            var commonIndex = commonText.IndexOf("a;");
            var commonFile = _session.FileFromText("var a = 1; a;");
            var primaryFile = _session.FileFromText("a");
            var dummyFile = _session.FileFromText("");

            var contextOne = _session.OpenContext(primaryFile, dummyFile, commonFile);
            var contextTwo = _session.OpenContext(commonFile);

            // Force a complete parse
            contextOne.GetCompletionsAt(0);
            contextTwo.GetCompletionsAt(0);

            // Try to find a
            var location1 = contextOne.GetDefinitionLocation(0);
            var location2 = contextTwo.GetDefinitionLocation(commonIndex);
            Assert.AreEqual(commonFile, location1.File);
            Assert.AreEqual(commonFile, location2.File);
        }

        [TestMethod]
        public void ArgumentsArray()
        {
            TestDefinition("function |define:arguments|fn(a, b) { alert(|reference:arguments|arguments[1]); } function x() { fn(1, 2); }");
        }

        [TestMethod]
        public void ThisDotGlobalReference()
        {
            // this.a refers to the global a
            TestDefinition("var |define:a|a = 0; function b() { var b = this.|reference:a|a;}");
            // this.a should not reference global a because B looks like a constructor
            TestDefinition("var |define:a|a = 0; function B() { var b = this.|reference:none|a; }");
            // same here.
            TestDefinition("var |define:a|a = 0; function b() { var b = this.|reference:none|a; } b.prototype.e = 1;");
            // this.a should be treated as the definition of a.
            TestDefinition("function F() { this.|define:a|a = 1 }; var f = new F(); f.|reference:a|a");
        }

        [TestMethod]
        [WorkItem(408607)]
        public void MultipleAssignments()
        {
            TestDefinition(@"
                function MyObject() {
                    /// <field name='myArray' type='Array' elementType='Number' >This is my array</field>
                    /// <field name='myNumber' type='Number' />
                    this.|define:myArray|myArray = null;
                    this.|define:myNumber|myNumber = null;
                }
                MyObject.prototype.AnotherFunction = function (width) {
                    this.myNumber = width;
                    this.myArray = new Array(10);
                }
　
                var m = new MyObject();
                m.|reference:myNumber|myNumber;
                m.|reference:myArray|myArray;
                m.AnotherFunction(10);
                m.|reference:myNumber|myNumber;
                m.|reference:myArray|myArray;
            ");
        }

        //[TestMethod]
        //public void LongForInNestedLoop()
        //{
        //    HurriedTestDefinition(LongForInNestedLoopText);
        //}
        #region Test data
        const string LongForInNestedLoopText = @"function f(|define:a|a, |define:b|b) {
|reference:a|a;
|reference:b|b;
}

for (var i = 0; i < 10; i++) {
for (var j = 0; j < i; j++) {
for (var k = 0; k < j; k++) { 
for (var l = 0; l < i; l++) { 
for (c in d) {
for (e in g) {
z[i][j][k][l]++;
}
}
}
}
}
}
";
        #endregion


        [TestMethod]
        public void RedirectFunctionDefinition()
        {
            TestDefinition("function foo() { } function |define:foo|bar() { } intellisense.redirectDefinition(foo, bar); |reference:foo|foo()");
        }

        [TestMethod]
        [WorkItem(403946)]
        public void WinJSPropertyDefintion()
        {
            TestDefinition(WinJSPropertyDefintionText,
                      "@@" + WinJSTestFiles.latest_base,
                      "!!" + Paths.WinJs.BaseIntellisensePath);
        }
        #region Test data
        const string WinJSPropertyDefintionText =
@"var c = WinJS.Class.define(function () { }, {
    /// <field>This is some text</field>
    |define:bar|bar: {
        get: function () {
            return 1;
        }
    }
});

var t = new c();

t.|reference:bar|bar;";
        #endregion

        [TestMethod]
        [WorkItem(403946)]
        public void WinJSPropertyDefintionInBaseJs()
        {
            TestDefinition(WinJSPropertyDefinitionInBaseJsText,
                      "!!" + Paths.WinJs.BaseIntellisenseSetupPath,
                      "@@" + WinJSTestFiles.latest_base,
                      BaseExtensionText,
                      "!!" + Paths.WinJs.BaseIntellisensePath);
        }
        #region Test data
        const string BaseExtensionText =
@"var c = WinJS.Class.define(function () { }, {
    /// <field>This is some text</field>
    |define:bar|bar: {
        get: function () {
            return 1;
        }
    }
});";
        const string WinJSPropertyDefinitionInBaseJsText =
@"var t = new c();

t.|reference:bar|bar;";
        #endregion
        [TestMethod]
        public void SimpleTryCatchVariable()
        {
            var text = "try {} catch (e) {}";
            var file = _session.FileFromText(text);
            var context = _session.OpenContext(file);
            DefintionResult result = new DefintionResult();
            var offset = text.IndexOf("(e)") + 1;
            context.GetDefinitionLocation(offset, out result.File, out result.Location);
            Assert.IsNotNull(result.File);
        }

        [TestMethod]
        public void CallerDefines()
        {
            TestDefinition(FooNamespaceDefinition + FooNamespaceReference);
        }

        [TestMethod]
        public void CallerDefinesDefinitionAndReferenceInSeparateFiles()
        {
            TestDefinition(FooNamespaceReference, FooNamespaceDefinition);
        }
        #region Test data
        const string FooNamespaceDefinition =
@" function createNamespace(namespaceName) {
                    var obj = {};
                    this[namespaceName] = obj;
                    intellisense.callerDefines(obj);
                }

                |define:Foo|createNamespace('Foo');";

        const string FooNamespaceReference =
@"|reference:Foo|Foo";
        #endregion

        [TestMethod]
        [WorkItem(730141)]
        public void When_getting_the_definition_of_a_field_it_should_navigation_to_the_field_definition_location_and_not_the_values_location()
        {
            TestDefinition(@"
                var |define:continent|continent = { location: 'north', |define:name|name: 'myCont' }; 
                var country = { area: 73, |define:cont|cont: |reference:continent|continent }; 
                var |define:city|city = { population: 1052, |define:coun|coun: country }; 
                var name; 
                name = |reference:city|city.|reference:coun|coun.|reference:cont|cont.|reference:name|name;
            ");
        }

        [TestMethod]
        [WorkItem(729464)]
        [WorkItem(729471)]
        public void When_getting_the_definition_of_a_class_it_should_go_to_the_variable_defining_it()
        {
            TestDefinition(@"
    var |define:cls|cls = WinJS.Class.define(function (x, y) { }, 
                {
                    a: 1
                },
                {
                    b: 2
                });

    cls.b--;
    var |define:c|c = new |reference:cls|cls(1, 2);
    |reference:c|c.a--;", 
           "!!" + Paths.WinJs.BaseIntellisenseSetupPath,
            "@@" + WinJSTestFiles.latest_base,
            "!!" + Paths.WinJs.BaseIntellisensePath);
        }

        [TestMethod]
        public void LambdaCaptureDefinition()
        {
            TestDefinition("(|define:x|x) => {|reference:x|x + 1; }");
            TestDefinition("(|define:x|x) => |reference:x|x + 1");
            TestDefinition("|define:x|x => |reference:x|x + 1");
            TestDefinition("|define:x|x => {|reference:x|x + 1; }");
        }

        [TestMethod]
        public void Spread()
        {
            TestDefinition("var |define:x|x = 'hello world'; var arr = [...|reference:x|x];");
            TestDefinition("var |define:x|x = 'hello world'; function foo() {}; foo(...|reference:x|x);");
        }

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        public void ClassAndMemberDefinitions()
        {
            TestDefinition(ClassAndMemberDefinitionsTest);
        }
        #region Test Data
        const string ClassAndMemberDefinitionsTest = @"
            class |define:Base|Base { }

            class |define:Test|Test extends |reference:Base|Base {
                constructor() {}
                |define:method1|method1() {}
                static |define:sMethod1|sMethod1() {}
            }

            var test = new |reference:Test|Test();
            test.|reference:method1|method1();
            Test.|reference:sMethod1|sMethod1();

            {
                class |define:Test2|TestInBlock extends |reference:Base|Base {
                    constructor() {}
                    |define:method2|method2() {}
                    static |define:sMethod2|sMethod2() {}
                }

                let test = new |reference:Test2|TestInBlock();
                test.|reference:method2|method2();
                TestInBlock.|reference:sMethod2|sMethod2();
            }
            ";
        #endregion

        [TestMethod]
        [WorkItem(917987)]
        [TestCategory("Classes"), TestCategory("super")]
        public void SuperMemberDefinitions()
        {
            TestDefinition(SuperMemberDefinitionsTest);
        }
        #region Test Data
        const string SuperMemberDefinitionsTest = @"
                class A {
                  constructor() { this.|define:initialized|initialized = true; }
                  |define:method|method()      { return 'method A'; }
                  static |define:staticMethod|staticMethod() { return 'staticMethod A'; }
                }

                class B extends A {
                  constructor() {
                    super();
                    super.|reference:newStaticMethod|newStaticMethod();
                    super.|reference:staticMethod|staticMethod();
                  }
                  method() { return super(); }
                  superMethod() { 
                    super.|reference:newMethod|newMethod();
                    return super.|reference:method|method();
                  }
                  superMethodIndex() { 
                    super['|reference:newMethod|newMethod']();
                    return super[|reference:method|'method']();
                  }
                  static staticSuperMethod() { 
                    super.|reference:newStaticMethod|newStaticMethod();
                    return super.|reference:staticMethod|staticMethod();
                  }
                  static staticSuperMethodIndex() { 
                    super['|reference:newStaticMethod|newStaticMethod']();
                    return super[|reference:staticMethod|'staticMethod']();
                  }
                  lambdaIndex() {
                    var mysuper = x => {
                        return super[x](); 
                    }
                    return mysuper('method');
                  }
                }

                A.prototype.newMethod = |define:newMethod|function () { return 'newMethod A'; };
                A.newStaticMethod = |define:newStaticMethod|function () { return 'newStaticMethod A'; };
                ";
        #endregion

        #region Helpers

        struct DefintionResult
        {
            public IAuthorTestFile File;
            public int Location;
        }

        private void TestDefinition(string[] texts, Func<IAuthorTestContext, int, DefintionResult> getLocation)
        {
            // If any files are in the texts read them off disk
            texts = (from t in texts select t.StartsWith("file:") ? _session.ReadFile(t.Substring(5)).Text : t).ToArray();

            // Parse the requests
            var requests = (from text in texts let r = ParseRequests(text) select new { File = _session.FileFromText(r.Text), Requests = r }).ToArray();
            var files = from request in requests select request.File;

            // Set up the context
            var primaryFile = files.First();
            var contextFiles = files.Skip(1).ToArray();
            var context = _session.OpenContext(primaryFile, contextFiles);

            // Collect the definitions
            var definitions = requests.SelectMany(file => from request in file.Requests.Requests where request.Data.StartsWith("define:") select new { File = file.File, Name = request.Data.Substring(7), Offset = request.Offset }).ToDictionary(r => r.Name);

            // Collect the references
            var primaryRequests = requests.First();
            var references = from request in primaryRequests.Requests.Requests where request.Data.StartsWith("reference:") select new { Name = request.Data.Substring(10), Offset = request.Offset };

            foreach (var reference in references)
            {
                var result = getLocation(context, reference.Offset);
                if (reference.Name == "none")
                {
                    Assert.IsNull(result.File);
                }
                else
                {
                    Assert.IsNotNull(result.File);
                    var definition = definitions[reference.Name];
                    Assert.AreEqual(definition.File, result.File);
                    Assert.AreEqual(definition.Offset, result.Location);
                }
            }
        }

        private void TestDefinition(params string[] texts)
        {
            TestDefinition(texts, (context, offset) =>
            {
                DefintionResult result = new DefintionResult();
                context.GetDefinitionLocation(offset, out result.File, out result.Location);
                return result;
            });
        }

        //private void HurriedTestDefinition(params string[] texts)
        //{
        //    Action perform = () =>
        //    {
        //        TestDefinition(texts, (context, offset) =>
        //        {
        //            DefintionResult result = new DefintionResult();
        //            using (IDisposable hurry = ExecutionLimiter(context))
        //            {
        //                context.GetDefinitionLocation(offset, out result.File, out result.Location);
        //            }
        //            return result;
        //        });
        //    };

        //    WithMTASession(perform);
        //}

        protected override ParsedRequests ParseRequests(string text)
        {
            // If the text begins with the prefix '@@', skip parsing requests
            if (text.StartsWith("@@"))
            {
                return new ParsedRequests() { Requests = Enumerable.Empty<Request>(), Text = text.Substring(2) };
            }

            var builder = new StringBuilder(text.Length);
            var requests = new List<Request>();

            for (int i = 0; i < text.Length; i++)
            {
                var ch = text[i];

                if (ch == '|')
                {
                    string data = null;
                    var j = i + 1;
                    while (j < text.Length)
                    {
                        ch = text[j++];
                        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == ',' || ch == ':')
                            continue;
                        if (ch == '|')
                        {
                            data = text.Substring(i + 1, j - i - 2);
                            j = j - 1;
                        }
                        break;
                    }
                    if (data != null)
                    {
                        requests.Add(new Request() { Offset = builder.Length, Data = data });
                        i = j;
                    }
                    else
                        builder.Append(ch);
                }
                else
                    builder.Append(ch);
            }
            return new ParsedRequests() { Requests = requests.ToArray(), Text = builder.ToString() };
        }

        #endregion
    }

    #region Helper classes
    class Definition
    {
        public IAuthorTestFile File;
        public int Location;
    }

    static class Extensions
    {
        public static Definition GetDefinitionLocation(this IAuthorTestContext context, int offset)
        {
            IAuthorTestFile file;
            int location;
            context.GetDefinitionLocation(offset, out file, out location);
            return new Definition() { File = file, Location = location };
        }
    }
    #endregion

}
