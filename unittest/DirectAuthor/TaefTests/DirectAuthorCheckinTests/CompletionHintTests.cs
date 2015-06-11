namespace DirectAuthorCheckinTests
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using Microsoft.BPT.Tests.DirectAuthor;
    using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    // When adding a test method to this class, include the same test method
    // to the derived class, CompletionHintWithExtensions.
    public class CompletionHintTests : DirectAuthorTest
    {
        protected const string extension = @"
                    intellisense.addEventListener('statementcompletion',  function(e) { 
                            intellisense.logMessage('base extension: completionHandler: ' + JSON.stringify(e) + '\n');
                    });

                     intellisense.addEventListener('statementcompletionhint',  
                        function (e) {
                            intellisense.logMessage('base extension: completionHintHandler: ' + JSON.stringify(e) + '\n');
                        }
                    );";

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
        [WorkItem(414972)]
        public void PropertyDescriptorDetection()
        {
            // In those cases object literal should not be considered a property descriptor
            // - It has properties that are not valid for a property descriptor
            // - It has no flag properties (configurable / enumerable / writable)
            TestCompletionHint(@"
                var x = {
                    /// <field>desc</value>
                    value: 0
                };
                x.|;

                x = {
                    foo: 0,                   
                    /// <field>desc</value>
                    value: 0
                };
                x.|;

                x = {
                    foo: 0,
                    configurable: false,                   
                    /// <field>desc</value>
                    value: 0
                };
                x.|;

                x = {
                    foo: 0,
                    get: false,                   
                    /// <field>desc</value>
                    value: 0
                };
                x.|;
            ",
             "value",
            hint =>
            {
                hint.Description.Expect("desc");
            });
            TestCompletionHint(@"
                var x = {
                    /// <field>desc</value>
                    get: 0
                };
                x.|;
            ",
             "get",
            hint =>
            {
                hint.Description.Expect("desc");
            });
            TestCompletionHint(@"
                var x = {
                    /// <field>desc</value>
                    set: 0
                };
                x.|;
            ",
             "set",
            hint =>
            {
                hint.Description.Expect("desc");
            });


            // In those cases object literal should be considered a property descriptor
            TestCompletionHint(@"
                var x = {
                    /// <field>desc</value>
                    value: 0,
                    configurable: true
                };
                x.|;

                x = {
                    /// <field>desc</value>
                    value: 0,
                    writable: true
                };
                x.|;

                x = {
                    /// <field>desc</value>
                    value: 0,
                    enumerable: true
                };
                x.|;

                x = {
                    /// <field>desc</value>
                    value: 0,
                    enumerable: true,
                    writable: true
                };
                x.|;

                x = {
                    /// <field>desc</value>
                    value: 0,
                    enumerable: true,
                    writable: true,
                    configurable: true
                };
                x.|;

                x = {
                    /// <field>desc</value>
                    value: 0,
                    get: function() {},
                    enumerable: true,
                    writable: true,
                    configurable: true
                };
                x.|;
            ",
             "value",
            hint =>
            {
                hint.Description.Expect(null);
            });
        }

        [TestMethod]
        public void EmptyLineInCommentBlock()
        {
            TestCompletionHint(@"
                function f1(a) {
                    /// <param name='a'>text</param>

                    /// <summary>text</summary>
                }
                f1|;
            }",
            "f1",
            hint =>
            {
                hint.Type.Expect(AuthorType.atFunction);
                hint.GetFunctionHelp().GetSignatures().ToEnumerable().Single().Description.Expect("text");
            });
        }

        [TestMethod]
        public void PropertyReturningFunction()
        {
            TestCompletionHint(
              @"
                var x = {};
                Object.defineProperty(x, 'funcProp', { 
                    get: function() { 
                        return function() {
                            /// <signature locid='locid'>
                            /// <summary>desc</summary>
                            /// </summary>
                        }; 
                    }, enumerable: true, configurable: true });

                x.funcProp|;
                ",
                "funcProp", hint =>
                {
                    hint.Type.Expect(AuthorType.atFunction);
                    var funcHelp = hint.GetFunctionHelp();
                    var signature = funcHelp.GetSignatures().ToEnumerable().Single();
                    signature.Description.Expect("desc");
                });
        }

        [TestMethod]
        public void PropertyValue()
        {
            var helpers = @"
                    function failWithOOM() {
                        intellisense.logMessage('failWithOOM');
                        var s = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
                        for(var x=0; x<100; x++) {
                            s = (s + s).toLowerCase();
                        }
                    }
                    function failRecursion() {
                        intellisense.logMessage('failRecursion');
                        function f(x) {
                            f(x+1);
                        }
                        f(1);
                    }
                ";

            // Verify long operation in getter before statementcompletion event
            PerformRequests(
              @"
                intellisense.addEventListener('statementcompletion',  function(e) {
                    e.items.forEach(function(item) {
                        if(item.name == 'p') {
                            intellisense.logMessage('>>> item p value:' + item.value);
                        }
                    });
                });

                intellisense.addEventListener('statementcompletionhint',  function(e) {
                    intellisense.logMessage('>>> ' + e.completionItem.value);
                });
                var x = {};
                Object.defineProperty(x, 'p', { 
                    get: function() {
                        failWithOOM();
                        failRecursion();
                        return 'hi';
                    }, enumerable: true, configurable: true });
                
                x.p|;
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset);
                     EnsureMessageLogged(context, ">>> item p value:hi");
                 }, helpers);

            // Verify long operation in getter
            PerformRequests(
              @"
                var x = {};
                Object.defineProperty(x, 'p', { 
                    get: function() {
                        failWithOOM();
                        return 'hi';
                    }, enumerable: true, configurable: true });
                
                x.p|;
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset);
                     var hint = completions.GetHintFor("p");
                     hint.Type.Expect(AuthorType.atString);
                 }, helpers);

            // Verify the property value returned by the getter is being considered in completion hint
            PerformRequests(
              @"
                var x = {};
                Object.defineProperty(x, 'p', { 
                    get: function() {
                        return 'hi';
                    }, enumerable: true, configurable: true });
                
                x.p|;
                ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("p");
                 hint.Type.Expect(AuthorType.atString);
             });

            // Verify that CompletionItem.value is initialized in statementcompletionhint event
            PerformRequests(
              @"
                intellisense.addEventListener('statementcompletionhint',  function(e) {
                    intellisense.logMessage('>>> ' + e.completionItem.value);
                });
                var x = {};
                Object.defineProperty(x, 'p', { 
                    get: function() {
                        return 'hi';
                    }, enumerable: true, configurable: true });
                
                x.p|;
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset);
                     var hint = completions.GetHintFor("p");
                     hint.Type.Expect(AuthorType.atString);
                     EnsureMessageLogged(context, ">>> hi");
                 });

            // Verify that CompletionItem.value is initialized in statementcompletion event
            PerformRequests(
              @"
                intellisense.addEventListener('statementcompletion',  function(e) {
                    e.items.forEach(function(item) {
                        if(item.name == 'p') {
                            intellisense.logMessage('>>> item p value:' + item.value);
                        }
                    });
                });

                intellisense.addEventListener('statementcompletionhint',  function(e) {
                    intellisense.logMessage('>>> ' + e.completionItem.value);
                });
                var x = {};
                Object.defineProperty(x, 'p', { 
                    get: function() {
                        return 'hi';
                    }, enumerable: true, configurable: true });
                
                x.p|;
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset);
                     EnsureMessageLogged(context, ">>> item p value:hi");
                 });

            // Verify setting CompletionItem.value in statementcompletion event
            PerformRequests(
              @"
                intellisense.addEventListener('statementcompletion',  function(e) {
                    e.items.forEach(function(item) {
                        if(item.name == 'p') {
                            intellisense.logMessage('>>> item p value before:' + item.value);
                            item.value = 'hi';
                            intellisense.logMessage('>>> item p value after:' + item.value);
                        }
                    });
                });

                intellisense.addEventListener('statementcompletionhint',  function(e) {
                    intellisense.logMessage('>>> e.completionItem.value:' + e.completionItem.value);
                });
                var x = {};
                Object.defineProperty(x, 'p', { 
                    get: function() {
                        return this._p;
                    }, 
                    set: function(v) {
                        this._p = v;
                    },
                    enumerable: true, configurable: true });
                
                x.p|;
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset);
                     var hint = completions.GetHintFor("p");
                     hint.Type.Expect(AuthorType.atString);
                 });
        }

        [TestMethod]
        [WorkItem(312521)]
        public void getVarDate()
        {
            TestCompletionHint(
              @"var oDate = Date.prototype.getVarDate();
                ;|
                ",
                "oDate", hint =>
                {
                    hint.Type.Expect(AuthorType.atObject);
                });

            TestCompletionHint(
              @"var oDate = Date.prototype.getVarDate();
                oDate.x.|;
                ",
                "toString", hint =>
                {
                    hint.Type.Expect(AuthorType.atFunction);
                });
        }

        [TestMethod]
        [WorkItem(273966)]
        public void MemberFunctionDescription()
        {
            PerformRequests(@"
                Server = function Server() {
                    /// <summary>Class description</summary>
                }
                Server.prototype = {
                    constructor: function() {
                        /// <param name='a' type='Number'>A parameter</param>
                        /// <field name='foo' type='Number'>Foo description</field>
                    },
                    prop1:  {
                        get: function() {
                            /// <param name='a' type='Number'>A parameter</param>
                            /// <field name='foo' type='Number'>Foo description</field>
                        }
                    },
                    prop2:  {
                        set: function() {
                            /// <param name='a' type='Number'>A parameter</param>
                            /// <field name='foo' type='Number'>Foo description</field>
                        }
                    },
                    bar: function bar(a, b, c) {
                        /// <summary>Bar method</summary>
                        /// <param name='a' type='Number'>A parameter</param>
                        /// <param name='b' type='Number'>B parameter</param>
                        /// <param name='c' type='Number'>C parameter</param>
                        /// <returns type='Number'>A + B + C</returns>
                        return a + b + c;
                    }
                };

                var server = new Server();
                server.|

            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 Action<string> ValidateNoDescription = (item) =>
                 {
                     var hint = completions.GetHintFor(item);
                     hint.Description.Expect(null);
                 };
                 ValidateNoDescription("prop1");
                 ValidateNoDescription("prop2");
                 ValidateNoDescription("constructor");
                 ValidateNoDescription("bar");
             });
        }


        [TestMethod]
        public void UndefinedFieldsDocComments()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion',  function() {});
                function F1() {
                    /// <field name='a' type='String' value='{ x: 1 }'>a desc</field>
                    /// <field name='b' type='String' value='2'>b desc</field>
                    /// <field name='c' value='0'>c desc</field>
                    /// <field name='d' value='1'>d desc</field>
                    /// <field name='e' type='Number'>e desc</field>
                    /// <field name='f' type='String'>f desc</field>
                    intellisense.logMessage('>>> e: ' + this.e);
                    intellisense.logMessage('>>> f: ' + this.f);
                }
                var f1 = new F1();
                intellisense.logMessage('>>> f1.e: ' + f1.e);
                f1.|;
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);

                 var hint = completions.GetHintFor("a");
                 hint.TypeName.Expect("String");
                 hint.Type.Expect(AuthorType.atObject);
                 hint.Description.Expect("a desc");

                 hint = completions.GetHintFor("b");
                 hint.TypeName.Expect("String");
                 hint.Type.Expect(AuthorType.atNumber);
                 hint.Description.Expect("b desc");

                 hint = completions.GetHintFor("c");
                 hint.Type.Expect(AuthorType.atNumber);
                 hint.Description.Expect("c desc");

                 hint = completions.GetHintFor("d");
                 hint.Type.Expect(AuthorType.atNumber);
                 hint.Description.Expect("d desc");

                 hint = completions.GetHintFor("e");
                 hint.Description.Expect("e desc");
             });

        }

        [TestMethod]
        public void FileChangeAfterCompletionRequest()
        {
            Action<string, string, string, string, Action<IAuthorTestFile>, Action<IAuthorTestFile>> Verify =
                (primaryText, contextText, completionItem, expectedDesc, ModifyPrimary, ModifyContext) =>
                {
                    PerformRequests(primaryText, (context, offset, data, index) =>
                    {
                        var completions = context.GetCompletionsAt(offset);
                        var contextFile = context.GetFileByText(contextText);
                        var primaryFile = context.PrimaryFile;
                        if (ModifyPrimary != null)
                            ModifyPrimary(primaryFile);
                        if (ModifyContext != null)
                            ModifyContext(contextFile);
                        context.Update();
                        var hint = completions.GetHintFor(completionItem);
                        hint.Description.Expect(expectedDesc);
                    }, contextText);
                };

            Action<IAuthorTestFile> addSpace = (file) =>
            {
                file.Append(" ");
            };

            Action<IAuthorTestFile> deleteAll = (file) =>
            {
                file.DeleteText(0, file.Text.Length);
            };

            string declText = @"
                    /// <var type='Number'>Employee Number</var>
                    var emp;";

            Verify(declText + "|", "", "emp", "Employee Number", addSpace, null);
            Verify(declText + "|", "", "emp", "Employee Number", deleteAll, null);
            Verify(";|", declText, "emp", "Employee Number", deleteAll, null);
            Verify(";|", declText, "emp", "Employee Number", null, deleteAll);
        }

        [TestMethod]
        [WorkItem(210501)]
        public void PrototypeFieldDocComments()
        {
            // Verifies that when a field comes from the prototype <field> doc comments defined in ctor are still visible

            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var msajax = TestFiles.MicrosoftAjax;

            TestCompletionHint(@"
                intellisense.addEventListener('statementcompletion',  function() {});
                intellisense.addEventListener('statementcompletion',  
                    function (e) { 
                        e.items.forEach(function(item) {
                            if(item.name == 'animalDescription') {
                                if(e.target === item.parentObject)
                                    intellisense.logMessage('>>>> item.name: ' + item.name + ' Parent OK');                                                
                                else {
                                    intellisense.logMessage('>>>> item.name: ' + item.name + ' Parent NOT OK');                                                
                                    intellisense.logMessage('>>>> e.target: ' + JSON.stringify(e.target));                                                
                                    intellisense.logMessage('>>>> item.parentObject: ' + JSON.stringify(item.parentObject));                                                
                                }
                            }
                        });
                    });
                
                Type.registerNamespace('Ms');
                Ms.Organism = function(species) {
                 /// <summary>The Ms.Organism ctor</summary>   
                 /// <param name='species' type='String'>The value for the species</param> 
                 /// <field name='animalDescription' type='String'>If the organism has legs</field>  
                 /// <field name='numLegs' type='Number'>Number of legs organism has</field>
                 /// <field name='noComments' type='String'>no comments</field>   
                 // Field in the ctor
                 this._species = species;
                }

                // SET THE FIELD ON THE PROTOTYPE
                Ms.Organism.prototype.animalDescription = null;

                // Register the class
                Ms.Organism.registerClass('Ms.Organism');
                var x = new Ms.Organism('foo');
                intellisense.logMessage('>>> x._$fieldDoc$animalDescription: ' + x._$fieldDoc$animalDescription.description);
                x.|
            ",
             "animalDescription",
            hint =>
            {
                hint.Description.Expect("If the organism has legs");
            }, dom, msajax);
        }

        [TestMethod]
        [WorkItem(192364)]
        public void BeginningOfScopeWithDirectExecution()
        {
            // Rewritting encloses a function contents into a 'try' statement and we need to make sure that
            // the doc comment was not left out when try statement is introduced.
            TestCompletionHint(
              @"/// <var type='Number'>foobaz</var>
                var global = 20;
                var cat = 10;
                function test()
                {
                   gl|
                ",
                "global", hint =>
                {
                    hint.TypeName.Expect("Number");
                });
            TestCompletionHint(
              @"function x() {
                /// <var type='Number'>foobaz</var>
                var global = 20;
                var cat = 10;
                function test()
                {
                   gl|
                ",
                "global", hint =>
                {
                    hint.TypeName.Expect("Number");
                });
        }

        [TestMethod]
        public void ReuseFileHandleInAnotherContext()
        {
            var text = @"
                    function completionTest(a, b) {
                        /// <signature>
                        /// <param name='a' type='Number'>param1</param>
                        /// <returns type='String' />
                        /// </signature>
                        /// <signature>
                        /// <param name='a' type='Number'>param1</param>
                        /// <param name='b' type='String'>str</param>
                        /// <returns type='String' />
                        /// </signature>
                    }
                    compl";

            var primary = _session.FileFromText(text);
            var context = _session.OpenContext(primary);
            context.Update();
            context.Close();
            context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            Assert.IsNotNull(completions);
            var hint = completions.GetHintFor("completionTest");
            Assert.IsNotNull(hint);
        }

        [TestMethod]
        public void EngineReleaseBeforeCompletionSetRelease()
        {
            PerformRequests(@"var x=0;|",
             (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 context.TakeOwnership(completions);
                 Cleanup();
                 Marshal.ReleaseComObject(completions);
             });
        }

        [TestMethod]
        [WorkItem(198432)]
        public void MalformedDocCommentsAboveVar()
        {
            TestCompletionHint(
              @"
                    /// <reference>
                    var x;|
                ",
                "x", hint =>
                {
                    hint.Name.Expect("x");
                    hint.Scope.Expect(AuthorScope.ascopeGlobal);
                });
        }

        [TestMethod]
        [WorkItem(198574)]
        public void LocalVariableComments()
        {
            TestCompletionHint(
              @"
                    function test() { 
                    /// <var type='Number'>Test</var>
                    var local = 5; loc|
                ",
                "local", hint =>
                {
                    hint.Description.Expect("Test");
                });
        }

        [TestMethod]
        [WorkItem(181280)]
        public void Bug181280()
        {
            PerformRequests(@"
                function test(param) { 
                    /// <param name='param' type='String'>A comment</param>
                    para|
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("param");
                 hint.Description.Expect("A comment");
                 hint.TypeName.Expect("String");
             });
        }

        [TestMethod]
        public void StaticFields()
        {
            TestCompletionHint(
              @"
                    function Animal() {
                        /// <field name='MaxLegCount' static='true' type='Number' helpKeyword='help' locid='locid'>static field desc</field> 
                    }
                    Animal.MaxLegCount = 40;
                    Animal.|
                ",
                "MaxLegCount", hint =>
                {
                    hint.TypeName.Expect("Number");
                    hint.Description.Expect("static field desc");
                    hint.Type.Expect(AuthorType.atNumber);
                    hint.Scope.Expect(AuthorScope.ascopeMember);
                    hint.HelpKeyword.Expect("help");
                    hint.Locid.Expect("locid");
                });

            TestCompletionHint(
              @"
                    function Animal() {
                        /// <field name='MaxLegCount' static='true' type='Number' helpKeyword='help' locid='locid'>desc</field> 
                        /// <field name='GenericName' static='true' type='String' helpKeyword='help' locid='locid'>static field desc</field> 
                    }
                    Animal.MaxLegCount = 40;
                    Animal.GenericName = 'Foo';
                    Animal.|
                ",
                "GenericName", hint =>
                {
                    hint.TypeName.Expect("String");
                    hint.Description.Expect("static field desc");
                    hint.Type.Expect(AuthorType.atString);
                    hint.Scope.Expect(AuthorScope.ascopeMember);
                    hint.HelpKeyword.Expect("help");
                    hint.Locid.Expect("locid");
                });

            PerformCompletionRequests(
              @"
                    function Animal() {
                        /// <field name='MaxLegCount' static='true' type='Number' helpKeyword='help' locid='locid'>desc</field> 
                        /// <field name='GenericName' static='true' type='String' helpKeyword='help' locid='locid'>static field desc</field> 
                        /// <field name='Name' type='String' helpKeyword='help' locid='locid'>field desc</field> 
                        this.Name='Boo';
                    }
                    Animal.MaxLegCount = 40;
                    Animal.GenericName = 'Foo';
                    var animal = new Animal();
                    animal.|
                ",
             (completions, data, index) =>
             {
                 completions.ToEnumerable().ExpectNotContains(new[] { "MaxLegCount", "GenericName" });
                 completions.ToEnumerable().ExpectContains(new[] { "Name" });
             });
        }

        [TestMethod]
        public void ArgumentDocComments()
        {
            TestCompletionHint(
              @"
                    function eat(foods) {
                        /// <signature> 
                        /// <param name='foods' type='Array' elementType='Food' locid='locid'>foods</param> 
                        /// </signature> 
                        foods|;
                    }
                ",
                "foods", hint =>
                {
                    hint.TypeName.Expect("Array");
                    hint.ElementType.Expect("Food");
                    hint.Description.Expect("foods");
                    hint.Type.Expect(AuthorType.atArray);
                    hint.Scope.Expect(AuthorScope.ascopeParameter);
                    hint.Locid.Expect("locid");
                });

            TestCompletionHint(
              @"
                    function Food() {
                        /// <field name='price' type='Number' helpKeyword='price help' locid='locid'>food price</field>        
                        this.price = 20;
                    }
                    function eat(food1, food2) {
                        /// <signature> 
                        /// <param name='food1' type='Food' >food1 to eat</param> 
                        /// <param name='food2' type='Food' >food2 to eat</param> 
                        /// </signature> 
                        food2.|;
                    }
                ",
                "price", hint =>
                {
                    hint.TypeName.Expect("Number");
                    hint.Description.Expect("food price");
                    hint.Type.Expect(AuthorType.atNumber);
                    hint.Scope.Expect(AuthorScope.ascopeMember);
                    hint.HelpKeyword.Expect("price help");
                    hint.Locid.Expect("locid");
                });

            // Ensure the argument doc comments are not propogated when an argument is 
            // returned from function
            TestCompletionHint(
              @"
                    function eat(food1, food2, food3) {
                        /// <signature> 
                        /// <param name='food1' type='Food' >food1 to eat</param> 
                        /// <param name='food2' type='Food' >food2 to eat</param> 
                        /// <param name='food3' type='Food' >food3 to eat</param> 
                        /// </signature> 
                        return food3;
                    }
                    var food3 = eat({}, {}, {});
                    food3|
                ",
                "food3", hint =>
                {
                    hint.TypeName.Expect(null);
                    hint.Description.Expect(null);
                    hint.Type.Expect(AuthorType.atObject);
                    hint.Scope.Expect(AuthorScope.ascopeGlobal);
                });

            TestCompletionHint(
              @"
                    function animal(legs, hasTail) {
                        /// <signature> 
                        /// <param name='legs' type='Number' >legs param</param> 
                        /// <param name='hasTail' type='Boolean' >hasTail param</param> 
                        /// </signature> 
                        hasTail|
                    }
                ",
                "hasTail", hint =>
                {
                    hint.TypeName.Expect("Boolean");
                    hint.Description.Expect("hasTail param");
                    hint.Scope.Expect(AuthorScope.ascopeParameter);
                });

            TestCompletionHint(
              @"
                    function animal(legs) { 
                        /// <param name='legs' type='Number' >legs param</param> 
                        legs|
                    }
                ",
                "legs", hint =>
                {
                    hint.TypeName.Expect("Number");
                    hint.Description.Expect("legs param");
                    hint.Scope.Expect(AuthorScope.ascopeParameter);
                });

        }

        [TestMethod]
        public void ConstructorFieldsDocComments()
        {
            string code = @"
                function Animal1() {
                    /// <field name = 'legs' type='Number'>legs field</field> 
                    /// <field name = 'IQ' type='IQ'>IQ field</field> 
                    /// <field name = 'Name' type='String'>Name field</field> 
                    this.legs = 4;
                    this.IQ = { value: 20 };
                    this.Name = 'Dino';
                }
                function Animal2() {
                    /// <field name = 'legs' type='Number'>legs field</field> 
                    /// <field name = 'IQ' type='IQ'>IQ field</field> 
                    /// <field name = 'Name' type='String'>Name field</field> 
                    return { legs: 4 , IQ : { value: 20 }, Name: 'Dino' };
                }
                new Animal1().|;
                new Animal2().|;
                ";
            TestCompletionHint(code, "legs", AuthorType.atNumber, AuthorScope.ascopeMember, "Number", "legs field");
            TestCompletionHint(code, "IQ", AuthorType.atObject, AuthorScope.ascopeMember, "IQ", "IQ field");
            TestCompletionHint(code, "Name", AuthorType.atString, AuthorScope.ascopeMember, "String", "Name field");

        }

        [TestMethod]
        public void ReturnDocComments()
        {
            string animals = @"
                    function AnimalCtor1() { 
                        /// <returns type='Animal'>this animal description</returns> 
                        this.legs = 4;
                    }
                    function AnimalCtor2() { 
                        /// <returns type='Animal'>this animal description</returns> 
                        return { legs : 4 };
                    }
                    function animal() { 
                        /// <returns type='Animal'>this animal description</returns> 
                        return { legs : 4 };
                    }
                    function animal2() { 
                        /// <returns type='Animal'>this animal description</returns> 
                        if(1) return { legs : 4 };
                    }
                    function animal3() { 
                        /// <signature>
                        /// <returns type='Animal'>this animal description</returns> 
                        /// </signature>
                        return { legs : 4 };
                    }
                    function animal4() { 
                        return { legs : 4 };
                    }
                    function animal5() { 
                        /// <signature>
                        /// <returns type='Animal'>this animal description</returns> 
                        /// </signature>
                        /// <signature>
                        /// <returns type='Animal'>this animal description2</returns> 
                        /// </signature>
                        return { legs : 4 };
                    }";

            // Verify the return value still has the right fields when doc comments applied
            PerformCompletionRequests(
              @"
                    new AnimalCtor1().|;
                    new AnimalCtor2().|;
                    animal().|;
                    animal2().|;
                    animal3().|;
                    animal4().|;
                    animal5().|;
                ",
             (completions, data, index) =>
             {
                 completions.ToEnumerable().ExpectContains(new[] { "legs" });
                 var hint = completions.GetHintFor("legs");
                 Assert.IsNotNull(hint);
                 hint.Type.Expect(AuthorType.atNumber);
             }, animals);

            TestCompletionHint(@"
                var a = new AnimalCtor1();
                a|;
                a = new AnimalCtor2();
                a|;
                a = animal();
                a|;
                a = animal2();
                a|;
                a = animal3();
                a|;
                a = animal5();
                a|;
                ",
             "a", AuthorType.atObject, AuthorScope.ascopeGlobal, "Animal", "this animal description", animals);
        }

        [TestMethod]
        public void DocumentingReturnValue()
        {
            TestCompletionHint(@"
                    function Animal() { 
                        /// <returns type='Animal'>animal description</returns> 
                        this.legs = 4; 
                    }
                    var animal = new Animal();|
                ",
             "animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Animal");
                 hint.Description.Expect("animal description");
             });

            TestCompletionHint(@"
                    function Animal() { 
                        /// <returns type='Animal'>animal description</returns> 
                        return { legs: '' }; 
                    }
                    var animal = new Animal();|
                ",
             "animal", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Animal");
                 hint.Description.Expect("animal description");
             });

            TestCompletionHint(@"
                    function outer() {
                        function A() { 
                            /// <returns type='Type' elementType='elementType' locid='locid' helpKeyword='helpKeyword'>description</returns> 
                            return {};
                        }
                        
                        var x = {
                            a: new A()                                                        
                        };

                        intellisense.annotate(x, {
                            /// <field type='Field_Type'>Field_Description</field>        
                            a: undefined
                        });

                        x.|;
                    }
                ",
             "a", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Field_Type");
                 hint.ElementType.Expect(null);
                 hint.Locid.Expect(null);
                 hint.HelpKeyword.Expect("helpKeyword");
                 hint.Description.Expect("Field_Description");
             });

            // <var> comments should take precedence over info attached by <returns>
            TestCompletionHint(@"
                    function A() { 
                        /// <returns type='Type' elementType='elementType' locid='locid' helpKeyword='helpKeyword'>description</returns> 
                    }
                    /// <var type='Var_Type' elementType='Var_elementType' locid='Var_locid' helpKeyword='Var_helpKeyword'>Var_description</var> 
                    var a = new A();|
                ",
             "a", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Var_Type");
                 hint.ElementType.Expect("Var_elementType");
                 hint.Locid.Expect("Var_locid");
                 hint.HelpKeyword.Expect("Var_helpKeyword");
                 hint.Description.Expect("Var_description");
             });

            // Absent <var> comments, info from <returns> should be used
            TestCompletionHint(@"
                    function A() { 
                        /// <returns type='Type' elementType='elementType' locid='locid' helpKeyword='helpKeyword'>description</returns> 
                    }
                    
                    /// <var></var> 
                    var a = new A();|
                    var a1 = new A();|
                ",
             "a", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Type");
                 hint.ElementType.Expect("elementType");
                 hint.Locid.Expect("locid");
                 hint.HelpKeyword.Expect("helpKeyword");
                 hint.Description.Expect("description");
             });

            // Merge <var> and <returns> info.
            // helpKeyword should come from <returns>
            TestCompletionHint(@"
                    function outer() {
                        function A() { 
                            /// <returns type='Type' elementType='elementType' locid='locid' helpKeyword='helpKeyword'>description</returns> 
                            return {};
                        }

                        /// <var type='Var_Type'>Var_description</var> 
                        var a = new A();|
                    }
                ",
             "a", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Var_Type");
                 hint.ElementType.Expect(null);
                 hint.Locid.Expect(null);
                 hint.HelpKeyword.Expect("helpKeyword");
                 hint.Description.Expect("Var_description");
             });

            // Verify that attributes from <returns> are merged correctly. 
            // elementType should not be merged because <var> has type attribute.
            // locid, description should not be merged because <var> has a description.
            // helpKeyword should be merged.
            TestCompletionHint(@"
                    function A() { 
                        /// <returns type='Type' elementType='elementType' locid='locid' helpKeyword='helpKeyword'>description</returns> 
                        return {};
                    }

                    /// <var type='Var_Type'>Var_description</var> 
                    var a = new A();|
                ",
             "a", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("Var_Type");
                 hint.ElementType.Expect(null);
                 hint.Locid.Expect(null);
                 hint.HelpKeyword.Expect("helpKeyword");
                 hint.Description.Expect("Var_description");
             });

            // Reassigning documented var value. Should ignore the original doc comments.
            TestCompletionHint(@"
                    /// <var type='a_Type' helpKeyword='a_helpKeyword'>a description</var> 
                    var a = {};
                    /// <var type='b_Type'>b description</var> 
                    var b = a;|
                ",
             "b", hint =>
             {
                 Assert.IsNotNull(hint);
                 hint.TypeName.Expect("b_Type");
                 hint.ElementType.Expect(null);
                 hint.Locid.Expect(null);
                 hint.HelpKeyword.Expect(null);
                 hint.Description.Expect("b description");
             });
        }

        [TestMethod]
        public void RecyclerTest()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var primary = _session.FileFromText(dom + @"
                    document.");
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            var hint = completions.GetHintFor("adoptNode");
            context.Update();
            // Allow recycler to collect
            System.Threading.Thread.Sleep(5000);
            hint = completions.GetHintFor("addEventListener");
            // Allow recycler to collect
            System.Threading.Thread.Sleep(5000);
            hint = completions.GetHintFor("adoptNode");
        }

        [TestMethod]
        public void builtInFunctions()
        {
            TestCompletionHint(@"
                    Math.| 
                ",
             "ceil", hint =>
             {
                 hint.Type.Expect(AuthorType.atFunction);
                 var funcHelp = hint.GetFunctionHelp();
                 funcHelp.FunctionName.Expect("ceil");
             });
        }

        [TestMethod]
        public void Bug181277()
        {
            TestCompletionHint(
            @"
                var x={};
                x.|     
            ",
             "toString",
             hint =>
             {
                 hint.Type.Expect(AuthorType.atFunction);
                 hint.Scope.Expect(AuthorScope.ascopeMember);
                 var funcHelp = hint.GetFunctionHelp();
                 funcHelp.FunctionName.Expect("toString");
             });
        }

        [TestMethod]
        public void CompletionHintExtensions()
        {
            // Empty handler
            TestCompletionHint(
            @"
                intellisense.addEventListener('statementcompletionhint', 
                    function (e) {
                        intellisense.logMessage('addCompletionHintHandler');
                });
                
                /// <var type='String'>x variable</var>
                var x = '';|;
                ",
             "x",
             AuthorType.atString,
             AuthorScope.ascopeGlobal,
             "String",
             "x variable");

            // Global scope - handler in a context file
            TestCompletionHint(
            @"
                var x = '';|;
                ",
             "x",
             AuthorType.atNumber,
             AuthorScope.ascopeLocal,
             "Integer",
             "x variable", @"
                intellisense.addEventListener('statementcompletionhint', 
                    function (e) {
                        intellisense.logMessage('addCompletionHintHandler');
                        e.symbolHelp.symbolType = 'Number';                    
                        e.symbolHelp.symbolDisplayType = 'Integer';
                        e.symbolHelp.description = 'x variable';                    
                        e.symbolHelp.scope = 'local';
                });
             ");

            // Global scope - handler in the primary file
            TestCompletionHint(
            @"
                intellisense.addEventListener('statementcompletionhint', 
                    function (e) {
                        intellisense.logMessage('addCompletionHintHandler');
                        e.symbolHelp.symbolType = 'Number';                    
                        e.symbolHelp.symbolDisplayType = 'Integer';
                        e.symbolHelp.description = 'x variable';                    
                        e.symbolHelp.scope = 'local';
                });
                
                var x = '';|;
                ",
             "x",
             AuthorType.atNumber,
             AuthorScope.ascopeLocal,
             "Integer",
             "x variable");

            // Member function
            TestCompletionHint(
            @"
                intellisense.addEventListener('statementcompletionhint', 
                    function (e) {
                        intellisense.logMessage('addCompletionHintHandler');
                        e.symbolHelp.symbolType = 'Function';                    
                        e.symbolHelp.symbolDisplayType = 'Integer';
                        e.symbolHelp.description = 'x variable';                    
                        e.symbolHelp.scope = 'local';
                        e.symbolHelp.functionHelp = {
                            functionName: 'func1',
                            signatures = [ 
                            { 
                                'description': 'adds two numbers',
                                'returnType':'String',
                                'returnValueDescription':'The sum of x, y',
                                'params':[ 
                                    { 'description':'Value of x', 'name':'x', 'type':'Number' }, 
                                    { 'description':'Value of y', 'name':'y', 'type':'Number' }
                                ]
                            }
                        ]
                    };
                });
                
                var x = { field1: '' };
                x.|;
                ",
             "field1", hint =>
             {
                 var funcHelp = hint.GetFunctionHelp();
                 Assert.IsNotNull(hint);
                 funcHelp.FunctionName.Except("func1");
                 var signature = funcHelp.GetSignatures().ToEnumerable().Single();
                 signature.Description.Except("adds two numbers");
             });

            // Field
            TestCompletionHint(
            @"
                intellisense.addEventListener('statementcompletionhint', 
                    function (e) {
                        intellisense.logMessage('addCompletionHintHandler');
                        e.symbolHelp.symbolType = 'Number';                    
                        e.symbolHelp.symbolDisplayType = 'Integer';
                        e.symbolHelp.description = 'x variable';                    
                        e.symbolHelp.scope = 'local';
                });
                
                var x = { field1: '' };
                x.|;
                ",
             "field1",
             AuthorType.atNumber,
             AuthorScope.ascopeLocal,
             "Integer",
             "x variable");

        }

        [TestMethod]
        [Ignore]
        //TODO: basipov - 'this' should appear in completions list but currently it is not.
        public void thisInFunc()
        {
            TestCompletionHint(
            @"
                function Point(x, y) {
                    this.x = x;
                    t|
                }
            ",
             "this",
             AuthorType.atObject,
             AuthorScope.ascopeLocal);
        }

        [TestMethod]
        public void FieldDocComments()
        {
            TestCompletionHint(
            @"var x = {
                    field1: 0,
                    /// <field type='Function'>field2desc</field>
                    field2: function(a) { 
                        /// <summary>field2 func description</summary>
                    },
                    field3: 2
                };
                x.|;
                ",
             "field2", hint =>
             {
                 hint.Type.Expect(AuthorType.atFunction);
                 hint.Scope.Expect(AuthorScope.ascopeMember);
                 hint.Description.Expect("field2desc");
                 hint.TypeName.Expect("Function");
                 var funcHelp = hint.GetFunctionHelp();
                 funcHelp.FunctionName.Expect("field2");
                 var signature = funcHelp.GetSignatures().ToEnumerable().Single();
                 signature.Description.Expect("field2 func description");
             });

            TestCompletionHint(
            @"  intellisense.addEventListener('statementcompletionhint',  function() {});
                function fctor() {
                /// <field type='Number' name='field1'>field1 variable</field>
                /// <field type='String' name='field2'>field2 variable</field>
                this.field1 = 1;
                this.field2 = 'a';
                return; 
              }
              var x = new fctor();
              intellisense.logMessage('>>>>>>>> v:' + JSON.stringify(x));   
              x.|;
              ",
             "field1",
             AuthorType.atNumber,
             AuthorScope.ascopeMember,
             "Number",
             "field1 variable");

            TestCompletionHint(
            @"var x = {
                    /// <field type='Number'>field1 variable</field>
                    field1: 0,
                    field2: 1,
                    field3: 2
                };
                x.|;
                ",
             "field1",
             AuthorType.atNumber,
             AuthorScope.ascopeMember,
             "Number",
             "field1 variable");

            TestCompletionHint(
            @"function fctor() {
                /// <field type='Number' name='field1'>field1 variable</field>
                /// <field type='String' name='field2'>field2 variable</field>
                this.field1=1;
                this.field2='a';
              }
              var x = new fctor();
              x.|;
              ",
             "field1",
             AuthorType.atNumber,
             AuthorScope.ascopeMember,
             "Number",
             "field1 variable");

            TestCompletionHint(
            @"var x = {
                    field1: 0,
                    /// <field type='Object'>field2 variable</field>
                    field2: {},
                    field3: 2
                };
                x.|;
                ",
             "field2",
             AuthorType.atObject,
             AuthorScope.ascopeMember,
             "Object",
             "field2 variable");
        }

        [TestMethod]
        [WorkItem(335240)]
        public void FieldDocCommentFile()
        {
            TestCompletionHint(
            @"function fctor() {
                /// <field type='Number' name='field1'>field1 variable</field>
                this.field1=1;
              }
              var x = new fctor();
              x.|;
              ",
             "field1",
             symbolHelp =>
             {
                 Assert.IsNotNull(symbolHelp.SourceFileHandle);
             });
        }

        [TestMethod]
        public void VarDocComments()
        {
            TestCompletionHint(
            @"function f() {
                  var v = 1;
                  /// <var type='Number'>x variable</var>
                  var x = 0;|;
                }
                ",
             "x",
             AuthorType.atNumber,
             AuthorScope.ascopeLocal,
             "Object",
             "x variable");

            TestCompletionHint(
            @"if(true) {
                  var v = 1;
                  for(var y = 0; y < 1; y++) { }    
                  /// <var type='Number'>x variable</var>
                  var x = 0;|;
                }
                ",
             "x",
             AuthorType.atNumber,
             AuthorScope.ascopeGlobal,
             "Object",
             "x variable");

            TestCompletionHint(
            @"if(true) {
                  /// <var type='Number'>x variable</var>
                  var x = 0;|;
                }
                ",
             "x",
             AuthorType.atNumber,
             AuthorScope.ascopeGlobal,
             "Object",
             "x variable");

            TestCompletionHint(
            @"/// <var type='Object'>x variable</var>
                  var x = { };|;
                ",
             "x",
             AuthorType.atObject,
             AuthorScope.ascopeGlobal,
             "Object",
             "x variable");
        }

        [TestMethod]
        [Ignore]
        public void ParamDocComments()
        {
            TestCompletionHint(
            @"function f(a) {
                  /// <param type='Number'>a param</param>
                  a|
                }
                ",
             "a",
             AuthorType.atNumber,
             AuthorScope.ascopeParameter,
             "Number",
             "a param");
        }

        [TestMethod]
        public void localObjectProperty()
        {
            TestCompletionHint(@"
                function PropertyTest() {
                    var x = { localNumProp: 2 };
                    x.|;
                }",
             "localNumProp",
             AuthorType.atNumber,
             AuthorScope.ascopeMember);
        }

        [TestMethod]
        public void funcProperty()
        {
            TestCompletionHint(@"
                function funcPropTest() {
                    var x = { f: function(a) {} };
                    x.|;
                }",
             "f",
             AuthorType.atFunction,
             AuthorScope.ascopeMember);
        }

        [TestMethod]
        public void localArray()
        {
            TestCompletionHint(@"
                function LocalArrayTest() {
                    var arr = [ 'a', 'b'];
                    arr|;
                }",
             "arr",
             AuthorType.atArray,
             AuthorScope.ascopeLocal);
        }


        [TestMethod]
        public void localFunction()
        {
            TestCompletionHint(@"
                    function localFunc(v) {}
                    localFunc|;
                ",
             "localFunc",
             AuthorType.atFunction,
             AuthorScope.ascopeGlobal);
        }

        [TestMethod]
        public void globalObjectWithDocComments()
        {
            var s = @"
                var j = {
                    size: function () {
                        ///	<summary>The number of elements currently matched.</summary>
                        ///	<returns type=""Number"" />
                        return this.length;
                    },
                    toArray: function() {
		                ///	<summary>Retrieve all the DOM elements contained in the jQuery set, as an array.</summary>
		                ///	<returns type=""Array"" />
		                return slice.call( this, 0 );
	                }
                };

                j.|;
            ";

            TestCompletionHint(s, "size", AuthorType.atFunction, AuthorScope.ascopeMember);
            TestCompletionHint(s, "toArray", AuthorType.atFunction, AuthorScope.ascopeMember);
        }

        [TestMethod]
        public void globalNumber()
        {
            TestCompletionHint(@"
                    var num = 2;
                    var num2 = num|;
                ",
             "num",
             AuthorType.atNumber,
             AuthorScope.ascopeGlobal);
        }

        [TestMethod]
        public void localNumber()
        {
            TestCompletionHint(@"
                   function testLocal() {
                    var localnum = 2;
                    var num2 = localnum|;
                }",
             "localnum",
             AuthorType.atNumber,
             AuthorScope.ascopeLocal);
        }

        [TestMethod]
        [Ignore]
        // TODO: basipov - temporarily disabled due to a bug. Supposed to return ascopeClosure.
        public void closureObject()
        {
            TestCompletionHint(@"
                   function testClosure() {
                    var v = {};
                    function inner() {
                        v|
                    }
                }",
             "v",
             AuthorType.atObject,
             AuthorScope.ascopeClosure
             );
        }

        [TestMethod]
        public void lambdaObject()
        {
            TestCompletionHint(@"
                   function testClosure() {
                    var v = {};
                    return () => {
                        v|
                    }
                }",
             "v",
             AuthorType.atObject,
             AuthorScope.ascopeLocal
             );
        }

        [TestMethod]
        public void localParam()
        {
            TestCompletionHint(@"
                    function testLocalParam(a, localarg) {
                        var num2 = localarg|;
                    }
                ",
             "localarg",
             AuthorType.atUnknown,
             AuthorScope.ascopeParameter);
        }

        [TestMethod]
        public void insideCallArgs()
        {
            TestCompletionHint(@"
                    function insideCallArgsTest() {
                        function calcRoot(v) {}
                        var localnum = 2;
                        calcRoot(localnum|);
                    }
                ",
             "localnum",
             AuthorType.atNumber,
             AuthorScope.ascopeLocal);
        }

        [TestMethod]
        public void paramOfFunctionType()
        {
            TestCompletionHint(@"
                    function funcParameterTest(fnc) { fnc|; }
                    funcParameterTest(function(a, b) { });
                ",
             "fnc",
             AuthorType.atFunction,
             AuthorScope.ascopeParameter);
        }

        [TestMethod]
        public void ExternalFileAttribute()
        {
            Action<IAuthorSymbolHelp> validate = (hint) =>
            {
                hint.ExternalFile.Expect("external File");
                hint.Externalid.Expect("external id");
            };

            TestCompletionHint(@"
                    /// <var externalFile='external File' externalid='external id'></var> 
                    var v = {}; // variable
                    v|;", "v", validate);

            TestCompletionHint(@"
                    function SignatureTest() { 
                        /// <field name='_field' externalFile='external File' externalid='external id'></field> 
                        this._field = 0;
                    }

                var signatureTest  = new SignatureTest(); 
                signatureTest._field|;", "_field", validate);
        }

        [TestMethod]
        [WorkItem(199843)]
        [WorkItem(205878)]
        public void InternalFileHandles()
        {
            Action<IAuthorSymbolHelp> validate = (hint) =>
            {
                Assert.IsNotNull(hint);
                switch (hint.Name)
                {
                    case "x":
                    case "y":
                        Assert.IsNotNull(hint.SourceFileHandle);
                        break;
                    case "foo":
                    case "bar":
                        var functionHelp = hint.GetFunctionHelp();
                        Assert.IsNotNull(functionHelp);
                        Assert.IsNotNull(functionHelp.SourceFileHandle);
                        break;
                    case "hasOwnProperty":
                        Assert.IsNull(hint.SourceFileHandle);
                        var internalFunctionhelp = hint.GetFunctionHelp();
                        Assert.IsNotNull(internalFunctionhelp);
                        Assert.IsNotNull(internalFunctionhelp.SourceFileHandle);
                        break;
                }
            };

            // definitions in primary file
            TestCompletionHint(@"var x = 0; ;|;", "x", validate);
            TestCompletionHint(@"function bar (a) { } ;|;", "bar", validate);

            // definitions in a context file
            TestCompletionHint(@"|", "y", validate, @" var y = {};");
            TestCompletionHint(@"|", "foo", validate, @" function foo(a,b,c) {}");

            // function in LibHelp
            TestCompletionHint(@"function f(){} f.|", "hasOwnProperty", validate);
        }

        [TestMethod]
        public void ContextChanges()
        {
            var c1 = _session.FileFromText("function subtract(a, b) { return a - b; }");
            var c2 = _session.FileFromText("function subtract(a, b, c) { return a - b - c; }");
            var c3 = _session.FileFromText("");
            var primaryFile = _session.FileFromText("subtract");
            var context = _session.OpenContext(primaryFile, c1, c2, c3);

            // Produce some result.
            var r1 = context.GetCompletionsAt(8);

            // Change the orderof the context files.
            context.RemoveContextFiles(c1);
            context.AddContextFiles(c1);

            // Get a completion list.
            var result = context.GetCompletionsAt(8);

            // Throw away the previous result.
            context.TakeOwnership(r1);
            Marshal.ReleaseComObject(r1);

            // Find subtract
            var index = 0;
            result.ToEnumerable().Where((c, i) =>
            {
                if (c.DisplayText == "subtract") index = i;
                return false;
            }).SingleOrDefault();

            // The the help for subtract
            var help = result.GetHintFor(index);
        }

        [TestMethod]
        public void BuiltinFunctions()
        {
            TestCompletionHint(
              @"
                    'string'.|
                ",
                "anchor", hint =>
                {
                    Assert.IsNotNull(hint);
                });
        }

        [TestMethod]
        [WorkItem(362097)]
        public void MemberCommentAssociation()
        {
            TestCompletionHint(
            @"
                function foo() {
                    return {};
                }

                var x = {
                    field1: 0,
                    /// <field type='Object'>field2desc</field>
                    field2: foo({
                        f: function () { 
                            // comment
                            var x = 0;
                        }
                    }),
                    field3: 2
                };
                x.|;
                ",
             "field2", hint =>
             {
                 hint.Type.Expect(AuthorType.atObject);
                 hint.Scope.Expect(AuthorScope.ascopeMember);
                 hint.Description.Expect("field2desc");
                 hint.TypeName.Expect("Object");
             });
        }


        [TestMethod]
        [WorkItem(362097)]
        public void CommentAfterIfStatement()
        {
            TestCompletionHint(
            @"
                if (true) {
                    var className;
                    /// <var type='Date'>z variable</var>
                }

                var x = new Date();

                ;|;
                ",
             "x", hint =>
             {
                 hint.Type.Expect(AuthorType.atDate);
                 hint.Scope.Expect(AuthorScope.ascopeGlobal);
                 Assert.IsNull(hint.Description);
             });
        }

        [TestMethod]
        public void DeprecatedSignatureDocComments()
        {
            string deprecateType = "deprecate";
            string removeType = "remove";
            string expectedMessage = "deprecated signature message";

            string test = @"
                    function FooCtor1() { 
                        /// <deprecated type='deprecate'>deprecated signature message</deprecated> 
                    }
                    function FooCtor2() { 
                        /// <deprecated type='remove'>deprecated signature message</deprecated> 
                    }
                    function foo() { 
                        /// <deprecated>deprecated signature message</deprecated> 
                    }
                    function foo2() { 
                        /// <deprecated type='deprecate'></deprecated> 
                    }
                    function foo3() { 
                        /// <signature>
                        /// <deprecated type='deprecate'>deprecated signature message</deprecated> 
                        /// </signature>
                    }
                    function foo4() { 
                    }
                    function foo5() { 
                        /// <signature>
                        /// <summary>NOT DEPRECATED</summary>
                        /// </signature>
                    }
                    function foo6() { 
                        /// <signature>
                        /// <deprecated type='deprecate'>deprecated signature message</deprecated> 
                        /// <deprecated type='remove'>removed signature message</deprecated> 
                        /// </signature>
                        /// <signature>
                        /// <deprecated type='deprecate'>deprecated signature message 2</deprecated> 
                        /// </signature>
                    }
                    function foo7() {
                        /// <deprecated></deprecated> 
                    }
                    f|";

            TestCompletionHint(test, "FooCtor1", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect(deprecateType);
                deprecated.Message.Expect(expectedMessage);
            });
            TestCompletionHint(test, "FooCtor2", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect(removeType);
                deprecated.Message.Expect(expectedMessage);
            });
            TestCompletionHint(test, "foo", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                Assert.IsNull(deprecated.Type);
                deprecated.Message.Expect(expectedMessage);
            });
            TestCompletionHint(test, "foo2", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect(deprecateType);
                Assert.IsNull(deprecated.Message);
            });
            TestCompletionHint(test, "foo3", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect(deprecateType);
                deprecated.Message.Expect(expectedMessage);
            });
            TestCompletionHint(test, "foo4", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);
                Assert.IsNull(signature.GetDeprecated());
            });
            TestCompletionHint(test, "foo5", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var deprecated = signature.GetDeprecated();
                Assert.IsNull(deprecated);
            });
            TestCompletionHint(test, "foo6", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signatures = functionHelp.GetSignatures().ToEnumerable().ToArray();
                Assert.AreEqual(2, signatures.Count());
                var signature = signatures[0];
                Assert.IsNotNull(signature);

                var deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect(removeType);
                deprecated.Message.Expect("removed signature message");

                signature = signatures[1];
                deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect(deprecateType);
                deprecated.Message.Expect("deprecated signature message 2");
            });
            TestCompletionHint(test, "foo7", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                Assert.IsNull(deprecated.Type);
                Assert.IsNull(deprecated.Message);
            });
        }

        [TestMethod]
        public void DeprecatedFieldDocComments()
        {
            string deprecateType = "deprecate";
            string expectedMessage = "deprecated field message";

            string test = @"
                    var x = {
                        /// <field>
                        /// <deprecated type='deprecate'>deprecated field message</deprecated>
                        /// </field>
                        foo: 0;
                    };
                    x.|;
                    x = {
                        /// <field>
                        /// <deprecated type='remove'>old deprecated field message</deprecated>
                        /// <deprecated type='deprecate'>deprecated field message</deprecated>
                        /// </field>
                        foo: 0;
                    };
                    x.|;
                    function XCtor() {
                        /// <field name='foo'>
                        /// <deprecated type='deprecate'>deprecated field message</deprecated>
                        /// </field>
                    };
                    x = new XCtor();
                    x.|;
                    function XCtorWithStatic() {
                        /// <field name='foo' static='true'>
                        /// <deprecated type='deprecate'>deprecated field message</deprecated>
                        /// </field>
                    };
                    x = XCtorWithStatic;
                    x.|;";

            TestCompletionHint(test, "foo", hint =>
            {
                var deprecated = hint.GetDeprecated();
                Assert.IsNotNull(deprecated);
                Assert.IsNull(hint.GetFunctionHelp());
                deprecated.Type.Expect(deprecateType);
                deprecated.Message.Expect(expectedMessage);
            });
        }

        [TestMethod]
        public void DeprecatedFunctionWithFieldDoc()
        {
            string deprecateType = "deprecate";
            string expectedMessage = "deprecated field message";

            string test = @"
                    var x = {
                        /// <field>
                        /// <deprecated type='deprecate'>deprecated field message</deprecated>
                        /// </field>
                        foo: function () { };
                    };
                    x.|;
                    x = {
                        /// <field>
                        /// <deprecated type='remove'>old deprecated field message</deprecated>
                        /// <deprecated type='deprecate'>deprecated field message</deprecated>
                        /// </field>
                        foo: function () { };
                    };
                    x.|;";

            TestCompletionHint(test, "foo", hint =>
            {
                var deprecated = hint.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect(deprecateType);
                deprecated.Message.Expect(expectedMessage);

                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var funcDeprecated = signature.GetDeprecated();
                Assert.IsNotNull(funcDeprecated);
                funcDeprecated.Type.Expect(deprecateType);
                funcDeprecated.Message.Expect(expectedMessage);
            });
        }

        [TestMethod]
        public void DeprecatedVarDocComments()
        {
            string test = @"
                    /// <var>
                    /// <deprecated type=remove>deprecated var message</deprecated>
                    /// </var>
                    var foo;
                    |";

            TestCompletionHint(test, "foo", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                Assert.IsNull(hint.Description);
                Assert.IsNull(hint.GetFunctionHelp());
            });
        }

        [TestMethod]
        public void GetDeprecatedReturnsNullWhenNoDeprecatedAttribute()
        {
            string expectedSummary = "field summary";

            string fieldTest = @"
                    var x = {
                        /// <field>field summary</field>
                        foo: 0;
                    };
                    x.|;
                    function XCtor() {
                        /// <field name='foo'>field summary</field>
                    };
                    x = new XCtor();
                    x.|;
                    function XCtorWithStatic() {
                        /// <field name='foo' static='true'>field summary</field>
                    };
                    x = XCtorWithStatic;
                    x.|;";

            TestCompletionHint(fieldTest, "foo", hint =>
            {
                Assert.AreEqual(expectedSummary, hint.Description);
                var deprecated = hint.GetDeprecated();
                Assert.IsNull(deprecated);
                Assert.IsNull(hint.GetFunctionHelp());
            });

            string functionFieldTest = @"
                    x = {
                        /// <field>field summary</field>
                        foo: function () { };
                    };
                    x.|;";

            TestCompletionHint(functionFieldTest, "foo", hint =>
            {
                Assert.AreEqual(expectedSummary, hint.Description);
                var deprecated = hint.GetDeprecated();
                Assert.IsNull(deprecated);

                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                Assert.AreEqual(expectedSummary, signature.Description);
                var funcDeprecated = signature.GetDeprecated();
                Assert.IsNull(funcDeprecated);
            });
        }

        [TestMethod]
        public void GetDeprecatedReturnsNonNullForEmptyDeprecatedAttribute()
        {
            string fieldTest = @"
                    var x = {
                        /// <field>
                        /// <deprecated></deprecated>
                        /// </field>
                        foo: 0;
                    };
                    x.|;
                    function XCtor() {
                        /// <field name='foo'>
                        /// <deprecated></deprecated>
                        /// </field>
                    };
                    x = new XCtor();
                    x.|;
                    function XCtorWithStatic() {
                        /// <field name='foo' static='true'>
                        /// <deprecated></deprecated>
                        /// </field>
                    };
                    x = XCtorWithStatic;
                    x.|;";

            TestCompletionHint(fieldTest, "foo", hint =>
            {
                var deprecated = hint.GetDeprecated();
                Assert.IsNotNull(deprecated);
                Assert.IsNull(deprecated.Type);
                Assert.IsNull(deprecated.Message);
                Assert.IsNull(hint.GetFunctionHelp());
            });

            string functionFieldTest = @"
                    x = {
                        /// <field>
                        /// <deprecated></deprecated>
                        /// </field>
                        foo: function () { };
                    };
                    x.|;";

            TestCompletionHint(functionFieldTest, "foo", hint =>
            {
                var deprecated = hint.GetDeprecated();
                Assert.IsNotNull(deprecated);
                Assert.IsNull(deprecated.Type);
                Assert.IsNull(deprecated.Message);

                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var funcDeprecated = signature.GetDeprecated();
                Assert.IsNotNull(funcDeprecated);
                Assert.IsNull(funcDeprecated.Type);
                Assert.IsNull(funcDeprecated.Message);
            });
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        public void CompatibleWithSignatureDocComments()
        {
            string windowsPlatform = "Windows";
            string windowsPhonePlatform = "WindowsPhone";
            string windowsPhoneAppxPlatform = "WindowsPhoneAppx";
            string blueMinVersion = "8.1";
            string sampleMinVersion = "42.0.9.1";

            string test = @"
                    function FooCtor() { 
                        /// <compatibleWith platform='Windows' minVersion='8.1' />
                        /// <compatibleWith platform='WindowsPhone' minVersion='8.1' />
                        /// <compatibleWith platform='WindowsPhoneAppx' minVersion='42.0.9.1' />
                    }
                    function foo() { 
                        /// <compatibleWith platform='Windows' />
                        /// <compatibleWith minVersion='8.1' />
                        /// <compatibleWith platform='WindowsPhoneAppx' minVersion='42.0.9.1' />
                    }
                    function foo2() { 
                        /// <signature>
                        /// <compatibleWith platform='Windows' minVersion='8.1' />
                        /// <compatibleWith platform='WindowsPhoneAppx' minVersion='42.0.9.1' />
                        /// </signature>
                    }
                    function foo3() { 
                    }
                    function foo4() { 
                        /// <signature>
                        /// <summary>NO COMPATIBLEWITH ELEMENTS</summary>
                        /// </signature>
                    }
                    function foo5() { 
                        /// <signature>
                        /// <compatibleWith platform='Windows' minVersion='8.1' />
                        /// <compatibleWith platform='WindowsPhone' minVersion='8.1' />
                        /// </signature>
                        /// <signature>
                        /// <compatibleWith platform='WindowsPhoneAppx' minVersion='42.0.9.1' />
                        /// </signature>
                    }
                    function foo6() {
                        /// <compatibleWith></compatibleWith> 
                    }
                    f|";

            TestCompletionHint(test, "FooCtor", hint =>
            {
                Assert.IsNull(hint.GetCompatibleWith());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var compatibleWith = signature.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(3, compatArray.Count());

                Assert.IsNotNull(compatArray[0]);
                compatArray[0].Platform.Expect(windowsPlatform);
                compatArray[0].MinVersion.Expect(blueMinVersion);

                Assert.IsNotNull(compatArray[1]);
                compatArray[1].Platform.Expect(windowsPhonePlatform);
                compatArray[1].MinVersion.Expect(blueMinVersion);

                Assert.IsNotNull(compatArray[2]);
                compatArray[2].Platform.Expect(windowsPhoneAppxPlatform);
                compatArray[2].MinVersion.Expect(sampleMinVersion);
            });
            TestCompletionHint(test, "foo", hint =>
            {
                Assert.IsNull(hint.GetCompatibleWith());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var compatibleWith = signature.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(3, compatArray.Count());

                Assert.IsNotNull(compatArray[0]);
                compatArray[0].Platform.Expect(windowsPlatform);
                compatArray[0].MinVersion.Expect(null);

                Assert.IsNotNull(compatArray[1]);
                compatArray[1].Platform.Expect(null);
                compatArray[1].MinVersion.Expect(blueMinVersion);

                Assert.IsNotNull(compatArray[2]);
                compatArray[2].Platform.Expect(windowsPhoneAppxPlatform);
                compatArray[2].MinVersion.Expect(sampleMinVersion);
            });
            TestCompletionHint(test, "foo2", hint =>
            {
                Assert.IsNull(hint.GetCompatibleWith());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var compatibleWith = signature.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(2, compatArray.Count());

                Assert.IsNotNull(compatArray[0]);
                compatArray[0].Platform.Expect(windowsPlatform);
                compatArray[0].MinVersion.Expect(blueMinVersion);

                Assert.IsNotNull(compatArray[1]);
                compatArray[1].Platform.Expect(windowsPhoneAppxPlatform);
                compatArray[1].MinVersion.Expect(sampleMinVersion);
            });
            TestCompletionHint(test, "foo3", hint =>
            {
                Assert.IsNull(hint.GetCompatibleWith());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var compatibleWith = signature.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(0, compatArray.Count());
            });
            TestCompletionHint(test, "foo4", hint =>
            {
                Assert.IsNull(hint.GetCompatibleWith());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var compatibleWith = signature.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(0, compatArray.Count());
            });
            TestCompletionHint(test, "foo5", hint =>
            {
                Assert.IsNull(hint.GetCompatibleWith());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signatures = functionHelp.GetSignatures().ToEnumerable().ToArray();
                Assert.AreEqual(2, signatures.Count());
                var signature = signatures[0];
                Assert.IsNotNull(signature);

                var compatibleWith = signature.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(2, compatArray.Count());
                Assert.IsNotNull(compatArray[0]);
                compatArray[0].Platform.Expect(windowsPlatform);
                compatArray[0].MinVersion.Expect(blueMinVersion);
                Assert.IsNotNull(compatArray[1]);
                compatArray[1].Platform.Expect(windowsPhonePlatform);
                compatArray[1].MinVersion.Expect(blueMinVersion);

                signature = signatures[1];
                compatibleWith = signature.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(1, compatArray.Count());
                Assert.IsNotNull(compatArray[0]);
                compatArray[0].Platform.Expect(windowsPhoneAppxPlatform);
                compatArray[0].MinVersion.Expect(sampleMinVersion);
            });
            TestCompletionHint(test, "foo6", hint =>
            {
                Assert.IsNull(hint.GetCompatibleWith());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var compatibleWith = signature.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(1, compatArray.Count());
                Assert.IsNotNull(compatArray[0]);
                compatArray[0].Platform.Expect(null);
                compatArray[0].MinVersion.Expect(null);

            });
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        public void CompatibleWithFieldDocComments()
        {
            string windowsPlatform = "Windows";
            string blueMinVersion = "8.1";

            string test = @"
                    var x = {
                        /// <field>
                        /// <compatibleWith platform='Windows' minVersion='8.1' />
                        /// </field>
                        foo: 0;
                    };
                    x.|;
                    function XCtor() {
                        /// <field name='foo'>
                        /// <compatibleWith platform='Windows' minVersion='8.1' />
                        /// </field>
                    };
                    x = new XCtor();
                    x.|;
                    function XCtorWithStatic() {
                        /// <field name='foo' static='true'>
                        /// <compatibleWith platform='Windows' minVersion='8.1' />
                        /// </field>
                    };
                    x = XCtorWithStatic;
                    x.|;";

            TestCompletionHint(test, "foo", hint =>
            {
                var compatibleWith = hint.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                Assert.IsNull(hint.GetFunctionHelp());
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(1, compatArray.Count());
                compatArray[0].Platform.Expect(windowsPlatform);
                compatArray[0].MinVersion.Expect(blueMinVersion);
            });
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        public void CompatibleWithFromFunctionFieldDoc()
        {
            string windowsPlatform = "Windows";
            string windowsPhonePlatform = "WindowsPhone";
            string blueMinVersion = "8.1";

            string test = @"
                    var x = {
                        /// <field>
                        /// <compatibleWith platform='Windows' minVersion='8.1' />
                        /// <compatibleWith platform='WindowsPhone' minVersion='8.1' />
                        /// </field>
                        foo: function () { };
                    };
                    x.|;";

            TestCompletionHint(test, "foo", hint =>
            {
                var compatibleWith = hint.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(2, compatArray.Count());
                Assert.IsNotNull(compatArray[0]);
                compatArray[0].Platform.Expect(windowsPlatform);
                compatArray[0].MinVersion.Expect(blueMinVersion);
                Assert.IsNotNull(compatArray[1]);
                compatArray[1].Platform.Expect(windowsPhonePlatform);
                compatArray[1].MinVersion.Expect(blueMinVersion);

                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var funcCompat = signature.GetCompatibleWith();
                Assert.IsNotNull(funcCompat);
                compatArray = funcCompat.ToEnumerable().ToArray();
                Assert.AreEqual(2, compatArray.Count());
                Assert.IsNotNull(compatArray[0]);
                compatArray[0].Platform.Expect(windowsPlatform);
                compatArray[0].MinVersion.Expect(blueMinVersion);
                Assert.IsNotNull(compatArray[1]);
                compatArray[1].Platform.Expect(windowsPhonePlatform);
                compatArray[1].MinVersion.Expect(blueMinVersion);
            });
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        public void CompatibleWithVarDocComments()
        {
            string test = @"
                    /// <var>
                    /// <compatibleWith platform='Windows' minVersion='8.1' />
                    /// </var>
                    var foo;
                    |";

            TestCompletionHint(test, "foo", hint =>
            {
                Assert.IsNull(hint.GetCompatibleWith());
                Assert.IsNull(hint.Description);
                Assert.IsNull(hint.GetFunctionHelp());
            });
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        public void GetCompatibleWithWhenNoCompatibleWithAttribute()
        {
            string expectedSummary = "field summary";

            string fieldTest = @"
                    var x = {
                        /// <field>field summary</field>
                        foo: 0;
                    };
                    x.|;
                    function XCtor() {
                        /// <field name='foo'>field summary</field>
                    };
                    x = new XCtor();
                    x.|;
                    function XCtorWithStatic() {
                        /// <field name='foo' static='true'>field summary</field>
                    };
                    x = XCtorWithStatic;
                    x.|;";

            TestCompletionHint(fieldTest, "foo", hint =>
            {
                Assert.AreEqual(expectedSummary, hint.Description);
                var compatibleWith = hint.GetCompatibleWith();
                Assert.IsNull(compatibleWith);
                Assert.IsNull(hint.GetFunctionHelp());
            });

            string functionFieldTest = @"
                    x = {
                        /// <field>field summary</field>
                        foo: function () { };
                    };
                    x.|;";

            TestCompletionHint(functionFieldTest, "foo", hint =>
            {
                Assert.AreEqual(expectedSummary, hint.Description);
                var compatibleWith = hint.GetCompatibleWith();
                Assert.IsNull(compatibleWith);

                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                Assert.AreEqual(expectedSummary, signature.Description);
                var funcCompat = signature.GetCompatibleWith();
                Assert.IsNotNull(funcCompat);
                var compatArray = funcCompat.ToEnumerable().ToArray();
                Assert.AreEqual(0, compatArray.Count());
            });
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        public void GetCompatibleWithReturnsNonNullForEmptyAttribute()
        {
            string fieldTest = @"
                    var x = {
                        /// <field>
                        /// <compatibleWith></compatibleWith>
                        /// </field>
                        foo: 0;
                    };
                    x.|;
                    function XCtor() {
                        /// <field name='foo'>
                        /// <compatibleWith></compatibleWith>
                        /// </field>
                    };
                    x = new XCtor();
                    x.|;
                    function XCtorWithStatic() {
                        /// <field name='foo' static='true'>
                        /// <compatibleWith></compatibleWith>
                        /// </field>
                    };
                    x = XCtorWithStatic;
                    x.|;";

            TestCompletionHint(fieldTest, "foo", hint =>
            {
                var compatibleWith = hint.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(1, compatArray.Count());
                Assert.IsNotNull(compatArray[0]);
                Assert.IsNull(compatArray[0].Platform);
                Assert.IsNull(compatArray[0].MinVersion);
                Assert.IsNull(hint.GetFunctionHelp());
            });

            string functionFieldTest = @"
                    x = {
                        /// <field>
                        /// <compatibleWith></compatibleWith>
                        /// </field>
                        foo: function () { };
                    };
                    x.|;";

            TestCompletionHint(functionFieldTest, "foo", hint =>
            {
                var compatibleWith = hint.GetCompatibleWith();
                Assert.IsNotNull(compatibleWith);
                var compatArray = compatibleWith.ToEnumerable().ToArray();
                Assert.AreEqual(1, compatArray.Count());
                Assert.IsNotNull(compatArray[0]);
                Assert.IsNull(compatArray[0].Platform);
                Assert.IsNull(compatArray[0].MinVersion);

                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                var funcCompat = signature.GetCompatibleWith();
                Assert.IsNotNull(funcCompat);
                compatArray = funcCompat.ToEnumerable().ToArray();
                Assert.AreEqual(1, compatArray.Count());
                Assert.IsNotNull(compatArray[0]);
                Assert.IsNull(compatArray[0].Platform);
                Assert.IsNull(compatArray[0].MinVersion);
            });
        }

        [TestMethod]
        [WorkItem(693117)]
        public void SkipUnknownElementsInSignatureAndParamDocComments()
        {
            string deprecateType = "deprecate";
            string expectedMessage = "deprecated message";

            string functionPropertyTest = @"
                var obj = {};
                
                function fooGetter() {
                    return function (ev) {
                        /// <signature>
                        /// <summary>Get a foo</summary>
                        /// <event>colorprofilechanged</event>
                        /// <deprecated type='deprecate'>deprecated message</deprecated>
                        /// </signature>
                    }
                }
                
                Object.defineProperty(obj, ""foo"", { enumerable: true, configurable: false, get: fooGetter });
                
                obj.|;";

            TestCompletionHint(functionPropertyTest, "foo", hint =>
            {
                Assert.IsNull(hint.GetDeprecated());
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                Assert.IsNotNull(signature.Description);
                signature.Description.Expect("Get a foo");
                var deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect(deprecateType);
                deprecated.Message.Expect(expectedMessage);
            });

            string paramTest = @"
                function foo(callback) {
                    /// <signature>
                    /// <summary>foo with callback</summary>
                    /// <param type=""Function"" name=""callback"">
                    /// <summary>callback description</summary>
                    /// <unknown />
                    /// <signature>
                    /// <param name=""bar"" type=""Number"" />
                    /// </signature>
                    /// </param>
                    /// </signature>
                }
                f|;";

            TestCompletionHint(paramTest, "foo", hint =>
            {
                var functionHelp = hint.GetFunctionHelp();
                Assert.IsNotNull(functionHelp);
                var signature = functionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                Assert.IsNotNull(signature.Description);
                signature.Description.Expect("foo with callback");
                var parameters = signature.GetParameters().ToEnumerable();
                Assert.AreEqual(1, parameters.Count());
                var param = parameters.First();
                Assert.IsNotNull(param);

                param.Name.Expect("callback");
                param.Type.Expect("Function");
                param.Description.Expect("callback description");
                var callbackSig = param.FunctionParamSignature;
                Assert.IsNotNull(callbackSig);

                var callbackParameters = callbackSig.GetParameters().ToEnumerable();
                Assert.AreEqual(1, callbackParameters.Count());
                var callbackParam = callbackParameters.First();
                Assert.IsNotNull(callbackParam);
                callbackParam.Name.Expect("bar");
                callbackParam.Type.Expect("Number");
            });
        }

        [TestMethod]
        [WorkItem(593718)]
        public void DeferredCompletionItemValueAvailableAfterGC()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletionhint', function (e) {
                    CollectGarbage();
                    CollectGarbage();
                    CollectGarbage();
                    CollectGarbage();
                    CollectGarbage();
                    CollectGarbage();
                    CollectGarbage();
                    CollectGarbage();
                    CollectGarbage();
                    CollectGarbage();
                    CollectGarbage();
                });

                Windows.UI.WebUI.WebUIBackgroundTaskInstance.|
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("current");
                 Assert.IsNotNull(hint);

                 hint = completions.GetHintFor("current");
                 Assert.IsNotNull(hint);
             }, TestFiles.winrt);
        }

        protected void TestCompletionHint(string script, string completionDisplayName, Action<IAuthorSymbolHelp> validate, params string[] contextFiles)
        {
            PerformCompletionRequests(script, (completions, data, i) =>
            {
                int entryIndex = -1;
                var completion = completions.ToEnumerable().Where((c, index) =>
                {
                    if (c.DisplayText == completionDisplayName)
                    {
                        entryIndex = index;
                        return true;
                    }
                    return false;
                }).SingleOrDefault();
                Assert.IsNotNull(completion);
                Assert.IsTrue(entryIndex >= 0);

                var hint = completions.GetHintFor(entryIndex);
                Assert.IsNotNull(hint);
                validate(hint);
            }, contextFiles);
        }

        private void TestCompletionHint(string script, string completionDisplayName, AuthorType expectedType, AuthorScope expectedScope, string expectedTypeName = "", string expectedDescription = "", params string[] contextFiles)
        {
            TestCompletionHint(script, completionDisplayName, hint =>
            {
                hint.Name.Expect(completionDisplayName);
                hint.Type.Expect(expectedType);
                hint.Scope.Expect(expectedScope);
                if (hint.Type == AuthorType.atFunction)
                {
                    var functionHelp = hint.GetFunctionHelp();
                }
                if (!String.IsNullOrEmpty(expectedTypeName))
                {
                    hint.TypeName.Except(expectedTypeName);
                }
                if (!String.IsNullOrEmpty(expectedDescription))
                {
                    hint.Description.Except(expectedDescription);
                }
            }, CombinedContextFiles(contextFiles));
        }

        private void PerformCompletionRequests(string text, Action<IAuthorCompletionSet, string, int> action, params string[] contextFiles)
        {
            this.PerformCompletionRequests(text, /*needReplacement = */ false, action, contextFiles);
        }

        private void PerformCompletionRequests(string text, bool needReplacement, Action<IAuthorCompletionSet, string, int> action, params string[] contextFiles)
        {
            PerformRequests(text, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                action(completions, data, index);
            },
            CombinedContextFiles(contextFiles));
        }

        private string[] CombinedContextFiles(string[] contextFiles)
        {
            var allContextFiles = new List<string>(contextFiles);
            allContextFiles.AddRange(AdditionalContextFiles);
            return allContextFiles.ToArray();
        }
    }
}
