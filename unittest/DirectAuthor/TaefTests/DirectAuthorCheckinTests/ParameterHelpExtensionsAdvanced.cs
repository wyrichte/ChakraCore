//----------------------------------------------------------------------------------------------------------------------
// <copyright file="ParameterHelpExtensionsAdvanced.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the ParameterHelpExtensionsAdvanced type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System;
    using System.Linq;
    using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class ParameterHelpExtensionsAdvanced : ParameterHelpTestsBase
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
        public void Extensions_ParameterHelp()
        {
            PerformRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                        intellisense.logMessage('>>>> e.parentObject: ' + JSON.stringify(e.parentObject));
                        if(e.parentObject) {
                            if(e.functionHelp.functionName.indexOf('set_') == 0) {
                                var getterName = 'get_' + e.functionHelp.functionName.substring(4, e.functionHelp.functionName.length);
                                intellisense.logMessage('>>>> getterName: ' + getterName);
                                var getter = e.parentObject[getterName];
                                if(getter) {
                                   var src = getter.toString(); 
                                   intellisense.logMessage('>>>> getter src: ' + src);
                                   var desc = src.substring(src.indexOf('<summary>') + 9, src.indexOf('</summary>')); 
                                   intellisense.logMessage('>>>> description: ' + desc);
                                   e.functionHelp.signatures[0].description = desc;
                                   return true;
                                }
                            }
                                                             
                        }
                        return false;
                    }
                );

                var x = {
                    set_X: function() {},
                    get_X: function() {
                        <summary>X desc</summary>
                    }
                };
                x.set_X(|);                    
            ",
            (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                help.FunctionHelp.FunctionName.Expect("set_X");
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().First();
                signature.Description.Expect("X desc");
            });

            PerformRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {});
                var x = undefined;
                x.set_X(|);                    
            ",
            (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                Assert.IsNull(help.FunctionHelp);
            });

            PerformRequests(@"
                              function add(x, y) { } 
                              add(|);",
            (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                help.FunctionHelp.FunctionName.Expect("add");
            }, // Extension defined in a context file
                new string[] {  @"
                    intellisense.addEventListener('signaturehelp', 
                        function (e) {
                            throw new Error('error from extension');
                        }
                    );" }
            );


            PerformRequests(@"
                              intellisense.addEventListener('signaturehelp',  function(e) {});
                              function add(x, y) { } 
                              add(|);",
            (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                help.FunctionHelp.FunctionName.Expect("x");
            }, // Extension defined in a context file
                new string[] {  @"
                    intellisense.addEventListener('signaturehelp', 
                        function (e) {
                            e.functionHelp.functionName = 'x';
                        }
                    );" }
            );

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = [ 
                        { 
                            'description': 'adds two numbers',
                            'returnValue': {
                                'type':'String',
                                'description':'The sum of x, y'
                            },
                            'params':[ 
                                { 'description':'Value of x', 'name':'x', 'type':'Number' }, 
                                { 'description':'Value of y', 'name':'y', 'type':'Number' }
                            ]
                        },
                        {
                            'description': 'signature 2',
                            'returnValue': {
                                'type':'MyType'
                            },
                            'params':[ 
                                { 'description':'param1', 'name':'a', 'type':'String' }
                            ]
                        }
                    ];
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    var signatures = help.FunctionHelp.GetSignatures().ToEnumerable().ToArray();
                    signatures[0].Description.Expect("adds two numbers");
                    signatures[0].GetReturnValue().Type.Except("String");
                    signatures[0].GetReturnValue().Description.Except("String");
                    var params0 = signatures[0].GetParameters().ToEnumerable().ToArray();
                    params0[0].Name.Except("x");
                    params0[0].Description.Except("Value of x");
                    params0[0].Type.Except("Number");
                    params0[1].Name.Except("y");
                    params0[1].Description.Except("Value of y");
                    params0[1].Type.Except("Number");

                    signatures[1].Description.Expect("signature 2");
                    Assert.IsTrue(String.IsNullOrEmpty(signatures[1].GetReturnValue().Description));
                    signatures[1].GetReturnValue().Type.Except("MyType");
                    Assert.IsTrue(String.IsNullOrEmpty(signatures[1].GetReturnValue().Description));
                    var params1 = signatures[1].GetParameters().ToEnumerable().ToArray();
                    params1[0].Name.Except("a");
                    params1[0].Description.Except("param1");
                    params1[0].Type.Except("String");

                });


            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.functionName = 1;
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNull(help.FunctionHelp.FunctionName);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = 1;
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNull(help.FunctionHelp);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = [  ];
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNull(help.FunctionHelp);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = undefined;
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNull(help.FunctionHelp);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = [ { } ];
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    signature.GetParameters().ToEnumerable().Count().Expect(0);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    function logMessage(msg) { intellisense.logMessage(msg + '\n'); }
                    logMessage('e.target: ' + e.target);
                    logMessage('e.functionHelp: ' + JSON.stringify(e.functionHelp));
                    var comments = intellisense.getFunctionComments(e.target);
                    logMessage('getComments result: ' + JSON.stringify(comments));
                });
                function add(x, y) {
                    /// <summary>adds two numbers</summary>
                    /// <param name=""x"" type=""Number"">Value of x</param>
                    /// <param name=""y"" type=""Number"">Value of y</param>
                    /// <returns type=""Number"">The sum of x, y</returns>
                    return x+y;
                }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    signature.Description.Expect("adds two numbers");
                    var p = signature.GetParameters().ToEnumerable().ToArray();
                    p[0].Description.Except("Value of x");
                    p[1].Description.Except("Value of y");
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures[0].description = 'modified description';
                    e.functionHelp.signatures[0].params[0].description = 'modified x description';
                    e.functionHelp.signatures[0].params[1].description = 'modified x description';
                });
                function add(x, y) {
                    /// <summary>adds two numbers</summary>
                    /// <param name=""x"" type=""Number"">Value of x</param>
                    /// <param name=""y"" type=""Number"">Value of y</param>
                    /// <returns type=""Number"">The sum of x, y</returns>
                    return x+y;
                }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    signature.Description.Expect("modified description");
                    var p = signature.GetParameters().ToEnumerable().ToArray();
                    p[0].Description.Except("modified x description");
                    p[1].Description.Except("modified x description");
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures[0].params.splice(0, 1); // remove x parameter
                });
                function add(x, y) {
                    /// <summary>adds two numbers</summary>
                    /// <param name=""x"" type=""Number"">Value of x</param>
                    /// <param name=""y"" type=""Number"">Value of y</param>
                    /// <returns type=""Number"">The sum of x, y</returns>
                    return x+y;
                }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    var p = signature.GetParameters().ToEnumerable().ToArray();
                    p[0].Description.Except("Value of y");
                });
        }
    }
}