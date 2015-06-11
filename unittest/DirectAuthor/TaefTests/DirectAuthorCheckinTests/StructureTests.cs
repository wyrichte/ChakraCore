using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class StructureTests : DirectAuthorTest
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
        [WorkItem(704815)]
        public void RangeOfFunctionMembersDeclaredInNamespaceDefine()
        {
            var scenario = DefineScenario(@"WinJS.Namespace.define('Foo', {
    // Used for mocking in tests
    _setHasWinRT: {
        value: function (value) {
            hasWinRT = value;
        },
        configurable: false,
        writable: false,
        enumerable: false
    }
});");

            var actualStructure = scenario.GetStructure(
                    "!!" + Paths.WinJs.BaseIntellisenseSetupPath,
                    "@@" + WinJSTestFiles.latest_base,
                    "!!" + Paths.WinJs.BaseIntellisensePath
                );

            var expectedStructure =
                scenario.Program(
                    scenario.Global(),
                    scenario.Namespace("Foo",
                        scenario.FunctionWithRegion("_setHasWinRT(value)", regionStartingText: "_setHasWinRT:")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);
            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);
        }


        [TestMethod]
        [WorkItem(id: 703497)]
        public void When_a_static_function_is_defined_under_a_dynamic_function_then_it_should_be_a_child_of_the_dynamic_node()
        {
            var scenario = DefineScenario(@"
                function define(ctor)
                {
                    ctor.prototype = { func: function(){} };
                    return ctor;
                }

                this.Robot = define(function () {
                    function underDynamicCtor() {

                    }
                });");

            var actualStructure = scenario.GetStructure();
            var expectedStructure =
                scenario.Program(
                    scenario.Global(
                        scenario.Node(
                        scenario.Function("define(ctor)"),
                            scenario.Node(
                            scenario.Object("ctor.prototype", offsetAdjustment: "ctor.prototype = ".Length),
                                scenario.Region(regionStartingText: "ctor.prototype"),
                                scenario.FunctionWithRegion("ctor.prototype.func()", regionStartingText: "func:")
                            )
                        )
                    ),
                    scenario.ClassWithMembers("Robot", "function ()",
                        scenario.FunctionWithRegion("func()", regionStartingText: "func:"),
                        scenario.Node(
                        scenario.Function("Robot()", regionStartingText: "function ()"),
                            scenario.Function("underDynamicCtor()")
                        )
                    )
                );

            AssertAreStructurallyEqual(
                expected: expectedStructure,
                actual: actualStructure);

            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);
        }

        [TestMethod]
        [WorkItem(id: 703497)]
        public void When_a_static_member_is_under_a_dynamic_member_we_should_add_the_static_children()
        {
            // the Identity function ( id(v) below )
            // forces static items to be considered dynamic 
            // because we can't detect the results name until running it
            var scenario = DefineScenario(@"
                function id(v) { return v; }
                function Robotoid() { }

                Robotoid.prototype = id({
                    modelName: '',
                    on: function () {
                        function fooStatic() {
                            return 3;
                        }
                    }
                });

                Robotoid.destroy = id(function () {
                    function destroyStatic() {
                    }
                });
");

            var actualStructure = scenario.GetStructure();

            var expectedStructure = scenario.Program(

                    scenario.Global(
                        scenario.Function("id(v)"),
                        scenario.Function("Robotoid()")                        
                    ),

                    scenario.ClassWithMembers("Robotoid",
                        scenario.Field("modelName"),
                        scenario.Node(
                            scenario.FunctionWithRegion("on()", regionStartingText: "on: "),
                                scenario.Function("fooStatic()")
                        ),
                        scenario.Node(
                            scenario.Function("destroy()", regionStartingText: "destroy =", adjustToFunctionStart: true),
                                scenario.Function("destroyStatic()")
                        ),
                        scenario.Function("Robotoid()")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);
            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);

        }

        [TestMethod]
        [WorkItem(id: 750115)]
        public void When_a_static_function_is_defined_in_a_literal_it_should_have_a_span_include_the_member()
        {
            var scenario = DefineScenario(@"
                (function (){
                    var literal = {
                        anon: function() { },
                        named: function myFunc() { }
                    };
                })();
            ");

            var actualStructure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                    scenario.Global(
                        scenario.Node(
                        scenario.Function("function ()", isIIFE: true),
                            scenario.Node(
                            scenario.ObjectWithRegion("literal", regionStartingText: "var literal"),
                                scenario.FunctionWithRegion("literal.anon()", regionStartingText: "anon: "),
                                scenario.FunctionWithRegion("myFunc()", regionStartingText: "named: ")
                            )
                        )
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);
            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);
        }

        [TestMethod]
        [WorkItem(id: 739923)]
        public void When_a_namespace_has_an_emptry_string_field_it_should_be_visible()
        {
            var scenario = DefineScenario(@"
                var namespace = {
                    fieldNS: '',
                    fieldNS2: 0,
                    memberNS: function () { },
                    fieldNS3: 1
                };");

            var actualStructure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(
                    scenario.Global(
                        scenario.Variable("namespace"),
                        scenario.ObjectWithRegion("namespace", regionStartingText: "var namespace"),
                        scenario.FunctionWithRegion("namespace.memberNS()", regionStartingText: "memberNS: ")
                    ),
                    scenario.Namespace("namespace",
                        scenario.Field("fieldNS"),
                        scenario.Field("fieldNS2"),
                        scenario.FunctionWithRegion("memberNS()", regionStartingText: "memberNS: "),
                        scenario.Field("fieldNS3")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);
        }

        [TestMethod]
        [WorkItem(id: 739923)]
        public void When_a_namespace_has_an_emptry_string_field_it_should_be_visible_when_winjs_is_included()
        {
            var scenario = DefineScenario(@"
                var namespace = {
                    fieldNS: '',
                    fieldNS2: 0,
                    memberNS: function () { },
                    fieldNS3: 1
                };");

            var actualStructure = scenario.GetStructure(
                    "!!" + Paths.WinJs.BaseIntellisenseSetupPath,
                    "@@" + WinJSTestFiles.latest_base,
                    "!!" + Paths.WinJs.BaseIntellisensePath,
                    "@@" + WinJSTestFiles.latest_ui
                );

            var expectedStructure =
                scenario.Program(
                    scenario.Global(
                        scenario.Variable("namespace"),
                        scenario.ObjectWithRegion("namespace", regionStartingText: "var namespace"),
                        scenario.FunctionWithRegion("namespace.memberNS()", regionStartingText: "memberNS: ")
                    ),
                    scenario.Namespace("namespace",
                        scenario.Field("fieldNS"),
                        scenario.Field("fieldNS2"),
                        scenario.FunctionWithRegion("memberNS()", regionStartingText: "memberNS: "),
                        scenario.Field("fieldNS3")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);
        }

        [TestMethod]
        [WorkItem(id: 740180)]
        public void When_a_class_is_defined_using_object_create_its_members_should_be_discovered()
        {
            var scenario = DefineScenario(@"
                var myClass = Object.create({
                    fieldCLS1: '',
                    fieldCLS2: 0,
                    memberCLS: function () { },
                    fieldCLS3: 1,
                    fieldCLS4: false
                });

                myClass.funcCLS = function(a) { };");

            var actualStructure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(
                    scenario.Global(
                        scenario.Variable("myClass", useSemicolonRegion: true),
                        scenario.FunctionWithRegion("myClass.funcCLS(a)")
                    ),
                    scenario.RawClassWithMembers("myClass",
                        scenario.Field("fieldCLS1"),
                        scenario.Field("fieldCLS2"),
                        scenario.FunctionWithRegion("memberCLS()", regionStartingText: "memberCLS: "),
                        scenario.Field("fieldCLS3"),
                        scenario.Field("fieldCLS4"),
                        scenario.FunctionWithRegion("funcCLS(a)", regionStartingText: "myClass.funcCLS")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);
            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);
        }

        [TestMethod]
        public void FunctionDeclarations()
        {
            var structure = StructureOf(FunctionDeclarationsText);
            AssertAreStructurallyEqual(
                new object[] {
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal, HasChildren = true },
                    new { ContainerName = (string)null, ItemName = "foo()", Kind = AuthorStructureNodeKind.asnkFunction, HasChildren = false },
                    new { ContainerName = (string)null, ItemName = "bar()", Kind = AuthorStructureNodeKind.asnkFunction, HasChildren = false },
                    new { ItemName = "outer()", Kind = AuthorStructureNodeKind.asnkFunction, HasChildren = true },
                    new { ContainerName = (string)null, ItemName = "inner1()", Kind = AuthorStructureNodeKind.asnkFunction, HasChildren = false },
                    new { ItemName = "inner2()", Kind = AuthorStructureNodeKind.asnkFunction, HasChildren = true },
                    new { ContainerName = (string)null, ItemName = "inner2_inner1()", Kind = AuthorStructureNodeKind.asnkFunction, HasChildren = false },
                    new { ContainerName = (string)null, ItemName = "inner2_inner2()", Kind = AuthorStructureNodeKind.asnkFunction, HasChildren = false }
                }, structure);
            VerifyHierarchy(structure, 0, 1, 1, 1, 4, 4, 6, 6);
            VerifyRanges(structure, FunctionDeclarationsText);
        }
        #region Test data
        const string FunctionDeclarationsText = @"
function foo() {
}

function bar() {
}

function outer() {
  function inner1() {
  }
  function inner2() {
    function inner2_inner1() {
    }
    function inner2_inner2() {
    }
  }
}";
        #endregion

        [TestMethod]
        public void ModulesWithFunctions()
        {
            var structure = StructureOf(ModulesWithFunctionsText);
            AssertAreStructurallyEqual(
                new[]
                {
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = (string)null, ItemName = "A", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "function (A)", ItemName = "function (A)", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "a1()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = "function (B)", ItemName = "function (B)", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "b1()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "B", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "function (B)", ItemName = "function (B)", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "b2()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "C", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "function (C)", ItemName = "function (C)", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = "function (B)", ItemName = "function (B)", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "b2()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = "A", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    new { ContainerName = (string)null, ItemName = "a1()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = "B", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    new { ContainerName = (string)null, ItemName = "b1()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = "B", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    new { ContainerName = (string)null, ItemName = "b2()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = "C", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    new { ContainerName = "B", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    new { ContainerName = (string)null, ItemName = "b2()", Kind = AuthorStructureNodeKind.asnkFunction },

                }, structure);
            VerifyHierarchy(structure,
                0,  //  1: global
                1,  //  2:  A
                1,  //  3:  function (A)
                3,  //  4:   a1
                3,  //  5:   function (B)
                5,  //  6:    b1
                1,  //  7:  B
                1,  //  8:  function (B)
                8,  //  9:   b2
                1,  // 10:  C
                1,  // 11:  function (C)
                11, // 12:   function (B)
                12, // 13:    b2
                0,  // 14: A
                14, // 15:  a1
                14, // 16:  B
                16, // 17:   b1
                0,  // 18: B
                18, // 19:  b2
                0,  // 20: C
                20, // 21:  B
                21);// 22:   b2
            VerifyRanges(structure, ModulesWithFunctionsText);
        }
        #region Test data
        const string ModulesWithFunctionsText = @"var A;
(function (A) {
    function a1() {
    }
    A.a1 = a1;
    (function (B) {
        function b1() {
        }
        B.b1 = b1;
    })(A.B || (A.B = {}));
    var B = A.B;

})(A || (A = {}));

var B;
(function (B) {
    function b2() {
    }
    B.b2 = b2;
})(B || (B = {}));

var C;
(function (C) {
    (function (B) {
        function b2() {
        }
        B.b2 = b2;
    })(C.B || (C.B = {}));
    var B = C.B;

})(C || (C = {}));
";
        #endregion

        [TestMethod]
        public void When_a_winjs_page_is_defined_its_span_should_be_the_object_literal()
        {
            const string TestText = @"
WinJS.UI.Pages.define('/wow.html', {
    _item: 3,
    ready: function() {
        return 3;
    }
});
";
            var structure = StructureOf(TestText,
                "@@" + WinJSTestFiles.latest_base,
                "!!" + Paths.WinJs.BaseIntellisensePath);
        }

        [TestMethod]
        public void When_a_field_is_null_it_should_still_appear_as_a_member()
        {
            var scenario = DefineScenario(@"
this.Namespace = {
    nullField: null,
    func: function() { }
};
");
            var structure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(

                    scenario.Global(),
                    scenario.Namespace("Namespace",
                        scenario.Field("nullField"),
                        scenario.FunctionWithRegion("func()", regionStartingText: "func:")
                    )

                );

            AssertAreStructurallyEqual(expectedStructure, structure);
            VerifyHierarchy(structure, expectedStructure.Heirarchy);
        }

        [TestMethod]
        public void ConstructorFunctionsWithPrototypes()
        {
            var scenario = DefineScenario(ConstructorFunctionsWithPrototypesText);

            var actualStructure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(

                    scenario.Global(
                        scenario.Function("Foo()"),
                        scenario.FunctionWithRegion("Foo.prototype.a()"),
                        scenario.FunctionWithRegion("Foo.prototype.b()"),
                        scenario.Function("Goo()"),
                        scenario.FunctionWithRegion("Goo.prototype.a()"),
                        scenario.FunctionWithRegion("Goo.prototype.b()")
                    ),

                    scenario.ClassWithMembers("Foo",
                        scenario.FunctionWithRegion("a()", regionStartingText: "Foo.prototype.a"),
                        scenario.FunctionWithRegion("b()", regionStartingText: "Foo.prototype.b"),
                        scenario.Function("Foo()")
                    ),

                    scenario.ClassWithMembers("Goo",
                        scenario.FunctionWithRegion("a()", regionStartingText: "Goo.prototype.a"),
                        scenario.FunctionWithRegion("b()", regionStartingText: "Goo.prototype.b"),
                        scenario.Function("Goo()")
                    )
                );



            AssertAreStructurallyEqual(expectedStructure, actualStructure);
            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);
            VerifyRanges(actualStructure, ConstructorFunctionsWithPrototypesText);
        }
        #region Test data
        const string ConstructorFunctionsWithPrototypesText = @"
function Foo() {

}

Foo.prototype.a = function () { 

};

Foo.prototype.b = function () { 

};

function Goo() {

}

Goo.prototype.a = function () {

}

Goo.prototype.b = function () {

}
";
        #endregion


        [TestMethod]
        public void NamepacesWithConstructorFunctions()
        {
            var scenario = DefineScenario(NamespacesWithConstructorFunctionsText);

            var actualStructure = scenario.GetStructure();

            var expectedStrcuture =
                scenario.Program(

                    scenario.Global(
                        scenario.Variable("A"),
                        scenario.FunctionWithRegion("A.Foo()", regionStartingText: "A.Foo ="),
                        scenario.FunctionWithRegion("A.Foo.prototype.b()", regionStartingText: "A.Foo.prototype.b"),
                        scenario.Variable("B"),
                        scenario.FunctionWithRegion("B.Goo()", regionStartingText: "B.Goo ="),
                        scenario.FunctionWithRegion("B.Goo.prototype.d()", regionStartingText: "B.Goo.prototype.d")
                    ),

                    scenario.Namespace("A",
                        scenario.Node(scenario.Class("Foo", regionStartingText: "A.Foo = ", adjustToFunctionStart: true),
                            scenario.FunctionWithRegion("b()", regionStartingText: "A.Foo.prototype.b"),
                            scenario.Field("a"),
                            scenario.FunctionWithRegion("Foo()", regionStartingText: "A.Foo =")
                        )
                    ),

                    scenario.Namespace("B",
                        scenario.Node(scenario.Class("Goo", regionStartingText: "B.Goo =", adjustToFunctionStart: true),
                            scenario.FunctionWithRegion("d()", regionStartingText: "B.Goo.prototype.d"),
                            scenario.Field("c"),
                            scenario.FunctionWithRegion("Goo()", regionStartingText: "B.Goo =")
                        )
                    )
                );

            AssertAreStructurallyEqual(expectedStrcuture, actualStructure);

            VerifyHierarchy(actualStructure, expectedStrcuture.Heirarchy);

            VerifyRanges(actualStructure, NamespacesWithConstructorFunctionsText);
        }
        #region Test data
        const string NamespacesWithConstructorFunctionsText = @"
var A = {};
A.Foo = function () {
  this.a = 1;
};
A.Foo.prototype.b = function () { 

};

var B = {};
B.Goo = function () {
  this.c = 1;
};
B.Goo.prototype.d = function () {

};";
        #endregion

        [TestMethod]
        public void ClassInADomContext()
        {
            // Ensure that global only appears once. For example, since window is equal to global, global classes might
            // appear under a window namespace incorrectly.
            var scenario = DefineScenario(ClassInADomContextText);

            var actualStructure = scenario.GetStructure("!!" + Paths.DomWebPath);

            var expectedStructure =
                scenario.Program(

                    scenario.Global(
                        scenario.Function("Foo()"),
                        scenario.FunctionWithRegion("Foo.prototype.a()"),
                        scenario.FunctionWithRegion("Foo.prototype.b()")
                    ),

                    scenario.ClassWithMembers("Foo",
                        scenario.FunctionWithRegion("a()", regionStartingText: "Foo.prototype.a"),
                        scenario.FunctionWithRegion("b()", regionStartingText: "Foo.prototype.b"),
                        scenario.Function("Foo()")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);
            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);
        }
        #region Test data
        const string ClassInADomContextText = @"
function Foo() {

}

Foo.prototype.a = function () { 

};

Foo.prototype.b = function () { 

};";

        #endregion

        [TestMethod]
        public void NamespacesContainingJustFunction()
        {
            var scenario = DefineScenario(NamespacesContainingJustFunctionText);

            var actualStructure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(

                    scenario.Global(
                        scenario.Node(
                            scenario.Function("function (A)", isIIFE: true),
                            scenario.FunctionWithRegion("A.a()"),
                            scenario.FunctionWithRegion("A.b()")
                        )
                    ),
                    
                    scenario.Namespace("A",
                        scenario.FunctionWithRegion("a()", regionStartingText: "A.a"),
                        scenario.FunctionWithRegion("b()", regionStartingText: "A.b")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);

            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);

            VerifyRanges(actualStructure, NamespacesContainingJustFunctionText);
        }
        #region Test data
        const string NamespacesContainingJustFunctionText = @"
(function (A) {
    A.a = function () {
    }
    A.b = function () {
    }
})(this.A = this.A || {});
";
        #endregion

        [TestMethod]
        public void ModulesInsteadOfFunction()
        {
            // Ensure a function that is recorded as a module only shows up as a module, not as a function as well.
            var structure = StructureOf(ModulesInsteadOfFunctionText);
            AssertAreStructurallyEqual(
                new[]
                {
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = "function ()", ItemName = "function ()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "a()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "b()", Kind = AuthorStructureNodeKind.asnkFunction },
                }, structure);
            VerifyHierarchy(structure,
                0, // 1: Global
                1, // 2: function ()
                2, // 3: a
                2);// 4: b
            VerifyRanges(structure, ModulesInsteadOfFunctionText);
        }
        #region Test data
        const string ModulesInsteadOfFunctionText = @"
(function () {
  function a() {
  }
  function b() {
  }
})();
";
        #endregion

        [TestMethod]
        public void ObjectLiterals()
        {
            var structure = StructureOf(ObjectLiteralsText);
            AssertAreStructurallyEqual(
                new[]
                {
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal }
                }, structure);
        }
        #region Test data
        const string ObjectLiteralsText = @"
define({
  foo: function () {
  },
  bar: function () {
  }
});
";
        #endregion

        [TestMethod]
        public void ObjectLiteralWithMoreThanThreeItems()
        {
            var structure = StructureOf(ObjectLiteralWithMoreThanThreeItemsText);
            AssertAreStructurallyEqual(
                new[]
                {
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal }
                }, structure);
        }
        #region Test data
        const string ObjectLiteralWithMoreThanThreeItemsText = @"
define({
  foo: function () {
  },
  bar: function () {
  },
  baz: function () {
  },
  goo: function () {
  },
  zoo: function () { 
  }
});
";
        #endregion

        [TestMethod]
        public void ModuleWithMoreThanTwoParameters()
        {
            var structure = StructureOf(ModuleWithMoreThanTwoParametersText);
            AssertAreStructurallyEqual(
                new[]
                {
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = "function (a, b, c, d)", ItemName = "function (a, b, c, d)", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "bar()", Kind = AuthorStructureNodeKind.asnkFunction },
                }, structure);
        }
        #region Test data
        const string ModuleWithMoreThanTwoParametersText = @"
(function (a, b, c, d) {
  function bar() {
  }
})(1,2,3,4);
";
        #endregion

        [TestMethod]
        [WorkItem(598666)]
        public void FunctionSyntaxError()
        {
            var structure = StructureOf(FunctionSyntaxError_Text);
            Assert.IsNotNull(structure);
        }
        #region Test data
        const string FunctionSyntaxError_Text = @"function WinJS.Class.define(function Foo_ctor() { });";
        #endregion

        [TestMethod]
        public void OverwrittingHelper()
        {
            var text = "_$analyzeClasses = undefined";
            var structure = StructureOf(text);
            Assert.IsNotNull(structure);
        }

        [TestMethod]
        [WorkItem(617076)]
        public void LiteralLabelsAndFunctionRanges()
        {
            var text = LiteralLabelsAndFunctionRanges_Text;
            var structure = StructureOf(text);
            var initItemStaticAnalysis = structure.Where(s => s.ItemName == "o.init()").FirstOrDefault();
            Assert.AreEqual(text.IndexOf("init: ") + "init: ".Length, initItemStaticAnalysis.Region.Offset);
            var initItemDynamicAnalysis = structure.Where(s => s.ItemName == "init()").FirstOrDefault();
            Assert.AreEqual(text.IndexOf("init: ") + "init: ".Length, initItemDynamicAnalysis.Region.Offset);
        }
        #region Test data
        const string LiteralLabelsAndFunctionRanges_Text = @"var o = {
maxwidth: 600,
maxheight: 400,
gimmeMax: function () {
return this.maxwidth + ""x"" + this.maxheight;
},
init: function () {
}
};
";
        #endregion


        [TestMethod]
        [WorkItem(598769)]
        public void ExplicitModuleNames()
        {
            var structure = StructureOf(ExplicitModuleNames_Text);
            AssertAreStructurallyEqual(
                new[]
                {
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = "myFunc()", ItemName = "myFunc()", Kind = AuthorStructureNodeKind.asnkFunction},
                    new { ContainerName = (string)null, ItemName = "bar()", Kind = AuthorStructureNodeKind.asnkFunction }
                }, structure);
        }
        #region Test data
        const string ExplicitModuleNames_Text = @"(function myFunc() {
var c;
function bar() { };
})();
";
        #endregion

        [TestMethod]
        [WorkItem(616615)]
        public void ClassesWithProperties()
        {
            var structure = StructureOf(ClassesWithProperties_Text, WinJSTestFiles.latest_base);
            structure.First(s => s.ItemName == "firstName");
            structure.First(s => s.ItemName == "setter");

            // Properties with both a setter and getter should have an additional region for the setter as a child.
            var setterGetter = structure.First(s => s.ItemName == "setterGetter");
            Assert.IsNull(setterGetter.ContainerName, "A setter/getter property was not expected to have a container name");
            var auxRegion = structure.First(s => s.Container == setterGetter.Key);
            Assert.AreEqual(auxRegion.Kind, AuthorStructureNodeKind.asnkRegion);
        }

        #region Test data
        const string ClassesWithProperties_Text = @"var Person = WinJS.Class.define(function () {},
    {
        firstName: { get: function () { } },
        setter: { set: function () { } },
        setterGetter: { get: function() { }, set: function () { } },
        eatDinner: function () {
        }
    });
";
        #endregion

        [TestMethod]
        public void CompletionsImmediatelyAfterAStructureRequest()
        {
            var primaryFile = _session.FileFromText(CompletionsImmediatelyAfterAStructureRequest_Text);
            var context = _session.OpenContext(primaryFile);
            context.GetCompletionsAt(CompletionsImmediatelyAfterAStructureRequest_Text.Length).ToEnumerable().ExpectContains("use");
            var structure = context.GetStructure();
            Assert.IsNotNull(structure);
            context.GetCompletionsAt(CompletionsImmediatelyAfterAStructureRequest_Text.Length).ToEnumerable().ExpectContains("use");
        }
        #region Test data
        const string CompletionsImmediatelyAfterAStructureRequest_Text = @"var variable =
/* SeaJS v1.0.1 | seajs.com | MIT Licensed */
this.seajs = { _seajs: this.seajs }; seajs.version = '1.0.1'; seajs._data = { config: { debug: '', preload: [] }, memoizedMods: {}, pendingMods: [] }; seajs._util = {}; seajs._fn = {};
(function (a) {
    var e = Object.prototype.toString, g = Array.prototype; a.isString = function (a) { return e.call(a) === '[object String]' }; a.isFunction = function (a) { return e.call(a) === '[object Function]' }; a.isArray = Array.isArray || function (a) { return e.call(a) === '[object Array]' }; a.indexOf = g.indexOf ? function (a, c) { return a.indexOf(c) } : function (a, c) { for (var b = 0, i = a.length; b < i; b++) if (a[b] === c) return b; return -1 }; var f = a.forEach = g.forEach ? function (a, c) { a.forEach(c) } : function (a, c) {
        for (var b = 0, i = a.length; b < i; b++) c(a[b], b,
        a)
    }; a.map = g.map ? function (a, c) { return a.map(c) } : function (a, c) { var b = []; f(a, function (a, e, h) { b.push(c(a, e, h)) }); return b }; a.filter = g.filter ? function (a, c) { return a.filter(c) } : function (a, c) { var b = []; f(a, function (a, e, h) { c(a, e, h) && b.push(a) }); return b }; a.now = Date.now || function () { return (new Date).getTime() }
})(seajs._util);
(function (a, e) { function g(a) { var c = ['{'], b; for (b in a) if (typeof a[b] === 'number' || typeof a[b] === 'string') c.push(b + ': ' + a[b]), c.push(', '); c.pop(); c.push('}'); return c.join('') } var f = e.config; a.error = function (a) { if (a.type === 'error') throw 'Error occurs! ' + g(a); else if (f.debug && typeof console !== 'undefined') console[a.type](g(a)) } })(seajs._util, seajs._data);
(function (a, e, g) {
    function f(a) { a = a.match(/.*(?=\/.*$)/); return (a ? a[0] : '.') + '/' } function j(m) { m = m.replace(/([^:\/])\/+/g, '$1/'); if (m.indexOf('.') === -1) return m; for (var d = m.split('/'), c = [], b, h = 0, e = d.length; h < e; h++) b = d[h], b === '..' ? (c.length === 0 && a.error({ message: 'invalid path: ' + m, type: 'error' }), c.pop()) : b !== '.' && c.push(b); return c.join('/') } function c(a) { a = j(a); /#$/.test(a) ? a = a.slice(0, -1) : a.indexOf('?') === -1 && !/\.(?:css|js)$/.test(a) && (a += '.js'); return a } function b(a) {
        function d(a, b) {
            var m = a[b]; c &&
            c.hasOwnProperty(m) && (a[b] = c[m])
        } var c = n.alias, a = a.split('/'), b = a.length - 1; d(a, 0); b && d(a, b); return a.join('/')
    } function i(d) { a.forEach(n.map, function (a) { a && a.length === 2 && (d = d.replace(a[0], a[1])) }); return d } function k(a) { return a.replace(/^(\w+:\/\/[^/]*)\/?.*$/, '$1') } function h(d, h, e) {
        if (p[d]) return d; !e && n.alias && (d = b(d)); h = h || l; q(d) && (d = '.' + d.substring(1)); d.indexOf('://') === -1 && (d.indexOf('./') === 0 || d.indexOf('../') === 0 ? (d = d.replace(/^\.\//, ''), d = f(h) + d) : d.indexOf('/') === 0 ? d = k(h) + d : (n.base || a.error({
            message: 'the config.base is empty',
            from: 'id2Uri', type: 'error'
        }), d = n.base + '/' + d)); d = c(d); n.map && (d = i(d)); p[d] = !0; return d
    } function d(d, b) { return a.map(d, function (a) { return h(a, b) }) } function r(d, b) { if (!d || d.ready) return !1; var c = d.dependencies || []; if (c.length) if (a.indexOf(c, b) !== -1) return !0; else for (var h = 0; h < c.length; h++) if (r(o[c[h]], b)) return !0; return !1 } function s(d, b) { a.forEach(b, function (b) { a.indexOf(d, b) === -1 && d.push(b) }) } function q(a) { return a.charAt(0) === '~' } var n = e.config, g = g.location, l = g.protocol + '//' + g.host + g.pathname; l.indexOf('\\') !==
    -1 && (l = l.replace(/\\/g, '/')); var p = {}, o = e.memoizedMods; a.dirname = f; a.id2Uri = h; a.ids2Uris = d; a.memoize = function (a, b, c) { var e; e = a ? h(a, b, !0) : b; c.dependencies = d(c.dependencies, e); o[e] = c; a && b !== e && (a = o[b]) && s(a.dependencies, c.dependencies) }; a.setReadyState = function (d) { a.forEach(d, function (a) { if (o[a]) o[a].ready = !0 }) }; a.getUnReadyUris = function (d) { return a.filter(d, function (a) { a = o[a]; return !a || !a.ready }) }; a.removeCyclicWaitingUris = function (d, b) { return a.filter(b, function (a) { return !r(o[a], d) }) }; a.isInlineMod =
    q; a.pageUrl = l; if (n.debug) a.realpath = j, a.normalize = c, a.parseAlias = b, a.getHost = k
})(seajs._util, seajs._data, this);
(function (a, e) {
    function g(d, b) { function c() { c.isCalled = !0; b(); clearTimeout(h) } d.nodeName === 'SCRIPT' ? f(d, c) : j(d, c); var h = setTimeout(function () { c(); a.error({ message: 'time is out', from: 'getAsset', type: 'warn' }) }, e.config.timeout) } function f(a, b) { a.addEventListener ? (a.addEventListener('load', b, !1), a.addEventListener('error', b, !1)) : a.attachEvent('onreadystatechange', function () { var c = a.readyState; (c === 'loaded' || c === 'complete') && b() }) } function j(a, b) {
        a.attachEvent ? a.attachEvent('onload', b) : setTimeout(function () {
            c(a,
            b)
        }, 0)
    } function c(a, b) { if (!b.isCalled) { var h = !1; if (i) a.sheet && (h = !0); else if (a.sheet) try { a.sheet.cssRules && (h = !0) } catch (e) { e.code === 1E3 && (h = !0) } h ? setTimeout(function () { b() }, 1) : setTimeout(function () { c(a, b) }, 1) } } var b = document.getElementsByTagName('head')[0], i = navigator.userAgent.indexOf('AppleWebKit') !== -1; a.getAsset = function (a, c, h) {
        var i = /\.css(?:\?|$)/i.test(a), f = document.createElement(i ? 'link' : 'script'); h && f.setAttribute('charset', h); g(f, function () {
            c && c.call(f); if (!i && !e.config.debug) {
                try {
                    if (f.clearAttributes) f.clearAttributes();
                    else for (var a in f) delete f[a]
                } catch (d) { } b.removeChild(f)
            }
        }); i ? (f.rel = 'stylesheet', f.href = a, b.appendChild(f)) : (f.async = !0, f.src = a, b.insertBefore(f, b.firstChild)); return f
    }; a.assetOnload = g; var k = null; a.getInteractiveScript = function () { if (k && k.readyState === 'interactive') return k; for (var a = b.getElementsByTagName('script'), c = 0; c < a.length; c++) { var h = a[c]; if (h.readyState === 'interactive') return k = h } return null }; a.getScriptAbsoluteSrc = function (a) { return a.hasAttribute ? a.src : a.getAttribute('src', 4) }; var h =
    'seajs-ts=' + a.now(); a.addNoCacheTimeStamp = function (a) { return a + (a.indexOf('?') === -1 ? '?' : '&') + h }; a.removeNoCacheTimeStamp = function (a) { var b = a; a.indexOf(h) !== -1 && (b = a.replace(h, '').slice(0, -1)); return b }
})(seajs._util, seajs._data);
(function (a, e, g, f) {
    function j(b, d) { function e() { a.setReadyState(f); d() } var f = a.getUnReadyUris(b); if (f.length === 0) return e(); for (var i = 0, g = f.length, l = g; i < g; i++) (function (b) { function d() { var c = (k[b] || 0).dependencies || [], h = c.length; if (h) c = a.removeCyclicWaitingUris(b, c), h = c.length; h && (l += h, j(c, function () { l -= h; l === 0 && e() })); --l === 0 && e() } k[b] ? d() : c(b, d) })(f[i]) } function c(c, d) {
        function f() {
            if (e.pendingMods) a.forEach(e.pendingMods, function (b) { a.memoize(b.id, c, b) }), e.pendingMods = []; i[c] && delete i[c]; k[c] ||
            a.error({ message: 'can not memoized', from: 'load', uri: c, type: 'warn' }); d && d()
        } i[c] ? a.assetOnload(i[c], f) : (e.pendingModIE = c, i[c] = a.getAsset(b(c), f, e.config.charset), e.pendingModIE = null)
    } function b(b) { e.config.debug == 2 && (b = a.addNoCacheTimeStamp(b)); return b } var i = {}, k = e.memoizedMods; g.load = function (b, c, e) { a.isString(b) && (b = [b]); var i = a.ids2Uris(b, e); j(i, function () { var b = g.createRequire({ uri: e }), h = a.map(i, function (a) { return b(a) }); c && c.apply(f, h) }) }
})(seajs._util, seajs._data, seajs._fn, this);
(function (a) { a.Module = function (a, g, f) { this.id = a; this.dependencies = g || []; this.factory = f } })(seajs._fn);
(function (a, e, g) {
    g.define = function (f, j, c) {
        arguments.length === 1 ? (c = f, f = '') : a.isArray(f) && (c = j, j = f, f = ''); if (!a.isArray(j) && a.isFunction(c)) { for (var b = c.toString(), i = /[^.]\brequire\s*\(\s*['']?([^'')]*)/g, k = [], h, b = b.replace(/(?:^|\n|\r)\s*\/\*[\s\S]*?\*\/\s*(?:\r|\n|$)/g, '\n').replace(/(?:^|\n|\r)\s*\/\/.*(?:\r|\n|$)/g, '\n') ; h = i.exec(b) ;) h[1] && k.push(h[1]); j = k } var b = new g.Module(f, j, c), d; if (a.isInlineMod(f)) d = a.pageUrl; else if (document.attachEvent && !window.opera) (d = a.getInteractiveScript()) ? (d = a.getScriptAbsoluteSrc(d),
        e.config.debug == 2 && (d = a.removeNoCacheTimeStamp(d))) : d = e.pendingModIE; d ? a.memoize(f, d, b) : e.pendingMods.push(b)
    }
})(seajs._util, seajs._data, seajs._fn);
(function (a, e, g) {
    function f(c) {
        function b(b) {
            var g = a.id2Uri(b, c.uri), b = e.memoizedMods[g]; if (!b) return null; if (j(c, g)) return a.error({ message: 'found cyclic dependencies', from: 'require', uri: g, type: 'warn' }), b.exports; if (!b.exports) {
                var g = { uri: g, deps: b.dependencies, parent: c }, h = b.factory; b.id = g.uri; b.exports = {}; delete b.factory; delete b.ready; if (a.isFunction(h)) {
                    var d = b.uri; h.toString().search(/\sexports\s*=\s*[^=]/) !== -1 && a.error({ message: 'found invalid setter: exports = {...}', from: 'require', uri: d, type: 'error' });
                    g = h(f(g), b.exports, b); if (g !== void 0) b.exports = g
                } else if (h !== void 0) b.exports = h
            } return b.exports
        } b.async = function (a, b) { g.load(a, b, c.uri) }; return b
    } function j(a, b) { return a.uri === b ? !0 : a.parent ? j(a.parent, b) : !1 } g.createRequire = f
})(seajs._util, seajs._data, seajs._fn);
(function (a, e, g, f) {
    function j(b, c) { b !== void 0 && b !== c && a.error({ message: 'config is conflicted', previous: b, current: c, from: 'config', type: 'error' }) } var c = e.config, e = document.getElementById('seajsnode'); e || (e = document.getElementsByTagName('script'), e = e[e.length - 1]); var b = a.getScriptAbsoluteSrc(e), i; if (b) { var b = i = a.dirname(b), k = b.match(/^(.+\/)seajs\/[\d\.]+\/$/); k && (b = k[1]); c.base = b } c.main = e.getAttribute('data-main') || ''; c.timeout = 2E4; if (i && (f.location.search.indexOf('seajs-debug') !== -1 || document.cookie.indexOf('seajs=1') !==
    -1)) c.debug = !0, c.preload.push(i + 'plugin-map'); g.config = function (b) { for (var d in b) { var e = c[d], f = b[d]; if (e && d === 'alias') for (var g in f) f.hasOwnProperty(g) && (j(e[g], f[g]), e[g] = f[g]); else e && (d === 'map' || d === 'preload') ? (a.isArray(f) || (f = [f]), a.forEach(f, function (a) { a && e.push(a) })) : c[d] = f } b = c.base; if (b.indexOf('://') === -1) c.base = a.id2Uri(b + '#'); return this }
})(seajs._util, seajs._data, seajs._fn, this);
(function (a, e, g) { var f = e.config; g.use = function (a, c) { var b = f.preload, e = b.length; e ? g.load(b, function () { f.preload = b.slice(e); g.use(a, c) }) : g.load(a, c) }; (e = f.main) && g.use([e]); (function (e) { if (e) { for (var c = { 0: 'config', 1: 'use', 2: 'define' }, b = 0; b < e.length; b += 2) g[c[e[b]]].apply(a, e[b + 1]); delete a._seajs } })((a._seajs || 0).args) })(seajs, seajs._data, seajs._fn);
(function (a, e, g, f) { if (a._seajs) f.seajs = a._seajs; else { a.config = g.config; a.use = g.use; var j = f.define; f.define = g.define; a.noConflict = function (c) { f.seajs = a._seajs; if (c) f.define = j, a.define = g.define; return a }; e.config.debug || (delete a._util, delete a._data, delete a._fn, delete a._seajs) } })(seajs, seajs._data, seajs._fn, this);

variable.";
        #endregion

        [TestMethod]
        public void StructureAndPreventExtensions()
        {
            var structure = StructureOf("Object.preventExtensions(this);");
            Assert.IsNotNull(structure);
        }

        [TestMethod]
        public void NamedObjectLiteralAndMethods()
        {
            var scenario = DefineScenario(NamedObjectLiteralAndMethods_Text);

            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                    scenario.Global(
                        scenario.Node(
                        scenario.Function("function ()", isIIFE: true),
                            scenario.Node(
                            scenario.ObjectWithRegion("foo", regionStartingText: "var foo"),
                                scenario.FunctionWithRegion("foo.bar()", regionStartingText: "bar:"),
                                scenario.FunctionWithRegion("foo.baz()", regionStartingText: "baz:")
                            ),
                            scenario.Node(
                            scenario.ObjectWithRegion("foo.goo"),
                                scenario.FunctionWithRegion("foo.goo.zoo()", regionStartingText: "zoo: "),
                                scenario.FunctionWithRegion("foo.goo.zaz()", regionStartingText: "zaz: ")
                            )
                        )
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, structure);
            VerifyHierarchy(structure, expectedStructure.Heirarchy);
        }
        #region Test data
        const string NamedObjectLiteralAndMethods_Text = @"
(function () {
  var foo = {
    bar: function () { },
    baz: function () { }
  };
  foo.goo = {
    zoo: function () { },
    zaz: function () { }
  };
})();";

        #endregion

        // Intellisense DeclareNavigationContainer Tests

        [TestMethod]
        public void DeclareNavigationContainer_ValidObjectDisplayNameGlyph()
        {
            var scenario = DefineScenario(ValidObjectDisplayNameGlyphText);
            
            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(

                    scenario.Global(),
                    scenario.Node(
                        new { ContainerName = "foobarContainer", Glyph = "vs:GlyphGroupEnum", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace},
                        scenario.FunctionWithRegion("foo()", "foo: "),
                        scenario.FunctionWithRegion("bar()", "bar: ")
                    )

                );

            AssertAreStructurallyEqual(expectedStructure, structure);
            VerifyHierarchy(structure, expectedStructure.Heirarchy);
        }
        #region Test data
        const string ValidObjectDisplayNameGlyphText = @"
intellisense.declareNavigationContainer({
    foo: function(){ },
    bar: function(){ } }, 'foobarContainer', 'vs:GlyphGroupEnum');
}";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_NullObject()
        {
            var structure = StructureOf(NullObjectText);

            AssertAreStructurallyEqual(
               new[]
                {
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal }
                }, structure);
        }
        #region Test data
        const string NullObjectText = @"
intellisense.declareNavigationContainer(null, 'foobarContainer');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_UndefinedObject()
        {
            var structure = StructureOf(UndefinedObjectText);

            AssertAreStructurallyEqual(
               new[]
                {
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal }
                }, structure);
        }
        #region Test data
        const string UndefinedObjectText = @"
intellisense.declareNavigationContainer(undefined, 'foobarContainer');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_NullDisplayName()
        {
            var structure = StructureOf(NullDisplayNameText);

            AssertAreStructurallyEqual(
               new[]
                {
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal }
                }, structure);
        }
        #region Test data
        const string NullDisplayNameText = @"
intellisense.declareNavigationContainer({
    foo: function(){ },
    bar: function(){ } }, null);
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_UndefinedDisplayName()
        {
            var structure = StructureOf(UndefinedDisplayNameText);

            AssertAreStructurallyEqual(
               new[]
                {
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal }
                }, structure);
        }
        #region Test data
        const string UndefinedDisplayNameText = @"
intellisense.declareNavigationContainer({
    foo: function(){ },
    bar: function(){ } }, undefined);
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_InvalidDisplayName()
        {
            var structure = StructureOf(InvalidDisplayNameText);

            AssertAreStructurallyEqual(
               new[]
                {
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal }
                }, structure);
        }
        #region Test data
        const string InvalidDisplayNameText = @"
intellisense.declareNavigationContainer({
    foo: function(){ },
    bar: function(){ }}, 123);
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_UnspecifiedGlyph()
        {
            var scenario = DefineScenario(UnspecifiedGlyphText);

            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(

                    scenario.Global(),
                    scenario.Node(
                        new { ContainerName = "foobarContainer", Glyph = "vs:GlyphGroupModule", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                        scenario.FunctionWithRegion("foo()", "foo: "),
                        scenario.FunctionWithRegion("bar()", "bar: ")
                    )

                );

            AssertAreStructurallyEqual(expectedStructure, structure);
            VerifyHierarchy(structure, expectedStructure.Heirarchy);
        }
        #region Test data
        const string UnspecifiedGlyphText = @"
intellisense.declareNavigationContainer({
    foo: function(){ },
    bar: function(){ }}, 'foobarContainer');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_InvalidGlyph()
        {
            var scenario = DefineScenario(InvalidGlyphText);

            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(

                    scenario.Global(),
                    scenario.Node(
                        new { ContainerName = "foobarContainer", Glyph = "vs:GlyphGroupModule", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                        scenario.FunctionWithRegion("foo()", "foo: "),
                        scenario.FunctionWithRegion("bar()", "bar: ")
                    )

                );

            AssertAreStructurallyEqual(expectedStructure, structure);
            VerifyHierarchy(structure, expectedStructure.Heirarchy);
        }

        #region Test data
        const string InvalidGlyphText = @"
intellisense.declareNavigationContainer({
    foo: function(){ },
    bar: function(){ }}, 'foobarContainer', 123);
}";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_OverrideGlobalObjectNameAndGlyph()
        {
            // First examine the structure of the container defined in the GlobalObject 
            var scenario = DefineScenario(OverrideGlobalObjectNameAndGlyphPartOneText);

            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                    scenario.Global(),
                    scenario.Namespace("testContainer",
                        scenario.FunctionWithRegion("foo()", "foo: "),
                        scenario.FunctionWithRegion("bar()", "bar: ")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, structure);
            VerifyHierarchy(structure, expectedStructure.Heirarchy);

            // Test to make sure declareNavigationContainer overrides with the user defined displayName and glyph
            var scenario2 = DefineScenario(OverrideGlobalObjectNameAndGlyphPartOneText + OverrideGlobalObjectNameAndGlyphPartTwoText);
            var structure2 = scenario2.GetStructure();

            var expectedStructure2 = scenario2.Program(
                    scenario2.Global(),
                    scenario2.Node(
                        new { ContainerName = "foobarContainer", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace},
                        scenario2.FunctionWithRegion("foo()", "foo: "),
                        scenario2.FunctionWithRegion("bar()", "bar: ")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure2, structure2);
            VerifyHierarchy(structure2, expectedStructure2.Heirarchy);

        }
        #region Test data
        const string OverrideGlobalObjectNameAndGlyphPartOneText = @"
this['testContainer'] = {
    foo: function(){ },
    bar: function(){ }
}";
        const string OverrideGlobalObjectNameAndGlyphPartTwoText = @"
intellisense.declareNavigationContainer(testContainer, 'foobarContainer', 'vs:GlyphGroupConstant');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_VarNameDefinedAndDisplayNameDefined()
        {
            var scenario = DefineScenario(VarNameDefinedAndDisplayNameDefined);

            // Test to make sure declareNavigationContainer uses displayName when the var name is also provided
            var structure = scenario.GetStructure();


            var expectedStructure = 
                scenario.Program(

                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = "testContainer", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "fooBarContainer", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                    scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")

                );


            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string VarNameDefinedAndDisplayNameDefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

var testContainer = callerName({
    foo: function(){ },
    bar: function(){ }
}, 'fooBarContainer');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_VarNameDefinedAndDisplayNameUndefined()
        {
            var scenario = DefineScenario(VarNameDefinedAndDisplayNameUndefined);

            // Test to make sure declareNavigationContainer uses the var name if no displayName is provided
            var structure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(

                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = "testContainer", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "testContainer", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                    scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")

                );


            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string VarNameDefinedAndDisplayNameUndefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

var testContainer = callerName({
    foo: function(){ },
    bar: function(){ }
}, '');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_VarNameUndefinedAndDisplayNameUndefined()
        {
            var scenario = DefineScenario(VarNameUndefinedAndDisplayNameUndefined);

            // Test to make sure if both the var name and the display name are undefined the container is not added to the structure
            var structure = scenario.GetStructure();

            AssertAreStructurallyEqual(
                new[]
                {
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction},
                    new { ContainerName = (string)null, Glyph = (string)null, ItemName = "?", Kind = AuthorStructureNodeKind.asnkVariable}
                }, structure);
        }
        #region Test data
        const string VarNameUndefinedAndDisplayNameUndefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

var  = callerName({
    foo: function(){ },
    bar: function(){ }
}, '');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_AssignmentNodeTypeAndDisplayNameDefined()
        {
            var scenario = DefineScenario(AssignmentNodeTypeAndDisplayNameDefined);

            // Test to make sure if both the assignment name and the display name are defined the display name is used
            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction },
                new { ContainerName = "fooBarContainer", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")
            );

            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string AssignmentNodeTypeAndDisplayNameDefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

testContainer = callerName({
    foo: function(){ },
    bar: function(){ }
}, 'fooBarContainer');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_AssignmentNodeTypeAndDisplayNameUndefined()
        {
            var scenario = DefineScenario(AssignmentNodeTypeAndDisplayNameUndefined);

            // Test to make sure if the assignment name and the display name is undefined,  the assignment name is used
            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction },
                new { ContainerName = "testContainer", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")
            );

            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string AssignmentNodeTypeAndDisplayNameUndefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

testContainer  = callerName({
    foo: function(){ },
    bar: function(){ }
}, '');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_AssignmentNodeTypeWithMultipleDotsAndDisplayNameUndefined()
        {
            var scenario = DefineScenario(AssignmentNodeTypeWithMultipleDotsAndDisplayNameUndefined);
            // Test to make sure if the assignment name has multiple dots the final portion of the dotted name is used
            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction },
                new { ContainerName = "four", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")
            );

            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string AssignmentNodeTypeWithMultipleDotsAndDisplayNameUndefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

one.two.three.four  = callerName({
    foo: function(){ },
    bar: function(){ }
}, '');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_MaxDisplayName()
        {
            var scenario = DefineScenario(MaxDisplayNameText);
            // Ensure the max number of displayName characters is 100
            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                new { ContainerName = "aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeeeffffffffffgggggggggghhhhhhhhhhiiiiiiiiiijjjjjjjjjj", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")
            );

            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string MaxDisplayNameText = @"
intellisense.declareNavigationContainer({
    foo: function(){ },
    bar: function(){ }}, 'aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeeeffffffffffgggggggggghhhhhhhhhhiiiiiiiiiijjjjjjjjjjkkkkkkkkkkmmmmmmmmmmnnnnnnnnnnooooooooooppppppppppqqqqqqqqqqrrrrrrrrrr', 'vs:GlyphGroupConstant');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_LetNameDefinedAndDisplayNameDefined()
        {
            var scenario = DefineScenario(LetNameDefinedAndDisplayNameDefined);

            // Test to make sure declareNavigationContainer uses displayName when the let name is also provided
            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "testContainer", Kind = AuthorStructureNodeKind.asnkVariable },
                new { ContainerName = "fooBarContainer", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")
            );

            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string LetNameDefinedAndDisplayNameDefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

let testContainer = callerName({
    foo: function(){ },
    bar: function(){ }
}, 'fooBarContainer');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_LetNameDefinedAndDisplayNameUndefined()
        {
            var scenario = DefineScenario(LetNameDefinedAndDisplayNameUndefined);

            // Test to make sure declareNavigationContainer uses the let name if no displayName is provided
            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "testContainer", Kind = AuthorStructureNodeKind.asnkVariable },
                new { ContainerName = "testContainer", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")
            );

            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string LetNameDefinedAndDisplayNameUndefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

let testContainer = callerName({
    foo: function(){ },
    bar: function(){ }
}, '');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_ConstNameDefinedAndDisplayNameDefined()
        {
            var scenario = DefineScenario(ConstNameDefinedAndDisplayNameDefined);

            // Test to make sure declareNavigationContainer uses displayName when the const name is also provided
            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "testContainer", Kind = AuthorStructureNodeKind.asnkVariable },
                new { ContainerName = "fooBarContainer", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")
            );

            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string ConstNameDefinedAndDisplayNameDefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

const testContainer = callerName({
    foo: function(){ },
    bar: function(){ }
}, 'fooBarContainer');
";
        #endregion

        [TestMethod]
        public void DeclareNavigationContainer_ConstNameDefinedAndDisplayNameUndefined()
        {
            var scenario = DefineScenario(ConstNameDefinedAndDisplayNameUndefined);

            // Test to make sure declareNavigationContainer uses the const name if no displayName is provided
            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "callerName(object, name)", Kind = AuthorStructureNodeKind.asnkFunction },
                new { ContainerName = (string)null, Glyph = (string)null, ItemName = "testContainer", Kind = AuthorStructureNodeKind.asnkVariable },
                new { ContainerName = "testContainer", Glyph = "vs:GlyphGroupConstant", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                scenario.FunctionWithRegion("foo()", regionStartingText: "foo: "),
                scenario.FunctionWithRegion("bar()", regionStartingText: "bar: ")
            );

            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string ConstNameDefinedAndDisplayNameUndefined = @"
function callerName(object, name) {
    intellisense.declareNavigationContainer(object, name, 'vs:GlyphGroupConstant');
}

const testContainer = callerName({
    foo: function(){ },
    bar: function(){ }
}, '');
";
        #endregion

        [TestMethod]
        [WorkItem(667920)]
        public void EnsureFunctionsAndFieldsOnlyAppearOnceInANamespace()
        {
            var scenario = DefineScenario(EnsureFunctionsAndFieldsOnlyAppearOnceInANamespace_Text);

            var structure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(

                    scenario.Global(
                        scenario.FunctionWithRegion("Object.prototype.clone()"),
                        scenario.Variable("SomeNamespace"),
                        scenario.Node(
                            scenario.ObjectWithRegion("SomeNamespace", regionStartingText: "var SomeNamespace"),
                            scenario.FunctionWithRegion("SomeNamespace.method1()", regionStartingText: "method1: ")
                        ),
                        scenario.Variable("OtherNamespace")
                    ),

                    scenario.RawClassWithMembers("Object",
                        scenario.FunctionWithRegion("clone()", regionStartingText: "Object.prototype.clone")
                    ),

                    scenario.Namespace("SomeNamespace",
                        scenario.FunctionWithRegion("method1()", regionStartingText: "method1: "),
                        scenario.Field("var1")
                    )

                );

            AssertAreStructurallyEqual(expectedStructure, structure);
            VerifyHierarchy(structure, expectedStructure.Heirarchy);
        }
        #region Test data
        const string EnsureFunctionsAndFieldsOnlyAppearOnceInANamespace_Text = @"
            Object.prototype.clone = function () {
                var newInstance = {};
                var that = this;
                Object.getOwnPropertyNames(that).forEach(function (name) {
                    newInstance[name] = that[name];
                });
                return newInstance;
            }

            var SomeNamespace = {
                method1: function () { },
                var1: 12
            };


            SomeNamespace.Nested = SomeNamespace.clone();

            var OtherNamespace = { Nested: SomeNamespace.clone() };";
        #endregion

        [TestMethod]
        [WorkItem(667920)]
        public void EnsureBestNamespaceIsGiven()
        {
            var scenario = DefineScenario(EnsureBestNamespaceIsGiven_Text);

            var structure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = (string)null, ItemName = "NS1", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "NS1", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkObjectLiteral },
                    new { Kind = AuthorStructureNodeKind.asnkRegion, Region = GetRegionForBalancedBraces(EnsureBestNamespaceIsGiven_Text, "var NS1") },
                    new { ContainerName = (string)null, ItemName = "NS1.f()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { Kind = AuthorStructureNodeKind.asnkRegion },
                    new { ContainerName = (string)null, ItemName = "_NS1", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = (string)null, ItemName = "NS2", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "NS2", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkObjectLiteral },
                    new { Kind = AuthorStructureNodeKind.asnkRegion, Region = GetRegionForBalancedBraces(EnsureBestNamespaceIsGiven_Text, "var NS2") },
                    new { ContainerName = (string)null, ItemName = "NS2.f()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { Kind = AuthorStructureNodeKind.asnkRegion },
                    new { ContainerName = (string)null, ItemName = "AN2", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = (string)null, ItemName = "NS3", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "NS3", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkObjectLiteral },
                    new { Kind = AuthorStructureNodeKind.asnkRegion, Region = GetRegionForBalancedBraces(EnsureBestNamespaceIsGiven_Text, "var NS3") },
                    new { ContainerName = (string)null, ItemName = "NS3.f()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { Kind = AuthorStructureNodeKind.asnkRegion },
                    new { ContainerName = (string)null, ItemName = "t", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "NS1", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    new { ContainerName = (string)null, ItemName = "f()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { Kind = AuthorStructureNodeKind.asnkRegion },
                    new { ContainerName = "AN2", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    new { ContainerName = (string)null, ItemName = "f()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { Kind = AuthorStructureNodeKind.asnkRegion },
                    new { Kind = AuthorStructureNodeKind.asnkRegion },
                    new { ContainerName = "NS3", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    new { ContainerName = (string)null, ItemName = "f()", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { Kind = AuthorStructureNodeKind.asnkRegion }
                );

            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string EnsureBestNamespaceIsGiven_Text = @"
            // Should prefer NS1 to _NS1 because _NS1 appears private.
            var NS1 = { f: function () { } };
            var _NS1 = { f: NS1.f };

            // Should prefer AN2 to NS2 because AN2 is lexically before NS2
            var NS2 = { f: function () { } };
            var AN2 = { f: NS2.f };

            // Should prefer NS3 to ANS3 because NS3 appears in the file as an identifier but ANS3 doesn't
            var NS3 = { f: function () { } };
            var t = 'AN' + '3';
            this[t] = { f: NS3.f };";
        #endregion

        [TestMethod]
        [WorkItem(669509)]
        public void StaticMethods()
        {
            var structure = StructureOf(StaticMethods_Text);
            var classKey = structure.Where(s => s.Kind == AuthorStructureNodeKind.asnkClass).Select(s => s.Key).First();
            // Ensure there is a method of the class called foo.
            structure.First(s => s.Container == classKey && s.ItemName == "foo()");
        }
        #region Test data
        const string StaticMethods_Text = @"
            var A = function () {};
            A.foo = function () {};
            A.prototype.bar = function (){};";
        #endregion

        [TestMethod]
        [WorkItem(702392)]
        public void ClassContainingOnlyStaticMethods()
        {
            var structure = StructureOf(ClassContainingOnlyStaticMethods_Text);
            var classKey = structure.Where(s => s.Kind == AuthorStructureNodeKind.asnkClass).Select(s => s.Key).First();
            // Ensure there is a method of the class called foo.
            structure.First(s => s.Container == classKey && s.ItemName == "foo()");
        }
        #region Test data
        const string ClassContainingOnlyStaticMethods_Text = @"
            var A = function () {};
            A.foo = function () {};";
        #endregion

        [TestMethod]
        [WorkItem(702392)]
        public void FunctionWithNonFunctionPropertyIsNotAClass()
        {
            var structure = StructureOf(FunctionWithNonFunctionPropertyIsNotAClass_Text);
            Assert.AreEqual(0, structure.Where(s => s.Kind == AuthorStructureNodeKind.asnkClass).Select(s => s.Key).Count(),
                "The structure should not contain a class");
            // Ensure there is a method called A.
            structure.First(s => s.ItemName == "A()");
        }
        #region Test data
        const string FunctionWithNonFunctionPropertyIsNotAClass_Text = @"
            var A = function () {};
            A.foo = 42;";
        #endregion

        [TestMethod]
        [WorkItem(635630)]
        public void ClassDefinitionJQueryCallback()
        {
            var structure = StructureOf(ClassDefinitionJQueryCallback_Text,
                "!!" + Paths.DomWebPath,
                JQueryTestFiles.jquery_1_7_2);
        }
        #region Test data
        const string ClassDefinitionJQueryCallback_Text = @"
var global = this;
$(function () {

  function Class() { this.field = 1; }
  Class.prototype.method = function () { };
   
  global.C1 = C1;
})();
";
        #endregion

        [TestMethod]
        public void FieldsInNamespaces()
        {
            var structure = StructureOf(@"
                var modules = {
                   foo: function () { },
                   bar: 'some value'
                };
            ");
            structure.First(s => s.ItemName == "bar");
        }

        [TestMethod]
        public void FunctionsInNamespaces()
        {
            var structure = StructureOf(@"
                var modules = {
                   foo: function () { },
                   bar: 'some value'
                };
            ");
            structure.First(s => s.ItemName == "foo()");
        }

        [TestMethod]
        [WorkItem(675609)]
        public void EnsureLetAndConstOnlyAppearsAtGlobalScope()
        {
            var structure = StructureOf(EnsureLetAndConstOnlyAppearAtGlobalScope_Text);

            var itemNames = structure.Where(s => !string.IsNullOrWhiteSpace(s.ItemName)).Select(s => s.ItemName);
            Func<string, bool> isGlobalName = name => name.StartsWith("glet", StringComparison.InvariantCulture) || name.StartsWith("gconst", StringComparison.InvariantCulture);

            // Verify that all 6 global lets and consts show up.
            Assert.AreEqual(12, itemNames.Where(isGlobalName).Count());

            // Verify that only global names appear.
            Assert.IsNull(itemNames.FirstOrDefault(name => name.Contains("const") && !isGlobalName(name)));
            Assert.IsNull(itemNames.FirstOrDefault(name => name.Contains("let") && !isGlobalName(name)));
        }
        #region Test data
        const string EnsureLetAndConstOnlyAppearAtGlobalScope_Text = @"
            let glet1 = 123;
            const gconst1 = 123;

            if (true) {
                let let1 = 123;
                const const1 = 123;
            }
            else {
                let let2 = 123;
                const const2 = 123;
            }
            
            let glet2 = 123;
            const gconst2 = 123;

            {
                let let3 = 123;
                const const3 = 123;
            }

            let glet3 = 123;
            const gconst3 = 123;

            switch (a) {
                case 'b':
                    let let4 = 123;
                    const const4 = 123;
                    break;
            }

            let glet4 = 123;
            const gconst4 = 123;

            for (let let5 = 1; let5 < 2; let5++)
                ;

            let glet5 = 123;
            const gconst5 = 123;

            for (let let6 in this)
                ;

            let glet6 = 123;
            const gconst7 = 123;";
        #endregion

        [TestMethod]
        [WorkItem(id: 670169)]
        [TestCategory("Structure")]
        public void When_a_function_is_assigned_it_should_have_a_region_starting_where_the_name_starts()
        {
            var scenario = DefineScenario(@"
                var obj = { a: { b: { c: {} } } };
                obj.a.b.c.func = function() { };
            ");

            const string FunctionName = "obj.a.b.c.func";

            var actualStructure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(

                    scenario.Global(
                        scenario.Variable("obj"),
                        scenario.FunctionWithRegion(FunctionName + "()", regionStartingText: FunctionName)
                    ),

                    scenario.Namespace("obj",
                        scenario.Namespace("a",
                            scenario.Namespace("b",
                                scenario.Namespace("c",
                                    scenario.FunctionWithRegion("func()", regionStartingText: FunctionName)
                                )
                            )
                        )
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);
            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);
            Func<string, AuthorStructureNode> findFunction = functionName =>
            {
                var function = actualStructure.SingleOrDefault(s =>
                    s.ItemName != null &&
                    s.Kind == AuthorStructureNodeKind.asnkFunction &&
                    s.ItemName.Contains(functionName));

                if (function.Kind == AuthorStructureNodeKind.asnkFunction)
                {
                    return function;
                }

                throw new AssertFailedException(
                    string.Format(
                        "Unable to find a function named '{0}' in the structure results: {1}",
                        functionName,
                        string.Join(", ", actualStructure.Select(s => new { s.Kind, s.ItemName }))));
            };

            AssertAreEqualOffsets(
                scenario.SourceText,
                expected: scenario.SourceText.IndexOf("function"),
                actual: findFunction(FunctionName).Region.Offset);
        }

        [TestMethod]
        public void GetCallerNameWithUnicodeCharacters()
        {
            var scenario = DefineScenario(GetCallerNameWithUnicodeCharacters_TextTemplate.Replace("REPLACE_HERE", new string('\u9988', 10)));

            var structure = scenario.GetStructure();

            var expectedStructure = scenario.Program(

                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = (string)null, ItemName = "define(name, obj)", Kind = AuthorStructureNodeKind.asnkFunction },
                    new { ContainerName = (string)null, ItemName = "X", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = (string)null, ItemName = "Y", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = "X", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    scenario.FunctionWithRegion("foo(a, b, c)", "foo: "),
                    new { ContainerName = "Y", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkNamespace },
                    scenario.FunctionWithRegion("bar(a, b, c)", "bar: ")

                );


            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string GetCallerNameWithUnicodeCharacters_TextTemplate = @"
function define(name, obj) {
    intellisense.declareNavigationContainer(obj, null, 'vs:GlyphGroupClass');
    return obj;
}

var X = define('REPLACE_HERE', {
    foo: function (a, b, c) { }
});

var Y = define('1234567890', {
    bar: function (a, b, c) { }
});
";
        #endregion

        [TestMethod]
        [WorkItem(702373)]
        public void ThisDotAssignmentOverridesPrototypeMember()
        {
            var scenario = DefineScenario(ThisDotAssignmentOverridesPrototypeMember_Text);

            var structure = scenario.GetStructure(WinJSTestFiles.latest_base);


            var expectedStructure =
                scenario.Program(
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal, Region = new { Offset = 0 } },
                    new { ContainerName = (string)null, ItemName = "Foo", Kind = AuthorStructureNodeKind.asnkVariable, Region = new { Offset = 0 } },
                    new { ItemName = "Foo()", Kind = AuthorStructureNodeKind.asnkFunction, Region = new { Offset = ThisDotAssignmentOverridesPrototypeMember_Text.IndexOf("function Foo()") } },
                    new { ContainerName = (string)null, ItemName = "c()", Kind = AuthorStructureNodeKind.asnkFunction, Region = new { Offset = ThisDotAssignmentOverridesPrototypeMember_Text.IndexOf("function c()") } },
                    new { Kind = AuthorStructureNodeKind.asnkRegion },
                    new { ContainerName = (string)null, ItemName = "d()", Kind = AuthorStructureNodeKind.asnkFunction, Region = new { Offset = ThisDotAssignmentOverridesPrototypeMember_Text.IndexOf("function d()") } },
                    new { Kind = AuthorStructureNodeKind.asnkRegion },
                    new { ContainerName = "Foo", ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkClass, Region = new { Offset = ThisDotAssignmentOverridesPrototypeMember_Text.IndexOf("function Foo()") } },
                    new { ContainerName = (string)null, ItemName = "a", Kind = AuthorStructureNodeKind.asnkField, Region = new { Offset = ThisDotAssignmentOverridesPrototypeMember_Text.IndexOf("this.a") } },
                    new { ContainerName = (string)null, ItemName = "b", Kind = AuthorStructureNodeKind.asnkField, Region = new { Offset = ThisDotAssignmentOverridesPrototypeMember_Text.IndexOf("this.b") } },
                    scenario.FunctionWithRegion("c()", "this.c = "),
                    scenario.FunctionWithRegion("d()", "this.d = "),
                    new { ContainerName = (string)null, ItemName = "Foo()", Kind = AuthorStructureNodeKind.asnkFunction, Region = new { Offset = ThisDotAssignmentOverridesPrototypeMember_Text.IndexOf("function Foo()") } }
                );


            AssertAreStructurallyEqual(expectedStructure, structure);
        }

        #region Test data
        const string ThisDotAssignmentOverridesPrototypeMember_Text = @"var Foo = WinJS.Class.define(function Foo() {
        this.a = "";
        this.b = 42;
        this.c = function c() { return 'c'; };
        this.d = function d() { return 0; };
    },
    {
        a: true,
        b: function () { return 'proto_b'; },
        c: 17,
        d: function () { return false; }
    });
";
        #endregion

        [TestMethod]
        [WorkItem(702233)]
        public void DetectingObjectsAsContainers()
        {
            var scenario = DefineScenario(DetectingObjectsAsContainers_Text);

            var actualStructure = scenario.GetStructure();

            var expectedStructure =
                scenario.Program(

                    scenario.Global(
                        scenario.Variable("NS1"),
                        scenario.Node(
                            scenario.ObjectWithRegion("NS1", regionStartingText: "var NS1"),
                            scenario.FunctionWithRegion("NS1.a()", regionStartingText: "a: ")
                        ),
                        scenario.Variable("NS2", isArray: true),
                        scenario.FunctionWithRegion("NS2.b()"),
                        scenario.Function("Class1()"),
                        scenario.FunctionWithRegion("Class1.prototype.c()"),
                        scenario.Variable("classInstance", useSemicolonRegion: true),
                        scenario.FunctionWithRegion("classInstance.d()")
                    ),
                    scenario.ClassWithMembers("Class1",
                        scenario.FunctionWithRegion("c()", regionStartingText: "Class1.prototype.c"),
                        scenario.Function("Class1()")
                    ),
                    scenario.Namespace("NS1",
                        scenario.FunctionWithRegion("a()", regionStartingText: "a: ")
                    ),
                    scenario.Namespace("NS2",
                        scenario.FunctionWithRegion("b()", regionStartingText: "NS2.b")
                    )
                );

            AssertAreStructurallyEqual(expectedStructure, actualStructure);
            VerifyHierarchy(actualStructure, expectedStructure.Heirarchy);
        }
        #region Test data
        const string DetectingObjectsAsContainers_Text = @"
            // Should recognize object with children as container
            var NS1 = { a: function () {} };

            // Should recognize array with children as container
            var NS2 = [];
            NS2.b = function () {};            

            // Should recognize class as container
            function Class1() { }
            Class1.prototype.c = function () {}

            // Should not recognize class instance as container
            var classInstance = new Class1();
            classInstance.d = function () {}";
        #endregion

        [TestMethod]
        [WorkItem(702233)]
        public void ClassInstancesNotConsideredContainers_WinJS()
        {
            var structure = StructureOf(ClassInstancesNotConsideredContainers_WinJS_Text, WinJSTestFiles.latest_base, WinJSTestFiles.latest_ui);
            AssertAreStructurallyEqual(
                new object[] { 
                    new { ContainerName = (string)null, ItemName = (string)null, Kind = AuthorStructureNodeKind.asnkGlobal },
                    new { ContainerName = (string)null, ItemName = "list", Kind = AuthorStructureNodeKind.asnkVariable },
                    new { ContainerName = (string)null, ItemName = "generateSampleData()", Kind = AuthorStructureNodeKind.asnkFunction }
                },
                structure);
        }
        #region Test data
        const string ClassInstancesNotConsideredContainers_WinJS_Text = @"
            var list = new WinJS.Binding.List();
            
            generateSampleData().forEach(function (item) {
                list.push(item);
            });
            
            function generateSampleData() {
                var itemContent = ""<p>Curabitur class aliquam vestibulum nam curae maecenas sed integer cras phasellus suspendisse quisque donec dis praesent accumsan bibendum pellentesque condimentum adipiscing etiam consequat vivamus dictumst aliquam duis convallis scelerisque est parturient ullamcorper aliquet fusce suspendisse nunc hac eleifend amet blandit facilisi condimentum commodo scelerisque faucibus aenean ullamcorper ante mauris dignissim consectetuer nullam lorem vestibulum habitant conubia elementum pellentesque morbi facilisis arcu sollicitudin diam cubilia aptent vestibulum auctor eget dapibus pellentesque inceptos leo egestas interdum nulla consectetuer suspendisse adipiscing pellentesque proin lobortis sollicitudin augue elit mus congue fermentum parturient fringilla euismod feugiat</p><p>Curabitur class aliquam vestibulum nam curae maecenas sed integer cras phasellus suspendisse quisque donec dis praesent accumsan bibendum pellentesque condimentum adipiscing etiam consequat vivamus dictumst aliquam duis convallis scelerisque est parturient ullamcorper aliquet fusce suspendisse nunc hac eleifend amet blandit facilisi condimentum commodo scelerisque faucibus aenean ullamcorper ante mauris dignissim consectetuer nullam lorem vestibulum habitant conubia elementum pellentesque morbi facilisis arcu sollicitudin diam cubilia aptent vestibulum auctor eget dapibus pellentesque inceptos leo egestas interdum nulla consectetuer suspendisse adipiscing pellentesque proin lobortis sollicitudin augue elit mus congue fermentum parturient fringilla euismod feugiat</p><p>Curabitur class aliquam vestibulum nam curae maecenas sed integer cras phasellus suspendisse quisque donec dis praesent accumsan bibendum pellentesque condimentum adipiscing etiam consequat vivamus dictumst aliquam duis convallis scelerisque est parturient ullamcorper aliquet fusce suspendisse nunc hac eleifend amet blandit facilisi condimentum commodo scelerisque faucibus aenean ullamcorper ante mauris dignissim consectetuer nullam lorem vestibulum habitant conubia elementum pellentesque morbi facilisis arcu sollicitudin diam cubilia aptent vestibulum auctor eget dapibus pellentesque inceptos leo egestas interdum nulla consectetuer suspendisse adipiscing pellentesque proin lobortis sollicitudin augue elit mus congue fermentum parturient fringilla euismod feugiat</p><p>Curabitur class aliquam vestibulum nam curae maecenas sed integer cras phasellus suspendisse quisque donec dis praesent accumsan bibendum pellentesque condimentum adipiscing etiam consequat vivamus dictumst aliquam duis convallis scelerisque est parturient ullamcorper aliquet fusce suspendisse nunc hac eleifend amet blandit facilisi condimentum commodo scelerisque faucibus aenean ullamcorper ante mauris dignissim consectetuer nullam lorem vestibulum habitant conubia elementum pellentesque morbi facilisis arcu sollicitudin diam cubilia aptent vestibulum auctor eget dapibus pellentesque inceptos leo egestas interdum nulla consectetuer suspendisse adipiscing pellentesque proin lobortis sollicitudin augue elit mus congue fermentum parturient fringilla euismod feugiat</p><p>Curabitur class aliquam vestibulum nam curae maecenas sed integer cras phasellus suspendisse quisque donec dis praesent accumsan bibendum pellentesque condimentum adipiscing etiam consequat vivamus dictumst aliquam duis convallis scelerisque est parturient ullamcorper aliquet fusce suspendisse nunc hac eleifend amet blandit facilisi condimentum commodo scelerisque faucibus aenean ullamcorper ante mauris dignissim consectetuer nullam lorem vestibulum habitant conubia elementum pellentesque morbi facilisis arcu sollicitudin diam cubilia aptent vestibulum auctor eget dapibus pellentesque inceptos leo egestas interdum nulla consectetuer suspendisse adipiscing pellentesque proin lobortis sollicitudin augue elit mus congue fermentum parturient fringilla euismod feugiat</p><p>Curabitur class aliquam vestibulum nam curae maecenas sed integer cras phasellus suspendisse quisque donec dis praesent accumsan bibendum pellentesque condimentum adipiscing etiam consequat vivamus dictumst aliquam duis convallis scelerisque est parturient ullamcorper aliquet fusce suspendisse nunc hac eleifend amet blandit facilisi condimentum commodo scelerisque faucibus aenean ullamcorper ante mauris dignissim consectetuer nullam lorem vestibulum habitant conubia elementum pellentesque morbi facilisis arcu sollicitudin diam cubilia aptent vestibulum auctor eget dapibus pellentesque inceptos leo egestas interdum nulla consectetuer suspendisse adipiscing pellentesque proin lobortis sollicitudin augue elit mus congue fermentum parturient fringilla euismod feugiat</p><p>Curabitur class aliquam vestibulum nam curae maecenas sed integer cras phasellus suspendisse quisque donec dis praesent accumsan bibendum pellentesque condimentum adipiscing etiam consequat vivamus dictumst aliquam duis convallis scelerisque est parturient ullamcorper aliquet fusce suspendisse nunc hac eleifend amet blandit facilisi condimentum commodo scelerisque faucibus aenean ullamcorper ante mauris dignissim consectetuer nullam lorem vestibulum habitant conubia elementum pellentesque morbi facilisis arcu sollicitudin diam cubilia aptent vestibulum auctor eget dapibus pellentesque inceptos leo egestas interdum nulla consectetuer suspendisse adipiscing pellentesque proin lobortis sollicitudin augue elit mus congue fermentum parturient fringilla euismod feugiat"";
                var itemDescription = ""Item Description: Pellentesque porta mauris quis interdum vehicula urna sapien ultrices velit nec venenatis dui odio in augue cras posuere enim a cursus convallis neque turpis malesuada erat ut adipiscing neque tortor ac erat"";
                var groupDescription = ""Group Description: Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus tempor scelerisque lorem in vehicula. Aliquam tincidunt, lacus ut sagittis tristique, turpis massa volutpat augue, eu rutrum ligula ante a ante"";
            
                var darkGray = ""data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsQAAA7EAZUrDhsAAAANSURBVBhXY3B0cPoPAANMAcOba1BlAAAAAElFTkSuQmCC"";
                var lightGray = ""data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsQAAA7EAZUrDhsAAAANSURBVBhXY7h4+cp/AAhpA3h+ANDKAAAAAElFTkSuQmCC"";
                var mediumGray = ""data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsQAAA7EAZUrDhsAAAANSURBVBhXY5g8dcZ/AAY/AsAlWFQ+AAAAAElFTkSuQmCC"";
            
                var sampleGroups = [
                    { key: ""group1"", title: ""Group Title: 1"", subtitle: ""Group Subtitle: 1"", backgroundImage: darkGray, description: groupDescription },
                    { key: ""group2"", title: ""Group Title: 2"", subtitle: ""Group Subtitle: 2"", backgroundImage: lightGray, description: groupDescription },
                ];
            
                var sampleItems = [
                    { group: sampleGroups[0], title: ""Item Title: 1"", subtitle: ""Item Subtitle: 1"", description: itemDescription, content: itemContent, backgroundImage: lightGray },
                    { group: sampleGroups[0], title: ""Item Title: 2"", subtitle: ""Item Subtitle: 2"", description: itemDescription, content: itemContent, backgroundImage: darkGray },
                    { group: sampleGroups[1], title: ""Item Title: 3"", subtitle: ""Item Subtitle: 3"", description: itemDescription, content: itemContent, backgroundImage: mediumGray }
                ];
            
                return sampleItems;
            }";
        #endregion

        [TestMethod]
        [Ignore] // TODO: Issue getting parseable function info for class method function returned from dynamic analysis
        [WorkItem(923320)]
        [TestCategory("Classes")]
        public void StructureOfES6ClassExpressionAssignedToVar()
        {
            // Placeholder test for future work to support GetStructure for classes
            var scenario = DefineScenario(StructureOfES6ClassExpressionAssignedToVar_Text);

            var structure = scenario.GetStructure();


            var expectedStructure =
                scenario.Program(
                );


            AssertAreStructurallyEqual(expectedStructure, structure);
        }
        #region Test data
        const string StructureOfES6ClassExpressionAssignedToVar_Text = @"
var Foo = class {
    constructor() {
        this.a = "";
        this.b = 42;
    }
    c() { return 'c'; }
    d() { return 0; }
    get e() { return true; }
    static s_a() { return 's_a'; }
    static get s_b() { return 's_b'; }
}
";
        #endregion

        void AssertAreEqualOffsets(
            string testData,
            int expected,
            int actual)
        {
            Assert.AreEqual(
                expected, actual,
                "Epected offset at: {0} ({1}...), but got offset: {2} ({3}...)",
                expected, testData.Substring(expected, length: 20),
                actual, testData.Substring(actual, length: 20));
        }

        #region Helpers

        sealed class StructureTestScenario
        {
            private readonly string sourceText;
            private readonly StructureTests testHost;

            public string SourceText { get { return sourceText; } }

            public StructureTestScenario(string sourceText, StructureTests testHost)
            {
                this.sourceText = sourceText;
                this.testHost = testHost;
            }

            public IEnumerable<AuthorStructureNode> GetStructure(params string[] contextFiles)
            {
                return testHost.StructureOf(sourceText, contextFiles);
            }
            
            public IStructureExpectation Global(params object[] children)
            {
                return new StructureExpectation(
                    new { Kind = AuthorStructureNodeKind.asnkGlobal, HasChildren = children != null && children.Length > 0 },
                    children);
            }

            public IStructureExpectation FunctionWithRegion(string name, string regionStartingText = null)
            {
                regionStartingText = regionStartingText ?? name.Substring(0, name.IndexOf('('));
                return Node(
                    Function(name, regionStartingText, adjustToFunctionStart: true),
                    Region(regionStartingText)
                );
            }

            public object Function(string name, string regionStartingText = null, int offsetAdjustment = 0, bool adjustToFunctionStart = false, bool isIIFE = false)
            {
                regionStartingText = regionStartingText ??  (isIIFE ? "(" + name : "function " + name);

                if (offsetAdjustment == 0 && adjustToFunctionStart)
                {
                    int regionStart = sourceText.IndexOf(regionStartingText);
                    if (regionStart >= 0)
                    {
                        int functionStart = sourceText.IndexOf("function", regionStart);
                        if (functionStart >= 0)
                        {
                            offsetAdjustment = functionStart - regionStart;
                        }
                    }
                }

                object region;

                if (isIIFE)
                {
                    region = GetIIFE(sourceText, regionStartingText);
                }
                else
                {
                    region = GetRegionForBalancedBraces(
                                sourceText,
                                startingText: regionStartingText,
                                offsetAdjustment: offsetAdjustment);
                }

                return new
                {
                    Kind = AuthorStructureNodeKind.asnkFunction,
                    ItemName = name,
                    Region = region
                };
            }

            public object Variable(string varName, string regionStartingText = null, bool isArray = false, bool useSemicolonRegion = false)
            {
                regionStartingText = regionStartingText ?? "var " + varName;

                if (isArray)
                {
                    return new
                    {
                        Kind = AuthorStructureNodeKind.asnkVariable,
                        ItemName = varName,
                        Region = GetRegionForBalancedBraces(sourceText, regionStartingText, openBrace: '[', closeBrace: ']')
                    };
                }

                Func<object> getSemiRegion = () =>
                {
                    int regionStart = sourceText.IndexOf(regionStartingText);
                    int end = sourceText.IndexOf(';', startIndex: regionStart);
                    return new { Offset = regionStart, Length = end - regionStart }; 
                };

                return new
                {
                    Kind = AuthorStructureNodeKind.asnkVariable,
                    ItemName = varName,
                    // ajust length by 1 for semi colon
                    Region = useSemicolonRegion ? getSemiRegion() : GetRegionForBalancedBraces(sourceText, regionStartingText),
                    ContainerName = (string)null,
                };
            }

            public IStructureExpectation RawClassWithMembers(string className, params object[] members)
            {
                return Node(
                    new {
                        Kind = AuthorStructureNodeKind.asnkClass,
                        ContainerName = className, 
                        HasChildren = members != null && members.Length > 0,
                        ItemName = (string)null,
                    },
                    members
                );
            }

            public IStructureExpectation ClassWithMembers(string className, string regionStartingText, params object[] members)
            {
                return Node(
                    Class(className, regionStartingText),
                    members
                );
            }

            public IStructureExpectation ClassWithMembers(string className, params object[] members)
            {
                return Node(
                    Class(className, hasChildren: members != null && members.Length > 0),
                    members
                );
            }

            public object Class(string className, string regionStartingText = null, bool adjustToFunctionStart = false, bool hasChildren = true)
            {
                int offsetAdjustment = 0;
                if (adjustToFunctionStart)
                {
                    int regionStart = sourceText.IndexOf(regionStartingText);
                    if (regionStart >= 0)
                    {
                        int functionStart = sourceText.IndexOf("function", regionStart);
                        if (functionStart >= 0)
                        {
                            offsetAdjustment = functionStart - regionStart;
                        }
                    }
                }

                return new
                {
                    Kind = AuthorStructureNodeKind.asnkClass,
                    ContainerName = className,
                    ItemName = (string)null,
                    // ajust length by 1 for semi colon
                    Region = GetRegionForBalancedBraces(sourceText, regionStartingText ?? "function " + className, offsetAdjustment: offsetAdjustment),
                    HasChildren = hasChildren,
                };
            }

            public IStructureExpectation ObjectWithRegion(string name, string regionStartingText = null, int? offsetAdjustment = null)
            {
                regionStartingText = regionStartingText ?? name;

                if (!offsetAdjustment.HasValue)
                {
                    int regionStart = sourceText.IndexOf(regionStartingText);
                    if (regionStart >= 0)
                    {
                        int braceStart = sourceText.IndexOf('{', startIndex: regionStart);
                        if (braceStart >= 0)
                        {
                            offsetAdjustment = braceStart - regionStart;
                        }
                    }
                }

                return Node(
                    Object(name, regionStartingText: regionStartingText, offsetAdjustment: offsetAdjustment.GetValueOrDefault(defaultValue: 0)),
                    Region(regionStartingText)
                );
            }

            public object Object(string name, string regionStartingText = null, int offsetAdjustment = 0)
            {
                return new
                {
                    Kind = AuthorStructureNodeKind.asnkObjectLiteral,
                    ContainerName = name,
                    // ajust length by 1 for semi colon
                    Region = GetRegionForBalancedBraces(sourceText, regionStartingText ?? name, offsetAdjustment)
                };
            }

            public object Region(string regionStartingText, int offsetAdjustment = 0)
            {
                return new
                {
                    Kind = AuthorStructureNodeKind.asnkRegion,
                    ContainerName = (string)null,
                    ItemName = (string)null,
                    // ajust length by 1 for semi colon
                    Region = GetRegionForBalancedBraces(sourceText, regionStartingText, offsetAdjustment)
                };
            }

            public IStructureExpectation Namespace(string name, params object[] children)
            {
                return new StructureExpectation(
                    new
                    {
                        Kind = AuthorStructureNodeKind.asnkNamespace,
                        ContainerName = name
                    },
                    children);
            }

            internal object Field(string name)
            {
                return new { ItemName = name, Kind = AuthorStructureNodeKind.asnkField };
            }

            public IStructureExpectation Node(object node, params object[] children)
            {
                return new StructureExpectation(node, children);
            }

            public IStructureExpectation Program(params object[] containers)
            {
                return new StructureContainers(containers);
            }

            public interface IStructureExpectation : IEnumerable<Object>
            {
                object[] ExpectedStructure { get; }
                int[] Heirarchy { get; }
            }

            private interface IStrucureExpecetationWithIndex : IStructureExpectation
            {
                IEnumerable<int> GetExpectedHeirarchy(int index = 0);
            }

            public sealed class StructureContainers : IStrucureExpecetationWithIndex
            {
                private readonly object[] containers;

                public StructureContainers(object[] containers)
                {
                    this.containers = containers;
                }

                private IEnumerable<object> GetExpectedStructure()
                {
                    foreach (var container in containers)
                    {
                        var childExpectation = container as IStructureExpectation;
                        if (childExpectation != null)
                        {
                            foreach (var childNode in childExpectation.ExpectedStructure)
                            {
                                yield return childNode;
                            }
                        }
                        else
                        {
                            yield return container;
                        }
                    }

                }

                IEnumerable<int> IStrucureExpecetationWithIndex.GetExpectedHeirarchy(int index)
                {
                    foreach (var container in containers)
                    {
                        var childExpectation = container as IStrucureExpecetationWithIndex;
                        if (childExpectation != null)
                        {
                            foreach (var childHeir in childExpectation.GetExpectedHeirarchy(index))
                            {
                                yield return childHeir;
                                index++;
                            }
                        }
                        else
                        {
                            yield return 0;
                            index++;
                        }
                    }
                }

                public object[] ExpectedStructure
                {
                    get { return GetExpectedStructure().ToArray(); }
                }

                public int[] Heirarchy
                {
                    get { return ((IStrucureExpecetationWithIndex)this).GetExpectedHeirarchy().ToArray(); }
                }

                public IEnumerator<object> GetEnumerator()
                {
                    return GetExpectedStructure().GetEnumerator();
                }

                System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
                {
                    return GetExpectedStructure().GetEnumerator();
                }
            }

            public sealed class StructureExpectation : IStrucureExpecetationWithIndex
            {
                private readonly object node;
                private readonly object[] children;

                public StructureExpectation(object node, params object[] children)
                {
                    this.node = node;
                    this.children = children ?? new object[0];
                }

                private IEnumerable<object> GetExpectedStructure()
                {
                    if (node is IStructureExpectation)
                    {
                        var nodeEpect = (IStructureExpectation)node;
                        foreach (var item in nodeEpect)
                        {
                            yield return item;
                        }
                    }
                    else
                    {
                        yield return node;
                    }

                    foreach (var child in children)
                    {
                        var childExpectation = child as StructureExpectation;
                        if (childExpectation != null)
                        {
                            foreach (var childNode in childExpectation.ExpectedStructure)
                            {
                                yield return childNode;
                            }
                        }
                        else
                        {
                            yield return child;
                        }
                    }

                }

                IEnumerable<int> IStrucureExpecetationWithIndex.GetExpectedHeirarchy(int index)
                {
                    int currentContainerIndex = index;

                    // This is for this.node, which is the container of it's children, 
                    // and thus has no parent -- unless it's called in a recursive call
                    if (node is IStrucureExpecetationWithIndex)
                    {
                        var nodeEpect = (IStrucureExpecetationWithIndex)node;
                        foreach (var item in nodeEpect.GetExpectedHeirarchy(index))
                        {
                            yield return item;
                            index++;
                        }
                        index--;
                    }
                    else
                    {
                        yield return 0;
                    }

                    foreach (var child in children)
                    {
                        var childExpectation = child as IStrucureExpecetationWithIndex;
                        if (childExpectation != null)
                        {
                            int childContainersAdjustmentToCurrentIndex = currentContainerIndex;
                            foreach (var childHeir in childExpectation.GetExpectedHeirarchy(index))
                            {
                                yield return childHeir + childContainersAdjustmentToCurrentIndex + 1;
                                // after we've offset the container all it's children have the correct reference
                                // because they're calculated off of their container's index
                                childContainersAdjustmentToCurrentIndex = 0;
                                index++;
                            }
                        }
                        else
                        {
                            yield return currentContainerIndex + 1;
                            index++;
                        }
                    }
                }

                public object[] ExpectedStructure { get { return GetExpectedStructure().ToArray(); } }
                public int[] Heirarchy { get { return ((IStrucureExpecetationWithIndex)this).GetExpectedHeirarchy().ToArray(); } }

                public IEnumerator<object> GetEnumerator()
                {
                    return GetExpectedStructure().GetEnumerator();
                }

                System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
                {
                    return GetExpectedStructure().GetEnumerator();
                }
            }
        }

        private StructureTestScenario DefineScenario(string sourceText)
        {
            return new StructureTestScenario(sourceText, this);
        }

        private static object GetRegionForBalancedBraces(
            string text,
            string startingText,
            int offsetAdjustment = 0,
            int lengthAdjustment = 0,
            char openBrace = '{',
            char closeBrace = '}')
        {
            var offsetAndLength = GetOffsetAndLengthForBalancedBraces(text, startingText, offsetAdjustment, lengthAdjustment, openBrace, closeBrace);
            return new { Offset = offsetAndLength.Item1, Length = offsetAndLength.Item2 };
        }

        private static object GetIIFE(string text, string startingText)
        {
            var funcOffsetAndLength = GetOffsetAndLengthForBalancedBraces(text, startingText, openBrace: '(', closeBrace: ')');
            var call = text.Substring(funcOffsetAndLength.Item1 + funcOffsetAndLength.Item2);
            var callOffsetAndLength = GetOffsetAndLengthForBalancedBraces(call, call, openBrace: '(', closeBrace: ')');
            return new { Offset = funcOffsetAndLength.Item1, Length = callOffsetAndLength.Item2 + funcOffsetAndLength.Item2 };
        }

        private static Tuple<int, int> GetOffsetAndLengthForBalancedBraces(
            string text, 
            string startingText, 
            int offsetAdjustment = 0, 
            int lengthAdjustment = 0, 
            char openBrace = '{', 
            char closeBrace = '}')
        {
            int offset = text.IndexOf(startingText);
            Assert.AreNotEqual(-1, offset, "Starting text '{0}' not found in text: {1}", startingText, text);
            Assert.AreEqual(-1, text.IndexOf(startingText, startIndex: offset + 1), "Same starting text found twice, be more specific in selection");

            bool hasEnteredBraces = false;
            int braceCount = 0;
            char[] braces = { openBrace, closeBrace };

            if (offsetAdjustment > 0)
            {
                offset += offsetAdjustment;
            }

            int endOffset = offset;

            while (braceCount > 0 || !hasEnteredBraces)
            {
                int braceIndex = text.IndexOfAny(braces, startIndex: endOffset);
                if (braceIndex < 0)
                {
                    Assert.Fail("Unbalanced braces, expected {0}",
                        !hasEnteredBraces ?
                            "at least one opening brace" :
                            braceCount + " closing braces.");
                }

                

                if (text[braceIndex] == openBrace)
                {
                    hasEnteredBraces = true;
                    braceCount++;
                }
                else if (text[braceIndex] == closeBrace)
                {
                    braceCount--;
                }

                endOffset = braceIndex + 1;
            }

            

            return Tuple.Create(offset, endOffset - offset + lengthAdjustment);
        }

        private IEnumerable<AuthorStructureNode> StructureOf(string primaryFile, params string[] contextFiles)
        {
            var context = _session.OpenContext(_session.FileFromText(primaryFile), contextFiles.Select(t => _session.FileFromText(t)).ToArray());
            return context.GetStructure().GetAllNodes().ToEnumerable();
        }

        private void VerifyHierarchy(IEnumerable<AuthorStructureNode> nodesEnum, params int[] containers)
        {
            var nodes = nodesEnum.ToArray();
            var actualHeirarchy =
                nodes.Select(n =>
                    n.Container == 0 ?
                        0 :
                        1 + nodes.Select((node, ind) => new { node, ind })
                                .Where(s => s.node.Key == n.Container)
                                .Select(s => s.ind).Single())
                .ToArray();
            foreach (var relationship in
                nodes.Zip(
                    containers.Select((container, index) => new { container, index }),
                    (node, pair) => new { Node = node, Container = pair.container, ContainerIndex = pair.index + 1 }))
            {
                bool isGlobalContainer =
                        relationship.Container == 0 &&
                        relationship.Node.Container == 0;
                bool isNonGlobalAndSameContainer =
                        relationship.Container > 0 &&
                        nodes[relationship.Container - 1].Key == relationship.Node.Container;

                Assert.IsTrue(
                    isGlobalContainer ||
                    isNonGlobalAndSameContainer,
                    "The heirarchy verification failed on item at index {0} under the following conditions: {1}",
                    relationship.ContainerIndex,
                    new
                    {
                        ExpectedContainerIndex = relationship.Container,
                        ActualContainerIndex = nodes.Select((n, i) => new { n, i }).Where(n => n.n.Key == relationship.Node.Container).Select(n => n.i + 1).DefaultIfEmpty(-1).Single(),

                        ExpectedContainerId = relationship.Container <= 0 ? (object)null : nodes[relationship.Container - 1].Key,
                        ExpectedContainerName = relationship.Container <= 0 ? (object)null : nodes[relationship.Container - 1].ContainerName,
                        ExpectedContainerItemName = relationship.Container <= 0 ? (object)null : nodes[relationship.Container - 1].ItemName,

                        ActualContainerId = relationship.Node.Container,
                        ActualNodeContainerName = nodes.SingleOrDefault(n => n.Key == relationship.Node.Container).ContainerName,
                        ActualNodeContainerItemName = nodes.SingleOrDefault(n => n.Key == relationship.Node.Container).ItemName,

                        ActualHeirarchy = "[" + string.Join(",", actualHeirarchy) + "]",
                        ExpectedHeirarchy = "[" + string.Join(",", containers) + "]"
                    });
            }
        }

        private static string NodeText(AuthorStructureNode node, string text)
        {
            return text.Substring(node.Region.Offset, node.Region.Length);
        }

        private void VerifyRanges(IEnumerable<AuthorStructureNode> nodes, string text)
        {
            foreach (var node in nodes)
            {
                switch (node.Kind)
                {
                    case AuthorStructureNodeKind.asnkGlobal:
                        Assert.AreEqual(0, node.Region.Offset);
                        Assert.AreEqual(text.Length, node.Region.Length);
                        break;
                    case AuthorStructureNodeKind.asnkFunction:
                        {
                            var nodeText = NodeText(node, text);
                            if (nodeText.StartsWith("("))
                            {
                                // IIFE
                                Assert.IsTrue(nodeText.StartsWith("(function"));
                                Assert.IsTrue(nodeText.EndsWith(")"));
                            }
                            else
                            {
                                // Normal function
                                var startsWithFunction = nodeText.StartsWith("function");
                                var startsWithNameOfFunction = nodeText.StartsWith(node.ItemName.TrimEnd('(', ')').Trim());
                                Assert.IsTrue(startsWithFunction || startsWithNameOfFunction);
                                Assert.IsTrue(nodeText.EndsWith("}"));
                            }
                        }
                        break;
                    case AuthorStructureNodeKind.asnkField:
                        {
                            var nodeText = NodeText(node, text);
                            Assert.IsTrue(nodeText.Contains(node.ItemName));
                        }
                        break;
                }
            }
        }
        #endregion
    }
}