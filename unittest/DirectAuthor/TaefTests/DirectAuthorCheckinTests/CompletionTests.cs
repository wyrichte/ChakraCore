using System;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class CompletionTests : CompletionsBase
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
        [WorkItem(id: 672552)]
        [WorkItem(id: 672163)]
        [TestCategory("Completions"), TestCategory("Let")]
        public void When_let_is_not_in_the_current_block_it_should_not_show_in_completions()
        {
            PerformCompletionRequests(@"
                let myObject = { c: 0 };
                for(let aLetVar in myObject)
                {
                    let myVariable = 3;
                }
                ^|!myVariable,!aLetVar,myObject|;
            ");
        }

        [TestMethod]
        [WorkItem(id: 672552)]
        [WorkItem(id: 672163)]
        [TestCategory("Completions"), TestCategory("Let")]
        public void When_let_is_in_an_implicit_block_but_not_in_the_current_block_it_should_not_show_in_completions()
        {
            PerformCompletionRequests(@"
                let myObject = { c: 0 };
                const aNumber = 1;
                
                for(let aProp in myObject)
                    let myVariable = 2;

                for(let iVar = 0; iVar < 10; iVar++)
                    let myVariableLoop = 3;

                switch(n)
                {
                    case 3: let mySwitchVar = 4; break;
                }

                ^|myObject,aNumber,!myVariable,!myVariableLoop,!mySwitchVar,!aProp,!iVar|;
            ");
        }

        [TestMethod]
        [WorkItem(688121)]
        [TestCategory("Completions"), TestCategory("Let"), TestCategory("Const")]
        public void LetConstGlobals()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests(@"
                const c = 1;
                let l = 2;
                var v = 3;
                
                ^|c,l,v|;
                window.|v,!c,!l|;

                function f() {
                    ^|c,l,v|;
                    this.|v,!c,!l|;
                }
                f();
            ",domjs);
        }

        [TestMethod]
        [WorkItem(760672)]
        [TestCategory("Completions"), TestCategory("Let"), TestCategory("Const")]
        public void LetConstCompletionBeforeAndAfterDecl()
        {
            PerformCompletionRequests(@"
                ^|!letVar,!constVar|;
                let letVar = 1;
                const constVar = 2;
                ^|letVar,constVar|;
            ");
        }

        [TestMethod]
        [WorkItem(802218)]
        [TestCategory("Completions"), TestCategory("ES6"), TestCategory("Symbol")]
        public void SymbolPrimitive()
        {
            PerformCompletionRequests(@"
                var sym = Symbol('some string');
                var obj = {};
                obj[sym] = 'some value string';
                ^|obj,sym|;
            ");
        }

        [TestMethod]
        [WorkItem(471759)]
        [TestCategory("Completions"), TestCategory("ES6"), TestCategory("StringTemplate")]
        public void SimpleStringTemplate()
        {
            PerformCompletionRequests(@"
                var str = `simple string template`;
                ^|str|;
            ");
        }

        [TestMethod]
        [WorkItem(471759)]
        [TestCategory("Completions"), TestCategory("ES6"), TestCategory("StringTemplate")]
        public void StringTemplateWithReplacements()
        {
            PerformCompletionRequests(@"
                var str = `string template ${'with'} ${'replacements'}`;
                ^|str|;
            ");
        }

        [TestMethod]
        [WorkItem(471759)]
        [TestCategory("Completions"), TestCategory("ES6"), TestCategory("StringTemplate")]
        public void TaggedStringTemplate()
        {
            PerformCompletionRequests(@"
                function foo() { };
                var str = foo`string template ${'with'} ${'replacements'}`;
                ^|foo,str|;
            ");
        }

        [TestMethod]
        [WorkItem(407945)]
        public void MultipleFieldAssignments()
        {
            PerformCompletionRequests(@"
                function myObject() {
                    /// <field name='myNumber' type='Number' />
                    this.myNumber = null;
                }
                myObject.prototype.AnotherFunction = function (width) {
                    this.myNumber = width;
                    this.myNumber.|toFixed|;
                }
            ");
            
            // Perform the same test, with an extra get in the beginning of the function
            // to ensure value is cached prior to the assignment to undefined.
            PerformCompletionRequests(@"
                function myObject() {
                    /// <field name='myNumber' type='Number' />
                    this.myNumber = null;
                }
                myObject.prototype.AnotherFunction = function (width) {
                    this.myNumber;
                    this.myNumber = width;
                    this.myNumber.|toFixed|;
                }
            ");
        }

//        [TestMethod]
//        [WorkItem(408600)]
//        public void CrashWhileEditingExtensions()
//        {
//            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
//            PerformHurriedCompletionRequests(@"if (!!intellisense) {
//                intellisense.addEventListener('statementcompletion', function (context) {
//                    for (var i = 0; i < context.items.length; i++) {
//                        var item = context.items[i];
//                        if(item.name[0] == '_') { 
//                            context.items.splice(i, 1); 
//                            i--;
//                        }
//                        var s = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
//                        for(var i=0; i<12; i++) {
//                            s = s.toLowerCase() + s.toLowerCase();
//                        }
//                        context.|!x|;|context|
//                        ", domjs);
//        }

        [TestMethod]
        [WorkItem(400260)]
        public void ElementDomElementAttributes()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            // With type='HTMLElement'
            PerformCompletionRequests(@"
                function f2() {
                    /// <returns type='HTMLElement' />
                }
                f2().|appendChild|;
                function f3(a) {
                    /// <param name='a' type='HTMLElement' />
                    a.|appendChild|;
                }
                function f4() {
                    /// <field name='a' type='HTMLElement' />
                    this.a.|appendChild|;
                }
                new f4();
                new f4().a.|appendChild|;
                var x = {
                    /// <field type='HTMLElement' />
                    field1: undefined
                };
                x.field1.|appendChild|;
            ", dom);

            // With domElement='true'
            PerformCompletionRequests(@"
                function f2() {
                    /// <returns domElement='true' />
                }
                f2().|appendChild|;
                function f3(a) {
                    /// <param name='a' domElement='true' />
                    a.|appendChild|;
                }
                function f4() {
                    /// <field name='a' domElement='true' />
                    this.a.|appendChild|;
                }
                new f4();
                new f4().a.|appendChild|;
                var x = {
                    /// <field domElement='true' />
                    field1: undefined
                };
                x.field1.|appendChild|;
            ", dom);

            // With elementType='HTMLElement'
            PerformCompletionRequests(@"
                function f2() {
                    /// <returns type='Array' elementType='HTMLElement' />
                }
                f2()[0].|appendChild|;
                function f3(a) {
                    /// <param name='a' type='Array' elementType='HTMLElement' />
                    a[0].|appendChild|;
                }
                
                function f4() {
                    /// <field name='a' type='Array' elementType='HTMLElement' />
                    this.a[0].|appendChild|;
                }
                new f4();
                new f4().a[0].|appendChild|;
                var x = {
                    /// <field type='Array' elementType='HTMLElement' />
                    field1: undefined
                };
                x.field1[0].|appendChild|;

            ", dom);

            // With elementDomElement='true'
            PerformCompletionRequests(@"
                function f2() {
                    /// <returns type='Array' elementDomElement='true' />
                }
                f2()[0].|appendChild|;
                function f3(a) {
                    /// <param name='a' type='Array' elementDomElement='true' />
                    a[0].|appendChild|;
                }
                
                function f4() {
                    /// <field name='a' type='Array' elementDomElement='true' />
                    this.a[0].|appendChild|;
                }
                new f4();
                new f4().a[0].|appendChild|;
                var x = {
                    /// <field type='Array' elementDomElement='true' />
                    field1: undefined
                };
                x.field1[0].|appendChild|;

            ", dom);

        }

        [TestMethod]
        [WorkItem(392532)]
        public void ForEachOnTaggedUndefined()
        {
            // On a field defined in a constructor and is assigned an empty array 
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion', function() {});
                function DragScroll() {
                    /// <field name='scrollZones' type='Array' elementType='Number'></field>
                    /// <field name='scrollZones1' type='Array' elementType='Number'></field>
                    /// <field name='scrollZones2' type='Array' elementType='Number'></field>
                    /// <field name='scrollZones3' type='Array' elementType='Number'></field>
                    /// <field name='scrollZones5' type='Array' elementType='Number'></field>

                    this.scrollZones = [];
                    this.scrollZones1 = [ undefined ];
                    this.scrollZones2 = [ null ];
                    this.scrollZones4 = [ '' ];
                    this.scrollZones5 = [ '' ];
                }

                DragScroll.prototype.pointerEvent = function (evt) {
                    this.scrollZones.forEach(function(zone) {
                        zone.|toFixed|;
                    });
                    this.scrollZones.forEach(function(zone) {
                        this.|num|;
                    }, { num: 0 } /* thisArg */);
                    // Verify that the patched forEach behaves correctly. 
                    // Push an item into the array and verify that the change takes effect.
                    this.scrollZones.push('');
                    this.scrollZones.forEach(function (zone) {
                        zone.|concat|;
                    });
                    this.scrollZones1.forEach(function (zone) {
                        zone.|!toFixed|;
                    });
                    this.scrollZones2.forEach(function (zone) {
                        zone.|!toFixed|;
                    });
                    this.scrollZones3.forEach(function (zone) {
                        zone.|toFixed|;
                    });
                    this.scrollZones4.forEach(function (zone) {
                        zone.|concat|;
                    });
                    this.scrollZones5.forEach(function (zone) {
                        zone.|concat|;
                    });
                };
            ");

            // On a field defined in a constructor function
            PerformCompletionRequests(@"
                var WW = { ScrollZone: function () { return { num: '' } } };
                function DragScroll() {
                    /// <field name='scrollZones' type='Array' elementType='WW.ScrollZone'>Current set of ScrollZones.</field>
                    var a = this.scrollZones;
                    a.forEach(function(item) { item.|num|; });
                }
                new DragScroll();
            ");

            // On global <var>
            PerformCompletionRequests(@"
                var WW = { ScrollZone: function () { return { num: '' } } };
                /// <var type='Array' elementType='WW.ScrollZone'></var>
                var arrayField;
                arrayField.forEach(function(item) { item.|num|; });
            ");

            // On local <var>
            PerformCompletionRequests(@"
                var WW = { ScrollZone: function () { return { num: '' } } };
                function f() {
                    /// <var type='Array' elementType='WW.ScrollZone'></var>
                    var arrayField;
                    arrayField.forEach(function(item) { item.|num|; });
                }
            ");

            // On <param>
            PerformCompletionRequests(@"
                var WW = { ScrollZone: function () { return { num: '' } } };
                function f(arrayArg) {
                    /// <param name='arrayArg' type='Array' elementType='WW.ScrollZone'></param>
                    arrayArg.forEach(function(item) { item.|num|; });
                }
            ");
        }

        [TestMethod]
        public void UnclosedBlockComment()
        {
            ValidateNoCompletion("var a; /*unclosed comment|");
            ValidateNoCompletion("var a; /*unclosed| comment");
            ValidateNoCompletion("var a; /*|unclosed comment");
            ValidateNoCompletion("var a; /*|");
            ValidateHasCompletions("|window|/*comment*/|window|");
        }

        [TestMethod]
        public void JQueryFormElements()
        {
            // Verifies form.elements, select.options, and jQuery .val()
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var jquery = JQueryTestFiles.jquery_1_7;
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                var form = document.createElement('form');
                form.name = 'form1';
                
                //
                // Verify input[type=text]
                //
                var input = document.createElement('input');
                input.type = 'text';
                input.value = 'testvalue';
                form.appendChild(input);
                intellisense.logMessage('>>> input[type=text].val():' + $(input).val());

                //
                // Verify input[type=checkbox]
                //
                var checkbox = document.createElement('checkbox');
                checkbox.value = 'testvalue';
                form.appendChild(checkbox);
                intellisense.logMessage('>>> input[type=checkbox].val():' + $(input).val());

                //
                // Verify textarea
                //
                var textarea = document.createElement('textarea');
                textarea.value = 'testvalue';
                form.appendChild(textarea);
                intellisense.logMessage('>>> textarea.val():' + $(textarea).val());

                //
                // Verify select
                //
                var select =  document.createElement('select');
                select.name = 'select1';
                select.value = 'testvalue';
                form.appendChild(select);
                
                // No options yet
                intellisense.logMessage('>>> select.val() (no options): ' + $(select).val());
                
                // Add one option (not selected yet)
                var option1 = document.createElement('option');
                option1.value = '1';
                option1.text = 'option 1';
                select.appendChild(option1);

                intellisense.logMessage('>>> select.val() (1 option, not selected): ' + $(select).val());
                
                // Now select the option
                option1.selected = 'selected';
                intellisense.logMessage('>>> select.val() (1 option, selected): ' + $(select).val());
                
                // Add another option
                var option2 = document.createElement('option');
                option2.value = '2';
                option2.text = 'option 2';
                select.appendChild(option2);
                intellisense.logMessage('>>> select.val() (2 options, 1 selected): ' + $(select).val());
                
                // Select option2
                option2.selected = 'selected';
                intellisense.logMessage('>>> select.val() (2 options, 2 selected): ' + $(select).val());
                
                // Create an optgroup
                var optgroup = document.createElement('optgroup');
                select.appendChild(optgroup);
                // Append option3 to optgroup
                var option3 = document.createElement('option');
                option3.value = '3';
                option3.text = 'option 3';
                option3.selected = 'selected';
                optgroup.appendChild(option3);
                
                intellisense.logMessage('>>> select.val() (3 options, with optgroup): ' + $(select).val());

                // Add another option after optgroup
                var option4 = document.createElement('option');
                option4.value = '4';
                option4.text = 'option 4';
                option4.selected = 'selected';
                select.appendChild(option4);

                intellisense.logMessage('>>> select.val() (with option after optgroup): ' + $(select).val());
                
                // Create a child div under the form and add a button to it.
                // We want to make sure that the button appears in form.elements collection.
                var div = document.createElement('div');
                form.appendChild(div);
                div.appendChild(document.createElement('button'));

                // Dump form.elements               
                var elements = form.elements;
                intellisense.logMessage('>>> elements.length: ' + elements.length);
                for(var i=0; i<elements.length; i++) {
                    intellisense.logMessage('>>> form element:' +  elements[i].tagName);
                }

                ;|
                ",
                (context, offset, data, index) =>
                {
                    context.GetCompletionsAt(offset);
                    EnsureMessageLogged(context, ">>> input[type=text].val():testvalue");
                    EnsureMessageLogged(context, ">>> input[type=checkbox].val():testvalue");
                    EnsureMessageLogged(context, ">>> textarea.val():testvalue");
                    EnsureMessageLogged(context, ">>> select.val() (no options): ");
                    EnsureMessageLogged(context, ">>> select.val() (1 option, not selected): ");
                    EnsureMessageLogged(context, ">>> select.val() (1 option, selected): 1");
                    EnsureMessageLogged(context, ">>> select.val() (2 options, 1 selected): 1");
                    EnsureMessageLogged(context, ">>> select.val() (2 options, 2 selected): 1,2");
                    EnsureMessageLogged(context, ">>> select.val() (3 options, with optgroup): 1,2,3");
                    EnsureMessageLogged(context, ">>> select.val() (with option after optgroup): 1,2,4,3");
                    EnsureMessageLogged(context, ">>> elements.length: 4");
                    EnsureMessageLogged(context, ">>> form element:INPUT");
                    EnsureMessageLogged(context, ">>> form element:TEXTAREA");
                    EnsureMessageLogged(context, ">>> form element:SELECT");
                    EnsureMessageLogged(context, ">>> form element:BUTTON");
                }, dom, jquery);

            // Verify intellisense on the first element of an empty form.elements 
            PerformCompletionRequests(@"
                var form = document.createElement('form');
                form.elements[0].|tagName|

                ", dom, jquery);

            // Verify intellisense on the first element of an empty select.options
            PerformCompletionRequests(@"
                var select = document.createElement('select');
                select.options[0].|value|
                ", dom, jquery);
        }

        [TestMethod]
        [WorkItem(417806)]
        public void msElementsFromPoint()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var jquery = JQueryTestFiles.jquery_1_7_2;

            PerformCompletionRequests(@"
                    var rawElementsFromPoint = document.msElementsFromPoint(0, 0);
                    var target = $(rawElementsFromPoint)[0];
                    target.|nodeName|;

                    var rawElementsFromRect = document.msElementsFromRect(0, 0, 0, 0);
                    target = $(rawElementsFromRect)[0];
                    target.|nodeName|;
            ", dom, jquery);
        }

        [TestMethod]
        public void JQuerySerializeArray()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var jquery = JQueryTestFiles.jquery_1_7;

            PerformRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });

                // Create a form
                var form = document.createElement('form');
                form.name = 'form1';
                
                // Create a child div under the form
                var div = document.createElement('div');
                form.appendChild(div);
                
                // Create input element under the div
                var input = document.createElement('input');
                   input.name = 'input1';
                   input.value = 'testvalue';
                   input.type = 'text'; 
                   div.appendChild(input);

                // Create select element under the div
                var select =  document.createElement('select');
                select.name = 'select1';
                select.type = 'select';
                select.value = 'testvalue';
                div.appendChild(select);

                // Create a selected option under select
                var option = document.createElement('option');
                option.value = '1';
                option.selected = 'selected';
                option.text = 'option 1';
                select.appendChild(option);
                
                // Create a fieldset - will not be included
                div.appendChild(document.createElement('fieldset'));
                
                // Another input directly under the form
                var input2 = document.createElement('input');
                input2.name = 'input2';
                input2.type = 'text';
                input2.value = 'v';
                form.appendChild(input2); 
                
                var fields = $(form).serializeArray();
                intellisense.logMessage('>>> fields.length: ' + fields.length);
                for(var i=0; i<fields.length; i++) {
                    intellisense.logMessage('>>> fields: name:' +  fields[i].name + ' value:' + fields[i].value);
                }
                ;|; 
            ",
            (context, offset, data, index) =>
            {
                context.GetCompletionsAt(offset);
                EnsureMessageLogged(context, ">>> fields.length: 3");
                EnsureMessageLogged(context, ">>> fields: name:input2 value:v");
                EnsureMessageLogged(context, ">>> fields: name:input1 value:testvalue");
                EnsureMessageLogged(context, ">>> fields: name:select1 value:1");
            },
             dom, jquery);
        }

        [TestMethod]
        [WorkItem(362444)]
        public void GetElementById()
        {
            var siteTypes = _session.ReadFile(Paths.SiteTypesWindowsPath).Text;
            var dom = _session.ReadFile(Paths.DomWindowsPath).Text;
            var generatedElemnts = @"
                (function(window){
                    // ----- GENERATED ELEMENTS FROM REFERENCED HTML DOCUMENTS ------
                    window.document._$documentElements = {
                        ""myCanvas"" : window.document.createElement(""canvas"")
                    };
                    window.myCanvas = window.document._$documentElements[""myCanvas""];
                })(window);";


            PerformRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                intellisense.logMessage('> id of canvas element = ' + document.getElementById('myCanvas').id);
                intellisense.logMessage('> id of missing element = ' +document.getElementById('missing').id);

                's'.|String|;
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 completions.ToEnumerable().ExpectContains(StringMethods);

                 EnsureMessageLogged(context, "> id of canvas element = myCanvas");
                 EnsureMessageLogged(context, "> id of missing element = missing");
             }, siteTypes, dom, generatedElemnts);
        }

        [TestMethod]
        [WorkItem(362444)]
        public void JQueryElementIdSelector()
        {
            var siteTypes = _session.ReadFile(Paths.SiteTypesWindowsPath).Text;
            var dom = _session.ReadFile(Paths.DomWindowsPath).Text;
            var jquery = Paths.LatestJQueryText;
            var generatedElemnts = @"
                (function(window){
                    // ----- GENERATED ELEMENTS FROM REFERENCED HTML DOCUMENTS ------
                    window.document._$documentElements = {
                        ""myCanvas"" : window.document.createElement(""canvas"")
                    };
                    window.myCanvas = window.document._$documentElements[""myCanvas""];
                })(window);";

            PerformCompletionRequests("$('#myCanvas')[0].|getContext,width,height|", siteTypes, dom, jquery, generatedElemnts);
        }

        [TestMethod]
        [WorkItem(363838)]
        public void WinJsElementIdSelector()
        {
            var siteTypes = _session.ReadFile(Paths.SiteTypesWindowsPath).Text;
            var dom = _session.ReadFile(Paths.DomWindowsPath).Text;
            var baseJs = WinJSTestFiles.latest_base;
            var generatedElemnts = @"
                (function(window){
                    // ----- GENERATED ELEMENTS FROM REFERENCED HTML DOCUMENTS ------
                    window.document._$documentElements = {
                        ""myCanvas"" : window.document.createElement(""canvas"")
                    };
                    window.myCanvas = window.document._$documentElements[""myCanvas""];
                })(window);";

            PerformCompletionRequests(@"WinJS.Utilities.query('#myCanvas')[0].|getContext,width,height|", siteTypes, dom, baseJs, generatedElemnts);
        }

        [TestMethod]
        [WorkItem(324848)]
        [Ignore] // Timing dependent test is not reliable.
        public void ManyExceptions()
        {
            DateTime start = DateTime.Now;
            try
            {
                PerformCompletionRequests(@"
                function functionA(counter, freq) {
                    var o = 1;
                    o.prototype.toString = function () { };
                    if (|counter|counter % freq == 0) { 
                        return functionA(counter + 1, freq);
                    }
                }
                functionA(0, 1);
            ");
            }
            finally
            {
                var testTime = (DateTime.Now - start).TotalMilliseconds;
                System.Diagnostics.Trace.WriteLine("Test time: " + testTime);
                // Supposed to take about 550ms in chk, 150ms in ret.
                // Was over 4500ms in before the fix in diagProbe.cpp.
                // Was 450ms in ret before the stack size for LS execution was reduced in ThreadContext.cpp.
                Assert.IsTrue(testTime < 2500);
            }
        }

        [TestMethod]
        public void Param_ValueAttribute()
        {
            // Implicit signature, various values and types 
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                function f(a,b,c) {
                    /// <param name='a' value='0'/>
                    /// <param name='b' value='1'/>
                    /// <param name='c' value='{ msg: ""hi"" }'/>
                    intellisense.logMessage('>>> a: ' + a);
                    intellisense.logMessage('>>> b: ' + b);
                    intellisense.logMessage('>>> c.msg: ' + c.msg);
                    a.|toFixed|;
                    b.|toFixed|;
                    c.|msg|;
                }
            ");

            // Returning a forced param value
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                function f(a) {
                    /// <param name='a' value='1'/>
                    return a;
                }
                var a = f();
                intellisense.logMessage('>>> a: ' + a);
                a.|toFixed|;
            ");

            // Using a forced param value in a callback
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                function f(a, callback) {
                    /// <param name='a' value='{ xml: ""result""}'/>
                    callback(a);
                }
                f(1, function(result) { 
                    intellisense.logMessage('>>> result.xml: ' + result.xml);
                    result.|xml|;
                });
            ");

            // Param without a name attribute (should be ignored). 
            PerformCompletionRequests(@"
                function f(a) {
                    /// <param name='a' value='1'/>
                    /// <param value='2'/>
                    a.|toFixed|;
                }
            ");

            // With explicit <signature>
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                function f(a,b,c) {
                    /// <signature>
                    ///     <param name='a' value='0'/>
                    /// </signature>
                    intellisense.logMessage('>>> a: ' + a);
                    a.|toFixed|;
                }
            ");

            // With implicit & explicit <signature>
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                function f(a,b,c) {
                    ///  <param name='a' value='1'/>
                    /// <signature>
                    ///     <param name='a' value='1'/>
                    /// </signature>
                    intellisense.logMessage('>>> a: ' + a);
                    a.|toFixed|;
                }
            ");
        }

        [TestMethod]
        [WorkItem(246249)]
        public void DataTransfer_Files()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests(@"
                var drop = document.getElementById('drop');
                drop.addEventListener('drop', function (e) {
                    e.preventDefault(); 
                    var fileList = e.dataTransfer.|files|
            ", domjs);
        }

        [TestMethod]
        [WorkItem(251279)]
        public void ArrayTypeAttribute()
        {
            // Expression in <returns elementType=''>
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                var Win = { File: function() { this.open = function() { }; } };
                function openFiles() {
                    /// <returns type='Array' elementType='Win.File' />
                }
                var r = openFiles();
                intellisense.logMessage('>>> r:' + r + ' r[0]:' + r[0]); 
                r[0].|open|;
            ");

            // Non-typed array
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                function f() {
                    /// <returns type='Array' elementType='Number'></return> 
                }
                intellisense.logMessage('>>> r:' + r + ' r[0]:' + r[0]); 
                var r = f();
                r[0].|toFixed|;
            ");

            // Expression in <param elementType=''>
            PerformCompletionRequests(@"
                var Win = { File: function() { this.open = function() { }; } };
                function openFiles(f1, f2, f3) {
                    /// <param name='f1' type='Array' elementType = 'Win.File' />
                    f1[0].|open|;
                }
            ");

            // Typed arrays
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                function testFoo(i8,i16,i32,ui8,u8c,ui16,ui32,f32,f64,nontyped) { 
                /// <param name='i8' type='Int8Array' elementType='Number' >param a</param> 
                /// <param name='i16' type='Int16Array' elementType='Number' >param a</param> 
                /// <param name='i32' type='Int32Array' elementType='String' >param a</param> 
                /// <param name='ui8' type='Uint8Array' elementType='Number' >param a</param> 
                /// <param name='u8c' type='Uint8ClampedArray' elementType='Number' >param a</param> 
                /// <param name='ui16' type='Uint16Array' elementType='Number' >param a</param> 
                /// <param name='ui32' type='Uint32Array' elementType='String' >param a</param> 
                /// <param name='f32' type='Float32Array' elementType='Number' >param a</param> 
                /// <param name='f64' type='Float64Array' >param a</param> 
                /// <param name='nontyped' type='Array' elementType='Number' >param a</param> 
                /// <returns type='Int8Array'>return document type</return> 
                    intellisense.logMessage('>>> i8:' + i8); 
                    intellisense.logMessage('>>> i8[0]:' + i8[0]); 
                    intellisense.logMessage('>>> i16[0]:' + i16[0]); 
                    intellisense.logMessage('>>> i32[0]:' + i32[0]); 
                    intellisense.logMessage('>>> ui8[0]:' + ui8[0]); 
                    intellisense.logMessage('>>> u8c[0]:' + u8c[0]); 
                    intellisense.logMessage('>>> ui16[0]:' + ui16[0]); 
                    intellisense.logMessage('>>> ui32[0]:' + ui32[0]); 
                    intellisense.logMessage('>>> f32[0]:' + f32[0]); 
                    intellisense.logMessage('>>> f64[0]:' + f64[0]); 
                    intellisense.logMessage('>>> nontyped[0]:' + nontyped[0]); 
                    i8[0].|toFixed|;
                    i16[0].|toFixed|;
                    i32[0].|toFixed|;
                    ui8[0].|toFixed|;
                    u8c[0].|toFixed|;
                    ui16[0].|toFixed|;
                    ui32[0].|toFixed|;
                    f32[0].|toFixed|;
                    f64[0].|toFixed|;
                    nontyped[0].|toFixed|;
                    i8.|set|;
                }
                // Verify return value
                var r = testFoo();
                intellisense.logMessage('>>> r:' + r + ' r[0]:' + r[0]); 
                r[0].|toFixed|;
            ");
        }

        [TestMethod]
        public void DebugObject()
        {
            var sitetypes = _session.ReadFile(Paths.SiteTypesWebPath).Text;
            PerformCompletionRequests(@"Debug.|write,writeln|", sitetypes);
        }

        [TestMethod]
        public void ExpressionInTypeAttribute()
        {
            // <param type=''>
            PerformCompletionRequests(@"
                var Win = { File: function() { this.open = function() { }; } };
                function openFiles(f1, f2, f3) {
                    /// <param name='f1' type = 'Win.File' />
                    /// <param name='f2' type = 'Win.File' />
                    /// <param name='f3' type = 'Win.File' />
                    f1.|open|;
                    f2.|open|;
                    f3.|open|;
                }
            ");

            // <returns type=''>
            PerformCompletionRequests(@"
                var Win = { File: function() { this.open = function() { }; } };
                function openFile(name) {
                    /// <returns type='Win.File' />
                }
                openFile('x').|open|
            ");

            PerformCompletionRequests(@"
                var Win = { File: function() { this.open = function() { }; } };
                function openFile(name) {
                    /// <returns value='new Win.File()' />
                }
                openFile('x').|open|
            ");
        }

        [TestMethod]
        public void ParentNode()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                var x;
                (function() {
                    function up(node) {
                        intellisense.logMessage('>>> up, node:' + node); 
                        if(node) {
                           up(node.parentNode);
                        } else {
                            x = { a: 1 };
                        }
                    }
                    up(document);
                })();
                x.|a|;      
            ", domjs);
        }

        [TestMethod]
        [WorkItem(210498)]
        public void DocCommentsForNonExistentFields()
        {
            PerformCompletionRequests(@"
                //
                // Verify the value attribute
                //
                intellisense.addEventListener('statementcompletion',  function() { });
                function F1() {
                    /// <field name='a' type='String' value='{ x: 1 }'></field>
                    /// <field name='b' type='String' value='2'></field>
                    /// <field name='c' value='2'></field>
                    intellisense.logMessage('>>> this.a: ' + this.a); 
                    intellisense.logMessage('>>> this.b: ' + this.b); 
                    this.a.|x|;
                    this.b.|toFixed|;
                    this.c = '';
                    this.c.|charAt|;
                }
                var f1 = new F1();
                f1.a.|x|;
                f1.c.|toFixed|;
            ");

            PerformCompletionRequests(@"
                //
                // Verify the type attribute is ignored and the actual type is used
                //
                intellisense.addEventListener('statementcompletion',  function() { });
                function F1() {
                    /// <field name='a' type='String'></field>
                    /// <field name='b' type='Number'></field>
                    intellisense.logMessage('>>> this.a: ' + this.a); 
                    intellisense.logMessage('>>> this.b: ' + this.b); 
                    this.a.|charAt|;
                }
            ");

            PerformCompletionRequests(@"
                //
                // Verify the type attribute is ignored and the actual type is used
                //
                intellisense.addEventListener('statementcompletion',  function() { });
                function F1() {
                    /// <field name='a' type='String'></field>
                    /// <field name='b' type='Number'></field>
                }
                var f1 = new F1();
                intellisense.logMessage('>>> f1.a: ' + f1.a); 
                intellisense.logMessage('>>> f1.b: ' + f1.b); 
                f1.a.|charAt|;
            ");

            PerformCompletionRequests(@"
                //
                // Verify unset fields appear in completion    
                //
                function F() {
                    /// <field name='a'></field>
                    /// <field name='b'></field>
                    /// <field name='c'></field>
                    this.c = 1;
                }

                var f = new F();
                f.|a,b,c|;
                f.c.|toFixed|;

                //
                // Verify that a type attribute is ignored if a field is initialized
                //
                function F1() {
                    /// <field name='c' type='String'></field>
                    /// <field name='a' type='String'></field>
                    this.c = 1;
                }
                var f1 = new F1();
                f1.|a,c|;
                f1.a.|charAt|;
                f1.c.|toFixed|;

                //
                // Verify prototype fields are not masked/affected    
                //
                function F2() {
                    /// <field name='b'></field>
                    /// <field name='a'></field>
                    this.x = 5;
                    this.y = 5;
                }
                F2.prototype.a = 0;
                var f2 = new F2();
                f2.a.|toFixed|;
            ");
        }


        [TestMethod]
        [WorkItem(183346)]
        [WorkItem(196279)]
        public void ArrayMethodsReturnValueCompletion()
        {
            PerformCompletionRequests(
            @"
                var a = [1, 2].map(function () { return 1; });
                a.|length|;
                var b = [1, 2].filter(function () { return true; });
                b.|length|;
                var c = [1, 2].reduce(function () { return new Date(); });
                c.|getDay|;
                var total = [0, 1, 2, 3].reduce(function(a, b){ return a + b; }); 
                total.|toFixed|
            ");
        }

        [TestMethod]
        [WorkItem(179976)]
        [WorkItem(199963)]
        public void DomGlobalFunctions()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            PerformRequests("|", (context, offset, data, index) =>
            {
                context.GetCompletionsAt(offset).ToEnumerable().ExpectContains("XMLHttpRequest");
            }, domjs);
        }

        [TestMethod]

        [WorkItem(165980)]
        [WorkItem(204396)]
        public void DomConstants()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            PerformRequests(@"document.|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                completions.ToEnumerable().ExpectContains(new[] { "ATTRIBUTE_NODE", "CDATA_SECTION_NODE", "ELEMENT_NODE" });
            }, dom);

            PerformRequests(@"window.NodeFilter.|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                completions.ToEnumerable().ExpectContains(new[] { "SHOW_NOTATION", "SHOW_ENTITY_REFERENCE", "SHOW_ELEMENT" });
            }, dom);
        }

        [TestMethod]
        [WorkItem(194861)]
        public void HideNonFunctionalDomProperties()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            PerformRequests(@"document.|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                completions.ToEnumerable().ExpectNotContains(new[] { "Script", "fileCreatedDate", "fileModifiedDate", "fileSize", "mimeType " });
            }, dom);
        }

        [TestMethod]
        public void Glyph()
        {
            PerformCompletionRequests(
                @"
                    intellisense.addEventListener('statementcompletion',  function(e) {
                        e.items.forEach(function (item) { 
                            if(item.name=='name1' || item.name=='name2')
                                item.glyph='myglyph'; 
                        });
                    });
                    var x = { 
                        /// <field type='Foo'>descr</field>
                        name1: 'foo', 
                        name2: 'foo', 
                        value: undefined };
                    x.|
                ", (completions, data, i) =>
                 {
                     completions.Item("name1").Glyph.Except("myglyph");
                     completions.Item("name2").Glyph.Except("myglyph");
                     Assert.IsNull(completions.Item("value").Glyph);
                 });
        }

        [TestMethod]
        public void VerifyJQuery151()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            var jQuery = JQueryTestFiles.jquery_1_5_1;
            PerformRequests("$.| ; $().|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNotNull(completions);
                var result = completions.ToEnumerable();
                result.ExpectContains(new[] { "each" });
            }, domjs, jQuery);
        }

        [TestMethod]
        public void VerifyJQuery161Doc()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            var jQuery = JQueryTestFiles.jquery_1_6_1_vsdoc;
            PerformRequests("$.|static|; $().|instance|; $('#abc').|instance|;", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNotNull(completions);
                var result = completions.ToEnumerable();
                switch (data)
                {
                    case "static": result.ExpectContains("ajax", "css", "data"); break;
                    case "instance": result.ExpectContains("add", "addClass", "after", "ajaxComplete"); break;
                    default: Assert.Fail("Check the test string"); break;
                }
            }, domjs, jQuery);
        }

        [TestMethod]
        public void VerifyMooTools()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            var mootools = TestFiles.mootools_core_1_3_2_full_compat;

            PerformRequests("$('test').|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNotNull(completions);

                // The above returns a completion list for the null object which is technically correct since it couldn't find 
                // the 'test' identifier, the above expression return JavaScript null.

                // The domWeb.js support needs to be improved or a vsextension needs to be written to allow the language service
                // to provide better intellisense for mootools. With the domWeb.js fix or the vsextension the test below should 
                // pass.

                //var result = completions.ToEnumerable();
                //result.ExpectContains("set", "get");
            }, domjs, mootools);
        }

        [TestMethod]
        public void VerifyDomCallbacks()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            PerformRequests(@"

    window.document.addEventListener('click', function (e) {
      e.|MouseEvent|;
      window.document.addEventListener('click', function (e) {
        e.|MouseEvent|;
      });
    });", (context, offset, data, index) =>
        {
            var completions = context.GetCompletionsAt(offset);
            Assert.IsNotNull(completions);
            var result = completions.ToEnumerable();
            switch (data)
            {
                case "MouseEvent": result.ExpectContains(new[] { "pageX", "offsetY", "x", "y", "altKey", "metaKey", "ctrlKey", "offsetX", "screenX", "clientY", "shiftKey", "screenY", "relatedTarget", "button", "pageY", "buttons", "clientX" }); break;
            }
        }, domjs);
        }
        [TestMethod]
        public void Bug185948()
        {
            PerformCompletionRequests(
                @"
                    var a = 'abc';
                    a.|split|
                ");
        }

        [TestMethod]
        public void ReturnValues()
        {
            PerformCompletionRequests(
                @"
                    function f1(v) {
                        /// <returns value='v' />
                        return '';
                    });
                    f1(1).|toFixed|;
                ");

            PerformCompletionRequests(
                @"
                    function f1(v) {
                        /// <returns value='{ a: 1, b: 2 }' />
                        return v;
                    });
                    f1('').|a,b|;
                    f1('').a.|toFixed|;
                ");

            PerformCompletionRequests(
                @"
                    function f1(v) {
                        /// <signature>
                        ///     <returns value='1' />
                        /// </signature>
                        return v;
                    });
                    f1('').|toFixed|;
                ");

            PerformCompletionRequests(
                @"
                    function f1(v) {
                        /// <returns value='1' />
                        return v;
                    });
                    f1('').|toFixed|;
                ");

            PerformRequests(@"f().|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset).ToEnumerable();
                completions.ExpectContains(new string[] { "toFixed" });
            },
            @"
                intellisense.addEventListener('statementcompletion',  function () {});

                function f2(val)
                {
                    /// <returns value='val' />
                    return val;
                });

                function f() 
                {
                    /// <returns value='0' />
                    return 'should not see this';
                });

                function f1(v) 
                {
                    /// <returns value='1' />
                    return v;
                });

                intellisense.logMessage('f2() value:' + f2('hi'));
                intellisense.logMessage('f() value:' + f());
                intellisense.logMessage('f1() value:' + f1(2));
                ");
        }

        [TestMethod]
        public void Bug135826_Completions()
        {
            #region Code

            var code = @"
/*!
 * jQuery JavaScript Library v1.4.4
 * http://jquery.com/
 *
 * Copyright 2010, John Resig
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 * Includes Sizzle.js
 * http://sizzlejs.com/
 * Copyright 2010, The Dojo Foundation
 * Released under the MIT, BSD, and GPL Licenses.
 *
 * Date: Thu Nov 11 19:04:53 2010 -0500
 */
(function( window, undefined ) {

// Use the correct document accordingly with window argument (sandbox)
var document = window.document;
var jQuery = (function() {

// Define a local copy of jQuery
var jQuery = function( selector, context ) {
		// The jQuery object is actually just the init constructor 'enhanced'
		return new jQuery.fn.init( selector, context );
	},

	// Map over jQuery in case of overwrite
	_jQuery = window.jQuery,

	// Map over the $ in case of overwrite
	_$ = window.$,

	// A central reference to the root jQuery(document)
	rootjQuery,

	// A simple way to check for HTML strings or ID strings
	// (both of which we optimize for)
	quickExpr = /^(?:[^<]*(<[\w\W]+>)[^>]*$|#([\w\-]+)$)/,

	// Is it a simple selector
	isSimple = /^.[^:#\[\.,]*$/,

	// Check if a string has a non-whitespace character in it
	rnotwhite = /\S/,
	rwhite = /\s/,

	// Used for trimming whitespace
	trimLeft = /^\s+/,
	trimRight = /\s+$/,

	// Check for non-word characters
	rnonword = /\W/,

	// Check for digits
	rdigit = /\d/,

	// Match a standalone tag
	rsingleTag = /^<(\w+)\s*\/?>(?:<\/\1>)?$/,

	// JSON RegExp
	rvalidchars = /^[\],:{}\s]*$/,
	rvalidescape = /\\(?:[""\\\/bfnrt]|u[0-9a-fA-F]{4})/g,
	rvalidtokens = /""[^""\\\n\r]*""|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g,
	rvalidbraces = /(?:^|:|,)(?:\s*\[)+/g,

	// Useragent RegExp
	rwebkit = /(webkit)[ \/]([\w.]+)/,
	ropera = /(opera)(?:.*version)?[ \/]([\w.]+)/,
	rmsie = /(msie) ([\w.]+)/,
	rmozilla = /(mozilla)(?:.*? rv:([\w.]+))?/,

	// Keep a UserAgent string for use with jQuery.browser
	userAgent = navigator.userAgent,

	// For matching the engine and version of the browser
	browserMatch,
	
	// Has the ready events already been bound?
	readyBound = false,
	
	// The functions to execute on DOM ready
	readyList = [],

	// The ready event handler
	DOMContentLoaded,

	// Save a reference to some core methods
	toString = Object.prototype.toString,
	hasOwn = Object.prototype.hasOwnProperty,
	push = Array.prototype.push,
	slice = Array.prototype.slice,
	trim = String.prototype.trim,
	indexOf = Array.prototype.indexOf,
	
	// [[Class]] -> type pairs
	class2type = {};

jQuery.fn = jQuery.prototype = {
	init: function( selector, context ) {
		var match, elem, ret, doc;

		// Handle $(""""), $(null), or $(undefined)
		if ( !selector ) {
			return this;
		}

		// Handle $(DOMElement)
		if ( selector.nodeType ) {
			this.context = this[0] = selector;
			this.length = 1;
			return this;
		}
		
		// The body element only exists once, optimize finding it
		if ( selector === ""body"" && !context && document.body ) {
			this.context = document;
			this[0] = document.body;
			this.selector = ""body"";
			this.length = 1;
			return this;
		}

		// Handle HTML strings
		if ( typeof selector === ""string"" ) {
			// Are we dealing with HTML string or an ID?
			match = quickExpr.exec( selector );

			// Verify a match, and that no context was specified for #id
			if ( match && (match[1] || !context) ) {

				// HANDLE: $(html) -> $(array)
				if ( match[1] ) {
					doc = (context ? context.ownerDocument || context : document);

					// If a single string is passed in and it's a single tag
					// just do a createElement and skip the rest
					ret = rsingleTag.exec( selector );

					if ( ret ) {
						if ( jQuery.isPlainObject( context ) ) {
							selector = [ document.createElement( ret[1] ) ];
							jQuery.fn.attr.call( selector, context, true );

						} else {
							selector = [ doc.createElement( ret[1] ) ];
						}

					} else {
						ret = jQuery.buildFragment( [ match[1] ], [ doc ] );
						selector = (ret.cacheable ? ret.fragment.cloneNode(true) : ret.fragment).childNodes;
					}
					
					return jQuery.merge( this, selector );
					
				// HANDLE: $(""#id"")
				} else {
					elem = document.getElementById( match[2] );

					// Check parentNode to catch when Blackberry 4.6 returns
					// nodes that are no longer in the document #6963
					if ( elem && elem.parentNode ) {
						// Handle the case where IE and Opera return items
						// by name instead of ID
						if ( elem.id !== match[2] ) {
							return rootjQuery.find( selector );
						}

						// Otherwise, we inject the element directly into the jQuery object
						this.length = 1;
						this[0] = elem;
					}

					this.context = document;
					this.selector = selector;
					return this;
				}

			// HANDLE: $(""TAG"")
			} else if ( !context && !rnonword.test( selector ) ) {
				this.selector = selector;
				this.context = document;
				selector = document.getElementsByTagName( selector );
				return jQuery.merge( this, selector );

			// HANDLE: $(expr, $(...))
			} else if ( !context || context.jquery ) {
				return (context || rootjQuery).find( selector );

			// HANDLE: $(expr, context)
			// (which is just equivalent to: $(context).find(expr)
			} else {
				return jQuery( context ).find( selector );
			}

		// HANDLE: $(function)
		// Shortcut for document ready
		} else if ( jQuery.isFunction( selector ) ) {
			return rootjQuery.ready( selector );
		}

		if (selector.selector !== undefined) {
			this.selector = selector.selector;
			this.context = selector.context;
		}

		return jQuery.makeArray( selector, this );
	},

	// Start with an empty selector
	selector: """",

	// The current version of jQuery being used
	jquery: ""1.4.4"",

	// The default length of a jQuery object is 0
	length: 0,

	// The number of elements contained in the matched element set
	size: function() {
		return this.length;
	},

	toArray: function() {
		return slice.call( this, 0 );
	},

	// Get the Nth element in the matched element set OR
	// Get the whole matched element set as a clean array
	get: function( num ) {
		return num == null ?

			// Return a 'clean' array
			this.toArray() :

			// Return just the object
			( num < 0 ? this.slice(num)[ 0 ] : this[ num ] );
	},

	// Take an array of elements and push it onto the stack
	// (returning the new matched element set)
	pushStack: function( elems, name, selector ) {
		// Build a new jQuery matched element set
		var ret = jQuery();

		if ( jQuery.isArray( elems ) ) {
			push.apply( ret, elems );
		
		} else {
			jQuery.merge( ret, elems );
		}

		// Add the old object onto the stack (as a reference)
		ret.prevObject = this;

		ret.context = this.context;

		if ( name === ""find"" ) {
			ret.selector = this.selector + (this.selector ? "" "" : """") + selector;
		} else if ( name ) {
			ret.selector = this.selector + ""."" + name + ""("" + selector + "")"";
		}

		// Return the newly-formed element set
		return ret;
	},

	// Execute a callback for every element in the matched set.
	// (You can seed the arguments with an array of args, but this is
	// only used internally.)
	each: function( callback, args ) {
		return jQuery.each( this, callback, args );
	},
	
	ready: function( fn ) {
		// Attach the listeners
		jQuery.bindReady();

		// If the DOM is already ready
		if ( jQuery.isReady ) {
			// Execute the function immediately
			fn.call( document, jQuery );

		// Otherwise, remember the function for later
		} else if ( readyList ) {
			// Add the function to the wait list
			readyList.push( fn );
		}

		return this;
	},
	
	eq: function( i ) {
		return i === -1 ?
			this.slice( i ) :
			this.slice( i, +i + 1 );
	},

	first: function() {
		return this.eq( 0 );
	},

	last: function() {
		return this.eq( -1 );
	},

	slice: function() {
		return this.pushStack( slice.apply( this, arguments ),
			""slice"", slice.call(arguments).join("","") );
	},

	map: function( callback ) {
		return this.pushStack( jQuery.map(this, function( elem, i ) {
			return callback.call( elem, i, elem );
		}));
	},
	
	end: function() {
		return this.prevObject || jQuery(null);
	},

	// For internal use only.
	// Behaves like an Array's method, not like a jQuery method.
	push: push,
	sort: [].sort,
	splice: [].splice
};

// Give the init function the jQuery prototype for later instantiation
jQuery.fn.init.prototype = jQuery.fn;

jQuery.extend = jQuery.fn.extend = function() {
	 var options, name, src, copy, copyIsArray, clone,
		target = arguments[0] || {},
		i = 1,
		length = arguments.length,
		deep = false;

	// Handle a deep copy situation
	if ( typeof target === ""boolean"" ) {
		deep = target;
		target = arguments[1] || {};
		// skip the boolean and the target
		i = 2;
	}

	// Handle case when target is a string or something (possible in deep copy)
	if ( typeof target !== ""object"" && !jQuery.isFunction(target) ) {
		target = {};
	}

	// extend jQuery itself if only one argument is passed
	if ( length === i ) {
		target = this;
		--i;
	}

	for ( ; i < length; i++ ) {
		// Only deal with non-null/undefined values
		if ( (options = arguments[ i ]) != null ) {
			// Extend the base object
			for ( name in options ) {
				src = target[ name ];
				copy = options[ name ];

				// Prevent never-ending loop
				if ( target === copy ) {
					continue;
				}

				// Recurse if we're merging plain objects or arrays
				if ( deep && copy && ( jQuery.isPlainObject(copy) || (copyIsArray = jQuery.isArray(copy)) ) ) {
					if ( copyIsArray ) {
						copyIsArray = false;
						clone = src && jQuery.isArray(src) ? src : [];

					} else {
						clone = src && jQuery.isPlainObject(src) ? src : {};
					}

					// Never move original objects, clone them
					target[ name ] = jQuery.extend( deep, clone, copy );

				// Don't bring in undefined values
				} else if ( copy !== undefined ) {
					target[ name ] = copy;
				}
			}
		}
	}

	// Return the modified object
	return target;
};

jQuery.extend({
	noConflict: function( deep ) {
		window.$ = _$;

		if ( deep ) {
			window.jQuery = _jQuery;
		}

		return jQuery;
	},
	
	// Is the DOM ready to be used? Set to true once it occurs.
	isReady: false,

	// A counter to track how many items to wait for before
	// the ready event fires. See #6781
	readyWait: 1,
	
	// Handle when the DOM is ready
	ready: function( wait ) {
		// A third-party is pushing the ready event forwards
		if ( wait === true ) {
			jQuery.readyWait--;
		}

		// Make sure that the DOM is not already loaded
		if ( !jQuery.readyWait || (wait !== true && !jQuery.isReady) ) {
			// Make sure body exists, at least, in case IE gets a little overzealous (ticket #5443).
			if ( !document.body ) {
				return setTimeout( jQuery.ready, 1 );
			}

			// Remember that the DOM is ready
			jQuery.isReady = true;

			// If a normal DOM Ready event fired, decrement, and wait if need be
			if ( wait !== true && --jQuery.readyWait > 0 ) {
				return;
			}

			// If there are functions bound, to execute
			if ( readyList ) {
				// Execute all of them
				var fn,
					i = 0,
					ready = readyList;

				// Reset the list of functions
				readyList = null;

				while ( (fn = ready[ i++ ]) ) {
					fn.call( document, jQuery );
				}

				// Trigger any bound ready events
				if ( jQuery.fn.trigger ) {
					jQuery( document ).trigger( ""ready"" ).unbind( ""ready"" );
				}
			}
		}
	},
	
	bindReady: function() {
		if ( readyBound ) {
			return;
		}

		readyBound = true;

		// Catch cases where $(document).ready() is called after the
		// browser event has already occurred.
		if ( document.readyState === ""complete"" ) {
			// Handle it asynchronously to allow scripts the opportunity to delay ready
			return setTimeout( jQuery.ready, 1 );
		}

		// Mozilla, Opera and webkit nightlies currently support this event
		if ( document.addEventListener ) {
			// Use the handy event callback
			document.addEventListener( ""DOMContentLoaded"", DOMContentLoaded, false );
			
			// A fallback to window.onload, that will always work
			window.addEventListener( ""load"", jQuery.ready, false );

		// If IE event model is used
		} else if ( document.attachEvent ) {
			// ensure firing before onload,
			// maybe late but safe also for iframes
			document.attachEvent(""onreadystatechange"", DOMContentLoaded);
			
			// A fallback to window.onload, that will always work
			window.attachEvent( ""onload"", jQuery.ready );

			// If IE and not a frame
			// continually check to see if the document is ready
			var toplevel = false;

			try {
				toplevel = window.frameElement == null;
			} catch(e) {}

			if ( document.documentElement.doScroll && toplevel ) {
				doScrollCheck();
			}
		}
	},

	// See test/unit/core.js for details concerning isFunction.
	// Since version 1.3, DOM methods and functions like alert
	// aren't supported. They return false on IE (#2968).
	isFunction: function( obj ) {
		return jQuery.type(obj) === ""function"";
	},

	isArray: Array.isArray || function( obj ) {
		return jQuery.type(obj) === ""array"";
	},

	// A crude way of determining if an object is a window
	isWindow: function( obj ) {
		return obj && typeof obj === ""object"" && ""setInterval"" in obj;
	},

	isNaN: function( obj ) {
		return obj == null || !rdigit.test( obj ) || isNaN( obj );
	},

	type: function( obj ) {
		return obj == null ?
			String( obj ) :
			class2type[ toString.call(obj) ] || ""object"";
	},

	isPlainObject: function( obj ) {
		// Must be an Object.
		// Because of IE, we also have to check the presence of the constructor property.
		// Make sure that DOM nodes and window objects don't pass through, as well
		if ( !obj || jQuery.type(obj) !== ""object"" || obj.nodeType || jQuery.isWindow( obj ) ) {
			return false;
		}
		
		// Not own constructor property must be Object
		if ( obj.constructor &&
			!hasOwn.call(obj, ""constructor"") &&
			!hasOwn.call(obj.constructor.prototype, ""isPrototypeOf"") ) {
			return false;
		}
		
		// Own properties are enumerated firstly, so to speed up,
		// if last one is own, then all properties are own.
	
		var key;
		for ( key in obj ) {}
		
		return key === undefined || hasOwn.call( obj, key );
	},

	isEmptyObject: function( obj ) {
		for ( var name in obj ) {
			return false;
		}
		return true;
	},
	
	error: function( msg ) {
		throw msg;
	},
	
	parseJSON: function( data ) {
		if ( typeof data !== ""string"" || !data ) {
			return null;
		}

		// Make sure leading/trailing whitespace is removed (IE can't handle it)
		data = jQuery.trim( data );
		
		// Make sure the incoming data is actual JSON
		// Logic borrowed from http://json.org/json2.js
		if ( rvalidchars.test(data.replace(rvalidescape, ""@"")
			.replace(rvalidtokens, ""]"")
			.replace(rvalidbraces, """")) ) {

			// Try to use the native JSON parser first
			return window.JSON && window.JSON.parse ?
				window.JSON.parse( data ) :
				(new Function(""return "" + data))();

		} else {
			jQuery.error( ""Invalid JSON: "" + data );
		}
	},

	noop: function() {},

	// Evalulates a script in a global context
	globalEval: function( data ) {
		if ( data && rnotwhite.test(data) ) {
			// Inspired by code by Andrea Giammarchi
			// http://webreflection.blogspot.com/2007/08/global-scope-evaluation-and-dom.html
			var head = document.getElementsByTagName(""head"")[0] || document.documentElement,
				script = document.createElement(""script"");

			script.type = ""text/javascript"";

			if ( jQuery.support.scriptEval ) {
				script.appendChild( document.createTextNode( data ) );
			} else {
				script.text = data;
			}

			// Use insertBefore instead of appendChild to circumvent an IE6 bug.
			// This arises when a base node is used (#2709).
			head.insertBefore( script, head.firstChild );
			head.removeChild( script );
		}
	},

	nodeName: function( elem, name ) {
		return elem.nodeName && elem.nodeName.toUpperCase() === name.toUpperCase();
	},

	// args is for internal usage only
	each: function( object, callback, args ) {
		var name, i = 0,
			length = object.length,
			isObj = length === undefined || jQuery.isFunction(object);

		if ( args ) {
			if ( isObj ) {
				for ( name in object ) {
					if ( callback.apply( object[ name ], args ) === false ) {
						break;
					}
				}
			} else {
				for ( ; i < length; ) {
					if ( callback.apply( object[ i++ ], args ) === false ) {
						break;
					}
				}
			}

		// A special, fast, case for the most common use of each
		} else {
			if ( isObj ) {
				for ( name in object ) {
					if ( callback.call( object[ name ], name, object[ name ] ) === false ) {
						break;
					}
				}
			} else {
				for ( var value = object[0];
					i < length && callback.call( value, i, value ) !== false; value = object[++i] ) {}
			}
		}

		return object;
	},

	// Use native String.trim function wherever possible
	trim: trim ?
		function( text ) {
			return text == null ?
				"""" :
				trim.call( text );
		} :

		// Otherwise use our own trimming functionality
		function( text ) {
			return text == null ?
				"""" :
				text.toString().replace( trimLeft, """" ).replace( trimRight, """" );
		},

	// results is for internal usage only
	makeArray: function( array, results ) {
		var ret = results || [];

		if ( array != null ) {
			// The window, strings (and functions) also have 'length'
			// The extra typeof function check is to prevent crashes
			// in Safari 2 (See: #3039)
			// Tweaked logic slightly to handle Blackberry 4.7 RegExp issues #6930
			var type = jQuery.type(array);

			if ( array.length == null || type === ""string"" || type === ""function"" || type === ""regexp"" || jQuery.isWindow( array ) ) {
				push.call( ret, array );
			} else {
				jQuery.merge( ret, array );
			}
		}

		return ret;
	},

	inArray: function( elem, array ) {
		if ( array.indexOf ) {
			return array.indexOf( elem );
		}

		for ( var i = 0, length = array.length; i < length; i++ ) {
			if ( array[ i ] === elem ) {
				return i;
			}
		}

		return -1;
	},

	merge: function( first, second ) {
		var i = first.length,
			j = 0;

		if ( typeof second.length === ""number"" ) {
			for ( var l = second.length; j < l; j++ ) {
				first[ i++ ] = second[ j ];
			}
		
		} else {
			while ( second[j] !== undefined ) {
				first[ i++ ] = second[ j++ ];
			}
		}

		first.length = i;

		return first;
	},

	grep: function( elems, callback, inv ) {
		var ret = [], retVal;
		inv = !!inv;

		// Go through the array, only saving the items
		// that pass the validator function
		for ( var i = 0, length = elems.length; i < length; i++ ) {
			retVal = !!callback( elems[ i ], i );
			if ( inv !== retVal ) {
				ret.push( elems[ i ] );
			}
		}

		return ret;
	},

	// arg is for internal usage only
	map: function( elems, callback, arg ) {
		var ret = [], value;

		// Go through the array, translating each of the items to their
		// new value (or values).
		for ( var i = 0, length = elems.length; i < length; i++ ) {
			value = callback( elems[ i ], i, arg );

			if ( value != null ) {
				ret[ ret.length ] = value;
			}
		}

		return ret.concat.apply( [], ret );
	},

	// A global GUID counter for objects
	guid: 1,

	proxy: function( fn, proxy, thisObject ) {
		if ( arguments.length === 2 ) {
			if ( typeof proxy === ""string"" ) {
				thisObject = fn;
				fn = thisObject[ proxy ];
				proxy = undefined;

			} else if ( proxy && !jQuery.isFunction( proxy ) ) {
				thisObject = proxy;
				proxy = undefined;
			}
		}

		if ( !proxy && fn ) {
			proxy = function() {
				return fn.apply( thisObject || this, arguments );
			};
		}

		// Set the guid of unique handler to the same of original handler, so it can be removed
		if ( fn ) {
			proxy.guid = fn.guid = fn.guid || proxy.guid || jQuery.guid++;
		}

		// So proxy can be declared as an argument
		return proxy;
	},

	// Mutifunctional method to get and set values to a collection
	// The value/s can be optionally by executed if its a function
	access: function( elems, key, value, exec, fn, pass ) {
		var length = elems.length;
	
		// Setting many attributes
		if ( typeof key === ""object"" ) {
			for ( var k in key ) {
				jQuery.access( elems, k, key[k], exec, fn, value );
			}
			return elems;
		}
	
		// Setting one attribute
		if ( value !== undefined ) {
			// Optionally, function values get executed if exec is true
			exec = !pass && exec && jQuery.isFunction(value);
		
			for ( var i = 0; i < length; i++ ) {
				fn( elems[i], key, exec ? value.call( elems[i], i, fn( elems[i], key ) ) : value, pass );
			}
		
			return elems;
		}
	
		// Getting an attribute
		return length ? fn( elems[0], key ) : undefined;
	},

	now: function() {
		return (new Date()).getTime();
	},

	// Use of jQuery.browser is frowned upon.
	// More details: http://docs.jquery.com/Utilities/jQuery.browser
	uaMatch: function( ua ) {
		ua = ua.toLowerCase();

		var match = rwebkit.exec( ua ) ||
			ropera.exec( ua ) ||
			rmsie.exec( ua ) ||
			ua.indexOf(""compatible"") < 0 && rmozilla.exec( ua ) ||
			[];

		return { browser: match[1] || """", version: match[2] || ""0"" };
	},

	browser: {}
});

// Populate the class2type map
jQuery.each(""Boolean Number String Function Array Date RegExp Object"".split("" ""), function(i, name) {
	class2type[ ""[object "" + name + ""]"" ] = name.toLowerCase();
});

browserMatch = jQuery.uaMatch( userAgent );
if ( browserMatch.browser ) {
	jQuery.browser[ browserMatch.browser ] = true;
	jQuery.browser.version = browserMatch.version;
}

// Deprecated, use jQuery.browser.webkit instead
if ( jQuery.browser.webkit ) {
	jQuery.browser.safari = true;
}

if ( indexOf ) {
	jQuery.inArray = function( elem, array ) {
		return indexOf.call( array, elem );
	};
}

// Verify that \s matches non-breaking spaces
// (IE fails on this test)
if ( !rwhite.test( ""\xA0"" ) ) {
	trimLeft = /^[\s\xA0]+/;
	trimRight = /[\s\xA0]+$/;
}

// All jQuery objects should point back to these
rootjQuery = jQuery(document);

// Cleanup functions for the document ready method
if ( document.addEventListener ) {
	DOMContentLoaded = function() {
		document.removeEventListener( ""DOMContentLoaded"", DOMContentLoaded, false );
		jQuery.ready();
	};

} else if ( document.attachEvent ) {
	DOMContentLoaded = function() {
		// Make sure body exists, at least, in case IE gets a little overzealous (ticket #5443).
		if ( document.readyState === ""complete"" ) {
			document.detachEvent( ""onreadystatechange"", DOMContentLoaded );
			jQuery.ready();
		}
	};
}

// The DOM ready check for Internet Explorer
function doScrollCheck() {
	if ( jQuery.isReady ) {
		return;
	}

	try {
		// If IE is used, use the trick by Diego Perini
		// http://javascript.nwbox.com/IEContentLoaded/
		document.documentElement.doScroll(""left"");
	} catch(e) {
		setTimeout( doScrollCheck, 1 );
		return;
	}

	// and execute any waiting functions
	jQuery.ready();
}

// Expose jQuery to the global object
return (window.jQuery = window.$ = jQuery);

})();


(function() {

	jQuery.support = {};

	var root = document.documentElement,
		script = document.createElement(""script""),
		div = document.createElement(""div""),
		id = ""script"" + jQuery.now();

	div.style.display = ""none"";
	div.innerHTML = ""   <link/><table></table><a href='/a' style='color:red;float:left;opacity:.55;'>a</a><input type='checkbox'/>"";

	var all = div.getElementsByTagName(""*""),
		a = div.getElementsByTagName(""a"")[0],
		select = document.createElement(""select""),
		opt = select.appendChild( document.createElement(""option"") );

	// Can't get basic test support
	if ( !all || !all.length || !a ) {
		return;
	}

	jQuery.support = {
		// IE strips leading whitespace when .innerHTML is used
		leadingWhitespace: div.firstChild.nodeType === 3,

		// Make sure that tbody elements aren't automatically inserted
		// IE will insert them into empty tables
		tbody: !div.getElementsByTagName(""tbody"").length,

		// Make sure that link elements get serialized correctly by innerHTML
		// This requires a wrapper element in IE
		htmlSerialize: !!div.getElementsByTagName(""link"").length,

		// Get the style information from getAttribute
		// (IE uses .cssText insted)
		style: /red/.test( a.getAttribute(""style"") ),

		// Make sure that URLs aren't manipulated
		// (IE normalizes it by default)
		hrefNormalized: a.getAttribute(""href"") === ""/a"",

		// Make sure that element opacity exists
		// (IE uses filter instead)
		// Use a regex to work around a WebKit issue. See #5145
		opacity: /^0.55$/.test( a.style.opacity ),

		// Verify style float existence
		// (IE uses styleFloat instead of cssFloat)
		cssFloat: !!a.style.cssFloat,

		// Make sure that if no value is specified for a checkbox
		// that it defaults to ""on"".
		// (WebKit defaults to """" instead)
		checkOn: div.getElementsByTagName(""input"")[0].value === ""on"",

		// Make sure that a selected-by-default option has a working selected property.
		// (WebKit defaults to false instead of true, IE too, if it's in an optgroup)
		optSelected: opt.selected,

		// Will be defined later
		deleteExpando: true,
		optDisabled: false,
		checkClone: false,
		scriptEval: false,
		noCloneEvent: true,
		boxModel: null,
		inlineBlockNeedsLayout: false,
		shrinkWrapBlocks: false,
		reliableHiddenOffsets: true
	};

	// Make sure that the options inside disabled selects aren't marked as disabled
	// (WebKit marks them as diabled)
	select.disabled = true;
	jQuery.support.optDisabled = !opt.disabled;

	script.type = ""text/javascript"";
	try {
		script.appendChild( document.createTextNode( ""window."" + id + ""=1;"" ) );
	} catch(e) {}

	root.insertBefore( script, root.firstChild );

	// Make sure that the execution of code works by injecting a script
	// tag with appendChild/createTextNode
	// (IE doesn't support this, fails, and uses .text instead)
	if ( window[ id ] ) {
		jQuery.support.scriptEval = true;
		delete window[ id ];
	}

	// Test to see if it's possible to delete an expando from an element
	// Fails in Internet Explorer
	try {
		delete script.test;

	} catch(e) {
		jQuery.support.deleteExpando = false;
	}

	root.removeChild( script );

	if ( div.attachEvent && div.fireEvent ) {
		div.attachEvent(""onclick"", function click() {
			// Cloning a node shouldn't copy over any
			// bound event handlers (IE does this)
			jQuery.support.noCloneEvent = false;
			div.detachEvent(""onclick"", click);
		});
		div.cloneNode(true).fireEvent(""onclick"");
	}

	div = document.createElement(""div"");
	div.innerHTML = ""<input type='radio' name='radiotest' checked='checked'/>"";

	var fragment = document.createDocumentFragment();
	fragment.appendChild( div.firstChild );

	// WebKit doesn't clone checked state correctly in fragments
	jQuery.support.checkClone = fragment.cloneNode(true).cloneNode(true).lastChild.checked;

	// Figure out if the W3C box model works as expected
	// document.body must exist before we can do this
	jQuery(function() {
		var div = document.createElement(""div"");
		div.style.width = div.style.paddingLeft = ""1px"";

		document.body.appendChild( div );
		jQuery.boxModel = jQuery.support.boxModel = div.offsetWidth === 2;

		if ( ""zoom"" in div.style ) {
			// Check if natively block-level elements act like inline-block
			// elements when setting their display to 'inline' and giving
			// them layout
			// (IE < 8 does this)
			div.style.display = ""inline"";
			div.style.zoom = 1;
			jQuery.support.inlineBlockNeedsLayout = div.offsetWidth === 2;

			// Check if elements with layout shrink-wrap their children
			// (IE 6 does this)
			div.style.display = """";
			div.innerHTML = ""<div style='width:4px;'></div>"";
			jQuery.support.shrinkWrapBlocks = div.offsetWidth !== 2;
		}

		div.innerHTML = ""<table><tr><td style='padding:0;display:none'></td><td>t</td></tr></table>"";
		var tds = div.getElementsByTagName(""td"");

		// Check if table cells still have offsetWidth/Height when they are set
		// to display:none and there are still other visible table cells in a
		// table row; if so, offsetWidth/Height are not reliable for use when
		// determining if an element has been hidden directly using
		// display:none (it is still safe to use offsets if a parent element is
		// hidden; don safety goggles and see bug #4512 for more information).
		// (only IE 8 fails this test)
		jQuery.support.reliableHiddenOffsets = tds[0].offsetHeight === 0;

		tds[0].style.display = """";
		tds[1].style.display = ""none"";

		// Check if empty table cells still have offsetWidth/Height
		// (IE < 8 fail this test)
		jQuery.support.reliableHiddenOffsets = jQuery.support.reliableHiddenOffsets && tds[0].offsetHeight === 0;
		div.innerHTML = """";

		document.body.removeChild( div ).style.display = ""none"";
		div = tds = null;
	});

	// Technique from Juriy Zaytsev
	// http://thinkweb2.com/projects/prototype/detecting-event-support-without-browser-sniffing/
	var eventSupported = function( eventName ) {
		var el = document.createElement(""div"");
		eventName = ""on"" + eventName;

		var isSupported = (eventName in el);
		if ( !isSupported ) {
			el.setAttribute(eventName, ""return;"");
			isSupported = typeof el[eventName] === ""function"";
		}
		el = null;

		return isSupported;
	};

	jQuery.support.submitBubbles = eventSupported(""submit"");
	jQuery.support.changeBubbles = eventSupported(""change"");

	// release memory in IE
	root = script = div = all = a = null;
})();



var windowData = {},
	rbrace = /^(?:\{.*\}|\[.*\])$/;

jQuery.extend({
	cache: {},

	// Please use with caution
	uuid: 0,

	// Unique for each copy of jQuery on the page	
	expando: ""jQuery"" + jQuery.now(),

	// The following elements throw uncatchable exceptions if you
	// attempt to add expando properties to them.
	noData: {
		""embed"": true,
		// Ban all objects except for Flash (which handle expandos)
		""object"": ""clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"",
		""applet"": true
	},

	data: function( elem, name, data ) {
		if ( !jQuery.acceptData( elem ) ) {
			return;
		}

		elem = elem == window ?
			windowData :
			elem;

		var isNode = elem.nodeType,
			id = isNode ? elem[ jQuery.expando ] : null,
			cache = jQuery.cache, thisCache;

		if ( isNode && !id && typeof name === ""string"" && data === undefined ) {
			return;
		}

		// Get the data from the object directly
		if ( !isNode ) {
			cache = elem;

		// Compute a unique ID for the element
		} else if ( !id ) {
			elem[ jQuery.expando ] = id = ++jQuery.uuid;
		}

		// Avoid generating a new cache unless none exists and we
		// want to manipulate it.
		if ( typeof name === ""object"" ) {
			if ( isNode ) {
				cache[ id ] = jQuery.extend(cache[ id ], name);

			} else {
				jQuery.extend( cache, name );
			}

		} else if ( isNode && !cache[ id ] ) {
			cache[ id ] = {};
		}

		thisCache = isNode ? cache[ id ] : cache;

		// Prevent overriding the named cache with undefined values
		if ( data !== undefined ) {
			thisCache[ name ] = data;
		}

		return typeof name === ""string"" ? thisCache[ name ] : thisCache;
	},

	removeData: function( elem, name ) {
		if ( !jQuery.acceptData( elem ) ) {
			return;
		}

		elem = elem == window ?
			windowData :
			elem;

		var isNode = elem.nodeType,
			id = isNode ? elem[ jQuery.expando ] : elem,
			cache = jQuery.cache,
			thisCache = isNode ? cache[ id ] : id;

		// If we want to remove a specific section of the element's data
		if ( name ) {
			if ( thisCache ) {
				// Remove the section of cache data
				delete thisCache[ name ];

				// If we've removed all the data, remove the element's cache
				if ( isNode && jQuery.isEmptyObject(thisCache) ) {
					jQuery.removeData( elem );
				}
			}

		// Otherwise, we want to remove all of the element's data
		} else {
			if ( isNode && jQuery.support.deleteExpando ) {
				delete elem[ jQuery.expando ];

			} else if ( elem.removeAttribute ) {
				elem.removeAttribute( jQuery.expando );

			// Completely remove the data cache
			} else if ( isNode ) {
				delete cache[ id ];

			// Remove all fields from the object
			} else {
				for ( var n in elem ) {
					delete elem[ n ];
				}
			}
		}
	},

	// A method for determining if a DOM node can handle the data expando
	acceptData: function( elem ) {
		if ( elem.nodeName ) {
			var match = jQuery.noData[ elem.nodeName.toLowerCase() ];

			if ( match ) {
				return !(match === true || elem.getAttribute(""classid"") !== match);
			}
		}

		return true;
	}
});

jQuery.fn.extend({
	data: function( key, value ) {
		var data = null;

		if ( typeof key === ""undefined"" ) {
			if ( this.length ) {
				var attr = this[0].attributes, name;
				data = jQuery.data( this[0] );

				for ( var i = 0, l = attr.length; i < l; i++ ) {
					name = attr[i].name;

					if ( name.indexOf( ""data-"" ) === 0 ) {
						name = name.substr( 5 );
						dataAttr( this[0], name, data[ name ] );
					}
				}
			}

			return data;

		} else if ( typeof key === ""object"" ) {
			return this.each(function() {
				jQuery.data( this, key );
			});
		}

		var parts = key.split(""."");
		parts[1] = parts[1] ? ""."" + parts[1] : """";

		if ( value === undefined ) {
			data = this.triggerHandler(""getData"" + parts[1] + ""!"", [parts[0]]);

			// Try to fetch any internally stored data first
			if ( data === undefined && this.length ) {
				data = jQuery.data( this[0], key );
				data = dataAttr( this[0], key, data );
			}

			return data === undefined && parts[1] ?
				this.data( parts[0] ) :
				data;

		} else {
			return this.each(function() {
				var $this = jQuery( this ),
					args = [ parts[0], value ];

				$this.triggerHandler( ""setData"" + parts[1] + ""!"", args );
				jQuery.data( this, key, value );
				$this.triggerHandler( ""changeData"" + parts[1] + ""!"", args );
			});
		}
	},

	removeData: function( key ) {
		return this.each(function() {
			jQuery.removeData( this, key );
		});
	}
});

function dataAttr( elem, key, data ) {
	// If nothing was found internally, try to fetch any
	// data from the HTML5 data-* attribute
	if ( data === undefined && elem.nodeType === 1 ) {
		data = elem.getAttribute( ""data-"" + key );

		if ( typeof data === ""string"" ) {
			try {
				data = data === ""true"" ? true :
				data === ""false"" ? false :
				data === ""null"" ? null :
				!jQuery.isNaN( data ) ? parseFloat( data ) :
					rbrace.test( data ) ? jQuery.parseJSON( data ) :
					data;
			} catch( e ) {}

			// Make sure we set the data so it isn't changed later
			jQuery.data( elem, key, data );

		} else {
			data = undefined;
		}
	}

	return data;
}




jQuery.extend({
	queue: function( elem, type, data ) {
		if ( !elem ) {
			return;
		}

		type = (type || ""fx"") + ""queue"";
		var q = jQuery.data( elem, type );

		// Speed up dequeue by getting out quickly if this is just a lookup
		if ( !data ) {
			return q || [];
		}

		if ( !q || jQuery.isArray(data) ) {
			q = jQuery.data( elem, type, jQuery.makeArray(data) );

		} else {
			q.push( data );
		}

		return q;
	},

	dequeue: function( elem, type ) {
		type = type || ""fx"";

		var queue = jQuery.queue( elem, type ),
			fn = queue.shift();

		// If the fx queue is dequeued, always remove the progress sentinel
		if ( fn === ""inprogress"" ) {
			fn = queue.shift();
		}

		if ( fn ) {
			// Add a progress sentinel to prevent the fx queue from being
			// automatically dequeued
			if ( type === ""fx"" ) {
				queue.unshift(""inprogress"");
			}

			fn.call(elem, function() {
				jQuery.dequeue(elem, type);
			});
		}
	}
});

jQuery.fn.extend({
	queue: function( type, data ) {
		if ( typeof type !== ""string"" ) {
			data = type;
			type = ""fx"";
		}

		if ( data === undefined ) {
			return jQuery.queue( this[0], type );
		}
		return this.each(function( i ) {
			var queue = jQuery.queue( this, type, data );

			if ( type === ""fx"" && queue[0] !== ""inprogress"" ) {
				jQuery.dequeue( this, type );
			}
		});
	},
	dequeue: function( type ) {
		return this.each(function() {
			jQuery.dequeue( this, type );
		});
	},

	// Based off of the plugin by Clint Helfers, with permission.
	// http://blindsignals.com/index.php/2009/07/jquery-delay/
	delay: function( time, type ) {
		time = jQuery.fx ? jQuery.fx.speeds[time] || time : time;
		type = type || ""fx"";

		return this.queue( type, function() {
			var elem = this;
			setTimeout(function() {
				jQuery.dequeue( elem, type );
			}, time );
		});
	},

	clearQueue: function( type ) {
		return this.queue( type || ""fx"", [] );
	}
});




var rclass = /[\n\t]/g,
	rspaces = /\s+/,
	rreturn = /\r/g,
	rspecialurl = /^(?:href|src|style)$/,
	rtype = /^(?:button|input)$/i,
	rfocusable = /^(?:button|input|object|select|textarea)$/i,
	rclickable = /^a(?:rea)?$/i,
	rradiocheck = /^(?:radio|checkbox)$/i;

jQuery.props = {
	""for"": ""htmlFor"",
	""class"": ""className"",
	readonly: ""readOnly"",
	maxlength: ""maxLength"",
	cellspacing: ""cellSpacing"",
	rowspan: ""rowSpan"",
	colspan: ""colSpan"",
	tabindex: ""tabIndex"",
	usemap: ""useMap"",
	frameborder: ""frameBorder""
};

jQuery.fn.extend({
	attr: function( name, value ) {
		return jQuery.access( this, name, value, true, jQuery.attr );
	},

	removeAttr: function( name, fn ) {
		return this.each(function(){
			jQuery.attr( this, name, """" );
			if ( this.nodeType === 1 ) {
				this.removeAttribute( name );
			}
		});
	},

	addClass: function( value ) {
		if ( jQuery.isFunction(value) ) {
			return this.each(function(i) {
				var self = jQuery(this);
				self.addClass( value.call(this, i, self.attr(""class"")) );
			});
		}

		if ( value && typeof value === ""string"" ) {
			var classNames = (value || """").split( rspaces );

			for ( var i = 0, l = this.length; i < l; i++ ) {
				var elem = this[i];

				if ( elem.nodeType === 1 ) {
					if ( !elem.className ) {
						elem.className = value;

					} else {
						var className = "" "" + elem.className + "" "",
							setClass = elem.className;

						for ( var c = 0, cl = classNames.length; c < cl; c++ ) {
							if ( className.indexOf( "" "" + classNames[c] + "" "" ) < 0 ) {
								setClass += "" "" + classNames[c];
							}
						}
						elem.className = jQuery.trim( setClass );
					}
				}
			}
		}

		return this;
	},

	removeClass: function( value ) {
		if ( jQuery.isFunction(value) ) {
			return this.each(function(i) {
				var self = jQuery(this);
				self.removeClass( value.call(this, i, self.attr(""class"")) );
			});
		}

		if ( (value && typeof value === ""string"") || value === undefined ) {
			var classNames = (value || """").split( rspaces );

			for ( var i = 0, l = this.length; i < l; i++ ) {
				var elem = this[i];

				if ( elem.nodeType === 1 && elem.className ) {
					if ( value ) {
						var className = ("" "" + elem.className + "" "").replace(rclass, "" "");
						for ( var c = 0, cl = classNames.length; c < cl; c++ ) {
							className = className.replace("" "" + classNames[c] + "" "", "" "");
						}
						elem.className = jQuery.trim( className );

					} else {
						elem.className = """";
					}
				}
			}
		}

		return this;
	},

	toggleClass: function( value, stateVal ) {
		var type = typeof value,
			isBool = typeof stateVal === ""boolean"";

		if ( jQuery.isFunction( value ) ) {
			return this.each(function(i) {
				var self = jQuery(this);
				self.toggleClass( value.call(this, i, self.attr(""class""), stateVal), stateVal );
			});
		}

		return this.each(function() {
			if ( type === ""string"" ) {
				// toggle individual class names
				var className,
					i = 0,
					self = jQuery( this ),
					state = stateVal,
					classNames = value.split( rspaces );

				while ( (className = classNames[ i++ ]) ) {
					// check each className given, space seperated list
					state = isBool ? state : !self.hasClass( className );
					self[ state ? ""addClass"" : ""removeClass"" ]( className );
				}

			} else if ( type === ""undefined"" || type === ""boolean"" ) {
				if ( this.className ) {
					// store className if set
					jQuery.data( this, ""__className__"", this.className );
				}

				// toggle whole className
				this.className = this.className || value === false ? """" : jQuery.data( this, ""__className__"" ) || """";
			}
		});
	},

	hasClass: function( selector ) {
		var className = "" "" + selector + "" "";
		for ( var i = 0, l = this.length; i < l; i++ ) {
			if ( ("" "" + this[i].className + "" "").replace(rclass, "" "").indexOf( className ) > -1 ) {
				return true;
			}
		}

		return false;
	},

	val: function( value ) {
		if ( !arguments.length ) {
			var elem = this[0];

			if ( elem ) {
				if ( jQuery.nodeName( elem, ""option"" ) ) {
					// attributes.value is undefined in Blackberry 4.7 but
					// uses .value. See #6932
					var val = elem.attributes.value;
					return !val || val.specified ? elem.value : elem.text;
				}

				// We need to handle select boxes special
				if ( jQuery.nodeName( elem, ""select"" ) ) {
					var index = elem.selectedIndex,
						values = [],
						options = elem.options,
						one = elem.type === ""select-one"";

					// Nothing was selected
					if ( index < 0 ) {
						return null;
					}

					// Loop through all the selected options
					for ( var i = one ? index : 0, max = one ? index + 1 : options.length; i < max; i++ ) {
						var option = options[ i ];

						// Don't return options that are disabled or in a disabled optgroup
						if ( option.selected && (jQuery.support.optDisabled ? !option.disabled : option.getAttribute(""disabled"") === null) && 
								(!option.parentNode.disabled || !jQuery.nodeName( option.parentNode, ""optgroup"" )) ) {

							// Get the specific value for the option
							value = jQuery(option).val();

							// We don't need an array for one selects
							if ( one ) {
								return value;
							}

							// Multi-Selects return an array
							values.push( value );
						}
					}

					return values;
				}

				// Handle the case where in Webkit """" is returned instead of ""on"" if a value isn't specified
				if ( rradiocheck.test( elem.type ) && !jQuery.support.checkOn ) {
					return elem.getAttribute(""value"") === null ? ""on"" : elem.value;
				}
				

				// Everything else, we just grab the value
				return (elem.value || """").replace(rreturn, """");

			}

			return undefined;
		}

		var isFunction = jQuery.isFunction(value);

		return this.each(function(i) {
			var self = jQuery(this), val = value;

			if ( this.nodeType !== 1 ) {
				return;
			}

			if ( isFunction ) {
				val = value.call(this, i, self.val());
			}

			// Treat null/undefined as """"; convert numbers to string
			if ( val == null ) {
				val = """";
			} else if ( typeof val === ""number"" ) {
				val += """";
			} else if ( jQuery.isArray(val) ) {
				val = jQuery.map(val, function (value) {
					return value == null ? """" : value + """";
				});
			}

			if ( jQuery.isArray(val) && rradiocheck.test( this.type ) ) {
				this.checked = jQuery.inArray( self.val(), val ) >= 0;

			} else if ( jQuery.nodeName( this, ""select"" ) ) {
				var values = jQuery.makeArray(val);

				jQuery( ""option"", this ).each(function() {
					this.selected = jQuery.inArray( jQuery(this).val(), values ) >= 0;
				});

				if ( !values.length ) {
					this.selectedIndex = -1;
				}

			} else {
				this.value = val;
			}
		});
	}
});

jQuery.extend({
	attrFn: {
		val: true,
		css: true,
		html: true,
		text: true,
		data: true,
		width: true,
		height: true,
		offset: true
	},
		
	attr: function( elem, name, value, pass ) {
		// don't set attributes on text and comment nodes
		if ( !elem || elem.nodeType === 3 || elem.nodeType === 8 ) {
			return undefined;
		}

		if ( pass && name in jQuery.attrFn ) {
			return jQuery(elem)[name](value);
		}

		var notxml = elem.nodeType !== 1 || !jQuery.isXMLDoc( elem ),
			// Whether we are setting (or getting)
			set = value !== undefined;

		// Try to normalize/fix the name
		name = notxml && jQuery.props[ name ] || name;

		// These attributes require special treatment
		var special = rspecialurl.test( name );

		// Safari mis-reports the default selected property of an option
		// Accessing the parent's selectedIndex property fixes it
		if ( name === ""selected"" && !jQuery.support.optSelected ) {
			var parent = elem.parentNode;
			if ( parent ) {
				parent.selectedIndex;

				// Make sure that it also works with optgroups, see #5701
				if ( parent.parentNode ) {
					parent.parentNode.selectedIndex;
				}
			}
		}

		// If applicable, access the attribute via the DOM 0 way
		// 'in' checks fail in Blackberry 4.7 #6931
		if ( (name in elem || elem[ name ] !== undefined) && notxml && !special ) {
			if ( set ) {
				// We can't allow the type property to be changed (since it causes problems in IE)
				if ( name === ""type"" && rtype.test( elem.nodeName ) && elem.parentNode ) {
					jQuery.error( ""type property can't be changed"" );
				}

				if ( value === null ) {
					if ( elem.nodeType === 1 ) {
						elem.removeAttribute( name );
					}

				} else {
					elem[ name ] = value;
				}
			}

			// browsers index elements by id/name on forms, give priority to attributes.
			if ( jQuery.nodeName( elem, ""form"" ) && elem.getAttributeNode(name) ) {
				return elem.getAttributeNode( name ).nodeValue;
			}

			// elem.tabIndex doesn't always return the correct value when it hasn't been explicitly set
			// http://fluidproject.org/blog/2008/01/09/getting-setting-and-removing-tabindex-values-with-javascript/
			if ( name === ""tabIndex"" ) {
				var attributeNode = elem.getAttributeNode( ""tabIndex"" );

				return attributeNode && attributeNode.specified ?
					attributeNode.value :
					rfocusable.test( elem.nodeName ) || rclickable.test( elem.nodeName ) && elem.href ?
						0 :
						undefined;
			}

			return elem[ name ];
		}

		if ( !jQuery.support.style && notxml && name === ""style"" ) {
			if ( set ) {
				elem.style.cssText = """" + value;
			}

			return elem.style.cssText;
		}

		if ( set ) {
			// convert the value to a string (all browsers do this but IE) see #1070
			elem.setAttribute( name, """" + value );
		}

		// Ensure that missing attributes return undefined
		// Blackberry 4.7 returns """" from getAttribute #6938
		if ( !elem.attributes[ name ] && (elem.hasAttribute && !elem.hasAttribute( name )) ) {
			return undefined;
		}

		var attr = !jQuery.support.hrefNormalized && notxml && special ?
				// Some attributes require a special call on IE
				elem.getAttribute( name, 2 ) :
				elem.getAttribute( name );

		// Non-existent attributes return null, we normalize to undefined
		return attr === null ? undefined : attr;
	}
});




var rnamespaces = /\.(.*)$/,
	rformElems = /^(?:textarea|input|select)$/i,
	rperiod = /\./g,
	rspace = / /g,
	rescape = /[^\w\s.|`]/g,
	fcleanup = function( nm ) {
		return nm.replace(rescape, ""\\$&"");
	},
	focusCounts = { focusin: 0, focusout: 0 };

/*
 * A number of helper functions used for managing events.
 * Many of the ideas behind this code originated from
 * Dean Edwards' addEvent library.
 */
jQuery.event = {

	// Bind an event to an element
	// Original by Dean Edwards
	add: function( elem, types, handler, data ) {
		if ( elem.nodeType === 3 || elem.nodeType === 8 ) {
			return;
		}

		// For whatever reason, IE has trouble passing the window object
		// around, causing it to be cloned in the process
		if ( jQuery.isWindow( elem ) && ( elem !== window && !elem.frameElement ) ) {
			elem = window;
		}

		if ( handler === false ) {
			handler = returnFalse;
		} else if ( !handler ) {
			// Fixes bug #7229. Fix recommended by jdalton
		  return;
		}

		var handleObjIn, handleObj;

		if ( handler.handler ) {
			handleObjIn = handler;
			handler = handleObjIn.handler;
		}

		// Make sure that the function being executed has a unique ID
		if ( !handler.guid ) {
			handler.guid = jQuery.guid++;
		}

		// Init the element's event structure
		var elemData = jQuery.data( elem );

		// If no elemData is found then we must be trying to bind to one of the
		// banned noData elements
		if ( !elemData ) {
			return;
		}

		// Use a key less likely to result in collisions for plain JS objects.
		// Fixes bug #7150.
		var eventKey = elem.nodeType ? ""events"" : ""__events__"",
			events = elemData[ eventKey ],
			eventHandle = elemData.handle;
			
		if ( typeof events === ""function"" ) {
			// On plain objects events is a fn that holds the the data
			// which prevents this data from being JSON serialized
			// the function does not need to be called, it just contains the data
			eventHandle = events.handle;
			events = events.events;

		} else if ( !events ) {
			if ( !elem.nodeType ) {
				// On plain objects, create a fn that acts as the holder
				// of the values to avoid JSON serialization of event data
				elemData[ eventKey ] = elemData = function(){};
			}

			elemData.events = events = {};
		}

		if ( !eventHandle ) {
			elemData.handle = eventHandle = function() {
				// Handle the second event of a trigger and when
				// an event is called after a page has unloaded
				return typeof jQuery !== ""undefined"" && !jQuery.event.triggered ?
					jQuery.event.handle.apply( eventHandle.elem, arguments ) :
					undefined;
			};
		}

		// Add elem as a property of the handle function
		// This is to prevent a memory leak with non-native events in IE.
		eventHandle.elem = elem;

		// Handle multiple events separated by a space
		// jQuery(...).bind(""mouseover mouseout"", fn);
		types = types.split("" "");

		var type, i = 0, namespaces;

		while ( (type = types[ i++ ]) ) {
			handleObj = handleObjIn ?
				jQuery.extend({}, handleObjIn) :
				{ handler: handler, data: data };

			// Namespaced event handlers
			if ( type.indexOf(""."") > -1 ) {
				namespaces = type.split(""."");
				type = namespaces.shift();
				handleObj.namespace = namespaces.slice(0).sort().join(""."");

			} else {
				namespaces = [];
				handleObj.namespace = """";
			}

			handleObj.type = type;
			if ( !handleObj.guid ) {
				handleObj.guid = handler.guid;
			}

			// Get the current list of functions bound to this event
			var handlers = events[ type ],
				special = jQuery.event.special[ type ] || {};

			// Init the event handler queue
			if ( !handlers ) {
				handlers = events[ type ] = [];

				// Check for a special event handler
				// Only use addEventListener/attachEvent if the special
				// events handler returns false
				if ( !special.setup || special.setup.call( elem, data, namespaces, eventHandle ) === false ) {
					// Bind the global event handler to the element
					if ( elem.addEventListener ) {
						elem.addEventListener( type, eventHandle, false );

					} else if ( elem.attachEvent ) {
						elem.attachEvent( ""on"" + type, eventHandle );
					}
				}
			}
			
			if ( special.add ) { 
				special.add.call( elem, handleObj ); 

				if ( !handleObj.handler.guid ) {
					handleObj.handler.guid = handler.guid;
				}
			}

			// Add the function to the element's handler list
			handlers.push( handleObj );

			// Keep track of which events have been used, for global triggering
			jQuery.event.global[ type ] = true;
		}

		// Nullify elem to prevent memory leaks in IE
		elem = null;
	},

	global: {},

	// Detach an event or set of events from an element
	remove: function( elem, types, handler, pos ) {
		// don't do events on text and comment nodes
		if ( elem.nodeType === 3 || elem.nodeType === 8 ) {
			return;
		}

		if ( handler === false ) {
			handler = returnFalse;
		}

		var ret, type, fn, j, i = 0, all, namespaces, namespace, special, eventType, handleObj, origType,
			eventKey = elem.nodeType ? ""events"" : ""__events__"",
			elemData = jQuery.data( elem ),
			events = elemData && elemData[ eventKey ];

		if ( !elemData || !events ) {
			return;
		}
		
		if ( typeof events === ""function"" ) {
			elemData = events;
			events = events.events;
		}

		// types is actually an event object here
		if ( types && types.type ) {
			handler = types.handler;
			types = types.type;
		}

		// Unbind all events for the element
		if ( !types || typeof types === ""string"" && types.charAt(0) === ""."" ) {
			types = types || """";

			for ( type in events ) {
				jQuery.event.remove( elem, type + types );
			}

			return;
		}

		// Handle multiple events separated by a space
		// jQuery(...).unbind(""mouseover mouseout"", fn);
		types = types.split("" "");

		while ( (type = types[ i++ ]) ) {
			origType = type;
			handleObj = null;
			all = type.indexOf(""."") < 0;
			namespaces = [];

			if ( !all ) {
				// Namespaced event handlers
				namespaces = type.split(""."");
				type = namespaces.shift();

				namespace = new RegExp(""(^|\\.)"" + 
					jQuery.map( namespaces.slice(0).sort(), fcleanup ).join(""\\.(?:.*\\.)?"") + ""(\\.|$)"");
			}

			eventType = events[ type ];

			if ( !eventType ) {
				continue;
			}

			if ( !handler ) {
				for ( j = 0; j < eventType.length; j++ ) {
					handleObj = eventType[ j ];

					if ( all || namespace.test( handleObj.namespace ) ) {
						jQuery.event.remove( elem, origType, handleObj.handler, j );
						eventType.splice( j--, 1 );
					}
				}

				continue;
			}

			special = jQuery.event.special[ type ] || {};

			for ( j = pos || 0; j < eventType.length; j++ ) {
				handleObj = eventType[ j ];

				if ( handler.guid === handleObj.guid ) {
					// remove the given handler for the given type
					if ( all || namespace.test( handleObj.namespace ) ) {
						if ( pos == null ) {
							eventType.splice( j--, 1 );
						}

						if ( special.remove ) {
							special.remove.call( elem, handleObj );
						}
					}

					if ( pos != null ) {
						break;
					}
				}
			}

			// remove generic event handler if no more handlers exist
			if ( eventType.length === 0 || pos != null && eventType.length === 1 ) {
				if ( !special.teardown || special.teardown.call( elem, namespaces ) === false ) {
					jQuery.removeEvent( elem, type, elemData.handle );
				}

				ret = null;
				delete events[ type ];
			}
		}

		// Remove the expando if it's no longer used
		if ( jQuery.isEmptyObject( events ) ) {
			var handle = elemData.handle;
			if ( handle ) {
				handle.elem = null;
			}

			delete elemData.events;
			delete elemData.handle;

			if ( typeof elemData === ""function"" ) {
				jQuery.removeData( elem, eventKey );

			} else if ( jQuery.isEmptyObject( elemData ) ) {
				jQuery.removeData( elem );
			}
		}
	},

	// bubbling is internal
	trigger: function( event, data, elem /*, bubbling */ ) {
		// Event object or event type
		var type = event.type || event,
			bubbling = arguments[3];

		if ( !bubbling ) {
			event = typeof event === ""object"" ?
				// jQuery.Event object
				event[ jQuery.expando ] ? event :
				// Object literal
				jQuery.extend( jQuery.Event(type), event ) :
				// Just the event type (string)
				jQuery.Event(type);

			if ( type.indexOf(""!"") >= 0 ) {
				event.type = type = type.slice(0, -1);
				event.exclusive = true;
			}

			// Handle a global trigger
			if ( !elem ) {
				// Don't bubble custom events when global (to avoid too much overhead)
				event.stopPropagation();

				// Only trigger if we've ever bound an event for it
				if ( jQuery.event.global[ type ] ) {
					jQuery.each( jQuery.cache, function() {
						if ( this.events && this.events[type] ) {
							jQuery.event.trigger( event, data, this.handle.elem );
						}
					});
				}
			}

			// Handle triggering a single element

			// don't do events on text and comment nodes
			if ( !elem || elem.nodeType === 3 || elem.nodeType === 8 ) {
				return undefined;
			}

			// Clean up in case it is reused
			event.result = undefined;
			event.target = elem;

			// Clone the incoming data, if any
			data = jQuery.makeArray( data );
			data.unshift( event );
		}

		event.currentTarget = elem;

		// Trigger the event, it is assumed that ""handle"" is a function
		var handle = elem.nodeType ?
			jQuery.data( elem, ""handle"" ) :
			(jQuery.data( elem, ""__events__"" ) || {}).handle;

		if ( handle ) {
			handle.apply( elem, data );
		}

		var parent = elem.parentNode || elem.ownerDocument;

		// Trigger an inline bound script
		try {
			if ( !(elem && elem.nodeName && jQuery.noData[elem.nodeName.toLowerCase()]) ) {
				if ( elem[ ""on"" + type ] && elem[ ""on"" + type ].apply( elem, data ) === false ) {
					event.result = false;
					event.preventDefault();
				}
			}

		// prevent IE from throwing an error for some elements with some event types, see #3533
		} catch (inlineError) {}

		if ( !event.isPropagationStopped() && parent ) {
			jQuery.event.trigger( event, data, parent, true );

		} else if ( !event.isDefaultPrevented() ) {
			var old,
				target = event.target,
				targetType = type.replace( rnamespaces, """" ),
				isClick = jQuery.nodeName( target, ""a"" ) && targetType === ""click"",
				special = jQuery.event.special[ targetType ] || {};

			if ( (!special._default || special._default.call( elem, event ) === false) && 
				!isClick && !(target && target.nodeName && jQuery.noData[target.nodeName.toLowerCase()]) ) {

				try {
					if ( target[ targetType ] ) {
						// Make sure that we don't accidentally re-trigger the onFOO events
						old = target[ ""on"" + targetType ];

						if ( old ) {
							target[ ""on"" + targetType ] = null;
						}

						jQuery.event.triggered = true;
						target[ targetType ]();
					}

				// prevent IE from throwing an error for some elements with some event types, see #3533
				} catch (triggerError) {}

				if ( old ) {
					target[ ""on"" + targetType ] = old;
				}

				jQuery.event.triggered = false;
			}
		}
	},

	handle: function( event ) {
		var all, handlers, namespaces, namespace_re, events,
			namespace_sort = [],
			args = jQuery.makeArray( arguments );

		event = args[0] = jQuery.event.fix( event || window.event );
		event.currentTarget = this;

		// Namespaced event handlers
		all = event.type.indexOf(""."") < 0 && !event.exclusive;

		if ( !all ) {
			namespaces = event.type.split(""."");
			event.type = namespaces.shift();
			namespace_sort = namespaces.slice(0).sort();
			namespace_re = new RegExp(""(^|\\.)"" + namespace_sort.join(""\\.(?:.*\\.)?"") + ""(\\.|$)"");
		}

		event.namespace = event.namespace || namespace_sort.join(""."");

		events = jQuery.data(this, this.nodeType ? ""events"" : ""__events__"");

		if ( typeof events === ""function"" ) {
			events = events.events;
		}

		handlers = (events || {})[ event.type ];

		if ( events && handlers ) {
			// Clone the handlers to prevent manipulation
			handlers = handlers.slice(0);

			for ( var j = 0, l = handlers.length; j < l; j++ ) {
				var handleObj = handlers[ j ];

				// Filter the functions by class
				if ( all || namespace_re.test( handleObj.namespace ) ) {
					// Pass in a reference to the handler function itself
					// So that we can later remove it
					event.handler = handleObj.handler;
					event.data = handleObj.data;
					event.handleObj = handleObj;
	
					var ret = handleObj.handler.apply( this, args );

					if ( ret !== undefined ) {
						event.result = ret;
						if ( ret === false ) {
							event.preventDefault();
							event.stopPropagation();
						}
					}

					if ( event.isImmediatePropagationStopped() ) {
						break;
					}
				}
			}
		}

		return event.result;
	},

	props: ""altKey attrChange attrName bubbles button cancelable charCode clientX clientY ctrlKey currentTarget data detail eventPhase fromElement handler keyCode layerX layerY metaKey newValue offsetX offsetY pageX pageY prevValue relatedNode relatedTarget screenX screenY shiftKey srcElement target toElement view wheelDelta which"".split("" ""),

	fix: function( event ) {
		if ( event[ jQuery.expando ] ) {
			return event;
		}

		// store a copy of the original event object
		// and ""clone"" to set read-only properties
		var originalEvent = event;
		event = jQuery.Event( originalEvent );

		for ( var i = this.props.length, prop; i; ) {
			prop = this.props[ --i ];
			event[ prop ] = originalEvent[ prop ];
		}

		// Fix target property, if necessary
		if ( !event.target ) {
			// Fixes #1925 where srcElement might not be defined either
			event.target = event.srcElement || document;
		}

		// check if target is a textnode (safari)
		if ( event.target.nodeType === 3 ) {
			event.target = event.target.parentNode;
		}

		// Add relatedTarget, if necessary
		if ( !event.relatedTarget && event.fromElement ) {
			event.relatedTarget = event.fromElement === event.target ? event.toElement : event.fromElement;
		}

		// Calculate pageX/Y if missing and clientX/Y available
		if ( event.pageX == null && event.clientX != null ) {
			var doc = document.documentElement,
				body = document.body;

			event.pageX = event.clientX + (doc && doc.scrollLeft || body && body.scrollLeft || 0) - (doc && doc.clientLeft || body && body.clientLeft || 0);
			event.pageY = event.clientY + (doc && doc.scrollTop  || body && body.scrollTop  || 0) - (doc && doc.clientTop  || body && body.clientTop  || 0);
		}

		// Add which for key events
		if ( event.which == null && (event.charCode != null || event.keyCode != null) ) {
			event.which = event.charCode != null ? event.charCode : event.keyCode;
		}

		// Add metaKey to non-Mac browsers (use ctrl for PC's and Meta for Macs)
		if ( !event.metaKey && event.ctrlKey ) {
			event.metaKey = event.ctrlKey;
		}

		// Add which for click: 1 === left; 2 === middle; 3 === right
		// Note: button is not normalized, so don't use it
		if ( !event.which && event.button !== undefined ) {
			event.which = (event.button & 1 ? 1 : ( event.button & 2 ? 3 : ( event.button & 4 ? 2 : 0 ) ));
		}

		return event;
	},

	// Deprecated, use jQuery.guid instead
	guid: 1E8,

	// Deprecated, use jQuery.proxy instead
	proxy: jQuery.proxy,

	special: {
		ready: {
			// Make sure the ready event is setup
			setup: jQuery.bindReady,
			teardown: jQuery.noop
		},

		live: {
			add: function( handleObj ) {
				jQuery.event.add( this,
					liveConvert( handleObj.origType, handleObj.selector ),
					jQuery.extend({}, handleObj, {handler: liveHandler, guid: handleObj.handler.guid}) ); 
			},

			remove: function( handleObj ) {
				jQuery.event.remove( this, liveConvert( handleObj.origType, handleObj.selector ), handleObj );
			}
		},

		beforeunload: {
			setup: function( data, namespaces, eventHandle ) {
				// We only want to do this special case on windows
				if ( jQuery.isWindow( this ) ) {
					this.onbeforeunload = eventHandle;
				}
			},

			teardown: function( namespaces, eventHandle ) {
				if ( this.onbeforeunload === eventHandle ) {
					this.onbeforeunload = null;
				}
			}
		}
	}
};

jQuery.removeEvent = document.removeEventListener ?
	function( elem, type, handle ) {
		if ( elem.removeEventListener ) {
			elem.removeEventListener( type, handle, false );
		}
	} : 
	function( elem, type, handle ) {
		if ( elem.detachEvent ) {
			elem.detachEvent( ""on"" + type, handle );
		}
	};

jQuery.Event = function( src ) {
	// Allow instantiation without the 'new' keyword
	if ( !this.preventDefault ) {
		return new jQuery.Event( src );
	}

	// Event object
	if ( src && src.type ) {
		this.originalEvent = src;
		this.type = src.type;
	// Event type
	} else {
		this.type = src;
	}

	// timeStamp is buggy for some events on Firefox(#3843)
	// So we won't rely on the native value
	this.timeStamp = jQuery.now();

	// Mark it as fixed
	this[ jQuery.expando ] = true;
};

function returnFalse() {
	return false;
}
function returnTrue() {
	return true;
}

// jQuery.Event is based on DOM3 Events as specified by the ECMAScript Language Binding
// http://www.w3.org/TR/2003/WD-DOM-Level-3-Events-20030331/ecma-script-binding.html
jQuery.Event.prototype = {
	preventDefault: function() {
		this.isDefaultPrevented = returnTrue;

		var e = this.originalEvent;
		if ( !e ) {
			return;
		}
		
		// if preventDefault exists run it on the original event
		if ( e.preventDefault ) {
			e.preventDefault();

		// otherwise set the returnValue property of the original event to false (IE)
		} else {
			e.returnValue = false;
		}
	},
	stopPropagation: function() {
		this.isPropagationStopped = returnTrue;

		var e = this.originalEvent;
		if ( !e ) {
			return;
		}
		// if stopPropagation exists run it on the original event
		if ( e.stopPropagation ) {
			e.stopPropagation();
		}
		// otherwise set the cancelBubble property of the original event to true (IE)
		e.cancelBubble = true;
	},
	stopImmediatePropagation: function() {
		this.isImmediatePropagationStopped = returnTrue;
		this.stopPropagation();
	},
	isDefaultPrevented: returnFalse,
	isPropagationStopped: returnFalse,
	isImmediatePropagationStopped: returnFalse
};

// Checks if an event happened on an element within another element
// Used in jQuery.event.special.mouseenter and mouseleave handlers
var withinElement = function( event ) {
	// Check if mouse(over|out) are still within the same parent element
	var parent = event.relatedTarget;

	// Firefox sometimes assigns relatedTarget a XUL element
	// which we cannot access the parentNode property of
	try {
		// Traverse up the tree
		while ( parent && parent !== this ) {
			parent = parent.parentNode;
		}

		if ( parent !== this ) {
			// set the correct event type
			event.type = event.data;

			// handle event if we actually just moused on to a non sub-element
			jQuery.event.handle.apply( this, arguments );
		}

	// assuming we've left the element since we most likely mousedover a xul element
	} catch(e) { }
},

// In case of event delegation, we only need to rename the event.type,
// liveHandler will take care of the rest.
delegate = function( event ) {
	event.type = event.data;
	jQuery.event.handle.apply( this, arguments );
};

// Create mouseenter and mouseleave events
jQuery.each({
	mouseenter: ""mouseover"",
	mouseleave: ""mouseout""
}, function( orig, fix ) {
	jQuery.event.special[ orig ] = {
		setup: function( data ) {
			jQuery.event.add( this, fix, data && data.selector ? delegate : withinElement, orig );
		},
		teardown: function( data ) {
			jQuery.event.remove( this, fix, data && data.selector ? delegate : withinElement );
		}
	};
});

// submit delegation
if ( !jQuery.support.submitBubbles ) {

	jQuery.event.special.submit = {
		setup: function( data, namespaces ) {
			if ( this.nodeName.toLowerCase() !== ""form"" ) {
				jQuery.event.add(this, ""click.specialSubmit"", function( e ) {
					var elem = e.target,
						type = elem.type;

					if ( (type === ""submit"" || type === ""image"") && jQuery( elem ).closest(""form"").length ) {
						e.liveFired = undefined;
						return trigger( ""submit"", this, arguments );
					}
				});
	 
				jQuery.event.add(this, ""keypress.specialSubmit"", function( e ) {
					var elem = e.target,
						type = elem.type;

					if ( (type === ""text"" || type === ""password"") && jQuery( elem ).closest(""form"").length && e.keyCode === 13 ) {
						e.liveFired = undefined;
						return trigger( ""submit"", this, arguments );
					}
				});

			} else {
				return false;
			}
		},

		teardown: function( namespaces ) {
			jQuery.event.remove( this, "".specialSubmit"" );
		}
	};

}

// change delegation, happens here so we have bind.
if ( !jQuery.support.changeBubbles ) {

	var changeFilters,

	getVal = function( elem ) {
		var type = elem.type, val = elem.value;

		if ( type === ""radio"" || type === ""checkbox"" ) {
			val = elem.checked;

		} else if ( type === ""select-multiple"" ) {
			val = elem.selectedIndex > -1 ?
				jQuery.map( elem.options, function( elem ) {
					return elem.selected;
				}).join(""-"") :
				"""";

		} else if ( elem.nodeName.toLowerCase() === ""select"" ) {
			val = elem.selectedIndex;
		}

		return val;
	},

	testChange = function testChange( e ) {
		var elem = e.target, data, val;

		if ( !rformElems.test( elem.nodeName ) || elem.readOnly ) {
			return;
		}

		data = jQuery.data( elem, ""_change_data"" );
		val = getVal(elem);

		// the current data will be also retrieved by beforeactivate
		if ( e.type !== ""focusout"" || elem.type !== ""radio"" ) {
			jQuery.data( elem, ""_change_data"", val );
		}
		
		if ( data === undefined || val === data ) {
			return;
		}

		if ( data != null || val ) {
			e.type = ""change"";
			e.liveFired = undefined;
			return jQuery.event.trigger( e, arguments[1], elem );
		}
	};

	jQuery.event.special.change = {
		filters: {
			focusout: testChange, 

			beforedeactivate: testChange,

			click: function( e ) {
				var elem = e.target, type = elem.type;

				if ( type === ""radio"" || type === ""checkbox"" || elem.nodeName.toLowerCase() === ""select"" ) {
					return testChange.call( this, e );
				}
			},

			// Change has to be called before submit
			// Keydown will be called before keypress, which is used in submit-event delegation
			keydown: function( e ) {
				var elem = e.target, type = elem.type;

				if ( (e.keyCode === 13 && elem.nodeName.toLowerCase() !== ""textarea"") ||
					(e.keyCode === 32 && (type === ""checkbox"" || type === ""radio"")) ||
					type === ""select-multiple"" ) {
					return testChange.call( this, e );
				}
			},

			// Beforeactivate happens also before the previous element is blurred
			// with this event you can't trigger a change event, but you can store
			// information
			beforeactivate: function( e ) {
				var elem = e.target;
				jQuery.data( elem, ""_change_data"", getVal(elem) );
			}
		},

		setup: function( data, namespaces ) {
			if ( this.type === ""file"" ) {
				return false;
			}

			for ( var type in changeFilters ) {
				jQuery.event.add( this, type + "".specialChange"", changeFilters[type] );
			}

			return rformElems.test( this.nodeName );
		},

		teardown: function( namespaces ) {
			jQuery.event.remove( this, "".specialChange"" );

			return rformElems.test( this.nodeName );
		}
	};

	changeFilters = jQuery.event.special.change.filters;

	// Handle when the input is .focus()'d
	changeFilters.focus = changeFilters.beforeactivate;
}

function trigger( type, elem, args ) {
	args[0].type = type;
	return jQuery.event.handle.apply( elem, args );
}

// Create ""bubbling"" focus and blur events
if ( document.addEventListener ) {
	jQuery.each({ focus: ""focusin"", blur: ""focusout"" }, function( orig, fix ) {
		jQuery.event.special[ fix ] = {
			setup: function() {
				if ( focusCounts[fix]++ === 0 ) {
					document.addEventListener( orig, handler, true );
				}
			}, 
			teardown: function() { 
				if ( --focusCounts[fix] === 0 ) {
					document.removeEventListener( orig, handler, true );
				}
			}
		};

		function handler( e ) { 
			e = jQuery.event.fix( e );
			e.type = fix;
			return jQuery.event.trigger( e, null, e.target );
		}
	});
}

jQuery.each([""bind"", ""one""], function( i, name ) {
	jQuery.fn[ name ] = function( type, data, fn ) {
		// Handle object literals
		if ( typeof type === ""object"" ) {
			for ( var key in type ) {
				this[ name ](key, data, type[key], fn);
			}
			return this;
		}
		
		if ( jQuery.isFunction( data ) || data === false ) {
			fn = data;
			data = undefined;
		}

		var handler = name === ""one"" ? jQuery.proxy( fn, function( event ) {
			jQuery( this ).unbind( event, handler );
			return fn.apply( this, arguments );
		}) : fn;

		if ( type === ""unload"" && name !== ""one"" ) {
			this.one( type, data, fn );

		} else {
			for ( var i = 0, l = this.length; i < l; i++ ) {
				jQuery.event.add( this[i], type, handler, data );
			}
		}

		return this;
	};
});

jQuery.fn.extend({
	unbind: function( type, fn ) {
		// Handle object literals
		if ( typeof type === ""object"" && !type.preventDefault ) {
			for ( var key in type ) {
				this.unbind(key, type[key]);
			}

		} else {
			for ( var i = 0, l = this.length; i < l; i++ ) {
				jQuery.event.remove( this[i], type, fn );
			}
		}

		return this;
	},
	
	delegate: function( selector, types, data, fn ) {
		return this.live( types, data, fn, selector );
	},
	
	undelegate: function( selector, types, fn ) {
		if ( arguments.length === 0 ) {
				return this.unbind( ""live"" );
		
		} else {
			return this.die( types, null, fn, selector );
		}
	},
	
	trigger: function( type, data ) {
		return this.each(function() {
			jQuery.event.trigger( type, data, this );
		});
	},

	triggerHandler: function( type, data ) {
		if ( this[0] ) {
			var event = jQuery.Event( type );
			event.preventDefault();
			event.stopPropagation();
			jQuery.event.trigger( event, data, this[0] );
			return event.result;
		}
	},

	toggle: function( fn ) {
		// Save reference to arguments for access in closure
		var args = arguments,
			i = 1;

		// link all the functions, so any of them can unbind this click handler
		while ( i < args.length ) {
			jQuery.proxy( fn, args[ i++ ] );
		}

		return this.click( jQuery.proxy( fn, function( event ) {
			// Figure out which function to execute
			var lastToggle = ( jQuery.data( this, ""lastToggle"" + fn.guid ) || 0 ) % i;
			jQuery.data( this, ""lastToggle"" + fn.guid, lastToggle + 1 );

			// Make sure that clicks stop
			event.preventDefault();

			// and execute the function
			return args[ lastToggle ].apply( this, arguments ) || false;
		}));
	},

	hover: function( fnOver, fnOut ) {
		return this.mouseenter( fnOver ).mouseleave( fnOut || fnOver );
	}
});

var liveMap = {
	focus: ""focusin"",
	blur: ""focusout"",
	mouseenter: ""mouseover"",
	mouseleave: ""mouseout""
};

jQuery.each([""live"", ""die""], function( i, name ) {
	jQuery.fn[ name ] = function( types, data, fn, origSelector /* Internal Use Only */ ) {
		var type, i = 0, match, namespaces, preType,
			selector = origSelector || this.selector,
			context = origSelector ? this : jQuery( this.context );
		
		if ( typeof types === ""object"" && !types.preventDefault ) {
			for ( var key in types ) {
				context[ name ]( key, data, types[key], selector );
			}
			
			return this;
		}

		if ( jQuery.isFunction( data ) ) {
			fn = data;
			data = undefined;
		}

		types = (types || """").split("" "");

		while ( (type = types[ i++ ]) != null ) {
			match = rnamespaces.exec( type );
			namespaces = """";

			if ( match )  {
				namespaces = match[0];
				type = type.replace( rnamespaces, """" );
			}

			if ( type === ""hover"" ) {
				types.push( ""mouseenter"" + namespaces, ""mouseleave"" + namespaces );
				continue;
			}

			preType = type;

			if ( type === ""focus"" || type === ""blur"" ) {
				types.push( liveMap[ type ] + namespaces );
				type = type + namespaces;

			} else {
				type = (liveMap[ type ] || type) + namespaces;
			}

			if ( name === ""live"" ) {
				// bind live handler
				for ( var j = 0, l = context.length; j < l; j++ ) {
					jQuery.event.add( context[j], ""live."" + liveConvert( type, selector ),
						{ data: data, selector: selector, handler: fn, origType: type, origHandler: fn, preType: preType } );
				}

			} else {
				// unbind live handler
				context.unbind( ""live."" + liveConvert( type, selector ), fn );
			}
		}
		
		return this;
	};
});

function liveHandler( event ) {
	var stop, maxLevel, related, match, handleObj, elem, j, i, l, data, close, namespace, ret,
		elems = [],
		selectors = [],
		events = jQuery.data( this, this.nodeType ? ""events"" : ""__events__"" );

	if ( typeof events === ""function"" ) {
		events = events.events;
	}

	// Make sure we avoid non-left-click bubbling in Firefox (#3861)
	if ( event.liveFired === this || !events || !events.live || event.button && event.type === ""click"" ) {
		return;
	}
	
	if ( event.namespace ) {
		namespace = new RegExp(""(^|\\.)"" + event.namespace.split(""."").join(""\\.(?:.*\\.)?"") + ""(\\.|$)"");
	}

	event.liveFired = this;

	var live = events.live.slice(0);

	for ( j = 0; j < live.length; j++ ) {
		handleObj = live[j];

		if ( handleObj.origType.replace( rnamespaces, """" ) === event.type ) {
			selectors.push( handleObj.selector );

		} else {
			live.splice( j--, 1 );
		}
	}

	match = jQuery( event.target ).closest( selectors, event.currentTarget );

	for ( i = 0, l = match.length; i < l; i++ ) {
		close = match[i];

		for ( j = 0; j < live.length; j++ ) {
			handleObj = live[j];

			if ( close.selector === handleObj.selector && (!namespace || namespace.test( handleObj.namespace )) ) {
				elem = close.elem;
				related = null;

				// Those two events require additional checking
				if ( handleObj.preType === ""mouseenter"" || handleObj.preType === ""mouseleave"" ) {
					event.type = handleObj.preType;
					related = jQuery( event.relatedTarget ).closest( handleObj.selector )[0];
				}

				if ( !related || related !== elem ) {
					elems.push({ elem: elem, handleObj: handleObj, level: close.level });
				}
			}
		}
	}

	for ( i = 0, l = elems.length; i < l; i++ ) {
		match = elems[i];

		if ( maxLevel && match.level > maxLevel ) {
			break;
		}

		event.currentTarget = match.elem;
		event.data = match.handleObj.data;
		event.handleObj = match.handleObj;

		ret = match.handleObj.origHandler.apply( match.elem, arguments );

		if ( ret === false || event.isPropagationStopped() ) {
			maxLevel = match.level;

			if ( ret === false ) {
				stop = false;
			}
			if ( event.isImmediatePropagationStopped() ) {
				break;
			}
		}
	}

	return stop;
}

function liveConvert( type, selector ) {
	return (type && type !== ""*"" ? type + ""."" : """") + selector.replace(rperiod, ""`"").replace(rspace, ""&"");
}

jQuery.each( (""blur focus focusin focusout load resize scroll unload click dblclick "" +
	""mousedown mouseup mousemove mouseover mouseout mouseenter mouseleave "" +
	""change select submit keydown keypress keyup error"").split("" ""), function( i, name ) {

	// Handle event binding
	jQuery.fn[ name ] = function( data, fn ) {
		if ( fn == null ) {
			fn = data;
			data = null;
		}

		return arguments.length > 0 ?
			this.bind( name, data, fn ) :
			this.trigger( name );
	};

	if ( jQuery.attrFn ) {
		jQuery.attrFn[ name ] = true;
	}
});

// Prevent memory leaks in IE
// Window isn't included so as not to unbind existing unload events
// More info:
//  - http://isaacschlueter.com/2006/10/msie-memory-leaks/
if ( window.attachEvent && !window.addEventListener ) {
	jQuery(window).bind(""unload"", function() {
		for ( var id in jQuery.cache ) {
			if ( jQuery.cache[ id ].handle ) {
				// Try/Catch is to handle iframes being unloaded, see #4280
				try {
					jQuery.event.remove( jQuery.cache[ id ].handle.elem );
				} catch(e) {}
			}
		}
	});
}


/*!
 * Sizzle CSS Selector Engine - v1.0
 *  Copyright 2009, The Dojo Foundation
 *  Released under the MIT, BSD, and GPL Licenses.
 *  More information: http://sizzlejs.com/
 */
(function(){

var chunker = /((?:\((?:\([^()]+\)|[^()]+)+\)|\[(?:\[[^\[\]]*\]|['""][^'""]*['""]|[^\[\]'""]+)+\]|\\.|[^ >+~,(\[\\]+)+|[>+~])(\s*,\s*)?((?:.|\r|\n)*)/g,
	done = 0,
	toString = Object.prototype.toString,
	hasDuplicate = false,
	baseHasDuplicate = true;

// Here we check if the JavaScript engine is using some sort of
// optimization where it does not always call our comparision
// function. If that is the case, discard the hasDuplicate value.
//   Thus far that includes Google Chrome.
[0, 0].sort(function() {
	baseHasDuplicate = false;
	return 0;
});

var Sizzle = function( selector, context, results, seed ) {
	results = results || [];
	context = context || document;

	var origContext = context;

	if ( context.nodeType !== 1 && context.nodeType !== 9 ) {
		return [];
	}
	
	if ( !selector || typeof selector !== ""string"" ) {
		return results;
	}

	var m, set, checkSet, extra, ret, cur, pop, i,
		prune = true,
		contextXML = Sizzle.isXML( context ),
		parts = [],
		soFar = selector;
	
	// Reset the position of the chunker regexp (start from head)
	do {
		chunker.exec( """" );
		m = chunker.exec( soFar );

		if ( m ) {
			soFar = m[3];
		
			parts.push( m[1] );
		
			if ( m[2] ) {
				extra = m[3];
				break;
			}
		}
	} while ( m );

	if ( parts.length > 1 && origPOS.exec( selector ) ) {

		if ( parts.length === 2 && Expr.relative[ parts[0] ] ) {
			set = posProcess( parts[0] + parts[1], context );

		} else {
			set = Expr.relative[ parts[0] ] ?
				[ context ] :
				Sizzle( parts.shift(), context );

			while ( parts.length ) {
				selector = parts.shift();

				if ( Expr.relative[ selector ] ) {
					selector += parts.shift();
				}
				
				set = posProcess( selector, set );
			}
		}

	} else {
		// Take a shortcut and set the context if the root selector is an ID
		// (but not if it'll be faster if the inner selector is an ID)
		if ( !seed && parts.length > 1 && context.nodeType === 9 && !contextXML &&
				Expr.match.ID.test(parts[0]) && !Expr.match.ID.test(parts[parts.length - 1]) ) {

			ret = Sizzle.find( parts.shift(), context, contextXML );
			context = ret.expr ?
				Sizzle.filter( ret.expr, ret.set )[0] :
				ret.set[0];
		}

		if ( context ) {
			ret = seed ?
				{ expr: parts.pop(), set: makeArray(seed) } :
				Sizzle.find( parts.pop(), parts.length === 1 && (parts[0] === ""~"" || parts[0] === ""+"") && context.parentNode ? context.parentNode : context, contextXML );

			set = ret.expr ?
				Sizzle.filter( ret.expr, ret.set ) :
				ret.set;

			if ( parts.length > 0 ) {
				checkSet = makeArray( set );

			} else {
				prune = false;
			}

			while ( parts.length ) {
				cur = parts.pop();
				pop = cur;

				if ( !Expr.relative[ cur ] ) {
					cur = """";
				} else {
					pop = parts.pop();
				}

				if ( pop == null ) {
					pop = context;
				}

				Expr.relative[ cur ]( checkSet, pop, contextXML );
			}

		} else {
			checkSet = parts = [];
		}
	}

	if ( !checkSet ) {
		checkSet = set;
	}

	if ( !checkSet ) {
		Sizzle.error( cur || selector );
	}

	if ( toString.call(checkSet) === ""[object Array]"" ) {
		if ( !prune ) {
			results.push.apply( results, checkSet );

		} else if ( context && context.nodeType === 1 ) {
			for ( i = 0; checkSet[i] != null; i++ ) {
				if ( checkSet[i] && (checkSet[i] === true || checkSet[i].nodeType === 1 && Sizzle.contains(context, checkSet[i])) ) {
					results.push( set[i] );
				}
			}

		} else {
			for ( i = 0; checkSet[i] != null; i++ ) {
				if ( checkSet[i] && checkSet[i].nodeType === 1 ) {
					results.push( set[i] );
				}
			}
		}

	} else {
		makeArray( checkSet, results );
	}

	if ( extra ) {
		Sizzle( extra, origContext, results, seed );
		Sizzle.uniqueSort( results );
	}

	return results;
};

Sizzle.uniqueSort = function( results ) {
	if ( sortOrder ) {
		hasDuplicate = baseHasDuplicate;
		results.sort( sortOrder );

		if ( hasDuplicate ) {
			for ( var i = 1; i < results.length; i++ ) {
				if ( results[i] === results[ i - 1 ] ) {
					results.splice( i--, 1 );
				}
			}
		}
	}

	return results;
};

Sizzle.matches = function( expr, set ) {
	return Sizzle( expr, null, null, set );
};

Sizzle.matchesSelector = function( node, expr ) {
	return Sizzle( expr, null, null, [node] ).length > 0;
};

Sizzle.find = function( expr, context, isXML ) {
	var set;

	if ( !expr ) {
		return [];
	}

	for ( var i = 0, l = Expr.order.length; i < l; i++ ) {
		var match,
			type = Expr.order[i];
		
		if ( (match = Expr.leftMatch[ type ].exec( expr )) ) {
			var left = match[1];
			match.splice( 1, 1 );

			if ( left.substr( left.length - 1 ) !== ""\\"" ) {
				match[1] = (match[1] || """").replace(/\\/g, """");
				set = Expr.find[ type ]( match, context, isXML );

				if ( set != null ) {
					expr = expr.replace( Expr.match[ type ], """" );
					break;
				}
			}
		}
	}

	if ( !set ) {
		set = context.getElementsByTagName( ""*"" );
	}

	return { set: set, expr: expr };
};

Sizzle.filter = function( expr, set, inplace, not ) {
	var match, anyFound,
		old = expr,
		result = [],
		curLoop = set,
		isXMLFilter = set && set[0] && Sizzle.isXML( set[0] );

	while ( expr && set.length ) {
		for ( var type in Expr.filter ) {
			if ( (match = Expr.leftMatch[ type ].exec( expr )) != null && match[2] ) {
				var found, item,
					filter = Expr.filter[ type ],
					left = match[1];

				anyFound = false;

				match.splice(1,1);

				if ( left.substr( left.length - 1 ) === ""\\"" ) {
					continue;
				}

				if ( curLoop === result ) {
					result = [];
				}

				if ( Expr.preFilter[ type ] ) {
					match = Expr.preFilter[ type ]( match, curLoop, inplace, result, not, isXMLFilter );

					if ( !match ) {
						anyFound = found = true;

					} else if ( match === true ) {
						continue;
					}
				}

				if ( match ) {
					for ( var i = 0; (item = curLoop[i]) != null; i++ ) {
						if ( item ) {
							found = filter( item, match, i, curLoop );
							var pass = not ^ !!found;

							if ( inplace && found != null ) {
								if ( pass ) {
									anyFound = true;

								} else {
									curLoop[i] = false;
								}

							} else if ( pass ) {
								result.push( item );
								anyFound = true;
							}
						}
					}
				}

				if ( found !== undefined ) {
					if ( !inplace ) {
						curLoop = result;
					}

					expr = expr.replace( Expr.match[ type ], """" );

					if ( !anyFound ) {
						return [];
					}

					break;
				}
			}
		}

		// Improper expression
		if ( expr === old ) {
			if ( anyFound == null ) {
				Sizzle.error( expr );

			} else {
				break;
			}
		}

		old = expr;
	}

	return curLoop;
};

Sizzle.error = function( msg ) {
	throw ""Syntax error, unrecognized expression: "" + msg;
};

var Expr = Sizzle.selectors = {
	order: [ ""ID"", ""NAME"", ""TAG"" ],

	match: {
		ID: /#((?:[\w\u00c0-\uFFFF\-]|\\.)+)/,
		CLASS: /\.((?:[\w\u00c0-\uFFFF\-]|\\.)+)/,
		NAME: /\[name=['""]*((?:[\w\u00c0-\uFFFF\-]|\\.)+)['""]*\]/,
		ATTR: /\[\s*((?:[\w\u00c0-\uFFFF\-]|\\.)+)\s*(?:(\S?=)\s*(['""]*)(.*?)\3|)\s*\]/,
		TAG: /^((?:[\w\u00c0-\uFFFF\*\-]|\\.)+)/,
		CHILD: /:(only|nth|last|first)-child(?:\((even|odd|[\dn+\-]*)\))?/,
		POS: /:(nth|eq|gt|lt|first|last|even|odd)(?:\((\d*)\))?(?=[^\-]|$)/,
		PSEUDO: /:((?:[\w\u00c0-\uFFFF\-]|\\.)+)(?:\((['""]?)((?:\([^\)]+\)|[^\(\)]*)+)\2\))?/
	},

	leftMatch: {},

	attrMap: {
		""class"": ""className"",
		""for"": ""htmlFor""
	},

	attrHandle: {
		href: function( elem ) {
			return elem.getAttribute( ""href"" );
		}
	},

	relative: {
		""+"": function(checkSet, part){
			var isPartStr = typeof part === ""string"",
				isTag = isPartStr && !/\W/.test( part ),
				isPartStrNotTag = isPartStr && !isTag;

			if ( isTag ) {
				part = part.toLowerCase();
			}

			for ( var i = 0, l = checkSet.length, elem; i < l; i++ ) {
				if ( (elem = checkSet[i]) ) {
					while ( (elem = elem.previousSibling) && elem.nodeType !== 1 ) {}

					checkSet[i] = isPartStrNotTag || elem && elem.nodeName.toLowerCase() === part ?
						elem || false :
						elem === part;
				}
			}

			if ( isPartStrNotTag ) {
				Sizzle.filter( part, checkSet, true );
			}
		},

		"">"": function( checkSet, part ) {
			var elem,
				isPartStr = typeof part === ""string"",
				i = 0,
				l = checkSet.length;

			if ( isPartStr && !/\W/.test( part ) ) {
				part = part.toLowerCase();

				for ( ; i < l; i++ ) {
					elem = checkSet[i];

					if ( elem ) {
						var parent = elem.parentNode;
						checkSet[i] = parent.nodeName.toLowerCase() === part ? parent : false;
					}
				}

			} else {
				for ( ; i < l; i++ ) {
					elem = checkSet[i];

					if ( elem ) {
						checkSet[i] = isPartStr ?
							elem.parentNode :
							elem.parentNode === part;
					}
				}

				if ( isPartStr ) {
					Sizzle.filter( part, checkSet, true );
				}
			}
		},

		"""": function(checkSet, part, isXML){
			var nodeCheck,
				doneName = done++,
				checkFn = dirCheck;

			if ( typeof part === ""string"" && !/\W/.test(part) ) {
				part = part.toLowerCase();
				nodeCheck = part;
				checkFn = dirNodeCheck;
			}

			checkFn( ""parentNode"", part, doneName, checkSet, nodeCheck, isXML );
		},

		""~"": function( checkSet, part, isXML ) {
			var nodeCheck,
				doneName = done++,
				checkFn = dirCheck;

			if ( typeof part === ""string"" && !/\W/.test( part ) ) {
				part = part.toLowerCase();
				nodeCheck = part;
				checkFn = dirNodeCheck;
			}

			checkFn( ""previousSibling"", part, doneName, checkSet, nodeCheck, isXML );
		}
	},

	find: {
		ID: function( match, context, isXML ) {
			if ( typeof context.getElementById !== ""undefined"" && !isXML ) {
				var m = context.getElementById(match[1]);
				// Check parentNode to catch when Blackberry 4.6 returns
				// nodes that are no longer in the document #6963
				return m && m.parentNode ? [m] : [];
			}
		},

		NAME: function( match, context ) {
			if ( typeof context.getElementsByName !== ""undefined"" ) {
				var ret = [],
					results = context.getElementsByName( match[1] );

				for ( var i = 0, l = results.length; i < l; i++ ) {
					if ( results[i].getAttribute(""name"") === match[1] ) {
						ret.push( results[i] );
					}
				}

				return ret.length === 0 ? null : ret;
			}
		},

		TAG: function( match, context ) {
			return context.getElementsByTagName( match[1] );
		}
	},
	preFilter: {
		CLASS: function( match, curLoop, inplace, result, not, isXML ) {
			match = "" "" + match[1].replace(/\\/g, """") + "" "";

			if ( isXML ) {
				return match;
			}

			for ( var i = 0, elem; (elem = curLoop[i]) != null; i++ ) {
				if ( elem ) {
					if ( not ^ (elem.className && ("" "" + elem.className + "" "").replace(/[\t\n]/g, "" "").indexOf(match) >= 0) ) {
						if ( !inplace ) {
							result.push( elem );
						}

					} else if ( inplace ) {
						curLoop[i] = false;
					}
				}
			}

			return false;
		},

		ID: function( match ) {
			return match[1].replace(/\\/g, """");
		},

		TAG: function( match, curLoop ) {
			return match[1].toLowerCase();
		},

		CHILD: function( match ) {
			if ( match[1] === ""nth"" ) {
				// parse equations like 'even', 'odd', '5', '2n', '3n+2', '4n-1', '-n+6'
				var test = /(-?)(\d*)n((?:\+|-)?\d*)/.exec(
					match[2] === ""even"" && ""2n"" || match[2] === ""odd"" && ""2n+1"" ||
					!/\D/.test( match[2] ) && ""0n+"" + match[2] || match[2]);

				// calculate the numbers (first)n+(last) including if they are negative
				match[2] = (test[1] + (test[2] || 1)) - 0;
				match[3] = test[3] - 0;
			}

			// TODO: Move to normal caching system
			match[0] = done++;

			return match;
		},

		ATTR: function( match, curLoop, inplace, result, not, isXML ) {
			var name = match[1].replace(/\\/g, """");
			
			if ( !isXML && Expr.attrMap[name] ) {
				match[1] = Expr.attrMap[name];
			}

			if ( match[2] === ""~="" ) {
				match[4] = "" "" + match[4] + "" "";
			}

			return match;
		},

		PSEUDO: function( match, curLoop, inplace, result, not ) {
			if ( match[1] === ""not"" ) {
				// If we're dealing with a complex expression, or a simple one
				if ( ( chunker.exec(match[3]) || """" ).length > 1 || /^\w/.test(match[3]) ) {
					match[3] = Sizzle(match[3], null, null, curLoop);

				} else {
					var ret = Sizzle.filter(match[3], curLoop, inplace, true ^ not);

					if ( !inplace ) {
						result.push.apply( result, ret );
					}

					return false;
				}

			} else if ( Expr.match.POS.test( match[0] ) || Expr.match.CHILD.test( match[0] ) ) {
				return true;
			}
			
			return match;
		},

		POS: function( match ) {
			match.unshift( true );

			return match;
		}
	},
	
	filters: {
		enabled: function( elem ) {
			return elem.disabled === false && elem.type !== ""hidden"";
		},

		disabled: function( elem ) {
			return elem.disabled === true;
		},

		checked: function( elem ) {
			return elem.checked === true;
		},
		
		selected: function( elem ) {
			// Accessing this property makes selected-by-default
			// options in Safari work properly
			elem.parentNode.selectedIndex;
			
			return elem.selected === true;
		},

		parent: function( elem ) {
			return !!elem.firstChild;
		},

		empty: function( elem ) {
			return !elem.firstChild;
		},

		has: function( elem, i, match ) {
			return !!Sizzle( match[3], elem ).length;
		},

		header: function( elem ) {
			return (/h\d/i).test( elem.nodeName );
		},

		text: function( elem ) {
			return ""text"" === elem.type;
		},
		radio: function( elem ) {
			return ""radio"" === elem.type;
		},

		checkbox: function( elem ) {
			return ""checkbox"" === elem.type;
		},

		file: function( elem ) {
			return ""file"" === elem.type;
		},
		password: function( elem ) {
			return ""password"" === elem.type;
		},

		submit: function( elem ) {
			return ""submit"" === elem.type;
		},

		image: function( elem ) {
			return ""image"" === elem.type;
		},

		reset: function( elem ) {
			return ""reset"" === elem.type;
		},

		button: function( elem ) {
			return ""button"" === elem.type || elem.nodeName.toLowerCase() === ""button"";
		},

		input: function( elem ) {
			return (/input|select|textarea|button/i).test( elem.nodeName );
		}
	},
	setFilters: {
		first: function( elem, i ) {
			return i === 0;
		},

		last: function( elem, i, match, array ) {
			return i === array.length - 1;
		},

		even: function( elem, i ) {
			return i % 2 === 0;
		},

		odd: function( elem, i ) {
			return i % 2 === 1;
		},

		lt: function( elem, i, match ) {
			return i < match[3] - 0;
		},

		gt: function( elem, i, match ) {
			return i > match[3] - 0;
		},

		nth: function( elem, i, match ) {
			return match[3] - 0 === i;
		},

		eq: function( elem, i, match ) {
			return match[3] - 0 === i;
		}
	},
	filter: {
		PSEUDO: function( elem, match, i, array ) {
			var name = match[1],
				filter = Expr.filters[ name ];

			if ( filter ) {
				return filter( elem, i, match, array );

			} else if ( name === ""contains"" ) {
				return (elem.textContent || elem.innerText || Sizzle.getText([ elem ]) || """").indexOf(match[3]) >= 0;

			} else if ( name === ""not"" ) {
				var not = match[3];

				for ( var j = 0, l = not.length; j < l; j++ ) {
					if ( not[j] === elem ) {
						return false;
					}
				}

				return true;

			} else {
				Sizzle.error( ""Syntax error, unrecognized expression: "" + name );
			}
		},

		CHILD: function( elem, match ) {
			var type = match[1],
				node = elem;

			switch ( type ) {
				case ""only"":
				case ""first"":
					while ( (node = node.previousSibling) )	 {
						if ( node.nodeType === 1 ) { 
							return false; 
						}
					}

					if ( type === ""first"" ) { 
						return true; 
					}

					node = elem;

				case ""last"":
					while ( (node = node.nextSibling) )	 {
						if ( node.nodeType === 1 ) { 
							return false; 
						}
					}

					return true;

				case ""nth"":
					var first = match[2],
						last = match[3];

					if ( first === 1 && last === 0 ) {
						return true;
					}
					
					var doneName = match[0],
						parent = elem.parentNode;
	
					if ( parent && (parent.sizcache !== doneName || !elem.nodeIndex) ) {
						var count = 0;
						
						for ( node = parent.firstChild; node; node = node.nextSibling ) {
							if ( node.nodeType === 1 ) {
								node.nodeIndex = ++count;
							}
						} 

						parent.sizcache = doneName;
					}
					
					var diff = elem.nodeIndex - last;

					if ( first === 0 ) {
						return diff === 0;

					} else {
						return ( diff % first === 0 && diff / first >= 0 );
					}
			}
		},

		ID: function( elem, match ) {
			return elem.nodeType === 1 && elem.getAttribute(""id"") === match;
		},

		TAG: function( elem, match ) {
			return (match === ""*"" && elem.nodeType === 1) || elem.nodeName.toLowerCase() === match;
		},
		
		CLASS: function( elem, match ) {
			return ("" "" + (elem.className || elem.getAttribute(""class"")) + "" "")
				.indexOf( match ) > -1;
		},

		ATTR: function( elem, match ) {
			var name = match[1],
				result = Expr.attrHandle[ name ] ?
					Expr.attrHandle[ name ]( elem ) :
					elem[ name ] != null ?
						elem[ name ] :
						elem.getAttribute( name ),
				value = result + """",
				type = match[2],
				check = match[4];

			return result == null ?
				type === ""!="" :
				type === ""="" ?
				value === check :
				type === ""*="" ?
				value.indexOf(check) >= 0 :
				type === ""~="" ?
				("" "" + value + "" "").indexOf(check) >= 0 :
				!check ?
				value && result !== false :
				type === ""!="" ?
				value !== check :
				type === ""^="" ?
				value.indexOf(check) === 0 :
				type === ""$="" ?
				value.substr(value.length - check.length) === check :
				type === ""|="" ?
				value === check || value.substr(0, check.length + 1) === check + ""-"" :
				false;
		},

		POS: function( elem, match, i, array ) {
			var name = match[2],
				filter = Expr.setFilters[ name ];

			if ( filter ) {
				return filter( elem, i, match, array );
			}
		}
	}
};

var origPOS = Expr.match.POS,
	fescape = function(all, num){
		return ""\\"" + (num - 0 + 1);
	};

for ( var type in Expr.match ) {
	Expr.match[ type ] = new RegExp( Expr.match[ type ].source + (/(?![^\[]*\])(?![^\(]*\))/.source) );
	Expr.leftMatch[ type ] = new RegExp( /(^(?:.|\r|\n)*?)/.source + Expr.match[ type ].source.replace(/\\(\d+)/g, fescape) );
}

var makeArray = function( array, results ) {
	array = Array.prototype.slice.call( array, 0 );

	if ( results ) {
		results.push.apply( results, array );
		return results;
	}
	
	return array;
};

// Perform a simple check to determine if the browser is capable of
// converting a NodeList to an array using builtin methods.
// Also verifies that the returned array holds DOM nodes
// (which is not the case in the Blackberry browser)
try {
	Array.prototype.slice.call( document.documentElement.childNodes, 0 )[0].nodeType;

// Provide a fallback method if it does not work
} catch( e ) {
	makeArray = function( array, results ) {
		var i = 0,
			ret = results || [];

		if ( toString.call(array) === ""[object Array]"" ) {
			Array.prototype.push.apply( ret, array );

		} else {
			if ( typeof array.length === ""number"" ) {
				for ( var l = array.length; i < l; i++ ) {
					ret.push( array[i] );
				}

			} else {
				for ( ; array[i]; i++ ) {
					ret.push( array[i] );
				}
			}
		}

		return ret;
	};
}

var sortOrder, siblingCheck;

if ( document.documentElement.compareDocumentPosition ) {
	sortOrder = function( a, b ) {
		if ( a === b ) {
			hasDuplicate = true;
			return 0;
		}

		if ( !a.compareDocumentPosition || !b.compareDocumentPosition ) {
			return a.compareDocumentPosition ? -1 : 1;
		}

		return a.compareDocumentPosition(b) & 4 ? -1 : 1;
	};

} else {
	sortOrder = function( a, b ) {
		var al, bl,
			ap = [],
			bp = [],
			aup = a.parentNode,
			bup = b.parentNode,
			cur = aup;

		// The nodes are identical, we can exit early
		if ( a === b ) {
			hasDuplicate = true;
			return 0;

		// If the nodes are siblings (or identical) we can do a quick check
		} else if ( aup === bup ) {
			return siblingCheck( a, b );

		// If no parents were found then the nodes are disconnected
		} else if ( !aup ) {
			return -1;

		} else if ( !bup ) {
			return 1;
		}

		// Otherwise they're somewhere else in the tree so we need
		// to build up a full list of the parentNodes for comparison
		while ( cur ) {
			ap.unshift( cur );
			cur = cur.parentNode;
		}

		cur = bup;

		while ( cur ) {
			bp.unshift( cur );
			cur = cur.parentNode;
		}

		al = ap.length;
		bl = bp.length;

		// Start walking down the tree looking for a discrepancy
		for ( var i = 0; i < al && i < bl; i++ ) {
			if ( ap[i] !== bp[i] ) {
				return siblingCheck( ap[i], bp[i] );
			}
		}

		// We ended someplace up the tree so do a sibling check
		return i === al ?
			siblingCheck( a, bp[i], -1 ) :
			siblingCheck( ap[i], b, 1 );
	};

	siblingCheck = function( a, b, ret ) {
		if ( a === b ) {
			return ret;
		}

		var cur = a.nextSibling;

		while ( cur ) {
			if ( cur === b ) {
				return -1;
			}

			cur = cur.nextSibling;
		}

		return 1;
	};
}

// Utility function for retreiving the text value of an array of DOM nodes
Sizzle.getText = function( elems ) {
	var ret = """", elem;

	for ( var i = 0; elems[i]; i++ ) {
		elem = elems[i];

		// Get the text from text nodes and CDATA nodes
		if ( elem.nodeType === 3 || elem.nodeType === 4 ) {
			ret += elem.nodeValue;

		// Traverse everything else, except comment nodes
		} else if ( elem.nodeType !== 8 ) {
			ret += Sizzle.getText( elem.childNodes );
		}
	}

	return ret;
};

// Check to see if the browser returns elements by name when
// querying by getElementById (and provide a workaround)
(function(){
	// We're going to inject a fake input element with a specified name
	var form = document.createElement(""div""),
		id = ""script"" + (new Date()).getTime(),
		root = document.documentElement;

	form.innerHTML = ""<a name='"" + id + ""'/>"";

	// Inject it into the root element, check its status, and remove it quickly
	root.insertBefore( form, root.firstChild );

	// The workaround has to do additional checks after a getElementById
	// Which slows things down for other browsers (hence the branching)
	if ( document.getElementById( id ) ) {
		Expr.find.ID = function( match, context, isXML ) {
			if ( typeof context.getElementById !== ""undefined"" && !isXML ) {
				var m = context.getElementById(match[1]);

				return m ?
					m.id === match[1] || typeof m.getAttributeNode !== ""undefined"" && m.getAttributeNode(""id"").nodeValue === match[1] ?
						[m] :
						undefined :
					[];
			}
		};

		Expr.filter.ID = function( elem, match ) {
			var node = typeof elem.getAttributeNode !== ""undefined"" && elem.getAttributeNode(""id"");

			return elem.nodeType === 1 && node && node.nodeValue === match;
		};
	}

	root.removeChild( form );

	// release memory in IE
	root = form = null;
})();

(function(){
	// Check to see if the browser returns only elements
	// when doing getElementsByTagName(""*"")

	// Create a fake element
	var div = document.createElement(""div"");
	div.appendChild( document.createComment("""") );

	// Make sure no comments are found
	if ( div.getElementsByTagName(""*"").length > 0 ) {
		Expr.find.TAG = function( match, context ) {
			var results = context.getElementsByTagName( match[1] );

			// Filter out possible comments
			if ( match[1] === ""*"" ) {
				var tmp = [];

				for ( var i = 0; results[i]; i++ ) {
					if ( results[i].nodeType === 1 ) {
						tmp.push( results[i] );
					}
				}

				results = tmp;
			}

			return results;
		};
	}

	// Check to see if an attribute returns normalized href attributes
	div.innerHTML = ""<a href='#'></a>"";

	if ( div.firstChild && typeof div.firstChild.getAttribute !== ""undefined"" &&
			div.firstChild.getAttribute(""href"") !== ""#"" ) {

		Expr.attrHandle.href = function( elem ) {
			return elem.getAttribute( ""href"", 2 );
		};
	}

	// release memory in IE
	div = null;
})();

if ( document.querySelectorAll ) {
	(function(){
		var oldSizzle = Sizzle,
			div = document.createElement(""div""),
			id = ""__sizzle__"";

		div.innerHTML = ""<p class='TEST'></p>"";

		// Safari can't handle uppercase or unicode characters when
		// in quirks mode.
		if ( div.querySelectorAll && div.querySelectorAll("".TEST"").length === 0 ) {
			return;
		}
	
		Sizzle = function( query, context, extra, seed ) {
			context = context || document;

			// Make sure that attribute selectors are quoted
			query = query.replace(/\=\s*([^'""\]]*)\s*\]/g, ""='$1']"");

			// Only use querySelectorAll on non-XML documents
			// (ID selectors don't work in non-HTML documents)
			if ( !seed && !Sizzle.isXML(context) ) {
				if ( context.nodeType === 9 ) {
					try {
						return makeArray( context.querySelectorAll(query), extra );
					} catch(qsaError) {}

				// qSA works strangely on Element-rooted queries
				// We can work around this by specifying an extra ID on the root
				// and working up from there (Thanks to Andrew Dupont for the technique)
				// IE 8 doesn't work on object elements
				} else if ( context.nodeType === 1 && context.nodeName.toLowerCase() !== ""object"" ) {
					var old = context.getAttribute( ""id"" ),
						nid = old || id;

					if ( !old ) {
						context.setAttribute( ""id"", nid );
					}

					try {
						return makeArray( context.querySelectorAll( ""#"" + nid + "" "" + query ), extra );

					} catch(pseudoError) {
					} finally {
						if ( !old ) {
							context.removeAttribute( ""id"" );
						}
					}
				}
			}
		
			return oldSizzle(query, context, extra, seed);
		};

		for ( var prop in oldSizzle ) {
			Sizzle[ prop ] = oldSizzle[ prop ];
		}

		// release memory in IE
		div = null;
	})();
}

(function(){
	var html = document.documentElement,
		matches = html.matchesSelector || html.mozMatchesSelector || html.webkitMatchesSelector || html.msMatchesSelector,
		pseudoWorks = false;

	try {
		// This should fail with an exception
		// Gecko does not error, returns false instead
		matches.call( document.documentElement, ""[test!='']:sizzle"" );
	
	} catch( pseudoError ) {
		pseudoWorks = true;
	}

	if ( matches ) {
		Sizzle.matchesSelector = function( node, expr ) {
			// Make sure that attribute selectors are quoted
			expr = expr.replace(/\=\s*([^'""\]]*)\s*\]/g, ""='$1']"");

			if ( !Sizzle.isXML( node ) ) {
				try { 
					if ( pseudoWorks || !Expr.match.PSEUDO.test( expr ) && !/!=/.test( expr ) ) {
						return matches.call( node, expr );
					}
				} catch(e) {}
			}

			return Sizzle(expr, null, null, [node]).length > 0;
		};
	}
})();

(function(){
	var div = document.createElement(""div"");

	div.innerHTML = ""<div class='test e'></div><div class='test'></div>"";

	// Opera can't find a second classname (in 9.6)
	// Also, make sure that getElementsByClassName actually exists
	if ( !div.getElementsByClassName || div.getElementsByClassName(""e"").length === 0 ) {
		return;
	}

	// Safari caches class attributes, doesn't catch changes (in 3.2)
	div.lastChild.className = ""e"";

	if ( div.getElementsByClassName(""e"").length === 1 ) {
		return;
	}
	
	Expr.order.splice(1, 0, ""CLASS"");
	Expr.find.CLASS = function( match, context, isXML ) {
		if ( typeof context.getElementsByClassName !== ""undefined"" && !isXML ) {
			return context.getElementsByClassName(match[1]);
		}
	};

	// release memory in IE
	div = null;
})();

function dirNodeCheck( dir, cur, doneName, checkSet, nodeCheck, isXML ) {
	for ( var i = 0, l = checkSet.length; i < l; i++ ) {
		var elem = checkSet[i];

		if ( elem ) {
			var match = false;

			elem = elem[dir];

			while ( elem ) {
				if ( elem.sizcache === doneName ) {
					match = checkSet[elem.sizset];
					break;
				}

				if ( elem.nodeType === 1 && !isXML ){
					elem.sizcache = doneName;
					elem.sizset = i;
				}

				if ( elem.nodeName.toLowerCase() === cur ) {
					match = elem;
					break;
				}

				elem = elem[dir];
			}

			checkSet[i] = match;
		}
	}
}

function dirCheck( dir, cur, doneName, checkSet, nodeCheck, isXML ) {
	for ( var i = 0, l = checkSet.length; i < l; i++ ) {
		var elem = checkSet[i];

		if ( elem ) {
			var match = false;
			
			elem = elem[dir];

			while ( elem ) {
				if ( elem.sizcache === doneName ) {
					match = checkSet[elem.sizset];
					break;
				}

				if ( elem.nodeType === 1 ) {
					if ( !isXML ) {
						elem.sizcache = doneName;
						elem.sizset = i;
					}

					if ( typeof cur !== ""string"" ) {
						if ( elem === cur ) {
							match = true;
							break;
						}

					} else if ( Sizzle.filter( cur, [elem] ).length > 0 ) {
						match = elem;
						break;
					}
				}

				elem = elem[dir];
			}

			checkSet[i] = match;
		}
	}
}

if ( document.documentElement.contains ) {
	Sizzle.contains = function( a, b ) {
		return a !== b && (a.contains ? a.contains(b) : true);
	};

} else if ( document.documentElement.compareDocumentPosition ) {
	Sizzle.contains = function( a, b ) {
		return !!(a.compareDocumentPosition(b) & 16);
	};

} else {
	Sizzle.contains = function() {
		return false;
	};
}

Sizzle.isXML = function( elem ) {
	// documentElement is verified for cases where it doesn't yet exist
	// (such as loading iframes in IE - #4833) 
	var documentElement = (elem ? elem.ownerDocument || elem : 0).documentElement;

	return documentElement ? documentElement.nodeName !== ""HTML"" : false;
};

var posProcess = function( selector, context ) {
	var match,
		tmpSet = [],
		later = """",
		root = context.nodeType ? [context] : context;

	// Position selectors must be done after the filter
	// And so must :not(positional) so we move all PSEUDOs to the end
	while ( (match = Expr.match.PSEUDO.exec( selector )) ) {
		later += match[0];
		selector = selector.replace( Expr.match.PSEUDO, """" );
	}

	selector = Expr.relative[selector] ? selector + ""*"" : selector;

	for ( var i = 0, l = root.length; i < l; i++ ) {
		Sizzle( selector, root[i], tmpSet );
	}

	return Sizzle.filter( later, tmpSet );
};

// EXPOSE
jQuery.find = Sizzle;
jQuery.expr = Sizzle.selectors;
jQuery.expr["":""] = jQuery.expr.filters;
jQuery.unique = Sizzle.uniqueSort;
jQuery.text = Sizzle.getText;
jQuery.isXMLDoc = Sizzle.isXML;
jQuery.contains = Sizzle.contains;


})();


var runtil = /Until$/,
	rparentsprev = /^(?:parents|prevUntil|prevAll)/,
	// Note: This RegExp should be improved, or likely pulled from Sizzle
	rmultiselector = /,/,
	isSimple = /^.[^:#\[\.,]*$/,
	slice = Array.prototype.slice,
	POS = jQuery.expr.match.POS;

jQuery.fn.extend({
	find: function( selector ) {
		var ret = this.pushStack( """", ""find"", selector ),
			length = 0;

		for ( var i = 0, l = this.length; i < l; i++ ) {
			length = ret.length;
			jQuery.find( selector, this[i], ret );

			if ( i > 0 ) {
				// Make sure that the results are unique
				for ( var n = length; n < ret.length; n++ ) {
					for ( var r = 0; r < length; r++ ) {
						if ( ret[r] === ret[n] ) {
							ret.splice(n--, 1);
							break;
						}
					}
				}
			}
		}

		return ret;
	},

	has: function( target ) {
		var targets = jQuery( target );
		return this.filter(function() {
			for ( var i = 0, l = targets.length; i < l; i++ ) {
				if ( jQuery.contains( this, targets[i] ) ) {
					return true;
				}
			}
		});
	},

	not: function( selector ) {
		return this.pushStack( winnow(this, selector, false), ""not"", selector);
	},

	filter: function( selector ) {
		return this.pushStack( winnow(this, selector, true), ""filter"", selector );
	},
	
	is: function( selector ) {
		return !!selector && jQuery.filter( selector, this ).length > 0;
	},

	closest: function( selectors, context ) {
		var ret = [], i, l, cur = this[0];

		if ( jQuery.isArray( selectors ) ) {
			var match, selector,
				matches = {},
				level = 1;

			if ( cur && selectors.length ) {
				for ( i = 0, l = selectors.length; i < l; i++ ) {
					selector = selectors[i];

					if ( !matches[selector] ) {
						matches[selector] = jQuery.expr.match.POS.test( selector ) ? 
							jQuery( selector, context || this.context ) :
							selector;
					}
				}

				while ( cur && cur.ownerDocument && cur !== context ) {
					for ( selector in matches ) {
						match = matches[selector];

						if ( match.jquery ? match.index(cur) > -1 : jQuery(cur).is(match) ) {
							ret.push({ selector: selector, elem: cur, level: level });
						}
					}

					cur = cur.parentNode;
					level++;
				}
			}

			return ret;
		}

		var pos = POS.test( selectors ) ? 
			jQuery( selectors, context || this.context ) : null;

		for ( i = 0, l = this.length; i < l; i++ ) {
			cur = this[i];

			while ( cur ) {
				if ( pos ? pos.index(cur) > -1 : jQuery.find.matchesSelector(cur, selectors) ) {
					ret.push( cur );
					break;

				} else {
					cur = cur.parentNode;
					if ( !cur || !cur.ownerDocument || cur === context ) {
						break;
					}
				}
			}
		}

		ret = ret.length > 1 ? jQuery.unique(ret) : ret;
		
		return this.pushStack( ret, ""closest"", selectors );
	},
	
	// Determine the position of an element within
	// the matched set of elements
	index: function( elem ) {
		if ( !elem || typeof elem === ""string"" ) {
			return jQuery.inArray( this[0],
				// If it receives a string, the selector is used
				// If it receives nothing, the siblings are used
				elem ? jQuery( elem ) : this.parent().children() );
		}
		// Locate the position of the desired element
		return jQuery.inArray(
			// If it receives a jQuery object, the first element is used
			elem.jquery ? elem[0] : elem, this );
	},

	add: function( selector, context ) {
		var set = typeof selector === ""string"" ?
				jQuery( selector, context || this.context ) :
				jQuery.makeArray( selector ),
			all = jQuery.merge( this.get(), set );

		return this.pushStack( isDisconnected( set[0] ) || isDisconnected( all[0] ) ?
			all :
			jQuery.unique( all ) );
	},

	andSelf: function() {
		return this.add( this.prevObject );
	}
});

// A painfully simple check to see if an element is disconnected
// from a document (should be improved, where feasible).
function isDisconnected( node ) {
	return !node || !node.parentNode || node.parentNode.nodeType === 11;
}

jQuery.each({
	parent: function( elem ) {
		var parent = elem.parentNode;
		return parent && parent.nodeType !== 11 ? parent : null;
	},
	parents: function( elem ) {
		return jQuery.dir( elem, ""parentNode"" );
	},
	parentsUntil: function( elem, i, until ) {
		return jQuery.dir( elem, ""parentNode"", until );
	},
	next: function( elem ) {
		return jQuery.nth( elem, 2, ""nextSibling"" );
	},
	prev: function( elem ) {
		return jQuery.nth( elem, 2, ""previousSibling"" );
	},
	nextAll: function( elem ) {
		return jQuery.dir( elem, ""nextSibling"" );
	},
	prevAll: function( elem ) {
		return jQuery.dir( elem, ""previousSibling"" );
	},
	nextUntil: function( elem, i, until ) {
		return jQuery.dir( elem, ""nextSibling"", until );
	},
	prevUntil: function( elem, i, until ) {
		return jQuery.dir( elem, ""previousSibling"", until );
	},
	siblings: function( elem ) {
		return jQuery.sibling( elem.parentNode.firstChild, elem );
	},
	children: function( elem ) {
		return jQuery.sibling( elem.firstChild );
	},
	contents: function( elem ) {
		return jQuery.nodeName( elem, ""iframe"" ) ?
			elem.contentDocument || elem.contentWindow.document :
			jQuery.makeArray( elem.childNodes );
	}
}, function( name, fn ) {
	jQuery.fn[ name ] = function( until, selector ) {
		var ret = jQuery.map( this, fn, until );
		
		if ( !runtil.test( name ) ) {
			selector = until;
		}

		if ( selector && typeof selector === ""string"" ) {
			ret = jQuery.filter( selector, ret );
		}

		ret = this.length > 1 ? jQuery.unique( ret ) : ret;

		if ( (this.length > 1 || rmultiselector.test( selector )) && rparentsprev.test( name ) ) {
			ret = ret.reverse();
		}

		return this.pushStack( ret, name, slice.call(arguments).join("","") );
	};
});

jQuery.extend({
	filter: function( expr, elems, not ) {
		if ( not ) {
			expr = "":not("" + expr + "")"";
		}

		return elems.length === 1 ?
			jQuery.find.matchesSelector(elems[0], expr) ? [ elems[0] ] : [] :
			jQuery.find.matches(expr, elems);
	},
	
	dir: function( elem, dir, until ) {
		var matched = [],
			cur = elem[ dir ];

		while ( cur && cur.nodeType !== 9 && (until === undefined || cur.nodeType !== 1 || !jQuery( cur ).is( until )) ) {
			if ( cur.nodeType === 1 ) {
				matched.push( cur );
			}
			cur = cur[dir];
		}
		return matched;
	},

	nth: function( cur, result, dir, elem ) {
		result = result || 1;
		var num = 0;

		for ( ; cur; cur = cur[dir] ) {
			if ( cur.nodeType === 1 && ++num === result ) {
				break;
			}
		}

		return cur;
	},

	sibling: function( n, elem ) {
		var r = [];

		for ( ; n; n = n.nextSibling ) {
			if ( n.nodeType === 1 && n !== elem ) {
				r.push( n );
			}
		}

		return r;
	}
});

// Implement the identical functionality for filter and not
function winnow( elements, qualifier, keep ) {
	if ( jQuery.isFunction( qualifier ) ) {
		return jQuery.grep(elements, function( elem, i ) {
			var retVal = !!qualifier.call( elem, i, elem );
			return retVal === keep;
		});

	} else if ( qualifier.nodeType ) {
		return jQuery.grep(elements, function( elem, i ) {
			return (elem === qualifier) === keep;
		});

	} else if ( typeof qualifier === ""string"" ) {
		var filtered = jQuery.grep(elements, function( elem ) {
			return elem.nodeType === 1;
		});

		if ( isSimple.test( qualifier ) ) {
			return jQuery.filter(qualifier, filtered, !keep);
		} else {
			qualifier = jQuery.filter( qualifier, filtered );
		}
	}

	return jQuery.grep(elements, function( elem, i ) {
		return (jQuery.inArray( elem, qualifier ) >= 0) === keep;
	});
}




var rinlinejQuery = / jQuery\d+=""(?:\d+|null)""/g,
	rleadingWhitespace = /^\s+/,
	rxhtmlTag = /<(?!area|br|col|embed|hr|img|input|link|meta|param)(([\w:]+)[^>]*)\/>/ig,
	rtagName = /<([\w:]+)/,
	rtbody = /<tbody/i,
	rhtml = /<|&#?\w+;/,
	rnocache = /<(?:script|object|embed|option|style)/i,
	// checked=""checked"" or checked (html5)
	rchecked = /checked\s*(?:[^=]|=\s*.checked.)/i,
	raction = /\=([^=""'>\s]+\/)>/g,
	wrapMap = {
		option: [ 1, ""<select multiple='multiple'>"", ""</select>"" ],
		legend: [ 1, ""<fieldset>"", ""</fieldset>"" ],
		thead: [ 1, ""<table>"", ""</table>"" ],
		tr: [ 2, ""<table><tbody>"", ""</tbody></table>"" ],
		td: [ 3, ""<table><tbody><tr>"", ""</tr></tbody></table>"" ],
		col: [ 2, ""<table><tbody></tbody><colgroup>"", ""</colgroup></table>"" ],
		area: [ 1, ""<map>"", ""</map>"" ],
		_default: [ 0, """", """" ]
	};

wrapMap.optgroup = wrapMap.option;
wrapMap.tbody = wrapMap.tfoot = wrapMap.colgroup = wrapMap.caption = wrapMap.thead;
wrapMap.th = wrapMap.td;

// IE can't serialize <link> and <script> tags normally
if ( !jQuery.support.htmlSerialize ) {
	wrapMap._default = [ 1, ""div<div>"", ""</div>"" ];
}

jQuery.fn.extend({
	text: function( text ) {
		if ( jQuery.isFunction(text) ) {
			return this.each(function(i) {
				var self = jQuery( this );

				self.text( text.call(this, i, self.text()) );
			});
		}

		if ( typeof text !== ""object"" && text !== undefined ) {
			return this.empty().append( (this[0] && this[0].ownerDocument || document).createTextNode( text ) );
		}

		return jQuery.text( this );
	},

	wrapAll: function( html ) {
		if ( jQuery.isFunction( html ) ) {
			return this.each(function(i) {
				jQuery(this).wrapAll( html.call(this, i) );
			});
		}

		if ( this[0] ) {
			// The elements to wrap the target around
			var wrap = jQuery( html, this[0].ownerDocument ).eq(0).clone(true);

			if ( this[0].parentNode ) {
				wrap.insertBefore( this[0] );
			}

			wrap.map(function() {
				var elem = this;

				while ( elem.firstChild && elem.firstChild.nodeType === 1 ) {
					elem = elem.firstChild;
				}

				return elem;
			}).append(this);
		}

		return this;
	},

	wrapInner: function( html ) {
		if ( jQuery.isFunction( html ) ) {
			return this.each(function(i) {
				jQuery(this).wrapInner( html.call(this, i) );
			});
		}

		return this.each(function() {
			var self = jQuery( this ),
				contents = self.contents();

			if ( contents.length ) {
				contents.wrapAll( html );

			} else {
				self.append( html );
			}
		});
	},

	wrap: function( html ) {
		return this.each(function() {
			jQuery( this ).wrapAll( html );
		});
	},

	unwrap: function() {
		return this.parent().each(function() {
			if ( !jQuery.nodeName( this, ""body"" ) ) {
				jQuery( this ).replaceWith( this.childNodes );
			}
		}).end();
	},

	append: function() {
		return this.domManip(arguments, true, function( elem ) {
			if ( this.nodeType === 1 ) {
				this.appendChild( elem );
			}
		});
	},

	prepend: function() {
		return this.domManip(arguments, true, function( elem ) {
			if ( this.nodeType === 1 ) {
				this.insertBefore( elem, this.firstChild );
			}
		});
	},

	before: function() {
		if ( this[0] && this[0].parentNode ) {
			return this.domManip(arguments, false, function( elem ) {
				this.parentNode.insertBefore( elem, this );
			});
		} else if ( arguments.length ) {
			var set = jQuery(arguments[0]);
			set.push.apply( set, this.toArray() );
			return this.pushStack( set, ""before"", arguments );
		}
	},

	after: function() {
		if ( this[0] && this[0].parentNode ) {
			return this.domManip(arguments, false, function( elem ) {
				this.parentNode.insertBefore( elem, this.nextSibling );
			});
		} else if ( arguments.length ) {
			var set = this.pushStack( this, ""after"", arguments );
			set.push.apply( set, jQuery(arguments[0]).toArray() );
			return set;
		}
	},
	
	// keepData is for internal use only--do not document
	remove: function( selector, keepData ) {
		for ( var i = 0, elem; (elem = this[i]) != null; i++ ) {
			if ( !selector || jQuery.filter( selector, [ elem ] ).length ) {
				if ( !keepData && elem.nodeType === 1 ) {
					jQuery.cleanData( elem.getElementsByTagName(""*"") );
					jQuery.cleanData( [ elem ] );
				}

				if ( elem.parentNode ) {
					 elem.parentNode.removeChild( elem );
				}
			}
		}
		
		return this;
	},

	empty: function() {
		for ( var i = 0, elem; (elem = this[i]) != null; i++ ) {
			// Remove element nodes and prevent memory leaks
			if ( elem.nodeType === 1 ) {
				jQuery.cleanData( elem.getElementsByTagName(""*"") );
			}

			// Remove any remaining nodes
			while ( elem.firstChild ) {
				elem.removeChild( elem.firstChild );
			}
		}
		
		return this;
	},

	clone: function( events ) {
		// Do the clone
		var ret = this.map(function() {
			if ( !jQuery.support.noCloneEvent && !jQuery.isXMLDoc(this) ) {
				// IE copies events bound via attachEvent when
				// using cloneNode. Calling detachEvent on the
				// clone will also remove the events from the orignal
				// In order to get around this, we use innerHTML.
				// Unfortunately, this means some modifications to
				// attributes in IE that are actually only stored
				// as properties will not be copied (such as the
				// the name attribute on an input).
				var html = this.outerHTML,
					ownerDocument = this.ownerDocument;

				if ( !html ) {
					var div = ownerDocument.createElement(""div"");
					div.appendChild( this.cloneNode(true) );
					html = div.innerHTML;
				}

				return jQuery.clean([html.replace(rinlinejQuery, """")
					// Handle the case in IE 8 where action=/test/> self-closes a tag
					.replace(raction, '=""$1"">')
					.replace(rleadingWhitespace, """")], ownerDocument)[0];
			} else {
				return this.cloneNode(true);
			}
		});

		// Copy the events from the original to the clone
		if ( events === true ) {
			cloneCopyEvent( this, ret );
			cloneCopyEvent( this.find(""*""), ret.find(""*"") );
		}

		// Return the cloned set
		return ret;
	},

	html: function( value ) {
		if ( value === undefined ) {
			return this[0] && this[0].nodeType === 1 ?
				this[0].innerHTML.replace(rinlinejQuery, """") :
				null;

		// See if we can take a shortcut and just use innerHTML
		} else if ( typeof value === ""string"" && !rnocache.test( value ) &&
			(jQuery.support.leadingWhitespace || !rleadingWhitespace.test( value )) &&
			!wrapMap[ (rtagName.exec( value ) || ["""", """"])[1].toLowerCase() ] ) {

			value = value.replace(rxhtmlTag, ""<$1></$2>"");

			try {
				for ( var i = 0, l = this.length; i < l; i++ ) {
					// Remove element nodes and prevent memory leaks
					if ( this[i].nodeType === 1 ) {
						jQuery.cleanData( this[i].getElementsByTagName(""*"") );
						this[i].innerHTML = value;
					}
				}

			// If using innerHTML throws an exception, use the fallback method
			} catch(e) {
				this.empty().append( value );
			}

		} else if ( jQuery.isFunction( value ) ) {
			this.each(function(i){
				var self = jQuery( this );

				self.html( value.call(this, i, self.html()) );
			});

		} else {
			this.empty().append( value );
		}

		return this;
	},

	replaceWith: function( value ) {
		if ( this[0] && this[0].parentNode ) {
			// Make sure that the elements are removed from the DOM before they are inserted
			// this can help fix replacing a parent with child elements
			if ( jQuery.isFunction( value ) ) {
				return this.each(function(i) {
					var self = jQuery(this), old = self.html();
					self.replaceWith( value.call( this, i, old ) );
				});
			}

			if ( typeof value !== ""string"" ) {
				value = jQuery( value ).detach();
			}

			return this.each(function() {
				var next = this.nextSibling,
					parent = this.parentNode;

				jQuery( this ).remove();

				if ( next ) {
					jQuery(next).before( value );
				} else {
					jQuery(parent).append( value );
				}
			});
		} else {
			return this.pushStack( jQuery(jQuery.isFunction(value) ? value() : value), ""replaceWith"", value );
		}
	},

	detach: function( selector ) {
		return this.remove( selector, true );
	},

	domManip: function( args, table, callback ) {
		var results, first, fragment, parent,
			value = args[0],
			scripts = [];

		// We can't cloneNode fragments that contain checked, in WebKit
		if ( !jQuery.support.checkClone && arguments.length === 3 && typeof value === ""string"" && rchecked.test( value ) ) {
			return this.each(function() {
				jQuery(this).domManip( args, table, callback, true );
			});
		}

		if ( jQuery.isFunction(value) ) {
			return this.each(function(i) {
				var self = jQuery(this);
				args[0] = value.call(this, i, table ? self.html() : undefined);
				self.domManip( args, table, callback );
			});
		}

		if ( this[0] ) {
			parent = value && value.parentNode;

			// If we're in a fragment, just use that instead of building a new one
			if ( jQuery.support.parentNode && parent && parent.nodeType === 11 && parent.childNodes.length === this.length ) {
				results = { fragment: parent };

			} else {
				results = jQuery.buildFragment( args, this, scripts );
			}
			
			fragment = results.fragment;
			
			if ( fragment.childNodes.length === 1 ) {
				first = fragment = fragment.firstChild;
			} else {
				first = fragment.firstChild;
			}

			if ( first ) {
				table = table && jQuery.nodeName( first, ""tr"" );

				for ( var i = 0, l = this.length; i < l; i++ ) {
					callback.call(
						table ?
							root(this[i], first) :
							this[i],
						i > 0 || results.cacheable || this.length > 1  ?
							fragment.cloneNode(true) :
							fragment
					);
				}
			}

			if ( scripts.length ) {
				jQuery.each( scripts, evalScript );
			}
		}

		return this;
	}
});

function root( elem, cur ) {
	return jQuery.nodeName(elem, ""table"") ?
		(elem.getElementsByTagName(""tbody"")[0] ||
		elem.appendChild(elem.ownerDocument.createElement(""tbody""))) :
		elem;
}

function cloneCopyEvent(orig, ret) {
	var i = 0;

	ret.each(function() {
		if ( this.nodeName !== (orig[i] && orig[i].nodeName) ) {
			return;
		}

		var oldData = jQuery.data( orig[i++] ),
			curData = jQuery.data( this, oldData ),
			events = oldData && oldData.events;

		if ( events ) {
			delete curData.handle;
			curData.events = {};

			for ( var type in events ) {
				for ( var handler in events[ type ] ) {
					jQuery.event.add( this, type, events[ type ][ handler ], events[ type ][ handler ].data );
				}
			}
		}
	});
}

jQuery.buildFragment = function( args, nodes, scripts ) {
	var fragment, cacheable, cacheresults,
		doc = (nodes && nodes[0] ? nodes[0].ownerDocument || nodes[0] : document);

	// Only cache ""small"" (1/2 KB) strings that are associated with the main document
	// Cloning options loses the selected state, so don't cache them
	// IE 6 doesn't like it when you put <object> or <embed> elements in a fragment
	// Also, WebKit does not clone 'checked' attributes on cloneNode, so don't cache
	if ( args.length === 1 && typeof args[0] === ""string"" && args[0].length < 512 && doc === document &&
		!rnocache.test( args[0] ) && (jQuery.support.checkClone || !rchecked.test( args[0] )) ) {

		cacheable = true;
		cacheresults = jQuery.fragments[ args[0] ];
		if ( cacheresults ) {
			if ( cacheresults !== 1 ) {
				fragment = cacheresults;
			}
		}
	}

	if ( !fragment ) {
		fragment = doc.createDocumentFragment();
		jQuery.clean( args, doc, fragment, scripts );
	}

	if ( cacheable ) {
		jQuery.fragments[ args[0] ] = cacheresults ? fragment : 1;
	}

	return { fragment: fragment, cacheable: cacheable };
};

jQuery.fragments = {};

jQuery.each({
	appendTo: ""append"",
	prependTo: ""prepend"",
	insertBefore: ""before"",
	insertAfter: ""after"",
	replaceAll: ""replaceWith""
}, function( name, original ) {
	jQuery.fn[ name ] = function( selector ) {
		var ret = [],
			insert = jQuery( selector ),
			parent = this.length === 1 && this[0].parentNode;
		
		if ( parent && parent.nodeType === 11 && parent.childNodes.length === 1 && insert.length === 1 ) {
			insert[ original ]( this[0] );
			return this;
			
		} else {
			for ( var i = 0, l = insert.length; i < l; i++ ) {
				var elems = (i > 0 ? this.clone(true) : this).get();
				jQuery( insert[i] )[ original ]( elems );
				ret = ret.concat( elems );
			}
		
			return this.pushStack( ret, name, insert.selector );
		}
	};
});

jQuery.extend({
	clean: function( elems, context, fragment, scripts ) {
		context = context || document;

		// !context.createElement fails in IE with an error but returns typeof 'object'
		if ( typeof context.createElement === ""undefined"" ) {
			context = context.ownerDocument || context[0] && context[0].ownerDocument || document;
		}

		var ret = [];

		for ( var i = 0, elem; (elem = elems[i]) != null; i++ ) {
			if ( typeof elem === ""number"" ) {
				elem += """";
			}

			if ( !elem ) {
				continue;
			}

			// Convert html string into DOM nodes
			if ( typeof elem === ""string"" && !rhtml.test( elem ) ) {
				elem = context.createTextNode( elem );

			} else if ( typeof elem === ""string"" ) {
				// Fix ""XHTML""-style tags in all browsers
				elem = elem.replace(rxhtmlTag, ""<$1></$2>"");

				// Trim whitespace, otherwise indexOf won't work as expected
				var tag = (rtagName.exec( elem ) || ["""", """"])[1].toLowerCase(),
					wrap = wrapMap[ tag ] || wrapMap._default,
					depth = wrap[0],
					div = context.createElement(""div"");

				// Go to html and back, then peel off extra wrappers
				div.innerHTML = wrap[1] + elem + wrap[2];

				// Move to the right depth
				while ( depth-- ) {
					div = div.lastChild;
				}

				// Remove IE's autoinserted <tbody> from table fragments
				if ( !jQuery.support.tbody ) {

					// String was a <table>, *may* have spurious <tbody>
					var hasBody = rtbody.test(elem),
						tbody = tag === ""table"" && !hasBody ?
							div.firstChild && div.firstChild.childNodes :

							// String was a bare <thead> or <tfoot>
							wrap[1] === ""<table>"" && !hasBody ?
								div.childNodes :
								[];

					for ( var j = tbody.length - 1; j >= 0 ; --j ) {
						if ( jQuery.nodeName( tbody[ j ], ""tbody"" ) && !tbody[ j ].childNodes.length ) {
							tbody[ j ].parentNode.removeChild( tbody[ j ] );
						}
					}

				}

				// IE completely kills leading whitespace when innerHTML is used
				if ( !jQuery.support.leadingWhitespace && rleadingWhitespace.test( elem ) ) {
					div.insertBefore( context.createTextNode( rleadingWhitespace.exec(elem)[0] ), div.firstChild );
				}

				elem = div.childNodes;
			}

			if ( elem.nodeType ) {
				ret.push( elem );
			} else {
				ret = jQuery.merge( ret, elem );
			}
		}

		if ( fragment ) {
			for ( i = 0; ret[i]; i++ ) {
				if ( scripts && jQuery.nodeName( ret[i], ""script"" ) && (!ret[i].type || ret[i].type.toLowerCase() === ""text/javascript"") ) {
					scripts.push( ret[i].parentNode ? ret[i].parentNode.removeChild( ret[i] ) : ret[i] );
				
				} else {
					if ( ret[i].nodeType === 1 ) {
						ret.splice.apply( ret, [i + 1, 0].concat(jQuery.makeArray(ret[i].getElementsByTagName(""script""))) );
					}
					fragment.appendChild( ret[i] );
				}
			}
		}

		return ret;
	},
	
	cleanData: function( elems ) {
		var data, id, cache = jQuery.cache,
			special = jQuery.event.special,
			deleteExpando = jQuery.support.deleteExpando;
		
		for ( var i = 0, elem; (elem = elems[i]) != null; i++ ) {
			if ( elem.nodeName && jQuery.noData[elem.nodeName.toLowerCase()] ) {
				continue;
			}

			id = elem[ jQuery.expando ];
			
			if ( id ) {
				data = cache[ id ];
				
				if ( data && data.events ) {
					for ( var type in data.events ) {
						if ( special[ type ] ) {
							jQuery.event.remove( elem, type );

						} else {
							jQuery.removeEvent( elem, type, data.handle );
						}
					}
				}
				
				if ( deleteExpando ) {
					delete elem[ jQuery.expando ];

				} else if ( elem.removeAttribute ) {
					elem.removeAttribute( jQuery.expando );
				}
				
				delete cache[ id ];
			}
		}
	}
});

function evalScript( i, elem ) {
	if ( elem.src ) {
		jQuery.ajax({
			url: elem.src,
			async: false,
			dataType: ""script""
		});
	} else {
		jQuery.globalEval( elem.text || elem.textContent || elem.innerHTML || """" );
	}

	if ( elem.parentNode ) {
		elem.parentNode.removeChild( elem );
	}
}




var ralpha = /alpha\([^)]*\)/i,
	ropacity = /opacity=([^)]*)/,
	rdashAlpha = /-([a-z])/ig,
	rupper = /([A-Z])/g,
	rnumpx = /^-?\d+(?:px)?$/i,
	rnum = /^-?\d/,

	cssShow = { position: ""absolute"", visibility: ""hidden"", display: ""block"" },
	cssWidth = [ ""Left"", ""Right"" ],
	cssHeight = [ ""Top"", ""Bottom"" ],
	curCSS,

	getComputedStyle,
	currentStyle,

	fcamelCase = function( all, letter ) {
		return letter.toUpperCase();
	};

jQuery.fn.css = function( name, value ) {
	// Setting 'undefined' is a no-op
	if ( arguments.length === 2 && value === undefined ) {
		return this;
	}

	return jQuery.access( this, name, value, true, function( elem, name, value ) {
		return value !== undefined ?
			jQuery.style( elem, name, value ) :
			jQuery.css( elem, name );
	});
};

jQuery.extend({
	// Add in style property hooks for overriding the default
	// behavior of getting and setting a style property
	cssHooks: {
		opacity: {
			get: function( elem, computed ) {
				if ( computed ) {
					// We should always get a number back from opacity
					var ret = curCSS( elem, ""opacity"", ""opacity"" );
					return ret === """" ? ""1"" : ret;

				} else {
					return elem.style.opacity;
				}
			}
		}
	},

	// Exclude the following css properties to add px
	cssNumber: {
		""zIndex"": true,
		""fontWeight"": true,
		""opacity"": true,
		""zoom"": true,
		""lineHeight"": true
	},

	// Add in properties whose names you wish to fix before
	// setting or getting the value
	cssProps: {
		// normalize float css property
		""float"": jQuery.support.cssFloat ? ""cssFloat"" : ""styleFloat""
	},

	// Get and set the style property on a DOM Node
	style: function( elem, name, value, extra ) {
		// Don't set styles on text and comment nodes
		if ( !elem || elem.nodeType === 3 || elem.nodeType === 8 || !elem.style ) {
			return;
		}

		// Make sure that we're working with the right name
		var ret, origName = jQuery.camelCase( name ),
			style = elem.style, hooks = jQuery.cssHooks[ origName ];

		name = jQuery.cssProps[ origName ] || origName;

		// Check if we're setting a value
		if ( value !== undefined ) {
			// Make sure that NaN and null values aren't set. See: #7116
			if ( typeof value === ""number"" && isNaN( value ) || value == null ) {
				return;
			}

			// If a number was passed in, add 'px' to the (except for certain CSS properties)
			if ( typeof value === ""number"" && !jQuery.cssNumber[ origName ] ) {
				value += ""px"";
			}

			// If a hook was provided, use that value, otherwise just set the specified value
			if ( !hooks || !(""set"" in hooks) || (value = hooks.set( elem, value )) !== undefined ) {
				// Wrapped to prevent IE from throwing errors when 'invalid' values are provided
				// Fixes bug #5509
				try {
					style[ name ] = value;
				} catch(e) {}
			}

		} else {
			// If a hook was provided get the non-computed value from there
			if ( hooks && ""get"" in hooks && (ret = hooks.get( elem, false, extra )) !== undefined ) {
				return ret;
			}

			// Otherwise just get the value from the style object
			return style[ name ];
		}
	},

	css: function( elem, name, extra ) {
		// Make sure that we're working with the right name
		var ret, origName = jQuery.camelCase( name ),
			hooks = jQuery.cssHooks[ origName ];

		name = jQuery.cssProps[ origName ] || origName;

		// If a hook was provided get the computed value from there
		if ( hooks && ""get"" in hooks && (ret = hooks.get( elem, true, extra )) !== undefined ) {
			return ret;

		// Otherwise, if a way to get the computed value exists, use that
		} else if ( curCSS ) {
			return curCSS( elem, name, origName );
		}
	},

	// A method for quickly swapping in/out CSS properties to get correct calculations
	swap: function( elem, options, callback ) {
		var old = {};

		// Remember the old values, and insert the new ones
		for ( var name in options ) {
			old[ name ] = elem.style[ name ];
			elem.style[ name ] = options[ name ];
		}

		callback.call( elem );

		// Revert the old values
		for ( name in options ) {
			elem.style[ name ] = old[ name ];
		}
	},

	camelCase: function( string ) {
		return string.replace( rdashAlpha, fcamelCase );
	}
});

// DEPRECATED, Use jQuery.css() instead
jQuery.curCSS = jQuery.css;

jQuery.each([""height"", ""width""], function( i, name ) {
	jQuery.cssHooks[ name ] = {
		get: function( elem, computed, extra ) {
			var val;

			if ( computed ) {
				if ( elem.offsetWidth !== 0 ) {
					val = getWH( elem, name, extra );

				} else {
					jQuery.swap( elem, cssShow, function() {
						val = getWH( elem, name, extra );
					});
				}

				if ( val <= 0 ) {
					val = curCSS( elem, name, name );

					if ( val === ""0px"" && currentStyle ) {
						val = currentStyle( elem, name, name );
					}

					if ( val != null ) {
						// Should return ""auto"" instead of 0, use 0 for
						// temporary backwards-compat
						return val === """" || val === ""auto"" ? ""0px"" : val;
					}
				}

				if ( val < 0 || val == null ) {
					val = elem.style[ name ];

					// Should return ""auto"" instead of 0, use 0 for
					// temporary backwards-compat
					return val === """" || val === ""auto"" ? ""0px"" : val;
				}

				return typeof val === ""string"" ? val : val + ""px"";
			}
		},

		set: function( elem, value ) {
			if ( rnumpx.test( value ) ) {
				// ignore negative width and height values #1599
				value = parseFloat(value);

				if ( value >= 0 ) {
					return value + ""px"";
				}

			} else {
				return value;
			}
		}
	};
});

if ( !jQuery.support.opacity ) {
	jQuery.cssHooks.opacity = {
		get: function( elem, computed ) {
			// IE uses filters for opacity
			return ropacity.test((computed && elem.currentStyle ? elem.currentStyle.filter : elem.style.filter) || """") ?
				(parseFloat(RegExp.$1) / 100) + """" :
				computed ? ""1"" : """";
		},

		set: function( elem, value ) {
			var style = elem.style;

			// IE has trouble with opacity if it does not have layout
			// Force it by setting the zoom level
			style.zoom = 1;

			// Set the alpha filter to set the opacity
			var opacity = jQuery.isNaN(value) ?
				"""" :
				""alpha(opacity="" + value * 100 + "")"",
				filter = style.filter || """";

			style.filter = ralpha.test(filter) ?
				filter.replace(ralpha, opacity) :
				style.filter + ' ' + opacity;
		}
	};
}

if ( document.defaultView && document.defaultView.getComputedStyle ) {
	getComputedStyle = function( elem, newName, name ) {
		var ret, defaultView, computedStyle;

		name = name.replace( rupper, ""-$1"" ).toLowerCase();

		if ( !(defaultView = elem.ownerDocument.defaultView) ) {
			return undefined;
		}

		if ( (computedStyle = defaultView.getComputedStyle( elem, null )) ) {
			ret = computedStyle.getPropertyValue( name );
			if ( ret === """" && !jQuery.contains( elem.ownerDocument.documentElement, elem ) ) {
				ret = jQuery.style( elem, name );
			}
		}

		return ret;
	};
}

if ( document.documentElement.currentStyle ) {
	currentStyle = function( elem, name ) {
		var left, rsLeft,
			ret = elem.currentStyle && elem.currentStyle[ name ],
			style = elem.style;

		// From the awesome hack by Dean Edwards
		// http://erik.eae.net/archives/2007/07/27/18.54.15/#comment-102291

		// If we're not dealing with a regular pixel number
		// but a number that has a weird ending, we need to convert it to pixels
		if ( !rnumpx.test( ret ) && rnum.test( ret ) ) {
			// Remember the original values
			left = style.left;
			rsLeft = elem.runtimeStyle.left;

			// Put in the new values to get a computed value out
			elem.runtimeStyle.left = elem.currentStyle.left;
			style.left = name === ""fontSize"" ? ""1em"" : (ret || 0);
			ret = style.pixelLeft + ""px"";

			// Revert the changed values
			style.left = left;
			elem.runtimeStyle.left = rsLeft;
		}

		return ret === """" ? ""auto"" : ret;
	};
}

curCSS = getComputedStyle || currentStyle;

function getWH( elem, name, extra ) {
	var which = name === ""width"" ? cssWidth : cssHeight,
		val = name === ""width"" ? elem.offsetWidth : elem.offsetHeight;

	if ( extra === ""border"" ) {
		return val;
	}

	jQuery.each( which, function() {
		if ( !extra ) {
			val -= parseFloat(jQuery.css( elem, ""padding"" + this )) || 0;
		}

		if ( extra === ""margin"" ) {
			val += parseFloat(jQuery.css( elem, ""margin"" + this )) || 0;

		} else {
			val -= parseFloat(jQuery.css( elem, ""border"" + this + ""Width"" )) || 0;
		}
	});

	return val;
}

if ( jQuery.expr && jQuery.expr.filters ) {
	jQuery.expr.filters.hidden = function( elem ) {
		var width = elem.offsetWidth,
			height = elem.offsetHeight;

		return (width === 0 && height === 0) || (!jQuery.support.reliableHiddenOffsets && (elem.style.display || jQuery.css( elem, ""display"" )) === ""none"");
	};

	jQuery.expr.filters.visible = function( elem ) {
		return !jQuery.expr.filters.hidden( elem );
	};
}




var jsc = jQuery.now(),
	rscript = /<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>/gi,
	rselectTextarea = /^(?:select|textarea)/i,
	rinput = /^(?:color|date|datetime|email|hidden|month|number|password|range|search|tel|text|time|url|week)$/i,
	rnoContent = /^(?:GET|HEAD)$/,
	rbracket = /\[\]$/,
	jsre = /\=\?(&|$)/,
	rquery = /\?/,
	rts = /([?&])_=[^&]*/,
	rurl = /^(\w+:)?\/\/([^\/?#]+)/,
	r20 = /%20/g,
	rhash = /#.*$/,

	// Keep a copy of the old load method
	_load = jQuery.fn.load;

jQuery.fn.extend({
	load: function( url, params, callback ) {
		if ( typeof url !== ""string"" && _load ) {
			return _load.apply( this, arguments );

		// Don't do a request if no elements are being requested
		} else if ( !this.length ) {
			return this;
		}

		var off = url.indexOf("" "");
		if ( off >= 0 ) {
			var selector = url.slice(off, url.length);
			url = url.slice(0, off);
		}

		// Default to a GET request
		var type = ""GET"";

		// If the second parameter was provided
		if ( params ) {
			// If it's a function
			if ( jQuery.isFunction( params ) ) {
				// We assume that it's the callback
				callback = params;
				params = null;

			// Otherwise, build a param string
			} else if ( typeof params === ""object"" ) {
				params = jQuery.param( params, jQuery.ajaxSettings.traditional );
				type = ""POST"";
			}
		}

		var self = this;

		// Request the remote document
		jQuery.ajax({
			url: url,
			type: type,
			dataType: ""html"",
			data: params,
			complete: function( res, status ) {
				// If successful, inject the HTML into all the matched elements
				if ( status === ""success"" || status === ""notmodified"" ) {
					// See if a selector was specified
					self.html( selector ?
						// Create a dummy div to hold the results
						jQuery(""<div>"")
							// inject the contents of the document in, removing the scripts
							// to avoid any 'Permission Denied' errors in IE
							.append(res.responseText.replace(rscript, """"))

							// Locate the specified elements
							.find(selector) :

						// If not, just inject the full result
						res.responseText );
				}

				if ( callback ) {
					self.each( callback, [res.responseText, status, res] );
				}
			}
		});

		return this;
	},

	serialize: function() {
		return jQuery.param(this.serializeArray());
	},

	serializeArray: function() {
		return this.map(function() {
			return this.elements ? jQuery.makeArray(this.elements) : this;
		})
		.filter(function() {
			return this.name && !this.disabled &&
				(this.checked || rselectTextarea.test(this.nodeName) ||
					rinput.test(this.type));
		})
		.map(function( i, elem ) {
			var val = jQuery(this).val();

			return val == null ?
				null :
				jQuery.isArray(val) ?
					jQuery.map( val, function( val, i ) {
						return { name: elem.name, value: val };
					}) :
					{ name: elem.name, value: val };
		}).get();
	}
});

// Attach a bunch of functions for handling common AJAX events
jQuery.each( ""ajaxStart ajaxStop ajaxComplete ajaxError ajaxSuccess ajaxSend"".split("" ""), function( i, o ) {
	jQuery.fn[o] = function( f ) {
		return this.bind(o, f);
	};
});

jQuery.extend({
	get: function( url, data, callback, type ) {
		// shift arguments if data argument was omited
		if ( jQuery.isFunction( data ) ) {
			type = type || callback;
			callback = data;
			data = null;
		}

		return jQuery.ajax({
			type: ""GET"",
			url: url,
			data: data,
			success: callback,
			dataType: type
		});
	},

	getScript: function( url, callback ) {
		return jQuery.get(url, null, callback, ""script"");
	},

	getJSON: function( url, data, callback ) {
		return jQuery.get(url, data, callback, ""json"");
	},

	post: function( url, data, callback, type ) {
		// shift arguments if data argument was omited
		if ( jQuery.isFunction( data ) ) {
			type = type || callback;
			callback = data;
			data = {};
		}

		return jQuery.ajax({
			type: ""POST"",
			url: url,
			data: data,
			success: callback,
			dataType: type
		});
	},

	ajaxSetup: function( settings ) {
		jQuery.extend( jQuery.ajaxSettings, settings );
	},

	ajaxSettings: {
		url: location.href,
		global: true,
		type: ""GET"",
		contentType: ""application/x-www-form-urlencoded"",
		processData: true,
		async: true,
		/*
		timeout: 0,
		data: null,
		username: null,
		password: null,
		traditional: false,
		*/
		// This function can be overriden by calling jQuery.ajaxSetup
		xhr: function() {
			return new window.XMLHttpRequest();
		},
		accepts: {
			xml: ""application/xml, text/xml"",
			html: ""text/html"",
			script: ""text/javascript, application/javascript"",
			json: ""application/json, text/javascript"",
			text: ""text/plain"",
			_default: ""*/*""
		}
	},

	ajax: function( origSettings ) {
		var s = jQuery.extend(true, {}, jQuery.ajaxSettings, origSettings),
			jsonp, status, data, type = s.type.toUpperCase(), noContent = rnoContent.test(type);

		s.url = s.url.replace( rhash, """" );

		// Use original (not extended) context object if it was provided
		s.context = origSettings && origSettings.context != null ? origSettings.context : s;

		// convert data if not already a string
		if ( s.data && s.processData && typeof s.data !== ""string"" ) {
			s.data = jQuery.param( s.data, s.traditional );
		}

		// Handle JSONP Parameter Callbacks
		if ( s.dataType === ""jsonp"" ) {
			if ( type === ""GET"" ) {
				if ( !jsre.test( s.url ) ) {
					s.url += (rquery.test( s.url ) ? ""&"" : ""?"") + (s.jsonp || ""callback"") + ""=?"";
				}
			} else if ( !s.data || !jsre.test(s.data) ) {
				s.data = (s.data ? s.data + ""&"" : """") + (s.jsonp || ""callback"") + ""=?"";
			}
			s.dataType = ""json"";
		}

		// Build temporary JSONP function
		if ( s.dataType === ""json"" && (s.data && jsre.test(s.data) || jsre.test(s.url)) ) {
			jsonp = s.jsonpCallback || (""jsonp"" + jsc++);

			// Replace the =? sequence both in the query string and the data
			if ( s.data ) {
				s.data = (s.data + """").replace(jsre, ""="" + jsonp + ""$1"");
			}

			s.url = s.url.replace(jsre, ""="" + jsonp + ""$1"");

			// We need to make sure
			// that a JSONP style response is executed properly
			s.dataType = ""script"";

			// Handle JSONP-style loading
			var customJsonp = window[ jsonp ];

			window[ jsonp ] = function( tmp ) {
				if ( jQuery.isFunction( customJsonp ) ) {
					customJsonp( tmp );

				} else {
					// Garbage collect
					window[ jsonp ] = undefined;

					try {
						delete window[ jsonp ];
					} catch( jsonpError ) {}
				}

				data = tmp;
				jQuery.handleSuccess( s, xhr, status, data );
				jQuery.handleComplete( s, xhr, status, data );
				
				if ( head ) {
					head.removeChild( script );
				}
			};
		}

		if ( s.dataType === ""script"" && s.cache === null ) {
			s.cache = false;
		}

		if ( s.cache === false && noContent ) {
			var ts = jQuery.now();

			// try replacing _= if it is there
			var ret = s.url.replace(rts, ""$1_="" + ts);

			// if nothing was replaced, add timestamp to the end
			s.url = ret + ((ret === s.url) ? (rquery.test(s.url) ? ""&"" : ""?"") + ""_="" + ts : """");
		}

		// If data is available, append data to url for GET/HEAD requests
		if ( s.data && noContent ) {
			s.url += (rquery.test(s.url) ? ""&"" : ""?"") + s.data;
		}

		// Watch for a new set of requests
		if ( s.global && jQuery.active++ === 0 ) {
			jQuery.event.trigger( ""ajaxStart"" );
		}

		// Matches an absolute URL, and saves the domain
		var parts = rurl.exec( s.url ),
			remote = parts && (parts[1] && parts[1].toLowerCase() !== location.protocol || parts[2].toLowerCase() !== location.host);

		// If we're requesting a remote document
		// and trying to load JSON or Script with a GET
		if ( s.dataType === ""script"" && type === ""GET"" && remote ) {
			var head = document.getElementsByTagName(""head"")[0] || document.documentElement;
			var script = document.createElement(""script"");
			if ( s.scriptCharset ) {
				script.charset = s.scriptCharset;
			}
			script.src = s.url;

			// Handle Script loading
			if ( !jsonp ) {
				var done = false;

				// Attach handlers for all browsers
				script.onload = script.onreadystatechange = function() {
					if ( !done && (!this.readyState ||
							this.readyState === ""loaded"" || this.readyState === ""complete"") ) {
						done = true;
						jQuery.handleSuccess( s, xhr, status, data );
						jQuery.handleComplete( s, xhr, status, data );

						// Handle memory leak in IE
						script.onload = script.onreadystatechange = null;
						if ( head && script.parentNode ) {
							head.removeChild( script );
						}
					}
				};
			}

			// Use insertBefore instead of appendChild  to circumvent an IE6 bug.
			// This arises when a base node is used (#2709 and #4378).
			head.insertBefore( script, head.firstChild );

			// We handle everything using the script element injection
			return undefined;
		}

		var requestDone = false;

		// Create the request object
		var xhr = s.xhr();

		if ( !xhr ) {
			return;
		}

		// Open the socket
		// Passing null username, generates a login popup on Opera (#2865)
		if ( s.username ) {
			xhr.open(type, s.url, s.async, s.username, s.password);
		} else {
			xhr.open(type, s.url, s.async);
		}

		// Need an extra try/catch for cross domain requests in Firefox 3
		try {
			// Set content-type if data specified and content-body is valid for this type
			if ( (s.data != null && !noContent) || (origSettings && origSettings.contentType) ) {
				xhr.setRequestHeader(""Content-Type"", s.contentType);
			}

			// Set the If-Modified-Since and/or If-None-Match header, if in ifModified mode.
			if ( s.ifModified ) {
				if ( jQuery.lastModified[s.url] ) {
					xhr.setRequestHeader(""If-Modified-Since"", jQuery.lastModified[s.url]);
				}

				if ( jQuery.etag[s.url] ) {
					xhr.setRequestHeader(""If-None-Match"", jQuery.etag[s.url]);
				}
			}

			// Set header so the called script knows that it's an XMLHttpRequest
			// Only send the header if it's not a remote XHR
			if ( !remote ) {
				xhr.setRequestHeader(""X-Requested-With"", ""XMLHttpRequest"");
			}

			// Set the Accepts header for the server, depending on the dataType
			xhr.setRequestHeader(""Accept"", s.dataType && s.accepts[ s.dataType ] ?
				s.accepts[ s.dataType ] + "", */*; q=0.01"" :
				s.accepts._default );
		} catch( headerError ) {}

		// Allow custom headers/mimetypes and early abort
		if ( s.beforeSend && s.beforeSend.call(s.context, xhr, s) === false ) {
			// Handle the global AJAX counter
			if ( s.global && jQuery.active-- === 1 ) {
				jQuery.event.trigger( ""ajaxStop"" );
			}

			// close opended socket
			xhr.abort();
			return false;
		}

		if ( s.global ) {
			jQuery.triggerGlobal( s, ""ajaxSend"", [xhr, s] );
		}

		// Wait for a response to come back
		var onreadystatechange = xhr.onreadystatechange = function( isTimeout ) {
			// The request was aborted
			if ( !xhr || xhr.readyState === 0 || isTimeout === ""abort"" ) {
				// Opera doesn't call onreadystatechange before this point
				// so we simulate the call
				if ( !requestDone ) {
					jQuery.handleComplete( s, xhr, status, data );
				}

				requestDone = true;
				if ( xhr ) {
					xhr.onreadystatechange = jQuery.noop;
				}

			// The transfer is complete and the data is available, or the request timed out
			} else if ( !requestDone && xhr && (xhr.readyState === 4 || isTimeout === ""timeout"") ) {
				requestDone = true;
				xhr.onreadystatechange = jQuery.noop;

				status = isTimeout === ""timeout"" ?
					""timeout"" :
					!jQuery.httpSuccess( xhr ) ?
						""error"" :
						s.ifModified && jQuery.httpNotModified( xhr, s.url ) ?
							""notmodified"" :
							""success"";

				var errMsg;

				if ( status === ""success"" ) {
					// Watch for, and catch, XML document parse errors
					try {
						// process the data (runs the xml through httpData regardless of callback)
						data = jQuery.httpData( xhr, s.dataType, s );
					} catch( parserError ) {
						status = ""parsererror"";
						errMsg = parserError;
					}
				}

				// Make sure that the request was successful or notmodified
				if ( status === ""success"" || status === ""notmodified"" ) {
					// JSONP handles its own success callback
					if ( !jsonp ) {
						jQuery.handleSuccess( s, xhr, status, data );
					}
				} else {
					jQuery.handleError( s, xhr, status, errMsg );
				}

				// Fire the complete handlers
				if ( !jsonp ) {
					jQuery.handleComplete( s, xhr, status, data );
				}

				if ( isTimeout === ""timeout"" ) {
					xhr.abort();
				}

				// Stop memory leaks
				if ( s.async ) {
					xhr = null;
				}
			}
		};

		// Override the abort handler, if we can (IE 6 doesn't allow it, but that's OK)
		// Opera doesn't fire onreadystatechange at all on abort
		try {
			var oldAbort = xhr.abort;
			xhr.abort = function() {
				if ( xhr ) {
					// oldAbort has no call property in IE7 so
					// just do it this way, which works in all
					// browsers
					Function.prototype.call.call( oldAbort, xhr );
				}

				onreadystatechange( ""abort"" );
			};
		} catch( abortError ) {}

		// Timeout checker
		if ( s.async && s.timeout > 0 ) {
			setTimeout(function() {
				// Check to see if the request is still happening
				if ( xhr && !requestDone ) {
					onreadystatechange( ""timeout"" );
				}
			}, s.timeout);
		}

		// Send the data
		try {
			xhr.send( noContent || s.data == null ? null : s.data );

		} catch( sendError ) {
			jQuery.handleError( s, xhr, null, sendError );

			// Fire the complete handlers
			jQuery.handleComplete( s, xhr, status, data );
		}

		// firefox 1.5 doesn't fire statechange for sync requests
		if ( !s.async ) {
			onreadystatechange();
		}

		// return XMLHttpRequest to allow aborting the request etc.
		return xhr;
	},

	// Serialize an array of form elements or a set of
	// key/values into a query string
	param: function( a, traditional ) {
		var s = [],
			add = function( key, value ) {
				// If value is a function, invoke it and return its value
				value = jQuery.isFunction(value) ? value() : value;
				s[ s.length ] = encodeURIComponent(key) + ""="" + encodeURIComponent(value);
			};
		
		// Set traditional to true for jQuery <= 1.3.2 behavior.
		if ( traditional === undefined ) {
			traditional = jQuery.ajaxSettings.traditional;
		}
		
		// If an array was passed in, assume that it is an array of form elements.
		if ( jQuery.isArray(a) || a.jquery ) {
			// Serialize the form elements
			jQuery.each( a, function() {
				add( this.name, this.value );
			});
			
		} else {
			// If traditional, encode the ""old"" way (the way 1.3.2 or older
			// did it), otherwise encode params recursively.
			for ( var prefix in a ) {
				buildParams( prefix, a[prefix], traditional, add );
			}
		}

		// Return the resulting serialization
		return s.join(""&"").replace(r20, ""+"");
	}
});

function buildParams( prefix, obj, traditional, add ) {
	if ( jQuery.isArray(obj) && obj.length ) {
		// Serialize array item.
		jQuery.each( obj, function( i, v ) {
			if ( traditional || rbracket.test( prefix ) ) {
				// Treat each array item as a scalar.
				add( prefix, v );

			} else {
				// If array item is non-scalar (array or object), encode its
				// numeric index to resolve deserialization ambiguity issues.
				// Note that rack (as of 1.0.0) can't currently deserialize
				// nested arrays properly, and attempting to do so may cause
				// a server error. Possible fixes are to modify rack's
				// deserialization algorithm or to provide an option or flag
				// to force array serialization to be shallow.
				buildParams( prefix + ""["" + ( typeof v === ""object"" || jQuery.isArray(v) ? i : """" ) + ""]"", v, traditional, add );
			}
		});
			
	} else if ( !traditional && obj != null && typeof obj === ""object"" ) {
		if ( jQuery.isEmptyObject( obj ) ) {
			add( prefix, """" );

		// Serialize object item.
		} else {
			jQuery.each( obj, function( k, v ) {
				buildParams( prefix + ""["" + k + ""]"", v, traditional, add );
			});
		}
					
	} else {
		// Serialize scalar item.
		add( prefix, obj );
	}
}

// This is still on the jQuery object... for now
// Want to move this to jQuery.ajax some day
jQuery.extend({

	// Counter for holding the number of active queries
	active: 0,

	// Last-Modified header cache for next request
	lastModified: {},
	etag: {},

	handleError: function( s, xhr, status, e ) {
		// If a local callback was specified, fire it
		if ( s.error ) {
			s.error.call( s.context, xhr, status, e );
		}

		// Fire the global callback
		if ( s.global ) {
			jQuery.triggerGlobal( s, ""ajaxError"", [xhr, s, e] );
		}
	},

	handleSuccess: function( s, xhr, status, data ) {
		// If a local callback was specified, fire it and pass it the data
		if ( s.success ) {
			s.success.call( s.context, data, status, xhr );
		}

		// Fire the global callback
		if ( s.global ) {
			jQuery.triggerGlobal( s, ""ajaxSuccess"", [xhr, s] );
		}
	},

	handleComplete: function( s, xhr, status ) {
		// Process result
		if ( s.complete ) {
			s.complete.call( s.context, xhr, status );
		}

		// The request was completed
		if ( s.global ) {
			jQuery.triggerGlobal( s, ""ajaxComplete"", [xhr, s] );
		}

		// Handle the global AJAX counter
		if ( s.global && jQuery.active-- === 1 ) {
			jQuery.event.trigger( ""ajaxStop"" );
		}
	},
		
	triggerGlobal: function( s, type, args ) {
		(s.context && s.context.url == null ? jQuery(s.context) : jQuery.event).trigger(type, args);
	},

	// Determines if an XMLHttpRequest was successful or not
	httpSuccess: function( xhr ) {
		try {
			// IE error sometimes returns 1223 when it should be 204 so treat it as success, see #1450
			return !xhr.status && location.protocol === ""file:"" ||
				xhr.status >= 200 && xhr.status < 300 ||
				xhr.status === 304 || xhr.status === 1223;
		} catch(e) {}

		return false;
	},

	// Determines if an XMLHttpRequest returns NotModified
	httpNotModified: function( xhr, url ) {
		var lastModified = xhr.getResponseHeader(""Last-Modified""),
			etag = xhr.getResponseHeader(""Etag"");

		if ( lastModified ) {
			jQuery.lastModified[url] = lastModified;
		}

		if ( etag ) {
			jQuery.etag[url] = etag;
		}

		return xhr.status === 304;
	},

	httpData: function( xhr, type, s ) {
		var ct = xhr.getResponseHeader(""content-type"") || """",
			xml = type === ""xml"" || !type && ct.indexOf(""xml"") >= 0,
			data = xml ? xhr.responseXML : xhr.responseText;

		if ( xml && data.documentElement.nodeName === ""parsererror"" ) {
			jQuery.error( ""parsererror"" );
		}

		// Allow a pre-filtering function to sanitize the response
		// s is checked to keep backwards compatibility
		if ( s && s.dataFilter ) {
			data = s.dataFilter( data, type );
		}

		// The filter can actually parse the response
		if ( typeof data === ""string"" ) {
			// Get the JavaScript object, if JSON is used.
			if ( type === ""json"" || !type && ct.indexOf(""json"") >= 0 ) {
				data = jQuery.parseJSON( data );

			// If the type is ""script"", eval it in global context
			} else if ( type === ""script"" || !type && ct.indexOf(""javascript"") >= 0 ) {
				jQuery.globalEval( data );
			}
		}

		return data;
	}

});

/*
 * Create the request object; Microsoft failed to properly
 * implement the XMLHttpRequest in IE7 (can't request local files),
 * so we use the ActiveXObject when it is available
 * Additionally XMLHttpRequest can be disabled in IE7/IE8 so
 * we need a fallback.
 */
if ( window.ActiveXObject ) {
	jQuery.ajaxSettings.xhr = function() {
		if ( window.location.protocol !== ""file:"" ) {
			try {
				return new window.XMLHttpRequest();
			} catch(xhrError) {}
		}

		try {
			return new window.ActiveXObject(""Microsoft.XMLHTTP"");
		} catch(activeError) {}
	};
}

// Does this browser support XHR requests?
jQuery.support.ajax = !!jQuery.ajaxSettings.xhr();




var elemdisplay = {},
	rfxtypes = /^(?:toggle|show|hide)$/,
	rfxnum = /^([+\-]=)?([\d+.\-]+)(.*)$/,
	timerId,
	fxAttrs = [
		// height animations
		[ ""height"", ""marginTop"", ""marginBottom"", ""paddingTop"", ""paddingBottom"" ],
		// width animations
		[ ""width"", ""marginLeft"", ""marginRight"", ""paddingLeft"", ""paddingRight"" ],
		// opacity animations
		[ ""opacity"" ]
	];

jQuery.fn.extend({
	show: function( speed, easing, callback ) {
		var elem, display;

		if ( speed || speed === 0 ) {
			return this.animate( genFx(""show"", 3), speed, easing, callback);

		} else {
			for ( var i = 0, j = this.length; i < j; i++ ) {
				elem = this[i];
				display = elem.style.display;

				// Reset the inline display of this element to learn if it is
				// being hidden by cascaded rules or not
				if ( !jQuery.data(elem, ""olddisplay"") && display === ""none"" ) {
					display = elem.style.display = """";
				}

				// Set elements which have been overridden with display: none
				// in a stylesheet to whatever the default browser style is
				// for such an element
				if ( display === """" && jQuery.css( elem, ""display"" ) === ""none"" ) {
					jQuery.data(elem, ""olddisplay"", defaultDisplay(elem.nodeName));
				}
			}

			// Set the display of most of the elements in a second loop
			// to avoid the constant reflow
			for ( i = 0; i < j; i++ ) {
				elem = this[i];
				display = elem.style.display;

				if ( display === """" || display === ""none"" ) {
					elem.style.display = jQuery.data(elem, ""olddisplay"") || """";
				}
			}

			return this;
		}
	},

	hide: function( speed, easing, callback ) {
		if ( speed || speed === 0 ) {
			return this.animate( genFx(""hide"", 3), speed, easing, callback);

		} else {
			for ( var i = 0, j = this.length; i < j; i++ ) {
				var display = jQuery.css( this[i], ""display"" );

				if ( display !== ""none"" ) {
					jQuery.data( this[i], ""olddisplay"", display );
				}
			}

			// Set the display of the elements in a second loop
			// to avoid the constant reflow
			for ( i = 0; i < j; i++ ) {
				this[i].style.display = ""none"";
			}

			return this;
		}
	},

	// Save the old toggle function
	_toggle: jQuery.fn.toggle,

	toggle: function( fn, fn2, callback ) {
		var bool = typeof fn === ""boolean"";

		if ( jQuery.isFunction(fn) && jQuery.isFunction(fn2) ) {
			this._toggle.apply( this, arguments );

		} else if ( fn == null || bool ) {
			this.each(function() {
				var state = bool ? fn : jQuery(this).is("":hidden"");
				jQuery(this)[ state ? ""show"" : ""hide"" ]();
			});

		} else {
			this.animate(genFx(""toggle"", 3), fn, fn2, callback);
		}

		return this;
	},

	fadeTo: function( speed, to, easing, callback ) {
		return this.filter("":hidden"").css(""opacity"", 0).show().end()
					.animate({opacity: to}, speed, easing, callback);
	},

	animate: function( prop, speed, easing, callback ) {
		var optall = jQuery.speed(speed, easing, callback);

		if ( jQuery.isEmptyObject( prop ) ) {
			return this.each( optall.complete );
		}

		return this[ optall.queue === false ? ""each"" : ""queue"" ](function() {
			// XXX 'this' does not always have a nodeName when running the
			// test suite

			var opt = jQuery.extend({}, optall), p,
				isElement = this.nodeType === 1,
				hidden = isElement && jQuery(this).is("":hidden""),
				self = this;

			for ( p in prop ) {
				var name = jQuery.camelCase( p );

				if ( p !== name ) {
					prop[ name ] = prop[ p ];
					delete prop[ p ];
					p = name;
				}

				if ( prop[p] === ""hide"" && hidden || prop[p] === ""show"" && !hidden ) {
					return opt.complete.call(this);
				}

				if ( isElement && ( p === ""height"" || p === ""width"" ) ) {
					// Make sure that nothing sneaks out
					// Record all 3 overflow attributes because IE does not
					// change the overflow attribute when overflowX and
					// overflowY are set to the same value
					opt.overflow = [ this.style.overflow, this.style.overflowX, this.style.overflowY ];

					// Set display property to inline-block for height/width
					// animations on inline elements that are having width/height
					// animated
					if ( jQuery.css( this, ""display"" ) === ""inline"" &&
							jQuery.css( this, ""float"" ) === ""none"" ) {
						if ( !jQuery.support.inlineBlockNeedsLayout ) {
							this.style.display = ""inline-block"";

						} else {
							var display = defaultDisplay(this.nodeName);

							// inline-level elements accept inline-block;
							// block-level elements need to be inline with layout
							if ( display === ""inline"" ) {
								this.style.display = ""inline-block"";

							} else {
								this.style.display = ""inline"";
								this.style.zoom = 1;
							}
						}
					}
				}

				if ( jQuery.isArray( prop[p] ) ) {
					// Create (if needed) and add to specialEasing
					(opt.specialEasing = opt.specialEasing || {})[p] = prop[p][1];
					prop[p] = prop[p][0];
				}
			}

			if ( opt.overflow != null ) {
				this.style.overflow = ""hidden"";
			}

			opt.curAnim = jQuery.extend({}, prop);

			jQuery.each( prop, function( name, val ) {
				var e = new jQuery.fx( self, opt, name );

				if ( rfxtypes.test(val) ) {
					e[ val === ""toggle"" ? hidden ? ""show"" : ""hide"" : val ]( prop );

				} else {
					var parts = rfxnum.exec(val),
						start = e.cur() || 0;

					if ( parts ) {
						var end = parseFloat( parts[2] ),
							unit = parts[3] || ""px"";

						// We need to compute starting value
						if ( unit !== ""px"" ) {
							jQuery.style( self, name, (end || 1) + unit);
							start = ((end || 1) / e.cur()) * start;
							jQuery.style( self, name, start + unit);
						}

						// If a +=/-= token was provided, we're doing a relative animation
						if ( parts[1] ) {
							end = ((parts[1] === ""-="" ? -1 : 1) * end) + start;
						}

						e.custom( start, end, unit );

					} else {
						e.custom( start, val, """" );
					}
				}
			});

			// For JS strict compliance
			return true;
		});
	},

	stop: function( clearQueue, gotoEnd ) {
		var timers = jQuery.timers;

		if ( clearQueue ) {
			this.queue([]);
		}

		this.each(function() {
			// go in reverse order so anything added to the queue during the loop is ignored
			for ( var i = timers.length - 1; i >= 0; i-- ) {
				if ( timers[i].elem === this ) {
					if (gotoEnd) {
						// force the next step to be the last
						timers[i](true);
					}

					timers.splice(i, 1);
				}
			}
		});

		// start the next in the queue if the last step wasn't forced
		if ( !gotoEnd ) {
			this.dequeue();
		}

		return this;
	}

});

function genFx( type, num ) {
	var obj = {};

	jQuery.each( fxAttrs.concat.apply([], fxAttrs.slice(0,num)), function() {
		obj[ this ] = type;
	});

	return obj;
}

// Generate shortcuts for custom animations
jQuery.each({
	slideDown: genFx(""show"", 1),
	slideUp: genFx(""hide"", 1),
	slideToggle: genFx(""toggle"", 1),
	fadeIn: { opacity: ""show"" },
	fadeOut: { opacity: ""hide"" },
	fadeToggle: { opacity: ""toggle"" }
}, function( name, props ) {
	jQuery.fn[ name ] = function( speed, easing, callback ) {
		return this.animate( props, speed, easing, callback );
	};
});

jQuery.extend({
	speed: function( speed, easing, fn ) {
		var opt = speed && typeof speed === ""object"" ? jQuery.extend({}, speed) : {
			complete: fn || !fn && easing ||
				jQuery.isFunction( speed ) && speed,
			duration: speed,
			easing: fn && easing || easing && !jQuery.isFunction(easing) && easing
		};

		opt.duration = jQuery.fx.off ? 0 : typeof opt.duration === ""number"" ? opt.duration :
			opt.duration in jQuery.fx.speeds ? jQuery.fx.speeds[opt.duration] : jQuery.fx.speeds._default;

		// Queueing
		opt.old = opt.complete;
		opt.complete = function() {
			if ( opt.queue !== false ) {
				jQuery(this).dequeue();
			}
			if ( jQuery.isFunction( opt.old ) ) {
				opt.old.call( this );
			}
		};

		return opt;
	},

	easing: {
		linear: function( p, n, firstNum, diff ) {
			return firstNum + diff * p;
		},
		swing: function( p, n, firstNum, diff ) {
			return ((-Math.cos(p*Math.PI)/2) + 0.5) * diff + firstNum;
		}
	},

	timers: [],

	fx: function( elem, options, prop ) {
		this.options = options;
		this.elem = elem;
		this.prop = prop;

		if ( !options.orig ) {
			options.orig = {};
		}
	}

});

jQuery.fx.prototype = {
	// Simple function for setting a style value
	update: function() {
		if ( this.options.step ) {
			this.options.step.call( this.elem, this.now, this );
		}

		(jQuery.fx.step[this.prop] || jQuery.fx.step._default)( this );
	},

	// Get the current size
	cur: function() {
		if ( this.elem[this.prop] != null && (!this.elem.style || this.elem.style[this.prop] == null) ) {
			return this.elem[ this.prop ];
		}

		var r = parseFloat( jQuery.css( this.elem, this.prop ) );
		return r && r > -10000 ? r : 0;
	},

	// Start an animation from one number to another
	custom: function( from, to, unit ) {
		var self = this,
			fx = jQuery.fx;

		this.startTime = jQuery.now();
		this.start = from;
		this.end = to;
		this.unit = unit || this.unit || ""px"";
		this.now = this.start;
		this.pos = this.state = 0;

		function t( gotoEnd ) {
			return self.step(gotoEnd);
		}

		t.elem = this.elem;

		if ( t() && jQuery.timers.push(t) && !timerId ) {
			timerId = setInterval(fx.tick, fx.interval);
		}
	},

	// Simple 'show' function
	show: function() {
		// Remember where we started, so that we can go back to it later
		this.options.orig[this.prop] = jQuery.style( this.elem, this.prop );
		this.options.show = true;

		// Begin the animation
		// Make sure that we start at a small width/height to avoid any
		// flash of content
		this.custom(this.prop === ""width"" || this.prop === ""height"" ? 1 : 0, this.cur());

		// Start by showing the element
		jQuery( this.elem ).show();
	},

	// Simple 'hide' function
	hide: function() {
		// Remember where we started, so that we can go back to it later
		this.options.orig[this.prop] = jQuery.style( this.elem, this.prop );
		this.options.hide = true;

		// Begin the animation
		this.custom(this.cur(), 0);
	},

	// Each step of an animation
	step: function( gotoEnd ) {
		var t = jQuery.now(), done = true;

		if ( gotoEnd || t >= this.options.duration + this.startTime ) {
			this.now = this.end;
			this.pos = this.state = 1;
			this.update();

			this.options.curAnim[ this.prop ] = true;

			for ( var i in this.options.curAnim ) {
				if ( this.options.curAnim[i] !== true ) {
					done = false;
				}
			}

			if ( done ) {
				// Reset the overflow
				if ( this.options.overflow != null && !jQuery.support.shrinkWrapBlocks ) {
					var elem = this.elem,
						options = this.options;

					jQuery.each( [ """", ""X"", ""Y"" ], function (index, value) {
						elem.style[ ""overflow"" + value ] = options.overflow[index];
					} );
				}

				// Hide the element if the ""hide"" operation was done
				if ( this.options.hide ) {
					jQuery(this.elem).hide();
				}

				// Reset the properties, if the item has been hidden or shown
				if ( this.options.hide || this.options.show ) {
					for ( var p in this.options.curAnim ) {
						jQuery.style( this.elem, p, this.options.orig[p] );
					}
				}

				// Execute the complete function
				this.options.complete.call( this.elem );
			}

			return false;

		} else {
			var n = t - this.startTime;
			this.state = n / this.options.duration;

			// Perform the easing function, defaults to swing
			var specialEasing = this.options.specialEasing && this.options.specialEasing[this.prop];
			var defaultEasing = this.options.easing || (jQuery.easing.swing ? ""swing"" : ""linear"");
			this.pos = jQuery.easing[specialEasing || defaultEasing](this.state, n, 0, 1, this.options.duration);
			this.now = this.start + ((this.end - this.start) * this.pos);

			// Perform the next step of the animation
			this.update();
		}

		return true;
	}
};

jQuery.extend( jQuery.fx, {
	tick: function() {
		var timers = jQuery.timers;

		for ( var i = 0; i < timers.length; i++ ) {
			if ( !timers[i]() ) {
				timers.splice(i--, 1);
			}
		}

		if ( !timers.length ) {
			jQuery.fx.stop();
		}
	},

	interval: 13,

	stop: function() {
		clearInterval( timerId );
		timerId = null;
	},

	speeds: {
		slow: 600,
		fast: 200,
		// Default speed
		_default: 400
	},

	step: {
		opacity: function( fx ) {
			jQuery.style( fx.elem, ""opacity"", fx.now );
		},

		_default: function( fx ) {
			if ( fx.elem.style && fx.elem.style[ fx.prop ] != null ) {
				fx.elem.style[ fx.prop ] = (fx.prop === ""width"" || fx.prop === ""height"" ? Math.max(0, fx.now) : fx.now) + fx.unit;
			} else {
				fx.elem[ fx.prop ] = fx.now;
			}
		}
	}
});

if ( jQuery.expr && jQuery.expr.filters ) {
	jQuery.expr.filters.animated = function( elem ) {
		return jQuery.grep(jQuery.timers, function( fn ) {
			return elem === fn.elem;
		}).length;
	};
}

function defaultDisplay( nodeName ) {
	if ( !elemdisplay[ nodeName ] ) {
		var elem = jQuery(""<"" + nodeName + "">"").appendTo(""body""),
			display = elem.css(""display"");

		elem.remove();

		if ( display === ""none"" || display === """" ) {
			display = ""block"";
		}

		elemdisplay[ nodeName ] = display;
	}

	return elemdisplay[ nodeName ];
}




var rtable = /^t(?:able|d|h)$/i,
	rroot = /^(?:body|html)$/i;

if ( ""getBoundingClientRect"" in document.documentElement ) {
	jQuery.fn.offset = function( options ) {
		var elem = this[0], box;

		if ( options ) { 
			return this.each(function( i ) {
				jQuery.offset.setOffset( this, options, i );
			});
		}

		if ( !elem || !elem.ownerDocument ) {
			return null;
		}

		if ( elem === elem.ownerDocument.body ) {
			return jQuery.offset.bodyOffset( elem );
		}

		try {
			box = elem.getBoundingClientRect();
		} catch(e) {}

		var doc = elem.ownerDocument,
			docElem = doc.documentElement;

		// Make sure we're not dealing with a disconnected DOM node
		if ( !box || !jQuery.contains( docElem, elem ) ) {
			return box || { top: 0, left: 0 };
		}

		var body = doc.body,
			win = getWindow(doc),
			clientTop  = docElem.clientTop  || body.clientTop  || 0,
			clientLeft = docElem.clientLeft || body.clientLeft || 0,
			scrollTop  = (win.pageYOffset || jQuery.support.boxModel && docElem.scrollTop  || body.scrollTop ),
			scrollLeft = (win.pageXOffset || jQuery.support.boxModel && docElem.scrollLeft || body.scrollLeft),
			top  = box.top  + scrollTop  - clientTop,
			left = box.left + scrollLeft - clientLeft;

		return { top: top, left: left };
	};

} else {
	jQuery.fn.offset = function( options ) {
		var elem = this[0];

		if ( options ) { 
			return this.each(function( i ) {
				jQuery.offset.setOffset( this, options, i );
			});
		}

		if ( !elem || !elem.ownerDocument ) {
			return null;
		}

		if ( elem === elem.ownerDocument.body ) {
			return jQuery.offset.bodyOffset( elem );
		}

		jQuery.offset.initialize();

		var computedStyle,
			offsetParent = elem.offsetParent,
			prevOffsetParent = elem,
			doc = elem.ownerDocument,
			docElem = doc.documentElement,
			body = doc.body,
			defaultView = doc.defaultView,
			prevComputedStyle = defaultView ? defaultView.getComputedStyle( elem, null ) : elem.currentStyle,
			top = elem.offsetTop,
			left = elem.offsetLeft;

		while ( (elem = elem.parentNode) && elem !== body && elem !== docElem ) {
			if ( jQuery.offset.supportsFixedPosition && prevComputedStyle.position === ""fixed"" ) {
				break;
			}

			computedStyle = defaultView ? defaultView.getComputedStyle(elem, null) : elem.currentStyle;
			top  -= elem.scrollTop;
			left -= elem.scrollLeft;

			if ( elem === offsetParent ) {
				top  += elem.offsetTop;
				left += elem.offsetLeft;

				if ( jQuery.offset.doesNotAddBorder && !(jQuery.offset.doesAddBorderForTableAndCells && rtable.test(elem.nodeName)) ) {
					top  += parseFloat( computedStyle.borderTopWidth  ) || 0;
					left += parseFloat( computedStyle.borderLeftWidth ) || 0;
				}

				prevOffsetParent = offsetParent;
				offsetParent = elem.offsetParent;
			}

			if ( jQuery.offset.subtractsBorderForOverflowNotVisible && computedStyle.overflow !== ""visible"" ) {
				top  += parseFloat( computedStyle.borderTopWidth  ) || 0;
				left += parseFloat( computedStyle.borderLeftWidth ) || 0;
			}

			prevComputedStyle = computedStyle;
		}

		if ( prevComputedStyle.position === ""relative"" || prevComputedStyle.position === ""static"" ) {
			top  += body.offsetTop;
			left += body.offsetLeft;
		}

		if ( jQuery.offset.supportsFixedPosition && prevComputedStyle.position === ""fixed"" ) {
			top  += Math.max( docElem.scrollTop, body.scrollTop );
			left += Math.max( docElem.scrollLeft, body.scrollLeft );
		}

		return { top: top, left: left };
	};
}

jQuery.offset = {
	initialize: function() {
		var body = document.body, container = document.createElement(""div""), innerDiv, checkDiv, table, td, bodyMarginTop = parseFloat( jQuery.css(body, ""marginTop"") ) || 0,
			html = ""<div style='position:absolute;top:0;left:0;margin:0;border:5px solid #000;padding:0;width:1px;height:1px;'><div></div></div><table style='position:absolute;top:0;left:0;margin:0;border:5px solid #000;padding:0;width:1px;height:1px;' cellpadding='0' cellspacing='0'><tr><td></td></tr></table>"";

		jQuery.extend( container.style, { position: ""absolute"", top: 0, left: 0, margin: 0, border: 0, width: ""1px"", height: ""1px"", visibility: ""hidden"" } );

		container.innerHTML = html;
		body.insertBefore( container, body.firstChild );
		innerDiv = container.firstChild;
		checkDiv = innerDiv.firstChild;
		td = innerDiv.nextSibling.firstChild.firstChild;

		this.doesNotAddBorder = (checkDiv.offsetTop !== 5);
		this.doesAddBorderForTableAndCells = (td.offsetTop === 5);

		checkDiv.style.position = ""fixed"";
		checkDiv.style.top = ""20px"";

		// safari subtracts parent border width here which is 5px
		this.supportsFixedPosition = (checkDiv.offsetTop === 20 || checkDiv.offsetTop === 15);
		checkDiv.style.position = checkDiv.style.top = """";

		innerDiv.style.overflow = ""hidden"";
		innerDiv.style.position = ""relative"";

		this.subtractsBorderForOverflowNotVisible = (checkDiv.offsetTop === -5);

		this.doesNotIncludeMarginInBodyOffset = (body.offsetTop !== bodyMarginTop);

		body.removeChild( container );
		body = container = innerDiv = checkDiv = table = td = null;
		jQuery.offset.initialize = jQuery.noop;
	},

	bodyOffset: function( body ) {
		var top = body.offsetTop,
			left = body.offsetLeft;

		jQuery.offset.initialize();

		if ( jQuery.offset.doesNotIncludeMarginInBodyOffset ) {
			top  += parseFloat( jQuery.css(body, ""marginTop"") ) || 0;
			left += parseFloat( jQuery.css(body, ""marginLeft"") ) || 0;
		}

		return { top: top, left: left };
	},
	
	setOffset: function( elem, options, i ) {
		var position = jQuery.css( elem, ""position"" );

		// set position first, in-case top/left are set even on static elem
		if ( position === ""static"" ) {
			elem.style.position = ""relative"";
		}

		var curElem = jQuery( elem ),
			curOffset = curElem.offset(),
			curCSSTop = jQuery.css( elem, ""top"" ),
			curCSSLeft = jQuery.css( elem, ""left"" ),
			calculatePosition = (position === ""absolute"" && jQuery.inArray('auto', [curCSSTop, curCSSLeft]) > -1),
			props = {}, curPosition = {}, curTop, curLeft;

		// need to be able to calculate position if either top or left is auto and position is absolute
		if ( calculatePosition ) {
			curPosition = curElem.position();
		}

		curTop  = calculatePosition ? curPosition.top  : parseInt( curCSSTop,  10 ) || 0;
		curLeft = calculatePosition ? curPosition.left : parseInt( curCSSLeft, 10 ) || 0;

		if ( jQuery.isFunction( options ) ) {
			options = options.call( elem, i, curOffset );
		}

		if (options.top != null) {
			props.top = (options.top - curOffset.top) + curTop;
		}
		if (options.left != null) {
			props.left = (options.left - curOffset.left) + curLeft;
		}
		
		if ( ""using"" in options ) {
			options.using.call( elem, props );
		} else {
			curElem.css( props );
		}
	}
};


jQuery.fn.extend({
	position: function() {
		if ( !this[0] ) {
			return null;
		}

		var elem = this[0],

		// Get *real* offsetParent
		offsetParent = this.offsetParent(),

		// Get correct offsets
		offset       = this.offset(),
		parentOffset = rroot.test(offsetParent[0].nodeName) ? { top: 0, left: 0 } : offsetParent.offset();

		// Subtract element margins
		// note: when an element has margin: auto the offsetLeft and marginLeft
		// are the same in Safari causing offset.left to incorrectly be 0
		offset.top  -= parseFloat( jQuery.css(elem, ""marginTop"") ) || 0;
		offset.left -= parseFloat( jQuery.css(elem, ""marginLeft"") ) || 0;

		// Add offsetParent borders
		parentOffset.top  += parseFloat( jQuery.css(offsetParent[0], ""borderTopWidth"") ) || 0;
		parentOffset.left += parseFloat( jQuery.css(offsetParent[0], ""borderLeftWidth"") ) || 0;

		// Subtract the two offsets
		return {
			top:  offset.top  - parentOffset.top,
			left: offset.left - parentOffset.left
		};
	},

	offsetParent: function() {
		return this.map(function() {
			var offsetParent = this.offsetParent || document.body;
			while ( offsetParent && (!rroot.test(offsetParent.nodeName) && jQuery.css(offsetParent, ""position"") === ""static"") ) {
				offsetParent = offsetParent.offsetParent;
			}
			return offsetParent;
		});
	}
});


// Create scrollLeft and scrollTop methods
jQuery.each( [""Left"", ""Top""], function( i, name ) {
	var method = ""scroll"" + name;

	jQuery.fn[ method ] = function(val) {
		var elem = this[0], win;
		
		if ( !elem ) {
			return null;
		}

		if ( val !== undefined ) {
			// Set the scroll offset
			return this.each(function() {
				win = getWindow( this );

				if ( win ) {
					win.scrollTo(
						!i ? val : jQuery(win).scrollLeft(),
						 i ? val : jQuery(win).scrollTop()
					);

				} else {
					this[ method ] = val;
				}
			});
		} else {
			win = getWindow( elem );

			// Return the scroll offset
			return win ? (""pageXOffset"" in win) ? win[ i ? ""pageYOffset"" : ""pageXOffset"" ] :
				jQuery.support.boxModel && win.document.documentElement[ method ] ||
					win.document.body[ method ] :
				elem[ method ];
		}
	};
});

function getWindow( elem ) {
	return jQuery.isWindow( elem ) ?
		elem :
		elem.nodeType === 9 ?
			elem.defaultView || elem.parentWindow :
			false;
}




// Create innerHeight, innerWidth, outerHeight and outerWidth methods
jQuery.each([ ""Height"", ""Width"" ], function( i, name ) {

	var type = name.toLowerCase();

	// innerHeight and innerWidth
	jQuery.fn[""inner"" + name] = function() {
		return this[0] ?
			parseFloat( jQuery.css( this[0], type, ""padding"" ) ) :
			null;
	};

	// outerHeight and outerWidth
	jQuery.fn[""outer"" + name] = function( margin ) {
		return this[0] ?
			parseFloat( jQuery.css( this[0], type, margin ? ""margin"" : ""border"" ) ) :
			null;
	};

	jQuery.fn[ type ] = function( size ) {
		// Get window width or height
		var elem = this[0];
		if ( !elem ) {
			return size == null ? null : this;
		}
		
		if ( jQuery.isFunction( size ) ) {
			return this.each(function( i ) {
				var self = jQuery( this );
				self[ type ]( size.call( this, i, self[ type ]() ) );
			});
		}

		if ( jQuery.isWindow( elem ) ) {
			// Everyone else use document.documentElement or document.body depending on Quirks vs Standards mode
			return elem.document.compatMode === ""CSS1Compat"" && elem.document.documentElement[ ""client"" + name ] ||
				elem.document.body[ ""client"" + name ];

		// Get document width or height
		} else if ( elem.nodeType === 9 ) {
			// Either scroll[Width/Height] or offset[Width/Height], whichever is greater
			return Math.max(
				elem.documentElement[""client"" + name],
				elem.body[""scroll"" + name], elem.documentElement[""scroll"" + name],
				elem.body[""offset"" + name], elem.documentElement[""offset"" + name]
			);

		// Get or set width or height on the element
		} else if ( size === undefined ) {
			var orig = jQuery.css( elem, type ),
				ret = parseFloat( orig );

			return jQuery.isNaN( ret ) ? orig : ret;

		// Set the width or height on the element (default to pixels if value is unitless)
		} else {
			return this.|css(| type|, typeof size === ""string"" ? size : size + ""px"" );
		}
	};

});


})(window);";

            #endregion

            PerformRequests(code, (context, offset, data, index) =>
            {
                if (offset != 190044) return;
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNotNull(completions);
            },
            true);
        }

        [TestMethod]
        public void Bug245383()
        {
            var primary = _session.FileFromText(TestFiles.Bug245383);
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            Assert.IsNotNull(completions);
        }

        [TestMethod]
        public void NoPrivatesVisible()
        {
            PerformCompletionRequests(@"this.|", (completions, data, i) =>
            {
                completions.Count(c => c.Name.StartsWith("__$")).Expect(0);
            });
        }

        [TestMethod]
        public void Bug135826_2()
        {
            string code = @"
			return function() {
				self;
			};
            Object.|
            ";

            PerformCompletionRequests(code, (completions, data, i) =>
            {
            });
        }

        [TestMethod]
        public void Bug123984_Completion()
        {
            ValidateHasCompletions(@"
                var x = { a: 1 };
                for(var item in somethingUndefined)
                    x.|;

                for(var item in [])
                    x.|;

                for(var item in {})
                    x.|;
                }
            ", "a");

            ValidateHasCompletions(@"
                for(var item in { a: 1 }) {
                    item.|
            ", "charAt");

        }

        [TestMethod]
        public void Extensions()
        {
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function (e) {
                    e.items = [ 
                        { name: 'item1', kind: 'property' },
                        { name: 'item2', kind: 'field' }
                    ];
                });
                var x = { f: function() {} };
                x.|                    
                ", (completions, data, i) =>
                 {
                     var completionsArray = completions.AsEnumerable().ToArray();
                     Assert.IsTrue(completionsArray.Single(c => c.Name == "item1").Kind == AuthorCompletionKind.ackProperty);
                     Assert.IsTrue(completionsArray.Single(c => c.Name == "item2").Kind == AuthorCompletionKind.ackField);
                 });

            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function (e) {
                    e.items = [ 
                        {  },
                    ];
                });
                var x = { f: function() {} };
                x.|                    
                ", (completions, data, i) =>
                 {
                     completions.AsEnumerable().Count().Expect(0);
                 });

            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function (e) {
                    e.items = [ 
                        { name: undefined },
                    ];
                });
                var x = { f: function() {} };
                x.|                    
                ", (completions, data, i) =>
                 {
                     completions.AsEnumerable().Count().Expect(0);
                 });

            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function (e) {
                    function printItems() {
                        intellisense.logMessage('Items: \n');
                        e.items.forEach(function (item) { intellisense.logMessage('Item: ' + item.name + '\n'); });
                    }
                    intellisense.logMessage('In addCompletionHandler\n');
                    printItems();    
                    e.items = e.items.filter(function(element) { return element.name[0] != '_'; });
                    printItems();    
                    e.items.push({ name: 'f1', kind: 'method' }); 
                    e.items.push({ name: 'field1', kind: 'field' }); 
                    e.items.push({ name: 'set_field2', kind: 'method' }); 
                    e.items.forEach(function(item) { if(item.name.indexOf('set_') == 0) { item.kind='property'; } });
                    printItems();    
                });
                var x = { _num1: 2, _num2: 2, num3: 'a', set_someVal: function() {} };
                x.|                    
                ", (completions, data, i) =>
                 {
                     var completionsArray = completions.AsEnumerable().ToArray();
                     Assert.IsTrue(completionsArray.Count(c => c.Name == "num3") == 1);
                     Assert.IsTrue(completionsArray.Count(c => c.Name == "_num1") == 0);
                     Assert.IsTrue(completionsArray.Count(c => c.Name == "_num2") == 0);
                     Assert.IsTrue(completionsArray.Single(c => c.Name == "f1").Kind == AuthorCompletionKind.ackMethod);
                     Assert.IsTrue(completionsArray.Single(c => c.Name == "field1").Kind == AuthorCompletionKind.ackField);
                     Assert.IsTrue(completionsArray.Single(c => c.Name == "set_field2").Kind == AuthorCompletionKind.ackProperty);
                     Assert.IsTrue(completionsArray.Single(c => c.Name == "set_someVal").Kind == AuthorCompletionKind.ackProperty);
                 });

            PerformCompletionRequests(@"
                var v8 = { hide: true };
                ;|v2,v3,v4,v6,!v1,!v5,!v7,!v8|
                this.|!v8|;
                function f(v7) {|v2,v3,v4,v6,!v1,!v5,v7|}
                f({ hide: true });                
                ",
                @"
                    var v1 = { hide: true };
                    var v2 = 3;
                    var v3 = {};
                    var v4 = ''; 
                    var v5 = { name: '', hide: true };
                    var v6;
                ",
                 @"
                intellisense.addEventListener('statementcompletion',  function (e) {
                        intellisense.logMessage('>>> hiding extension');
                        e.items = e.items.filter(function(item) { 
                            if(item.value && item.value.hide) { 
                                intellisense.logMessage('>>> hiding item: ' + item.name + ' scope: ' + item.scope + ' value: ' + JSON.stringify(item.value);
                                return false;
                            }
                            intellisense.logMessage('>>> leaving item: ' + item.name);
                            return true; 
                        });
                        intellisense.logMessage('>>> items left:');
                        e.items.forEach(function(item) { intellisense.logMessage('>>> item: ' + item.name); });
                    });
                ");

            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function (e) {
                    e.items = e.items.filter(function(item) { 
                        if(item.parentObject) {
                            if(item.name.indexOf('get_') == 0) {
                                var setter = 'set_' + item.name.substring(4, item.name.length);
                                if(item.parentObject[setter]) {
                                   item.name =  item.name.substring(4, item.name.length); 
                                   return true;
                                }
                            }
                        }
                        return false; 
                    });
                });
                var x = {
                    set_X: function() {},
                    get_X: function() {},
                    get_Y: function() {}
                };
                x.|                    
                ", (completions, data, i) =>
                 {
                     var completionsArray = completions.AsEnumerable().ToArray();
                     completions.ExpectContains(new[] { "X" });
                     completions.ExpectNotContains(new[] { "Y" });
                 });
        }

        [TestMethod]
        public void CompletionOnKeywords()
        {
            ValidateNoCompletion("do {} while.| (false);");
            ValidateNoCompletion("do {} while.|(false);");
            ValidateNoCompletion("do {} while(false).|");
            ValidateNoCompletion("do.|");
            ValidateNoCompletion("do {}.|");
            ValidateNoCompletion("do {} while.|");
            ValidateNoCompletion("function f() { switch(a).| }");
            ValidateNoCompletion("function f() { switch(a) { case.| } }");
            ValidateNoCompletion("function f() { switch.| }");
            ValidateNoCompletion("function f() { switch(a) { case 1:.| } }");
            ValidateNoCompletion("function f() { return.| }");
            ValidateNoCompletion("if(1).|");
            ValidateNoCompletion("if.| () {}");
            ValidateNoCompletion("if.|() {}");
            ValidateNoCompletion("if.|");
            ValidateNoCompletion("var x; var y; if.|");
            ValidateNoCompletion("var x; var y; if(1);else.|");
            ValidateNoCompletion("var x; var y; if(1);else { }.|");
            ValidateNoCompletion("var x=1; do.|");
            ValidateNoCompletion("function() { do {}.| }");
            ValidateNoCompletion("for(;;){}.|");
            ValidateNoCompletion("function f() { for.| (;;); }");
            ValidateNoCompletion("function f() { for().| }");
            ValidateNoCompletion("function f() { for.| }");
            ValidateNoCompletion("function f() { x=1; for(;;) { }.| }");
            ValidateNoCompletion(".|");
            ValidateNoCompletion("var.|");
            ValidateNoCompletion("while.|");
            ValidateNoCompletion("while(true) { break. }");
            ValidateNoCompletion("function f() { while.| }");
            ValidateNoCompletion("function f() { switch.| }");
            ValidateNoCompletion("function f() { catch.| }");
            ValidateNoCompletion("function f() { finally.| }");
            ValidateNoCompletion("function f() { while.| }");
            ValidateNoCompletion("function f() { if(true).| }");
            ValidateNoCompletion("try{}.| catch(e) {}");
            ValidateNoCompletion("try{} catch(e).| {}");
            ValidateNoCompletion("try{} catch(e) {}.|");
            ValidateNoCompletion("try{} finally.|");
            ValidateNoCompletion("try{} finally{}.|");
            ValidateNoCompletion("var a={}; with.|");
            ValidateNoCompletion("var a={}; with(a).|");
            ValidateNoCompletion("var a={}; with(a){}.|");

            // Validate completion works inside for
            ValidateHasCompletions("var v = { num: 2 }; for(var i=0; i<v.n|", "num");
            ValidateHasCompletions("try{} catch(e) { Number.| }");

            // keywords are legal as field names
            ValidateHasCompletions("var x = { var: 1, if: 2, while: 'while' }; x.|", "var", "if", "while");

            // validate completion is available during typing
            ValidateHasCompletions("var x; switch(|", "x");
            ValidateHasCompletions("var x; while (|", "x");
            ValidateHasCompletions("var x; for (|", "x");
            ValidateHasCompletions("var x; if (|", "x");
            ValidateHasCompletions("var x; with (|", "x");
            ValidateHasCompletions("var x; catch(|", "x");
            ValidateHasCompletions("var x; do {} while (|", "x");
        }

        [TestMethod]
        public void ASimpleCompletion()
        {
            PerformCompletionRequests("var a = 10; a.|", (completions, data, i) =>
            {
                completions.ExpectContains(NumberMethods);
            });
        }

        [TestMethod]
        public void CaseExpressionCompletion()
        {
            PerformCompletionRequests("var frameworkStage = 0; switch (firework.Stage) { case frameworkSt|frameworkStage|age.Exploding:");
        }

        [TestMethod]
        public void BasicCompletions()
        {
            PerformCompletionRequests(Completions, (completions, data, i) =>
            {
                switch (data)
                {
                    case "n":
                        completions.ExpectContains(NumberMethods);
                        completions.Count(item => item.Kind != AuthorCompletionKind.ackMethod).Expect(1);
                        break;
                    case "s": completions.ExpectContains(StringMethods); break;
                    case "o": completions.ExpectContains(ObjectMethods.Concat("a", "b")); break;
                    case "a": completions.ExpectContains(ArrayMethods); break;
                    case "f": completions.ExpectContains(FunctionMethods); break;
                    case "r": completions.ExpectContains(RegExpMethods); break;
                    case "b": completions.ExpectContains(BooleanMethods); break;
                    default: Assert.Fail("Unhandled data: " + data); break;
                }
            });
        }
        #region Test data
        const string Completions = @"
function etest(t, s) {
    try {
        eval(s);
        write(t + ' parsed');
    }
    catch (e) {
        write(t + ':' + e.message);
    }
}

function write(a) {
    if (this.WScript == undefined) {
        document.write(a);
        document.write('</br>');
    }
    else
        WScript.Echo(a)
}


var expected;
var n = 10;                 var n_e = ['constructor', 'hasOwnProperty', 'isPrototypeOf', 'propertyIsEnumerable', 'toExpotential', 'toFixed', 'toLocaleString', 'toPrecision', 'toString', 'valueOf'];
var s = 'a';                var s_e = ['anchor', 'big', 'blink', 'bold', 'charAt', 'charCodeAt', 'concat', 'constructor', 'fixed', 'fontcolor', 'fontsize', 'hasOwnProperty', 'indexOf', 'isPrototypeOf', 'italics', 'lastIndexOf', 'length', 'link', 'localeCompare', 'match', 'propertyIsEnumerable', 'replace', 'search', 'slice', 'small', 'split', 'strike', 'sub', 'substr', 'substring', 'sup', 'toLocaleLowerCase', 'toLocaleString', 'toLocaleUpperCase', 'toLowerCase', 'toString', 'toUpperCase', 'valueOf'];
var o = { a: 0, b: 1 };     var o_e = ['a', 'b', 'constructor', 'hasOwnProperty', 'isPrototypeOf', 'propertyIsEnumerable', 'toLocaleString', 'toString', 'valueOf'];
var a = [1, 2, 3];          var a_e = ['concat', 'constructor', 'hasOwnProperty', 'isPrototypeOf', 'join', 'length', 'pop', 'propertyIsEnumerable', 'push', 'reverse', 'shift', 'slice', 'sort', 'splice', 'toLocaleString', 'toString', 'unshift', 'valueOf'];
function f() { }            var f_e = ['apply', 'call', 'constructor', 'hasOwnProperty', 'isPrototypeOf', 'length', 'propertyIsEnumerable', 'prototype', 'toLocaleString', 'toString', 'valueOf'];
var r = /^\d$/;             var r_e = ['compile', 'constructor', 'exec', 'global', 'ignoreCase', 'lastIndex', 'multiline', 'source', 'test'];
var b = true;               var b_e = ['constructor', 'hasOwnProperty', 'isPrototypeOf', 'propertyIsEnumerable', 'toLocalString', 'toString', 'valueOf'];

expected = n_e; (n.|n| ); 
expected = s_e; (s.|s| );
expected = o_e; (o.|o| );
expected = a_e; (a.|a| );
expected = f_e; (f.|f| );
expected = r_e; (r.|r| );
expected = b_e; (b.|b| );

expected = n_e; (n.a|n| ); 
expected = s_e; (s. ab|s|ccd );
expected = o_e; (o.d9Ad|o|A );
expected = a_e; (a.dd|a| );
expected = f_e; (f.dd|f|d );
expected = r_e; (r.dd|r|d );
expected = b_e; (b.dd|b|d );
";
        #endregion

        [TestMethod]
        public void SampleFileCompletion()
        {
            PerformCompletionRequests(SampleFile, (completions, data, i) =>
            {
                switch (data)
                {
                    case "g": completions.ExpectContains(Globals); break;
                    case "e": completions.ExpectContains(ObjectMethods.Concat("ReadPastEnd", "LoadFailed")); break;
                }
            });
        }
        #region Test data
        const string SampleFile = @"
var seekOrigin = {
    begin: 1,
    current: 2,
    end: |g|3

}

var binaryReader = function() {    
    var exception = { }
    exception.ReadPastEnd = 1;
    exception.LoadFailed = 2;
    exception.|e|

    var fileSize = 0;
    var filePointer = 0;
    var fileContents;

    var readByteAt;

    var loadFile = function(url) {
        var loadFileAgnostic = function() {
            var request = new XMLHttpRequest();

            request.open('GET', url, false);


            if (request.overrideMimeType) 
                request.overrideMimeType('text/plain; charset=x-user-defined');
				
            request.send(null);        

            if (request.status != 200) 
                throwException(exception.LoadFailed);

            fileContents = request.responseText;

            fileSize = fileContents.length;

            readByteAt = function(offset) {   
   
                return fileContents.charCodeAt(offset) & 0xff;
            }
        }

        var loadFileIE = function() {
        	var vbArr = BinFileReaderImpl_IE_VBAjaxLoader(url);
		    fileContents = vbArr.toArray();

		    fileSize = fileContents.length - 1;

		    if (fileSize < 0) 
                throwException(exception.LoadFailed);

		    readByteAt = function(offset) {
			    return fileContents[offset];
		    }
        }

        if ((/msie/i.test(navigator.userAgent)) && (!/opera/i.test(navigator.userAgent))) {
            loadFileIE();
        }
        else {
            loadFileAgnostic();
        }
    }   

    var throwException = function(errorCode) {
        switch(errorCode) {
 
            case exception.ReadPastEnd:
                throw {
                    name: 'ReadPastEnd',
                    message: 'Read past the end of the file'
                }
            break;
            case exception.LoadFailed:
                throw {
                    name: 'LoadFailed',
                    message: 'Failed to load ' + url                    
                }
            break;
        }        
    }

    var movePointerTo = function(offset) {
        if (offset < 0) 
            filePointer = 0;
        else if (offset > fileSize) 
            throwException(exception.ReadPastEnd);
        else 
            filePointer = offset;


        return filePointer;
    }

    var seek = function(offset, origin) {
        switch(origin) {
            case seekOrigin.begin:
                movePointerTo(offset);
                break;
            case seekOrigin.current:
                movePointerTo(filePointer + offset);
                break;
            case seekOrigin.end:
                movePointerTo(fileSize + offset);
                break;
        }
    }

    var readNumber = function(numBytes, offset) {
        numBytes = numBytes || 1;
        offset = offset | filePointer;

        movePointerTo(offset + numBytes);

        var result = 0;
        for(var i = offset + numBytes; i > offset; i--) {
            result = result * 256 + readByteAt(i - 1);
        }

        return result;
    }

    var readStringInternal = function(numChars, offset, charSize) {
        numChars = numChars || 1;
        offset = offset || filePointer;

        movePointerTo(offset);

        var result = '';
        var endPosition = offset + numChars * charSize;

        for(var i = offset; i < endPosition; i += charSize) {
            result += String.fromCharCode(readNumber(charSize));
        }

        return result;
    }

    var readString = function(numChars, offset) {
        return readStringInternal(numChars, offset, 1);
    }

    var readStringUnicode = function(numChars, offset) {
        return readStringInternal(numChars, offset, 2);
    }

    return {
        loadFile: loadFile,
        readNumber: readNumber,
        seek: seek,
        readStringUnicode: readStringUnicode,
        readString: readString
    }
}

document.write('<script type=""text/vbscript"">\n\
Function BinFileReaderImpl_IE_VBAjaxLoader(fileName)\n\
	Dim xhr\n\
	Set xhr = CreateObject(""Microsoft.XMLHTTP"")\n\
\n\
	xhr.Open ""GET"", fileName, False\n\
\n\
	xhr.setRequestHeader ""Accept-Charset"", ""x-user-defined""\n\
	xhr.send\n\
\n\
	Dim byteArray()\n\
\n\
	if xhr.Status = 200 Then\n\
		Dim byteString\n\
		Dim i\n\
\n\
		byteString=xhr.responseBody\n\
\n\
		ReDim byteArray(LenB(byteString))\n\
\n\
		For i = 1 To LenB(byteString)\n\
			byteArray(i-1) = AscB(MidB(byteString, i, 1))\n\
		Next\n\
	End If\n\
\n\
	BinFileReaderImpl_IE_VBAjaxLoader=byteArray\n\
End Function\n\
</script>');";
        #endregion

        [TestMethod]
        public void IfFalse()
        {
            PerformCompletionRequests(IfFalseFile, (completions, data, i) =>
            {
                completions.ExpectContains(NumberMethods);
            });
        }
        #region Test data
        const string IfFalseFile = @"var n = 10;                 

// This branch is not taken but we should get completions anyway
if (false)
    n.|";
        #endregion

        [TestMethod]
        public void Implict()
        {
            PerformCompletionRequests(SampleFile, (completions, data, i) =>
            {
                switch (data)
                {
                    case "i": completions.ExpectContains(NumberMethods); break;
                    case "n": Assert.IsNull(completions); break;
                }
            });
        }
        #region Test data
        const string ImplicitFile = @"
var a = 1;

// ------------------------------------------------
// Positive cases. All of these should return a completion list.
// ------------------------------------------------
a.|i|;
a.to|i|Expotent;
a.toExpotent|i|;
a.to|i|Expotential();
/* some comment */ a.|i|
a.|i| // some comment
;
// ------------------------------------------------
// Negative cases. These should not return symbols
// ------------------------------------------------

// Statement scope
|n|

// Statement scope in a function
function() {
  |n|
}

// After an integer.
5.|n|;

// After a double
5.3.|n| ;

// In single line comment.
a. // .|n|
;

// In a multi line comment.
a. /* 

  .|n| 

*/
;
// In strings
""unterminated string.|n|
""terminated string.|n|""
'unterminated single quote.|n|
'terminated single quote.|n|'

// In regular expressions
//  terminated
var r1 = /.|n|*/;
//  unterminated
var r1 = /abc.|n|
";
        #endregion

        [TestMethod]
        public void InfiniteLoop()
        {
            InMTA(() =>
            {
                var mySession = new AuthorTestSession(Directory.GetCurrentDirectory());
                try
                {
                    var file = mySession.FileFromText(InfiniteLoopFile);
                    var context = mySession.OpenContext(file);
                    using (CallHurryIn(context, 250))
                    {
                        var completions = context.GetCompletionsAt(InfiniteLoopFile.IndexOf("n.") + 2);
                        Assert.IsNotNull(completions);
                        Marshal.ReleaseComObject(completions);
                    }
                }
                finally
                {
                    mySession.Close();
                }

            });
        }
        #region Test data
        const string InfiniteLoopFile = @"var n = 10;                 

// This loop will never terminate.
while (true)
  ;

// We should still get completions
n.
";
        #endregion

        [TestMethod]
        public void InfiniteLoopWithProgress()
        {
            InMTA(() =>
            {
                var mySession = new AuthorTestSession(Directory.GetCurrentDirectory());
                try
                {
                    var file = mySession.FileFromText(InfiniteLoopWithProgressFile);
                    var context = mySession.OpenContext(file);
                    using (CallHurryIn(context, 250))
                    {
                        var completions = context.GetCompletionsAt(InfiniteLoopWithProgressFile.Length);
                        Assert.IsNotNull(completions);
                        completions.ToEnumerable().ExpectContains(NumberMethods);
                        Marshal.ReleaseComObject(completions);
                    }
                }
                finally
                {
                    mySession.Close();
                }

            });
        }
        #region Test data
        const string InfiniteLoopWithProgressFile = @"var n = 10;                 

// This loop will never terminate.
while (true)
  intellisense.progress();

// We should still get completions
n.";
        #endregion

        private void CopyOnWriteTest(string contextFileText, string primaryFileText, string changeToStringCode, string completionString = "a.")
        {
            CopyOnWriteTest_NoCopying(contextFileText, primaryFileText, changeToStringCode, completionString);
            CopyOnWriteTest_PrimaryModification(contextFileText, primaryFileText, changeToStringCode, completionString);
            CopyOnWriteTest_ContextModification(contextFileText, primaryFileText, changeToStringCode, completionString);
            CopyOnWriteTest_IntermediateFileModification(contextFileText, primaryFileText, changeToStringCode, completionString);
        }

        private void CopyOnWriteTest_NoCopying(string contextFileText, string primaryFileText, string changeToStringCode, string completionString)
        {
            var primaryFile = _session.FileFromText(contextFileText + "\n" + primaryFileText, "compositeFile.js");

            var context = _session.OpenContext(primaryFile);

            // Request completions
            var completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(NumberMethods);

            // Modify the string
            primaryFile.InsertText(contextFileText.Length + 1, changeToStringCode + "\n");

            // Methods should be string methods.
            completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(StringMethods);

            // Revert the text. The number value should be back.
            primaryFile.DeleteText(contextFileText.Length + 1, changeToStringCode.Length + 1);
            completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(NumberMethods);
        }

        private void CopyOnWriteTest_PrimaryModification(string contextFileText, string primaryFileText, string changeToStringCode, string completionString)
        {
            var primaryFile = _session.FileFromText(primaryFileText, "primaryFile.js");
            var contextFile = _session.FileFromText(contextFileText, "contextFile.js");

            var context = _session.OpenContext(primaryFile, contextFile);

            // Request completions
            var completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(NumberMethods);

            // Modify the primaryFile to change the value to a string.
            primaryFile.InsertText(0, changeToStringCode);

            // Methods should be string methods.
            completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(StringMethods);

            // Revert the text. The number value should be back.
            primaryFile.DeleteText(0, changeToStringCode.Length);
            completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(NumberMethods);
        }

        private void CopyOnWriteTest_ContextModification(string contextFileText, string primaryFileText, string changeToStringCode, string completionString)
        {
            var primaryFile = _session.FileFromText(primaryFileText, "primaryFile.js");
            var contextFile = _session.FileFromText(contextFileText, "contextFile.js");

            var context = _session.OpenContext(primaryFile, contextFile);

            // Request completions
            var completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(NumberMethods);

            // Modify the contextFile to change the value to a string.
            contextFile.InsertText(contextFileText.Length, changeToStringCode);

            // Methods should be string methods.
            completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(StringMethods);

            // Revert the text. The number value should be back.
            contextFile.DeleteText(contextFileText.Length, changeToStringCode.Length);
            completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(NumberMethods);
        }

        private string GetCommentText(int size)
        {
            StringBuilder comment = new StringBuilder();
            comment.Append("/*");
            for (int i = 0; i < size; i++)
            {
                comment.Append('a');
            }
            comment.Append("*/");

            return comment.ToString();
        }

        public void CopyOnWriteTest_IntermediateFileModification(string contextFileText, string primaryFileText, string changeToStringCode, string completionString)
        {
            var primaryFile = _session.FileFromText(primaryFileText, "primaryFile.js");
            var contextFile = _session.FileFromText(contextFileText, "contextFile.js");
            var commentFile = _session.FileFromText(GetCommentText(2 * 1024 * 1024), "commentFile.js"); // add a large file in the path to bypass scriptcontext folding and ensure we are copying script contexts
            var intermediateFile = _session.FileFromText("", "intermediateFile.js");

            var context = _session.OpenContext(primaryFile, contextFile, commentFile, intermediateFile);

            // Request completions
            var completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(NumberMethods);

            // Modify the contextFile to change the value to a string.
            intermediateFile.InsertText(0, changeToStringCode);

            // Methods should be string methods.
            completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(StringMethods);

            // Revert the text. The number value should be back.
            intermediateFile.DeleteText(0, changeToStringCode.Length);
            completions = context.GetCompletionsAt(primaryFile.OffsetAfter(completionString));
            completions.ToEnumerable().ExpectContains(NumberMethods);
        }

        [TestMethod]
        public void CopyOnWrite_ChangeAGlobal()
        {
            CopyOnWriteTest("var a = 10;", "a.", "a = 'string';");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeAGlobalInteger()
        {
            CopyOnWriteTest("var a = 10; var b = 23;", "if (b == 1) a = 'string'; a.", "b = 1;");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeAGlobalDouble()
        {
            CopyOnWriteTest("var a = 10; var b = 1.3;", "if (b == 1) a = 'string'; a.", "b = 1;");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeAGlobalBoolean()
        {
            CopyOnWriteTest("var a = 10; var b = false;", "if (b) a = 'string'; a.", "b = true;");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeAGlobalArray()
        {
            CopyOnWriteTest("var a = 10; var b = [];", "if (b.length) a = 'string'; a.", "b.push(1);");
        }

        [TestMethod]
        [Ignore] // This currently doesn't work because of the how get/set is stored in arrays
        public void CopyOnWrite_ChangeAGlobalES5Array()
        {
            CopyOnWriteTest(@"
                // Create an array and force it to be an ES5 array by adding a setter/getter
                var a = [10]; 
                Object.defineProperty(a, '2', 
                { 
                  get: function() { return this[0]; },
                  set: function (value) { this[0] = value; }
                });",
                "a[0].",
                "a[2] = 'string'", // Mutating index 2 should also set index 0.
                "a[0].");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeAGlobalES5Object()
        {
            CopyOnWriteTest(@"
                // Create an object with a setter getter that has a side-effect
                var a = {
                  get prop() {
                    return this._a;
                  },
                  set prop(value) {
                    this._a = value;
                  }
                };
                Object.defineProperty(a, 'foo', 
                { 
                  get: function() { return this._a; },
                  set: function (value) { this._a = value; }
                });
                a.prop = 10;",
                "a._a.",
                "a.prop = 'string';", // This should set _a to be a string.
                "a._a.");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeAnObjectValue()
        {
            CopyOnWriteTest("var o = { a: 1, b: 2 };", "o.a.", "o.a = 'string';");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeNestedObject()
        {
            CopyOnWriteTest("var o = { c: { b: { a: 1 } } };", "o.c.b.a.", "o.c.b.a = 'string';");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeRegex()
        {
            CopyOnWriteTest("var v = /a/g; v.a = 10;", "v.a.", "v.a = 'string';");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeDate()
        {
            CopyOnWriteTest("var v = new Date(); v.a = 10;", "v.a.", "v.a = 'string';");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeShared()
        {
            CopyOnWriteTest("var s = { a: 1 }; var s1 = { s: s}; var s2 = { s: s };", "s1.s.a.", "s2.s.a = 'string';");
        }

        [TestMethod]
        public void CopyOnWrite_ChangePrototype()
        {
            CopyOnWriteTest("var p = { a: 1}; var o = Object.create(p);", "o.a.", "p.a = 'string';");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeArray()
        {
            CopyOnWriteTest("var arr = [1,2,3,4];", "arr[0].", "arr[0] = 'string';", "].");
        }

        [TestMethod]
        public void CopyOnWrite_ChangeClosureCaptureValue()
        {
            CopyOnWriteTest("(function() { var v = 10; this.g = function() { return v; }; this.s = function(nv) { v = nv; }; })();", "g().", "s('string');", "g().");
        }

        [TestMethod]
        [Ignore] // This doesn't work correctly because index properties are not handled correctly when copying setter/getters.
        public void CopyOnWrite_ChangeArgumentElementProperties()
        {
            CopyOnWriteTest(@"
               var a = (function (arg0, arg1) {
                   function g() {
                      return arg0;
                   }
                   
                   function s(a) {
                      return function (v) { a[2] = v; };
                   }

                   // Turn the arguments object into an ES5 arguments object.
                   Object.defineProperty(arguments, 2, { set: function(value) { arg0 = value; }, get: function () { return arg0; }});
                   return { s: s(arguments), g: g };
               })(10, true);
", "a.g().", "a.s('string');", "a.g().");
        }

        [TestMethod]
        public void CopyOnWrite_ArrayBuffer()
        {
            CopyOnWriteTest("var buffer = new ArrayBuffer(16); (function() { var i = new Int32Array(buffer); i[0] = 0; i[1] = 1; })();", "var arr = new Int32Array(buffer); var a = arr[1] == 1 ? 1 : 'string'; a.", "(function() { var i = new Int32Array(buffer); i[1] = 2; })();");
        }

        [TestMethod]
        public void CopyOnWrite_TypedArray()
        {
            VerifyTypedArray("Int8Array");
            VerifyTypedArray("UInt8Array");
            VerifyTypedArray("UInt8ClampedArray");
            VerifyTypedArray("Int16Array");
            VerifyTypedArray("UInt16Array");
            VerifyTypedArray("Int32Array");
            VerifyTypedArray("UInt32Array");
            VerifyTypedArray("Float32Array");
            VerifyTypedArray("Float64Array");
        }

        [TestMethod]
        public void CopyOnWrite_ToPrimitiveOnNumberPrototype()
        {
            PerformCompletionRequests(@"
                var b = isFinite(Number.prototype) ? 1 : '';                
                b.|Number|;
            ");
        }



        #region Typed array utilties
        private void VerifyTypedArray(string type)
        {
            CopyOnWriteTest("var arr = new " + type + "(16); arr[1] = 1;", "var a = arr[1] == 1 ? 1 : 'string'; a.", "arr[1] = 0;", "a.");
        }
        #endregion

        [TestMethod]
        public void CopyOnWrite_CallerCalleeReference()
        {
            PerformRequests("function primary() { return arguments.callee.caller; } function context() { return primary(); } var t = context(); var a = typeof t === 'function' ? 1 : 'string'; a.|",
                (context, offset, data, index) =>
                {
                    context.GetCompletionsAt(offset).ToEnumerable().ExpectContains(NumberMethods);
                });

            PerformRequests(" function context() { return primary(); } var t = context(); var a = typeof t === 'function' ? 1 : 'string'; a.|",
                (context, offset, data, index) =>
                {
                    context.GetCompletionsAt(offset).ToEnumerable().ExpectContains(NumberMethods);
                }, "function primary() { return arguments.callee.caller; } ");
        }

        [TestMethod]
        [WorkItem(273622)]
        public void CopyOnWrite_GetterExecutionContext()
        {
            var contextFile = _session.FileFromText(@"
                   var counter = 0;
                   var o = { get Getter() { return ++counter; }};
                ");
            var primaryFile = _session.FileFromText(@"
                    var a;
                    if (o.Getter == 1)
                        a = 'string;;
                    else
                        a = 0;
  
                    a.");

            var context = _session.OpenContext(primaryFile, contextFile);
            var completion = context.GetCompletionsAt(primaryFile.Text.Length);
            completion.ToEnumerable().ExpectContains(CompletionsBase.StringMethods);

            // verify the getter was executed in the context of the primary file not the context file
            primaryFile.InsertText(0, "/*comment*/\n");
            completion = context.GetCompletionsAt(primaryFile.Text.Length);
            completion.ToEnumerable().ExpectContains(CompletionsBase.StringMethods);

        }

        [TestMethod]
        public void ParseDomJs()
        {
            var file = _session.ReadFile(Paths.DomWebPath);
            var text = file.Text + "\n document.";
            var primary = _session.FileFromText(text);
            var offset = text.Length;
            var context = _session.OpenContext(primary);
            var result = context.GetCompletionsAt(offset);
            primary.InsertText(offset, " ");
            primary.DeleteText(offset, 1);
            context.GetRegions();
            result = context.GetCompletionsAt(offset);
            Assert.IsNotNull(result);
        }

        [TestMethod]
        [WorkItem(140185)]
        public void LsCalls()
        {
            // Ensure the global _$ls called and any _$ls methods on global functions.
            var contextFile = "function _$ls() { globalLsCalled = true; }; var someGlobal = { _$ls: function() { someGlobalLsCalled = true } };";
            var primaryFile = "function() { | }";
            PerformRequests(primaryFile, (context, offset, data, index) =>
            {
                var result = context.GetCompletionsAt(offset);
                result.ToEnumerable().ExpectContains(new[] { "globalLsCalled", "someGlobalLsCalled" });
            }, contextFile);

        }

        [TestMethod]
        [WorkItem(140185)]
        public void LsCalls_ImprovedCallbacks()
        {
            // Use _$ls to improve parameter completions in callbacks.
            var contextFile = "var a = { _$ls: function () { if (this.cb) this.cb('a', 1); } };";
            var primaryFile = "a.cb = function(a, n) { a.|s|; n.|n| };";

            PerformRequests(primaryFile, (context, offset, data, index) =>
            {
                var result = context.GetCompletionsAt(offset).ToEnumerable();
                switch (data)
                {
                    case "s": result.ExpectContains(StringMethods); break;
                    case "n": result.ExpectContains(NumberMethods); break;
                }
            }, contextFile);
        }

        [TestMethod]
        public void ScopeRecord_Simple()
        {
            var primaryFile = "function foo(p_a) { var p_b; var d = {g:1, f: 1}; function bar(a, b) { var c = 1; with (d) {|n|} } }";
            PerformRequests(primaryFile, (context, offset, data, index) =>
            {
                var result = context.GetCompletionsAt(offset).ToEnumerable();
                result.ExpectContains(new[] { "foo", "p_a", "p_b", "d", "g", "f", "bar", "a", "b", "c" });
            });

        }

        [TestMethod]
        public void ScopeRecord_Global()
        {
            PerformRequests(ScopeRecord_Global_Text, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNotNull(completions, string.Format("Error at: offset : {0}, index: {1}", offset, index));
                var result = completions.ToEnumerable();
                var g = ScopeRecord_Global_Result_g;

                switch (data)
                {
                    case "g": result.ExpectContains(g); break;
                    case "r": result.ExpectContains(g.Concat("r1")); break;
                    case "e": result.ExpectContains(g.Concat("e")); break;
                    case "b": result.ExpectContains(g.Concat("b1").Concat("b2")); break;
                    case "w2": result.ExpectContains(g.Concat("w2_1").Concat("w2_2")); break;
                }
            });
        }
        #region TestData
        readonly string[] ScopeRecord_Global_Result_g = new string[] { "a", "a1", "w2" };
        const string ScopeRecord_Global_Text = @"
          var a = {};
          var a1 = {};
          var w2 = { w2_1:1, w2_2: 2};
          ;|g|this|g|;
          ;|g|a1|g|;
          ;1;
          ;|g|[1,2]|g|;
          ;|g| ({a:1, b:2});
          ;|g|(|g|a|g|)|g|;

          // 11.1.4 ArrayLiteral
          ;[|g|a|g|,|g|,|g|];
          
          // 11.1.5 ObjectLiteral
          ;({a:|g|d|g|,b:3,get c() {|g|}, set c(v){|g|}});

          // 11.2 MemberExpression
          ;a[|g|a|g|];
          ;a(|g|a|g|);
          ;new a(|g|a|g|);

          // 11.4 UnaryExpression
          ;delete 
|g|a|g|;
          ;void 
|g|a|g|;
          ;typeof 
|g|a|g|;
          ;++|g|a|g|;
          ;--|g|a|g|;
          ;+|g|a|g|;
          ;-|g|a|g|;
          ;~|g|a|g|;
          ;!|g|a|g|;

          // 11.5 MultiplicativeExpression
          ;a|g|*|g|a;
          ;a|g|/|g|a;
          ;a|g|%|g|a;

          // 11.6 AdditiveExpression
          ;a|g|+|g|a;
          ;a|g|-|g|a;

          // 11.7 ShiftExpression
          ;a|g|<<|g|a;
          ;a|g|>>|g|a;
          ;a|g|>>>|g|a;

          // 11.8 RelationalExpression
          ;a|g|<|g|a;
          ;a|g|>|g|a;
          ;a|g|<=|g|a;
          ;a|g|>=|g|a;
          ;a|g| instanceof
|g|a;
          ;a|g| in
|g|a;

          // 11.9 EqualityExpression
          ;a|g|==|g|a;
          ;a|g|!=|g|a;
          ;a|g|===|g|a;
          ;a|g|!==|g|a;
         
          // 11.10 BitwiseANDExpression, BitwiseXORExpression, BitwiseORExpression
          ;a|g|&|g|a;
          ;a|g|^|g|a;
          ;a|g|
|
|g|a;

          // 11.11 LogicalANDExpression, LogicalORexpression
          ;a|g|&&|g|a;
          ;a|g|
 ||
|g|a;

          // 11.12 ConditionalExpression
          ;|g|a|g|?|g|a|g|:|g|a|g|;
          
          // 11.13 AssignmentExpression
          ;a|g|=|g|a;
          ;a|g|*=|g|a;
          ;a|g|/=|g|a;
          ;a|g|%=|g|a;
          ;a|g|+=|g|a;
          ;a|g|-=|g|a;
          ;a|g|<<=|g|a;
          ;a|g|>>=|g|a;
          ;a|g|>>>=|g|a;
          ;a|g|&=|g|a;
          ;a|g|^=|g|a;
          ;a|g|
|=|g|a;

          // 11.14 Expression
          ;a|g|,|g|a;

          // 12.1 Block
          ;{|g|a;|g|;|g|};
          
          // 12.2 VariableStatement
          (function () {
            ;var r1 =|r|a|r|,r2=|r|a|r|;
          })();

          // 12.3 EmptyStatement
          ;|g|;|g|;
          
          // 12.4 ExpressionStatement
          ;|g|a|g|;

          // 12.5 IfStatement
          ;|g|if(|g|a|g|)|g|a|g|;
          ;|g|if(|g|a|g|)|g|a|g|;|g| 
           else|g|a|g|;

          // 12.6 IterationStatement
          ;|g|do|g| a|g|; while(|g|false|g|);
          ;|g|while(|g|a1++<tmp|g|)|g|a|g|;
          ;|g|for(|g|a|g|;|g|a1<tmp|g|;|g|a1++|g|)|g|a|g|;
          (function () {
            ;|r|for(var r1=|r|a|r|;|r|r1<tmp|r|;|r|r1++|g|)|r|a|r|;
          })();
          ;|g|for(a|g| in
|g|a|g|)|g|a|g|;
          (function () {
|g|for(var r1 in
|r|a)|r|a|r|;
          })();
          
          // 12.7 ContinueStatement
          a1=1;
          ;|g|while(a1++<2){|g|continue;|g|};

          // 12.8 BreakStatement
          ;|g|while(a1++<3){|g|break;|g|};

          // 12.9 ReturnStatement
          (function() {|g|return;|g|})();

          // 12.10 WithStatement
          ;|g|with(|g|w2|g|)|w2|a|w2|;
          
          // 12.11 SwitchStatement
          ;|g|switch(|g|a|g|){case 1:|g|a|g|;default:|g|a|g|;};

          // 12.13 ThrowStatement
          ;|g|throw|g| a
|g|;

          // 12.14 TryStatement
          ;|g|try {|g|a|g|;|g|} catch(e) {|e|a|e|;|e|} finally {|g|a|g|;|g|};

          // 12.15 DebuggerStatement
          ;|g|debugger
|g|;

          // 13 FunctionDeclaration, FunctionExpression
          ;|g|function bar(b1,b2){|b|a|b|;|b|};
          ;|g|(function(){|g|a|g|;|g|})();
";
        #endregion

        [TestMethod]
        public void ScopeRecord_Nested()
        {
            PerformRequests(ScopeRecord_Nested_Text, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNotNull(completions, string.Format("Error at: offset : {0}, index: {1}", offset, index));
                var result = completions.ToEnumerable();
                var g = ScopeRecord_Nested_Result_g;

                switch (data)
                {
                    case "g": result.ExpectContains(g); break;
                    case "r": result.ExpectContains(g.Concat("r1")); break;
                    case "e": result.ExpectContains(g.Concat("e")); break;
                    case "b": result.ExpectContains(g.Concat("b1").Concat("b2")); break;
                    case "w2": result.ExpectContains(g.Concat("w2_1").Concat("w2_2")); break;
                }
            });
        }
        #region Test data
        readonly string[] ScopeRecord_Nested_Result_g = new string[] { "a", "foo", "b", "a_1", "w", "w2", "w_1", "w_2", "c", "d", "a1", "a2", "a3" };
        const string ScopeRecord_Nested_Text = @"
var a = 0;
function foo(b) {
  var a_1 = 1;
  var w = { w_1: 1, w_2: 2 };
  var w2 = { w2_1:1, w2_2: 2};
  function c(d) {
    var c_1 = 1;
    with (w) {
      (function(a1, a2, a3) {
          // 11.1 PrimaryExpression
          ;|g|this|g|;
          ;|g|a1|g|;
          ;1;
          ;|g|[1,2]|g|;
          ;|g| ({a:1, b:2});
          ;|g|(|g|a|g|)|g|;

          // 11.1.4 ArrayLiteral
          ;[|g|a|g|,|g|,|g|];
          
          // 11.1.5 ObjectLiteral
          ;({a:|g|d|g|,b:3,get c() {|g|}, set c(v){|g|}});

          // 11.2 MemberExpression
          ;a[|g|a|g|];
          ;a(|g|a|g|);
          ;new a(|g|a|g|);

          // 11.4 UnaryExpression
          ;delete 
|g|a|g|;
          ;void 
|g|a|g|;
          ;typeof 
|g|a|g|;
          ;++|g|a|g|;
          ;--|g|a|g|;
          ;+|g|a|g|;
          ;-|g|a|g|;
          ;~|g|a|g|;
          ;!|g|a|g|;

          // 11.5 MultiplicativeExpression
          ;a|g|*|g|a;
          ;a|g|/|g|a;
          ;a|g|%|g|a;

          // 11.6 AdditiveExpression
          ;a|g|+|g|a;
          ;a|g|-|g|a;

          // 11.7 ShiftExpression
          ;a|g|<<|g|a;
          ;a|g|>>|g|a;
          ;a|g|>>>|g|a;

          // 11.8 RelationalExpression
          ;a|g|<|g|a;
          ;a|g|>|g|a;
          ;a|g|<=|g|a;
          ;a|g|>=|g|a;
          ;a|g| instanceof
|g|a;
          ;a|g| in
|g|a;

          // 11.9 EqualityExpression
          ;a|g|==|g|a;
          ;a|g|!=|g|a;
          ;a|g|===|g|a;
          ;a|g|!==|g|a;
         
          // 11.10 BitwiseANDExpression, BitwiseXORExpression, BitwiseORExpression
          ;a|g|&|g|a;
          ;a|g|^|g|a;
          ;a|g|
|
|g|a;

          // 11.11 LogicalANDExpression, LogicalORexpression
          ;a|g|&&|g|a;
          ;a|g|
 ||
|g|a;

          // 11.12 ConditionalExpression
          ;|g|a|g|?|g|a|g|:|g|a|g|;
          
          // 11.13 AssignmentExpression
          ;a|g|=|g|a;
          ;a|g|*=|g|a;
          ;a|g|/=|g|a;
          ;a|g|%=|g|a;
          ;a|g|+=|g|a;
          ;a|g|-=|g|a;
          ;a|g|<<=|g|a;
          ;a|g|>>=|g|a;
          ;a|g|>>>=|g|a;
          ;a|g|&=|g|a;
          ;a|g|^=|g|a;
          ;a|g|
|=|g|a;

          // 11.14 Expression
          ;a|g|,|g|a;

          // 12.1 Block
          ;{|g|a;|g|;|g|};
          
          // 12.2 VariableStatement
          (function () {
            ;var r1 =|r|a|r|,r2=|r|a|r|;
          })();

          // 12.3 EmptyStatement
          ;|g|;|g|;
          
          // 12.4 ExpressionStatement
          ;|g|a|g|;

          // 12.5 IfStatement
          ;|g|if(|g|a|g|)|g|a|g|;
          ;|g|if(|g|a|g|)|g|a|g|;|g| 
           else|g|a|g|;

          // 12.6 IterationStatement
          ;|g|do|g| a|g|; while(|g|false|g|);
          ;|g|while(|g|a1++<tmp|g|)|g|a|g|;
          ;|g|for(|g|a|g|;|g|a1<tmp|g|;|g|a1++|g|)|g|a|g|;
          (function () {
            ;|r|for(var r1=|r|a|r|;|r|r1<tmp|r|;|r|r1++|g|)|r|a|r|;
          })();
          ;|g|for(a|g| in
|g|a|g|)|g|a|g|;
          (function () {
|g|for(var r1 in
|r|a)|r|a|r|;
          })();
          
          // 12.7 ContinueStatement
          a1=1;
          ;|g|while(a1++<2){|g|continue;|g|};

          // 12.8 BreakStatement
          ;|g|while(a1++<3){|g|break;|g|};

          // 12.9 ReturnStatement
          (function() {|g|return;|g|})();

          // 12.10 WithStatement
          ;|g|with(|g|w2|g|)|w2|a|w2|;
          
          // 12.11 SwitchStatement
          ;|g|switch(|g|a|g|){case 1:|g|a|g|;default:|g|a|g|;};

          // 12.13 ThrowStatement
          ;|g|throw|g| a
|g|;

          // 12.14 TryStatement
          ;|g|try {|g|a|g|;|g|} catch(e) {|e|a|e|;|e|} finally {|g|a|g|;|g|};

          // 12.15 DebuggerStatement
          ;|g|debugger
|g|;

          // 13 FunctionDeclaration, FunctionExpression
          ;|g|function bar(b1,b2){|b|a|b|;|b|};
          ;|g|(function(){|g|a|g|;|g|})();
          
      })(1, 2, 3);
    }
  }
}

";
        #endregion

        [TestMethod]
        public void ObjectKind()
        {
            PerformRequests("var u = undefined; var n = null; var d = {}; var e = d.foo(); u.|undefined|; n.|null|;  d.|dynamic|; e.|error|;|none|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                var objectKind = completions.ObjectKind;
                switch (data)
                {
                    case "undefined": Assert.AreEqual(AuthorCompletionObjectKind.acokUndefined, objectKind); break;
                    case "null": Assert.AreEqual(AuthorCompletionObjectKind.acokNull, objectKind); break;
                    case "dynamic": Assert.AreEqual(AuthorCompletionObjectKind.acokDynamic, objectKind); break;
                    case "error": Assert.AreEqual(AuthorCompletionObjectKind.acokError, objectKind); break;
                    case "none": Assert.AreEqual(AuthorCompletionObjectKind.acokNone, objectKind); break;
                    default: Assert.Fail(string.Format("Unexpected data in test '{0}'", data)); break;
                }
            });
        }

        [TestMethod]
        [WorkItem(391538)]
        public void ReturnDocComments_DomTypes()
        {
            var domFile = _session.ReadFile(Paths.DomWebPath);

            PerformCompletionRequests(@"
                function testDocument() {
                    /// <returns type='Document'> return desc </returns>
                };
                function testWindow() {
                    /// <returns type='Window'> return desc </returns>
                };
                function testNode() {
                    /// <returns type='Node'> return desc </returns>
                };
                function testHTMLFormElement() {
                    /// <returns type='HTMLFormElement'> return desc </returns>
                };
                function testHTMLElement() {
                    /// <returns type='HTMLElement'> return desc </returns>
                };
                function testHTMLElementImplicit() {
                    /// <returns domElement='true'> return desc </returns>
                };
                function testMSPointerEvent() {
                    /// <returns type='MSPointerEvent'> return desc </returns>
                };

                testDocument().|doctype,xmlVersion,documentElement,createComment|;
                testWindow().|document,navigator,location|;
                testNode().|nodeType,previousSibling,nodeName|;
                testHTMLFormElement().|submit,action,method,name|;
                testHTMLElement().|onload,title,children,contains|;
                testHTMLElementImplicit().|onload,title,children,contains|;
                testMSPointerEvent().|width,rotation,pressure,pointerType,tiltY,tiltX,pointerId|;"
                , domFile.Text);
        }

        [TestMethod]
        [WorkItem(391538)]
        public void ParamDocComments_DomTypes()
        {
            var domFile = _session.ReadFile(Paths.DomWebPath);

            PerformCompletionRequests(@"
                 function testDocument(documentParam, windowParam, nodeParam, htmlFormElementparam, htmlElementParam, htmlElementImplicitwindowParam, msPointerEventParam) {
                        /// <signature> 
                        /// <param name='documentParam' type='Document'/>
                        /// <param name='windowParam' type='Window'/>
                        /// <param name='nodeParam' type='Node'/>
                        /// <param name='htmlFormElementparam' type='HTMLFormElement'/>
                        /// <param name='htmlElementParam' type='HTMLElement'/>
                        /// <param name='htmlElementImplicitwindowParam' domElement='true'/>
                        /// <param name='msPointerEventParam' type='MSPointerEvent'>Any relevant pointer event for the scrollzone</param>
                        /// </signature> 

                        documentParam.|doctype,xmlVersion,documentElement,createComment|;
                        windowParam.|document,navigator,location|;
                        nodeParam.|nodeType,previousSibling,nodeName|;
                        htmlFormElementparam.|submit,action,method,name|;
                        htmlElementParam.|onload,title,children,contains|;
                        htmlElementImplicitwindowParam.|onload,title,children,contains|;
                        msPointerEventParam.|width,rotation,pressure,pointerType,tiltY,tiltX,pointerId|;
                    }", domFile.Text);
        }

        [TestMethod]
        [WorkItem(196600)]
        public void GetIdentifiers()
        {
            PerformRequests("var a; var b; function foo(p1, p2) { this.bar = p1; }; function foo2() { c = 1; var c = 2; } var f = new foo(); f.|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfFileIdentifiersFilter);
                var results = completions.ToEnumerable();
                results.ExpectContains("a", "b", "foo", "p1", "p2", "bar", "foo2", "c", "f");
                foreach (var result in results)
                {
                    switch (result.Name)
                    {
                        case "a":
                        case "b":
                        case "c":
                        case "f":
                            Assert.AreEqual(AuthorCompletionKind.ackVariable, result.Kind);
                            break;
                        case "bar":
                            Assert.AreEqual(AuthorCompletionKind.ackField, result.Kind);
                            break;
                        case "foo":
                        case "foo2":
                            Assert.AreEqual(AuthorCompletionKind.ackMethod, result.Kind);
                            break;
                        case "p1":
                        case "p2":
                            Assert.AreEqual(AuthorCompletionKind.ackParameter, result.Kind);
                            break;
                    }
                }
            });
        }

        [TestMethod]
        [WorkItem(203379)]
        public void StrictModeFunction()
        {
            PerformRequests("function f(){ 'use strict'; } f.|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNotNull(completions);
            });
        }

        [TestMethod]
        [WorkItem(192685)]
        public void ArrayBuffer()
        {
            PerformCompletionRequests("var buffer = new ArrayBuffer(16); buffer.|", (completions, data, index) =>
            {
                completions.ExpectContains("constructor");
            });
        }

        [TestMethod]
        public void ArrayExpando()
        {
            PerformCompletionRequests("var foo2 = new Array(); foo2.bar = 3; foo2.|", (completions, data, index) =>
            {
                completions.ExpectContains("bar");
            });
        }

        //[TestMethod]
        //[WorkItem(210333)]
        //public void Bug210333()
        //{
        //    var defaultjs = _session.ReadFile(Path.Combine(Paths.FilesPath, @"210333\default_210333.js")).Text;
        //    PerformHurriedCompletionRequests(defaultjs, (completions, data) =>
        //    {
        //        Assert.IsNotNull(completions);
        //        Assert.AreEqual(AuthorCompletionObjectKind.acokNone, completions.ObjectKind);
        //    },
        //        false,
        //        Path.Combine(Paths.FilesPath, @"210333\winrt_210333.js"),
        //        Paths.SiteTypesWebPath,
        //        Paths.DomWebPath,
        //        Path.Combine(Paths.FilesPath, @"210333\base_210333.js"),
        //        Path.Combine(Paths.FilesPath, @"210333\wwaapp_210333.js"),
        //        Paths.JQuery.JQuery_1_6_1FilePath,
        //        Path.Combine(Paths.FilesPath, @"210333\context_210333.js")
        //    );
        //}

        [TestMethod]
        [WorkItem(204139)]
        public void WinRtCrash()
        {
            PerformRequests(WinRtCrashText, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNotNull(completions);
            }, true, TestFiles.winrt);
        }
        #region Test data
        const string WinRtCrashText = @"function Log(str) {
    var el = document.getElementById(""Log"");
    el.innerHTML = str + ""<br />"" + el.innerHTML;
}
var s;
var defaultService = ""4529"";
var remoteHost;
var PlayerName = ""DoNotKnowName"";
var lookfor;
var playerlist;
var picture;
var winner;
var winnername;
var playernames = {};
function Setup() {
    lookfor = document.getElementById (""LookFor"");
    playerlist = document.getElementById (""PlayerList"");
    picture = document.getElementById (""Picture"");
    winner = document.getElementById (""Winner"");
    winnername = document.getElementById (""WinnerName"");
}

function OnLogIn() {
    Setup();
    var remote = document.getElementById(""ConnectTo"").value;
    remoteHost = new Windows.Networking.HostName (remote);
    s = new Windows.Networking.Sockets.StreamSocket;
    var async = s.connectAsync (remoteHost, defaultService, Windows.Networking.Sockets.SocketProtectionLevel.plainSocket);
    async.completed = OnConnected;
    async.start();
}        

function OnConnected(conn) {
    PlayerName = document.getElementById(""Name"").value;
    SendPlayer (PlayerName);
    async = s.readAsync(12, 0); // 0==read exactly 12 bytes
    async.completed = OnReadOpcodeStart;
    async.start()
}
function OnSend (conn) {
} 

var currOp = {};
function OnReadOpcodeStart (status) {
    if (status.errorCode == 0) {
        var d = status.getResults();
        currOp.headerlen = d.readInt32();
        currOp.datalen = d.readInt32();
        currOp.opcode = d.readInt32();
        if (currOp.headerlen > 12) {
            async = s.readAsync (currOp.headerlen - 12, 0);
            async.completed = OnReadOpcodeFinal;
            async.start();
        } else if (currOp.datalen > 0) {
            async = s.readAsync (currOp.datalen, 0);
            async.completed = OnReadDataFinal;
            async.start();
        } else {
            OnOpcodeComplete();
        }
    }
}
function OnReadOpcodeFinal(status) {
    if (status.errorCode == 0) {
        // read in data depending on opcode
        var d = status.getResults();
        var NRead = 0;
        switch (currOp.opcode) {
            case 9: // compressed picture
                currOp.h = d.readInt32();
                currOp.w = d.readInt32();
                NRead = 8;
                break;
            case 11:
                currOp.index = d.readInt32();
                NRead = 4;
                break;
            case 13: // found
                currOp.x = d.readInt32();
                currOp.y = d.readInt32();
                currOp.h = d.readInt32();
                currOp.w = d.readInt32();
                NRead = 16;
                break;
            case 15: // player score
                currOp.score = d.readInt32();
                currOp.index = d.readInt32();
                NRead = 8;
                break;
        }
        var NLeft = currOp.headerlen - 12 - NRead;
        if (NLeft > 0) {
            d.readBytes (NLeft);
        }
        if (currOp.datalen > 0) {
            async = s.readAsync (currOp.datalen, 0);
            async.completed = OnReadDataFinal;
            async.start();
        } else {
            OnOpcodeComplete();
        }
    } // end if everything OK
}
function dot(x, y) {
    ctx.moveTo (x, y);
    ctx.lineTo (x + 1, y);
}
function line(x, y, dx, dy) {
    ctx.moveTo(x, y);
    ctx.lineTo (x + dx, y + dy);
}
function SetColor (data, idx, r, g, b, a) {
    data[idx * 4 + 0] = r;
    data[idx * 4 + 1] = g;
    data[idx * 4 + 2] = b;
    data[idx * 4 + 3] = a;
}
var canwidth = 500;
var canheight = 500;
// Handles reading in the DATA part of the opcode
function OnReadDataFinal(status) {
    if (status.errorCode == 0) {
        var d = status.getResults();
        switch (currOp.opcode) {
            case 9: // Picture -- is handled right here instead of later on
                picture.height = currOp.h;
                picture.width = currOp.w; 
                var ratio = picture.height / picture.width;
                if (currOp.h > currOp.w) {
                    canheight = 500;
                    canwidth = Math.min(500 / ratio);
                } else {
                    canheight = Math.min(500 * ratio);
                    canwidth = 500;
                }
                picture.style.width = canwidth + ""px"";
                picture.style.height = canheight + ""px"";
                ctx = picture.getContext(""2d"");
                ctx.fillStyle = ""rgb(255,255,255)"";
                ctx.fillRect (0, 0, picture.height, picture.width);
                ctx.beginPath();
                ctx.strokeStyle = ""rgb(50, 200, 50)"";
                ctx.moveTo(0, 0);
                ctx.lineTo (currOp.w - 1, 0);
                ctx.lineTo (currOp.w - 1, currOp.h - 1);
                ctx.lineTo (0, currOp.h - 1);
                ctx.lineTo (0, 0);
                ctx.stroke();
                var r = 50;
                var g = 200;
                var b = 50;
                var imgdata = ctx.getImageData(0, 0, picture.width, picture.height);
                var pixelarray = imgdata.data;
                for (var i = 0; i < currOp.datalen; i++) {
                    var byte = d.readByte();
                    if ((byte & 0x01) == 0) SetColor (pixelarray, i * 8 + 0, r, g, b, 255);
                    if ((byte & 0x02) == 0) SetColor (pixelarray, i * 8 + 1, r, g, b, 255);
                    if ((byte & 0x04) == 0) SetColor (pixelarray, i * 8 + 2, r, g, b, 255);
                    if ((byte & 0x08) == 0) SetColor (pixelarray, i * 8 + 3, r, g, b, 255);
                    if ((byte & 0x10) == 0) SetColor (pixelarray, i * 8 + 4, r, g, b, 255);
                    if ((byte & 0x20) == 0) SetColor (pixelarray, i * 8 + 5, r, g, b, 255);
                    if ((byte & 0x40) == 0) SetColor (pixelarray, i * 8 + 6, r, g, b, 255);
                    if ((byte & 0x80) == 0) SetColor (pixelarray, i * 8 + 7, r, g, b, 255);
                }
                ctx.putImageData (imgdata, 0, 0);
                break;

            default: // pretty much all ""data"" is really a string
                currOp.str = d.readString(currOp.datalen);
                break;
        }
        OnOpcodeComplete();
    }
}
function X(value) {
    var Retval = value / 1000.0 * picture.width;
    return Retval;
}
function Y(value) {
    var Retval = value / 1000.0 * picture.height
    return Retval;
}

var winnert;
function OnOpcodeComplete() {
    //Log (""Note: got a complete opcode"" + currOp.opcode);
    switch (currOp.opcode) {
        case 7: // picture start
            // Reset the canvas to be greyish
            ctx = picture.getContext(""2d"");
            ctx.fillStyle = ""rgba(100,100,100,.2)"";
            ctx.fillRect (0, 0, picture.width, picture.height);
            break;
        case 10: // look for
            lookfor.innerHTML = currOp.str;
            break;
        case 11: // allfound
            var name = playernames[currOp.index];
            winnername.innerHTML = name + ""("" + currOp.index + "")"";
            winner.style.visibility = ""visible"";
            winnert = setTimeout (""winner.style.visibility = 'hidden'"", 3000);
            break;
        case 13: // found
            ctx.beginPath();
            ctx.strokeStyle = ""red"";
            ctx.moveTo (X(currOp.x), Y(currOp.y));
            ctx.lineTo (X(currOp.x + currOp.w), Y(currOp.y));
            ctx.lineTo (X(currOp.x + currOp.w), Y(currOp.y + currOp.h));
            ctx.lineTo (X(currOp.x), Y(currOp.y + currOp.h));
            ctx.lineTo (X(currOp.x), Y(currOp.y));
            ctx.fillStyle = ""rgba(255, 0, 0, .3)"";
            ctx.fillRect (X(currOp.x), Y(currOp.y), X(currOp.w), Y(currOp.h));
            ctx.stroke();
            break;
        case 14: // Player score start
            nr = playerlist.rows.length;
            for (var i = 1; i < nr; i++) playerlist.deleteRow(1)
            break;
        case 15: // Player
            var r = playerlist.insertRow(-1); // -1=at end
            var c = r.insertCell ();
            c.innerHTML = currOp.str
            c = r.insertCell();
            c.innerHTML = currOp.score
            playernames[currOp.index] = currOp.str;
            break;
    }
    async = s.readAsync(12, 0); // 0==read exactly 12 bytes
    async.completed = OnReadOpcodeStart;
    async.start();
    var d = new Windows.Networking.Sockets.StreamSocket();
    d.information.localHostName.ipInformation.networkAdapter.network.networkType.|
    info = s.information();
    }
function OnClick(e) {
    var x = 1000 * e.offsetX / canwidth;
    var y = 1000 * e.offsetY / canheight;
    SendClick (x, y, PlayerName); 
}

function SendClick (x, y, PlayerName) {
    var dp = new Windows.Networking.Data();
    dp.writeString (PlayerName);
    var dplen = dp.length;
    var d = new Windows.Networking.Data();
    d.byteOrder = Windows.Networking.ByteOrder.bigEndian;
    d.writeInt32 (12 + 8); // header len
    d.writeInt32 (dplen);
    d.writeInt32 (12) // Opcode: 12=click
    d.writeInt32 (x);
    d.writeInt32 (y);
    d.writeString (PlayerName);
    d.position = 0;
    var async = s.writeAsync (d);
    async.completed = OnSend;
    async.start();
}
function SendPlayer(name) {
    var dp = new Windows.Networking.Data();
    dp.writeString (PlayerName);
    var dplen = dp.length;
    var d = new Windows.Networking.Data();
    d.byteOrder = Windows.Networking.ByteOrder.bigEndian;
    d.writeInt32 (12); // header len
    d.writeInt32 (dplen);
    d.writeInt32 (2) // Opcode: 2=player
    d.writeString (PlayerName);
    d.position = 0;
    var async = s.writeAsync (d);
    async.completed = OnSend;
    async.start();
}";
        #endregion

        [TestMethod]
        [WorkItem(246844)]
        public void FileCloseBeforeGetHintFor()
        {
            var c0 = _session.FileFromText("function bar() { }\nbar._$doc = function() { \n///<summary>Some text</summary>\n//<param name='a' type='number'>Some parameter</param>\n };");
            var c1 = _session.FileFromText("var a = {foo: {}, bar: bar};");
            var c2 = _session.FileFromText("var b = {foo: {}, bar: {}};");

            var primary = _session.FileFromText("a.");
            var context = _session.OpenContext(primary, c0, c1, c2);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            Assert.IsNotNull(completions);

            Action<IAuthorTestFile> remove = (file) =>
            {
                // Remove c2 from the list
                var handle = _session.TakeOwnershipOf(file);
                handle.Close();
                context.RemoveContextFiles(file);
                Marshal.ReleaseComObject(handle);
            };

            remove(c1);
            remove(c2);

            // Get a hint for bar 
            int entryIndex = -1;
            var completion = completions.ToEnumerable().Where((c, index) =>
            {
                if (c.DisplayText == "bar")
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
        }


        private void PerformExtentRequest(string text)
        {
            var start = 0;
            PerformRequests(text, (context, offset, data, index) =>
            {
                switch (data)
                {
                    case "start":
                        start = offset;
                        break;
                    case "complete":
                        var results = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfFileIdentifiersFilter);
                        Assert.AreEqual(start, results.Extent.Offset);
                        Assert.AreEqual(offset - start, results.Extent.Length);
                        break;
                    default:
                        Assert.Fail("Check test string");
                        break;
                }
            });
        }

        [TestMethod]
        [WorkItem(209214)]
        public void ExtentOfFileIdentifier()
        {
            string text = @"
function test() {
  var divContent = null;
  var a = null;
  a.|start|di|complete|
}";
            PerformExtentRequest(text);
        }

        [TestMethod]
        [WorkItem(303644)]
        public void ExtentOfAnIdentifierInParenthesisedExpression()
        {
            PerformExtentRequest(@" (|start|app|complete| ");
        }

        [TestMethod]
        [WorkItem(323062)]
        public void CompletionAfterACursorSubTree()
        {
            var primaryFile = _session.FileFromText("function foo() { var f, b, test; ; }");
            var offset = primaryFile.OffsetAfter("; ;");
            var context = _session.OpenContext(primaryFile);
            var completions = context.GetCompletionsAt(offset);
            var cursor = context.GetASTCursor();
            var subtree = cursor.GetSubTree(3);
            Marshal.ReleaseComObject(subtree);
            Marshal.ReleaseComObject(cursor);
            var completions2 = context.GetCompletionsAt(offset);
        }

        [TestMethod]
        [WorkItem(214048)]
        public void MalformedStringLiteral()
        {
            PerformRequests(MalformedStringLiteralText, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNotNull(completions);
            });
        }
        #region Test data
        const string MalformedStringLiteralText = @"document.write('<html><head>');
document.write('<LINK HREF=""http://special.auctions.yahoo.co.jp/html/css/affiliate.css"" REL=""stylesheet"" TYPE=""text/css"">');
document.write('</head><body>');
document.write('<TABLE WIDTH=""120"" HEIGHT=""300"" BORDER=""0"" CELLPADDING=""1"" CELLSPACING=""0"" BGCOLOR=""#0000ff"">');
document.write('<TR>');
document.write('<TD VALIGN=""TOP"">');
document.write('<TABLE WIDTH=""118"" HEIGHT=""298"" CELLPADDING=""0"" CELLSPACING=""0"" BGCOLOR=""#ffffff"" BORDER=""0"">');
document.write('<TR>');
document.write('<TD VALIGN=""TOP"">');
document.write('<TABLE CELLPADDING=""0"" CELLSPACING=""0"" BORDER=""0"" WIDTH=""100%"">');
document.write('<TR>');
document.write('<TD ALIGN=""CENTER"" VALIGN=""TOP""><A HREF=""http://ck.jp.ap.valuecommerce.com/servlet/referral?va=2170806&rid=1&sid=2219441&pid=874226848&vcptn=auct/p/ZIeOj9dxVKf4_43EsIUcqVn6&vc_url=http%3A%2F%2Fpage6.auctions.yahoo.co.jp/jp%2Fauction%2Ff92791712"" TARGET=""yauc""><IMG SRC=""http://f2.auctions.c.yimg.jp/img199.auctions.yahoo.co.jp/users/3/7/6/2/exc0926-thumb-1289213608402584.jpg"" ALT=""\u65b0\u54c1/PS3\u30bd\u30d5\u30c8/\u30a6\u30a4\u30cb\u30f3\u30b0\u30a4\u30ec\u30d6\u30f32011/\u30a6\u30a4\u30a4\u30ec2011/\u7279\u5178\u4ed8\u304d!"" WIDTH=""50"" HEIGHT=""65"" VSPACE=""1"" ALIGN=""ABSMIDDLE"" BORDER=""0""></A></TD>');
document.write('</TR>');
document.write('<TR>');
document.write('<TD>');
document.write('<TABLE CELLPADDING=""0"" CELLSPACING=""0"" BORDER=""0"" WIDTH=""100%"">');
document.write('<TR>');
document.write('<TD ALIGN=""CENTER""><A HREF=""http://ck.jp.ap.valuecommerce.com/servlet/referral?va=2170806&rid=1&sid=2219441&pid=874226848&vcptn=auct/p/ZIeOj9dxVKf4_43EsIUcqVn6&vc_url=http%3A%2F%2Fpage6.auctions.yahoo.co.jp/jp%2Fauction%2Ff92791712"" TITLE=""\u65b0\u54c1/PS3\u30bd\u30d5\u30c8/\u30a6\u30a4\u30cb\u30f3\u30b0\u30a4\u30ec\u30d6\u30f32011/\u30a6\u30a4\u30a4\u30ec2011/\u7279\u5178\u4ed8\u304d!"" TARGET=""yauc""><FONT COLOR=""#0000ff"" CLASS=""style1_2 wb"">\u65b0\u54c1/PS3\u30bd\u30d5\u30c8/\u30a6\u30a4\u30cb\u30f3\u30b0\u30a4\u30ec\u30d6\u30f32011/\u30a6...</FONT></A></TD>');
document.write('</TR>');
document.write('<TR>');
document.write('<TD ALIGN=""CENTER""><FONT COLOR=""#000000"" CLASS=""style2_2 wb"">5,849 \u5186<BR>');
document.write('54 \u5165\u672d/\u6b8b\u308a3\u65e5</FONT></TD>');
document.write('</TR>');
document.write('</TABLE>');
document.write('</TD>');
document.write('</TR>');
document.write('</TABLE>');
document.write('<TABLE CELLPADDING=""0"" CELLSPACING=""0"" BORDER=""0"" WIDTH=""100%"">');
document.write('<TR>');
document.write('<TD ALIGN=""CENTER"" VALIGN=""TOP""><A HREF=""http://ck.jp.ap.valuecommerce.com/servlet/referral?va=2170806&rid=1&sid=2219441&pid=874226848&vcptn=auct/p/ZIeOj9dxVKf4_43EsIUcqVn6&vc_url=http%3A%2F%2Fpage11.auctions.yahoo.co.jp/jp%2Fauction%2Fn88673835"" TARGET=""yauc""><IMG SRC=""http://f6.auctions.c.yimg.jp/img307.auctions.yahoo.co.jp/users/0/2/4/5/sanden2201-thumb-1289062502193170.jpg"" ALT=""PS3 80GB \u
|";
        #endregion

        [TestMethod]
        [WorkItem(206845)]
        public void LiteralErrorCorrection()
        {
            PerformRequests(LiteralErrorCorrectionText, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                var results = completions.ToEnumerable();
                results.ExpectContains("currentTallest");
            });
        }
        #region Test data
        const string LiteralErrorCorrectionText = @"$.fn.equalHeights = function (px) {
$(this).each(function () {
var currentTallest = 0;
$(this).children().each(function (i) {
if ($(this).height() > currentTallest) {
currentTallest = $(this).height();
}
});
if (!px || !Number.prototype.pxToEm) currentTallest = currentTallest.pxToEm($(this));
var ie6 = (navigator.appName == ""Microsoft Internet Explorer"" && parseInt(navigator.appVersion) == 4 && navigator.appVersion.indexOf(""MSIE 6.0"") != -1);
if ($.browser.msie && (ie6)) {
$(this).children().css({ 'height': curren|  ";
        #endregion

        [TestMethod]
        [WorkItem(202426)]
        public void AfterVarBeforeExpression()
        {
            PerformCompletionRequests("var s = 'abc'; s.| (function () {})();", (completions, data, index) =>
            {
                completions.ExpectContains(StringMethods);
            });
        }

        //[TestMethod]
        //public void LotsOfLittleForIns()
        //{
        //    PerformHurriedCompletionRequests(LotsOfLittleForInsText, _session.ReadFile(Paths.DomWebPath).Text);
        //}
        #region Test data
        const string LotsOfLittleForInsText = @"var htmlElements = [
	HTMLAnchorElement,
	HTMLAreaElement,
	HTMLAudioElement,
	HTMLBaseElement,
	HTMLBaseFontElement,
	HTMLBGSoundElement,
	HTMLBlockElement,
	HTMLBodyElement,
	HTMLBRElement,
	HTMLButtonElement,
	HTMLCanvasElement,
	HTMLDDElement,
	HTMLDivElement,
	HTMLDListElement,
	HTMLDTElement,
	HTMLEmbedElement,
	HTMLFieldSetElement,
	HTMLFontElement,
	HTMLFormElement,
	HTMLFrameElement,
	HTMLFrameSetElement,
	HTMLHeadElement,
	HTMLHeadingElement,
	HTMLHRElement,
	HTMLHtmlElement,
	HTMLIFrameElement,
	HTMLImageElement,
	HTMLInputElement,
	HTMLIsIndexElement,
	HTMLLabelElement,
	HTMLLegendElement,
	HTMLLIElement,
	HTMLLinkElement,
	HTMLMapElement,
	HTMLMarqueeElement,
	HTMLMetaElement,
	HTMLNextIdElement,
	HTMLObjectElement,
	HTMLOListElement,
	HTMLOptionElement,
	HTMLParagraphElement,
	HTMLParamElement,
	HTMLPhraseElement,
	HTMLScriptElement,
	HTMLSelectElement,
	HTMLSpanElement,
	HTMLStyleElement,
	HTMLTableCaptionElement,
	HTMLTableCellElement,
	HTMLTableColElement,
	HTMLTableElement,
	HTMLTableRowElement,
	HTMLTableSectionElement,
	HTMLTextAreaElement,
	HTMLTitleElement,
	HTMLUListElement,
	HTMLVideoElement,
];

intellisense.addEventListener('statementcompletion', function () { });

var options = [true, false];
var loopCount = 0;

function generatePropertyDescriptors() {
    var descs = {};
    var propCount = 0;
    for (var a in options)										// accessor
        for (var w in options)									// writable
            for (var c in options)								// configurable
                for (var e in options) {						// enumerable
                    var prop = 'prop' + propCount++;
                     loopCount++;
                    descs[prop] = { configurable: options[c], enumerable: options[e] };
                    if (options[a]) {
                        if (options[w]) {
                            descs[prop].get = function () { return this._prop; }
                            descs[prop].set = function (v) { this._prop = v; }
                        }
                        else descs[prop].get = function () { return ""get""; }
                    }
                    else {
                        descs[prop].value = 'value';
                        descs[prop].writable = options[w];
                    }
                }
    return descs;
}
var descs = generatePropertyDescriptors();		// global property bag

intellisense.logMessage('made it past generatePropertyDescriptors, loopCount = ' + loopCount);

function test() {
    function verifyPropertyDescriptors(d1, d2, o) {
        var items = ['writable', 'configurable', 'get', 'set', 'enumerable'];
        for (var i in items) {
            var item = items[i];
        }
    }

    function verifyProperties(o, prop, desc) {
        o[prop] = prop;
        if (desc.writable || desc.set) {
            delete o[prop];
            delete o._prop;
        }
    }

    loopCount = 0;

    // Test for Object.create(o) and Object.defineProperty()
    (function () {
        for (var o in htmlElements) {
            var e = htmlElements[o];
            var i = Object.create(e);

            for (var pn in descs) {
                var prop = descs[pn];
                loopCount++;
                Object.defineProperty(i, pn, prop);
                var desc = Object.getOwnPropertyDescriptor(i, pn);
                verifyPropertyDescriptors(prop, desc, e);
                verifyProperties(i, pn, prop);
            }
        }
    })();

    intellisense.logMessage('made it past Object.create 1, loopCount = ' + loopCount);
    loopCount = 0;

    // Test for Object.create(o, props)
    (function () {
        for (var o in htmlElements) {
            var e = htmlElements[o];
            var i = Object.create(e, descs);

            for (var pn in descs) {
                loopCount++;
                var prop = descs[pn];
                var desc = Object.getOwnPropertyDescriptor(i, pn);
                verifyPropertyDescriptors(prop, desc, e);
                verifyProperties(i, pn, prop);
            }
        }
    })();

    intellisense.logMessage('made it past Object.create 2, loopCount = ' + loopCount);
    loopCount = 0;

    // Test for Object.create(o) and Object.defineProperties()
    (function () {
        for (var o in htmlElements) {
            var e = htmlElements[o];
            var i = Object.create(e);
            Object.defineProperties(i, descs);

            for (var pn in descs) {
                var prop = descs[pn];

                var desc = Object.getOwnPropertyDescriptor(i, pn);
                verifyPropertyDescriptors(prop, desc, e);
                verifyProperties(i, pn, prop);
            }
        }
    })();

    intellisense.logMessage('made it past Object.create 3');

    // Test for Object.defineProperties() on elements
    (function () {
        for (var e in htmlElements) {
            var o = htmlElements[e];
            Object.defineProperties(o, descs);

            for (var pn in descs) {
                var prop = descs[pn];

                var desc = Object.getOwnPropertyDescriptor(o, pn);
                verifyPropertyDescriptors(prop, desc, o);
                ver|verifyPropertyDescriptors|ifyProperties(o, pn, prop);
            }
        }
    })();
}
test();";
        #endregion

        //[TestMethod]
        //[WorkItem(204157)]
        //public void AfterLotsOfExceptions()
        //{
        //    PerformHurriedCompletionRequests("var count = 0; for (var i = 0; i < 10; i++) { for (var j = 0; j < i; j++) { for (var k = 0; k < j; k++) { for (var l = 0; l < i; l++) { for (c in d) { for (e in g) { z[i][j][k][l]++; count++; throw new Error(); } } } } } } var a = _$lg_limit > 1 ? 0 : ''; a.|split|", (completions, data, offset) =>
        //    {
        //        completions.ExpectContains(StringMethods);
        //    });
        //}

        [TestMethod]
        [WorkItem(224831)]
        public void DotAfterANumber()
        {
            PerformRequests("10.|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNull(completions);
            });
        }

        [TestMethod]
        [WorkItem(197513)]
        public void DomHTMLElementChild()
        {
            var HtmlElementCompletions = new[] { "appendChild", "attributes", "childNodes", "cloneNode", "firstChild", "hasChildNodes", "insertBefore", "lastChild", "nextSibling", "nodeName", "nodeType", "nodeValue", "ownerDocument", "parentNode", "previousSibling", "removeChild", "replaceChild" };

            PerformRequests(@"
                // document.body
                ;document.body.firstChild.|;
                ;document.body.lastChild.|;
                ;document.body.firstElementChild.|;
                ;document.body.lastElementChild.|;
                ;document.body.previousElementSibling.|;
                ;document.body.nextElementSibling.|;

                // document.getElementsByTagName
                ;document.getElementsByTagName('button')[0].firstChild.|;
                ;document.getElementsByTagName('p')[0].lastChild.|;
                ;document.getElementsByTagName('center')[0].firstElementChild.|;
                ;document.getElementsByTagName('body')[0].lastElementChild.|;
                ;document.getElementsByTagName('h1')[0].previousElementSibling.|;
                ;document.getElementsByTagName('img')[0].nextElementSibling.|;

                // document.createElement
                ;document.createElement('input').firstChild.|;
                ;document.createElement('font').lastChild.|;
                ;document.createElement('meta').firstElementChild.|;
                ;document.createElement('td').lastElementChild.|;
                ;document.createElement('ul').previousElementSibling.|;
            ", (context, offset, data, index) =>
             {
                 context.GetCompletionsAt(offset).ToEnumerable().ExpectContains(HtmlElementCompletions);
             }, false, Paths.DomWebPath);
        }

        [TestMethod]
        [WorkItem(245377)]
        public void CatchWithEval_a()
        {
            var primary = _session.FileFromText("try { } catch(e) { eval('e').");
            var context = _session.OpenContext(primary);
            var offset = primary.Text.Length;
            var completions = context.GetCompletionsAt(offset);
            Assert.IsNotNull(completions);
        }

        [TestMethod]
        [WorkItem(245377)]
        public void CatchWithEval_b()
        {
            var primary = _session.FileFromText("function a() { try { } catch(e) { eval('e').");
            var context = _session.OpenContext(primary);
            var offset = primary.Text.Length;
            var completions = context.GetCompletionsAt(offset);
            Assert.IsNotNull(completions);
        }

        [TestMethod]
        [WorkItem(319209)]
        public void CompletionsFromBuiltinsWithInvalidParameters()
        {
            PerformCompletionRequests(@"
                Object.create().|@Object|;
                Object.create(1).|@Object|;
                Object.create({}, null).|@Object|;
                Object.defineProperty().|@Object|;
                Object.defineProperties().|@Object|;
                Object.getOwnPropertyDescriptor().|value,writable,enumerable,configurable|;
                Object.keys().|Array|;
                Object.keys()[0].|String|;
                Object.getPrototypeOf().|@Object|;
                Object.freeze().|@Object|;
                Object.preventExtensions().|@Object|;
                Object.seal().|@Object|;
                Object.isSealed().|Boolean|;
                Object.isFrozen().|Boolean|;
                Object.isExtensible().|Boolean|;");
        }

        [TestMethod]
        [WorkItem(224472)]
        public void CompletionInAString()
        {
            Action<IAuthorTestContext, int, string, int> verify = (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsNull(completions);
            };
            PerformRequests("\"|", verify);
            PerformRequests("\"W|", verify);
            PerformRequests("\"Windows|", verify);
        }

        [TestMethod]
        [WorkItem(165848)]
        public void PrototypeJsExtendCompletion()
        {
            var ElementMethods = new[] { "absolutize", "ancestors", "addClassName", "adjacent", "clone", "childElements" };

            PerformRequests(@"
                var my_div = document.createElement('div');
                Element.extend(my_div);
                my_div.|;
                $('element').|;
                $('element').parentElement.|;
            ", (context, offset, data, index) =>
             {
                 context.GetCompletionsAt(offset).ToEnumerable().ExpectContains(ElementMethods);
             }, true, "!!" + Paths.DomWebPath, TestFiles.prototype);
        }

        [TestMethod]
        [WorkItem(245381)]
        public void DuplicateCaseDefaults()
        {
            var primary = _session.FileFromText("var a = 1; switch (a) { default: break; default: break; case 1: break; } a.");
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            Assert.IsNotNull(completions);
        }

        [TestMethod]
        [WorkItem(229194)]
        public void WWAInterfaceOnlyObjects()
        {
            PerformRequests(@"URL.|", AuthorHostType.ahtBrowser, (context, offset, data, index) =>
            {
                context.GetCompletionsAt(offset).ToEnumerable().ExpectContains(new[] { "revokeObjectURL", "createObjectURL" });
            }, false, Paths.DomWindowsPath);

            PerformRequests(@"MSApp.|", AuthorHostType.ahtBrowser, (context, offset, data, index) =>
            {
                context.GetCompletionsAt(offset).ToEnumerable().ExpectContains(new[] { "execUnsafeLocalFunction", "createBlobFromRandomAccessStream", "createStreamFromInputStream", "addPublicLocalApplicationUri" });
            }, false, Paths.DomWindowsPath);
        }

        [TestMethod]
        [WorkItem(245350)]
        public void Bug245350()
        {
            var primary = _session.FileFromText(Bug245350Text);
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(Bug245350Text.Length);
            Assert.IsNotNull(completions);
        }
        #region Test data
        const string Bug245350Text = @"/// <reference />
;
continue \u2e12;
with([]._Lf[this][v][\u5891]){$[.8E+0[5]][$Rx7]%={.5E+3:[],7.1E+5:[{},,,,{},,0[this[this[this]]].\u2e06Mc],}._e[$bC._]<=-(this[\u6dc9][""Ã¥Â¯â€”Ã«Â»Å¸ÃªÂ¿ÂºÃ¥Â¨Â´Ã¯ÂÂºÃ®Â·ÂºÃ¢â€ â€Ã¬â€ºÂ¶ÃªÂ¹Â­Ã®Â¡Å¸Ã¢Â½Â¥Ã¦Â¥Å“Ã¯â€Â¦Ã¢Â·Ëœ""])._b.$._()--;with(0x3.\u5441)try{break ;
}
catch(y){;
;
WK:{try{}
catch(U6CC){}
break nhb;
}";
        #endregion


        // There is a diagnostics mode Chakra bug this reproduces
        /*
        [TestMethod]
        [WorkItem(245387)]
        public void Bug245387()
        {
            var primary = _session.FileFromText(TestFiles.Bug245387);
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            Assert.IsNotNull(completions);
        }
        */

        [TestMethod]
        [WorkItem(234929)]
        public void CompletionInRedefinedFunction()
        {
            // Simple redefinition
            ValidateHasCompletions(@"
                    var x = 0; 
                    function foo() {x.|} 
                    function foo() {x.|} 
                    function foo() {x.|} 
                ", CompletionsBase.NumberMethods);

            // Redefinition with function expressions
            ValidateHasCompletions(@"
                    var x = 0; 
                    function foo() {x.|} 
                    (function foo() {x.|})()
                    function foo() {x.|} 
                    var bar = function foo() {x.|};
                ", CompletionsBase.NumberMethods);

            // Redefinition with function variable decaration
            ValidateHasCompletions(@"
                     var x = 0; 
                    function foo() {x.|} 
                    var foo = function () {};
                    function foo() {} 
                    var foo = function () {};
                ", CompletionsBase.NumberMethods);

            // Nested redefinitions
            ValidateHasCompletions(@"
                    var x = 0; 
                    function foo() {
                        x.|;
                        function foo() {x.|} 
                        function bar() {x.|} 
                        function foo() {
                            x.|;
                            function foo() {x.|} 
                            function bar() {x.|} 
                            function foo() {x.|} 
                        } 
                    }
                ", CompletionsBase.NumberMethods);
        }

        [TestMethod]
        [WorkItem(245384)]
        public void ScopeParsingErrorRecover()
        {
            var primary = _session.FileFromText("function aa::32() { } var a = 1; a.");
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            Assert.IsNotNull(completions);
        }

        [TestMethod]
        [WorkItem(234929)]
        public void CompletionInRedefinedAccessor()
        {
            // getters
            ValidateHasCompletions(@"
                    var x = 0; 
                    var o = {
                        get foo(){ x.|; },
                        get foo(){ x.|; },
                        get foo(){ x.|; }
                    };
                ", CompletionsBase.NumberMethods);

            // setters
            ValidateHasCompletions(@"
                    var x = 0; 
                    var o = {
                        set foo(){ x.|; },
                        set foo(){ x.|; },
                        set foo(){ x.|; }
                    };
                ", CompletionsBase.NumberMethods);


            // mixed
            ValidateHasCompletions(@"
                                var x = 0; 
                                var o = {
                                    foo : 0,
                                    get foo(){ x.|; },
                                    foo : 1,
                                    set foo(){ x.|; },
                                    foo : 2,
                                    get foo(){ x.|; },
                                    foo : 3,
                                    set foo(){ x.|; },
                                    foo : function () {x.|}
                                };
                            ", CompletionsBase.NumberMethods);
        }


        [TestMethod]
        public void BooleanLiterals()
        {
            ValidateHasCompletions("false.|", CompletionsBase.BooleanMethods);
            ValidateHasCompletions("true.|", CompletionsBase.BooleanMethods);
            ValidateHasCompletions("(false).|", CompletionsBase.BooleanMethods);
            ValidateHasCompletions("(true).|", CompletionsBase.BooleanMethods);
        }

        [TestMethod]
        public void ThisCompletions()
        {
            ValidateHasCompletions("this.|");
            ValidateHasCompletions("(this).|");
        }

        [TestMethod]
        [WorkItem(320310)]
        public void CompletionAfterSpuriousCloseBrace()
        {
            PerformCompletionRequests("if (true) var a = 1; }\n|Math|; a.|Number|");
        }

        [TestMethod]
        public void WindowEvents()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests("window.onload = function(e) { e.|initEvent| }", dom);
        }

        [TestMethod]
        public void GlobalObjectProperty()
        {
            PerformCompletionRequests(
                @"Object.defineProperty(this, 'P',  {
                    set: function(v) { this.p=v; },
                    get: function() { return this.p; }
                  });
                  this.P = 2;
                  this.|p|");

            PerformCompletionRequests(
                @"this.P = 2;
                  this.|p|",
                @"Object.defineProperty(this, 'P', {
                    set: function(v) { this.p=v; },
                    get: function() { return this.p; }
                  });");
        }

        [TestMethod]
        [WorkItem(281092)]
        public void DomListTypes()
        {
            PerformRequests(@"
                document.querySelectorAll('*').|NodeList|;
                document.getElementsByTagName('p').|NodeList|;
                document.body.children.|HTMLCollection|;
                document.createElement('input').files.|FileList|;
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset);
                     // Verify collection functions show in the list
                     completions.ToEnumerable().ExpectContains("length", "item");
                     // Verify that array functions do not show up
                     completions.ToEnumerable().ExpectNotContains("concat", "join", "pop", "push", "reverse", "shift", "slice", "sort", "splice", "unshift");
                 }, false, Paths.DomWebPath);
        }

        //[TestMethod]
        //public void InfiniteLoopInEval()
        //{
        //    PerformHurriedCompletionRequests("var test = 1; eval('while (true) {}'); test.|Number|");
        //}

        [TestMethod]
        [WorkItem(324805)]
        public void NegativeInfinity()
        {
            var primary = _session.FileFromText("function foo() { var a = -Infinity; a.; }");
            var offset = primary.OffsetAfter("a.");
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(offset);
            _session.Cleanup();
            var hint = completions.GetHintFor(0);
            Assert.IsNotNull(hint);
        }

        [TestMethod]
        public void IncrementExpression()
        {
            PerformCompletionRequests(@"var i = 1; (i++).|Number|;");
        }

        [TestMethod]
        public void VarCommentOnNullAssignedVariable()
        {
            PerformCompletionRequests(@"
            /// <var type='String'>
            var foo = null;
            foo.|String|;");
        }

        [TestMethod]
        public void SetTimeout()
        {
            PerformCompletionRequests(@"
                // eval expression
                setTimeout('x = { a:1 };', 0);
                x.|a|;

                // function call
                function foo() {
                  this.y = { b:1 };
                }
                setTimeout(foo, 0);
                y.|b|;
                ", _session.ReadFile(Paths.DomWebPath).Text);
        }

        [TestMethod]
        public void IdentifierListWithFilter()
        {
            var primary = _session.FileFromText("var a,b,c,d,e,f; function foo() { } function bar() { }");
            var contextFile = _session.FileFromText(@"
                VSIntellisenseExtensions.addCompletionHandler(function (ev) {
                    ev.items = ev.items.filter(function (item) {
                        return item.name != 'a';
                });");
            var context = _session.OpenContext(primary, contextFile);
            var identifiers = context.GetCompletionsAt(0, AuthorCompletionFlags.acfFileIdentifiersFilter);
            Assert.IsNotNull(identifiers);
            Assert.IsTrue(identifiers.ToEnumerable().All(c => c.Group == AuthorCompletionFlags.acfFileIdentifiersFilter));
        }

        [TestMethod]
        public void UsingTrackingNull()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            PerformCompletionRequests(@"
                ;document.parentNode.|body,head,title|;
                ;document.ownerDocument.|body,head,title|;
                ;document.nextSibling.nextSibling.|nodeType,nextSibling,nodeName|;
                ", dom);
        }

        [TestMethod]
        [WorkItem(320374)]
        public void CompletionInInfiniteLoop()
        {
            PerformCompletionRequests(@"
                function test (rule, someOtherParam) {
                    while (true) {
                        if (ru|rule,someOtherParam|le.isLegal()) {
                            break;
                        }
                    }
                }");
        }

        [TestMethod]
        [WorkItem(323512)]
        public void DedicatedWorkerSelf()
        {
            PerformCompletionRequests(@"self.|close,onerror|", _session.ReadFile(Paths.DedicatedWorkerPath).Text);
        }

        [TestMethod]
        [WorkItem(317326)]
        public void ArrayConcat()
        {
            PerformCompletionRequests("var x = [].concat(a); var result = x.length == 4 ? 'string' : 0; result.|String|", "var a = ['1', '2', '3', '4'];");
        }

        [TestMethod]
        [WorkItem(317326)]
        public void IsArray()
        {
            PerformCompletionRequests("function t(a) { return Array.isArray(a) ? 'array' : 0 }; var a = [1,2,3,4]; t(a).|String|;  t(b).|String|", "var b = [1,2,3,4]");
        }

        [TestMethod]
        [WorkItem(320456)]
        public void FieldCommentForANullLiteral()
        {
            PerformCompletionRequests(@"
                var foo = {
                    /// <field type=""String"" />
                    bar : null
                };
                foo.bar.|String|;
            ");
        }

        [TestMethod]
        [WorkItem(313316)]
        public void SortWithMissingElement()
        {
            var primary = _session.FileFromText("var arr = [0,1,,3,4,5]; var res = arr.sort(function(a,b) { return a - b; }); arr[2].");
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            Assert.IsNotNull(completions);
        }

        [TestMethod]
        [WorkItem(320143)]
        public void CallingACopyOnWriteBoundFunction()
        {
            PerformCompletionRequests("var f = bound(); f.|Number|; var j = f == 5 ? '' : 1; j.|String|;", "var o = { a: 1 }, a = { a: 2 }, b = { a: 2}; function foo(p1, p2) { return this.a + p1.a + p2.a; } bound = foo.bind(o, a, b);");
        }

        [TestMethod]
        [WorkItem(318039)]
        public void EnsureNoCompletionsInUnterminatedStrings()
        {
            var primary = _session.FileFromText("alert('j \n");
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(primary.Text.IndexOf("j "));
            Assert.IsNull(completions);
        }

        [TestMethod]
        [WorkItem(331157)]
        public void FuzzTesting1()
        {
            var primary = _session.FileFromText(FuzzTesting1Text);
            var context = _session.OpenContext(primary);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            Assert.IsNotNull(completions);
        }
        #region Test data
        const string FuzzTesting1Text = @"var trackStep = 10; var trackCnt=1; function print(s) {};
if(trackCnt++ % trackStep == 0) WScript.Echo(""0"");
try{eval("""");}catch(ex){}
try{eval(""\""use strict\"";  for  each(z in this.arguments) print(z);\nlet wgkcjh;print(\""\\u59\"");\n"");}catch(ex){}
try{eval(""M:for([a, c] = x in  /x/ ) print( /x/ );"");}catch(ex){}
try{eval(""throw undefined; '' ;"");}catch(ex){}
try{eval(""\""use strict\""; {\u0009print((let (y) undefined));print((/*wrap3*/(function(){ var vzhbqu = \""\\uE16452\""; (Proxy.create)(); })).call( /x/g , this));print((({w: 8589934592,  set z functional (-1057125028.0394777) { \""use strict\""; yield [1,,] }  }))); }"");}catch(ex){}
try{eval(""\""use strict\""; M:with(this){print(-2876244146.917278.prettyIndent);true; }\n/*bLoop*/for (jteqyu = 0; jteqyu < 17; ++jteqyu) { if (jteqyu % 9 == 4) { return (\""\\u34B5D\"" for (b in e)); } else { print(x); }  } \n"");}catch(ex){}
try{eval(""\""use strict\"";  /x/ ;#3={a:#3#};"");}catch(ex){}
try{eval(""(window);"");}catch(ex){}
try{eval(""let function::x = eval(\""(4277)\"");let x;gczeal(0);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""10"");
try{eval(""(new (773260861.7828547)(null, true));"");}catch(ex){}
try{eval(""/*tLoop*/for each (let e in [true, true, true, true, -3/0, true, -3/0, -3/0, -3/0, true, true, true, true, -3/0, true, -3/0, -3/0, -3/0, true, true, -3/0, true, true, true, true]) { print((yield  /* Comment */\""\\uCCA9\"")); }"");}catch(ex){}
try{eval(""\""use strict\""; print(x);print((eval(\""print(x);\"")));"");}catch(ex){}
try{eval(""/*oLoop*/for (nanktu = 0, [] =  \""\"" ; nanktu < 3; ++nanktu) { yield; } "");}catch(ex){}
try{eval(""/*vLoop*/for (let bfekgj = 0; bfekgj < 2; ++bfekgj) { var e = bfekgj; /*oLoop*/for (let edanez = 0, window; edanez < 7; ++edanez) { print(window); }  } "");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (var ratsfa = 0; ratsfa < 4; ++ratsfa) { [1];window;\nprint(#1=[#1#]);\n } "");}catch(ex){}
try{eval(""var window = [1,,].unwatch(\""b\""), z =  /x/ , fuijix, z =  /x/g , awrlcb, cwhkrf, d = x, \u3056;/*vLoop*/for (hlppcb = 0; hlppcb < 2; ++hlppcb) { y = hlppcb; print(false); } "");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""20"");
try{eval(""throw x;(window);const a = x.function::ignoreWhitespace;"");}catch(ex){}
try{eval(""let (x = , x = x, x, yaatgo) {  for  each(let x in eval) {print(x); } }"");}catch(ex){}
try{eval(""/*vLoop*/for (var sbrzkr = 0; sbrzkr < 11; ++sbrzkr) { let b = sbrzkr; print(this);(b); } "");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (zpwmis = 0; zpwmis < 21; ++zpwmis) { if (zpwmis % 9 == 3) { print( /x/g ); } else { default xml namespace  = x; }  } "");}catch(ex){}
try{eval(""\""use strict\""; print([] = Math.max(\""\\u8BDA0\"" ?  /x/g  : -2281895221, (4277)));a = new Exception() |= x;"");}catch(ex){}
try{eval(""let(a, x = e, z, e) ((function(){try { ; } finally { return \""\\uC86AF3\""; } })());"");}catch(ex){}
try{eval(""print(\""\\uD4E3\"");(arguments);print(x);"");}catch(ex){}
try{eval(""\""use strict\""; function shapeyConstructor(gzywyq){if (gzywyq) this.x =  \""\"" ;delete this.x;delete this.x;Object.preventExtensions(this);if (gzywyq) this.x = Array.isArray;this.x = ( /* Comment */false);if (gzywyq) for (var ytqwketup in this) { }if (gzywyq) Object.seal(this);return this; }/*tLoopC*/for each (let w in [ /x/g , x, new Boolean(true), (-1/0),  /x/g , new Boolean(true), new Boolean(true),  /x/g , (-1/0),  /x/g , x,  /x/g ,  /x/g , x, new Boolean(true), x, new Boolean(true), x, new Boolean(true), (-1/0), new Boolean(true),  /x/g ,  /x/g ,  /x/g , new Boolean(true), (-1/0), new Boolean(true),  /x/g ,  /x/g , x, x,  /x/g ]) { try{let hcvpbx = shapeyConstructor(w); print('EETT'); ({});}catch(e){print('TTEE ' + e); } }x;"");}catch(ex){}
try{eval(""( /x/g ); \""\"" ;"");}catch(ex){}
try{eval(""\""use strict\""; let (b) { print(b); }"");}catch(ex){}
try{eval(""\""use strict\""; yield;print(({ get c x ()new (undefined)(\""\\u77A28\"",  /x/g ) }));"");}catch(ex){}
try{eval(""\""use strict\""; if(x) {print(x); } else  if ((p={}, (p.z =  \""\"" )()))  for  each(var d in  /x/ ) (#3={a:#3#});"");}catch(ex){}
try{eval("";"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; return y;print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""40"");
try{eval(""\""use strict\""; /*bLoop*/for (xfjrdl = 0; xfjrdl < 19; ++xfjrdl) { if (xfjrdl % 4 == 2) { print((w.callee = Object.keys)); } else { (eval = \""\\uE\""); }  } "");}catch(ex){}
try{eval(""\""use strict\""; \""use strict\""; print((new Object.isFrozen((x) =  /x/ .watch(\""\\u3056\"", Object.isFrozen), x)));throw x;(w) = c;"");}catch(ex){}
try{eval(""(x);"");}catch(ex){}
try{eval(""switch(\""\\uE243F3\"") { default: break; var \u3056 = 4035918098;print( /x/g );break; case 6: print((x = (function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: undefined, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: undefined, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: undefined, set: Object.getPrototypeOf, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: decodeURIComponent, }; })((Math.abs(0)))));break; break; case 7: break; print(x);break;  }"");}catch(ex){}
try{eval(""with({}) { with({}) { this.zzz.zzz; }  } "");}catch(ex){}
try{eval(""window = linkedList(window, 240);"");}catch(ex){}
try{eval(""var x, x, z, hkcpln, tdtbyb, ctxgav, \u3056;(\u3056);let x =  \""\"" ;"");}catch(ex){}
try{eval(""/*tLoop*/for each (let c in [undefined, x, .2, undefined, .2, undefined, [], x, [], [], .2, [], x, [], [], .2, undefined, [], undefined, x, x, [], undefined, x]) { return  /x/g /*\n*/; }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""50"");
try{eval(""\""use strict\""; {if(x) print(x); }"");}catch(ex){}
try{eval(""while(([[1]]) && 0)print( /x/g );"");}catch(ex){}
try{eval(""x;let(x = print(x);, bpvpol, xlexwy, functional, x, z, kpjngk, abceky) ((function(){with({}) print(\""\\u14\"");})());\u000c"");}catch(ex){}
try{eval(""\""use strict\""; let(z = \""\\uC4\"", x, guvfxa, dvmbll) ((function(){print(x);})());"");}catch(ex){}
try{eval(""\""use strict\""; var d = undefined;this;const y = (gc((function ([y]) { })(), this)) ? ([].hasOwnProperty(false)) : (4277)++;"");}catch(ex){}
try{eval(""yield;\nprint(function ([y]) { }.unwatch(\""x\""));\n"");}catch(ex){}
try{eval(""/*tLoop*/for each (let z in [new Number(1.5), function(){}, (void 0), function(){}, function(){}]) { print( '' ); }"");}catch(ex){}
try{eval(""if( \""\"" ) { if (true) {break ; }} else {{} }function y(){/*jjj*/}yield;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""60"");
try{eval(""print(x);"");}catch(ex){}
try{eval(""/*bLoop*/for (xogias = 0; xogias < 8; ++xogias) { if (xogias % 10 == 3) { yield  /x/ ; } else { (\""\\u9B\""); }  } "");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""/*iii*/print(x);/*hhh*/function zovsal(){print(b);}"");}catch(ex){}
try{eval(""/*iii*//*vLoop*/for (yviahw = 0; yviahw < 6; ++yviahw) { c = yviahw; print(\u3056); } /*hhh*/function gbjkmk(){((4277));}"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""/*vLoop*/for (var zyzfrr = 0; zyzfrr < 12; ++zyzfrr) { let z = zyzfrr; /*tLoop*/for each (let a in [ /x/ ,  /x/ ,  /x/ , function(){}, x, x, x, (-1/0),  /x/ , function(){}, (-1/0), (-1/0), function(){},  /x/ , x, x]) { print(null); } } "");}catch(ex){}
try{eval(""const b = ( /* Comment */ \""\"" );b;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""70"");
try{eval(""\""use strict\""; /*bLoop*/for (let vjvlnn = 0; vjvlnn < 16; ++vjvlnn) { if (vjvlnn % 5 == 3) { print(x); } else { (window); }  } continue L;"");}catch(ex){}
try{eval(""{/*tLoop*/for each (let z in [ /x/g , new Boolean(false), new Boolean(true), new Boolean(true), new Boolean(true), function(){},  /x/g , (-1/0), new Boolean(true), (-1/0), function(){}, new Boolean(true), (-1/0), (-1/0), function(){},  /x/g ,  /x/g ,  /x/g , function(){}, function(){}, function(){}, (-1/0), new Boolean(false),  /x/g , new Boolean(false), new Boolean(false),  /x/g , new Boolean(false),  /x/g , new Boolean(true), function(){},  /x/g , function(){}, new Boolean(true),  /x/g ]) { print(x); } }yield;"");}catch(ex){}
try{eval(""/*tLoop*/for each (let c in [true, 1e+81,  \""\"" , 1e+81, true, true, true, new Number(1.5), 1e+81, 1e+81]) { print( \""\"" ); }"");}catch(ex){}
try{eval(""var dofxac = new ArrayBuffer(8); var dofxac_0 = new WebGLIntArray(dofxac); dofxac_0[0] = 0.7066068660203488; (\""\\uFC\"");"");}catch(ex){}
try{eval(""print(x);function \u3056(\u3056){/*jjj*/}let (b) { print(Object.seal.prototype); }"");}catch(ex){}
try{eval(""var uzaevj, b = this, kyjnfj, x, y, d, daazfy;M:while((window) && 0)continue ;"");}catch(ex){}
try{eval(""if(eval(\""{}\"")) print(x);\n(NaN.parent.__proto__);let (e) { print(x); }\n"");}catch(ex){}
try{eval(""/*iii*/print(nvbdgl);/*hhh*/function nvbdgl(){/*vLoop*/for (iqbphe = 0; iqbphe < 5; ++iqbphe) { b = iqbphe; {} } }"");}catch(ex){}
try{eval(""{const bjhsns, qyiuav, ocrchu, adfxnb, x, bcwakp, dwxqfm;print(x); }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""80"");
try{eval(""/*oLoop*/for (ixxtod = 0; \""\\u53\"" && ixxtod < 1; ++ixxtod) { print(x); } "");}catch(ex){}
try{eval(""/*tLoop*/for each (let z in [({}), ({}), ({}), -Infinity, -Infinity, ({}), [undefined], [undefined], -Infinity, [undefined], [undefined], [undefined], -Infinity, -Infinity, -Infinity, -Infinity, ({}), ({}), ({}), ({}), [undefined], ({}), [undefined], -Infinity, ({}), [undefined], -Infinity, [undefined], [undefined], -Infinity, -Infinity, -Infinity, -Infinity, ({}), [undefined], [undefined], -Infinity, -Infinity]) { ( '' );\nvar w = [,,z1];\n }"");}catch(ex){}
try{eval(""( '' );"");}catch(ex){}
try{eval(""((null));function functional(x, functional){/*jjj*/}print((x = this.x));"");}catch(ex){}
try{eval(""print(undefined);var y = -709210212.0141351;"");}catch(ex){}
try{eval(""M:if(33554431) print( /x/ ); else print(x);"");}catch(ex){}
try{eval(""/*oLoop*/for (afbzcx = 0; afbzcx < 10; ++afbzcx) { print(x); } "");}catch(ex){}
try{eval(""/*oLoop*/for (wbggli = 0; wbggli < 9; ++wbggli) { print((4277)); } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""90"");
try{eval(""print(new RegExp( /x/ ));function x(\u3056, x){/*jjj*/}yield;"");}catch(ex){}
try{eval(""for(var c = let (x) 274877906945 in ((function a_indexing(xhjnjf, azsbxb) { ; if (xhjnjf.length == azsbxb) { ; return ~Math.min(-3782344242, 33554431); } var pgmyxp = xhjnjf[azsbxb]; var sfrqkx = a_indexing(xhjnjf, azsbxb + 1); return let (e) (this.__defineSetter__(\""z\"", Object.isExtensible)); })([(1/0), function(){}, function(){}, (1/0), (1/0), (1/0), (1/0), (1/0), function(){}, function(){}], 0)).arguments = JSON.parse) {\""\\u646760\"";let (c = eval) { print(x); } }"");}catch(ex){}
try{eval(""\""use strict\""; var mcsrqe = new ArrayBuffer(4); var mcsrqe_0 = new Int8Array(mcsrqe); print(mcsrqe_0[0]); var mcsrqe_1 = new Int8Array(mcsrqe); mcsrqe_1[0] = -0; for(var [b, c] = this in window) { /x/ ; }gczeal(0);print(mcsrqe_1[0]);( '' );"");}catch(ex){}
try{eval(""L:if(([15,16,17,18].sort(ArrayBuffer, this))) break ; else  if (x) {ArrayBufferbreak ; } else print(x);"");}catch(ex){}
try{eval(""print(new XPCNativeWrapper(4611686018427388000));"");}catch(ex){}
try{eval(""\""use strict\""; new XPCNativeWrapper( '' );\nreturn;;\n"");}catch(ex){}
try{eval(""{throw Boolean(\""\\u2\"");print(\""\\u92C0F5\"");print(x); }"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let y in [ '' , eval([z1,,])(Math.max(-0, -0)).descendants--, null, Infinity, null, null,  '' , null, Infinity, Infinity]) { (\""\\uCB90\"");0.15594280403547678; }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""100"");
try{eval(""let wjzwnb, udgroo, iwojsy, lecymk;print(\""\\u08F020\"");\nprint(x);\n"");}catch(ex){}
try{eval(""\""use strict\"";  for  each(var w in new Array(-562949953421313)) {//h\nprint(x);oqmfva;print(window); }function x(){/*jjj*/}return;\nthrow window;\n"");}catch(ex){}
try{eval(""\""use strict\""; x, irmbma, yyvmyz, c, mqudsj, x, lqmsov;\""\\u1E1\"";"");}catch(ex){}
try{eval(""\""use strict\""; (x);"");}catch(ex){}
try{eval(""continue ;\nprint([[1]]);\nyield x;"");}catch(ex){}
try{eval(""var awtidq = new ArrayBuffer(8); var awtidq_0 = new Int16Array(awtidq); return ({awtidq getter: (1 for (x in [])),  set eval()awtidq_0 });(awtidq_0[0]);"");}catch(ex){}
try{eval(""/*tLoop*/for each (let a in [undefined, new Boolean(false), arguments.callee, new Boolean(false), new Boolean(false), arguments.callee, undefined, new Boolean(false), undefined, new Boolean(false), new Boolean(false), undefined, new Boolean(false), arguments.callee, arguments.callee, undefined, undefined, undefined, arguments.callee, undefined]) { gc()\n(0);\n }"");}catch(ex){}
try{eval(""/*bLoop*/for (adjjto = 0, x &= b; adjjto < 0; ++adjjto) { if (adjjto % 6 == 3) { yield  /x/g ;function x(\u3056, x){/*jjj*/}print(y); } else { yield; }  } "");}catch(ex){}
try{eval(""\""use strict\""; ;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""110"");
try{eval(""\""use strict\""; /*oLoop*/for (var kgzbdj = 0; kgzbdj < 12; ++kgzbdj) { delete \""\\uAD1\""; } "");}catch(ex){}
try{eval(""\""use strict\""; qgkdve([[]]);/*hhh*/function qgkdve(functional, y){window;}"");}catch(ex){}
try{eval(""default xml namespace  = (function(y) { return true }).call(131073,  \""\"" , true);L:while((window.yoyo( /x/g )) && 0){print((4277).function::ignoreProcessingInstructions);false; }"");}catch(ex){}
try{eval(""/*bLoop*/for (let iltcff = 0; iltcff < 6; ++iltcff) { if (iltcff % 6 == 3) { print(x); } else { x = linkedList(x, 2303); }  } "");}catch(ex){}
try{eval(""while(([(function(id) { return id }.prototype)/*\n*/ for each (b in [{}, {}, false, {}, {}, false, {}, {}, {}, false, {}, {}, false, {}, {}, {}, false, {}, false, false, {}, {}, {}, {}, false, {}, false, {}, {}, false, {}, {}, {}, false, false, false, false, false, {}]) for each (x in  /x/g )]) && 0)let (w) { print((\""\\u2E0\""( \""\"" )) ? (w < (({functional:  /x/g  }))) : w =  /x/g ); }"");}catch(ex){}
try{eval(""x = x;break L;"");}catch(ex){}
try{eval(""print(\""\\uBE482\"");var a = [,];"");}catch(ex){}
try{eval(""with(-9223372036854776000)( \""\"" );\nbreak M;\n"");}catch(ex){}
try{eval(""let (x) { null;x; }"");}catch(ex){}
try{eval(""this.zzz.zzz;let(y) { (undefined);}"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""120"");
try{eval(""( \""\"" );\n\""\\u72\"";\n"");}catch(ex){}
try{eval(""switch(1023) { case x: true;throw \""\\u137\""; }"");}catch(ex){}
try{eval(""\""use strict\""; x;"");}catch(ex){}
try{eval(""( \""\"" );"");}catch(ex){}
try{eval(""\""use strict\""; /*hhh*/function mornjp(b, functional){ '' ;}mornjp([,,]);const b = x['_' + (\n\""\\u15A\"")];"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let d in [NaN, (0/0), NaN, new Number(1.5), (0/0), function(){}, (0/0), (0/0), new Number(1.5), new Number(1.5), new Number(1.5)]) { /*hhh*/function fyxodr(){print(x);}fyxodr(x in  \""\"" , 'fafafa'.replace(/a/g, /*wrap1*/(function(){ {}return function(y) { return (function ([y]) { })() }})()))\307\321\552\684\262\527\523\771\548\628\134\880\042\524\021\154\051\836\842\667\183\354\036\147\480\774\848\355\710\603\736\433\166\602\670\234\616\662l)));"");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (let fjlbxy = 0; fjlbxy < 0; ++fjlbxy) { if (fjlbxy % 2 == 0) { print(x); } else { /*tLoop*/for each (let y in [z / x]) { print( \""\"" ); } }  } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""130"");
try{eval(""\""use strict\""; for(let w in ((x) = true)) {var gdgrdd = new ArrayBuffer(0); var gdgrdd_0 = new Uint16Array(gdgrdd); print(gdgrdd_0[0]); gdgrdd_0[0] = 0; \u000c(undefined); }"");}catch(ex){}
try{eval(""\""use strict\""; with({}) return  \""\"" ;"");}catch(ex){}
try{eval(""\""use strict\""; var vyuzbu = new ArrayBuffer(4); var vyuzbu_0 = new Int8Array(vyuzbu); ( \""\"" );"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; options('strict');"");}catch(ex){}
try{eval(""/*tLoop*/for each (let x in [(-1), null, (-1), null]) { ; }"");}catch(ex){}
try{eval(""\""use strict\""; if( \""\"" ) {print(x);print(x); }\u000c else syvtwq(gczeal(0));/*hhh*/function syvtwq(NaN){yield [z1,,];}"");}catch(ex){}
try{eval(""\""use strict\""; yield function ([y]) { };"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""return [15,16,17,18].map(function (a) { {} } , x);(w.function::y = Object.getOwnPropertyNames);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""140"");
try{eval(""\""use strict\""; if(x) const a =  \""\"" ;print(x); else {print(undefined); }"");}catch(ex){}
try{eval(""this.zzz.zzz;return \nthis;"");}catch(ex){}
try{eval(""/*vLoop*/for (let kihfpq = 0; kihfpq < 7; ++kihfpq) { let b = kihfpq; L: return -2627426677; } "");}catch(ex){}
try{eval("" for  each(var w in x) ;"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let a in [null, null]) { print(c); }"");}catch(ex){}
try{eval("" '' ;"");}catch(ex){}
try{eval(""/*tLoop*/for each (let z in [new String('q'), new String('q'), new String('q'), new String('q'), #1=[#1#].watch(\""eval\"", this),  '\\0' , #1=[#1#].watch(\""eval\"", this), #1=[#1#].watch(\""eval\"", this), new String('q'), #1=[#1#].watch(\""eval\"", this), new String(''), new String(''), new String(''), new String('q'), new String(''),  '\\0' , new String('q'), new String(''),  '\\0' , new String('q'), #1=[#1#].watch(\""eval\"", this), new String(''),  '\\0' , #1=[#1#].watch(\""eval\"", this), #1=[#1#].watch(\""eval\"", this),  '\\0' , new String(''),  '\\0' ,  '\\0' ,  '\\0' , new String('q')]) { gczeal(0);(Math); }"");}catch(ex){}
try{eval(""\""use strict\""; /*vLoop*/for (let qrqyfm = 0; qrqyfm < 12; ++qrqyfm) { var w = qrqyfm; ( /x/g ); } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""150"");
try{eval(""with({}) throw StopIteration;"");}catch(ex){}
try{eval(""\""use strict\""; w = new Array(-2973443065);break L;var e = x;"");}catch(ex){}
try{eval(""/*vLoop*/for (let bjpibr = 0; bjpibr < 8; ++bjpibr) { var w = bjpibr; 0;function \u3056({}, []){/*jjj*/}print(\""\\u688\""); } "");}catch(ex){}
try{eval(""/*vLoop*/for (ujazek = 0, x = (#0=[(4277)]), -0; ujazek < 7; ++ujazek) { b = ujazek; eval = linkedList(eval, 7832); } "");}catch(ex){}
try{eval(""switch(({\u3056: (this.watch(\""c\"", Object.getPrototypeOf)), x setter: Object.seal })) { default: L:if((4277)) { if ((4277)) {false; }} else {this; }case x: break; print(x);break;  }"");}catch(ex){}
try{eval(""let(\u3056, x = (let (e=eval) e), x = x, window, functional = x, sdunks, lwhpno, qikrgf) { this.zzz.zzz;}"");}catch(ex){}
try{eval(""y = a;let(d) ((function(){let(NaN) ((function(){yield e += x;})());})());"");}catch(ex){}
try{eval(""/*bLoop*/for (var gwltca = 0, e; gwltca < 1; \""\\u25E15\"", ++gwltca) { if (gwltca % 5 == 2) { ; } else { window; }  } \n(window);yield;\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""160"");
try{eval(""gczeal(0);\nreturn  /x/ ;\nvar liyxee = new ArrayBuffer(12); var liyxee_0 = new Uint32Array(liyxee); liyxee_0[0] = 1e81; var liyxee_1 = new Uint16Array(liyxee); liyxee_1[0] = x; var liyxee_2 = new Uint8Array(liyxee); liyxee_2[0] = -3516735290.282424; print(liyxee_2);throw \""\\uC3C9\"";L:with({b: length})print(liyxee_0[0]);w;"");}catch(ex){}
try{eval(""\""use strict\""; print(x);let w = ((this)( /x/g ) = x);"");}catch(ex){}
try{eval(""let e, x, e, x = \""\\uDBD5CE\"", x, wdayaq;print(x);"");}catch(ex){}
try{eval(""if((namespace = -0)) { if (b) /*oLoop*/for (fznlcj = 0; fznlcj < 0; ++fznlcj) { (x); }  else continue L;}return \""\\uFD8AD8\"";print(\""\\u69AC0\"");"");}catch(ex){}
try{eval(""( '' );"");}catch(ex){}
try{eval(""const x = (4277);print((4277));"");}catch(ex){}
try{eval(""+="");}catch(ex){}
try{eval(""/*oLoop*/for (vogiqm = 0; vogiqm < 12; ++vogiqm) { yield  '' ; } "");}catch(ex){}
try{eval(""/*tLoop*/for each (let a in [ '' , new String('q'), new String('q'), x, false, x, new String('q'),  '' , x, false, new String('q'), x, false,  '' , x, new String('q'), x,  '' , new String('q'), x, false, false, x,  '' , false, false, false, false, false,  '' ,  '' , false, new String('q'), false,  '' , x, new String('q'),  '' ]) { let (w = eval(\""functional\""), eval, iuhlgw, fzlwag, a, d, kqgrrk, NaN, d) a.throw(new XPCSafeJSObjectWrapper(-4218312487, this)); }"");}catch(ex){}
try{eval(""for(let z = (4277) in  \""\"" ) {var hycddq, dsvaik, bgcgdx, d, functional,  /x/ , functional, z, d, c;print(\""\\uDBC48\"");\""\\uA\""; }"");}catch(ex){}
try{eval(""let fddpfx, x = \n \""\"" , eval = (yield \""\\u66AE61\"");/*bLoop*/for (cdlrmq = 0; cdlrmq < 17; ++cdlrmq) { if (cdlrmq % 7 == 2) { gczeal(0); } else { d;print(\""\\u2\""); }  } "");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let e in [ /x/ , x,  /x/ , x, Infinity, Infinity, x, function(){}, x,  /x/ , x, Infinity, Infinity, x, Infinity, x, x, function(){}, Infinity,  /x/ , Infinity]) { yield;; }"");}catch(ex){}
try{eval(""\""use strict\""; gndifq();/*hhh*/function gndifq(){\""\\uCEC\"";}"");}catch(ex){}
try{eval(""print(new ((function factorial_tail(udrgid, krjcqi) { -281474976710656;; if (udrgid == 0) { ; return krjcqi; } ; return factorial_tail(udrgid - 1, krjcqi * udrgid); (undefined); })(9, 1))((4277)));"");}catch(ex){}
try{eval(""\""use strict\""; /*hhh*/function mfxwsg(){(null);}/*iii*/this;"");}catch(ex){}
try{eval(""L: print(x >>> x);\nprint(x)\n\n"");}catch(ex){}
try{eval(""let x = (a) =  \""\"" ; for  each(d in  '' ) {print(x); }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""180"");
try{eval(""do print(void ((uneval(({a2:z2}))))); while(((4277)/*\n*/) && 0);"");}catch(ex){}
try{eval(""/*tLoop*/for each (let d in [ \""\"" , 0x3FFFFFFE,  \""\"" ,  \""\"" ,  \""\"" ,  \""\"" , 0x3FFFFFFE]) { yield function ([y]) { }; }"");}catch(ex){}
try{eval(""M:if((e = Proxy.create((function handlerFactory() {return {getOwnPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor'); var desc = Object.getOwnPropertyDescriptor(x); desc.configurable = true; return desc; }, getPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor'); var desc = Object.getPropertyDescriptor(x); desc.configurable = true; return desc; }, defineProperty: undefined, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return Object.getOwnPropertyNames(x); }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); if (Object.isFrozen(x)) { return Object.getOwnProperties(x); } }, has: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return name in x; }, hasOwn: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return Object.prototype.hasOwnProperty.call(x, name); }, get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: function(receiver, name, val) { var yum = 'PCAL'; dumpln(yum + 'LED: set'); x[name] = val; return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { for (var name in x) { yield name; } })(); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); var result = []; for (var name in x) { result.push(name); }; return result; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return Object.keys(x); }, }; })(\""\\u27\""),  \""\"" ))) {wrapprint(eval); } else prettyPrinting\nprint(w % NaN);\n"");}catch(ex){}
try{eval(""/*bLoop*/for (eocpej = 0; eocpej < 7; ++eocpej) { if (eocpej % 3 == 1) { print(window); } else { print([[]]);yield; }  } "");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (upkjwl = 0; ({a2:z2}) && upkjwl < 4; ++upkjwl) { print(x); } "");}catch(ex){}
try{eval(""\""use strict\""; /*iii*/print(x);/*hhh*/function tfrsvf([]){print(new XPCNativeWrapper(eval(\""true;\"").function::y\u0009 = x.\u3056 getter= (new Function(\""print( /x/g );\""))));}"");}catch(ex){}
try{eval(""\""use strict\""; print(x);\nprint(x);\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""190"");
try{eval(""print(this)\nprint(x);\u0009{}"");}catch(ex){}
try{eval(""print(x);function x(NaN){/*jjj*/};function a(e){/*jjj*/}/*bLoop*/for (xweqvq = 0; xweqvq < 23; ++xweqvq) { if (xweqvq % 10 == 8) { print(x); } else { ( '' ); }  } "");}catch(ex){}
try{eval(""print(gczeal(0));function z(){/*jjj*/}/*hhh*/function pqvsic(){print(true);}pqvsic(b, [[1]]);"");}catch(ex){}
try{eval(""(#3={a:#3#}\n);function a(){/*jjj*/}print(x);"");}catch(ex){}
try{eval(""/*oLoop*/for (var roxghm = 0; roxghm < 11; ++roxghm) { /*oLoop*/for (let xtbqva = 0; x && xtbqva < 12; ++xtbqva) { print(\""\\u6A\""); }  } "");}catch(ex){}
try{eval(""const c = x;c = true;(null);"");}catch(ex){}
try{eval(""if(function(y) { yield y; print(x);; yield y; }.prototype) {\u3056++; } else  if ((window.__defineGetter__(\""d\"", decodeURIComponent) || [[]]).__count__ = \""\\uD8\"") heeuug(undefined, [z1]);/*hhh*/function heeuug(c){print([z1]);} else qtxhca();/*hhh*/function qtxhca(){print(true);}"");}catch(ex){}
try{eval(""for(var x in ((Int32Array)(Error(z, Math))))(x);function c(c){/*jjj*/}/*iii*/print(((4277).krohzt::isPrototypeOf)--);/*hhh*/function krohzt({x: x}){b;print(712327205);}"");}catch(ex){}
try{eval(""egbodg((w.__iterator__ = 16384));/*hhh*/function egbodg(){ for  each(w in -1605003405.4669153) w;}"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""200"");
try{eval(""/*bLoop*/for (var opylxf = 0; opylxf < 18; ++opylxf) { if (opylxf % 8 == 7) { print(x); } else { ibfjlk( /x/g );/*hhh*/function ibfjlk(){return;} }  } "");}catch(ex){}
try{eval(""for(let w = null.x in window) \""\\u4\"";"");}catch(ex){}
try{eval(""/*tLoop*/for each (let d in [function(){}, [1], [1], function(){}, function(){}, function(){}, function(){}, function(){}, [1], function(){}, [1], function(){}, [1], [1]]) { continue M; }"");}catch(ex){}
try{eval(""\""use strict\""; let(z) ((function(){let(d, b = z, z = window, lxffxa, eval) ((function(){try { let(a) { window;} } catch(z) { (NaN) = z; } })());})());"");}catch(ex){}
try{eval(""/*tLoop*/for each (let x in [new Boolean(true), 1e4, 1e4, 0x40000000, 1e4, 0x40000000, 1e4, new Boolean(true), 0x40000000, new Boolean(true), new Boolean(false), (1/0), (1/0), new Boolean(false), new Boolean(true), new Boolean(true), (1/0), (1/0), new Boolean(true), new Boolean(true), (1/0)]) {  /x/ ; }"");}catch(ex){}
try{eval(""\""use strict\""; var esgnym = new ArrayBuffer(12); var esgnym_0 = new Uint8Array(esgnym); esgnym_0[0] = -0.6103812715083661; var esgnym_1 = new Int32Array(esgnym); print(esgnym_1[0]); ( /x/g );{}yield;"");}catch(ex){}
try{eval(""let (c) { var pdsvlw = new ArrayBuffer(6); var pdsvlw_0 = new WebGLFloatArray(pdsvlw); print(pdsvlw_0[0]); pdsvlw_0[0] = this; {} }"");}catch(ex){}
try{eval(""/*bLoop*/for (elgbyk = 0; elgbyk < 19; ++elgbyk) { if (elgbyk % 6 == 5) { var azagtp = new ArrayBuffer(0); var azagtp_0 = new Uint32Array(azagtp); var azagtp_1 = new Int32Array(azagtp); print(azagtp_1[0]); azagtp_1[0] = 0.5269503106622084; (azagtp_1); '' ; } else { let (d) { (\""\\u9E7\""); } }  } "");}catch(ex){}
try{eval("" /x/ "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""210"");
try{eval(""\""use strict\""; /*oLoop*/for (var neyskk = 0, ({x: eval = false }), ((4277) , x-= /x/ ); neyskk < 10; null.throw(x), ++neyskk) { gc() } "");}catch(ex){}
try{eval(""/*tLoop*/for each (let a in [({}), ({}),  /x/g , (void 0), ({}),  /x/g , ({}), (void 0),  /x/g ,  /x/g , (void 0), (void 0), (void 0),  /x/g , ({}),  /x/g , ({}),  /x/g , (void 0), ({}),  /x/g , (void 0), (void 0), (void 0),  /x/g , ({}),  /x/g , ({}),  /x/g , ({}),  /x/g , (void 0)]) { /*bLoop*/for (let iwrtze = 0; iwrtze < 4; ++iwrtze) { if (iwrtze % 5 == 4) { print(x); } else { print(({ get window(x, functional) { \""use strict\""; yield new QName() }  })); }  }  }"");}catch(ex){}
try{eval(""(NaN.function::functional);"");}catch(ex){}
try{eval(""/*hhh*/function owqqdu(){window;}/*iii*/(-524288);"");}catch(ex){}
try{eval(""\""use strict\""; print(x);print( /x/g ['_' + (\u3056)]);"");}catch(ex){}
try{eval(""continue ;var b = Object(window);"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""220"");
try{eval(""/*oLoop*/for (let ulcycn = 0; ulcycn < 7; ++ulcycn) { \u000cif((4277)) ; else  if (x) (({})); else print( /x/g ); } "");}catch(ex){}
try{eval(""(x);"");}catch(ex){}
try{eval(""var llmzhw = new ArrayBuffer(0); var llmzhw_0 = new Int8Array(llmzhw); print(llmzhw_0[0]); var llmzhw_1 = new Uint8Array(llmzhw); print(llmzhw_1[0]); llmzhw_1[0] = -0; var llmzhw_2 = new Uint32Array(llmzhw); llmzhw_2[0] = 0/0; var llmzhw_3 = new Int8Array(llmzhw); print(llmzhw_3[0]); var llmzhw_4 = new Int32Array(llmzhw); llmzhw_4[0] = 4.; (eval(\""4194303\"", y));print(e.length = (delete z.window));((((Object.isExtensible).bind()).bind(this.llmzhw_4, \""\\u75A6\"")).bind(d, window)(undefined, null));"");}catch(ex){}
try{eval(""var jymdsk = new ArrayBuffer(12); var jymdsk_0 = new Uint32Array(jymdsk); print(jymdsk_0[0]); jymdsk_0[0] = 1287768563.7700474; (\""\\u9F5D\"");function e(){/*jjj*/}255\n"");}catch(ex){}
try{eval(""print( '' );function y(){/*jjj*/}print( '' );"");}catch(ex){}
try{eval(""\""use strict\""; (\""\\uC5088A\""); \""\"" ;var x = ((function sum_indexing(gyauup, uexhyx) { ; return gyauup.length == uexhyx ? 0 : gyauup[uexhyx] + sum_indexing(gyauup, uexhyx + 1); })([(void 0), (void 0), (void 0), (void 0), {}, (void 0), (void 0), {}, {}, {}, {}, {}, {}, (void 0), (void 0), (void 0), (void 0), {}, (void 0), (void 0)], 0));"");}catch(ex){}
try{eval(""x;"");}catch(ex){}
try{eval(""var mtigzm, NaN = (yield [z1,,]), function::window =  '' , z, lisjzi, pyejfm, hlvtef, this.y, eudqbq;print(x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor'); var desc = Object.getOwnPropertyDescriptor(x); desc.configurable = true; return desc; }, getPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor'); var desc = Object.getPropertyDescriptor(x); desc.configurable = true; return desc; }, defineProperty: function(name, desc) { var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty'); Object.defineProperty(x, name, desc); }, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return Object.getOwnPropertyNames(x); }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); if (Object.isFrozen(x)) { return Object.getOwnProperties(x); } }, has: undefined, hasOwn: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return Object.prototype.hasOwnProperty.call(x, name); }, get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: undefined, iterate: undefined, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); var result = []; for (var name in x) { result.push(name); }; return result; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return Object.keys(x); }, }; })( /x/g ), function(q) { return q; }, null));"");}catch(ex){}
try{eval(""eval.window::eval;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""230"");
try{eval(""Math.pow( '' , window);\""\\u9B0F\"";"");}catch(ex){}
try{eval(""print((new (x)(+[])))\n(({\u3056:  \""\""  }).getPrototypeOf( \""\"" , \""\\u32\""));"");}catch(ex){}
try{eval(""let (e) { print(x); }"");}catch(ex){}
try{eval(""\""use strict\""; var jhbuzy = new ArrayBuffer(2); var jhbuzy_0 = new Int16Array(jhbuzy); print(jhbuzy_0[0]); jhbuzy_0[0] = 033; yield jhbuzy_0[1];"");}catch(ex){}
try{eval(""print(x);print(x);"");}catch(ex){}
try{eval(""L:while((functional = Proxy.create((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: undefined, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: window.isScopeNamespace, keys: Proxy.create, }; })(\u3056.byteLength = (function handlerFactory(x) {return {getOwnPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor'); var desc = Object.getOwnPropertyDescriptor(x); desc.configurable = true; return desc; }, getPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor'); var desc = Object.getPropertyDescriptor(x); desc.configurable = true; return desc; }, defineProperty: function(name, desc) { var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty'); Object.defineProperty(x, name, desc); }, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return Object.getOwnPropertyNames(x); }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); if (Object.isFrozen(x)) { return Object.getOwnProperties(x); } }, has: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return name in x; }, hasOwn: undefined, get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: function(receiver, name, val) { var yum = 'PCAL'; dumpln(yum + 'LED: set'); x[name] = val; return true; }, iterate: ((decodeURIComponent).bind(undefined)).bind, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); var result = []; for (var name in x) { result.push(name); }; return result; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return Object.keys(x); }, }; })), (this |=  \""\"" ))) && 0)yield;function x(functional, x){/*jjj*/}print(null);function eval(a, functional){/*jjj*/}print( /x/ );function this(x){/*jjj*/}return this;( /x/g );"");}catch(ex){}
try{eval(""/*oLoop*/for (hpcgta = 0; hpcgta < 6; ++hpcgta) { print(x); } \n(((Function).call(((w | \u3056)[encodeURIComponent()]), \n \""\"" )));\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""240"");
try{eval(""eval, [] = (let (d = ({x::PI: \""\\uA9CE\"", x: [,] })) Math.atan2(this, 1e4));(Math.cos(-2147483648));"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; mjblzy();/*hhh*/function mjblzy([, ], eval){print(x);}"");}catch(ex){}
try{eval(""this;"");}catch(ex){}
try{eval(""\""use strict\""; /*hhh*/function bococs((c)(window)){return;}bococs(-0.yoyo(a), this);"");}catch(ex){}
try{eval(""var beegcr = new ArrayBuffer(12); var beegcr_0 = new Float32Array(beegcr); beegcr_0[0] = (new XPCNativeWrapper(-1672847067)); var beegcr_1 = new Uint8ClampedArray(beegcr); print(beegcr_1[0]); beegcr_1[0] = -0; var beegcr_2 = new Uint8ClampedArray(beegcr); beegcr_2[0] = 274877906944; var beegcr_3 = new Int16Array(beegcr); print(beegcr_3[0]); beegcr_3[0] = 524287; print(beegcr_3); /x/ ;[z1];print(window);print(\""\\u4\"");L:while((\""\\u28CD6\"") && 0)(033);"");}catch(ex){}
try{eval(""let (agonwq, functional = (4277), d = (functional =  /x/  for (a in null)\u0009 for each (x in this)), c = undefined, x, wkgldo) { print(x);\nObject.defineProperty(a, \""a\"", ({}));\n }"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let z in [ '\\0' ,  '\\0' , function(){},  '\\0' , function(){}, function(){},  '\\0' ,  '\\0' , function(){}, function(){}, function(){},  '\\0' ,  '\\0' , function(){},  '\\0' ,  '\\0' , function(){}, function(){}, function(){},  '\\0' ,  '\\0' ,  '\\0' , function(){},  '\\0' ]) { continue M; }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""250"");
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let e in [new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), x, x, new Number(1.5)]) { return; }"");}catch(ex){}
try{eval(""do print(x); while(((p={}, (p.z = #3={a:#3#})())) && 0);"");}catch(ex){}
try{eval(""\""use strict\""; /*vLoop*/for (var cxffao = 0; cxffao < 12; ++cxffao) { var c = cxffao;  '' ; } "");}catch(ex){}
try{eval(""print('fafafa'.replace(/a/g, d)['_' + (arguments)]);"");}catch(ex){}
try{eval(""\""use strict\""; bvhfes();/*hhh*/function bvhfes(w, [, ([])]){undefined.function::concat(window);}"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let w in [new String(''), x, -971875916, new String(''), new String(''), new String(''), -971875916, new String(''), null, -971875916, x, x, -971875916, new String(''), -971875916, new String(''), -971875916, null, x, x, x, x, new String(''), new String(''), null, new String(''), -971875916, null, new String(''), -971875916, null, null, -971875916, -971875916, new String(''), x, null, new String('')]) { print(x); }"");}catch(ex){}
try{eval(""\""use strict\""; gczeal(0);\nprint(x);\n"");}catch(ex){}
try{eval(""L:if(x.eval(z)) { if (x) {print(x);window } else {a; }}"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""260"");
try{eval(""null;function  /x/g (){/*jjj*/}print(this);"");}catch(ex){}
try{eval(""\""use strict\""; ( \""\"" );function w(){/*jjj*/}return \""\\u7D\"";"");}catch(ex){}
try{eval(""\""use strict\""; print(x)"");}catch(ex){}
try{eval(""\""use strict\""; qydvoj(x = Proxy.createFunction((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor'); var desc = Object.getOwnPropertyDescriptor(x); desc.configurable = true; return desc; }, getPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor'); var desc = Object.getPropertyDescriptor(x); desc.configurable = true; return desc; }, defineProperty: function(name, desc) { var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty'); Object.defineProperty(x, name, desc); }, getOwnPropertyNames: Object.getOwnPropertyNames, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); if (Object.isFrozen(x)) { return Object.getOwnProperties(x); } }, has: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return name in x; }, hasOwn: /a/gi, get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: undefined, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { for (var name in x) { yield name; } })(); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); var result = []; for (var name in x) { result.push(name); }; return result; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return Object.keys(x); }, }; })(null), function(y) { yield y; print(y);; yield y; },  /x/ ) ? x : w.x::valueOf(\""number\""));/*hhh*/function qydvoj(){(x &=  /x/ );}"");}catch(ex){}
try{eval(""\""use strict\""; /*vLoop*/for (let dcsape = 0; dcsape < 11; ++dcsape) { let e = dcsape; \""use strict\""; print(e); } "");}catch(ex){}
try{eval(""var ftzpch = new ArrayBuffer(6); var ftzpch_0 = new Float64Array(ftzpch); print(ftzpch_0[0]); ftzpch_0[0] = (Proxy.isTrapping)((function ([y]) { }.__defineSetter__(\""x\"", function(y) { return window })), x); var ftzpch_1 = new Uint32Array(ftzpch); ftzpch_1[0] = 2372951125; var ftzpch_2 = new Int8Array(ftzpch); print(ftzpch_2[0]); ftzpch_2[0] = 0; var ftzpch_3 = new WebGLIntArray(ftzpch); ftzpch_3[0] = -17179869184; (( '' .__defineSetter__(\""d\"", Math.cos)));/*vLoop*/for (var ykcaxy = 0; ykcaxy < 6; ++ykcaxy) { c = ykcaxy; print(ftzpch_0[0]); } print(1.3);yield;print(new (function  ftzpch_3[0] (z)\""\\uC\"")()); /x/g ;"");}catch(ex){}
try{eval(""print(x);\ndo {print([1,,]); } while((('fafafa'.replace(/a/g, function shapeyConstructor(lmixmy){for (var ytqizgtzw in lmixmy) { }return lmixmy; }))) && 0);\n"");}catch(ex){}
try{eval(""/*vLoop*/for (let wnxjjt = 0; wnxjjt < 4; ++wnxjjt) { y = wnxjjt; {\""use strict\""; ; } } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""270"");
try{eval(""L: ( /x/ );"");}catch(ex){}
try{eval(""b::y, \u3056 = ( \""\""  >> window++), fhqzws, vnkdth, y, ocdznw, x, itgnnt;gczeal(0);\n[z1];print(x);\n"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""for(let a in ((function shapeyConstructor(ejoelg){if ( '' ) this.c = ({});return this; })( '' ))){print(a); }"");}catch(ex){}
try{eval(""{}\nprint( /x/ );\n\n/*oLoop*/for (fqzsgk = 0; fqzsgk < 10; ++fqzsgk) { print(x); } \n"");}catch(ex){}
try{eval(""{}function window(){/*jjj*/}( '' );"");}catch(ex){}
try{eval(""switch(~null) { case eval().__proto__ = gc: print((gczeal(0).throw(Object.defineProperty(x::x, \""x\"", ({})))));break; case ((new XPCSafeJSObjectWrapper(\""\\u9\""))['_' + ((x = Proxy.create((function handlerFactory(x) {return {getOwnPropertyDescriptor: ArrayBuffer, getPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor'); var desc = Object.getPropertyDescriptor(x); desc.configurable = true; return desc; }, defineProperty: function(name, desc) { var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty'); Object.defineProperty(x, name, desc); }, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return Object.getOwnPropertyNames(x); }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); if (Object.isFrozen(x)) { return Object.getOwnProperties(x); } }, has: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return name in x; }, hasOwn: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return Object.prototype.hasOwnProperty.call(x, name); }, get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: function(receiver, name, val) { var yum = 'PCAL'; dumpln(yum + 'LED: set'); x[name] = val; return true; }, iterate: undefined, enumerate: -2611368886, keys: undefined, }; })(\""\\u93FB48\""),  '' )))]): print(x);break; case NaN: return;case 9: ;break;  }"");}catch(ex){}
try{eval(""\""use strict\""; for(w in ((XPCNativeWrapper)((x)))){gc() }"");}catch(ex){}
try{eval(""\""use strict\""; let (lmrsas, yhcoxm) { ; }"");}catch(ex){}
try{eval(""/*tLoop*/for each (let c in [[], [], function(){}, function(){}, function(){}, [], function(){}, function(){}, [], [], function(){}, [], function(){}, [], function(){}, [], function(){}, [], [], [], function(){}, function(){}, function(){}, function(){}, function(){}, [], [], function(){}, [], [], [], [], function(){}, function(){}, [], function(){}, function(){}]) { var avpnmj = new ArrayBuffer(0); var avpnmj_0 = new Int32Array(avpnmj); avpnmj_0[0] = -4294967297; print(c); }"");}catch(ex){}
try{eval(""print(new (Proxy.createFunction)());\n((this || \""\\u5931\""));\n"");}catch(ex){}
try{eval(""/*tLoop*/for each (let e in [new Boolean(true), (0/0), (0/0), (0/0), new Boolean(true), new Boolean(true), (0/0), new Boolean(false), new Boolean(false), new Boolean(false), (0/0), new Boolean(false), (0/0), new Boolean(false), (0/0), new Boolean(true), new Boolean(true), new Boolean(true), new Boolean(false)]) { {}function x(){/*jjj*/}print(x); }"");}catch(ex){}
try{eval(""(print(x));function x(eval){/*jjj*/}/*bLoop*/for (let osxggc = 0; osxggc < 2; ++osxggc) { if (osxggc % 10 == 9) { yield; } else { (b); }  } "");}catch(ex){}
try{eval(""const d = x += (let (e=eval) e).prototype, x = this, kcacta, d = functional.x, x = function(id) { return id }.text(\""\\u254\"", this), ltyndr, x =  '' , set, y, \u3056;\""\\u9\"";"");}catch(ex){}
try{eval(""\""use strict\""; print(x);function functional(){/*jjj*/}yield false;"");}catch(ex){}
try{eval(""const x = x.watch(\""NaN\"", function  y (a) { yield window } );"");}catch(ex){}
try{eval(""\""use strict\""; let (x = x, a, yoclyy, fmwhum) { /*vLoop*/for (let bdfyov = 0; x && bdfyov < 2; ++bdfyov) { var d = bdfyov; 2251799813685247; }  }"");}catch(ex){}
try{eval(""/*vLoop*/for (var pnodjo = 0, x; pnodjo < 0; ++pnodjo) { z = pnodjo; print( \""\"" ); } "");}catch(ex){}
try{eval(""jrnnoe(x, );/*hhh*/function jrnnoe(z, x){}"");}catch(ex){}
try{eval(""/*tLoop*/for each (let d in [[1], [1], [1], (1/0), [1], (1/0), (1/0), [1], (1/0), [1], [1], [1], (1/0), [1], [1], (1/0), [1], [1], (1/0), (1/0), [1], [1], (1/0), [1], [1], (1/0), (1/0), [1], (1/0), [1], (1/0), (1/0), (1/0), (1/0), [1]]) { print(x); }"");}catch(ex){}
try{eval(""print(false % true);\nprint(x);\nvar oprepd = new ArrayBuffer(4); var oprepd_0 = new Int8Array(oprepd); print(oprepd_0[0]); oprepd_0[0] = -3435616826.6081147; var oprepd_1 = new Uint32Array(oprepd); oprepd_1[0] = -4503599627370495; (\""\\u6\"");print(oprepd_0[0]);"");}catch(ex){}
try{eval(""\""\\uD5DC4\"";"");}catch(ex){}
try{eval(""{print(x);print(\""\\u670F4\"".unwatch(\""eval\"")); }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""300"");
try{eval(""\""use strict\""; var ndqatt = new ArrayBuffer(0); var ndqatt_0 = new Int32Array(ndqatt); ndqatt_0[0] = 3.141592653589793; print(ndqatt_0[0]);function window(){/*jjj*/} /x/g ;\nprint(\""\\uB3\"");\n"");}catch(ex){}
try{eval(""/*bLoop*/for (axivcz = 0; axivcz < 3; (4277), ++axivcz) { if (axivcz % 10 == 9) { print(x); } else { yield this; }  } "");}catch(ex){}
try{eval(""\""use strict\""; print(x + \""\\u21\"");function w(c){/*jjj*/}(true);"");}catch(ex){}
try{eval("" /x/ ;return  /x/ ;"");}catch(ex){}
try{eval(""let elpoyt, osjwnn;((p={}, (p.z = (x) = functional.__count__ = (e = z))()));"");}catch(ex){}
try{eval(""\""use strict\""; ( /x/g );"");}catch(ex){}
try{eval(""/*hhh*/function ssnzaf(x, x::x){gczeal(0);}ssnzaf(((4277) ? NaN /=  /x/g  : 'fafafa'.replace(/a/g, Proxy.createFunction))[( /x/ .throw((b =  /x/g ))--)], -0.2583279378441378);\n/*oLoop*/for (let nlnaga = 0; x::a && nlnaga < 10; ++nlnaga) { print( \""\"" ); } \n"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""310"");
try{eval(""print(x);"");}catch(ex){}
try{eval(""/*iii*/print(ojusyi);yield;/*hhh*/function ojusyi(x, x){[z1];\nreturn;\n}"");}catch(ex){}
try{eval(""/*hhh*/function rffedn(){{print(x);throw this; }}/*iii*/let aovfyi, rffedn, x::rffedn, atbfjm, upfoap;print(rffedn);"");}catch(ex){}
try{eval(""default xml namespace  = new Object.getOwnPropertyNames( /* Comment */functional.prettyIndent = null, ('fafafa'.replace(/a/g, Array.reduce)));"");}catch(ex){}
try{eval("""");}catch(ex){}
try{eval("" for  each(var y in (x)((function () /x/g ).call( \""\"" , ), \""\\uBA1C\"")) {{x } }"");}catch(ex){}
try{eval(""\""use strict\""; ;function x(x){/*jjj*/}print(x);"");}catch(ex){}
try{eval(""((e = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: undefined, defineProperty: window, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum \000+ 'LED: hasOwn'); return false; }, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: window, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; })(x), eval -= window.__lookupGetter__))); /x/g ;"");}catch(ex){}
try{eval(""\""use strict\""; ( '' );\nprint(x);\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""320"");
try{eval(""x[window];"");}catch(ex){}
try{eval(""M:if(x) /*tLoop*/for each (let y in [new String('q'), (x.function::x), (x.function::x)]) { print(x); } else  if (new String()) {/*tLoop*/for each (let b in [ \""use strict\"" ,  \""use strict\"" , new Number(1.5),  /x/g ,  /x/ ,  /x/g ,  /x/g ,  /x/g ,  /x/g ,  \""use strict\"" , new Number(1.5),  /x/ ,  \""use strict\"" ,  /x/ ,  /x/ ,  \""use strict\"" ,  \""use strict\"" ,  \""use strict\"" , new Number(1.5),  /x/g ,  /x/g , new Number(1.5),  /x/ ,  \""use strict\"" ,  /x/g ,  \""use strict\"" ,  /x/ , new Number(1.5),  \""use strict\"" ,  /x/ ,  /x/ ,  /x/ , new Number(1.5)]) { print(b); } } else {var koygmn = new ArrayBuffer(0); var koygmn_0 = new Int32Array(koygmn); koygmn_0[0] = 1048575; var koygmn_1 = new Float32Array(koygmn); koygmn_1[0] = (4. ?  /x/g  :  /x/g ); var koygmn_2 = new Uint16Array(koygmn); koygmn_2[0] = -0.46547039590515576; var koygmn_3 = new Float64Array(koygmn); print(koygmn_3[0]); koygmn_3[0] = 1.3; Functionreturn (({x setter: XPCSafeJSObjectWrapper, window setter: Object.freeze }));print(Object.defineProperty(eval, \""koygmn_2\"", ({configurable: false})).__defineGetter__(\""b\"", (function ([y]) { })())); }"");}catch(ex){}
try{eval(""with({}) { gczeal(0); } let(e) {  '' ;}"");}catch(ex){}
try{eval(""/*oLoop*/for (var crhkin = 0; crhkin < 1; ++crhkin) { print(this); } "");}catch(ex){}
try{eval(""yield;"");}catch(ex){}
try{eval("" for  each(var z in #1#) {M:for(var x in  /x/g ) null; }"");}catch(ex){}
try{eval(""do print(x); while((#2=({x: Math.min(-2001444430.7729876, window)})) && 0);"");}catch(ex){}
try{eval(""Math.pow(1099511627776, (4277));"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""330"");
try{eval(""(b instanceof eval) %= x;"");}catch(ex){}
try{eval(""let (e = (RegExp(new XPCSafeJSObjectWrapper(a))), x = (4277), window = Math, x = [11,12,13,14].map) { L:if(this.prototype.elements(x)) {print(x);gc() } }"");}catch(ex){}
try{eval(""--(function ([y]) { })().unwatch(\""x\"")\u000c;\nprint( /x/ );\n"");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (fnnore = 0, y = ({}); fnnore < 24; ++fnnore) { if (fnnore % 9 == 4) { ; } else { print(x); }  } "");}catch(ex){}
try{eval(""\""use strict\""; let(b) { print(b);}for(let b in []);"");}catch(ex){}
try{eval(""switch(d && \u3056) { case \""\\u858C19\"": case Math.sin(Math.min(-0.5962413515443898,  /x/g )): /*hhh*/function epiwow(w){return  \""\"" ;}/*iii*/( /x/g );case 5: (eval(\""print(\\\""\\\\u1128\\\"");\""));break;  }"");}catch(ex){}
try{eval(""switch( \""\"" ) { default: case 1: break; case x: break; case 6: print(x); /x/g ;break; break;  }"");}catch(ex){}
try{eval(""let (z) { gc() }"");}catch(ex){}
try{eval(""let ({b, a: [, ], c: {x: z, x, window: {x: NaN, b: []}, functional}} = x, x, function::z = x = Proxy.createFunction((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor:  \""\"" , defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: undefined, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; })(null), window, (new Function(\""print(x);\""))), x) { /*bLoop*/for (let jotbbc = 0, (4277); jotbbc < 6; this.__defineSetter__(\""NaN\"", Object.isExtensible), ++jotbbc) { if (jotbbc % 10 == 1) { gc()function x(e){/*jjj*/}print(x); } else { print(x); }  }  }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""340"");
try{eval(""with({b: (4277)})var bdwuwc = new ArrayBuffer(8); var bdwuwc_0 = new Float64Array(bdwuwc); bdwuwc_0[0] = -524289; yield  '' ;print(bdwuwc_0);print(-1611142111);print(bdwuwc_0[9]);print(\""\\u14127\"");true;"");}catch(ex){}
try{eval(""(x);\nprint(x);\n"");}catch(ex){}
try{eval(""var htsljb = new ArrayBuffer(1); var htsljb_0 = new WebGLIntArray(htsljb); print(htsljb_0[0]); yield;print(x);gczeal(0);print(htsljb_0[9]);/*oLoop*/for (qaqrog = 0,  '' ; qaqrog < 12; ++qaqrog) { yield; } yield;(eval);"");}catch(ex){}
try{eval(""const c = d.yoyo(window);Object.sealx;"");}catch(ex){}
try{eval(""print(\""\\u8CE45D\"");if( /x/ ) { if ( /x/g ) {(null); }} else {\""\\u6F7532\"";{} }"");}catch(ex){}
try{eval(""\""use strict\""; var qouehb = new ArrayBuffer(6); var qouehb_0 = new Uint8ClampedArray(qouehb); print(qouehb_0[0]); qouehb_0[0] = 0.03850380533489073; \""\\u1636\"".reduceRight"");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (let ypwwdi = 0; ypwwdi < 17; ++ypwwdi) { if (ypwwdi % 11 == 1) { print((4277)); } else { d = window; }  } "");}catch(ex){}
try{eval(""\""use strict\""; true;"");}catch(ex){}
try{eval(""JSON.parselet b = yield ((x)) = x/*\n*/;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""350"");
try{eval(""\""use strict\""; let (x) { /*bLoop*/for (var crvouh = 0, function::x; crvouh < 14; ++crvouh) { if (crvouh % 3 == 1) { (c); } else { print(x); }  }  }"");}catch(ex){}
try{eval(""var gjnlms = new ArrayBuffer(6); var gjnlms_0 = new Int32Array(gjnlms); print(gjnlms_0[0]); gjnlms_0[0] = 1e+81; /*oLoop*/for (var klpcdw = 0; klpcdw < 7; ++klpcdw) { print(gjnlms); } "");}catch(ex){}
try{eval(""var gpfypu = new ArrayBuffer(1); var gpfypu_0 = new Int32Array(gpfypu); gpfypu_0[0] = -1023; var gpfypu_1 = new Uint16Array(gpfypu); print(gpfypu_1[0]); gpfypu_1[0] = 0; (x);(true);"");}catch(ex){}
try{eval(""print(\u3056.hasSimpleContent(false,  '' ).E = encodeURIComponent);"");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (var uwbxgo = 0; uwbxgo < 5; ++uwbxgo) { ; } "");}catch(ex){}
try{eval(""print(window);function a(){/*jjj*/}(1 for (x in []))"");}catch(ex){}
try{eval(""\""use strict\""; ((function ([y]) { })());var x = -0;"");}catch(ex){}
try{eval(""{}\n( '' );\n\n\u0009yield this;\n"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""yield;(false);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""360"");
try{eval(""\""use strict\""; break M;(\""\\u0DAF\"");"");}catch(ex){}
try{eval(""if(Math.acos(4.)) {/*tLoop*/for each (let y in [x = this, new Boolean(true), 2, (-1/0), x = this, x = this, 2, x = this, x = this, x = this, x = this, new String(''), (-1/0), new String(''), (-1/0), new Boolean(true), (-1/0), (-1/0), (-1/0), x = this, (-1/0), (-1/0), (-1/0), new String('')]) { (false); } } else  if ([\""\\u0\""]) {switch( \""\""  /  /x/g ) { default: print(((/a/gi).call(w.x::d, (4277) ^= window, void x))); } }"");}catch(ex){}
try{eval(""print(x);\nprint(x);\n"");}catch(ex){}
try{eval(""\""use strict\""; this.zzz.zzz;throw x;"");}catch(ex){}
try{eval(""\""use strict\""; default xml namespace  = (#0=x);"");}catch(ex){}
try{eval(""/*oLoop*/for (var oqtaeh = 0; oqtaeh < 7; ++oqtaeh) { yield; } "");}catch(ex){}
try{eval(""L: {print(new (this)()); }"");}catch(ex){}
try{eval(""/*hhh*/function dyjajm/*\n*/(NaN){for([a, w] = x in x) {print((([]) = (new XPCSafeJSObjectWrapper( \""\"" )))); }}dyjajm(-511);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""370"");
try{eval(""\""use strict\""; /*tLoop*/for each (let b in [true, true, new String(''), (1/0), new String(''), (1/0), new String(''), new String(''), new String(''), new String(''), new Boolean(false), (1/0), true, true, (1/0), new Boolean(false), true, (1/0), new String(''), new String(''), new String(''), true, (1/0), true, new String(''), new String(''), new Boolean(false), (1/0), (1/0), true, true, new Boolean(false), new String(''), new Boolean(false), new Boolean(false), new Boolean(false), true, new Boolean(false), new Boolean(false)]) { print(((yield z.x setter= true))); }"");}catch(ex){}
try{eval(""print(x);function x( ){/*jjj*/}9007199254740992;"");}catch(ex){}
try{eval(""/*tLoop*/for each (let d in []) { print(x); }"");}catch(ex){}
try{eval(""\""use strict\""; const grcors, x, mfqyyr, \u3056, NaN, yield, grvofw, bufzyd, x;print( /x/g );function \u3056(){/*jjj*/}print(((function factorial(zdbwvd) { ; if (zdbwvd == 0) return 1; print((\u0009this.__defineGetter__(\""x\"", ArrayBuffer)));; return zdbwvd * factorial(zdbwvd - 1);  })(2)));"");}catch(ex){}
try{eval(""/*bLoop*/for (var ixpabh = 0; ixpabh < 18; ++ixpabh,  \""\"" ) { if (ixpabh % 6 == 1) { (undefined); } else { print(x); }  } "");}catch(ex){}
try{eval(""let eeszyv;print(this.yoyo( /x/g ));"");}catch(ex){}
try{eval(""L: {print(x);(function handlerFactory() {return {getOwnPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor'); var desc = Object.getOwnPropertyDescriptor(x); desc.configurable = true; return desc; }, getPropertyDescriptor: JSON.parse, defineProperty: function(name, desc) { var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty'); Object.defineProperty(x, name, desc); }, getOwnPropertyNames: undefined, fix:  '' , has: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return name in x; }, hasOwn: \""\\u246E\"", get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: function(receiver, name, val) { var yum = 'PCAL'; dumpln(yum + 'LED: set'); x[name] = val; return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { for (var name in x) { yield name; } })(); }, enumerate: -281474976710656, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return Object.keys(x); }, }; }) }"");}catch(ex){}
try{eval(""(((c = new (Math.atan)(((w =  \""\"" )), -0).__defineGetter__(\""z\"", function(y) { yield; })).parent((x >>= x), x)));"");}catch(ex){}
try{eval(""L:for(var [c, a] = x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: undefined, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: undefined, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; })([NaN = \u3056 if ( '' )]), XPCNativeWrapper) in (eval) = x) {print(c); }"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""print(x);gczeal(0);"");}catch(ex){}
try{eval(""\""use strict\""; /*hhh*/function uftxwb(x, functional){print(x);}/*iii*/print(x);"");}catch(ex){}
try{eval(""\""use strict\""; default xml namespace  = 4503599627370496;\nreturn eval(\""print(x);\"", \""\\u63\"");\n"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""\""use strict\""; let iehdjh, zkgewj, retcco, wttznr, mhbrdv, voznlk, zuvyaj, krqrnq, iyzrui;gczeal(0);"");}catch(ex){}
try{eval(""\""use strict\""; return;"");}catch(ex){}
try{eval(""let \u3056 = Math.min(262145, 549755813889);if(yield = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: undefined, getPropertyDescriptor: encodeURI, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: undefined, fix: undefined, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: undefined, iterate: undefined, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; })(new \""\\u4B3\""()), new Function)) { if (d) {gczeal(0);print( '' ); }} else { '' ;print(x); }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""390"");
try{eval(""print(x);"");}catch(ex){}
try{eval(""do throw Math; while((null) && 0);"");}catch(ex){}
try{eval(""let(d) ((function(){let(NaN = (4277)) ((function(){false;})());})());"");}catch(ex){}
try{eval(""\""use strict\""; if(delete x. ) c = -3490026166;yield  /x/g ; else {{}(Proxy.createFunction)( /x/ , (function ([y]) { })()) }"");}catch(ex){}
try{eval(""\""use strict\""; let (z) { print(x); }"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""throw StopIteration;\nprint(x);\n"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""400"");
try{eval(""var ifasks = new ArrayBuffer(12); var ifasks_0 = new Float32Array(ifasks); print( /x/ );print(ifasks_0[2]);"");}catch(ex){}
try{eval(""with({x: (4277).yoyo(x)}){x = \u3056; }"");}catch(ex){}
try{eval(""print([,]);print(x);"");}catch(ex){}
try{eval(""\""use strict\""; var keewtk = new ArrayBuffer(12); var keewtk_0 = new Uint16Array(keewtk); var keewtk_1 = new WebGLIntArray(keewtk); var keewtk_2 = new Uint8ClampedArray(keewtk); print(keewtk_2);"");}catch(ex){}
try{eval(""if(x) { if (new Object.keys((function ([y]) { })(), this)) {( \""\"" ); } else {( /x/ );this; }}"");}catch(ex){}
try{eval(""break L;\nprint(x);\n"");}catch(ex){}
try{eval(""yield"");}catch(ex){}
try{eval(""return x;throw window;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""410"");
try{eval(""x;function x(){/*jjj*/}continue ;"");}catch(ex){}
try{eval(""print(x);function a()\u000c{/*jjj*/}return 3141745214.744757;"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let a in [true, true,  /x/g , 0xB504F332, true, true, 0xB504F332, 0xB504F332,  /x/g ,  /x/g ]) { print(-0.5813145310637711);function x(){/*jjj*/}print(a); }"");}catch(ex){}
try{eval(""/*vLoop*/for (syoqaa = 0; syoqaa < 6 && (d = (( /x/g )(a, this))); ++syoqaa) { var y = syoqaa; var d, kxhcgw, x, qrnddu, y;([,,]); } "");}catch(ex){}
try{eval(""/*hhh*/function ketbky(){var zpdbrr = new ArrayBuffer(12); var zpdbrr_0 = new Int8Array(zpdbrr); zpdbrr_0[0] = (-0); var zpdbrr_1 = new Uint16Array(zpdbrr); print(zpdbrr_1[0]); zpdbrr_1[0] = -35184372088831; return this;print(\""\\uA3\"");}/*iii*/print(ketbky);"");}catch(ex){}
try{eval(""for(a = window in  /x/g ) {print(a);print(x); }"");}catch(ex){}
try{eval(""\""use strict\""; this.zzz.zzz;"");}catch(ex){}
try{eval(""\""use strict\""; x;(function ([y]) { })();"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""420"");
try{eval(""\""use strict\""; if(x) { if (new NaN(window) ? (new Array(-16)) : x) for(let e in  '' ) print(x);} else {function this.x(b){/*jjj*/}(\""\\u2E\""); }"");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (qxhbnb = 0; qxhbnb < 9; ++qxhbnb) { if (qxhbnb % 10 == 9) { yield Math.log(undefined); } else { /*iii*//*hhh*/function wqritt(w){print(w);} }  } "");}catch(ex){}
try{eval(""/*tLoop*/for each (let b in [ \""\"" ,  \""\"" , true,  \""\"" , true, true, true,  \""\"" ]) { var nebber = new ArrayBuffer(4); var nebber_0 = new Uint32Array(nebber); print(nebber_0[0]); nebber_0[0] = -3304037903; print(window); }"");}catch(ex){}
try{eval(""const d = ({}) = (4277);(p={}, (p.z = \""\\u4\"")());"");}catch(ex){}
try{eval(""/*oLoop*/for (let rwummz = 0; rwummz < 0; ++rwummz) { gczeal(0);function this.x(){/*jjj*/}window; } "");}catch(ex){}
try{eval(""\""use strict\""; delete x.c;"");}catch(ex){}
try{eval(""function shapeyConstructor(ojwzmw){{ print(new RegExp(x,  '' )); } Object.freeze(ojwzmw);delete ojwzmw.w;ojwzmw.b = (ojwzmw + (4277));ojwzmw.b = ojwzmw;if ((let (a = this) \""\\uCFA52\"")) delete ojwzmw.b;ojwzmw.d = eval;{ print(z = undefined); } return ojwzmw; }/*tLoopC*/for each (let x in [(void 0), new Boolean(false), (void 0), new Boolean(false), new Boolean(false), (void 0), new Boolean(false), new Boolean(false), (void 0), new Boolean(false), (void 0), (void 0), new Boolean(false), new Boolean(false), new Boolean(false), (void 0), (void 0), new Boolean(false), new Boolean(false), (void 0), new Boolean(false), (void 0), (void 0), (void 0)]) { try{let gfgdhk = shapeyConstructor(x); print('EETT'); print(x);}catch(e){print('TTEE ' + e); } }"");}catch(ex){}
try{eval(""try { (window); } catch(d) { ; } finally { continue ; } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""430"");
try{eval(""\""use strict\""; /*bLoop*/for (let opgavu = 0; opgavu < 23; ++opgavu) { if (opgavu % 7 == 3) { print( /x/  *= {}); } else { \""use strict\""; print(function ([y]) { }); }  } "");}catch(ex){}
try{eval(""x;"");}catch(ex){}
try{eval(""\""use strict\"";  set this.b a ([NaN]) { \""use strict\""; return (++x != -1099511627775 != {}) } "");}catch(ex){}
try{eval(""do print(x); while(((uneval(false))) && 0);"");}catch(ex){}
try{eval(""/*bLoop*/for (var yxtdhk = 0; yxtdhk < 0; ++yxtdhk) { if (yxtdhk % 5 == 1) { throw w;this.zzz.zzz; } else { (([] = 'fafafa'.replace(/a//h\n/g, Function))); }  } "");}catch(ex){}
try{eval(""x =  '' , riljqp, qshwie, auskqm, ccefji, x, functional, amghvh;{}"");}catch(ex){}
try{eval(""print(this);return;"");}catch(ex){}
try{eval(""(72057594037927940);\nreturn x;\n"");}catch(ex){}
try{eval(""yield \""\\u9AA\"";"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""440"");
try{eval(""continue M;"");}catch(ex){}
try{eval(""/*tLoop*/for each (let b in [new Number(1), {x:3}, new Number(1), {x:3}, new Number(1), {x:3}, new Number(1), new Number(1), {x:3}, new Number(1), new Number(1), new Number(1), {x:3}, {x:3}, new Number(1), new Number(1), {x:3}, new Number(1), {x:3}, new Number(1), {x:3}, new Number(1), new Number(1), new Number(1), new Number(1), {x:3}, {x:3}, new Number(1), new Number(1), new Number(1), {x:3}]) { print(x); }"");}catch(ex){}
try{eval(""/*iii*/new (this.__defineGetter__(\""c\"", \""\\uF7AF\""))()\nprint(otceap);gc()/*hhh*/function otceap(w){(this.e << c);let y = let (z)  /x/ ;}{const x = this, tkkdmo;qqtems();/*hhh*/function qqtems(a, x){print(x);} }"");}catch(ex){}
try{eval(""(d);"");}catch(ex){}
try{eval(""M:for(w = new XPCSafeJSObjectWrapper((x.yoyo( '' .__defineSetter__(\""this.NaN\"",  /x/g )))) in 'fafafa'.replace(/a/g, x instanceof  /x/g )) print(x);\nprint(x);\n"");}catch(ex){}
try{eval(""/*oLoop*/for (var knsipd = 0, x = x; knsipd < 0; ++knsipd, (4277)) { print((new [1])); } "");}catch(ex){}
try{eval(""var pywqfr = new ArrayBuffer(4); var pywqfr_0 = new Uint8ClampedArray(pywqfr); print(pywqfr_0[0]); pywqfr_0[0] = (let (z) function(id) { return id }); var pywqfr_1 = new Float64Array(pywqfr); pywqfr_1[0] = -17179869185; var pywqfr_2 = new Uint8ClampedArray(pywqfr); var pywqfr_3 = new Float32Array(pywqfr); pywqfr_3[0] = 268435457; yield;(#1=[#1#]);\n\""\\uC3A\"";\nprint(pywqfr_1[0]);print(\""\\u636\"");"");}catch(ex){}
try{eval(""\""use strict\""; for(e in (((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: undefined, iterate: (encodeURIComponent).bind([[1]]), enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: Proxy.createFunction, }; }))(Math))){-17; }"");}catch(ex){}
try{eval(""\""use strict\""; if(x.d getter= Object.isExtensible) { if (x =  '' ) {gczeal(2);return  \""\"" ; }} else {null; }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""450"");
try{eval(""const enfvvl, cupxll, NaN, y = ( /x/  != window), NaN = \u3056, hakvrm;w && 3.141592653589793.yoyo([[]])--;"");}catch(ex){}
try{eval(""x = let (x = x) ({eval: \""\\uA7AE9\"" }), rldqmf, functional = x.w\u000c setter= encodeURI, kfozlq, x = window, x, duzvyq, d, NaN;print([z1]);"");}catch(ex){}
try{eval(""\""use strict\""; -4503599627370495;\n\""\\u3783\"";\n\nconst b = false;gczeal(0);\n"");}catch(ex){}
try{eval(""\""use strict\""; if(( \""\"" .__defineSetter__(\""function::NaN\"", /*wrap1*/(function(){ return window;return Proxy.create})()))) ( '' ); else  if ((Proxy.isTrapping).call(-1099511627777, )) print(x);"");}catch(ex){}
try{eval(""gczeal(0);\nprint([]());\n\nwith(new null)yield undefined;\n"");}catch(ex){}
try{eval("";"");}catch(ex){}
try{eval(""var zuvika = new ArrayBuffer(2); var zuvika_0 = new Int8Array(zuvika); zuvika_0[0] = -35184372088832; var zuvika_1 = new Uint32Array(zuvika); zuvika_1[0] = 1e4; var zuvika_2 = new Int8Array(zuvika); zuvika_2[0] = 4071007061; var zuvika_3 = new Int8Array(zuvika); print(zuvika_3[1]);print(this.zzz.zzz);"");}catch(ex){}
try{eval(""print(Object.isExtensible.prototype);print(([] = this));"");}catch(ex){}
try{eval(""const ggbgvj, gjyaxm, \u3056, y, kzmfzu;(typeof  \""\"" );function x(){/*jjj*/}M:if((eval(\""(([]) = ((x) = NaN))\"", [x if ([,,])]))) {print(x);print(null); } else \u000c if ((Object.isSealed)()) {print( /x/g );NaN; } else (\""\\uB3\"")\u0009;"");}catch(ex){}
try{eval(""\""use strict\""; (x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""460"");
try{eval(""\""use strict\""; switch([false].sort(Object.freeze)) { default: \""\\uA\""break;  }"");}catch(ex){}
try{eval(""/*hhh*/function cralay(x){print(x);}/*iii*/const zuterj, xsidpd, x = 34359738369, ykacyi, x =  /x/g ;return c;function this.functional(a, y){/*jjj*/}print(cralay);"");}catch(ex){}
try{eval(""print('fafafa'.replace(/a/g, Object.freeze));Object.defineProperties"");}catch(ex){}
try{eval(""function shapeyConstructor(xkuzrq){this.a = Object.freeze;this.a =  /x/ ;this.a = Object.defineProperties;if (xkuzrq) Object.defineProperty(this, \""a\"", ({enumerable: false}));Object.freeze(this);this.a = x;return this; }/*tLoopC*/for each (let b in [(void 0), function(){}, (void 0), (0/0), function(){}, (void 0), function(){}, function(){}, function(){}, (void 0), -3/0, -3/0, function(){}, (void 0), (void 0), function(){}, (void 0), (void 0), function(){}, -3/0, function(){}, function(){}, (void 0), (void 0), (void 0), function(){}, function(){}, (void 0), function(){}, -3/0, -3/0, function(){}, (void 0), (void 0)]) { try{let ibbkzr = shapeyConstructor(b); print('EETT'); yield delete \""\\u53F1C0\"";let(b, \u3056, b, ibbkzr) { yield 0.04267613113333496;}}catch(e){print('TTEE ' + e); } }"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (var vyzytm = 0; vyzytm < 8; ++vyzytm) { let d =  /x/g ;b; } "");}catch(ex){}
try{eval(""\n"");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (suezir = 0; suezir < 8; yield \""\\u4DB\"", ++suezir) { print(x); } "");}catch(ex){}
try{eval(""\""use strict\""; print(window);\nprint(x);\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""470"");
try{eval(""\""use strict\""; print(x);\nprint([this.__defineGetter__(\""w\"", Object.isSealed) for each (window in c) for each (\u0009get in [[1],  '\\0' ])]);\n"");}catch(ex){}
try{eval(""print((4277));"");}catch(ex){}
try{eval(""\""use strict\""; print(([x for each (z in x) if (false)]));print(x.y = wrap);"");}catch(ex){}
try{eval(""let (x) { yield; }"");}catch(ex){}
try{eval(""for(let w in ((JSON.stringify)(let (d =  '' ) \""\\uD1D\"")))(\""\\u599\"");"");}catch(ex){}
try{eval(""print(new Namespace(16777216, x));\nprint(x);\n"");}catch(ex){}
try{eval(""y = d;"");}catch(ex){}
try{eval(""{print(Number(({functional: (new e)}), ((function ([y]) { })() = 2030036074))); }"");}catch(ex){}
try{eval(""/*oLoop*/for (var nhdbpq = 0; nhdbpq < 5 && (4277); ++nhdbpq) { throw Math; } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""480"");
try{eval(""print(([]) = (x ?  /x/  :  /x/ ));const z = x;"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; print((#1=({x: (0.8679590467290769 >= length)})));"");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (let lxnixp = 0; lxnixp < 4; ++lxnixp) { /*oLoop*/for (let rcyfsc = 0; rcyfsc < 12; ++rcyfsc) { print(y); }  } "");}catch(ex){}
try{eval(""with({a: \u000cthis\n}){gc()print(x); }"");}catch(ex){}
try{eval(""\""use strict\""; print(x)\nyield;function d(this.x, c){/*jjj*/}print(x);\ndo {return; } while((this) && 0);{}\n"");}catch(ex){}
try{eval(""function get(){/*jjj*/}562949953421311;"");}catch(ex){}
try{eval(""gc()\nprint(x);\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""490"");
try{eval(""/*oLoop*/for (let kssqbj = 0, functional, x = ({a2:z2}); kssqbj < 6; ++kssqbj) { print(x); } "");}catch(ex){}
try{eval(""print((Object.keys()));return  /x/ ;"");}catch(ex){}
try{eval(""let e = Function, x.y = +null, hhefkb, \u3056 = this, {d: z} = x, y, dbjkpr;{print(x);\nObject.getOwnPropertyNames\n }"");}catch(ex){}
try{eval(""print(undefined /= ((eval) = undefined in \""\\u88F1E\""));"");}catch(ex){}
try{eval(""(function ([y]) { }.x = eval);"");}catch(ex){}
try{eval(""/*tLoop*/for each (let c in [function(){}, function(){},  \""use strict\"" , new String('q'), new Boolean(true), function(){}, new String('q'),  \""use strict\"" , function(){}, new String('q'), function(){}, new Boolean(true), new String('q'), new Boolean(true), new Boolean(true), function(){}, new String('q'), new Boolean(true), new String('q'), new Boolean(true), new String('q'), function(){}, new Boolean(true),  \""use strict\"" , function(){}, new Boolean(true), new String('q'), new String('q'), new Boolean(true), function(){}, new String('q')]) { print(window); }"");}catch(ex){}
try{eval(""2609062375.6648645;print(-4294967297 ? [] :  \""\"" );"");}catch(ex){}
try{eval(""print(false);\nreturn;\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""500"");
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""/*vLoop*/for (var hbeuub = 0; hbeuub < 2; ++hbeuub) { let z = hbeuub; \""\\uA94\""; } "");}catch(ex){}
try{eval(""(eval(\""print(false.__defineGetter__(\\\""x\\\"", WebGLIntArray));\""));\nprint(x);\n"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""{\""\\u89\""; }"");}catch(ex){}
try{eval(""\""use strict\""; break ;x = x;"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""\""use strict\""; (x);"");}catch(ex){}
try{eval(""/*vLoop*/for (let nhhgru = 0; nhhgru < 3; ++nhhgru) { var w = nhhgru; L:do {print((w = \""\\uA6133\""));/*iii*/yield 0;/*hhh*/function dtbutg(w, x){(32);} } while((w) && 0); } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""510"");
try{eval(""\""use strict\""; (3.throw(length));function NaN(){/*jjj*/} for each (x in \""\\u4534\""++) for each (x in [new Number(1), new Number(1), new Boolean(false), new Boolean(false), new Boolean(false), new Number(1), new Boolean(false), new Boolean(false), new Boolean(false), new Number(1), new Number(1), new Number(1), new Number(1), new Boolean(false), new Boolean(false), new Number(1)])"");}catch(ex){}
try{eval(""\""use strict\""; cbfjeq();/*hhh*/function cbfjeq(NaN, []){gczeal(0);}"");}catch(ex){}
try{eval(""L: {print((window &  ''  -= x.\u3056)); }"");}catch(ex){}
try{eval(""{print(b = -67108863);; }const w = (this.__defineSetter__(//h\n\""c\"", [x]));"");}catch(ex){}
try{eval(""\""use strict\""; return;\nx;\n"");}catch(ex){}
try{eval(""\""use strict\""; with(#1=[#1#]){print( /x/ ); }\nif(eval(\""d\"", this)) print(x);\n"");}catch(ex){}
try{eval(""let a = (new Array(281474976710655)), x = eval(\""x\"", ( ''  -=  \""\"" )), x = x, x = (Math.pow(function ([y]) { }, 2308089634.415196)), bblvsj;print(x);"");}catch(ex){}
try{eval(""(({ get d(d, c) { ; }  }));"");}catch(ex){}
try{eval(""print(x)\nL: {print(c);let c = window;-0; }\n"");}catch(ex){}
try{eval(""/*tLoop*/for each (let w in [(void 0), eval, {x:3}, {x:3}, {x:3}, eval, eval, eval, eval, {x:3}, {x:3}, eval, (void 0), eval, eval, eval, eval, (void 0), {x:3}, (void 0), eval, {x:3}, (void 0), (void 0), eval, (void 0)]) { print(x); }"");}catch(ex){}
try{eval(""/*bLoop*/for (let lxrirn = 0, x; lxrirn < 10; ++lxrirn) { if (lxrirn % 8 == 5) { print(x); } else { ( \""\"" ); }  } "");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""return x;x = x;"");}catch(ex){}
try{eval(""\""use strict\""; const e, y = \""\\uA045F8\""++, eval, x, NaN = (window === undefined);print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""530"");
try{eval(""(new (a.__parent__ =  /x/g )());"");}catch(ex){}
try{eval(""x;(x = Proxy.create((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: new Function, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: undefined, has: undefined, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; })(-3265207004.803368), this) += [15,16,17,18].filter(x, -0))\n"");}catch(ex){}
try{eval(""\""use strict\""; const x = x, vyxpuv, d = x, NaN =  '' , c, x, a, tvgfrv, e, z;print(x);"");}catch(ex){}
try{eval(""{print(x);(function shapeyConstructor(nndmcm){nndmcm.z = function(){};if (nndmcm) for (var ytqsqxkea in nndmcm) { }return nndmcm; }(let (b) ({a1:1}), undefined)); }"");}catch(ex){}
try{eval(""\""use strict\""; ((({x: -0})));function z(){/*jjj*/}/*tLoop*/for each (let c in [new Number(1.5), x, new Number(1.5), null, (new (JSON.stringify)(\""\\u63E450\"", \""\\uC3B6E8\"")), null, new Number(1.5), (new (JSON.stringify)(\""\\u63E450\"", \""\\uC3B6E8\"")), x, (new (JSON.stringify)(\""\\u63E450\"", \""\\uC3B6E8\"")), x, null, new Number(1.5), null, x, new Number(1.5), null, new Number(1.5), null, x, (new (JSON.stringify)(\""\\u63E450\"", \""\\uC3B6E8\""))]) { (function shapeyConstructor(yehbbg){\""use strict\""; if (yehbbg) yehbbg.c = #3={a:#3#};for (var ytqkjswhb in yehbbg) { }return yehbbg; }); }"");}catch(ex){}
try{eval(""\""use strict\""; print((4277));function d(functional){/*jjj*/}throw x;"");}catch(ex){}
try{eval(""[[1]];"");}catch(ex){}
try{eval(""if(((x =  /x/g )).throw(x)) { if ((p={}, (p.z = (\""\\u33D2\"".eval(this)))()))  else { \""\""  }}"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""540"");
try{eval(""\""use strict\""; M:switch(x) { case 4: break;  }"");}catch(ex){}
try{eval(""\""use strict\""; L:with( /x/g )return;var c = x;"");}catch(ex){}
try{eval(""print(x);\nyield;\n"");}catch(ex){}
try{eval(""x = z;let(x) ((function(){w = w;})());"");}catch(ex){}
try{eval(""x = linkedList(x, 3888);"");}catch(ex){}
try{eval(""if( '' ) print(-65535); else  if (true) {print( '' );{} } else {x; }"");}catch(ex){}
try{eval(""\""use strict\""; M:do print(x); while((let (a) null) && 0);"");}catch(ex){}
try{eval(""\""use strict\""; {print(<bbb xmlns:ccc=\""(a.x::w) = undefined\""><ccc:eee></ccc:eee></bbb>); }const x =  /x/g ;"");}catch(ex){}
try{eval(""var a, ydsatx;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""550"");
try{eval(""print(x);"");}catch(ex){}
try{eval(""print(x);let x = d;"");}catch(ex){}
try{eval(""\""use strict\""; let (b) b;"");}catch(ex){}
try{eval(""print(x);\n(x);\n"");}catch(ex){}
try{eval(""with({}) yield (4277);"");}catch(ex){}
try{eval(""print(\""\\u1D429\"");\n(false);\nfunction window(){/*jjj*/}(-2047);\nprint(x);\n"");}catch(ex){}
try{eval(""default xml namespace  = new XPCSafeJSObjectWrapper(x);var vdpvqy = new ArrayBuffer(0); var vdpvqy_0 = new Uint32Array(vdpvqy); print(window);"");}catch(ex){}
try{eval(""\""use strict\""; var {} = (x)(), w = (x.x::c), c = (4277);print((x = window));"");}catch(ex){}
try{eval(""/*oLoop*/for (let vzabhe = 0; vzabhe < 11; ++vzabhe) { print( /x/ ); } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""560"");
try{eval(""default xml namespace  = \""\\u014B\"".x::callee = ({a1:1});"");}catch(ex){}
try{eval(""\""use strict\""; print(this ? this :  '' );"");}catch(ex){}
try{eval(""\""use strict\""; x;"");}catch(ex){}
try{eval(""\""use strict\""; /*vLoop*/for (var kidztk = 0; kidztk < 2; ++kidztk) { var e = kidztk; var dfqxpw = new ArrayBuffer(2); var dfqxpw_0 = new Uint16Array(dfqxpw); dfqxpw_0[0] = 72057594037927940; var dfqxpw_1 = new Int16Array(dfqxpw); print(dfqxpw_1[0]); var dfqxpw_2 = new Int32Array(dfqxpw); dfqxpw_2[0] = -0; (\""\\uA\"");print(x);(x)\n } "");}catch(ex){}
try{eval(""( '' );throw  \""\"" ;"");}catch(ex){}
try{eval(""x = linkedList(x, 253);"");}catch(ex){}
try{eval(""let (ropank, x = y, x, b, yffsld) { M:for(var y = null in ((y = \""\\uC\"")).ignoreProcessingInstructions = decodeURIComponent) {print(window); } }"");}catch(ex){}
try{eval(""with((let (w = ( /x/ )(-4398046511105), c =  \""\"" , vojlnd, upqmze, cmhirc, functional, gkwxjr) x) ?  ''  : ((yield e-= /x/ ))){/*bLoop*/for (var vqkxvs = 0, yigszb; vqkxvs < 5; ++vqkxvs) { if (vqkxvs % 10 == 1) { (({})); } else { yield {}; }  } print(x);(\""\\uD6C\""); }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""570"");
try{eval(""print(x);\nvar c = (delete x.a), NaN = x, window = x, eval, myrjms, hehbbe, NaN, xamdcx, qxolqt;var xrsdvq = new ArrayBuffer(0); var xrsdvq_0 = new Uint32Array(xrsdvq); xrsdvq_0[0] = 8388607; return  '' ;\n"");}catch(ex){}
try{eval(""\""use strict\""; x = a;"");}catch(ex){}
try{eval(""/*vLoop*/for (var jvvwkm = 0; jvvwkm < 4 && (4277); ++jvvwkm) { let a = jvvwkm; var hgsosa = new ArrayBuffer(2); var hgsosa_0 = new Uint16Array(hgsosa); (undefined); } "");}catch(ex){}
try{eval(""\""use strict\""; switch([z1].__defineSetter__(\""x\"", Object.preventExtensions)) { default: let e = \""\\u614\"", \u3056 = NaN, \u3056 = Math.asin(true), \u3056 = -268435457, z, gdtpcm, qbbrgt;\nthrow false;\n }print(x++);print(x);"");}catch(ex){}
try{eval(""let (x) { (this); }\nx;\n"");}catch(ex){}
try{eval(""print((let (w = window) null));\nreturn \""\\uF\"";\n"");}catch(ex){}
try{eval(""/*bLoop*/for (var srxgke = 0; srxgke < 6; ++srxgke) { if (srxgke % 9 == 3) { {#1=[#1#];print(0); } } else { \u000d '' ;\nprint(x);\n }  } "");}catch(ex){}
try{eval(""\""use strict\""; (x);(eval);[,,z1];"");}catch(ex){}
try{eval(""var kjhvza = new ArrayBuffer(8); var kjhvza_0 = new Int8Array(kjhvza); var kjhvza_1 = new Float32Array(kjhvza); var grvnmh = new ArrayBuffer(16); var grvnmh_0 = new Uint8ClampedArray(grvnmh); grvnmh_0[0] = -0.8384507859746477; break M;print(this =  /x/ );"");}catch(ex){}
try{eval(""let (d) { d; }"");}catch(ex){}
try{eval(""\""use strict\""; print(x);\n\""\\u8A7919\"";\n"");}catch(ex){}
try{eval(""\""use strict\""; 0.8982889064828228;\ngc()\n"");}catch(ex){}
try{eval(""let (functional) { yield z; }"");}catch(ex){}
try{eval(""/*tLoop*/for each (let a in [0x20000000, [], function(){}, 0x20000000, function(){}, function(){}, 0x20000000, a.x getter= Math.min, new String(''), new String(''), new String(''), new String(''), 0x20000000, 0x20000000, new String(''), [], 0x20000000, new String(''), 0x20000000, a.x getter= Math.min, a.x getter= Math.min, [], a.x getter= Math.min, function(){}, function(){}, a.x getter= Math.min, 0x20000000, function(){}]) { print('fafafa'.replace(/a/g, \""\\uE8\"")); }"");}catch(ex){}
try{eval(""eval(\""print((\\\""\\\\u419\\\""(\\\""\\\\uFD\\\"", false)));\"");"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""590"");
try{eval(""print(x);function c([, ], c){/*jjj*/}print(x);print( \""\"" );function x(e, x){/*jjj*/} /x/g ;\nthis;\n"");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (var rlhcfe = 0; rlhcfe < 24; ++rlhcfe) { if (rlhcfe % 11 == 1) { print(x); } else { print(#1#); }  } "");}catch(ex){}
try{eval(""w = x;"");}catch(ex){}
try{eval(""let (e) { print(x); }"");}catch(ex){}
try{eval(""/*vLoop*/for (ipurin = 0; ipurin < 8; ++ipurin) { e = ipurin; print((\n[\""\\u70\"" if ([z1])])); } ((Math.max(0, ( /x/g  >>>=  /x/g ))));"");}catch(ex){}
try{eval(""for(let b = window in [,,]) { }"");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (tteqip = 0; tteqip < 24; ++tteqip) { if (tteqip % 9 == 0) { print(\""\\u7\""); } else { function (e) {  \""\"" ; }  }  } "");}catch(ex){}
try{eval(""(\""\\u3603\"");print(x);"");}catch(ex){}
try{eval(""\""use strict\""; throw StopIteration;this.zzz.zzz;"");}catch(ex){}
try{eval(""default xml namespace  = x;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""600"");
try{eval(""\""use strict\""; print(\""\\u7\"");\nreturn  /x/ ;\n"");}catch(ex){}
try{eval(""const y, qfujvl, x = this, adolky, x;((window));"");}catch(ex){}
try{eval(""((Object.isSealed([], try { print(x); } catch(e) { ; } )));"");}catch(ex){}
try{eval(""return;(-0.6588293392654448);"");}catch(ex){}
try{eval(""print(x);\""\\u9E86D\"";"");}catch(ex){}
try{eval(""/*hhh*/function zgftev(){/*vLoop*/for (let rjrccj = 0; rjrccj < 5; delete d.d, ++rjrccj) { let x = rjrccj; this %=  \""\"" ; } }/*iii*/break ;\nprint(zgftev);\n"");}catch(ex){}
try{eval(""\""use strict\""; L:with({a: (this.__defineSetter__(\""b\"", Array.reduce))})(0.6947080464415192);"");}catch(ex){}
try{eval(""M:if((4277)) {({a:  '' , x:  ''  });print((~x)); else  if (x) print(x); else {this; '' ;print(x); }"");}catch(ex){}
try{eval(""/*iii*/print( /x/g );/*hhh*/function mimibc(x){/*bLoop*/for (let xxvdtp = 0; xxvdtp < 21; ++xxvdtp) { if (xxvdtp % 6 == 4) { (0.04448750606557356); } else { /*iii*/print( /x/g );/*hhh*/function vwioae(d){print(x);} }  } }"");}catch(ex){}
try{eval(""\""use strict\""; with(\""\\uA\"")yield;"");}catch(ex){}
try{eval(""\""use strict\""; var zrvbyg = new ArrayBuffer(0); var zrvbyg_0 = new Uint16Array(zrvbyg);  \""\"" ;"");}catch(ex){}
try{eval(""/*\n*/throw Math;print(Math);"");}catch(ex){}
try{eval(""print(x);function functional(x, NaN){/*jjj*/}print((4277));"");}catch(ex){}
try{eval(""function shapeyConstructor(zebrsr){\""use strict\""; zebrsr.c = x;return zebrsr; }function x(eval, window){/*jjj*/}print( /x/g );\nreturn  /x/g ;\nprint( /x/ );\n\n"");}catch(ex){}
try{eval(""gczeal(0);\nM:for(a in ((Object.isSealed)()))print(a);\n"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""620"");
try{eval(""w = x;print(w);"");}catch(ex){}
try{eval(""/*tLoop*/for each (let x in [new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), (0/0), new Number(1.5), (0/0), new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), (0/0), new Number(1.5), new Number(1.5), (0/0), (0/0), (0/0), (0/0), new Number(1.5), new Number(1.5), (0/0)]) { /*tLoop*/for each (let a in [undefined, 1, 1,  /x/g , 1, function(){}, 1, undefined, undefined,  /x/g ,  /x/g , function(){},  /x/g ,  '\\0' , undefined, function(){}, undefined, undefined,  '\\0' ,  /x/g , 1, 1,  /x/g , 1,  '\\0' , 1, undefined, function(){},  '\\0' , 1, 1]) { (undefined); } }"");}catch(ex){}
try{eval(""with({}) { (this); } throw StopIteration;"");}catch(ex){}
try{eval(""/*vLoop*/for (var ryhfpe = 0; ryhfpe < 7; ++ryhfpe) { y = ryhfpe; ; } "");}catch(ex){}
try{eval(""/*tLoop*/for each (let z in [new Boolean(true), new Boolean(true), (1/0), arguments.callee, (1/0), (1/0), new Boolean(true), arguments.callee, arguments.callee, new Boolean(true), (1/0), arguments.callee, new Boolean(true), (1/0), arguments.callee, new Boolean(true), new Boolean(true), new Boolean(true), arguments.callee, new Boolean(true), arguments.callee, arguments.callee, new Boolean(true), (1/0)]) { ( /x/g ); }"");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (oycysr = 0; oycysr < 9; ++oycysr) { return x; } "");}catch(ex){}
try{eval(""\""use strict\""; /*vLoop*/for (let rowjmq = 0, nbediw; rowjmq < 7; ++rowjmq, (x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: undefined, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: undefined, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: undefined, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; })( '' .__noSuchMethod__ = this), function(y) { return (4277) }))) { var z = rowjmq; print(z); } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""630"");
try{eval(""\""use strict\""; const d = c.x::y;return;"");}catch(ex){}
try{eval(""/*tLoop*/for each (let a in [undefined, ['z'], ((x)([11,12,13,14].sort, w)), ((x)([11,12,13,14].sort, w)), undefined, ((x)([11,12,13,14].sort, w)), ['z'], undefined, ['z'], true, undefined, ((x)([11,12,13,14].sort, w)), true, true]) { throw  \""\"" ; }"");}catch(ex){}
try{eval(""\""use strict\""; /*hhh*/function bgcwyh(x, d){;\nprint(x);\n}bgcwyh((4277), (w)-=[,,z1]);"");}catch(ex){}
try{eval(""\""use strict\""; (#2= /x/g .c);"");}catch(ex){}
try{eval(""\""use strict\""; throw x = true;\nprint(x);\n"");}catch(ex){}
try{eval(""var rqhbga = new ArrayBuffer(4); var rqhbga_0 = new Int8Array(rqhbga); rqhbga_0[0] = -0.4004922856963009; print(rqhbga_0);"");}catch(ex){}
try{eval(""\""use strict\""; var sbbtfb = new ArrayBuffer(4); var sbbtfb_0 = new Int32Array(sbbtfb); var sbbtfb_1 = new Float64Array(sbbtfb); sbbtfb_1[0] = 0.4448938080664718; var sbbtfb_2 = new WebGLFloatArray(sbbtfb); print(sbbtfb_2[0]); var sbbtfb_3 = new Uint8Array(sbbtfb); sbbtfb_3[0] = 4398046511103; var sbbtfb_4 = new Int8Array(sbbtfb); sbbtfb_4[0] = 0x5a827999; var sbbtfb_5 = new WebGLIntArray(sbbtfb); sbbtfb_5[0] = 0; var sbbtfb_6 = new Uint8Array(sbbtfb); window;print((4277));;throw  '' ;print(NaN.d getter= Proxy.createFunction);(#1#);"");}catch(ex){}
try{eval(""\""use strict\""; /*iii*/gczeal(0);gc()/*hhh*/function unckpz(y){window;}"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""640"");
try{eval(""var ufcsmq = new ArrayBuffer(8); var ufcsmq_0 = new Uint8Array(ufcsmq); ufcsmq_0[0] = -4611686018427388000; var ufcsmq_1 = new Uint16Array(ufcsmq); print(ufcsmq_1[0]); ufcsmq_1[0] = -0; var ufcsmq_2 = new Float64Array(ufcsmq); var ufcsmq_3 = new WebGLIntArray(ufcsmq); ufcsmq_3[0] = 670022898.3109457; var ufcsmq_4 = new Uint8ClampedArray(ufcsmq); print(ufcsmq_4[0]); ufcsmq_4[0] = 1.2e3; Math.atan(-765971392.1672316);if(ufcsmq_0[0]) {print(ufcsmq_1[4]); } else  if ([11,12,13,14].map) {} else print(ufcsmq_3[0]);"");}catch(ex){}
try{eval(""M:do print(x);function {y}(x){/*jjj*/}gc()print(x); while((x) && 0);"");}catch(ex){}
try{eval(""while(( \""\"" ) && 0)yield;\nprint((({x: ({}) })));\n"");}catch(ex){}
try{eval(""/*oLoop*/for (var dqiise = 0; dqiise < 6; ++dqiise) { ( \""\"" ); } "");}catch(ex){}
try{eval(""\""use strict\""; return;"");}catch(ex){}
try{eval(""\""use strict\""; (d);\n{}\n"");}catch(ex){}
try{eval(""idxrow(\""\\uB3C464\"");/*hhh*/function idxrow(a, x){false;}"");}catch(ex){}
try{eval(""{M:while((true.E) && 0){\u0009([[]]);//h\n/*hhh*/function ftairj(window, y){\u000dyield  \""\"" ;}ftairj(); }\""\\uD02A\"";{} }"");}catch(ex){}
try{eval(""\""use strict\""; c =  \""\"" ;Function.prototype.bind"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""650"");
try{eval(""\""use strict\""; /*tLoop*/for each (let e in [x, x, (void 0), (void 0), (void 0), (void 0), (void 0), (void 0), x, x, (void 0), x, (void 0), x, (void 0), x, (void 0), x, x, (void 0), (void 0), x, (void 0), x, (void 0), x, x, (void 0), x, x, (void 0), x, (void 0), x, (void 0), (void 0)]) { ( '' ); }"");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (var wahzra = 0, ({ set let \u3056 () { e; }  }); wahzra < 2; (4277), ++wahzra) { print(x); } "");}catch(ex){}
try{eval(""function shapeyConstructor(mbwxnh){{ return; } this.e = d;delete this.b;this.a =  \""\"" ;this.e =  /x/ ;Object.freeze(this);Object.defineProperty(this, \""\\u3056\"", ({enumerable: null}));return this; }/*tLoopC*/for each (let w in [this, this, NaN, x, x, NaN, x, this, x, this, x, x, this, [1], NaN, [1], NaN, x, NaN, this, [1], [1], [1]]) { try{let mucozf = shapeyConstructor(w); print('EETT'); (true);}catch(e){print('TTEE ' + e); } }"");}catch(ex){}
try{eval(""with({}) throw x;with({}) { x = b; } "");}catch(ex){}
try{eval(""(#1=[#1#]);function x(e){/*jjj*/}\u000cprint( /x/g );"");}catch(ex){}
try{eval(""\""use strict\""; var fnagno = new ArrayBuffer(3); var fnagno_0 = new Uint32Array(fnagno); fnagno_0[0] = 70368744177665; var fnagno_1 = new Int32Array(fnagno); var fnagno_2 = new Uint8ClampedArray(fnagno); print(fnagno_2[0]); fnagno_2[0] = -129; var fnagno_3 = new Uint8ClampedArray(fnagno); print(fnagno_3[0]); var fnagno_4 = new Int16Array(fnagno); print(fnagno_4[0]); fnagno_4[0] = 0.4829348658732904; print(fnagno_3[2]);[];yield length;\nprint(fnagno_2);\ngczeal(0);{}"");}catch(ex){}
try{eval(""/*bLoop*/for (var qzrton = 0; qzrton < 24; ++qzrton) { if (qzrton % 5 == 3) { print(x); } else { print(x); }  } "");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (zqhzxe = 0; zqhzxe < 8; ++zqhzxe) {  '' \n } "");}catch(ex){}
try{eval(""M:if(window) {Math; }"");}catch(ex){}
try{eval(""{print(new \""\\uBEE\""); }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""660"");
try{eval(""\""use strict\""; function shapeyConstructor(rpyiej){Object.seal(this);return this; }/*tLoopC*/for each (let z in [ /x/ ,  /x/ , function(){}, function(){},  /x/ , function(){}, function(){}, new Array(-0),  /x/ , new Array(-0), null, function(){},  /x/ , function(){}, new Array(-0), new Array(-0),  /x/ , function(){}, null, new Array(-0), new Array(-0), new Array(-0), new Array(-0), new Array(-0), null,  /x/ , new Array(-0), function(){}, function(){}, function(){}]) { try{let xvzqte = new shapeyConstructor(z); print('EETT'); yield x;\nprint(xvzqte);\n}catch(e){print('TTEE ' + e); } }"");}catch(ex){}
try{eval(""for(let c in [e, e, [(void 0)], [(void 0)], (0/0), new ( /x/ )(), new ( /x/ )(), new ( /x/ )(), [(void 0)], [(void 0)], new ( /x/ )(), e, e, (0/0), [(void 0)], [(void 0)], new ( /x/ )(), new ( /x/ )(), e, e, e, [(void 0)], new ( /x/ )(), e, (0/0), new ( /x/ )(), new ( /x/ )(), new ( /x/ )(), [(void 0)]]) with({}) { ('fafafa'.replace(/a/g, Object.keys)); } "");}catch(ex){}
try{eval(""/*oLoop*/for (let kbwnbs = 0; kbwnbs < 0; ++kbwnbs) { /*iii*/ /x/ ;/*hhh*/function phayiz(){print(x);} } "");}catch(ex){}
try{eval(""(window);function z(NaN){/*jjj*/}yield \""\\u682B6\"";"");}catch(ex){}
try{eval(""print();\nprint(x);\n"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; print((x = Proxy.createFunction((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: undefined, defineProperty: undefined, getOwnPropertyNames: (let (e=eval) e), fix: x, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: Array.isArray, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; })(\""\\uD4C3A\""), \""\\u406\"")));\n/*bLoop*/for (var bxyrqv = 0; bxyrqv < 18; ++bxyrqv) { if (bxyrqv % 4 == 2) { print(x); } else { print(this); }  } \n"");}catch(ex){}
try{eval(""\""use strict\""; with(this.length =  /x/ )break M;"");}catch(ex){}
try{eval(""{let (x = (new ArrayBuffer()), pqthyb, x, x) { print(x); } }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""670"");
try{eval(""var sqznkb = new ArrayBuffer(12); var sqznkb_0 = new Uint16Array(sqznkb); sqznkb_0[0] = -3899678464; var sqznkb_1 = new Float32Array(sqznkb); var sqznkb_2 = new Int8Array(sqznkb); print(sqznkb_2[0]); sqznkb_2[0] = 0; var sqznkb_3 = new Float64Array(sqznkb); sqznkb_3[0] = -0.6841772237261663; print(sqznkb_1[3]);(((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor'); var desc = Object.getOwnPropertyDescriptor(x); desc.configurable = true; return desc; }, getPropertyDescriptor: undefined, defineProperty: function(name, desc) { var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty'); Object.defineProperty(x, name, desc); }, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return Object.getOwnPropertyNames(x); }, fix: XPCSafeJSObjectWrapper, has: encodeURI, hasOwn: undefined, get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: undefined, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { for (var name in x) { yield name; } })(); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); var result = []; for (var name in x) { result.push(name); }; return result; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return Object.keys(x); }, }; }))(sqznkb_0[0], sqznkb_3[3]));\n(x *=  );\n"");}catch(ex){}
try{eval(""\""use strict\""; (\n(4277));"");}catch(ex){}
try{eval(""\""use strict\""; return [,,].functional::match;yield  /x/g ;"");}catch(ex){}
try{eval(""var zoxujw = new ArrayBuffer(8); var zoxujw_0 = new Uint8ClampedArray(zoxujw); print(zoxujw_0[0]); zoxujw_0[0] = (0x50505050 >> 1); var zoxujw_1 = new Int8Array(zoxujw); zoxujw_1[0] = -3/0; var zoxujw_2 = new Int32Array(zoxujw); print(zoxujw_2[0]); {print( /x/ );continue L; }throw  /x/g ;( '' );function functional(zoxujw_0[0]){/*jjj*/}print(#3={a:#3#});print(zoxujw_2);"");}catch(ex){}
try{eval(""\""use strict\""; (((this)));"");}catch(ex){}
try{eval(""\""use strict\""; print((4277));print((4277));"");}catch(ex){}
try{eval(""print(x);\n( /x/ );\n"");}catch(ex){}
try{eval(""\""use strict\""; var a = ((uneval((new XPCNativeWrapper(this.__defineGetter__(\""x\"", gc()))))));break L;"");}catch(ex){}
try{eval(""let (y = (d = Proxy.createFunction((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor'); var desc = Object.getOwnPropertyDescriptor(x); desc.configurable = true; return desc; }, getPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor'); var desc = Object.getPropertyDescriptor(x); desc.configurable = true; return desc; }, defineProperty: undefined, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return Object.getOwnPropertyNames(x); }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); if (Object.isFrozen(x)) { return Object.getOwnProperties(x); } }, has: undefined, hasOwn: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return Object.prototype.hasOwnProperty.call(x, name); }, get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: function(receiver, name, val) { var yum = 'PCAL'; dumpln(yum + 'LED: set'); x[name] = val; return true; }, iterate: undefined, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); var result = []; for (var name in x) { result.push(name); }; return result; }, keys: undefined, }; })(this), Proxy.isTrapping, Object.getPrototypeOf)).prototype, adrdlu) { for(let b in [{}, new Boolean(true), (-1/0),  /x/ ,  /x/ ,  /x/ ,  /x/ , new Boolean(true), new Boolean(true), {}, {},  /x/ ,  /x/ ,  /x/ , arguments.callee, arguments.callee,  /x/ ])  '' ; }"");}catch(ex){}
try{eval(""if((4277)) { if (((new ( '' )()).callee = ((Uint32Array)(this)))) {}} else {print(x); }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""680"");
try{eval(""let e = ~({functional setter: true, d: [,,z1] });var nsmbxm = new ArrayBuffer(0); var nsmbxm_0 = new Float32Array(nsmbxm); print(this);"");}catch(ex){}
try{eval(""\""use strict\""; var x = (--(window %=  /x/g .arguments)), sgqqvg;var ghhtcm = new ArrayBuffer(4); var ghhtcm_0 = new Uint16Array(ghhtcm); ghhtcm_0[0] = 8796093022208; var ghhtcm_1 = new Uint8Array(ghhtcm); print(this.__defineSetter__(\""window\"",  /x/ ));print(ghhtcm_0[0]);print(new XPCNativeWrapper(((uneval(\""\\u6ED\"")))));print(ghhtcm_1);function(q) { return q; }"");}catch(ex){}
try{eval(""\""use strict\""; 1073741825;with({}) throw true;"");}catch(ex){}
try{eval(""print(x);\nprint(false);\n"");}catch(ex){}
try{eval("";"");}catch(ex){}
try{eval(""with({a: x}){const fzldgl, wfwrty, dldrzk, zvmycq, get;\u000c(this); }"");}catch(ex){}
try{eval(""with({c: x.__defineSetter__(\""d\"", Object.create)}){print(window); }"");}catch(ex){}
try{eval(""/*hhh*/function eazkll(){return;}/*iii*/([15,16,17,18].map((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor'); var desc = Object.getOwnPropertyDescriptor(x); desc.configurable = true; return desc; }, getPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor'); var desc = Object.getPropertyDescriptor(x); desc.configurable = true; return desc; }, defineProperty: function(name, desc) { var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty'); Object.defineProperty(x, name, desc); }, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return Object.getOwnPropertyNames(x); }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); if (Object.isFrozen(x)) { return Object.getOwnProperties(x); } }, has: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return name in x; }, hasOwn: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return Object.prototype.hasOwnProperty.call(x, name); }, get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: Object.seal, iterate: /*wrap3*/(function(){ var nmuxsf = #1#; (34359738369)(); }), enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); var result = []; for (var name in x) { result.push(name); }; return result; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return Object.keys(x); }, }; }), ( /x/g )(this)));"");}catch(ex){}
try{eval(""var bvuwmb, pbfxiv, x =  /* Comment */this;print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""690"");
try{eval(""var xqvday, NaN = \""\\u3C41F2\"", crlego, iopwkv, exjsip, b;if(c) { if ( \""\"" ) print(x);} else print( /x/ );\nprint(x);\n"");}catch(ex){}
try{eval(""/*iii*/print(jrawwy);/*hhh*/function jrawwy(x){gc()}"");}catch(ex){}
try{eval(""print([]);function y(z, window){/*jjj*/}print(0);{}"");}catch(ex){}
try{eval(""/*tLoop*/for each (let d in [new Number(1.5),  /x/ , new Boolean(true), new Boolean(true), new Number(1.5)]) { print(d); }"");}catch(ex){}
try{eval(""L:for(c in ((new XPCNativeWrapper(eval(\""throw  /x/g ;\"")))( '' )))(false);"");}catch(ex){}
try{eval(""let (this, window = (4277), x = x, x = x['\u3056'] = (4277), eval = (Object.keys).call(z, )) { /*tLoop*/for each (let y in [[], [], [],  /x/ , [], [],  /x/ , (void 0), (void 0), [],  /x/ , (void 0), true, true, true, [],  /x/ , [], [], (void 0), true, (void 0),  /x/ ,  /x/ ,  /x/ , [],  /x/ , (void 0),  /x/ , (void 0),  /x/ , true, true, [], [], (void 0)]) { eval = window.wrappedJSObject = this, function::byteOffset = \""\\uCEA9\"", ojpdww;print(x); } }"");}catch(ex){}
try{eval(""\""use strict\""; return this;\nprint(x);\n"");}catch(ex){}
try{eval(""/*iii*/var syqbsg = new ArrayBuffer(8); var syqbsg_0 = new WebGLFloatArray(syqbsg); syqbsg_0[0] = -0; var syqbsg_1 = new WebGLFloatArray(syqbsg); syqbsg_1[0] = 8192; (window);window;/*hhh*/function fpzift(function::c, x){var pipakc = new ArrayBuffer(6); var pipakc_0 = new WebGLIntArray(pipakc); print(pipakc_0[0]); pipakc_0[0] = 4.; print(pipakc_0);\ncontinue ;\n//h\nprint(x);\n\n}"");}catch(ex){}
try{eval(""print(x);gc()"");}catch(ex){}
try{eval(""\u0009const y = \""\\u35\"";with((4277))print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""700"");
try{eval(""\""use strict\""; with({z: this.zzz.zzz}){var fsonxi, e, ksqqnu, z;print(x); }"");}catch(ex){}
try{eval(""with({x: (x-=this)}){print( \""\"" );print(undefined); }"");}catch(ex){}
try{eval(""\""use strict\""; return (4277);throw x;"");}catch(ex){}
try{eval(""print(let (w)  /x/g );\nprint(x);\n"");}catch(ex){}
try{eval(""let (d) { if(x) print(d); else  /x/ ; }"");}catch(ex){}
try{eval(""/*oLoop*/for (wouasx = 0; wouasx < 10; ++wouasx) { (this); } "");}catch(ex){}
try{eval(""\""use strict\""; [1]"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""710"");
try{eval(""/*oLoop*/for (pxaifb = 0, mnzgdc; pxaifb < 0; ++pxaifb) { M:with(x)print( /x/ ); } "");}catch(ex){}
try{eval(""\""use strict\""; print(x);return arguments;"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let w in [true, -134217727[({})],  '' ]) { print( /x/g ); }"");}catch(ex){}
try{eval(""\""use strict\""; var wxkpga = new ArrayBuffer(4); var wxkpga_0 = new Int8Array(wxkpga); print(wxkpga_0[0]); wxkpga_0[0] = 1e81; var wxkpga_1 = new Float64Array(wxkpga); wxkpga_1[0] = 0.16969324191302781; print([15,16,17,18].filter((function handlerFactory(x) {return {getOwnPropertyDescriptor: undefined, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: undefined, getOwnPropertyNames: undefined, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: QName, set: undefined, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; }), \""\\u9F50\""));print(wxkpga_0[0]);print(wxkpga);"");}catch(ex){}
try{eval(""((p={}, (p.z = y = Proxy.create((function handlerFactory(x) {return {getOwnPropertyDescriptor: Proxy.isTrapping, getPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor'); var desc = Object.getPropertyDescriptor(x); desc.configurable = true; return desc; }, defineProperty: undefined, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return Object.getOwnPropertyNames(x); }, fix: function(y) { return -1113914427 }, has: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return name in x; }, hasOwn: undefined, get: undefined, set: function(receiver, name, val) { var yum = 'PCAL'; dumpln(yum + 'LED: set'); x[name] = val; return true; }, iterate: function (w) { \""use strict\""; print( /x/ ); } , enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); var result = []; for (var name in x) { result.push(name); }; return result; }, keys: \""\\uB5AC3\"", }; })({}), this))()));"");}catch(ex){}
try{eval(""\""use strict\""; "");}catch(ex){}
try{eval(""/*oLoop*/for (vedvto = 0; vedvto < 9; ++vedvto) { \""use strict\""; (x); } "");}catch(ex){}
try{eval(""\""use strict\""; print(window);const b = ({}) %= -370914387.60893893;"");}catch(ex){}
try{eval(""gczeal(0);\nprint(-0);\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""720"");
try{eval(""x.searchfunction b(){/*jjj*/}const a, pkutnp, functional;print( /x/g );"");}catch(ex){}
try{eval(""\""use strict\""; w = yield x;print((4277));"");}catch(ex){}
try{eval(""\""use strict\""; print((4277));\nprint(x);\n"");}catch(ex){}
try{eval(""c = \u0009((((d = a--) for each (eval in [,,])))[(Object.getOwnPropertyNames.prototype)]);print([this].sort(wrap));"");}catch(ex){}
try{eval(""/*bLoop*/for (let zwtqoo = 0; zwtqoo < 5; ++zwtqoo) { if (zwtqoo % 2 == 0) { ([1,,]); } else { /*hhh*/function wrdouq(){print(x);}wrdouq(); }  } "");}catch(ex){}
try{eval(""if(z = undefined ? (-34359738367.__count__) : (\""\\u3D\"".throw(\""\\u31\""))) Math;\nvar jfuaym;yield;\n else  if (x) /*hhh*/function jztynr(z){{}}/*iii*/\""\\u1ED9C7\"";"");}catch(ex){}
try{eval(""\""use strict\""; print([15,16,17,18].map(encodeURIComponent,  /x/g ));"");}catch(ex){}
try{eval(""\""use strict\""; default xml namespace  = {};"");}catch(ex){}
try{eval(""print(#2=[x]);print(({a1:1}));\n /x/g ;\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""730"");
try{eval(""switch(eval(\""( \\\""\\\"" );\"")) { default: yield (x) = [[]];if(undefined) { if ( '' ) {print( /x/ ); } else print(x);}break;  }\n\u000cL: print(false);\n"");}catch(ex){}
try{eval(""(\""\\uDD137\"");\nprint(x);\n"");}catch(ex){}
try{eval(""\""use strict\""; print(x); /x/g .functional::normalize;"");}catch(ex){}
try{eval(""print(((function factorial_tail(lqiend, aijcrd) { ; if (lqiend == 0) { ; return aijcrd; } x = linkedList(x, 3010);; return factorial_tail(lqiend - 1, aijcrd * lqiend);  })(12, 1)));"");}catch(ex){}
try{eval(""/*tLoop*/for each (let e in [ \""use strict\"" ,  \""use strict\"" ,  \""use strict\"" ,  /x/g ,  /x/g ]) { (y); }"");}catch(ex){}
try{eval(""\""use strict\""; while((eval(\""while((x.x) && 0)(~(b , x));\"", x.e getter= decodeURI)) && 0){print(x)\nprint(849907611);\n(window);\n }"");}catch(ex){}
try{eval(""\""use strict\""; (new (function(q) { return q; }).bind());function a(w){/*jjj*/}print((4277));"");}catch(ex){}
try{eval(""const w =  /x/g ;print(x);"");}catch(ex){}
try{eval(""/*hhh*/function cjiryf(NaN){/*hhh*/function jxheti(w){(15);}jxheti( /x/ , x);}cjiryf(x for each (x in [ '\\0' , ({x:3}), ({x:3}), ({x:3}), ({x:3}),  '\\0' , ({x:3}), ({x:3}), ({x:3}),  '\\0' , ({x:3}), ({x:3}), ({x:3}),  '\\0' ,  '\\0' ,  '\\0' , ({x:3}), ({x:3}), ({x:3}),  '\\0' ,  '\\0' , ({x:3}),  '\\0' ,  '\\0' , ({x:3}),  '\\0' ,  '\\0' ,  '\\0' , ({x:3})]) if ([z1].__defineSetter__(\""x\"", new Function)));"");}catch(ex){}
try{eval(""\""use strict\""; print((x).call(x, ));"");}catch(ex){}
try{eval(""\""use strict\""; var x = (let (x =  /x/g ) window), x, ceudpe, d, deqice;gc()"");}catch(ex){}
try{eval(""var vewsei = new ArrayBuffer(8); var vewsei_0 = new Int32Array(vewsei); vewsei_0[0] = 2251799813685248; var vewsei_1 = new Int32Array(vewsei); vewsei_1[0] = -0; ;return false;print(vewsei_0[2]);"");}catch(ex){}
try{eval(""jjgqdg, e = x = Proxy.create((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: Object.create, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: this.toSource, }; })(a), \""\\uBD3A9\""), e;x = x = [z1,,];\""\\uBBF2D3\"";"");}catch(ex){}
try{eval(""yield let (eval, e, \u3056, x, mouiqr, x, b, w, \u3056) /*wrap1*/(function(){ print(2251799813685249);return /*wrap2*/(function(){ var hddebk = this; var recpmk = (function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: undefined, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: Proxy.createFunction, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; }); return recpmk;})()})();\nfor(let y in [1e4, 1e4, 1e4, {}, {}, 1e4, 1e4, {}, 1e4, {}, {}, 1e4, {}, 1e4, 1e4, 1e4, {}, 1e4, {}, 1e4, {}, 1e4, 1e4, 1e4, {}, {}, 1e4, {}, 1e4, 1e4]) (y);\n"");}catch(ex){}
try{eval(""\""use strict\""; options('strict');"");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (let uecpde = 0, \u3056; uecpde < 5; ++uecpde) { if (uecpde % 11 == 1) {  \""\"" ; } else { (undefined); }  } "");}catch(ex){}
try{eval(""print(x);print(z);"");}catch(ex){}
try{eval(""(this);function w(){/*jjj*/} /x/ ;\u0009let(w) { for(let b in [arguments, arguments, arguments]) return eval(\""yield [,,]\"", (Math.min(x, 0)));}"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""750"");
try{eval(""\""use strict\""; let (x, x, fruokn, z) { print('fafafa'.replace(/a/g, decodeURI).__defineGetter__(\""x\"", Proxy.createFunction)); }\u000c"");}catch(ex){}
try{eval(""let (w) { {}(\""\\u452D\""); }"");}catch(ex){}
try{eval(""/*vLoop*/for (rrsctm = 0, rjxqzj; rrsctm < 3; x, ++rrsctm) { var x = rrsctm; ; } "");}catch(ex){}
try{eval(""L:with(this.__defineSetter__(\""y\"", eval))c;window;"");}catch(ex){}
try{eval(""\""use strict\""; let ckyvbt, x, NaN, z, x;print(window);"");}catch(ex){}
try{eval(""let(ncnkpo, window::\u3056, c = (d =  '' ), x =  /x/ ) { x['x'] = c;}let(e) { let(x = e) { throw e;}}"");}catch(ex){}
try{eval(""/*hhh*/function mcycht\u000c(x){print( '' );}/*iii*/throw  /x/g ;"");}catch(ex){}
try{eval(""e;(window);"");}catch(ex){}
try{eval(""\""use strict\""; /*hhh*/function txwgid(z, [, ]){(#1=({x: [this for each (this in  \""\"" )]})).eval([window]);}txwgid(, x > window);"");}catch(ex){}
try{eval(""\""use strict\""; print(this);break ;"");}catch(ex){}
try{eval(""\""use strict\""; this.zzz.zzz;return Object.defineProperty(x, \""e\"", ({get: Proxy.create, set: decodeURI})).__defineGetter__(\""d\"", w);"");}catch(ex){}
try{eval(""\""use strict\""; ((eval(\""var jdpcxv = new ArrayBuffer(3); var jdpcxv_0 = new Int16Array(jdpcxv); print(jdpcxv_0[0]); jdpcxv_0[0] = -182779572; true;\"", (4277))));"");}catch(ex){}
try{eval(""{for(let b in [function(){}, function(){}, function(){}, x, x, function(){}, x, function(){}, function(){}, function(){}, function(){}, x, x, x])   = b;print(x);\n(function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: undefined, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: undefined, get: Object.isExtensible, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: undefined, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; })\nprint(x);\n\n }"");}catch(ex){}
try{eval(""print(x);for(var w = (({x: 524289})) in  '' ) print(w);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""770"");
try{eval(""buthdg(x, (new XPCSafeJSObjectWrapper((Math.min(3.141592653589793, -134217728))))\n);/*hhh*/function buthdg(){Object.freeze}"");}catch(ex){}
try{eval(""\""use strict\""; with(this){print(x);print(x); }print(x);"");}catch(ex){}
try{eval(""while(((((x)(NaN & z)) = (4277))) && 0)/*oLoop*/for (hyugaj = 0, 200941837.44683903; hyugaj < 5; ++hyugaj) { print(x); } \nlet (x) { (false * x); }\n"");}catch(ex){}
try{eval(""\""use strict\""; \""use strict\"";  /x/g ;"");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (let cohgvn = 0, [,],  /x/g ; cohgvn < 3; ++cohgvn) { window; } "");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let w in [NaN, new Boolean(true), new Boolean(true), new Boolean(true), NaN, NaN, (-1/0), NaN, (-1/0), NaN, NaN, (-1/0), NaN, NaN, NaN, (-1/0), NaN, NaN, new Boolean(true), NaN, (-1/0), new Boolean(true), (-1/0), NaN, new Boolean(true), NaN]) { print(w); }"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let b in [true,  /x/g ,  /x/g , true, true]) { print(x); }\n"");}catch(ex){}
try{eval(""let window, \u3056, qyxwxi, puuevg, zgxbgx, xytwpj;print(x);"");}catch(ex){}
try{eval(""/*vLoop*/for (let hszjmr = 0; hszjmr < 2; ++hszjmr) { e = hszjmr; (-310947227); } function x(d, functional){/*jjj*/}print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""780"");
try{eval(""\""use strict\""; print(b.function::length =  /x/ );print(x);"");}catch(ex){}
try{eval(""print((uneval(arguments)));this;"");}catch(ex){}
try{eval(""\""use strict\""; /*oLoop*/for (var kocase = 0; kocase < 8; ++kocase) { print(false); } "");}catch(ex){}
try{eval(""yield\n(b);/*bLoop*/for (kwomka = 0; kwomka < 21; false, ++kwomka) { if (kwomka % 9 == 3) {  /x/ ; } else { window; }  } "");}catch(ex){}
try{eval(""/*oLoop*/for (jblhil = 0; jblhil < 11; ++jblhil) { print(x); } "");}catch(ex){}
try{eval(""\""use strict\""; (this.zzz.zzz);/*\n*/"");}catch(ex){}
try{eval(""\""use strict\""; var d = x;yield \u000c \""\"" ;"");}catch(ex){}
try{eval(""( '' );"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""790"");
try{eval(""/*tLoop*/for each (let y in [NaN, NaN]) { let a, pekgjj, functional, y, cmwpbh, uryayd, c, gqfeam;( '' ); }"");}catch(ex){}
try{eval(""throw StopIteration;eval = eval;"");}catch(ex){}
try{eval(""gczeal(0);"");}catch(ex){}
try{eval(""switch( /x/ ()) { case  /x/ : print(x);case x: (this.__defineSetter__(\""z\"", encodeURI));default: break; case 6:  }"");}catch(ex){}
try{eval(""\""use strict\""; L: for  each(let c in new (Proxy.isTrapping)()) {print(x);return; /x/g ;\nprint(x);\n\nprint(c);\n }"");}catch(ex){}
try{eval(""wwrpuf(Math.sin(this.zzz.zzz));/*hhh*/function wwrpuf(z, {}){print(x);}"");}catch(ex){}
try{eval(""/*vLoop*/for (yvipta = 0; yvipta < 7; ++yvipta) { let z = yvipta; throw  /x/g ; } "");}catch(ex){}
try{eval(""true\nprint([[1]]);"");}catch(ex){}
try{eval(""/*oLoop*/for (let ivdrwq = 0; ivdrwq < 1; ++ivdrwq) {  /x/g \n; } "");}catch(ex){}
try{eval(""options('strict');"");}catch(ex){}
try{eval(""print(x);print(x);"");}catch(ex){}
try{eval(""/*hhh*/function hsljek(){/*vLoop*/for (var vfgzrf = 0; vfgzrf < 0; ++vfgzrf) { var y = vfgzrf; {} } }/*iii*/print(eval(\"" /x/g \""));"");}catch(ex){}
try{eval(""let (b) { \nprint(b);\n }"");}catch(ex){}
try{eval(""\""use strict\""; print(x);(\""\\u99809\"" in x);\n{var mepyjz = new ArrayBuffer(8); var mepyjz_0 = new Uint32Array(mepyjz); print(mepyjz_0[0]); mepyjz_0[0] = -388876750; (functional); }\n"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""810"");
try{eval(""M:with({a: undefined.yoyo(window)}){return true;print(491907785.6204296); }var w = x;"");}catch(ex){}
try{eval(""/*bLoop*/for (let juhfrh = 0, x = [,,], \n2304430999, -- \""\"" ; juhfrh < 1; ++juhfrh) { if (juhfrh % 8 == 6) { false; } else { throw [z1,,]; }  } "");}catch(ex){}
try{eval(""print({} = buffer);print(x);"");}catch(ex){}
try{eval(""c = arguments;print(window);"");}catch(ex){}
try{eval(""continue ;"");}catch(ex){}
try{eval(""if(let (d) null) {print([[]]);print(x); } else  if (-1073741823.wrappedJSObject) {print(x); } else print(x);"");}catch(ex){}
try{eval(""var c = (e.__proto__ = Object.getOwnPropertyDescriptor.__defineSetter__(\""eval\"", undefined));let (y) { print(y.ignoreWhitespace = [,,z1]); }\nM:for(let z = (yield x) in x) print();\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""820"");
try{eval(""M:if(this) { if (x.__noSuchMethod__ = undefined) {print(({y: [[]]})); } else {print(x);\n(\""\\uD\"");\nlet c, y, x, x, ryazya, kjypdt, abchsv, functional, c, klhdiy;print(x); }}"");}catch(ex){}
try{eval(""return x;"");}catch(ex){}
try{eval(""while(( \""\"" .x::fromCharCode) && 0);\n(4277);\n"");}catch(ex){}
try{eval(""/*vLoop*/for (var uvrmov = 0; uvrmov < 8; ++uvrmov) { var b = uvrmov; return; } "");}catch(ex){}
try{eval(""let x = !\""\\u45483\"", NaN = \u3056|=(4277), b = (x++), eval = x, dlmbfv, ualhdi, lxfsms, bqdzck, window;print(x);"");}catch(ex){}
try{eval(""/*tLoop*/for each (let c in [ \""\"" , x, x, x, x, x, x, [ \""\""  if (false)],  \""\"" , [ \""\""  if (false)],  \""\"" , function(){}, function(){},  \""\"" , x, x, [ \""\""  if (false)], x, function(){},  \""\"" , function(){},  \""\"" ,  \""\"" ,  \""\"" , function(){},  \""\"" ,  \""\"" ,  \""\"" , function(){}, [ \""\""  if (false)],  \""\"" , x]) { (eval); }"");}catch(ex){}
try{eval(""(undefined);const d = window >>= c;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""830"");
try{eval(""(\""\\uA1D\"");print(( '' .y::c));"");}catch(ex){}
try{eval(""\""use strict\""; const a = null.unwatch(\""w\"");print(281474976710656);"");}catch(ex){}
try{eval(""\""use strict\""; fqacdr, x;print(this);"");}catch(ex){}
try{eval(""var b =  \""\""  ^ 164057539;return;"");}catch(ex){}
try{eval(""print(x);function functional(){/*jjj*/}print(x);"");}catch(ex){}
try{eval(""\""use strict\""; e;(({a2:z2}));"");}catch(ex){}
try{eval(""\""use strict\""; var wsacid = new ArrayBuffer(0); var wsacid_0 = new Int32Array(wsacid); print(wsacid_0[0]); var wsacid_1 = new Uint8Array(wsacid); var wsacid_2 = new WebGLIntArray(wsacid); print(wsacid_2[0]); wsacid_2[0] = -4611686018427388000; default xml namespace  =  /x/g ; ^  '' ;"");}catch(ex){}
try{eval(""for([d, a] = (4277) in this.__defineSetter__(\""\\u3056\"", window)) Array.isArray"");}catch(ex){}
try{eval(""{print( \""\"" );((z =  \""\"" )); }/*tLoop*/for each (let z in [null, function ([y]) { }, null, function ([y]) { }, function ([y]) { }, null, null, function ([y]) { }, null, function ([y]) { }, null, function ([y]) { }, function ([y]) { }, null, function ([y]) { }, null, function ([y]) { }, function ([y]) { }, function ([y]) { }, function ([y]) { }, null, function ([y]) { }, null, null, null, null, null, function ([y]) { }, function ([y]) { }, null, null, function ([y]) { }, function ([y]) { }, null, function ([y]) { }, null, function ([y]) { }, null]) { break ; }"");}catch(ex){}
try{eval(""print(x);\nprint(-0);\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""840"");
try{eval(""return  ''  ?  \""\""  : false;const a = -0.3601009447831096;"");}catch(ex){}
try{eval(""\""use strict\""; /*hhh*/function rlersv(x){print(x);function x(x){/*jjj*/}(x);}rlersv(Math.max(-4503599627370495, -0), (null.watch(\""x\"", ([[1]].sort(Object.getOwnPropertyDescriptor)).bind)));"");}catch(ex){}
try{eval(""\""use strict\""; const e = this ? window :  /x/ ;gczeal(0);x;function x(){/*jjj*/}false;function x(){/*jjj*/}return;"");}catch(ex){}
try{eval(""( \""\"" );function NaN(functional){/*jjj*/}print(x);function d(functional){/*jjj*/}let (y) { print(y); }"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""false;"");}catch(ex){}
try{eval(""var piklxr = new ArrayBuffer(4); var piklxr_0 = new WebGLIntArray(piklxr); print(piklxr_0[0]); piklxr_0[0] = -34359738368; var piklxr_1 = new Int8Array(piklxr); print(piklxr_1[0]); piklxr_1[0] = 1884800476; var piklxr_2 = new Uint8ClampedArray(piklxr); piklxr_2[0] = -4132538199; var piklxr_3 = new Int16Array(piklxr); print(piklxr_3[0]); piklxr_3[0] = 0.758615074046613; sxrcyo();/*hhh*/function sxrcyo(piklxr_3[0], eval){(functional);}this;throw piklxr_3;\""\\u4\"";"");}catch(ex){}
try{eval(""print(x);.y::hasSimpleContent"");}catch(ex){}
try{eval(""var lbbpqs = new ArrayBuffer(12); var lbbpqs_0 = new Float64Array(lbbpqs); print(lbbpqs_0[0]); lbbpqs_0[0] = -4398046511105; var lbbpqs_1 = new Uint16Array(lbbpqs); print(lbbpqs_1[0]); switch(\""\\uBFE911\"") { default: print(lbbpqs_1[5]);break;  }/*vLoop*/for (whpzsx = 0; whpzsx < 4; ++whpzsx) { var x = whpzsx; print( /x/g ); } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""850"");
try{eval(""\""use strict\""; let (x = Math, x, x = x) { do {[1,,]; } while((x) && 0); }"");}catch(ex){}
try{eval(""print(x);[this.ignoreComments = Object.getOwnPropertyDescriptor].filter(Object.defineProperties);"");}catch(ex){}
try{eval(""\""use strict\""; throw {};(\""\\u8\"");"");}catch(ex){}
try{eval(""let (d) { gc() }"");}catch(ex){}
try{eval(""{\""\\u03\"";print(x); }"");}catch(ex){}
try{eval(""/*tLoop*/for each (let x in [ '\\0' , {}]) { (function ([y]) { })().window::toUpperCase( /x/ ); }"");}catch(ex){}
try{eval(""{break ;print(\""\\uA646\""); }"");}catch(ex){}
try{eval(""\""use strict\""; {}\nprint(x);\n"");}catch(ex){}
try{eval(""\""use strict\""; yield;print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""860"");
try{eval(""/*tLoop*/for each (let d in [x = b, [undefined], [undefined], [undefined], [undefined], new String(''), [undefined], x = b, x = b]) { gc() }\n/*bLoop*/for (let qnzykt = 0; qnzykt < 4; ++qnzykt) { if (qnzykt % 6 == 4) { throw ({a1:1}); } else { ( /x/g ); }  } \n"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""decodeURIyield false;"");}catch(ex){}
try{eval(""for(let b in [ /x/g , new Number(1.5), new Number(1.5), 2792391547,  /x/g , new Number(1.5),  /x/g , new Number(1.5), ({}), ({}),  /x/g , 2792391547, new Number(1.5), ({})]) let(eval = (delete w.let), b, w, {} = z = Proxy.createFunction((function handlerFactory(x) {return {getOwnPropertyDescriptor: undefined, getPropertyDescriptor: undefined, defineProperty: function(name, desc) { var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty'); Object.defineProperty(x, name, desc); }, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return Object.getOwnPropertyNames(x); }, fix: undefined, has: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return name in x; }, hasOwn: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return Object.prototype.hasOwnProperty.call(x, name); }, get: function(receiver, name) { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return x[name]; }, set: function(receiver, name, val) { var yum = 'PCAL'; dumpln(yum + 'LED: set'); x[name] = val; return true; }, iterate: undefined, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); var result = []; for (var name in x) { result.push(name); }; return result; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return Object.keys(x); }, }; })(\""\\uF4\""), JSON.parse), b = \""\\uF7C5\"", x = \""\\u2\"", z, wbpkgh) { try { let(e) ((function(){with({}) { 0.8000223536343636; } })()); } catch(z) { this.zzz.zzz; } }let(c) ((function(){return (\u000d((a) >>>= x-=([11,12,13,14].some)) == c);})());"");}catch(ex){}
try{eval(""\""use strict\""; gptejo(x, -3433473648 >>> x);/*hhh*/function gptejo([]){print(x);}(Math.max(1048576, -536870911));\nprint(!new ( /x/g )());\n"");}catch(ex){}
try{eval(""var owzzyx = new ArrayBuffer(6); var owzzyx_0 = new Int32Array(owzzyx); print(owzzyx_0[0]); var owzzyx_1 = new Uint8Array(owzzyx); owzzyx_1[0] = 2097152; var owzzyx_2 = new Uint16Array(owzzyx); owzzyx_2[0] = -1031795320; var owzzyx_3 = new Float32Array(owzzyx); owzzyx_3[0] = 1155545964; var owzzyx_4 = new WebGLFloatArray(owzzyx); var owzzyx_5 = new Uint16Array(owzzyx); print(owzzyx_5[0]); owzzyx_5[0] = 1272484087.0472934; (\""\\uD6\"");"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""870"");
try{eval(""let (yixsgy, yeybwl, x) { (\""\\u873CC\""); }"");}catch(ex){}
try{eval(""var bpfddy = new ArrayBuffer(0); var bpfddy_0 = new Float64Array(bpfddy); bpfddy_0[0] = -72057594037927940; var bpfddy_1 = new Int8Array(bpfddy); bpfddy_1[0] = -0.3029701538609314; var bpfddy_2 = new WebGLIntArray(bpfddy); bpfddy_2[0] = -1989893640; print((this.zzz.zzz));print(bpfddy_0);"");}catch(ex){}
try{eval(""throw w;"");}catch(ex){}
try{eval(""yield e;(x);( /x/g );"");}catch(ex){}
try{eval(""switch((false.function::x) ? (x ? null : c) : 3772402579.7828846) { case ([null for each (d in [['z'], new String('q'), new String('q'), new String('q'),  '' ,  '' , ['z'], new String('q'),  '' ,  '' ,  \""use strict\"" ])]) > 2379562020.9947643.prototype: break; default: print( /x/ );break; case \""\\u421C32\"": print(0);case  '' .unwatch(\""\\u7FDA\"", d)\u000c: case  /x/g : print(x);break;  }\nb = NaN;\n"");}catch(ex){}
try{eval(""with(0.36267610761547036){if( '' ) {this;print(x); } else x;{} }"");}catch(ex){}
try{eval(""var oqzozw = new ArrayBuffer(2); var oqzozw_0 = new Uint32Array(oqzozw); var oqzozw_1 = new Int32Array(oqzozw); print(oqzozw_1[0]); oqzozw_1[0] = -0; var oqzozw_2 = new Float64Array(oqzozw); print(oqzozw_2[0]); oqzozw_2[0] = d; var oqzozw_3 = new WebGLFloatArray(oqzozw); print(oqzozw_3[0]); oqzozw_3[0] = 32; var oqzozw_4 = new Int16Array(oqzozw); var oqzozw_5 = new WebGLFloatArray(oqzozw); print(oqzozw_5[0]); oqzozw_5[0] = 8388609; print(oqzozw_3[0]);(this);var ciltfx = new ArrayBuffer(6); var ciltfx_0 = new Uint8Array(ciltfx); ciltfx_0[0] = 1873643337; var ciltfx_1 = new Uint16Array(ciltfx); ciltfx_1[0] = 33554433; var ciltfx_2 = new Int16Array(ciltfx); ciltfx_2[0] = -2333264758; {}"");}catch(ex){}
try{eval(""let eymjmn, x = (undefined , d);(( /x/g )(96215216, x));"");}catch(ex){}
try{eval(""\""use strict\""; {for(var d in  /x/g ) {(this); } }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""880"");
try{eval(""if((4277)) { if (((false)(x)) = new /*wrap1*/(function(){ window;return Proxy.isTrapping})()(0x80000000,  '' )) {print(x);return x; }} else {return this; }"");}catch(ex){}
try{eval(""return  '' ;\n36028797018963970;\n"");}catch(ex){}
try{eval(""with([x.w::ignoreComments = Object.isFrozen\u000dInfinity]++ >>>= x)/*tLoop*/for each (let z in [arguments.caller, (1/0), (1/0), NaN, NaN, NaN]) { print(\""\\u4F1\""); }"");}catch(ex){}
try{eval(""let c, x = 16777215;(window);function z(a, \u3056){/*jjj*/}print(x);\n( /x/ );\n"");}catch(ex){}
try{eval(""\u0009L:if(x) { if (((({functional: false })) ^= (yield  /x/g ))) {(z);false; } else {;mabzny, wgezxz, cqjnou, peuwdc, y, yield, z, b;continue L; }}"");}catch(ex){}
try{eval(""switch('fafafa'.replace(/a/g, (Object.isFrozen).bind)) { case 4: break; case (-- /x/g ) == z = null: break; (gczeal(0));case 8: break; default: print(\""\\u5\"");\nprint(x);\ncase x: print(((\""\\uA0A\"")));break;  }"");}catch(ex){}
try{eval(""default xml namespace  = [1];"");}catch(ex){}
try{eval(""print( '' );\n(window);\n"");}catch(ex){}
try{eval(""let (vhuolc, vzxuty, qawvxi) { throw true;break M; }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""890"");
try{eval(""ufpyqn((4277), (w >>>= x));/*hhh*/function ufpyqn(x, x){(let (e=eval) e)}"");}catch(ex){}
try{eval(""\""use strict\""; {}/*tLoop*/for each (let w in [(0/0), new Number(1.5), (0/0), (0/0), -Infinity, new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), false, (0/0), false, new Number(1.5), -Infinity]) { print(x); }"");}catch(ex){}
try{eval(""d = y = Proxy.create((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: undefined, has: String, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: Math.acos, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: function() { var yum = 'PCAL'; dumpln(yum + 'LED: keys'); return []; }, }; })(\u3056),  /x/g ) , (4277)--;/*vLoop*/for (yaxvwu = 0, x, window; yaxvwu < 3; ++yaxvwu) { var b = yaxvwu; false; } "");}catch(ex){}
try{eval(""(undefined);\n({a2:z2});\nprint(((x = Object.defineProperties(this, windo;"");}catch(ex){}
try{eval(""/*oLoop*/for (var hbejpa = 0, this.zzz.zzz; hbejpa < 0; ++hbejpa) { throw (4277); } "");}catch(ex){}
try{eval(""/*oLoop*/for (let gefdcp = 0; gefdcp < 4; ++gefdcp) { print((x ^= \""\\u3A6\"")); } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""900"");
try{eval(""\""use strict\""; x, {} = \""\\u477D86\"";function(y) { print(x !== false); }"");}catch(ex){}
try{eval(""let (x) { print(x); }"");}catch(ex){}
try{eval(""/*vLoop*/for (vndyge = 0; vndyge < 9; ++vndyge) { w = vndyge; 1532629437.216395;null; } "");}catch(ex){}
try{eval(""if((4277)) { if (((x))) throw -0.5197524959927126;(length); else const x = Array.reduce.prototype, d = (z = #3={a:#3#}), pqwrhf, x, x, khsuws, gdubya, b, this.b, x;{gczeal(0);print(this); }}"");}catch(ex){}
try{eval(""\""use strict\""; print(( \""\"" .constructor = (let (e=eval) e)));"");}catch(ex){}
try{eval(""\""use strict\""; {} = \""\\u8C\"", nkdeaw, eval, x = ( /x/  >>= this >>> ne\000w  /x/ ( '' ))\u0009;print(x);\nfor(let d = (4277) in \""\\u3B\"") {print(x); /x/g ; }\n"");}catch(ex){}
try{eval(""/*bLoop*/for (let fcemeg = 0; fcemeg < 14 && w; ++fcemeg) { if (fcemeg % 7 == 4) { gc() } else { \""\\u7CF5\""; }  } "");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""910"");
try{eval(""print(false);print((4277));function x(c){/*jjj*/}print(\""\\uE34\"");"");}catch(ex){}
try{eval(""return;\n(-0);\n"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let b in [1e4, 1e4, new Boolean(false), new Boolean(false), true, (void 0), {}, {}, (void 0), new Boolean(false), 1e4, new Boolean(false), 1e4, true, (void 0), {}, true, 1e4, 1e4, true]) { /*tLoop*/for each (let e in [ /x/ , [undefined]]) {  '' ; } }"");}catch(ex){}
try{eval(""let (a = [[]], fvmtut, omdrzb, x, x, kuyjie, e, window, llqjqs) { print(x); }\n(\""\\u32AC\"");\n"");}catch(ex){}
try{eval(""{{print(x); } }"");}catch(ex){}
try{eval(""x;function y(a)//h\n{/*jjj*/}print(18446744073709552000);\nprint(x);\n"");}catch(ex){}
try{eval(""with({}) { with({}) let(w = -0[\""\\u1B\""]++, window = eval(\""gczeal(0);\"") in x.ignoreWhitespace = x, x = ((function too_much_recursion(iexcon) { print(x);; if (iexcon > 0) { print(length);; too_much_recursion(iexcon - 1); }  })(6778)), b, a = (new XPCSafeJSObjectWrapper(\""\\uC38BA\"")), x = (-0), x) ((function(){let(a) ((function(){this.zzz.zzz;})());})()); } "");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (ploivh = 0; ploivh < 13; ++ploivh) { if (ploivh % 5 == 3) { print(x); } else { print((4277)); }  } "");}catch(ex){}
try{eval(""\""use strict\""; print(x);\n /x/g ;\nwsotih(c);/*hhh*/function wsotih(b){print(true);}print(new XPCNativeWrapper(Math.pow(-134217727, function ([y]) { })));(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""920"");
try{eval(""/*vLoop*/for (let ffohzu = 0; ffohzu < 5; ++ffohzu) { var z = ffohzu; (-512); } "");}catch(ex){}
try{eval(""\""use strict\""; /*hhh*/function survjq(){let (kzlyiw, x = x, shfnfa, c = \""\\u8\"", window, tjujlw) { print(x([ ''  for (functional in this)], [11,12,13,14].filter)); }}/*iii*/(\""\\u59\"");"");}catch(ex){}
try{eval(""/*oLoop*/for (let qulfli = 0; qulfli < 11; ++qulfli) { gczeal(0); } "");}catch(ex){}
try{eval(""print(\""\\u19\"");var x = this;"");}catch(ex){}
try{eval(""{;print(x::window); }"");}catch(ex){}
try{eval(""for(a in \""\\u68481F\"") /*bLoop*/for (var fmlhre = 0; fmlhre < 21; ++fmlhre) { if (fmlhre % 6 == 4) { print(a); } else { print(functional); }  } function e(){/*jjj*/}print(window);"");}catch(ex){}
try{eval(""if(({y: x.PI = Object.isSealed }) <= [ \""\"" ].some(/*wrap3*/(function(){ var kvcyei = -3245936870; (Array.isArray)(); }))) print(window); else  if (x) {#1=[#1#];throw [1]; } else print(-17179869184);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""930"");
try{eval(""var   = x, x, x = \""\\uA58F3\"", x, x, z, laoffa, boxdps, x, vkzend;print(undefined);\n\n"");}catch(ex){}
try{eval(""\""use strict\""; switch(x) { ( '' ); }"");}catch(ex){}
try{eval(""\""use strict\""; L:if(undefined) yield; else  if (true) {(functional); } else (({a2:z2}));"");}catch(ex){}
try{eval(""var d;-2375193910;"");}catch(ex){}
try{eval(""\""use strict\""; /*vLoop*/for (let fxdnqg = 0; fxdnqg < 7; ++fxdnqg) { let y = fxdnqg; (\""\\u77CB\""); } "");}catch(ex){}
try{eval(""\""use strict\""; var hxdnak = new ArrayBuffer(8); var hxdnak_0 = new WebGLFloatArray(hxdnak); print(false);var qihbbg = new ArrayBuffer(16); var qihbbg_0 = new WebGLFloatArray(qihbbg); qihbbg_0[0] = 1e4; print(null);a;return;this;yield;"");}catch(ex){}
try{eval(""( '' )\n"");}catch(ex){}
try{eval(""undefined;"");}catch(ex){}
try{eval(""with({}) { Math; } let(c) { (window);}"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""940"");
try{eval(""print(caller);throw x;"");}catch(ex){}
try{eval(""\u0009/*hhh*/function jpbihn(){(this);;}/*iii*/{print( \""\"" ); }"");}catch(ex){}
try{eval(""if(gczeal(0)) {let caiymy, window =  \""\"" ['_' + (y)], x = null, dmrjmx, wxmbws, pgpeam, azebll;-2910079019; }"");}catch(ex){}
try{eval(""e = this;print(x);"");}catch(ex){}
try{eval(""/*hhh*/function qceevr(z, a){print(x);}qceevr(x, x);"");}catch(ex){}
try{eval(""(4277);yield;"");}catch(ex){}
try{eval(""var xzkqdl = new ArrayBuffer(16); var xzkqdl_0 = new Int8Array(xzkqdl); print(xzkqdl_0[0]); var xzkqdl_1 = new Uint8Array(xzkqdl); print(xzkqdl_1[0]); (0x80000000);let d = (4277);"");}catch(ex){}
try{eval(""\""use strict\"";  for  each(let z in  '' ) {([,]); }"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""950"");
try{eval(""\""use strict\""; print(x);\ngc()( '' );\n"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""\""use strict\""; /*bLoop*/for (var rpmfus = 0; rpmfus < 2 && undefined; ++rpmfus) { if (rpmfus % 4 == 3) { return \""\\u6F6E\""; } else { gc }  } "");}catch(ex){}
try{eval(""M:switch( '' ) { case this: break;  }function x(eval(\""\\u3E\"")){/*jjj*/}print(true++);(false++);"");}catch(ex){}
try{eval(""\""use strict\""; print(x);"");}catch(ex){}
try{eval(""\""use strict\""; if(window.wrappedJSObject = x) {throw -1276111504.519173; }\u000d else /*\n*/ if (Math.pow(-16384, x)) ;"");}catch(ex){}
try{eval(""L:for(var [x, a] = /*wrap2*/(function(){ var rmrmaj = ((function sum_indexing(behzaq, hrrbjk) { (1 for (x in [])); return behzaq.length == hrrbjk ? 0 : behzaq[hrrbjk] + sum_indexing(behzaq, hrrbjk + 1); })([eval, eval, d.buffer = 17, eval, [(void 0)], [(void 0)], d.buffer = 17, d.buffer = 17, d.buffer = 17,  '' , [(void 0)], eval], 0)); var otrrcc = /*wrap3*/(function(){ var ttcsqm = x; (true >= eval)(); }); return otrrcc;})().prototype ? x.e : \""\\u4B03\"" in #2=({z: window( \""\"" , set)})) {print(x);function \u3056(a){/*jjj*/}(({ get x(e) /x/g , x:  /x/g  })); }"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""var ubfxyi = new ArrayBuffer(4); var ubfxyi_0 = new WebGLIntArray(ubfxyi); ubfxyi_0[0] = 3/0; var ubfxyi_1 = new Int32Array(ubfxyi); ubfxyi_1[0] = 2047; var ubfxyi_2 = new Float32Array(ubfxyi); print(ubfxyi_2[0]); return ((function sum_indexing(bgopfp, ogasaz) { ; return bgopfp.length == ogasaz ? 0 : bgopfp[ogasaz] + sum_indexing(bgopfp, ogasaz + 1); })([window, window, null, null,  /x/ ,  /x/ , window,  /x/ ,  /x/ ,  /x/ ,  /x/ , window, null, window, null, window, window, null, window, null], 0));print(-0);(eval(\""[,,z1]\""));(0.08337182740532445)[[,,z1]];print(ubfxyi_2);"");}catch(ex){}
try{eval(""var z = (4277);yield;var b = x;"");}catch(ex){}
try{eval(""\""use strict\""; {(-0);print(x); }\nx = Proxy.create((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(y) { {} }, getOwnPropertyNames: undefined, fix: undefined, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: undefined, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: undefined, }; })( /x/ ),  '' )['_' + (((x['x'] = (3/0 for each (each in [undefined, window, window])))))];\n"");}catch(ex){}
try{eval(""\""use strict\""; x = linkedList(x, 4704);"");}catch(ex){}
try{eval(""/*tLoop*/for each (let d in [null, {x:3}, {x:3},  /x/g , {x:3}, {x:3}, {x:3}, function(){}, function(){}, {x:3}, function(){}, {x:3}, function(){}, {x:3}, function(){}, function(){},  /x/g ,  /x/g , null, null, {x:3}, null, null,  /x/g , function(){},  /x/g , null, null, null, function(){}, function(){},  /x/g , {x:3}]) { let getter = Math, qcrlci, fzudtd, aplthc, w;; }\n/*bLoop*/for (var vyxhep = 0; vyxhep < 1; ++vyxhep) { if (vyxhep % 11 == 1) { ; } else { print((\""\\uB4B\"".valueOf(\""number\""))); }  } \n"");}catch(ex){}
try{eval(""let w = let //h\n(b)  /x/g , window;null;"");}catch(ex){}
try{eval("";\nprint(x);\nArray.reduce\n\n(e.w getter= window);function w(){/*jjj*/}print(x);\n\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""970"");
try{eval(""\""use strict\""; return;"");}catch(ex){}
try{eval(""\""use strict\""; print(((w)) = x);"");}catch(ex){}
try{eval(""\""use strict\""; vilsqa, vmrkso, \u3056, uviuqs, tabgge, NaN, gybclo;Int32Array"");}catch(ex){}
try{eval(""print(x);print([,,]);"");}catch(ex){}
try{eval(""((x = [ /x/  for (functional in -3567194238.4506616)]));"");}catch(ex){}
try{eval(""\""use strict\""; switch( /x/ ) { default: print(x); }"");}catch(ex){}
try{eval(""return;throw this.x;"");}catch(ex){}
try{eval(""\""use strict\""; /*tLoop*/for each (let w in [new Number(1),  /x/g ,  /x/g , new Number(1), new Number(1),  /x/g , function(){}, function(){}, new Number(1),  /x/g , new Number(1), function(){}, new Number(1), function(){}, function(){}, function(){}, function(){},  /x/g ,  /x/g , function(){},  /x/g ,  /x/g ,  /x/g ,  /x/g ]) { print(w); }"");}catch(ex){}
try{eval(""\nthrow this;\nprint(window);\n"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""980"");
try{eval(""print(x);"");}catch(ex){}
try{eval(""\""use strict\""; const x = delete c.a, each = [11,12,13,14].some(x, \""\\uF3\""), x = \n( /* Comment */(new ([])(false))), NaN, e, y = false, b; /x/g ;(this);"");}catch(ex){}
try{eval(""a = linkedList(a, 576);"");}catch(ex){}
try{eval(""/*tLoop*/for each (let x in [null, {x:3}, null, null, new String(''), {x:3}, {x:3}, new String(''), new String(''), {x:3}, new String(''), {x:3}, null, null, {x:3}, null, {x:3}, null]) { ; }"");}catch(ex){}
try{eval(""\""use strict\""; L:switch(c.x::prettyIndent = new Function > (function ([y]) { })()) { case ([15,16,17,18].some(eval, (x = \""\\u5F\""))): print( /* Comment */ \""\"" );break; case -8388608: break; case 6: break; case 4: print(x);case 6: break;  }"");}catch(ex){}
try{eval(""Object.sealfunction window(c){/*jjj*/} '' ;"");}catch(ex){}
try{eval(""Math.max(-140737488355327, -3);function w([], d){/*jjj*/}print(x);"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""990"");
try{eval(""let(eval = c, z, ofsbep, sejxkm, x) { gczeal(0);}"");}catch(ex){}
try{eval(""\""use strict\""; /*iii*/print(x);/*hhh*/function hctgzq(){(x);}"");}catch(ex){}
try{eval(""\""use strict\""; /*vLoop*/for (jmgwsm = 0; jmgwsm < 3; ++jmgwsm) { d = jmgwsm; print(this);[[]]; } "");}catch(ex){}
try{eval(""let({z} = ((4277) for each (y in  /x/ ) if ( \""\"" )), y = x = z, x = this--, zesolh) ((function(){yield 'fafafa'.replace(/a/g, function(y) { return \""\\u4594\"" });})());"");}catch(ex){}
try{eval(""print(x);"");}catch(ex){}
try{eval(""do window = w; while((Math.atan2( /* Comment */window,  '' )) && 0);"");}catch(ex){}
try{eval(""\""use strict\""; print(true);\nthis;\n"");}catch(ex){}
try{eval(""\""use strict\""; function shapeyConstructor(nfaxms){\""use strict\""; { \""use strict\""; {} } if (nfaxms) this.w = eval.callee = Object.isFrozen;this.w = [[1]](\""\\uC2\"");this.w = null;{ print(let (\u3056)  /x/ ); } delete this.w;{  \""\"" ; } return this; }/*tLoopC*/for each (let b in [x = NaN, x, null, x, {x:3}, null, {x:3}, {x:3}, {x:3}, x = NaN, {x:3}, x, null, x = NaN, null, x, new String('q'), x = NaN, {x:3}, {x:3}, new String('q'), x, new String('q'), x = NaN, null, x]) { try{let cehxul = shapeyConstructor(b); print('EETT'); print(x);}catch(e){print('TTEE ' + e); } }"");}catch(ex){}
try{eval(""\""use strict\""; return;"");}catch(ex){}
if(trackCnt++ % trackStep == 0) WScript.Echo(""1000"");
try{eval(""window;function x(\u3056){/*jjj*/}this;"");}catch(ex){}


";
        #endregion

        [TestMethod]
        [WorkItem(323102)]
        public void UnintentionalInfinitelyRecursiveFunction()
        {
            PerformCompletionRequests(@"
                function functionA(counter, freq) {
                    if (counter > 2) throw '1234';
                    if (co|counter,freq|unter % freq == 0) return functionA(counter + 1, freq);
                }
                try{
                    functionA(0, 1);
                }
                catch (e) {
                }");
        }

        //[TestMethod]
        //[WorkItem(331612)]
        //public void FuzzingTest2()
        //{
        //    WithMTASession(delegate
        //    {
        //        var primary = _session.FileFromText(FuzzingTest2Text);
        //        var context = _session.OpenContext(primary);
        //        using (IDisposable hurry = ExecutionLimiter(context))
        //        {
        //            var completions = context.GetCompletionsAt(primary.Text.Length);
        //            Assert.IsNotNull(completions);
        //        }
        //    });
        //}
        #region Test data
        const string FuzzingTest2Text = @"
try{eval(""switch(8.watch(\""functional\"", ArrayBuffer)) { case eval: (1373119662.callee = false);case (4277):  }"");}catch(ex){}
try{eval(""\""use strict\""; do {(\""\\uA582D\"");let c = \""\'\uD\"".__defineGetter__(\""c\"", Object.defineProperty);for(x in (( /x/ \n)((4277)))){print(((function fibonacci(zaxecv) { print(\u3056);; if (zaxecv <= 1) { ; return 1; } print(1152921504606847000);; return fibonacci(zaxecv - 1) + fibonacci(zaxecv - 2);  })(3)) = x);print(false); } } while((([11,12,13,14].sort)\u000c) && 0);"");}catch(ex){}";
        #endregion

        [TestMethod]
        public void AccessingGlobalUndefinedValues()
        {
            PerformCompletionRequests(@"
                Box4.a = 1;
                Box4.b = 2;
                Box4.c = 3;
                Box4.d = 4;
                Box4.|!a|;
            ");
        }

        [TestMethod]
        public void ClosureScopeInUncalledMethodsAutoCallParent()
        {
            PerformCompletionRequests(@"
            var x = 2;
            (function ($) {
              var test = function () {
                   $.|Number|
                };
            })(x);");
        }

        [TestMethod]
        public void ClosureScopeInUncalledMethodsExplictCallParent()
        {
            PerformCompletionRequests(@"
                function a(x) {
                   function b() {
                     x.|Number|;
                   }
                }
                a(1);");
        }

        [TestMethod]
        public void LambdaInUncalledMethodsAutoCallParent()
        {
            PerformCompletionRequests(@"
            var x = 2;
            (function ($) {
              var test = () => $.|Number|;
            })(x);");
        }

        [TestMethod]
        public void LambdaInUncalledMethodsExplictCallParent()
        {
            PerformCompletionRequests(@"
                function a(x) {
                   () => x.|Number|;
                }
                a(1);");
        }

        [TestMethod]
        public void LambdaCapturedVarsNotInParent()
        {
            PerformCompletionRequests(@"
                function foo(x) {
                    ;|!a,!b,x|;
                    return (a, b) => {
                        a*b;
                        ;|a,b,x|;
                    }
                }
                ;|!a,!b|;
                ");
        }

        [TestMethod]
        public void LambdaNestedCapturedVars()
        {
            PerformCompletionRequests(@"
                function foo(x) {
                    ;|!a,!b,x|;
                    return (a, b) => {
                        a*b;
                        ;|a,b,x,!c,!d|;
                        (c, d) => { ;|a,b,x,c,d|; }
                    }
                }
                ;|!a,!b|;
                ");
        }

        [TestMethod]
        public void LambdaWithCapturedThis()
        {
            PerformCompletionRequests(@"
                function a(x) {
                    this.a = 1;
                    return (this) => 
                    {
                        this.|a|; 
                    }
                }");
        }

        [TestMethod]
        public void LambdaWithUncapturedThis()
        {
            PerformCompletionRequests(@"
                function a(x) {
                    this.a = 1;
                    return () => 
                    {
                        this.|a|; 
                    }
                }");
        }

        [TestMethod]
        public void LambdaHeavilyNested()
        {
            PerformCompletionRequests(@"
                x => => => => { ;|x|; }
            ");
        }

        [TestMethod]
        public void SpreadCall()
        {
            PerformCompletionRequests(@"
                var x = [1, 2, 3];
                var y = [100];
                var z = ['a', 'b' 'c', 'd'];

                function add(a, b, c) {
                    return a + b + c;
                }

                add(...|x,y,z|);

                var mixed = [1, 'hello world', function () { }];

                function validate() {
                    if(arguments[0] === 1 && arguments[1] === 'hello world' &&  
                       arguments[2].toString() == 'function () { }')
                    {
                        
                        intellisense.logMessage('Assert Passed.');
                    }
                    else
                    {
                        intellisense.logMessage('Assert Failed.');
                    }

                }
                validate(...mixed);
            ");
        }

        [TestMethod]
        public void SpreadArrayLiteral()
        {
            PerformCompletionRequests(@"
                var x = [1, 2, 3];
                var y = [100];
                var z = ['a', 'b', 'c', 'd'];

                var array = [...|x,y,z|];

                var mixed = [1, 'hello world', function () { }];
                var validateArray = [...mixed];
                if(validateArray[0] === 1 && validateArray[1] === 'hello world' 
                                       &&  validateArray[2].toString() == 'function () { }')
                {
                    validateArray[0].Number;
                    validateArray[1].String;
                    validateArray[2].Function;
                    intellisense.logMessage('Assert Passed.');
                }
                else
                {
                    intellisense.logMessage('Assert Failed.');
                }
            ");
        }

        [TestMethod]
        [WorkItem(405915)]
        public void IfConditionExpressionErrorCorrection()
        {
            PerformCompletionRequests(IfConditionExpressionErrorCorrectionText);
        }
        #region Test data
        const string IfConditionExpressionErrorCorrectionText = @"
function init() {
    var localToInit;
    function foo() {
        function bar() {
            var num1 = 1;
            if(num
        }
    }
}

function baz() {
|!localToInit|
}";
        #endregion

        [TestMethod]
        [WorkItem(417638)]
        public void IDBRequestCompletions()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests(@"
                var Result = {};
                intellisense.addEventListener('statementcompletion',  function(e) {});
                function Assert(expr, message) {
                    if (expr !== true) {
                        Result.Failed = true;
                        intellisense.logMessage('Assert failed. '+ message);
                    }
                    else {
                        intellisense.logMessage('Assert Passed.');
                    }
                }

                function VerifyDomType(expectedTypeName, actual){
                    Assert(window[expectedTypeName].prototype.isPrototypeOf(actual), 'Expected type: ' + expectedTypeName);
                }


                /***** IDBFactory *****/
                // open
                var IDBFactory_openResult = indexedDB.open ('name', 3);
                VerifyDomType('IDBOpenDBRequest', IDBFactory_openResult);
                Assert(IDBFactory_openResult.source === null, 'IDBFactory.open source does not match expected');
                VerifyDomType('IDBDatabase', IDBFactory_openResult.result);
                

                // deleteDatabase
                var IDBFactory_deleteDatabaseResult = indexedDB.deleteDatabase ('name', 3);
                VerifyDomType('IDBOpenDBRequest', IDBFactory_deleteDatabaseResult);
                Assert(IDBFactory_deleteDatabaseResult.source === null, 'IDBFactory.deleteDatabase source does not match expected');
                Assert(IDBFactory_deleteDatabaseResult.result === null, 'IDBFactory.deleteDatabase result does not match expected');


                /***** IDBObjectStore *****/
                var db = indexedDB.open('name', 3).result;
                var objectStore = db.createObjectStore('name');

                // put 
                var IDBObjectStore_putResult = objectStore.put({},'key');
                Assert(IDBObjectStore_putResult.source === objectStore, 'IDBObjectStore.put source does not match expected');
                Assert(IDBObjectStore_putResult.result === 'key', 'IDBObjectStore.put result does not match expected');

                // add 
                var IDBObjectStore_addResult = objectStore.add({},'key');
                Assert(IDBObjectStore_addResult.source === objectStore, 'IDBObjectStore.add source does not match expected');
                Assert(IDBObjectStore_addResult.result === 'key', 'IDBObjectStore.add result does not match expected');

                // delete 
                var IDBObjectStore_deleteResult = objectStore.delete('key');
                Assert(IDBObjectStore_deleteResult.source === objectStore, 'IDBObjectStore.delete source does not match expected');
                Assert(IDBObjectStore_deleteResult.result === undefined, 'IDBObjectStore.delete result does not match expected');

                // get 
                var IDBObjectStore_getResult = objectStore.get('key');
                Assert(IDBObjectStore_getResult.source === objectStore, 'IDBObjectStore.get source does not match expected');
                Assert(!!IDBObjectStore_getResult.result, 'IDBObjectStore.get result does not match expected');

                // clear 
                var IDBObjectStore_clearResult = objectStore.clear();
                Assert(IDBObjectStore_clearResult.source === objectStore, 'IDBObjectStore.clear source does not match expected');
                Assert(IDBObjectStore_clearResult.result === undefined, 'IDBObjectStore.clear result does not match expected');

                // openCursor 
                var IDBObjectStore_openCursorResult = objectStore.openCursor();
                Assert(IDBObjectStore_openCursorResult.source === objectStore, 'IDBObjectStore.openCursor source does not match expected');
                VerifyDomType('IDBCursorWithValue', IDBObjectStore_openCursorResult.result);
                Assert(IDBObjectStore_openCursorResult.result.source === objectStore, 'IDBObjectStore.openCursor result.source does not match expected');

                // count 
                var IDBObjectStore_countResult = objectStore.count();
                Assert(IDBObjectStore_countResult.source === objectStore, 'IDBObjectStore.count source does not match expected');
                Assert(IDBObjectStore_countResult.result === 0, 'IDBObjectStore.count result does not match expected');


                /***** IDBCursor *****/
                var cursor = objectStore.openCursor().result;

                // update
                var IDBCursor_updateResult = cursor.update('key');
                Assert(IDBCursor_updateResult.source === cursor, 'IDBCursor.update source does not match expected');
                Assert(IDBCursor_updateResult.result === 'key', 'IDBCursor.update result does not match expected');

                // delete
                var IDBCursor_deleteResult = cursor.delete();
                Assert(IDBCursor_deleteResult.source === cursor, 'IDBCursor.delete source does not match expected');
                Assert(IDBCursor_deleteResult.result === undefined, 'IDBCursor.delete result does not match expected');


                /***** IDBIndex *****/
                var index = objectStore.createIndex('name', '');

                // openCursor 
                var IDBIndex_openCursorResult = index.openCursor('range', 'direction');
                Assert(IDBIndex_openCursorResult.source === index , 'IDBIndex.openCursor source does not match expected');
                VerifyDomType('IDBCursorWithValue', IDBIndex_openCursorResult.result);
                Assert(IDBIndex_openCursorResult.result.source == index, 'IDBIndex.openCursor result.source does not match expected');

                // openKeyCursor
                var IDBIndex_openKeyCursorResult = index.openKeyCursor('range', 'direction'); 
                Assert(IDBIndex_openKeyCursorResult.source === index.objectStore , 'IDBIndex.openKeyCursor source does not match expected');
                VerifyDomType('IDBCursor', IDBIndex_openKeyCursorResult.result);
                Assert(IDBIndex_openKeyCursorResult.result.source == index, 'IDBIndex.openKeyCursor result.source does not match expected');

                // get 
                var IDBIndex_getResult = index.get('key'); 
                Assert(IDBIndex_getResult.source === index.objectStore , 'IDBIndex.get source does not match expected');
                Assert(!!IDBIndex_getResult.result, 'IDBIndex.get result does not match expected');

                // getKey 
                var IDBIndex_getKeyResult = index.getKey('key'); 
                Assert(IDBIndex_getKeyResult.source === index.objectStore , 'IDBIndex.getKey source does not match expected');
                Assert(!!IDBIndex_getKeyResult.result, 'IDBIndex.getKey result does not match expected');

                // count 
                var IDBIndex_countResult = index.count('key');
                Assert(IDBIndex_countResult.source === index, 'IDBIndex.count source does not match expected');
                Assert(IDBIndex_countResult.result === 0, 'IDBIndex.count result does not match expected');

                Result.|!Failed|;
            ", domjs);
        }

        [TestMethod]
        [WorkItem(431475)]
        public void RegExpErrorInContextFile()
        {
            PerformCompletionRequests(@"'s'.|String|", @"/A/G");
        }

        [TestMethod]
        [WorkItem(534628)]
        public void FieldMembersInPrototypeFunctions()
        {
            PerformCompletionRequests(FieldMembersInPrototypeFunctionsText);
        }
        #region Test data
        const string FieldMembersInPrototypeFunctionsText = @"
var Test = {};
Test.SimpleObject = function () {
  ///<summary>A very simple example</summary> 
  ///<field name=""testString"" type=""String""/> 
  ///<field name=""testBool"" type=""Boolean"" /> 
  this.testString = null;
  this.testBool = false;
};

Test.SimpleObject.prototype =
{
    getStuff: function () {
        ///<summary>Get stuff</summary> 
        ///<returns type=""Boolean""/> 
        this.|getStuff,testString,testBool|
        return false;
    }
};";
        #endregion

        [TestMethod]
        public void Verify__proto__usage()
        {
            PerformCompletionRequests(Verify__proto__usageText);
        }
        #region Test data
        const string Verify__proto__usageText = @"
var p = {
  p1: function () { },
  p2: function () { }
};

var a = {};
a.__proto__ = p;

a.|p1,p2|

";
        #endregion

        [TestMethod]
        [WorkItem(464679)]
        public void FunctionInTryCatchInSwitch()
        {
            PerformCompletionRequests(FunctionInTryCatchInSwitchText);
        }
        #region Test data
        public const string FunctionInTryCatchInSwitchText = @"
var a = 1;
switch (a) {
    default:
        try {
            a++;
        } catch (e) {
            function test() {
                (a|a,e|
            }
        }
        break;
}
";
        #endregion

        [TestMethod]
        [WorkItem(438979)]
        public void ParameterValuesAffectingResult()
        {
            PerformCompletionRequests(@"
                function foo(a) {
                    /// <param name=""a"" type=""Number"">The first parameter</param>
                    return a;
                }
                function bar(a) {
                    return a;
                }
                function goo(a, b) {
                    /// <param name=""a"" type=""Number"">The first parameter</param>
                    /// <param name=""b"" type=""String"">The second parameter</param>
                    return { a: a, b: b };
                }
                var d = foo();
                d.|Number|;
                d = foo();
                d.|Number|;
                d = foo(""some string"");
                d.|String|;
                d = bar();
                d.|!Number|;
                var o = goo();
                o.a.|Number|;
                o.b.|String|;");

        }

        [TestMethod]
        [WorkItem(620244)]
        public void ReferenceUpdateMemoryLeak()
        {
            var primaryFile = _session.FileFromText("");
            var defineAddAll = _session.FileFromText(defineAddAll_Text);
            var dummy = _session.FileFromText("");

            var context = _session.OpenContext(primaryFile, defineAddAll);

            Action<bool> doCompletion = expected =>
            {

                var completion = context.GetCompletionsAt(0);
                if (expected)
                    completion.ToEnumerable().ExpectContains("addAll", "useAddAll");
                else
                    completion.ToEnumerable().ExpectNotContains("addAll", "useAddAll");
                context.TakeOwnership(completion);
                Marshal.ReleaseComObject(completion);

            };

            // Establish a base line
            doCompletion(true);
            context.RemoveContextFiles(defineAddAll);
            doCompletion(false);
            context.AddContextFiles(defineAddAll);
            doCompletion(true);

            _session.Cleanup(true);

            // Remove the defineAddAll text
            context.RemoveContextFiles(defineAddAll);
            doCompletion(false);

            // Add it back again
            context.AddContextFiles(defineAddAll);
            doCompletion(true);

            context.Close();

            _session.Cleanup(true);
        }
        #region Test data
        const string defineAddAll_Text = @"function addAll(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) {
    var result = arg1 + arg2 + arg3 + arg4 + arg5 + arg6 + arg7 + arg8 + arg9
    return result
}
function add(a, b){
    return a + b
}
function useAddAll(a, b, c, d, e, f, g, h, i, j) {
    return addAll(a, b, c, d, e, f, g, h, add(i ,j))
}";
        #endregion

        // Completions for ES6 features
        [TestMethod]
        [WorkItem(508887)]
        [Ignore]
        public void IntlCompletions()
        {
            PerformCompletionRequests(IntlCompletionsTest, (completions, data, i) =>
            {
                switch (data)
                {
                    case "global": completions.ExpectContains("Intl"); break;
                    case "Intl": 
                        completions.ExpectContains(IntlMethods); 
                        completions.ExpectNotContains("EngineInterface");
                        break;
                    case "IntlConstructor": completions.ExpectContains(IntlObjectConstructorMethods); break;
                    case "Collator": completions.ExpectContains(CollatorMethods); break;
                    case "IntlFormat": completions.ExpectContains(IntlFormatObjectMethods); break;
                    case "Function": completions.ExpectContains(FunctionMethods); break;
                    case "BoundFunction": completions.ExpectContains(BoundFunctionMethods); break;
                    default: Assert.Fail("Unhandled data: " + data); break;
                }
            });
        }
        #region Test Data
        const string IntlCompletionsTest = @"
I|global|;
Intl.|Intl|;
Intl.Collator.|IntlConstructor|;
Intl.NumberFormat.|IntlConstructor|;
Intl.DateTimeFormat.|IntlConstructor|;

var collator = new Intl.Collator();
collator.|Collator|;
collator.compare.|BoundFunction|;
collator.resolvedOptions.|Function|;

var numberFormat = new Intl.NumberFormat();
numberFormat.|IntlFormat|;
numberFormat.format.|BoundFunction|;
numberFormat.resolvedOptions.|Function|;

var dateTimeFormat = new Intl.DateTimeFormat();
dateTimeFormat.|IntlFormat|;
dateTimeFormat.format.|BoundFunction|;
dateTimeFormat.resolvedOptions.|Function|;
";
        #endregion

        [TestMethod]
        [WorkItem(508892)]
        public void MapWeakMapAndSetCompletions()
        {
            PerformCompletionRequests(MapWeakMapAndSetCompletionsTest, (completions, data, i) =>
            {
                switch (data)
                {
                    case "global": completions.ExpectContains("Map", "WeakMap", "Set"); break;
                    case "Map": completions.ExpectContains(MapMembers); break;
                    case "WeakMap": completions.ExpectContains(WeakMapMembers); break;
                    case "Set": completions.ExpectContains(SetMembers); break;
                    case "Number": completions.ExpectContains(NumberMethods); break;
                    case "Function": completions.ExpectContains(BoundFunctionMethods); break;
                    default: Assert.Fail("Unhandled data: " + data); break;
                }
            });
        }
        #region Test Data
        const string MapWeakMapAndSetCompletionsTest = @"
M|global|;
var map = new Map();
map.|Map|;
map.get.|Function|;
map.set.|Function|;
map.has.|Function|;
map.clear.|Function|;
map.delete.|Function|;
map.forEach.|Function|;
map.size.|Number|;

map = new WeakMap();
map.|WeakMap|;
map.get.|Function|;
map.set.|Function|;
map.has.|Function|;
map.delete.|Function|;

var set = new Set();
set.|Set|;
set.add.|Function|;
set.has.|Function|;
set.clear.|Function|;
set.delete.|Function|;
set.forEach.|Function|;
set.size.|Number|;
";
        #endregion

        [TestMethod]
        [WorkItem(665287)]
        public void Bug665287()
        {
            InMTA(() =>
            {
                var mySession = new AuthorTestSession(Directory.GetCurrentDirectory());
                try
                {
                    // Ensure the following no longer crashes.
                    var file = mySession.FileFromText(Bug665287_Text);
                    var context = mySession.OpenContext(file);
                    using (CallHurryIn(context, 250))
                    {
                        var completions = context.GetCompletionsAt(Bug665287_Text.Length);
                        Assert.IsNotNull(completions);
                        Marshal.ReleaseComObject(completions);
                    }
                }
                finally
                {
                    mySession.Close();
                }

            });
        }
        #region Test Data
        const string Bug665287_Text = @"var obj = {};
var x = function(){return this};
var print = function(){return this};
try{var shouldBailout = (WScript.Arguments[0] == 'bailout');}catch(e){};
//fuzzSeed = 98920083;
(function(){/*sStart*/;""use strict""; print(x); '' ;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*bLoop*/for (ccofow = 0, x; ccofow < 15; ++ccofow) { if (ccofow % 2 == 0) { (this); } else { print(x); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (var trycny = 0; trycny < 8 && null; ++trycny) { var w = trycny;  } ;;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (otmoyz = 0, a = x; otmoyz < 3 && (""u775D"".eval( /x/ ))['_' + (x)]; ++otmoyz) { c = otmoyz; print(x); } ;;/*sEnd*/})();
(function(){/*sStart*/;var opn = Object.getOwnPropertyNames(@_jscript_version); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;print(-536870912);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; aecskq((x < a), x);/*hhh*/function aecskq(u3056){if(new XPCSafeJSObjectWrapper(y.__defineGetter__(""x"", Object.keys))) { if (x = ""u91AC"" <<  """" 
) throw window; else {print(x); }}};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*vLoop*/for (var dlmpmr = 0; dlmpmr < 7; ++dlmpmr) { var b = dlmpmr; (this) } ;;/*sEnd*/})();
(function(){/*sStart*/;{print( /x/g );{} };;/*sEnd*/})();
(function(){/*sStart*/;var qefyda = new ArrayBuffer(6); var qefyda_0 = new Uint8ClampedArray(qefyda); qefyda_0[0] = -2147483649; (({}));y = x;;;/*sEnd*/})();
(function(){/*sStart*/;var e =  '' ;print(window);;;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;/*hhh*/function henbnd(){/*wrap3*/(function(){ var pzwbis = window; (Object.freeze)(); })}henbnd();;;/*sEnd*/})();
(function(){/*sStart*/;throw x;;;/*sEnd*/})();
(function(){/*sStart*/;L:for(var c in ""uE315"") {print(NaN); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print('fafafa'.replace(/a/g,  /x/g ));;;/*sEnd*/})();
(function(){/*sStart*/;L: {print( '' );/*oLoop*/for (var wtrvnr = 0; wtrvnr < 2; ++wtrvnr) { print(null); }  };;/*sEnd*/})();
(function(){/*sStart*/;for(z in Math.ceil(1.3090966374572502e+308)) {print((Boolean(z)));print(x); };;/*sEnd*/})();
(function(){/*sStart*/;/*bLoop*/for (sgstds = 0; sgstds < 11; ++sgstds) { if (sgstds % 3 == 0) { -35184372088831; } else { print(x); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;{/*bLoop*/for (iiudng = 0, pnsqqq; iiudng < 19; ++iiudng) { if (iiudng % 5 == 0) { throw  /x/g ; } else { print(true); }  }  };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; M:@if (@_win16) print(arguments instanceof  /x/ ); @else print(x); @end ;;/*sEnd*/})();
(function(){/*sStart*/;try{try{try { {} } catch(x) { ( """" ); } finally { ""u2C17""; } }catch(e){}try{throw y;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);function x(){/*jjj*/}print(x);;;/*sEnd*/})();
(function(){/*sStart*/;gc();;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);(function(){return;})()
print(x);;;/*sEnd*/})();
(function(){/*sStart*/;print(((function fibonacci(wrqyxg) { ; if (wrqyxg <= 1) { ; return 1; } ; return fibonacci(wrqyxg - 1) + fibonacci(wrqyxg - 2);  })(6)));
M:if(((function factorial(dlgiqk) { ; if (dlgiqk == 0) return 1; (function(){return ""u4D58"";})();; return dlgiqk * factorial(dlgiqk - 1);  })(11))) { if ((eval = this)) {(function(){return;})(); } else {(""u90A0"");print(x); }}
;;/*sEnd*/})();
(function(){/*sStart*/;print(window);
print(x);
;;/*sEnd*/})();
(function(){/*sStart*/;/*tLoop*/for (var c in [ /x/g ,  /x/g ,  /x/g ]) { print(c); };;/*sEnd*/})();
(function(){/*sStart*/;print( /x/ );;;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (var dfrdwe = 0; dfrdwe < 8; ++dfrdwe) { print(""u28A1""); } ;;/*sEnd*/})();
(function(){/*sStart*/;x = linkedList(x, 112);;;/*sEnd*/})();
(function(){/*sStart*/;for (var p in x) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; (window);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var opn = Object.getOwnPropertyNames(x); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; a.b = x;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; L:switch(a) { case y + c: (""u16BB"");break; default: break; case ((function factorial_tail(gdxofq, esmckk) { ; if (gdxofq == 0) { /*oLoop*/for (ydtyam = 0; ydtyam < 1; ++ydtyam) { print(x); } ; return esmckk; } L:if(x.x) print(x); else  if (""u3C75"") {(false);print(x); }; return factorial_tail(gdxofq - 1, esmckk * gdxofq);  })(11, 1)): /*vLoop*/for (var phvkma = 0; phvkma < 8; ++phvkma) { var b = phvkma; (x); } break; switch((4277)) { case 8: print(x);break; default: case 2: print(x);break;  }break; break;  };;/*sEnd*/})();
(function(){/*sStart*/;print(((uneval(x))));;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (haulkf = 0; haulkf < 2; ++haulkf) { var a = haulkf; print(a); } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; M:switch((4277)) { case 4: (null);z = ((function too_much_recursion(skspqm) { ; if (skspqm > 0) { ; too_much_recursion(skspqm - 1); }  })(7217));break; case x | window: break; case 3: break;  };;/*sEnd*/})();
(function(){/*sStart*/;(uneval(Math[ '' ]));;/*sEnd*/})();
(function(){/*sStart*/;for (var p in get) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;if(shouldBailout){function shapeyConstructor(qcrncv){""use strict""; this.a = ScriptEngineMinorVersion;delete this.a;{  } this.a = ({a2:z2});return 2+this; }; shapeyConstructor(each);};;;/*sEnd*/})();
(function(){/*sStart*/;print([[1]]);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(""u4397"");print(x);;;/*sEnd*/})();
(function(){/*sStart*/;/*bLoop*/for (gqwcko = 0; gqwcko < 18; ++gqwcko,  /x/ ) { if (gqwcko % 2 == 1) { ""u44B1""; } else { print(x); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);;;/*sEnd*/})();
(function(){/*sStart*/;(""uB220"");function u3056(w, b){/*jjj*/}(({}));gc();(function(){return (x.__defineGetter__(""x"", Proxy.create));})();;;/*sEnd*/})();
(function(){/*sStart*/;gc();( /x/g );;;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (var nbdjsz = 0; nbdjsz < 3; ++nbdjsz) { print(x); } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(""uFDA3"") {(""u7DC1"");(function(){return  /x/ ;})(); } else  if (x) [,,z1];
/*vLoop*/for (bzufht = 0, gjwhki; bzufht < 11; ++bzufht, undefined) { var y = bzufht; null; } 
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print( """" );function a(){/*jjj*/}print(""u1ABD"");;;/*sEnd*/})();
(function(){/*sStart*/;try{try{try { try{throw StopIteration;}catch(e){} } catch(window) { try{let(w) ((function(){(function(){return  """" ;})();})());}catch(e){} } finally { try{with({}) (false);}catch(e){} } }catch(e){}try{""uE646"";}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;if(gczeal(0)) print(x);;;/*sEnd*/})();
(function(){/*sStart*/;L: {@if (@_win16) ((uneval(this))); @elif (@_mac) print(window <=  /x/ ); @else  @end  };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*bLoop*/for (xysgnp = 0; xysgnp < 22; ++xysgnp) { if (xysgnp % 2 == 1) { x;function e(window, NaN){/*jjj*/}print(x); } else { M:@if (@_win16) print(x); @else ""u9292""; @end  }  } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(x) { if (""uF5AA"") print( '' ); else { /x/g ; }};;/*sEnd*/})();
(function(){/*sStart*/;print(new (x)((a)[this]) &= (4277));;;/*sEnd*/})();
(function(){/*sStart*/;switch(([11,12,13,14].some)) { default: ([z1]);this;case 5: window; };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; ( '' );
print(x);
print(x);
/*tLoop*/for (var x in [new Boolean(false), (1/0),  /x/g , new Boolean(false),  /x/g , new Boolean(false), new Boolean(false), new Boolean(false), new Boolean(false),  /x/g ,  /x/g ,  /x/g ,  /x/g ]) {  '' ; }
;;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (var gdpttc = 0; gdpttc < 2; ++gdpttc) { print(d); } d = x;;;/*sEnd*/})();
(function(){/*sStart*/;try{print(x);}catch(e){}print( /x/ );;;/*sEnd*/})();
(function(){/*sStart*/;/*tLoop*/for (var x in [, new Boolean(true), , , , new Boolean(true), , , , new Boolean(true), , new Boolean(true), , , , new Boolean(true), new Boolean(true), new Boolean(true), new Boolean(true), new Boolean(true), new Boolean(true), new Boolean(true), , , , new Boolean(true), , , new Boolean(true), new Boolean(true), ]) { print(y); };;/*sEnd*/})();
(function(){/*sStart*/;for (var p in getter) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var w =  /x/g ;true;print(w);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*vLoop*/for (var pjkeqq = 0, cxjcgp; pjkeqq < 11; ++pjkeqq) { var c = pjkeqq; ( /x/ ); } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){{}};;;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;gczeal(0);function window(){	/*jjj*/} """" ;;;/*sEnd*/})();
(function(){/*sStart*/;/*hhh*/function yhwwwm(){var opn = Object.getOwnPropertyNames(w); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); }}yhwwwm();;;/*sEnd*/})();
(function(){/*sStart*/;print(false);function window(u3056, d){/*jjj*/}( """" );;;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (var qtpihe = 0; qtpihe < 12; ++qtpihe) { d = qtpihe; print(([11,12,13,14].sort)); } ;;/*sEnd*/})();
(function(){/*sStart*/;for (var p in @_jscript_version) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;for(var z in ((isFinite)(x))){if(shouldBailout){function shapeyConstructor(qeiakx){""use strict""; Object.seal(this.obj);if (-0) delete this.e;{ -67108863; } Object.preventExtensions(this);for (var ytqkgxrwj in this) { }return 2+this; }; shapeyConstructor(x);};(function(){return window;})(); };;/*sEnd*/})();
(function(){/*sStart*/;throw functional;{};;/*sEnd*/})();
(function(){/*sStart*/;var opn = Object.getOwnPropertyNames(x); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; try{try{this.zzz.zzz;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;var peaeff = new ArrayBuffer(0); var peaeff_0 = new Float32Array(peaeff); peaeff_0[0] = 0x2D413CCC; print( /x/ );;;/*sEnd*/})();
(function(){/*sStart*/;print(x);
print(x);
;;/*sEnd*/})();
(function(){/*sStart*/;var htwedb = new ArrayBuffer(3); var htwedb_0 = new Int32Array(htwedb); print(htwedb_0[0]); htwedb_0[0] = 1.3; var htwedb_1 = new Float64Array(htwedb); var htwedb_2 = new Int32Array(htwedb); print(htwedb_0);""uD96B"";c = (window.throw(-68719476736));print(((function factorial(xmnqzo) { ; if (xmnqzo == 0) return 1; ; return xmnqzo * factorial(xmnqzo - 1);  })(16)));print((Proxy.create)());( """" );//@cc_on Math.atan2(033,  '' );;;/*sEnd*/})();
(function(){/*sStart*/;with({a: -3.0985449848836315e+307}){ /x/g ; }	;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);;;/*sEnd*/})();
(function(){/*sStart*/;wavpao(((function a_indexing(efrtbk, pcvker) { ; if (efrtbk.length == pcvker) { ; return b; } var daiehu = efrtbk[pcvker]; var rgwluz = a_indexing(efrtbk, pcvker + 1); if(shouldBailout){function shapeyConstructor(onohmi){{ print(onohmi);z = (4277); } if (onohmi) this.w = x;for (var ytqwshvbs in this) { }if ((false++) -= [,,].unwatch(""w"")) this.x = ({});if (daiehu) this.d = onohmi | @_jscript_version;if (rgwluz) Object.defineProperty(this, ""c"", ({}));Object.preventExtensions(this);return 2+this; }; shapeyConstructor(y);}; })([new Boolean(true),  ""use strict"" ,  ""use strict"" ,  ""use strict"" ,  ""use strict"" ,  ""use strict"" , new Boolean(true),  ""use strict"" ,  ""use strict"" , new Boolean(true),  ""use strict"" ,  ""use strict"" ], 0)), x);/*hhh*/function wavpao(eval, x){eval = [,], fxxqay, ncmgiy, fjuybt, d, x, NaN, xxyaqr;print(x);};;/*sEnd*/})();
(function(){/*sStart*/;x//h
;({a2:z2});;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);
window;
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; for(var b in ((function(y) { return y })(({y: function shapeyConstructor(xkntzn){return 2+xkntzn; }, eval: (delete x.u3056) }))))print((4277));function NaN(x, u3056){/*jjj*/}if(shouldBailout){print(x);};;;/*sEnd*/})();
(function(){/*sStart*/;if(delete e.x) {( /x/g );print(72057594037927940); } else  if ((uneval(""uFEC2""))) ""uF5B1"";;/*sEnd*/})();
(function(){/*sStart*/;false;(this);;;/*sEnd*/})();
(function(){/*sStart*/;x;
(function(){return (eval(""\""uB014\""""));})();
;;/*sEnd*/})();
(function(){/*sStart*/;if(new (window)(bzoqvm, hafilv, z, eval, a, bgxyag, x, divrkr)) { if ( """" ) {(({a1:1}));print(x); } else {print([-524287]); }};;/*sEnd*/})();
(function(){/*sStart*/;var dyskvy;(x);;;/*sEnd*/})();
(function(){/*sStart*/;((uneval(1e4)));;;/*sEnd*/})();
(function(){/*sStart*/;for (var p in x) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (luqwev = 0; luqwev < 10; ++luqwev) { print(x); } (""u6B03"");;;/*sEnd*/})();
(function(){/*sStart*/;NaN = (4277), ogdfet;try{gc();(window);}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;for(var e = [z1] in -4503599627370497) {print((Proxy.create.prototype));this;
window;
 };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; (true);;;/*sEnd*/})();
(function(){/*sStart*/;if(shouldBailout){function shapeyConstructor(gxntbn){""use strict""; if (gxntbn) Object.preventExtensions(gxntbn);return 2+gxntbn; }; shapeyConstructor(x);};;;/*sEnd*/})();
(function(){/*sStart*/;gczeal(0)
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; new Function;(x);;;/*sEnd*/})();
(function(){/*sStart*/;print(((function factorial(ystcqc) { ; if (ystcqc == 0) return 1; ; return ystcqc * factorial(ystcqc - 1);  })(7)));;;/*sEnd*/})();
(function(){/*sStart*/;options('strict');;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var wrtywa = new ArrayBuffer(4); var wrtywa_0 = new Uint16Array(wrtywa); var wrtywa_1 = new Uint32Array(wrtywa); wrtywa_1[0] = 0x80000000; var wrtywa_2 = new Uint32Array(wrtywa); wrtywa_2[0] = 0; var wrtywa_3 = new Uint32Array(wrtywa); wrtywa_3[0] = 1.2665481695623367e+308; var wrtywa_4 = new Uint8ClampedArray(wrtywa); print(wrtywa_4[0]); wrtywa_4[0] = -281474976710657; var wrtywa_5 = new Float32Array(wrtywa); var wrtywa_6 = new WebGLFloatArray(wrtywa); wrtywa_6[0] = 524287; y =  '' ;([,,z1]);print(wrtywa_1[0]);/*tLoop*/for (var z in [true, new Boolean(false), new Boolean(false)]) { 1.4244923772760242e+308; }print(Uint32Array( /x/g , 0));print(wrtywa_5[0]);print(this++);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){{(function(){return;})(); }};;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);
gczeal(0);
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; (new (""u34FC"")(window, window));
print(x);
;;/*sEnd*/})();
(function(){/*sStart*/;x = linkedList(x, 210);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; L:for(	w in ((new Function)( '' )))/*oLoop*/for (var kutjff = 0; kutjff < 7; ++kutjff) { {} } ;;/*sEnd*/})();
(function(){/*sStart*/;var opn = Object.getOwnPropertyNames(x); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; L: 4611686018427388000;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; ( /x/ );;;/*sEnd*/})();
(function(){/*sStart*/;var wrlnqn = new ArrayBuffer(0); var wrlnqn_0 = new Int8Array(wrlnqn); print(wrlnqn_0[0]); wrlnqn_0[0] =  /x/ .__defineGetter__(""x"", NaN); var wrlnqn_1 = new Uint8Array(wrlnqn); functional;print(false);print(wrlnqn_0);""u57E8""; """" ;;;/*sEnd*/})();
(function(){/*sStart*/;/*iii*/{gczeal(0); }/*hhh*/function jbeqxd(){print(x);};;/*sEnd*/})();
(function(){/*sStart*/;if(this) { if (x) gc();
decodeURIComponenty =  /x/ ;
} else /*bLoop*/for (behaqo = 0; (null['_' + ([])]) && behaqo < 4 && x; ++behaqo, (4277)) { if (behaqo % 2 == 0) { print(x); } else { print(x);print(x); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x)
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; {};;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; for (var p in x) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*bLoop*/for (czcfoi = 0; czcfoi < 24; ++czcfoi) { if (czcfoi % 10 == 8) {  } else { print( /x/ ); }  } d = this;;;/*sEnd*/})();
(function(){/*sStart*/;{print(x.__defineSetter__(""x"", Object.getPrototypeOf)['_' + (( """"  ? [,] : function(id) { return id }))]); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var nwoply = new ArrayBuffer(3); var nwoply_0 = new Int16Array(nwoply); nwoply_0[0] = -274877906945; var nwoply_1 = new Int16Array(nwoply); print(nwoply_1[0]); nwoply_1[0] = -536870913; print( """" );;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*oLoop*/for (vvclcb = 0; vvclcb < 6; ++vvclcb) { /*tLoop*/for (var b in [new String('q'), new String('q'), new Number(1), new String('q'), new String('q'), new Boolean(false), new String('q'), false, new String('q'), false, false, new String('q'), new Number(1), new String('q'), new Boolean(false), new Boolean(false), new Boolean(false), new Number(1), false, new Number(1), new Boolean(false), new Boolean(false), new Boolean(false), new Boolean(false), new String('q'), new String('q'), new Boolean(false), false, false, new Boolean(false), false, new Boolean(false), new Number(1), new String('q'), new Boolean(false), new String('q'), new Boolean(false), false]) { print(length); } } ;;/*sEnd*/})();
(function(){/*sStart*/;{print(1.014781929029904e+308); };;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (var cwgvnv = 0; cwgvnv < 1; ++cwgvnv) { (-4194305); } ;;/*sEnd*/})();
(function(){/*sStart*/;{( """" .yoyo( '' )); };;/*sEnd*/})();
(function(){/*sStart*/;((Math.log(window)));;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*bLoop*/for (jjroac = 0; jjroac < 2 && eval(""undefined.__defineSetter__(\""d\"", ( \""\"" ).bind)""); ++jjroac) { if (jjroac % 4 == 1) { print(x); } else { L:switch(""u3D0B"") { default: break;  } }  } ;;/*sEnd*/})();
(function(){/*sStart*/;(window);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; try{ArrayBuffer}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var txlxeq = new ArrayBuffer(0); var txlxeq_0 = new Float32Array(txlxeq); print(txlxeq_0[0]); var txlxeq_1 = new WebGLIntArray(txlxeq); var txlxeq_2 = new Float64Array(txlxeq); (function(){return (4277);})();;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(true);;;/*sEnd*/})();
(function(){/*sStart*/;var x = delete (x =  '' ).__defineSetter__(""callee"", function(y) { return [] });{};;/*sEnd*/})();
(function(){/*sStart*/;if(shouldBailout){function shapeyConstructor(zarhve){""use strict""; return 2+this; }; shapeyConstructor(x);};
/*vLoop*/for (ukheix = 0; ukheix < 3; ++ukheix) { var a = ukheix; (function(){return;})(); } 
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; (window);function z(c){/*jjj*/}print(x);;;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (var vjejvm = 0; vjejvm < 6; ++vjejvm) { if(shouldBailout){function shapeyConstructor(azvwso){""use strict""; { ; } { print( """" ); } this.a = new Boolean(true);this.a = JSON.stringify;for (var ytqoflord in this) { }{ (32767); } return 2+this; }; shapeyConstructor(z);}; } ;;/*sEnd*/})();
(function(){/*sStart*/;if(shouldBailout){function shapeyConstructor(ofgzwg){for (var ytqgeegdo in this) { }return 2+this; }; shapeyConstructor(x);};;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; with({c: function(q) { return q; }.prototype++})print(x);;;/*sEnd*/})();
(function(){/*sStart*/;d = x;print(d);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*vLoop*/for (mseisl = 0; mseisl < 10; ++mseisl) { x = mseisl; (arguments); } ;;/*sEnd*/})();
(function(){/*sStart*/;print(((function sum_slicing(rxkzoj) { print(x);; return rxkzoj.length == 0 ? 0 : rxkzoj[0] + sum_slicing(rxkzoj.slice(1)); })([new Number(1), null, (0/0), new Number(1), (0/0), (0/0), new Number(1), (0/0), null, (0/0), (0/0), null, new Number(1), (0/0), null, null, new Number(1), null, null, new Number(1), null, null, (0/0), null, new Number(1), null, new Number(1), (0/0)])));;;/*sEnd*/})();
(function(){/*sStart*/;for (var p in x) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;var opn = Object.getOwnPropertyNames(window); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;/*tLoop*/for (var z in [0x50505050, 0x50505050, x, 0x50505050, new Number(1.5), 0x50505050, new Number(1.5), new Number(1.5), new Number(1.5), 0x50505050, new Number(1.5), x]) { print(x); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; x;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);""uE091"";
{}
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*bLoop*/for (var thptyw = 0; thptyw < 8; ++thptyw) { if (thptyw % 11 == 5) { print(x); } else { print(x); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;print(x);null;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; window = linkedList(window, 5810);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var opn = Object.getOwnPropertyNames(x); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;while((x.getVarDate(this)) && 0){print(x);print(x); }
(Function);
;;/*sEnd*/})();
(function(){/*sStart*/;{var d = ((p={}, (p.z =  """" )()) && x);/*oLoop*/for (var wlajam = 0; wlajam < 6; ++wlajam) { print(d); } print(d); };;/*sEnd*/})();
(function(){/*sStart*/;@if (@_win16) L:if(d) undefined; else  if (-33554433) print(x); else x; @else /*tLoop*/for (var a in [new Number(1.5),  ""use strict"" ,  ""use strict"" , 0/0, 0/0, new Number(1.5), (1/0), this, this, this, 0/0, 0/0, this,  ""use strict"" , 0/0,  ""use strict"" , this, 0/0,  ""use strict"" , this, this, new Number(1.5), (1/0), this, new Number(1.5), 0/0, 0/0, 0/0,  ""use strict"" , 0/0, new Number(1.5), (1/0), 0/0,  ""use strict"" , this, 0/0, new Number(1.5)]) { ([,,]); }this;
print(x);
print(x); @end ;;/*sEnd*/})();
(function(){/*sStart*/;(-131073);;;/*sEnd*/})();
(function(){/*sStart*/;var xzqoun = new ArrayBuffer(6); var xzqoun_0 = new Float64Array(xzqoun); print(x);Proxy.isTrappingprint(xzqoun);print(""uFB0F"");;;/*sEnd*/})();
(function(){/*sStart*/;print(x);//@cc_on ;;/*sEnd*/})();
(function(){/*sStart*/;var dfmoei = new ArrayBuffer(4); var dfmoei_0 = new Int32Array(dfmoei); print(dfmoei_0[0]);;;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (var voolqc = 0; voolqc < 4; ++voolqc, new XPCNativeWrapper(@_jscript_version)) { a = voolqc; ((""u879E"".throw(window))); } ;;/*sEnd*/})();
(function(){/*sStart*/;a = (4277).prototype;/*bLoop*/for (gmfqsa = 0;  ''  && gmfqsa < 18; ++gmfqsa) { if (gmfqsa % 4 == 2) { [,]; } else {  }  } ;;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (fnlyyr = 0; fnlyyr < 12; ++fnlyyr) { var x = fnlyyr; print(x); } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; {(""u171A""); };;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;for (var p in x) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;var pjkbpv = new ArrayBuffer(16); var pjkbpv_0 = new Float32Array(pjkbpv); print(pjkbpv_0[0]); pjkbpv_0[0] = 2251799813685249; eval(""-0.9.430489950842214e+307"", true);try{gczeal(0);}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; for (var p in y) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;throw new XPCSafeJSObjectWrapper( /x/g );;;/*sEnd*/})();
(function(){/*sStart*/;print(z);z = x;;;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (var aztzfw = 0; aztzfw < 2;  /x/ , ++aztzfw, ""u088F"") { switch([1]) { default: break;  } } ;;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){{print(this);print(x); }};;;/*sEnd*/})();
(function(){/*sStart*/;/*tLoop*/for (var a in [-Infinity, (Math.round(""u3DF6"")), new Boolean(false), (Math.round(""u3DF6"")), -Infinity, (4277), (Math.round(""u3DF6"")), -Infinity, (Math.round(""u3DF6"")), true, (Math.round(""u3DF6"")), new Boolean(false), (Math.round(""u3DF6"")), new Boolean(false), -Infinity, true, true, -Infinity, (4277), new Boolean(false), (Math.round(""u3DF6"")), (4277), (Math.round(""u3DF6"")), (4277)]) { print(""u42DF""); }
((new Math.pow(0, 4095)(x,  /x/g  in ""u7673"")));
;;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;a;;;/*sEnd*/})();
(function(){/*sStart*/;if((4277)) {print(x);print( /x/g ); } else  if (x) {/*vLoop*/for (kaxivu = 0; kaxivu < 5; ++kaxivu) { var d = kaxivu; print(w); }  };;/*sEnd*/})();
(function(){/*sStart*/;M:switch(new wrap((Math)[""u1842""])) { case 0: break; break; with([,,]){print(""u05A1""); }break; default:  };;/*sEnd*/})();
(function(){/*sStart*/;x.unwatch(""x"");;;/*sEnd*/})();
(function(){/*sStart*/;try{(""u67A4"");gczeal(0);}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;try{this;}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;if(shouldBailout){function shapeyConstructor(garhga){""use strict""; return 2+this; }; shapeyConstructor(c);};;;/*sEnd*/})();
(function(){/*sStart*/;var hysmiy = new ArrayBuffer(12); var hysmiy_0 = new Uint16Array(hysmiy); print(true);;;/*sEnd*/})();
(function(){/*sStart*/;var iuooke = new ArrayBuffer(12); var iuooke_0 = new Float32Array(iuooke); print(iuooke_0[0]); var iuooke_1 = new Float32Array(iuooke); iuooke_1[0] = 35184372088833; evalprint(-0);;;/*sEnd*/})();
(function(){/*sStart*/;print(3.141592653589793);
gc();
;;/*sEnd*/})();
(function(){/*sStart*/;/*tLoop*/for (var a in [ /x/ ,  /x/ , function(){}, -Infinity, function(){},  /x/ , function(){},  /x/ ,  /x/ , (void 0), -Infinity, (void 0), -Infinity, -Infinity,  /x/ , (void 0), -Infinity,  /x/ , function(){}, function(){}, function(){},  /x/ , -Infinity,  /x/ , function(){}, -Infinity, (void 0)]) { ""uC4DD""; }x;
'fafafa'.replace(/a/g, ScriptEngineBuildVersion);
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*oLoop*/for (stacfa = 0; stacfa < 4; ++stacfa) { print(x); } ;;/*sEnd*/})();
(function(){/*sStart*/;print((4277));;;/*sEnd*/})();
(function(){/*sStart*/;;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; try{try{u3056 = x;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; b = gczeal(0);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*vLoop*/for (xinluv = 0; xinluv < 2; ++xinluv) { x = xinluv; print(x); } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print((x ^ x).eval((Function.prototype.bind())));;;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;x;;;/*sEnd*/})();
(function(){/*sStart*/;var kavczf = new ArrayBuffer(16); var kavczf_0 = new Int32Array(kavczf); print(kavczf_0[0]); print(""uEF96"");(z);;;/*sEnd*/})();
(function(){/*sStart*/;d = ([]) = ""uF889"";print(x);;;/*sEnd*/})();
(function(){/*sStart*/;[1,,];;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; switch(Math.max((function shapeyConstructor(cmvvkc){""use strict""; if (cmvvkc) {  /x/g ; } { window; } { print(cmvvkc); } this.a = new Number(1.5);if (cmvvkc) delete this.y;if ( /x/g ) this.y = true;return 2+this; })(), eval(""null""))) { default: print(eval("" /x/ "", Math.atan2(1099511627777,  '' ))); };;/*sEnd*/})();
(function(){/*sStart*/;for (var p in c) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; try{try{throw functional;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; @if (@_win16) /*hhh*/function uvnooa(x){(x);}/*iii*/print(uvnooa); @else print(x); @end ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var usmjew = new ArrayBuffer(16); var usmjew_0 = new Float64Array(usmjew); var usmjew_1 = new Int32Array(usmjew); print(usmjew_1[0]); var usmjew_2 = new Float32Array(usmjew); var usmjew_3 = new Float32Array(usmjew); usmjew_3[0] = -68719476735; var usmjew_4 = new Uint16Array(usmjew); usmjew_4[0] = -1.217286023853593e+308; var usmjew_5 = new Int16Array(usmjew); usmjew_5[0] = 1e+81; usmjew_1;;;/*sEnd*/})();
(function(){/*sStart*/;@if (@_win32) var a = eval(""gczeal(0);"");(function(){return window;})(); @end ;;/*sEnd*/})();
(function(){/*sStart*/;print( /x/g );(function(){return this;})();
print(y);var y = this;
;;/*sEnd*/})();
(function(){/*sStart*/;print( """"  + ""uCC8C"");;;/*sEnd*/})();
(function(){/*sStart*/;(false);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; x;function x(){/*jjj*/}[z1,,];print(x);;;/*sEnd*/})();
(function(){/*sStart*/;if( /* Comment */(4277)) print((4277)); else {throw window;print(x);

 };;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (var hxappu = 0; hxappu < 5; ++hxappu) { print(this); } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; ;;/*sEnd*/})();
(function(){/*sStart*/;try{try{let(a) ((function(){try{{}}catch(e){}})());}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;try{gczeal(0);}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;/*bLoop*/for (var lmhsij = 0, gdghca, cegldj, jtrvjy; lmhsij < 17; ++lmhsij) { if (lmhsij % 2 == 1) { throw @_jscript_version % x instanceof ""u68E4""; } else { try{this;{}}catch(e){} }  } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var opn = Object.getOwnPropertyNames(x); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*tLoop*/for (var x in [function(){}, new Number(1.5), function(){}, function(){}, new Number(1.5), new Number(1.5), function(){}, new Number(1.5), new Number(1.5), function(){}, function(){}, function(){}, new Number(1.5), new Number(1.5), function(){}, function(){}, new Number(1.5), function(){}, function(){}, function(){}, new Number(1.5), new Number(1.5), new Number(1.5), function(){}, new Number(1.5), new Number(1.5), new Number(1.5), new Number(1.5), function(){}, function(){}, function(){}]) { print(({})(window)); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; x = ((function factorial_tail(nvyesh, fnflab) { ; if (nvyesh == 0) { ; return fnflab; } ; return factorial_tail(nvyesh - 1, fnflab * nvyesh); try{try{//@cc_on }catch(e){}}catch(e){} })(12, 1)), u3056 = ('fafafa'.replace(/a/g, Proxy.createFunction)), c = ({});print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var vjwyze = new ArrayBuffer(6); var vjwyze_0 = new Uint32Array(vjwyze); print(vjwyze_0[0]); vjwyze_0[0] = -1.2498444859242916e+308; var vjwyze_1 = new Uint8Array(vjwyze); print(vjwyze_1[0]); vjwyze_1[0] = 2251799813685248; print(x);gc();print(vjwyze_1[0]);(""u7369"");;;/*sEnd*/})();
(function(){/*sStart*/;if(b) { } else print( /x/ );//h

var opn = Object.getOwnPropertyNames(NaN); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); }
var z = (4277);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; try{try{(Object.defineProperty(x, ""a"", ({})));}catch(e){}try{throw StopIteration;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;x, nghler;/*hhh*/function zdpyoc(y){print(x);}zdpyoc(new (4277)(), (new XPCSafeJSObjectWrapper(Math)));print(y = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor'); var desc = Object.getOwnPropertyDescriptor(x); desc.configurable = true; return desc; }, getPropertyDescriptor: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor'); var desc = Object.getPropertyDescriptor(x); desc.configurable = true; return desc; }, defineProperty: function(name, desc) { var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty'); Object.defineProperty(x, name, desc); }, getOwnPropertyNames: undefined, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); if (Object.isFrozen(x)) { return Object.getOwnProperties(x); } }, has: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return name in x; }, hasOwn: function(name) { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return Object.prototype.hasOwnProperty.call(x, name); }, get: isNaN, set: this, iterate: Exception, enumerate: Object.isExtensible, keys: function(y) { return  """"  }, }; })({}), x));;;/*sEnd*/})();
(function(){/*sStart*/;/*bLoop*/for (icjftk = 0; icjftk < 20; function(id) { return id }, ++icjftk) { if (icjftk % 11 == 10) { gczeal(0); } else { throw x; }  } ;;/*sEnd*/})();
(function(){/*sStart*/;if(x) {print(this); } else  if ((ReferenceError(-1.4791686173987918e+308))) {null;print( /x/g ); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*hhh*/function nmijgp(d, z){/*tLoop*/for (var w in [undefined, undefined, undefined, undefined, 5.0000000000000000000000, function(){}, function(){}, function(){}, undefined, 5.0000000000000000000000, 5.0000000000000000000000, 5.0000000000000000000000, 5.0000000000000000000000, function(){}, undefined, undefined, undefined, 5.0000000000000000000000, undefined, function(){}, undefined, undefined, 5.0000000000000000000000, function(){}, 5.0000000000000000000000, 5.0000000000000000000000, 5.0000000000000000000000, undefined, function(){}, function(){}]) { throw  """" ; }}/*iii*/print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; M:switch(undefined) { case 1: break;  };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print('fafafa'.replace(/a/g, Math.atan));
print(x);
;;/*sEnd*/})();
(function(){/*sStart*/;var hnbton = new ArrayBuffer(16); var hnbton_0 = new WebGLFloatArray(hnbton); print(hnbton_0[0]); hnbton_0[0] = -131072; var hnbton_1 = new WebGLFloatArray(hnbton); print(hnbton_1[0]); hnbton_1[0] = new XPCSafeJSObjectWrapper(x = x); var hnbton_2 = new WebGLIntArray(hnbton); var hnbton_3 = new Int32Array(hnbton); print(hnbton_3[0]); hnbton_3[0] = 0; var hnbton_4 = new Uint8ClampedArray(hnbton); var w, NaN, fisxof;-1.3826582728684544e+308;var x = x;{}print(hnbton_1[0] = Proxy.createFunction((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: ArrayBuffer, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: Proxy.createFunction, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: enumerate'); return []; }, keys: Proxy.isTrapping, }; })(true), Object.freeze));/*hhh*/function dkijbo(){(hnbton_2[9]);}dkijbo();;;/*sEnd*/})();
(function(){/*sStart*/;print(x);print(x);;;/*sEnd*/})();
(function(){/*sStart*/;print( """" ());try{true;}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){function shapeyConstructor(ggubiz){if ( '' ) ggubiz.y = x;{ print( '' ); } ggubiz.a = b;delete ggubiz.b;{ print(x); } ggubiz.b = new Boolean(false);return 2+ggubiz; }; shapeyConstructor(x);};;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){function shapeyConstructor(kvixmx){return 2+this; }; shapeyConstructor(x);};;;/*sEnd*/})();
(function(){/*sStart*/;/*bLoop*/for (vatrlf = 0, blzocb; vatrlf < 17 &&  /x/g ; ++vatrlf) { if (vatrlf % 6 == 0) { ( """" ); } else { print(x); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;var plcirz = new ArrayBuffer(12); var plcirz_0 = new Int8Array(plcirz); plcirz_0[0] = 2199023255552; var plcirz_1 = new Int8Array(plcirz); var plcirz_2 = new WebGLIntArray(plcirz); print(plcirz_2[0]); plcirz_2[0] = 17; true;print( '' );function plcirz_2(plcirz_1){/*jjj*/}(""u0E4F"");gczeal(0);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);;;/*sEnd*/})();
(function(){/*sStart*/;L:@if (@_win16) /*bLoop*/for (var wzbjii = 0; wzbjii < 3 &&  '' ; ++wzbjii) { if (wzbjii % 6 == 5) {  """" ; } else {  }  }  @else /*tLoop*/for (var z in [new Number(1),  /x/g , function(){},  /x/g , new Number(1),  /x/g , x,  /x/g , function(){}, ['z']]) { print((4277)); } @end ;;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (dajfpt = 0; dajfpt < 5; ++dajfpt) { print(x); } ;;/*sEnd*/})();
(function(){/*sStart*/;switch(((function factorial_tail(mfrkfv, gacgnn) { try{( /x/ );0.1;}catch(e){}; if (mfrkfv == 0) { gc; return gacgnn; } {}; return factorial_tail(mfrkfv - 1, gacgnn * mfrkfv);  })(11, 1))) { case (this.eval(x)): with({d: [,,z1]}){this; /x/ ; }break;  };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*vLoop*/for (tvmxgj = 0; tvmxgj < 4; ++tvmxgj) { var b = tvmxgj; print((4277)); } ;;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (dqendn = 0; dqendn < 10; ++dqendn) { print(y); } ;;/*sEnd*/})();
(function(){/*sStart*/;this;print(x);
print((new Error(a, this)));
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*tLoop*/for (var z in [null, (1/0), (1/0), ({}), (x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: undefined, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: u3056, enumerate: undefined, keys: Object.seal, }; })(x), Object.getPrototypeOf)), (x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: undefined, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: u3056, enumerate: undefined, keys: Object.seal, }; })(x), Object.getPrototypeOf)), ({}), (x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: undefined, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: u3056, enumerate: undefined, keys: Object.seal, }; })(x), Object.getPrototypeOf)), (1/0), ({}), (x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: undefined, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: u3056, enumerate: undefined, keys: Object.seal, }; })(x), Object.getPrototypeOf)), (x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: undefined, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: u3056, enumerate: undefined, keys: Object.seal, }; })(x), Object.getPrototypeOf)), (x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: undefined, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: u3056, enumerate: undefined, keys: Object.seal, }; })(x), Object.getPrototypeOf)), null, (x = Proxy.createFunction((function handlerFactory() {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: function() { var yum = 'PCAL'; dumpln(yum + 'LED: has'); return false; }, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: undefined, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: u3056, enumerate: undefined, keys: Object.seal, }; })(x), Object.getPrototypeOf))]) {  ''  >>= 17592186044416;x; };;/*sEnd*/})();
(function(){/*sStart*/;try{try{;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;functional, xnzwsg, jiqrid, daqznl, x, y, x, yamypp, lbozge;(window);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*
*/with( /x/g )print(((c = [,,z1]).yoyo(new (x)())));;;/*sEnd*/})();
(function(){/*sStart*/;w = [,], z, y, emmflg;( '' );;;/*sEnd*/})();
(function(){/*sStart*/;with(eval(""this""))/*oLoop*/for (var smnpdx = 0; smnpdx < 11; ++smnpdx) { print(x); } ;;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (gkpkum = 0; gkpkum < 2; ++gkpkum) { (false); } ;;/*sEnd*/})();
(function(){/*sStart*/;/*tLoop*/for (var c in [null, new Boolean(false), null, new Boolean(false), new Boolean(false), new Boolean(false), new Boolean(false), new Boolean(false), new Boolean(false), new Boolean(false), (1/0), (1/0), null, new Boolean(false)]) { print(c); };;/*sEnd*/})();
(function(){/*sStart*/;var opn = Object.getOwnPropertyNames(z); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (var bcsync = 0; bcsync < 8; ++bcsync) { z = bcsync; /*oLoop*/for (var lmczhq = 0; lmczhq < 7; ++lmczhq) { print((4277)); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;switch( /x/g ) { case 0: print(x); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; ( /x/ );print(x);;;/*sEnd*/})();
(function(){/*sStart*/;throw 2199023255551;gc();;;/*sEnd*/})();
(function(){/*sStart*/;print(((4277).e));;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; //@cc_on 
( /x/g );
;;/*sEnd*/})();
(function(){/*sStart*/;try{try{x = x;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (llusiu = 0, z; llusiu < 2; ++llusiu) { /*bLoop*/for (var yhxfqe = 0; yhxfqe < 7; ++yhxfqe) { if (yhxfqe % 3 == 0) { (e); } else {  }  }  } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; x, x, x, pyrtlv, najlzk, aplnzo, x, vjiqfd;""u80BF"";;;/*sEnd*/})();
(function(){/*sStart*/;;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; do print(x); while((x) && 0);;;/*sEnd*/})();
(function(){/*sStart*/;/*bLoop*/for (zktram = 0; zktram < 21; ++zktram) { if (zktram % 10 == 5) { switch(new (w.create)()) { default: ;break; print(18014398509481984 -  '' );break;  } } else { print(eval(""print( \""\"" );""));
1125899906842623;
 }  } ;;/*sEnd*/})();
(function(){/*sStart*/;b = linkedList(b, 2193);;;/*sEnd*/})();
(function(){/*sStart*/;try{(function(){return;})();(""u5E72"");}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;{/*oLoop*/for (var ynkuhx = 0; ynkuhx < 1; ++ynkuhx) { function b(){/*jjj*/}(function(){return;})(); }  };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; {print(x); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print( ''  != ""uA626"");;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){print(x);};;;/*sEnd*/})();
(function(){/*sStart*/;try{try{try { (function(){return;})(); } catch(w) { (""u6ED2""); } }catch(e){}try{throw StopIteration;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; M:@if (@_win16) print(x);
print(this);

 @else print(""u9DDB"");print( """" ); @end ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; switch(x) { default: print(x);break; break; break; case  /x/ : case 0: case undefined: print(x);break; case (new XPCNativeWrapper(arguments)): var eval;throw this;break; case 3: print(x);break; print(x);break; case 7: break;  /x/g ; };;/*sEnd*/})();
(function(){/*sStart*/;L:with((4277)){try{([1]);(function(){return;})();}catch(e){}print(x); };;/*sEnd*/})();
(function(){/*sStart*/;;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print( '' );;;/*sEnd*/})();
(function(){/*sStart*/;( /x/ );
var functional = ""uE8C2"", x = eval("" \""\"" ""), x, ajotjc;a =  /x/ , w, a, mftses;print(x);
;;/*sEnd*/})();
(function(){/*sStart*/;qckhnb(this, ({a1:1}));/*hhh*/function qckhnb(y, x){{}};;/*sEnd*/})();
(function(){/*sStart*/;var klnrpk = new ArrayBuffer(4); var klnrpk_0 = new Int8Array(klnrpk);  """" ;true;print( /x/g );null;;;/*sEnd*/})();
(function(){/*sStart*/;if('fafafa'.replace(/a/g, isFinite)) {//@cc_on  } else {print(@_jscript_version); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(""u2D48"");;;/*sEnd*/})();
(function(){/*sStart*/;var opn = Object.getOwnPropertyNames(x); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;/*tLoop*/for (var d in [arguments.callee, eval]) { /*vLoop*/for (var fustfl = 0; fustfl < 8; ++fustfl) { var d = fustfl; (null); }  };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){{gczeal(0);print(this); }};;;/*sEnd*/})();
(function(){/*sStart*/;if(shouldBailout){(function(){return [,,];})();};try{try{;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;var sgqxrn = new ArrayBuffer(6); var sgqxrn_0 = new Uint32Array(sgqxrn); print(sgqxrn_0[0]);print( /x/ );print(sgqxrn_0[6]);print(x);function u3056(x){/*jjj*/}print(x);
print();
;;/*sEnd*/})();
(function(){/*sStart*/;L: {(window);print(window); };;/*sEnd*/})();
(function(){/*sStart*/;var d = (x.x);print(""uFFA6""--);;;/*sEnd*/})();
(function(){/*sStart*/;if(shouldBailout){{{} }};;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*bLoop*/for (gbpcaf = 0; gbpcaf < 2; ++gbpcaf) { if (gbpcaf % 3 == 2) { print([1,,]); } else { print(""u0997""); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;((4277));;;/*sEnd*/})();
(function(){/*sStart*/;/*tLoop*/for (var c in [ /x/g ,  ""use strict"" , 2,  ""use strict"" ,  ""use strict"" , null, null, null,  /x/g , null,  /x/g , 2,  ""use strict"" , 2,  ""use strict"" , 2, 2, null, null, 2, 2,  /x/g ,  ""use strict"" ,  ""use strict"" , null, 2]) { d;print(arguments); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; with(x)length
;;/*sEnd*/})();
(function(){/*sStart*/;for(var d = 3 in  """" )  """" ;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; (x), zhioob, coldvt, ekmxub, a, x, function shapeyConstructor(gjjttv){""use strict""; Object.defineProperty(gjjttv, new AttributeName(), ({}));gjjttv.w = ScriptEngine;delete gjjttv.w;for (var ytqqmcynq in gjjttv) { }{ //@cc_on  } return 2+gjjttv; }, window, z;[1].__proto__ = ""uB4AF"";;;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (hgvrmm = 0; hgvrmm < 4; ++hgvrmm) { print(window); } ;;/*sEnd*/})();
(function(){/*sStart*/;print(x);var b;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*oLoop*/for (gjouor = 0; gjouor < 9; ++gjouor) { -137438953473 } ;;/*sEnd*/})();
(function(){/*sStart*/;print(c % c);
print(x);
(x = e);function x(){/*jjj*/}x;gczeal(0);
print(x);print(((uneval([]))));
;;/*sEnd*/})();
(function(){/*sStart*/;print(x);((4277));;;/*sEnd*/})();
(function(){/*sStart*/;/*bLoop*/for (var uckygk = 0; uckygk < 4; ++uckygk) { if (uckygk % 6 == 2) { ((new b())); } else { z;Object.isExtensible }  } ;;/*sEnd*/})();
(function(){/*sStart*/;while(((4277)) && 0){( """" );//@cc_on  };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; (x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*bLoop*/for (mrhsxo = 0, b =  /x/ ; mrhsxo < 6; ++mrhsxo) { if (mrhsxo % 7 == 6) { gczeal(0); } else { (function(){return;})(); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;print(x);yield  /x/g ;;;/*sEnd*/})();
(function(){/*sStart*/;/*bLoop*/for (xemwgc = 0; xemwgc < 5; ++xemwgc) { if (xemwgc % 8 == 3) { print(x); } else { print(0xB504F332); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;print(eval(""new window(false)"", -281474976710657));;;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; function shapeyConstructor(zpllbi){{ print(zpllbi); } if (false) this.c =  /x/g ;{ (null); } for (var ytqlynjhi in this) { }if (b) this.b = true;delete this.c;this.a = (this).bind(arguments, y);return 2+this; }(x);;;/*sEnd*/})();
(function(){/*sStart*/;M://h
if(x) { /x/g ; } else  if (((eval(""\""u3B74\"""",  '' )).unwatch(""c"").valueOf(""number""))) {window;options('strict'); } else {/*bLoop*/for (var evmoix = 0; evmoix < 19; ++evmoix) { if (evmoix % 8 == 5) { Math; } else { arguments; }  }  };;/*sEnd*/})();
(function(){/*sStart*/;var z = d / NaN;(window);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*tLoop*/for (var c in [null, null, null, new String(''),  /x/ ,  /x/ , new String(''), null]) { print(c); }var b = 1e4;print(window);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; -7;
print(NaN);
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; for (var p in x) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (mscjmt = 0; mscjmt < 0; ++mscjmt) { var e = mscjmt; print( /* Comment */ /x/g ); } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){function shapeyConstructor(jgenxj){{ print( /x/ ); } { {gc(); } } jgenxj.c = new Boolean(true);Object.defineProperty(jgenxj, ""d"", ({value: gczeal(0), enumerable: undefined--}));jgenxj.z = [(void 0)];{ (window); } jgenxj.z = x;Object.preventExtensions(jgenxj);return 2+jgenxj; }; shapeyConstructor(set);};;;/*sEnd*/})();
(function(){/*sStart*/;print((Math.sqrt(-134217727)));;;/*sEnd*/})();
(function(){/*sStart*/;/*tLoop*/for (var c in [-Infinity, null, false, -Infinity,  """" , false,  """" , null, false, false,  """" ,  """" , -Infinity,  """" , false, -Infinity, -Infinity]) { (a); }function x(x){/*jjj*/}throw window;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(window);print(x);;;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;print((""uDA3E"".throw(functional %= window)));
print(x);
;;/*sEnd*/})();
(function(){/*sStart*/;print(delete x.x);function x(x){/*jjj*/}print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*tLoop*/for (var w in [NaN, 0x3FFFFFFF, NaN, 0x3FFFFFFF, x, true, 0x3FFFFFFF, 0x99, 0x99, 0x3FFFFFFF, true, 0x3FFFFFFF, x, 0x99, true, x, x, true, true, x, 0x3FFFFFFF, x, 0x99, 0x99]) { for (var p in x) { addPropertyName(p); } };;/*sEnd*/})();
(function(){/*sStart*/;print(( /x/g .valueOf(""number"")));
(({}));
;;/*sEnd*/})();
(function(){/*sStart*/;@if (@_win32) Object.isExtensible @end ;;/*sEnd*/})();
(function(){/*sStart*/;M:while((++x) && 0){(function(){return;})();print(@_jscript_version+=this); };;/*sEnd*/})();
(function(){/*sStart*/;try{try{;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;{print(x);for (var p in x) { addPropertyName(p); } };;/*sEnd*/})();
(function(){/*sStart*/;;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; (this.x.unwatch(""b""));;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*hhh*/function dgqfxf(){x, agfcku, z, @_jscript_version;window}/*iii*/(dgqfxf);
print(dgqfxf);
;;/*sEnd*/})();
(function(){/*sStart*/;var pvqweu = new ArrayBuffer(3); var pvqweu_0 = new Uint8Array(pvqweu); pvqweu_0[0] = false; print(x);;;/*sEnd*/})();
(function(){/*sStart*/;try{try{throw StopIteration;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){{print(this);{} }};;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; try{gczeal(0);(function(){return  /x/ ;})();}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;print(x);;;/*sEnd*/})();
(function(){/*sStart*/;try{try{x = y;}catch(e){}}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;{/*vLoop*/for (var eculji = 0; eculji < 8; ++eculji) { var e = eculji; print(""u5F3B""); } print(x);function x(){/*jjj*/}(function(){return;})(); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*bLoop*/for (var mfzpll = 0; mfzpll < 21; ++mfzpll) { if (mfzpll % 3 == 2) { (true); } else { ""uED6A""; }  } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);print(x);;;/*sEnd*/})();
(function(){/*sStart*/;if(shouldBailout){{(this); }};
{print(x);x; }
;;/*sEnd*/})();
(function(){/*sStart*/;(function(){return  '' ;})();function x(functional, c){/*jjj*/}gczeal(0);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(gczeal(0));print(x);;;/*sEnd*/})();
(function(){/*sStart*/;print(x);function y(){/*jjj*/}print(x);;;/*sEnd*/})();
(function(){/*sStart*/;print(d);var d =  /* Comment */window;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; for(var d = x in -68719476736) {gczeal(0); /x/ ; };;/*sEnd*/})();
(function(){/*sStart*/;throw true;
try{/*
*/(function(){return d;})();( '' );}catch(e){}
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; /*iii*/print(x);/*hhh*/function kddmmr(c){print(c);};;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (var zfiozn = 0; zfiozn < 3; ++zfiozn) { print(""uBF59""); } 
switch(d = Proxy.createFunction((function handlerFactory(x) {return {getOwnPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyDescriptor');}, getPropertyDescriptor: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: getPropertyDescriptor');}, defineProperty: function(){ var yum = 'PCAL'; dumpln(yum + 'LED: defineProperty');}, getOwnPropertyNames: function() { var yum = 'PCAL'; dumpln(yum + 'LED: getOwnPropertyNames'); return []; }, fix: function() { var yum = 'PCAL'; dumpln(yum + 'LED: fix'); return []; }, has: undefined, hasOwn: function() { var yum = 'PCAL'; dumpln(yum + 'LED: hasOwn'); return false; }, get: function() { var yum = 'PCAL'; dumpln(yum + 'LED: get'); return undefined }, set: function() { var yum = 'PCAL'; dumpln(yum + 'LED: set'); return true; }, iterate: function() { var yum = 'PCAL'; dumpln(yum + 'LED: iterate'); return (function() { throw StopIteration; }); }, enumerate: Array.reduce, keys: undefined, }; })(c), Object.seal)) { default:  }
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(new ( '' )(""u1DFB"", ""u4AC0""));;;/*sEnd*/})();
(function(){/*sStart*/;{print(new  '' ());print(x); };;/*sEnd*/})();
(function(){/*sStart*/;{(b);print(x); };;/*sEnd*/})();
(function(){/*sStart*/;try{eval}catch(e){};;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var opn = Object.getOwnPropertyNames(x); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;b = linkedList(b, 3036);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; for (var p in w) { addPropertyName(p); };;/*sEnd*/})();
(function(){/*sStart*/;gc();
print( '' );
do print(x); while(((new XPCNativeWrapper((4277)++)['_' + (undefined)])) && 0);;;/*sEnd*/})();
(function(){/*sStart*/;var cejrli = new ArrayBuffer(12); var cejrli_0 = new Int32Array(cejrli); print(cejrli_0[0]); cejrli_0[0] = 2305843009213694000; print(cejrli);;;/*sEnd*/})();
(function(){/*sStart*/;(function(){return  /* Comment */y;})();;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){x =  """" ;};;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print( /x/ );function x(x){/*jjj*/}print(x);;;/*sEnd*/})();
(function(){/*sStart*/;eval, NaN = {} = Math.sqrt(70368744177663), NaN;throw  /x/ ;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; (x);
new ""uE66E""(""u9318"", ""uB49A"");
;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(x);print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){{print(x); }};;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var d = Math.pow( /x/ , ""u032F"".yoyo(this));print(d);;;/*sEnd*/})();
(function(){/*sStart*/;(this);;;/*sEnd*/})();
(function(){/*sStart*/;x, x;this;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if(shouldBailout){{(""u9623"");throw [1]; }};;;/*sEnd*/})();
(function(){/*sStart*/;var diwuhl = new ArrayBuffer(8); var diwuhl_0 = new Int32Array(diwuhl); diwuhl_0[0] = -536870913; gc();/*tLoop*/for (var e in [undefined, (void 0), (void 0), (void 0), x, (-1/0), x, (void 0), (-1/0)]) { (window);
print(x);
 };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; with({w: new Date((
 /x/  ? null : -1099511627777), x|= /x/g )}){/*bLoop*/for (var dshyuj = 0; dshyuj < 13; ++dshyuj) { if (dshyuj % 3 == 2) { print(this); } else { gc(); }  }  };;/*sEnd*/})();
(function(){/*sStart*/;print(x);w = this;;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; if([11,12,13,14].sort) {print(x);print(eval(""x"")); } else {for (var p in x) { addPropertyName(p); }gc(); };;/*sEnd*/})();
(function(){/*sStart*/;/*oLoop*/for (nzdexg = 0, gichqb, ([ """" ].some(ScriptEngineMinorVersion)); nzdexg < 6; ++nzdexg) { /*hhh*/function flyryx(z){print(z);}/*iii*/ /x/g ; } ;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; with(new (parseFloat)(""u24B0"", null)){var b = 36893488147419104000;print( /x/g );{} };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; {M:with(window = -2.948880492438008e+307)print((Math.min(NaN, -2199023255552))); }
(a = x);var y = x;
;;/*sEnd*/})();
(function(){/*sStart*/;((Math.asin(x)));print(y);function window(){/*jjj*/}(""u1D91"");var y = x;;;/*sEnd*/})();
(function(){/*sStart*/;{try{true;//@cc_on }catch(e){}(this);//@cc_on  };;/*sEnd*/})();
(function(){/*sStart*/;b = Math.atan2(-72057594037927940,  """" ), y = 18014398509481984, nbcwbf, eval, muxlzo;;;/*sEnd*/})();
(function(){/*sStart*/;L:with({d: (x.throw(x))}){(function(){return;})(); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; print(new (/*wrap3*/(function(){ var woaskq = undefined; (Object.preventExtensions)(); }))(""u89EE""));;;/*sEnd*/})();
(function(){/*sStart*/;/*hhh*/function fukmen(x){//@cc_on }fukmen( /x/g );;;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (var qaklqh = 0; qaklqh < 8; ++qaklqh) { d = qaklqh; print(d);function x(){/*jjj*/}{} } ;;/*sEnd*/})();
(function(){/*sStart*/;/*vLoop*/for (xodxrf = 0; xodxrf < 10; ++xodxrf) { var d = xodxrf; /*oLoop*/for (cbhmbo = 0; cbhmbo < 5; ++cbhmbo) { print( /x/ ); }  } ;;/*sEnd*/})();
(function(){/*sStart*/;print(eval(""window"", true));;;/*sEnd*/})();
(function(){/*sStart*/;if(shouldBailout){{print(x); }};;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; var opn = Object.getOwnPropertyNames(x); for (var j = 0; j < opn.length; ++j) { addPropertyName(opn[j]); };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; with( /x/g ){window; };;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; ""use strict""; /*hhh*/function owhywo(x, e){(x);}/*iii*/print( '' );
ScriptEngine
new XPCNativeWrapper(window);

;;/*sEnd*/})();
(function(){/*sStart*/;/*hhh*/function uiykjy(a){print(this);}/*iii*/( """" );print(x);;;/*sEnd*/})();
(function(){/*sStart*/;""use strict""; c >>>= b((undefined.watch(""d"", ScriptEngine)), x);;;/*sEnd*/})();
(function(){/*sStart*/;;/*sEnd*/})();
(function(){
WScript.Echo(""\n\n"");
    try{
        for (var i in this){
            _fnCnt = 0;
            WScript.Echo(i + "" = "" + (typeof(this[i])==""function"" ? ""function"" : this[i]));
            for ( var j in this[i]){
                if (typeof(this[i][j]) == ""function"") _fnCnt++;
                else if (typeof(this[i][j]) == ""object"")
                    for (var z in this[i][j]) WScript.Echo(i + ""."" + j + ""."" + z + "" = "" + this[i][j][z]);
           }
           WScript.Echo(""No of functions = "" + _fnCnt);
        }
    }
    catch(e){}
})();
";
        #endregion

        [TestMethod]
        [WorkItem(626155)]
        public void Bug626155()
        {
            var primaryFile = _session.FileFromText(Bug626155_Text);
            var context = _session.OpenContext(primaryFile);
            var completions = context.GetCompletionsAt(primaryFile.Text.Length);
            Assert.IsNotNull(completions);
        }
        #region Test data
        const string Bug626155_Text = @"function print(s) {};
try { eval(""a = (new ((Math.asin(this.__defineGetter__(\""x\"", function shapeyConstructor(kfqudv){\""use strict\""; if (window) { print(x); } this.e = NaN;if (kfqudv) Object.seal(this);this.y = kfqudv;this.d =  /x/g ;Object.defineProperty(this, \""eval\"", ({configurable: false}));delete this.d;Object.freeze(this);return this; }))))(z));print(functional);""); } catch (ex) { }";
        #endregion

        [TestMethod]
        public void EnsureEvalDoesNotStopHurry()
        {
            InMTA(() =>
            {
                var mySession = new AuthorTestSession(Directory.GetCurrentDirectory());
                try
                {
                    var primaryFile = mySession.FileFromText(EnsureEvalDoesNotStopHurry_Text);
                    var context = mySession.OpenContext(primaryFile);
                    using (var phaseReported = ExecutionLimiter(context))
                    {
                        var completions = context.GetCompletionsAt(primaryFile.Text.Length);
                        Assert.IsNotNull(completions);
                        completions.ToEnumerable().ExpectContains(NumberMethods);
                        Assert.IsTrue(phaseReported.ExecuteCount < 100);
                    }
                }
                finally
                {
                    mySession.Close();
                }
            });
        }
        #region Test data
        const string EnsureEvalDoesNotStopHurry_Text = @"
        var a = 1;
        var b = 2;
        var c = 1;
        do {
            c = eval('a + b + ' + c);
        } while(1);
        a.";
        #endregion

        [TestMethod]
        [WorkItem(841798)]
        [TestCategory("DiagnosticFlags")]
        public void VerifyInferredTypesCorrectlyFlagged()
        {
            var primary = _session.FileFromText(VerifyInferredTypesCorrectlyMarked_Text);
            var context = _session.OpenContext(primary);
            var completionString = "obj.";
            var completions = context.GetCompletionsAt(primary.Text.IndexOf(completionString) + completionString.Length);
            Assert.AreEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfObjectTypeInferred,
                "Completion set should NOT be flagged as having an inferred object type");
            Assert.AreNotEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfMemberTypeInferred,
                "Completion set should be flagged as having an inferred member type");

            completionString = "obj.x.";
            completions = context.GetCompletionsAt(primary.Text.IndexOf(completionString) + completionString.Length);
            Assert.AreNotEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfObjectTypeInferred,
                "Completion set should be flagged as having an inferred object type");
            Assert.AreEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfMemberTypeInferred,
                "Completion set should NOT be flagged as having an inferred member type");
        }
        #region Test data
        const string VerifyInferredTypesCorrectlyMarked_Text = @"
        var obj = {
            /// <field name='x' type='Number' />
            x = null;
        };
        obj.x.
        ";
        #endregion

        [TestMethod]
        [WorkItem(841798)]
        [Ignore] // Disabled due to flakiness of timing dependent test. Re-enable when task 874350 completed.
        [TestCategory("DiagnosticFlags")]
        [TestProperty("IsolationLevel", "Method")]
        [TestProperty("SNAP", "No")]
        public void VerifyContextFileTimeoutCorrectlyFlagged()
        {
            InMTA(() =>
            {
                var mySession = new AuthorTestSession(Directory.GetCurrentDirectory());
                try
                {
                    var primary = mySession.FileFromText("");
                    var dom = mySession.ReadFile(Paths.DomWebPath);
                    var context = mySession.OpenContext(primary, dom);

                    IAuthorCompletionSet completions = null;

                    using (IDisposable hurry = ExecutionLimiter(context, 1))
                    {
                        completions = context.GetCompletionsAt(0);
                    }

                    Assert.IsNotNull(completions);
                    Assert.AreEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfPrimaryFileHalted,
                        "Completion set should NOT be flagged as having halted during execution of the primary file");
                    Assert.AreNotEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfContextFileHalted,
                        "Completion set should be flagged as having halted during execution of a context file");
                }
                finally
                {
                    mySession.Close();
                }
            });
        }

        [TestMethod]
        [WorkItem(841798)]
        [Ignore] // Disabled due to flakiness of timing dependent test. Re-enable when task 874350 completed.
        [TestCategory("DiagnosticFlags")]
        [TestProperty("IsolationLevel", "Method")]
        [TestProperty("SNAP", "No")]
        public void VerifyPrimaryFileTimeoutCorrectlyFlagged()
        {
            InMTA(() =>
            {
                var mySession = new AuthorTestSession(Directory.GetCurrentDirectory());
                try
                {
                    var dom = mySession.ReadFile(Paths.DomWebPath);
                    var context = mySession.OpenContext(dom);

                    IAuthorCompletionSet completions = null;

                    using (IDisposable hurry = ExecutionLimiter(context, 1))
                    {
                        completions = context.GetCompletionsAt(dom.Text.Length);
                    }

                    Assert.IsNotNull(completions);
                    Assert.AreEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfSuccessfulExecution,
                        "Completion set should NOT be flagged as having succesfully hit the breakpoint");
                    Assert.AreNotEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfPrimaryFileHalted,
                        "Completion set should be flagged as having halted during execution of the primary file");
                    Assert.AreEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfContextFileHalted,
                        "Completion set should NOT be flagged as having halted during execution of a context file");
                }
                finally
                {
                    mySession.Close();
                }
            });
        }

        [TestMethod]
        [WorkItem(841798)]
        [Ignore] // Disabled due to flakiness of timing dependent test. Re-enable when task 874350 completed.
        [TestCategory("DiagnosticFlags")]
        [TestProperty("IsolationLevel", "Method")]
        [TestProperty("SNAP", "No")]
        public void VerifyPrimaryAndContextFileTimeoutCorrectlyFlagged()
        {
            InMTA(() =>
            {
                var mySession = new AuthorTestSession(Directory.GetCurrentDirectory());
                try
                {
                    var primary = mySession.FileFromText(TestFiles.winrt);
                    var dom = mySession.ReadFile(Paths.DomWindowsPath);
                    var context = mySession.OpenContext(primary, dom);

                    IAuthorCompletionSet completions = null;

                    using (IDisposable hurry = ExecutionLimiter(context, 1))
                    {
                        completions = context.GetCompletionsAt(primary.Text.Length);
                    }

                    Assert.IsNotNull(completions);
                    Assert.AreEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfSuccessfulExecution,
                        "Completion set should NOT be flagged as having succesfully hit the breakpoint");
                    Assert.AreNotEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfPrimaryFileHalted,
                        "Completion set should be flagged as having halted during execution of the primary file");
                    Assert.AreNotEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfContextFileHalted,
                        "Completion set should be flagged as having halted during execution of a context file");
                }
                finally
                {
                    mySession.Close();
                }
            });
        }

        [TestMethod]
        [WorkItem(841801)]
        [TestCategory("DiagnosticCallback")]
        public void CallbackInvokedForUndefinedObjectResult()
        {
            var primary = _session.FileFromText(CallbackInvokedForUndefinedObjectResult_Text);
            var context = _session.OpenContext(primary);
            int timesInvoked = 0;
            context.SetCompletionDiagnosticCallback(() =>
                {
                    timesInvoked++;
                });
            var completions = context.GetCompletionsAt(primary.Text.Length);

            Assert.AreEqual(1, timesInvoked, "The callback should have been invoked exactly once.");
        }
        #region Test data
        const string CallbackInvokedForUndefinedObjectResult_Text = @"
        var a;
        a.";
        #endregion

        [TestMethod]
        [WorkItem(841801)]
        [TestCategory("DiagnosticCallback")]
        public void CallbackInvokedForNullObjectResult()
        {
            var primary = _session.FileFromText(CallbackInvokedForNullObjectResult_Text);
            var context = _session.OpenContext(primary);
            int timesInvoked = 0;
            context.SetCompletionDiagnosticCallback(() =>
            {
                timesInvoked++;
            });
            var completions = context.GetCompletionsAt(primary.Text.Length);

            Assert.AreEqual(1, timesInvoked, "The callback should have been invoked exactly once.");

            // Reset the diagnostics callback
            context.SetCompletionDiagnosticCallback(null);
        }
        #region Test data
        const string CallbackInvokedForNullObjectResult_Text = @"
        var a = null;
        a.";
        #endregion

        [TestMethod]
        [WorkItem(841801)]
        [TestCategory("DiagnosticCallback")]
        public void CallbackInvokedForExceptionObjectResult()
        {
            var primary = _session.FileFromText(CallbackInvokedForExceptionObjectResult_Text);
            var context = _session.OpenContext(primary);
            int timesInvoked = 0;
            context.SetCompletionDiagnosticCallback(() =>
            {
                timesInvoked++;
            });
            var completions = context.GetCompletionsAt(primary.Text.Length);

            Assert.AreEqual(1, timesInvoked, "The callback should have been invoked exactly once.");

            // Reset the diagnostics callback
            context.SetCompletionDiagnosticCallback(null);
        }
        #region Test data
        const string CallbackInvokedForExceptionObjectResult_Text = @"
        var foo;
        foo().";
        #endregion

        [TestMethod]
        [WorkItem(841801)]
        [TestCategory("DiagnosticCallback")]
        public void CallbackNotInvokedForObjectResult()
        {
            var primary = _session.FileFromText(CallbackNotInvokedForObjectResult_Text);
            var context = _session.OpenContext(primary);
            int timesInvoked = 0;
            context.SetCompletionDiagnosticCallback(() =>
            {
                timesInvoked++;
            });
            var completions = context.GetCompletionsAt(primary.Text.Length);

            Assert.AreEqual(0, timesInvoked, "The callback should NOT have been invoked.");

            // Reset the diagnostics callback
            context.SetCompletionDiagnosticCallback(null);
        }
        #region Test data
        const string CallbackNotInvokedForObjectResult_Text = @"
        var a = 42;
        a.";
        #endregion

        [TestMethod]
        [WorkItem(841801)]
        [TestCategory("DiagnosticCallback")]
        public void CallbackNotInvokedForExceptionsResolvedBySetTimeout()
        {
            var primary = _session.FileFromText(CallbackNotInvokedForExceptionsResolvedBySetTimeout_Text);
            var dom = _session.ReadFile(Paths.DomWebPath);
            var context = _session.OpenContext(primary, dom);
            int timesInvoked = 0;
            context.SetCompletionDiagnosticCallback(() =>
            {
                timesInvoked++;
            });
            var completions = context.GetCompletionsAt(primary.Text.Length);

            Assert.AreEqual(0, timesInvoked, "The callback should NOT have been invoked.");

            // Reset the diagnostics callback
            context.SetCompletionDiagnosticCallback(null);
        }
        #region Test data
        const string CallbackNotInvokedForExceptionsResolvedBySetTimeout_Text = @"
        var foo;

        setImmediate(function() {
            foo = function () { return 42; }
        });

        foo().";
        #endregion

        [TestMethod]
        [WorkItem(575783)]
        public void CompletionsAfterIntlCalls()
        {
            PerformCompletionRequests(@"
            new Date().toLocaleString().|String|;
            new Date().toLocaleDateString().|String|;
            new Date().toLocaleTimeString().|String|;
            new String().localeCompare().|Number|;
            new Number().toLocaleString().|String|;
            ");
        }

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassNameInGlobalCompletion()
        {
            PerformCompletionRequests(ClassNameInGlobalCompletionTest);
        }
        #region Test Data
        const string ClassNameInGlobalCompletionTest = @"
                class Empty { }
                ^|Empty|
                ";
        #endregion

        
        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void BasicClassCompletion()
        {
            PerformCompletionRequests(BasicClassCompletionTest);
        }
        #region Test Data
        const string BasicClassCompletionTest = @"
                class Empty { }
                Empty.|Function|;
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassMemberCompletion()
        {
            PerformCompletionRequests(ClassMemberCompletionTest);
        }
        #region Test Data
        const string ClassMemberCompletionTest = @"
                class WithMembers {
                    constructor() {
                        this._b = ""hello"";
                    }

                    method1() { }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() { return this._b; }
                    set b(v) { this._b = v; }
                }
                var instance = new WithMembers();
                instance.|Object|;
                instance.|method1,method2,method3,a,b|;

                instance.method1.|Function|;
                instance.method2.|Function|;
                instance.method3.|Function|;

                instance.a.|Number|;
                instance.b.|String|;
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassStaticMemberCompletion()
        {
            PerformCompletionRequests(ClassStaticMemberCompletionTest);
        }
        #region Test Data
        const string ClassStaticMemberCompletionTest = @"
                class WithStaticMembers {
                    static method1() { }
                    static method2(a) { return a; }
                    static method3(a,b) { return a+b; }

                    static get a() { return 42; }
                    static get b() { return this._b; }
                    static set b(v) { this._b = v; }
                }
                WithStaticMembers.|Function|;
                WithStaticMembers.|method1,method2,method3,a,b|;

                WithStaticMembers.method1.|Function|;
                WithStaticMembers.method2.|Function|;
                WithStaticMembers.method3.|Function|;

                WithStaticMembers.a.|Number|;
                WithStaticMembers.b.|!Object|;
                WithStaticMembers.b = ""hello"";
                WithStaticMembers.b.|String|;
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassInstanceAndStaticMemberCompletion()
        {
            PerformCompletionRequests(ClassInstanceAndStaticMemberCompletionTest);
        }
        #region Test Data
        const string ClassInstanceAndStaticMemberCompletionTest = @"
                class WithInstanceAndStaticMembers {
                    constructor() {
                        this._b = ""hello"";
                    }

                    method1() { }
                    method2(a) { return a; }
                    static method3(a,b) { return a+b; }

                    static get a() { return 42; }
                    get b() { return this._b; }
                    set b(v) { this._b = v; }
                }
                WithInstanceAndStaticMembers.|Function|;
                WithInstanceAndStaticMembers.|!method1,!method2,method3,a,!b|;

                WithInstanceAndStaticMembers.method3.|Function|;
                WithInstanceAndStaticMembers.a.|Number|;

                var instance = new WithInstanceAndStaticMembers();
                instance.|Object|;
                instance.|method1,method2,!method3,!a,b|;

                instance.method1.|Function|;
                instance.method2.|Function|;
                instance.b.|String|;
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassConstructor()
        {
            PerformCompletionRequests(CompletionsInClassConstructorTest);
        }
        #region Test Data
        const string CompletionsInClassConstructorTest = @"
            {
                var answer = 42;

                class CtorCompletions {
                    constructor(b) {
                        this._b = b;
                        ^|answer,CtorCompletions|
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                    }

                    method1() { }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() { return this._b; }
                    set b(v) { this._b = v; }
                }
            }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassConstructorWithoutThisDotAssignments()
        {
            PerformCompletionRequests(CompletionsInClassConstructorWithoutThisDotAssignmentsTest);
        }
        #region Test Data
        const string CompletionsInClassConstructorWithoutThisDotAssignmentsTest = @"
                var answer = 42;

                class CtorCompletions {
                    constructor(b) {
                        ^|answer,CtorCompletions|
                        answer.|Number|;
                        this.|Object|;
                        this.|method1,method2,method3,a,b|;
                    }

                    method1() { }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() { return this._b; }
                    set b(v) { this._b = v; }
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassMembers()
        {
            PerformCompletionRequests(CompletionsInClassMembersTest);
        }
        #region Test Data
        const string CompletionsInClassMembersTest = @"
                var answer = 42;

                class MemberCompletions {
                    constructor() {
                        this._b = ""hello"";
                    }

                    method1() {
                        ^|answer,MemberCompletions|
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                    }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() {
                        ^|answer,MemberCompletions|
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b.|String|;
                        return this._b;
                    }
                    set b(v) {
                        ^|answer,MemberCompletions|
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b = v;
                    }
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassStaticMembers()
        {
            PerformCompletionRequests(CompletionsInClassStaticMembersTest);
        }
        #region Test Data
        const string CompletionsInClassStaticMembersTest = @"
                var answer = 42;

                class StaticMemberCompletions {
                    constructor() {
                        this._b = ""hello"";
                    }

                    static method1() {
                        ^|answer,StaticMemberCompletions|
                        answer.|Number|;
                        this.|Function|;
                    }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    static get a() {
                        ^|answer,StaticMemberCompletions|
                        answer.|Number|;
                        this.|Function|;
                        return 42;
                    }
                    static set a(v) {
                        ^|answer,StaticMemberCompletions|
                        answer.|Number|;
                        this.|Function|;
                    }
                    get b() { return this._b; }
                    set b(v) { this._b = v; }
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassExpr()
        {
            PerformCompletionRequests(CompletionsInClassExprTest);
        }
        #region Test Data
        const string CompletionsInClassExprTest = @"
                var answer = 42;

                var TestClass = class {
                    constructor() {
                        this._b = ""hello"";
                    }

                    method1() {
                        ^|answer,TestClass|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                    }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() {
                        ^|answer,TestClass|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b.|String|;
                        return this._b;
                    }
                    set b(v) {
                        ^|answer,TestClass|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b = v;
                    }

                    static method4() {
                        ^|answer,TestClass|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                    static get c() {
                        ^|answer,TestClass|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                    static set c() {
                        ^|answer,TestClass|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInPrototypeMemberAddedToClass()
        {
            PerformCompletionRequests(CompletionsInPrototypeMemberAddedToClassTest);
        }
        #region Test Data
        const string CompletionsInPrototypeMemberAddedToClassTest = @"
                var answer = 42;

                class ExtraProtoMemberCompletions {
                    constructor(b) {
                        this._b = b;
                    }

                    method1() { }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() { return this._b; }
                    set b(v) { this._b = v; }
                }

                ExtraProtoMemberCompletions.prototype.method4 = function () {
                    ^|answer,ExtraProtoMemberCompletions|
                    this.|Object|;
                    this.|_b,method1,method2,method3,a,b,method4|
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInStaticMemberAddedToClass()
        {
            PerformCompletionRequests(CompletionsInStaticMemberAddedToClassTest);
        }
        #region Test Data
        const string CompletionsInStaticMemberAddedToClassTest = @"
                var answer = 42;

                class ExtraStaticMemberCompletions {
                    constructor(b) {
                        this._b = b;
                    }

                    method1() { }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() { return this._b; }
                    set b(v) { this._b = v; }
                }

                ExtraStaticMemberCompletions.method4 = function () {
                    ^|answer,ExtraStaticMemberCompletions|
                    this.|Object|;
                    this.|!_b,!method1,!method2,!method3,!a,!b,answer|
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("DocComments")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassConstructor_DocComments()
        {
            PerformCompletionRequests(CompletionsInClassConstructor_DocCommentsTest);
        }
        #region Test Data
        const string CompletionsInClassConstructor_DocCommentsTest = @"
                class CtorWithDocCompletions {
                    constructor(b) {
                        /// <param name='b' type='String' />
                        b.|String|;
                        this._b = b;
                    }

                    method1() { }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() { return this._b; }
                    set b(v) { this._b = v; }
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("DocComments")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassMembers_ParamDocComments()
        {
            PerformCompletionRequests(CompletionsInClassMembers_ParamDocCommentsTest);
        }
        #region Test Data
        const string CompletionsInClassMembers_ParamDocCommentsTest = @"
                class MembersWithDocCompletions {
                    constructor(b) {
                        /// <param name='b' type='String' />
                        this._b = b;
                    }

                    method1() { }
                    method2(a) {
                        /// <param name='a' value='{ msg: ""hi"" }' />
                        a.|Object|;
                        a.|msg|;
                        a.msg.|String|;
                        return a;
                    }
                    method3(a,b) {
                        /// <param name='a' type='String' />
                        /// <param name='b' type='Number' />
                        a.|String|;
                        b.|Number|;
                        return a+b;
                    }

                    get a() { return 42; }
                    get b() { return this._b; }
                    set b(v) {
                        /// <param name='v' type='Function' />
                        v.|Function|;
                        this._b = v;
                    }
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("DocComments")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassMembers_ReturnDocComments()
        {
            PerformCompletionRequests(CompletionsInClassMembers_ReturnDocCommentsTest);
        }
        #region Test Data
        const string CompletionsInClassMembers_ReturnDocCommentsTest = @"
                class MembersWithDocCompletions {
                    constructor(b) {
                        /// <param name='b' type='String' />
                        this._b = b;
                    }

                    method1() {
                        /// <returns type='Array' elementType='Number' />
                        return;
                    }
                    method2(a) {
                        /// <param name='a' value='{ msg: ""hi"" }' />
                        return a;
                    }
                    method3(a,b) {
                        /// <param name='a' type='String' />
                        /// <param name='b' type='Number' />
                        return a+b;
                    }
                    method4() {
                        /// <returns type='Number' />
                    }

                    get a() {
                        /// <returns type='String' />
                        return 42;
                    }
                    get b() {
                        /// <returns value='1' />
                        return this._b;
                    }
                    set b(v) {
                        /// <param name='v' type='Function' />
                        this._b = v;
                    }
                }

                var instance = new MembersWithDocCompletions();
                instance.a.|Number|;
                instance.b.|Number|;
                instance.b = function () {};
                instance._b.|Function|;
                instance.b.|Number|;

                instance.method3().|Number|;
                instance.method3(""add "", 4).|String|;

                instance.method2().|Object|;
                instance.method2().|msg|;
                instance.method2().msg.|String|;
                var result2 = instance.method2({ message: 0 });
                result2.|Object|;
                result2.|msg,!message|;
                result2.msg.|String|;
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782), WorkItem(917984)]
        [TestCategory("Classes"), TestCategory("DocComments")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassMembers_MissingValuesBasedOnDoc()
        {
            PerformCompletionRequests(CompletionsInClassMembers_MissingValuesBasedOnDocTest);
        }
        #region Test Data
        const string CompletionsInClassMembers_MissingValuesBasedOnDocTest = @"
                class MembersWithDocCompletions {
                    constructor(b) {
                        /// <param name='b' type='String' />
                        this._b = b;
                    }

                    method1() {
                        /// <returns type='Array' elementType='Number' />
                    }
                    method2(a) {
                        /// <param name='a' value='{ msg: ""hi"" }' />
                        return a;
                    }
                    method3(a,b) {
                        /// <param name='a' type='String' />
                        /// <param name='b' type='Number' />
                        return a+b;
                    }
                    method4() {
                        /// <returns type='Number' />
                    }

                    get a() {
                        /// <returns type='String' />
                        return 42;
                    }
                    get b() {
                        /// <returns value='1' />
                        return this._b;
                    }
                    set b(v) {
                        /// <param name='v' type='Function' />
                        this._b = v;
                    }
                }

                var instance = new MembersWithDocCompletions();
                instance._b.|String|;
                
                instance.method1().|Array|;
                instance.method1()[0].|Number|;
                instance.method4().|Number|;
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("DocComments")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassStaticMembers_ParamDocComments()
        {
            PerformCompletionRequests(CompletionsInClassStaticMembers_ParamDocCommentsTest);
        }
        #region Test Data
        const string CompletionsInClassStaticMembers_ParamDocCommentsTest = @"
                class MembersWithDocCompletions {
                    static method1() { }
                    static method2(a) {
                        /// <param name='a' value='{ msg: ""hi"" }' />
                        a.|Object|;
                        a.|msg|;
                        a.msg.|String|;
                        return a;
                    }
                    static method3(a,b) {
                        /// <param name='a' type='String' />
                        /// <param name='b' type='Number' />
                        a.|String|;
                        b.|Number|;
                        return a+b;
                    }

                    static get a() { return 42; }
                    static get b() { return this._b || 0; }
                    static set b(v) {
                        /// <param name='v' type='Function' />
                        v.|Function|;
                        this._b = v;
                    }
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("DocComments")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassStaticMembers_ReturnDocComments()
        {
            PerformCompletionRequests(CompletionsInClassStaticMembers_ReturnDocCommentsTest);
        }
        #region Test Data
        const string CompletionsInClassStaticMembers_ReturnDocCommentsTest = @"
                class MembersWithDocCompletions {
                    static method1() {
                        /// <returns type='Array' elementType='Number' />
                    }
                    static method2(a) {
                        /// <param name='a' value='{ msg: ""hi"" }' />
                        return a;
                    }
                    static method3(a,b) {
                        /// <param name='a' type='String' />
                        /// <param name='b' type='Number' />
                        return a+b;
                    }

                    static get a() {
                        /// <returns type='String' />
                        return 42;
                    }
                    static get b() {
                        /// <returns value='[]' />
                        return this._b || 0;
                    }
                    static set b(v) {
                        /// <param name='v' type='Function' />
                        this._b = v;
                    }
                }

                var instance = MembersWithDocCompletions;
                instance.a.|Number|;
                instance.b.|Array|;
                instance.b = ""hello"";
                instance.b.|Array|;

                instance.method3().|Number|;
                instance.method3(""add "", 4).|String|;

                instance.method2().|Object|;
                instance.method2().|msg|;
                instance.method2().msg.|String|;
                var result2 = instance.method2({ message: 0 });
                result2.|Object|;
                result2.|msg,!message|;
                result2.msg.|String|;

                instance._b.|String|;
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782), WorkItem(917984)]
        [TestCategory("Classes"), TestCategory("DocComments")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInClassStaticMembers_MissingValuesBasedOnDoc()
        {
            PerformCompletionRequests(CompletionsInClassStaticMembers_MissingValuesBasedOnDocTest);
        }
        #region Test Data
        const string CompletionsInClassStaticMembers_MissingValuesBasedOnDocTest = @"
                class MembersWithDocCompletions {
                    static method1() {
                        /// <returns type='Array' elementType='Number' />
                    }
                    static method2(a) {
                        /// <param name='a' value='{ msg: ""hi"" }' />
                        return a;
                    }
                    static method3(a,b) {
                        /// <param name='a' type='String' />
                        /// <param name='b' type='Number' />
                        return a+b;
                    }

                    static get a() {
                        /// <returns type='String' />
                        return 42;
                    }
                    static get b() {
                        /// <returns value='[]' />
                        return this._b || 0;
                    }
                    static set b(v) {
                        /// <param name='v' type='Function' />
                        this._b = v;
                    }
                }

                var instance = MembersWithDocCompletions;
                instance.method1().|Array|;
                instance.method1()[0].|Number|;
                ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("DocComments")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassMemberCompletion_FieldDocComments()
        {
            PerformCompletionRequests(ClassMemberCompletion_FieldDocCommentsTest);
        }
        #region Test Data
        const string ClassMemberCompletion_FieldDocCommentsTest = @"
            class Test {
                constructor() {
                    /// <field name=""a"" type=""String"" />
                    /// <field name=""b"" type=""Array"" elementType=""Number"" />
                    /// <field name=""c"" value=""{ msg: 'hi' }"" />

                    this.|a,b,c|;
                }
            }

            var test = new Test();

            test.|a,b,c|;
            test.a.|String|;
            test.b.|Array|;
            test.b[0].|Number|;
            test.c.|msg|;
            ";
        #endregion

        [TestMethod]
        [WorkItem(892172)]
        [TestCategory("Let")]
        public void ThisCompletionInBlockScopedNestedPrototypeFunction()
        {
            PerformCompletionRequests(LetBoundNestedFunctionCallTest);
        }
        #region Test Data
        const string LetBoundNestedFunctionCallTest = @"
            {
                let Foo = function (a) {
                    this._a = a;
                }
                
                Foo.prototype = {
                    get fooProp() {
                        this.|fooMethod,fooProp,_a|;
                    },
                    set fooProp(v) {
                        this.|fooMethod,fooProp,_a|;
                    }
                };

                Foo.prototype.fooMethod = function () {
                    ^|Foo|   
                    this.|fooMethod,fooProp,_a|;
                };
            }
            ";
        #endregion

        [TestMethod]
        [WorkItem(892173)]
        [TestCategory("get/set")]
        public void GetterSetterNestedCallCompletions()
        {
            PerformCompletionRequests(GetterSetterNestedCallCompletionsTest);
        }
        #region Test Data
        const string GetterSetterNestedCallCompletionsTest = @"
            var obj = {
                objMethod: function () { },
                get objProp() {
                    var a = {};
                    a.|Object|;                 // Expect Object members
                    this.|objMethod,objProp|;   // Expect obj members
                },
                set objProp(v) {
                    var a = {};
                    a.|Object|;                 // Expect Object members
                    this.|objMethod,objProp|;   // Expect obj members
                }        
            };

            function Bar() { }
            Bar.prototype = {
                barMethod: function () { },
                get barProp() {
                    var a = {};
                    a.|Object|;                 // Expect Object members
                    this.|barMethod,barProp|;   // Expect Bar.prototype members
                },
                set barProp(v) {
                    var a = {};
                    a.|Object|;                 // Expect Object members
                    this.|barMethod,barProp|;   // Expect obj members
                }        
            };
            ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("ErrorRecovery")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassDeclarationWithoutIdentifier()
        {
            PerformCompletionRequests(ClassDeclarationWithoutIdentifierTest);
        }
        #region Test Data
        const string ClassDeclarationWithoutIdentifierTest = @"
            var answer = 42;

            class {
                method1() {
                    ^|answer|;
                    this.|method1,a|;
                }
                get a() {
                    ^|answer|;
                    this.|method1,a|;
                }

                static method2() {
                    ^|answer|;
                    this.|method2,b|;
                }
                static get b() {
                    ^|answer|;
                    this.|method2,b|;
                }
            }

            ^|answer|;
            ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("ErrorRecovery")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassDeclarationMissingLCurly()
        {
            PerformCompletionRequests(ClassDeclarationMissingLCurlyTest);
        }
        #region Test Data
        const string ClassDeclarationMissingLCurlyTest = @"
            var answer = 42;

            class Test
                method1() 
                    ^|answer|;
                    this.|method1,a|;
                }
                get a() 
                    ^|answer|;
                    this.|method1,a|;
                }

                static method2() 
                    ^|answer|;
                    this.|method2,b|;
                }
                static get b() 
                    ^|answer|;
                    this.|method2,b|;
                }
            }

            ^|answer|;
            ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("ErrorRecovery")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassDeclarationWithMultipleConstructors()
        {
            PerformCompletionRequests(ClassDeclarationWithMultipleConstructorsTest);
        }
        #region Test Data
        const string ClassDeclarationWithMultipleConstructorsTest = @"
            var answer = 42;

            class Test {
                constructor() { 
                    ^|answer|;
                    this.|method1,a,ctor3,!ctor2,!ctor1|;
                    this.ctor1 = ""test ctor 1"";
                }
                constructor() {
                    ^|answer|;
                    this.|method1,a,ctor3,!ctor2,!ctor1|;
                    this.ctor2 = ""test ctor 2"";
                }
                constructor() {
                    ^|answer|;
                    this.|method1,a|;
                    this.ctor3 = ""test ctor 3"";
                }
                method1() {
                    ^|answer|;
                    this.|method1,a,ctor3,!ctor2,!ctor1|;
                }
                get a() 
                    ^|answer|;
                    this.|method1,a,ctor3,!ctor2,!ctor1|;
                }
            }

            ^|answer|;
            ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("ErrorRecovery")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassDeclarationMissingMemberIdentifier()
        {
            PerformCompletionRequests(ClassDeclarationMissingMemberIdentifierTest);
        }
        #region Test Data
        const string ClassDeclarationMissingMemberIdentifierTest = @"
            var answer = 42;

            class Test {
                () 
                    ^|answer|;
                    this.|Object|;
                }
                get :() {
                    ^|answer|;
                    this.|Object|;
                }
                set :() {
                    ^|answer|;
                    this.|Object|;
                }

                static () {
                    ^|answer|;
                    this.|Function|;
                }
                static get :() {
                    ^|answer|;
                    this.|Function|;
                }
                static set :() {
                    ^|answer|;
                    this.|Function|;
                }
            }

            ^|answer|;
            ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("ErrorRecovery")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassDeclarationWithInvalidMemberIdentifiers()
        {
            PerformCompletionRequests(ClassDeclarationWithInvalidMemberIdentifiersTest);
        }
        #region Test Data
        const string ClassDeclarationWithInvalidMemberIdentifiersTest = @"
            var answer = 42;

            class Test1 {
                method1() {
                }
                get a() {
                }
                get constructor() {
                    ^|answer|;
                    this.|Object|;
                    this.|method1,a|;
                }
                set constructor() {
                    ^|answer|;
                    this.|Object|;
                    this.|method1,a|;
                }
            }

            class Test2 {
                method1() {
                }
                get a() {
                }
                static prototype() {
                    ^|answer|;
                    this.|Function|;
                    this.prototype.|method1,a|;
                }
            }

            class Test3 {
                method1() {
                }
                get a() {
                }
                static get prototype() {
                    ^|answer|;
                    this.|Function|;
                    this.prototype.|method1,a|;
                }
                static set prototype() {
                    ^|answer|;
                    this.|Function|;
                    this.prototype.|method1,a|;
                }
            }

            ^|answer|;
            ";
        #endregion    

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("ErrorRecovery")]
        [TestProperty("IsolationLevel", "Method")]
        public void ClassDeclarationWithDuplicateMemberIdentifiers()
        {
            PerformCompletionRequests(ClassDeclarationWithDuplicateMemberIdentifiersTest);
        }
        #region Test Data
        const string ClassDeclarationWithDuplicateMemberIdentifiersTest = @"
            var answer = 42;

            class Test {
                // Redeclarations of member functions.
                foo1() 
                    ^|answer|;
                    this.|Object|;
                    this.|foo1,foo2,foo3|;
                }
                get foo1() {
                    ^|answer|;
                    this.|Object|;
                    this.|foo1,foo2,foo3|;
                }
                foo2() 
                    ^|answer|;
                    this.|Object|;
                    this.|foo1,foo2,foo3|;
                }
                set foo2() {
                    ^|answer|;
                    this.|Object|;
                    this.|foo1,foo2,foo3|;
                }
                foo3() 
                    ^|answer|;
                    this.|Object|;
                    this.|foo1,foo2,foo3|;
                }
                foo3() {
                    ^|answer|;
                    this.|Object|;
                    this.|foo1,foo2,foo3|;
                }

                // Redeclarations of getters/setters
                static get bar1() {
                    ^|answer|;
                    this.|Function|;
                    this.|bar1,bar2,bar3,bar4,bar5|;
                }
                static get bar1() {
                    ^|answer|;
                    this.|Function|;
                    this.|bar1,bar2,bar3,bar4,bar5|;
                }
                static set bar2() {
                    ^|answer|;
                    this.|Function|;
                    this.|bar1,bar2,bar3,bar4,bar5|;
                }
                static set bar2() {
                    ^|answer|;
                    this.|Function|;
                    this.|bar1,bar2,bar3,bar4,bar5|;
                }
                static get bar3() {
                    ^|answer|;
                    this.|Function|;
                    this.|bar1,bar2,bar3,bar4,bar5|;
                }
                static bar3() {
                    ^|answer|;
                    this.|Function|;
                    this.|bar1,bar2,bar3,bar4,bar5|;
                }
                static set bar4() {
                    ^|answer|;
                    this.|Function|;
                    // 'bar5' will not be in this completion list, since the initialization of the 'bar4' method
                    // will cause this setter to be called before 'bar5' is initialized.
                    this.|bar1,bar2,bar3,bar4|;
                }
                static bar4() {
                    ^|answer|;
                    this.|Function|;
                    this.|bar1,bar2,bar3,bar4,bar5|;
                }
                static bar5() {
                    ^|answer|;
                    this.|Function|;
                    this.|bar1,bar2,bar3,bar4,bar5|;
                }                
                static bar5() {
                    ^|answer|;
                    this.|Function|;
                    this.|bar1,bar2,bar3,bar4,bar5|;
                }                
            }

            ^|answer|;
            ";
        #endregion


        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("extends")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsAfterExtendsKeyword()
        {
            PerformCompletionRequests(CompletionsAfterExtendsKeywordTest);
        }
        #region Test Data
        const string CompletionsAfterExtendsKeywordTest = @"
            var answer = 42;
            function Foo () {}

            class Test extends ^|answer,Foo| {} 
        ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("extends")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsInExtendsExpr()
        {
            PerformCompletionRequests(CompletionsInExtendsExprTest);
        }
        #region Test Data
        const string CompletionsInExtendsExprTest = @"
            var answer = 42;
            function Foo () {}

            class ExtendIIFE extends (function() {
                    ^|answer,Foo|;
                    var bar = { a: ""a"", b: 10, c: true };
                    bar.|a,b,c|;
                    this.|answer,Foo|;
                })() {}

            class ExtendFunction extends function() {
                    ^|answer,Foo|;
                    var bar = { a: ""a"", b: 10, c: true };
                    bar.|a,b,c|;
                } {}

            class ExtendClass extends class {
                    constructor() {
                        ^|answer,Foo|;
                        this._b = ""hello"";
                        this.|_b,method1,method2,method3,a,b|;
                    }

                    method1() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                    }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b.|String|;
                        return this._b;
                    }
                    set b(v) {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b = v;
                    }

                    static method4() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                    static get c() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                    static set c() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                } {}
            ";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("extends")]
        [TestProperty("IsolationLevel", "Method")]
        public void CompletionsAfterInvalidExtendsExpr()
        {
            PerformCompletionRequests(CompletionsAfterInvalidExtendsExprTest);
        }
        #region Test Data
        const string CompletionsAfterInvalidExtendsExprTest = @"
            var answer = 42;
            function Foo () {}
            Foo.prototype = 0;

            class TestIntegerExtends extends 0 {
                    constructor() {
                        this._b = ""hello"";
                    }

                    method1() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                    }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b.|String|;
                        return this._b;
                    }
                    set b(v) {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b = v;
                    }

                    static method4() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                    static get c() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                    static set c() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                }

            class TestIntegerPrototypeExtends extends Foo {
                    constructor() {
                        this._b = ""hello"";
                    }

                    method1() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                    }
                    method2(a) { return a; }
                    method3(a,b) { return a+b; }

                    get a() { return 42; }
                    get b() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b.|String|;
                        return this._b;
                    }
                    set b(v) {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Object|;
                        this.|_b,method1,method2,method3,a,b|;
                        this._b = v;
                    }

                    static method4() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                    static get c() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                    static set c() {
                        ^|answer,Foo|;
                        answer.|Number|;
                        this.|Function|;
                        this.|method4,c|;
                    }
                }
                ";
        #endregion

        [TestMethod]
        [WorkItem(917987)]
        [TestCategory("Classes"), TestCategory("super")]
        public void SuperCompletionsInClassMembers()
        {
            PerformCompletionRequests(SuperCompletionsInClassMembersTest);
        }
#region Test Data
        const string SuperCompletionsInClassMembersTest = @"
                class A {
                  constructor() { this._prop = 'prop';}
                  method()      { return 'method A'; }
                  get prop() { return this._prop ? this._prop : 42; }
                  set prop(v) { this._prop = v; }
                  static staticMethod() { return 'staticMethod A'; }
                  static get staticProp() { return 'prop'; }
                }

                class B extends A {
                  constructor() {
                    super();
                    super.|Function|;
                    super.|!method,!prop,!newMethod,staticMethod,staticProp,newStaticMethod,!_prop|;
                  }
                  method()           { return super().|String|; }
                  superMethod() { 
                    super.|Object|;
                    return super.|method,prop,newMethod,!staticMethod,!staticProp,!newStaticMethod,!_prop|;
                    super.method().|String|;
                    super.prop.|String|;
                  }
                  superMethodIndex() { 
                    super.|Object|;
                    return super['|method,prop,newMethod,!staticMethod,!staticProp,!newStaticMethod,!_prop|'];
                    super['method']().|String|;
                    super['prop'].|String|;
                  }
                  static staticSuperMethod() { 
                    super.|Function|;
                    return super.|!method,!prop,!newMethod,staticMethod,staticProp,newStaticMethod,!_prop|;
                    super.staticMethod().|String|;
                    super.staticProp.|String|;
                  }
                  static staticSuperMethodIndex() { 
                    super.|Function|;
                    return super['|!method,!prop,!newMethod,staticMethod,staticProp,newStaticMethod,!_prop|'];
                    super['staticMethod']().|String|;
                    super['staticProp'].|String|;
                  }
                }

                A.prototype.newMethod = function () { return 'newMethod A'; };
                A.newStaticMethod = function () { return 'newStaticMethod A'; };

                ";
#endregion

        [TestMethod]
        [WorkItem(917987)]
        [TestCategory("Classes"), TestCategory("super")]
        public void CompletionsAfterInvalidSuper()
        {
            PerformCompletionRequests(CompletionsAfterInvalidSuperTest);
        }
#region Test Data
        const string CompletionsAfterInvalidSuperTest = @"
            var answer = 42;
            super;
            ^|answer|
            
            var mysuper = x => {
                super[x]();
                ^|answer|
            };

            var x = {
                get test() { return super.x; },
                a: function () { this.|test|; }
            }            

            class A {
                constructor() { }
                method() { }
                static staticMethod() { }
            }
            
            class B extends A {
                constructor() {
                    super++;
                    super.|staticMethod,!method|;
                }
            }

            class C {
                test() {
                    super();
                    this.|Object|;
                    this.|test|;
                }
            }

        ";
#endregion

        [TestMethod]
        [Ignore] // Disabled pending fix for 189873
        [WorkItem(917987), WorkItem(189873)]
        [TestCategory("Classes"), TestCategory("super")]
        public void SuperCompletionsInLambda()
        {
            PerformCompletionRequests(SuperCompletionsInLambdaTest);
        }
        #region Test Data
        const string SuperCompletionsInLambdaTest = @"
                class A {
                  constructor() { this._prop = 'prop';}
                  method()      { return 'method A'; }
                  get prop() { return this._prop ? this._prop : 42; }
                  set prop(v) { this._prop = v; }
                  static staticMethod() { return 'staticMethod A'; }
                  static get staticProp() { return 'prop'; }
                }

                class B extends A {
                  lambda() {
                    var mysuper = x => {
                        super.|Object|;
                        super.|method,prop,newMethod,!staticMethod,!staticProp,!newStaticMethod,!_prop,!lambda|;
                        return super[x](); 
                    }
                    return mysuper('method');
                  }
                  lambdaWithThis() {
                    var mysuperwiththis = x => {
                        this.|Object|;
                        this.|method,prop,newMethod,!staticMethod,!staticProp,!newStaticMethod,_prop,lambda|;
                        super.|Object|;
                        super.|method,prop,newMethod,!staticMethod,!staticProp,!newStaticMethod,!_prop,!lambda|;
                        return super[x](); 
                    }
                    return mysuperwiththis('method');
                  }
                  static staticLambda() {
                    var mystaticsuper = x => {
                        super.|Function|;
                        super.|!method,!prop,!newMethod,staticMethod,staticProp,newStaticMethod,!_prop,!staticLambda|;
                        return super[x](); 
                    }
                    return mystaticsuper('staticMethod');
                  }
                  static staticLambdaWithThis() {
                    var mystaticsuperwiththis = x => {
                        this.|Function|;
                        this.|!method,!prop,!newMethod,staticMethod,staticProp,newStaticMethod,!_prop,staticLambda|;
                        super.|Function|;
                        super.|!method,!prop,!newMethod,staticMethod,staticProp,newStaticMethod,!_prop,!staticLambda|;
                        return super[x](); 
                    }
                    return mystaticsuperwiththis('staticMethod');
                  }
                }

                A.prototype.newMethod = function () { return 'newMethod A'; };
                A.newStaticMethod = function () { return 'newStaticMethod A'; };

        ";
        #endregion
    }
}
