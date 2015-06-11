namespace DirectAuthorCheckinTests
{
    using System.Linq;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    //
    //  Ensure that the same completion hint tests run when extensions are installed. 
    //  Extensibility mechanism roundtrips the data to JS objects and back and we need to ensure that no data is lost in the process.
    //
    [TestClass]
    public class CompletionHintWithExtensions : CompletionHintTests
    {
        protected override string[] AdditionalContextFiles
        {
            get
            {
                return base.AdditionalContextFiles.Concat(new string[] { extension }).ToArray();
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
        [WorkItem(414972)]
        new public void PropertyDescriptorDetection()
        {
            base.PropertyDescriptorDetection();
        }

        [TestMethod]
        new public void EmptyLineInCommentBlock()
        {
            base.EmptyLineInCommentBlock();
        }

        [TestMethod]
        new public void PropertyReturningFunction()
        {
            base.PropertyReturningFunction();
        }

        [TestMethod]
        new public void PropertyValue()
        {
            base.PropertyValue();
        }

        [TestMethod]
        [WorkItem(312521)]
        new public void getVarDate()
        {
            base.getVarDate();
        }

        [TestMethod]
        [WorkItem(273966)]
        new public void MemberFunctionDescription()
        {
            base.MemberFunctionDescription();
        }


        [TestMethod]
        new public void UndefinedFieldsDocComments()
        {
            base.UndefinedFieldsDocComments();
        }

        [TestMethod]
        new public void FileChangeAfterCompletionRequest()
        {
            base.FileChangeAfterCompletionRequest();
        }

        [TestMethod]
        [WorkItem(210501)]
        new public void PrototypeFieldDocComments()
        {
            base.PrototypeFieldDocComments();
        }

        [TestMethod]
        [WorkItem(192364)]
        new public void BeginningOfScopeWithDirectExecution()
        {
            base.BeginningOfScopeWithDirectExecution();
        }

        [TestMethod]
        new public void ReuseFileHandleInAnotherContext()
        {
            base.ReuseFileHandleInAnotherContext();
        }

        [TestMethod]
        new public void EngineReleaseBeforeCompletionSetRelease()
        {
            base.EngineReleaseBeforeCompletionSetRelease();
        }

        [TestMethod]
        [WorkItem(198432)]
        new public void MalformedDocCommentsAboveVar()
        {
            base.MalformedDocCommentsAboveVar();
        }

        [TestMethod]
        [WorkItem(198574)]
        new public void LocalVariableComments()
        {
            base.LocalVariableComments();
        }

        [TestMethod]
        [WorkItem(181280)]
        new public void Bug181280()
        {
            base.Bug181280();
        }

        [TestMethod]
        new public void StaticFields()
        {
            base.StaticFields();
        }

        [TestMethod]
        new public void ArgumentDocComments()
        {
            base.ArgumentDocComments();
        }

        [TestMethod]
        new public void ConstructorFieldsDocComments()
        {
            base.ConstructorFieldsDocComments();
        }

        [TestMethod]
        new public void ReturnDocComments()
        {
            base.ReturnDocComments();
        }

        [TestMethod]
        new public void DocumentingReturnValue()
        {
            base.DocumentingReturnValue();
        }

        [TestMethod]
        new public void RecyclerTest()
        {
            base.RecyclerTest();
        }

        [TestMethod]
        new public void builtInFunctions()
        {
            base.builtInFunctions();
        }

        [TestMethod]
        new public void Bug181277()
        {
            base.Bug181277();
        }

        [TestMethod]
        new public void CompletionHintExtensions()
        {
            base.CompletionHintExtensions();
        }

        [TestMethod]
        [Ignore]
        //TODO: basipov - 'this' should appear in completions list but currently it is not.
        new public void thisInFunc()
        {
            base.thisInFunc();
        }

        [TestMethod]
        new public void FieldDocComments()
        {
            base.FieldDocComments();
        }

        [TestMethod]
        [WorkItem(335240)]
        new public void FieldDocCommentFile()
        {
            base.FieldDocCommentFile();
        }

        [TestMethod]
        new public void VarDocComments()
        {
            base.VarDocComments();
        }

        [TestMethod]
        [Ignore]
        new public void ParamDocComments()
        {
            base.ParamDocComments();
        }

        [TestMethod]
        new public void localObjectProperty()
        {
            base.localObjectProperty();
        }

        [TestMethod]
        new public void funcProperty()
        {
            base.funcProperty();
        }

        [TestMethod]
        new public void localArray()
        {
            base.localArray();
        }


        [TestMethod]
        new public void localFunction()
        {
            base.localFunction();
        }

        [TestMethod]
        new public void globalObjectWithDocComments()
        {
            base.globalObjectWithDocComments();
        }

        [TestMethod]
        new public void globalNumber()
        {
            base.globalNumber();
        }

        [TestMethod]
        new public void localNumber()
        {
            base.localNumber();
        }

        [TestMethod]
        [Ignore]
        // TODO: basipov - temporarily disabled due to a bug. Supposed to return ascopeClosure.
        new public void closureObject()
        {
            base.closureObject();
        }

        [TestMethod]
        new public void lambdaObject()
        {
            base.lambdaObject();
        }

        [TestMethod]
        new public void localParam()
        {
            base.localParam();
        }

        [TestMethod]
        new public void insideCallArgs()
        {
            base.insideCallArgs();
        }

        [TestMethod]
        new public void paramOfFunctionType()
        {
            base.paramOfFunctionType();
        }

        [TestMethod]
        new public void ExternalFileAttribute()
        {
            base.ExternalFileAttribute();
        }

        [TestMethod]
        [WorkItem(199843)]
        [WorkItem(205878)]
        new public void InternalFileHandles()
        {
            base.InternalFileHandles();
        }

        [TestMethod]
        new public void ContextChanges()
        {
            base.ContextChanges();
        }

        [TestMethod]
        new public void BuiltinFunctions()
        {
            base.BuiltinFunctions();
        }

        [TestMethod]
        [WorkItem(362097)]
        new public void MemberCommentAssociation()
        {
            base.MemberCommentAssociation();
        }


        [TestMethod]
        [WorkItem(362097)]
        new public void CommentAfterIfStatement()
        {
            base.CommentAfterIfStatement();
        }

        [TestMethod]
        new public void DeprecatedFieldDocComments()
        {
            base.DeprecatedFieldDocComments();
        }

        [TestMethod]
        new public void DeprecatedFunctionWithFieldDoc()
        {
            base.DeprecatedFunctionWithFieldDoc();
        }

        [TestMethod]
        new public void DeprecatedSignatureDocComments()
        {
            base.DeprecatedSignatureDocComments();
        }

        [TestMethod]
        new public void DeprecatedVarDocComments()
        {
            base.DeprecatedVarDocComments();
        }

        [TestMethod]
        new public void GetDeprecatedReturnsNullWhenNoDeprecatedAttribute()
        {
            base.GetDeprecatedReturnsNullWhenNoDeprecatedAttribute();
        }

        [TestMethod]
        new public void GetDeprecatedReturnsNonNullForEmptyDeprecatedAttribute()
        {
            base.GetDeprecatedReturnsNonNullForEmptyDeprecatedAttribute();
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        new public void CompatibleWithFieldDocComments()
        {
            base.CompatibleWithFieldDocComments();
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        new public void CompatibleWithFromFunctionFieldDoc()
        {
            base.CompatibleWithFromFunctionFieldDoc();
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        new public void CompatibleWithSignatureDocComments()
        {
            base.CompatibleWithSignatureDocComments();
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        new public void CompatibleWithVarDocComments()
        {
            base.CompatibleWithVarDocComments();
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        new public void GetCompatibleWithWhenNoCompatibleWithAttribute()
        {
            base.GetCompatibleWithWhenNoCompatibleWithAttribute();
        }

        [TestMethod]
        [TestCategory("CompatibleWith")]
        new public void GetCompatibleWithReturnsNonNullForEmptyAttribute()
        {
            base.GetCompatibleWithReturnsNonNullForEmptyAttribute();
        }
    }
}
