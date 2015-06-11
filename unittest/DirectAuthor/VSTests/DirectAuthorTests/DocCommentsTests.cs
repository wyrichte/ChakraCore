using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorTests
{
    [TestClass]
    public class DocCommentsTests : DirectAuthorTest
    {
        [TestMethod]
        public void VarWithPlainComments()
        {
            PerformCompletionRequests(@"
                // <var type='String'>var x</var>
                var x = null;
                x.|!substring|;
            ");

            PerformCompletionRequests(@"
                var o = {
                    // <field type='Array' elementType='Number'/>
                    arr: undefined
                };
                o.arr.|!concat|;
            ");
        }

        [TestMethod]
        public void NewLine()
        {
            Action<IAuthorCompletionSet, string, string> ValidateFunctionDescription = (completions, name, expected) =>
            {
                var actual = completions.GetHintFor(name).GetFunctionHelp().GetSignatures().ToEnumerable().Single().Description;
                actual.Expect(expected);
            };

            PerformRequests(@"
                function f1(a) {
                    /// <summary>Line1&#10;Line2</summary>
                }
                function f2(a) {
                    /// <summary>Line1 &#10; Line2</summary>
                }
                function f3(a) {
                    /// <summary>Line1 &#10;&#10; Line2</summary>
                }
                function f4(a) {
                    /// <summary>Line1 &#10;&#10;</summary>
                }
                function f5(a) {
                    /// <summary>&#10;</summary>
                }
                function f6(a) {
                    /// <summary>Line1&#10;Line2&#10;Line3</summary>
                }
                function f7(a) {
                    /// <signature><summary>Line1&#10;Line2&#10;Line3</summary></signature>
                }
                function f8(a) {
                    /// <param name='a'>Line1 &#10;</param>
                }
                function f9(a) {
                    /// <summary>&#10;2</summary>
                }
                /// <var>Line1&#10;Line2</var>
                var v1 = {};
                ;|
            ", (context, offset, data, index) => {
                 var completions = context.GetCompletionsAt(offset);
                 ValidateFunctionDescription(completions, "f1", "Line1<br/>Line2");
                 ValidateFunctionDescription(completions, "f2", "Line1 <br/> Line2");
                 ValidateFunctionDescription(completions, "f3", "Line1 <br/><br/> Line2");
                 ValidateFunctionDescription(completions, "f4", "Line1 <br/><br/>");
                 ValidateFunctionDescription(completions, "f5", "<br/>");
                 ValidateFunctionDescription(completions, "f6", "Line1<br/>Line2<br/>Line3");
                 ValidateFunctionDescription(completions, "f7", "Line1<br/>Line2<br/>Line3");
                 completions.GetHintFor("v1").Description.Expect("Line1<br/>Line2");
                 ValidateFunctionDescription(completions, "f9", "<br/>2");
                 completions.GetHintFor("f8").GetFunctionHelp().GetSignatures().ToEnumerable().Single()
                     .GetParameters().ToEnumerable().Single().Description.Expect("Line1 <br/>");
             });
        }
    }
}
