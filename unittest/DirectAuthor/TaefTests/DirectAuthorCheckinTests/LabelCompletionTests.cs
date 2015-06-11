//----------------------------------------------------------------------------------------------------------------------
// <copyright file="LabelCompletionTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the LabelCompletionTests type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System;
    using Microsoft.BPT.Tests.DirectAuthor;
    using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class LabelCompletionTests : CompletionsBase
    {
        private static Action<IAuthorTestContext, int, string, int> validateHasEmptyCompletionSet = (context, offset, data, index) =>
        {
            var completion = context.GetCompletionsAt(offset);
            Assert.IsTrue(completion.Count == 0);
        };

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
        [WorkItem(193249)]
        public void ValidCompletions()
        {
            // Valid lable completions
            PerformRequests(@"
                loop1: while (false) { 
                             break|loop1|;
                             continue|loop1|;
                loop2:      for (var i = 0; i < 10; i ++) {
                             var x = 0;
                             break|loop1,loop2|;
                             continue|loop1,loop2|;
                loop3:         do {
                                    x ++;
                                    var a = [1, 3];
                                    break|loop1,loop2,loop3|;
                                    continue|loop1,loop2,loop3|;
                loop4:             for (var j in a) {
                                        alert(j);
                                        break|loop1,loop2,loop3,loop4|;
                                        continue|loop1,loop2,loop3,loop4|;
                                    }
                                } while (x > 1);
                            }
                        }
                ", (context, offset, data, index) =>
                 {
                     var completion = context.GetCompletionsAt(offset);
                     completion.ToEnumerable().ExpectContains(data.Split(','));
                     Assert.AreEqual(completion.Count, data.Split(',').Length);
                 });

            // while typing
            ValidateHasCompletions(@"loop1: while (false) { break|", "loop1");
            ValidateHasCompletions(@"loop1: while (false) { break lo|", "loop1");
        }

        [TestMethod]
        [WorkItem(193249)]
        public void NoStatementLables()
        {
            // no labels
            PerformRequests(@"while (false) { break|; }", validateHasEmptyCompletionSet);
            PerformRequests(@"while (false) { continue|; }", validateHasEmptyCompletionSet);
        }

        [TestMethod]
        [WorkItem(193249)]
        public void LabelsOutsideFunctionScope()
        {
            // labels outside function scope
            PerformRequests(@"
                outerlabel: while(false) { 
                                function foo () {
                                    while (false) { break|; }
                                }
                            };", validateHasEmptyCompletionSet);
            PerformRequests(@"
                outerlabel: while(false) { 
                                function foo () {
                                    while (false) { continue|; }
                                }
                            };", validateHasEmptyCompletionSet);
        }

        [TestMethod]
        [WorkItem(193249)]
        public void EmptyCurrentLableSet()
        {
            // labels outside the current label set
            PerformRequests(@"
                loop_1: while(false) { 
                loop_2:     for (var i = 0; i < 10; i++) {
                            }
                        };
                var_1:  var x = 0;
                        while (false) {break|;}
                ", validateHasEmptyCompletionSet);
            PerformRequests(@"
                loop_1: while(false) { 
                loop_2:     for (var i = 0; i < 10; i++) {
                            }
                        };
                var_1:  var x = 0;
                        while (false) {continue|;}
                ", validateHasEmptyCompletionSet);
        }
    }
}