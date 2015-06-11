namespace DirectAuthorCheckinTests
{
    using System.Linq;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    //
    //  Ensure that the same jsdoc completion tests run when extensions are installed. 
    //  Extensibility mechanism roundtrips the data to JS objects and back and we need to ensure that no data is lost in the process.
    //
    [TestClass]
    public class JsDocCommentsWithExtensionsTests : JsDocCommentsTests
    {
        protected override string[] AdditionalContextFiles
        {
            get
            {
                return base.AdditionalContextFiles.Concat(new string[] { extension }).ToArray();
            }
        }
        
        #region Completion Tests

        [TestMethod]
        new public void JsDocTypeTag_Completion()
        {
            base.JsDocTypeTag_Completion();
        }

        [TestMethod]
        new public void JsDocReturnsTag_Completion()
        {
            base.JsDocReturnsTag_Completion();
        }

        [TestMethod]
        new public void JsDocReturnsTagMultipleTypes_Completion()
        {
            base.JsDocReturnsTagMultipleTypes_Completion();
        }

        [TestMethod]
        new public void JsDocTypeTagOnGlobalVariable_Completion()
        {
            base.JsDocTypeTagOnGlobalVariable_Completion();
        }

        [TestMethod]
        new public void JsDocPropertyTagOnVarDeclField_Completion()
        {
            base.JsDocPropertyTagOnVarDeclField_Completion();
        }

        [TestMethod]
        new public void JsDocMultipleMatchingPropertyTagsOnVarDeclField_Completion()
        {
            base.JsDocMultipleMatchingPropertyTagsOnVarDeclField_Completion();
        }

        [TestMethod]
        new public void JsDocPropertyTagOnAssignmentField_Completion()
        {
            base.JsDocPropertyTagOnAssignmentField_Completion();
        }

        [TestMethod]
        new public void JsDocMismatchPropertyTagOnAssignmentField_Completion()
        {
            base.JsDocMismatchPropertyTagOnAssignmentField_Completion();
        }

        [TestMethod]
        new public void JsDocPropertyTagOnFunctionField_Completion()
        {
            base.JsDocPropertyTagOnFunctionField_Completion();
        }

        [TestMethod]
        new public void JsDocMultiplePropertyTagOnFunctionField_Completion()
        {
            base.JsDocMultiplePropertyTagOnFunctionField_Completion();
        }

        [TestMethod]
        new public void JsDocParamTag_Completion()
        {
            base.JsDocParamTag_Completion();
        }

        [TestMethod]
        new public void JsDocParamTagWithHyphen_Completion()
        {
            base.JsDocParamTagWithHyphen_Completion();
        }

        [TestMethod]
        new public void JsDocParamTagWithMultipleLinesDescription_Completion()
        {
            base.JsDocParamTagWithMultipleLinesDescription_Completion();
        }

        #endregion

        #region GetFunctionHelp Tests

        [TestMethod]
        new public void JsDocParamTag_GetFunctionHelp()
        {
            base.JsDocParamTag_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocReturnTag_GetFunctionHelp()
        {
            base.JsDocReturnTag_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocReturnsTag_GetFunctionHelp()
        {
            base.JsDocReturnsTag_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocDeprecatedTag_GetFunctionHelp()
        {
            base.JsDocDeprecatedTag_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocReturnsTagMultipleTypes_GetFunctionHelp()
        {
            base.JsDocReturnsTagMultipleTypes_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocParamTagMultipleTypes_GetFunctionHelp()
        {
            base.JsDocParamTagMultipleTypes_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocParamTagAndReturnsTagMultipleTypes_GetFunctionHelp()
        {
            base.JsDocParamTagAndReturnsTagMultipleTypes_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocDescriptionOnly_GetFunctionHelp()
        {
            base.JsDocDescriptionOnly_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocDescriptionTag_GetFunctionHelp()
        {
            base.JsDocDescriptionTag_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocSummaryTag_GetFunctionHelp()
        {
            base.JsDocSummaryTag_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocDescriptionOnlyWithErrorTag_GetFunctionHelp()
        {
            base.JsDocDescriptionOnlyWithErrorTag_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocParamTagWithErrorTag_GetFunctionHelp()
        {
            base.JsDocParamTagWithErrorTag_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocOptionalParam_GetFunctionHelp()
        {
            base.JsDocOptionalParam_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocStarType_GetFunctionHelp()
        {
            base.JsDocStarType_GetFunctionHelp();
        }

        [TestMethod]
        new public void JsDocFunctionType_GetFunctionHelp()
        {
            base.JsDocFunctionType_GetFunctionHelp();
        }

        #endregion

    }
}
