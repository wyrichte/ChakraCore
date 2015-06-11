using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using System.IO;

using Microsoft.VisualStudio.TestTools.UnitTesting;

using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;

namespace DirectAuthorTests
{
    [TestClass]
    public class QuickInfoTests : DirectAuthorTest
    {
        [TestMethod]
        public void FieldDocCommentInCtor()
        {
            PerformRequests(@"
                var NS = {};
                NS.Class1 = function() {
                    function make() { 
                        /// <field name='file1' helpKeyword='helpkw'>
                        /// <summary>file1</summary>
                        /// <deprecated type='remove'>fieldDeprecatedMessage</deprecated>
                        /// </field>
                    }
                    return new make();
                };
                var c = new NS.Class1();
                c.file1|;
            ",
            (context, offset, data, index) =>
            {
                var hint = context.GetCompletionsAt(offset).GetHintFor("file1");
                hint.HelpKeyword.Expect("helpkw");
                var help = context.GetQuickInfoAt(offset);
                Assert.IsNotNull(help);
                help.HelpKeyword.Expect("helpkw");
                var deprecated = help.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect("remove");
                deprecated.Message.Expect("fieldDeprecatedMessage");
            });

            // Verify quick info for a property getter which returns a function.
            PerformRequests(@"
                var NS = {};
                NS.Class1 = function() {
                    function make() { 
                        /// <field name='file1' helpKeyword='helpkw'>file1</field>
                    }
                    Object.defineProperty(make.prototype, 'dismissed', {
                        get: function () {
                            return function () {
                                /// <signature helpKeyword='Windows.ApplicationModel.Activation.SplashScreen.Dismissed'>
                                /// <param name='ev' type='Object' />
                                /// <event>dismissed</event>
                                /// </signature>
                            };
                        }
                    });
                    return new make();
                };
                NS.Class1().dismissed|;
            ",
            (context, offset, data, index) =>
            {
                var hint = context.GetCompletionsAt(offset).GetHintFor("dismissed");
                //hint.HelpKeyword.Expect("helpkw");
                var help = context.GetQuickInfoAt(offset);
                Assert.IsNotNull(help);
                var funcHelp = help.GetFunctionHelp();
                var signature = funcHelp.GetSignatures().ToEnumerable().Single();
                signature.HelpKeyword.Expect("Windows.ApplicationModel.Activation.SplashScreen.Dismissed");
            });
        }


        [TestMethod]
        public void Annotate()
        {
            PerformRequests(@"
                var NS = {};
                function make() {}
                NS.Class1 = function() {
                    return new make();
                };
                Object.defineProperty(make.prototype, 'file1', { get: function() { 
                    return 'hi'; 
                }, enumerable: true, configurable: true });
                
                intellisense.annotate(make.prototype, { 
                    /// <field name='file1' helpKeyword='helpkw'>
                    /// <summary>file1</summary>
                    /// <deprecated type='remove'>fieldDeprecatedMessage</deprecated>
                    /// </field>
                    file1: undefined
                });

                var c = new NS.Class1();
                c.file1|;
                    
            ",
            (context, offset, data, index) =>
            {
                var hint = context.GetCompletionsAt(offset).GetHintFor("file1");
                hint.HelpKeyword.Expect("helpkw");
                var help = context.GetQuickInfoAt(offset);
                Assert.IsNotNull(help);
                help.HelpKeyword.Expect("helpkw");
                var deprecated = help.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect("remove");
                deprecated.Message.Expect("fieldDeprecatedMessage");
            });
        }

        [TestMethod]
        public void FunctionDocComments()
        {
            PerformRequests(@"
                var NS = {};
                NS.Class1 = function() {
                    /// <signature helpKeyword='func-helpKeyword' externalFile='func-externalFile' externalid='func-externalid'>
                    /// <deprecated type='deprecate'>func-deprecatedMessage</deprecated>
                    /// </signature>
                    function make() { }
                    return new make();
                };
                new NS.Class1|();
            ",
            (context, offset, data, index) =>
            {
                var help = context.GetQuickInfoAt(offset);
                Assert.IsNotNull(help);
                Assert.IsTrue(help.Type == AuthorType.atFunction);
                var funcHelp = help.GetFunctionHelp();
                Assert.IsNotNull(funcHelp);
                var signature = funcHelp.GetSignatures().ToEnumerable().First();
                signature.HelpKeyword.Expect("func-helpKeyword");
                signature.ExternalFile.Expect("func-externalFile");
                signature.Externalid.Expect("func-externalid");
                var deprecated = signature.GetDeprecated();
                Assert.IsNotNull(deprecated);
                deprecated.Type.Expect("deprecate");
                deprecated.Message.Expect("func-deprecatedMessage");
            });
        }

        [TestMethod]
        public void DocCommentAttributeTests()
        {
            PerformRequests(@"
                function enclosing() {
                    /// <var type='Object' helpKeyword='helpKeyword' locid='locid' externalFile='externalFile' externalid='externalid'></var> 
                    var x = {}; // local variable same as global
                    x|var|;    
                    /// <var type='Object' helpKeyword='helpKeyword' locid='locid' externalFile='externalFile' externalid='externalid'></var> 
                    var y = {}; // local variable
                    y|var|;    
                }

                var o = {
                    /// <field type='Number' helpKeyword='helpKeyword' locid='locid' externalFile='externalFile' externalid='externalid'></field> 
                    f: 1
                };
                o.f|field|;

                /// <var type='Object' helpKeyword='helpKeyword' locid='locid' externalFile='externalFile' externalid='externalid'></var> 
                var x = {}; // Global variable
                x|var|;    
                
                function Cat(name) {
                    /// <signature helpKeyword='func-helpKeyword' externalFile='func-externalFile' externalid='func-externalid'>
                    /// <summary locid='func-locid'></summary>
                    /// <param name='name' type='String' locid='locid'></param>
                    /// <returns type='Cat' helpKeyword='helpKeyword' locid='locid'></returns>
                    /// </signature>
                    /// <field name='_name' type='String' helpKeyword='helpKeyword' locid='locid' externalFile='externalFile' externalid='externalid'></field> 
                    try { with(this) { hasTail = true; } } catch(e) {}
                    name|param|;
                    this._name = name;
                }
                var cat = new Cat|function|();
                cat|return|; // Object created via ctor 
                cat._name|field|; // ctor field

                function Food() {}
                function eat(foods) {
                    /// <signature> 
                    /// <param name='foods' type='Food' locid='locid'>foods</param> 
                    /// </signature> 
                    foods|param|;
                }
            ", 
            (context, offset, data, index) =>
            {
                var help = context.GetQuickInfoAt(offset);
                Assert.IsNotNull(help);
                switch (data)
                {
                    case "function":
                        Assert.IsTrue(help.Type == AuthorType.atFunction);
                        var funcHelp = help.GetFunctionHelp();
                        Assert.IsNotNull(funcHelp);
                        var signature = funcHelp.GetSignatures().ToEnumerable().First();
                        signature.HelpKeyword.Expect("func-helpKeyword");
                        signature.Locid.Expect("func-locid");
                        signature.ExternalFile.Expect("func-externalFile");
                        signature.Externalid.Expect("func-externalid");
                        break;
                    case "field":
                    case "var":
                        help.HelpKeyword.Expect("helpKeyword");
                        help.Locid.Expect("locid");
                        help.ExternalFile.Expect("externalFile");
                        help.Externalid.Expect("externalid");
                        break;
                    case "param":
                        help.Locid.Expect("locid");
                        break;
                    case "return":
                        help.HelpKeyword.Expect("helpKeyword");
                        help.Locid.Expect("locid");
                        break;
                }
            });
        }

        [TestMethod]
        [Ignore] // TODO: Currently doesn't work because this is not added to completion list
        public void quickInfoOnThis()
        {
            PerformRequests(@"
                function Point(x, y) {
                    this.x = x;
                    th|is.y = y;
                }
                new Point(1, 1);
            ",
            (context, offset, data, index) =>
            {
                var help = context.GetQuickInfoAt(offset);
                Assert.IsNotNull(help);
            });
        }
    }
}