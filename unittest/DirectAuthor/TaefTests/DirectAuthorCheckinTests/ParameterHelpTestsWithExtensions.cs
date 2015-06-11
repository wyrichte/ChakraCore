//----------------------------------------------------------------------------------------------------------------------
// <copyright file="ParameterHelpTestsWithExtensions.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the ParameterHelpTestsWithExtensions type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System.Linq;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    //
    // Summary: Runs all parameter help tests with an extension handler to ensure the tests
    //          are not affected.
    //
    [TestClass]
    public class ParameterHelpTestsWithExtensions : ParameterHelpTests
    {
        protected override string[] AdditionalContextFiles
        {
            get
            {
                return base.AdditionalContextFiles.Concat(new string[] {
                    @"
                     intellisense.addEventListener('signaturehelp',  function(e) {
                            intellisense.logMessage('base extension: e.functionHelp: ' + JSON.stringify(e.functionHelp) + '\n');
                        }
                    );"}).ToArray();
            }
        }

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
        [WorkItem(426016)]
        new public void BoundFunction()
        {
            base.BoundFunction();
        }

        [TestMethod]
        [WorkItem(510260)]
        new public void BindHelp()
        {
            base.BindHelp();
        }

        [TestMethod]
        [WorkItem(385192)]
        [Ignore]     // Relies on hurry
        new public void CallingUndefinedFunctionAfterInfiniteLoop()
        {
            base.CallingUndefinedFunctionAfterInfiniteLoop();
        }

        [TestMethod]
        new public void TaggedUndefined()
        {
            base.TaggedUndefined();
        }

        [TestMethod]
        new public void DomConstructorMethods()
        {
            base.DomConstructorMethods();
        }

        [TestMethod]
        [WorkItem(230115)]
        new public void Extent()
        {
            base.Extent();
        }

        [TestMethod]
        [WorkItem(212846)]
        new public void CJKCharsInDocComments()
        {
            base.CJKCharsInDocComments();
        }

        [TestMethod]
        new public void UseMemberDocComment()
        {
            base.UseMemberDocComment();
        }

        [TestMethod]
        [WorkItem(192364)]
        new public void ErrorCorrectionNoLCurly()
        {
            base.ErrorCorrectionNoLCurly();
        }

        [TestMethod]
        [WorkItem(192364)]
        new public void ErrorCorrectionNoParamName()
        {
            base.ErrorCorrectionNoParamName();
        }

        [TestMethod]
        [WorkItem(214842)]
        new public void BuiltInFunctionAsAnonymous()
        {
            base.BuiltInFunctionAsAnonymous();
        }

        [TestMethod]
        [WorkItem(90468)]
        [WorkItem(245387)]
        new public void SubsumedFunction()
        {
            base.SubsumedFunction();
        }

        [TestMethod]
        [WorkItem(196251)]
        [WorkItem(183346)]
        [WorkItem(196279)]
        new public void ArrayMethodsReturnType()
        {
            base.ArrayMethodsReturnType();
        }

        [TestMethod]
        [WorkItem(182494)]
        new public void JQuerySignaturesDescription()
        {
            base.JQuerySignaturesDescription();
        }

        [TestMethod]
        [WorkItem(197171)]
        new public void DomOptionalArguments()
        {
            base.DomOptionalArguments();
        }

        [TestMethod]
        new public void DomOverloads()
        {
            base.DomOverloads();
        }

        [TestMethod]
        [WorkItem(183885)]
        new public void MultipleJQueryIncluded()
        {
            base.MultipleJQueryIncluded();
        }

        [TestMethod]
        new public void Bug180443()
        {
            base.Bug180443();
        }

        [TestMethod]
        new public void Bug150800()
        {
            base.Bug150800();
        }

        [TestMethod]
        new public void DefineViaFunctionConstructor()
        {
            base.DefineViaFunctionConstructor();
        }

        [TestMethod]
        [Ignore]
        new public void Bug135826()
        {
            base.Bug135826();
        }

        [TestMethod]
        new public void Bug90468()
        {
            base.Bug90468();
        }

        [TestMethod]
        new public void Bug123984()
        {
            base.Bug123984();
        }

        [TestMethod]
        new public void Bug111891()
        {
            base.Bug111891();
        }

        [TestMethod]
        new public void Bug124704()
        {
            base.Bug124704();
        }

        [TestMethod]
        new public void Bug125633()
        {
            base.Bug125633();
        }

        [TestMethod]
        new public void Bug176776()
        {
            base.Bug176776();
        }

        [TestMethod]
        new public void Bug148407()
        {
            base.Bug148407();
        }

        [TestMethod]
        [Ignore]
        new public void BuiltInObjects()
        {
            base.BuiltInObjects();
        }

        [TestMethod]
        [Ignore]     // Relies on hurry
        new public void Bug123032()
        {
            base.Bug123032();
        }

        [TestMethod]
        new public void DocCommentsInInnerFunction()
        {
            base.DocCommentsInInnerFunction();
        }

        [TestMethod]
        new public void DocCommentsInMiddleOfAFunction()
        {
            base.DocCommentsInMiddleOfAFunction();
        }

        [TestMethod]
        new public void ComplexArgument()
        {
            base.ComplexArgument();
        }

        [TestMethod]
        new public void AnonymousFunction()
        {
            base.AnonymousFunction();
        }

        [TestMethod]
        new public void CommentAfterFuncName()
        {
            base.CommentAfterFuncName();
        }

        [TestMethod]
        new public void RecursiveFunc()
        {
            base.RecursiveFunc();
        }

        [TestMethod]
        new public void Bug91434()
        {
            base.Bug91434();
        }

        [TestMethod]
        new public void Bug75760()
        {
            base.Bug75760();
        }

        [TestMethod]
        new public void Bug111872()
        {
            base.Bug111872();
        }

        [TestMethod]
        new public void CurrentParameterIndex()
        {
            base.CurrentParameterIndex();
        }

        [TestMethod]
        new public void FunctionParamWithSignature()
        {
            base.FunctionParamWithSignature();
        }

        [TestMethod]
        new public void ImplicitSignature()
        {
            base.ImplicitSignature();
        }

        [TestMethod]
        new public void ExplicitSingleSignatureSameAsDecl()
        {
            base.ExplicitSingleSignatureSameAsDecl();
        }

        [TestMethod]
        new public void ExplicitSignatureNotSameAsDecl()
        {
            base.ExplicitSignatureNotSameAsDecl();
        }

        [TestMethod]
        new public void MultipleSignatures()
        {
            base.MultipleSignatures();
        }

        [TestMethod]
        new public void ImplicitAndMultipleSignatures()
        {
            base.ImplicitAndMultipleSignatures();
        }

        [TestMethod]
        new public void MissingReturns()
        {
            base.MissingReturns();
        }

        [TestMethod]
        new public void MissingType()
        {
            base.MissingType();
        }

        [TestMethod]
        new public void MissingParamName()
        {
            base.MissingParamName();
        }

        [TestMethod]
        new public void MissingReturnsDescr()
        {
            base.MissingReturnsDescr();
        }

        [TestMethod]
        new public void invalidContent()
        {
            base.invalidContent();
        }

        [TestMethod]
        new public void MissingSignatureClosingTag()
        {
            base.MissingSignatureClosingTag();
        }

        [TestMethod]
        new public void InvalidTags()
        {
            base.InvalidTags();
        }

        [TestMethod]
        new public void EmptyParamTags()
        {
            base.EmptyParamTags();
        }

        [TestMethod]
        new public void IgnoreComments()
        {
            base.IgnoreComments();
        }

        [TestMethod]
        new public void IgnoreProcessingInstructions()
        {
            base.IgnoreProcessingInstructions();
        }

        [TestMethod]
        new public void PreserveCDATA()
        {
            base.PreserveCDATA();
        }

        [TestMethod]
        new public void PreserveFormating()
        {
            base.PreserveFormating();
        }

        [TestMethod]
        new public void UnknownContent()
        {
            base.UnknownContent();
        }

        [TestMethod]
        new public void ParameterNameMatching()
        {
            base.ParameterNameMatching();
        }

        [TestMethod]
        new public void LongFunctionComment()
        {
            base.LongFunctionComment();
        }


        [TestMethod]
        [Ignore]
        new public void FunctionHelp()
        {
            base.FunctionHelp();
        }

        [TestMethod]
        new public void Bug160093()
        {
            base.Bug160093();
        }

        [TestMethod]
        [WorkItem(210832)]
        new public void ExtraFunctionHelpAttributes()
        {
            base.ExtraFunctionHelpAttributes();
        }

        [TestMethod]
        new public void FunctionHelpSourceFileHandle()
        {
            base.FunctionHelpSourceFileHandle();
        }

        [TestMethod]
        new public void EscapedXML()
        {
            base.EscapedXML();
        }

        [TestMethod]
        new public void ValueElement()
        {
            base.ValueElement();
        }

        [TestMethod]
        new public void DomElement()
        {
            base.DomElement();
        }

        [TestMethod]
        [WorkItem(199843)]
        [WorkItem(205878)]
        new public void InternalFileHandles()
        {
            base.InternalFileHandles();
        }

        [TestMethod]
        new public void EngineHelperFunctions()
        {
            base.EngineHelperFunctions();
        }

        [TestMethod]
        new public void RedirectFunctionDefinition()
        {
            base.RedirectFunctionDefinition();
        }

        [TestMethod]
        new public void AnnotateFunction()
        {
            base.AnnotateFunction();
        }

        [TestMethod]
        new public void AnnotateLambdas()
        {
            base.AnnotateLambdas();
        }

        [TestMethod]
        new public void IndirectAnnotatedFunction()
        {
            base.IndirectAnnotatedFunction();
        }

        [TestMethod]
        new public void AnnotateFunctions()
        {
            base.AnnotateFunctions();
        }

        [TestMethod]
        new public void IndirectAnnotateFunctions()
        {
            base.IndirectAnnotateFunctions();
        }

        [TestMethod]
        new public void CircularAnnotations()
        {
            base.CircularAnnotations();
        }


        [TestMethod]
        new public void AnnotateFields()
        {
            base.AnnotateFields();
        }

        [TestMethod]
        new public void ParameterWithErrors()
        {
            base.ParameterWithErrors();
        }

        [TestMethod]
        new public void CrashTest()
        {
            base.CrashTest();
        }

        [TestMethod]
        [WorkItem(323062)]
        new public void ImplicitRequestTest()
        {
            base.ImplicitRequestTest();
        }

        [TestMethod]
        new public void ImplicitRequestWithArgs()
        {
            base.ImplicitRequestWithArgs();
        }
    }
}