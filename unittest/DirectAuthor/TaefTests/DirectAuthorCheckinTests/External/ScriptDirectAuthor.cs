//-----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.JavaScript.LanguageService.Shared;

// Disable warnings for uninitialized fields in this file as it simply defines the correct marshalling for structures
// that will be passed in through Chakra
#pragma warning disable 649

namespace Microsoft.VisualStudio.JavaScript.LanguageService.Engine
{
    public static class AuthorConstants
    {
        // See Parser::InitPids() in \inetcore\jscript\lib\parser\parse.cpp.
        public const string ErrorIdentifier = "?";
    }

    // Conversion of ScriptDirectAuthor.dll
    public enum AuthorTokenKind : uint
    {
        // No more tokens are in the provided range.
        atkEnd,

        // The token is normal text (or otherwise unclassified token)
        atkText,

        // The token is an identifier
        atkIdentifier,

        // The token is a comment.
        atkComment,

        // The token is a numberic literal.
        atkNumber,

        // The token is a string literal.
        atkString,

        // The token is a regular expression literal.
        atkRegexp,

        // The Scanner has encountered an error in syntax coloring.
        atkScanError,

        // The token is ";".
        atkSColon,

        // The token is "(".
        atkLParen,

        // The token is ")".
        atkRParen,

        // The token is "[".
        atkLBrack,

        // The token is "]".
        atkRBrack,

        // The token is "{".
        atkLCurly,

        // The token is "}".
        atkRCurly,

        // The token is ",".
        atkComma,

        // The token is "=>".
        atkDArrow,

        // The token is "=".
        atkAsg,

        // The token is "+=".
        atkAsgAdd,

        // The token is "-=".
        atkAsgSub,

        // The token is "*=".
        atkAsgMul,

        // The token is "/=".
        atkAsgDiv,

        // The token is "%=".
        atkAsgMod,

        // The token is "&=".
        atkAsgAnd,

        // The token is "^=".
        atkAsgXor,

        // The token is "|=".
        atkAsgOr,

        // The token is "<<=".
        atkAsgLsh,

        // The token is ">>=".
        atkAsgRsh,

        // The token is ">>>=".
        atkAsgRs2,

        // The token is "?".
        atkQMark,

        // The token is ":".
        atkColon,

        // The token is "||".
        atkLogOr,

        // The token is "&&".
        atkLogAnd,

        // The token is "|".
        atkOr,

        // The token is "^".
        atkXor,

        // The token is "&".
        atkAnd,

        // The token is "==".
        atkEQ,

        // The token is "!=".
        atkNE,

        // The token is "===".
        atkEqv,

        // The token is "!==".
        atkNEqv,

        // The token is "<".
        atkLT,

        // The token is "<=".
        atkLE,

        // The token is ">".
        atkGT,

        // The token is ">=".
        atkGE,

        // The token is "<<".
        atkLsh,

        // The token is ">>".
        atkRsh,

        // The token is ">>>".
        atkRs2,

        // The token is "+".
        atkAdd,

        // The token is "-".
        atkSub,

        // The token is "*".
        atkStar,

        // The token is "/".
        atkDiv,

        // The token is "%".
        atkPct,

        // The token is "~".
        atkTilde,

        // The token is "!".
        atkBang,

        // The token is "++".
        atkInc,

        // The token is "--".
        atkDec,

        // The token is ".".
        atkDot,

        // The token is "::".
        atkScope,

        // The token is "...".
        atkEllipsis,

        // The token is the "break" keyword.
        atkBreak,

        // The token is the "case" keyword.
        atkCase,

        // The token is the "catch" keyword.
        atkCatch,

        // The token is the "class" keyword.
        atkClass,

        // The token is the "const" keyword.
        atkConst,

        // The token is the "continue" keyword.
        atkContinue,

        // The token is the "debugger" keyword.
        atkDebugger,

        // The token is the "default" keyword.
        atkDefault,

        // The token is the "delete" keyword.
        atkDelete,

        // The token is the "do" keyword.
        atkDo,

        // The token is the "else" keyword.
        atkElse,

        // The token is the "enum" keyword.
        atkEnum,

        // The token is the "export" keyword.
        atkExport,

        // The token is the "extends" keyword.
        atkExtends,

        // The token is the "else" keyword.
        atkFalse,

        // The token is the "finally" keyword.
        atkFinally,

        // The token is the "for" keyword.
        atkFor,

        // The token is the "function" keyword.
        atkFunction,

        // The token is the "if" keyword.
        atkIf,

        // The token is the "import" keyword.
        atkImport,

        // The token is the "in" keyword.
        atkIn,

        // The token is the "instanceof" keyword.
        atkInstanceof,

        // The token is the "new" keyword.
        atkNew,

        // The token is the "null" keyword.
        atkNull,

        // The token is the "return" keyword.
        atkReturn,

        // The token is the "super" keyword.
        atkSuper,

        // The token is the "switch" keyword.
        atkSwitch,

        // The token is the "this" keyword.
        atkThis,

        // The token is the "throw" keyword.
        atkThrow,

        // The token is the "true" keyword.
        atkTrue,

        // The token is the "try" keyword.
        atkTry,

        // The token is the "typeof" keyword.
        atkTypeof,

        // The token is the "var" keyword.
        atkVar,

        // The token is the "void" keyword.
        atkVoid,

        // The token is the "while" keyword.
        atkWhile,

        // The token is the "with" keyword.
        atkWith,

        // The token is the future reserved keyword "implements".
        atkImplements,

        // The token is the future reserved keyword "interface".
        atkInterface,

        // The token is the future reserved keyword "let".
        atkLet,

        // The token is the future reserved keyword "package".
        atkPackage,

        // The token is the future reserved keyword "private".
        atkPrivate,

        // The token is the future reserved keyword "protected".
        atkProtected,

        // The token is the future reserved keyword "public".
        atkPublic,

        // The token is the future reserved keyword "static".
        atkStatic,

        // The token is the future reserved keyword "yield".
        atkYield,

        // The token represents a string template.
        atkStrTemplate,

        // The token is the heuristically identified contextual keyword "of" used in a for-of loop
        atkOf
    }

    public enum AuthorTaskCommentPriority : uint
    {
        Low,
        Normal,
        High
    }

    // A value used to store syntax state information needed between tokenization ranges.
    public enum AuthorSourceState : uint
    {
        // The initial state that is the state of the colorizer at the beginning of file.
        SOURCE_STATE_INITIAL = 0,

        // A number defining the potentially bits the state can used to repesent scanner state.
        SOURCE_STATE_INTERNAL = 0xffffffff
    }

    // Represents the kind of a multi-line token.
    public enum AuthorMultilineTokenKind
    {
        // The current token is not a multi-line token.
        amtkNone,

        // The current token is a multi-line comment.
        amtkMultilineComment,

        // The current token is a multi-line string.
        amtkMultilineString
    }

    // Represents the various supported script versions.
    public enum AuthorScriptVersion
    {
        ScriptVersion10,
        ScriptVersion11,
        ScriptVersion12
    }

    // Represents a source token.
    public struct AuthorTokenColorInfo
    {
        // The number of UTF-16 code-points from the beginning of the tokenization range is the start of the token.
        public int StartPosition;

        // The number of UTF-16 code-points from the beginning of the tokenization range is the end of the token.
        public int EndPosition;

        // The kind of token represented by the given range.
        public AuthorTokenKind Kind;

        // The state of the scanner after the token. The State must be valid at the end of the tokenization range
        // (i.e. when atkEnd is returned) but it is implementation defined if State is valid after each token.
        public AuthorSourceState State;
    }

    // Represents a range of tokens.
    [Guid(GuidStrings.IAuthorTokenEnumeratorGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorTokenEnumerator
    {
        // Advance the enumeration and retrieve the current token information. 
        AuthorTokenColorInfo Next();
    }

    // Represents a colorization service
    [Guid(GuidStrings.IAuthorColorizeTextGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorColorizeText
    {
        // Obtain a colorized enumeration of tokens in the given text. The state should be the last state returned by a
        // IAuthorTokenEnumerator to the atkEnd token or SOURCE_STATE_INITIAL if the source is the at the beginning of the
        // file.
        IAuthorTokenEnumerator Colorize(string text, int length, AuthorSourceState state);

        // Obtain an AuthorSourceState at the end of the given text without requiring the tokens.
        AuthorSourceState GetStateForText(string text, int length, AuthorSourceState state);

        // Returns the multi-line token kind of the token the tokenizer is processing, given authoring state.
        AuthorMultilineTokenKind GetMultilineTokenKind(AuthorSourceState state);
    }

    // Represents the text of a file.
    [Guid(GuidStrings.IAuthorFileReaderGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorFileReader
    {
        // Read length UTF-16 code-points from a file that is assumed to be offset UTF-16 code-points
        // from the beginning of the file. If there isn't length UTF-16 code-points after offset
        // only the remaing code-points should be copied. read should be updated with the actual number 
        // of code-points copied into buffer.
        int Read(int length, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 0), Out] char[] buffer);

        // Seeks to the specified offset. The offset is specified in code points.
        void Seek(int offset);

        // Closes the reader.
        void Close();
    }

    [Flags]
    public enum AuthorFileStatus
    {
        // No messages were generated for the file
        afsNoMessages = 0x000,

        // Information messages were generated
        afsInformation = 0x001,

        // Warning messages were generated
        afsWarnings = 0x002,

        // Error messages were generated
        afsErrors = 0x004,

        // A mask to detemine which part of the file status referes to file messages
        afsMessageMask = 0x0FF,

        // The file's ranges are not necessarily valid and the result of GetRegions() should be ignored.
        // This will be true if, for exmaple, error correction skipped a brace.
        afsInvalidRegions = 0x100
    }

    // Indicates the kind of message is represented by an AuthorFileMessage
    public enum AuthorMessageKind
    {
        // The message represents an error in the file at parse time or runtime.
        amkError,

        // The message represents a warning generated at parse time.
        amkWarning,

        // The message represents information about the file.
        amkInformation
    }

    // Represents a parse message including errors and warnings
    public struct AuthorFileMessage
    {
        // The kind of message this is.
        public AuthorMessageKind Kind;

        // The count the UTF-16 code-points from the beginning of the file where the error occured.
        public int Position;

        // The number of UTF-16 code-points in the lexical element in error. Could be 0.
        public int Length;

        // The text of the message
        [MarshalAs(UnmanagedType.BStr)]
        public string Message;

        // The ID of the message
        public int MessageID;
    }

    // All error id values
    public enum AuthorMessageId
    {
        ERRnoMemory = 1001,
        ERRsyntax = 1002,
        ERRnoColon = 1003,
        ERRnoSemic = 1004,
        ERRnoLparen = 1005,
        ERRnoRparen = 1006,
        ERRnoRbrack = 1007,
        ERRnoLcurly = 1008,
        ERRnoRcurly = 1009,
        ERRnoIdent = 1010,
        ERRnoEq = 1011,
        ERRnoSlash = 1012,
        ERRbadNumber = 1013,
        ERRillegalChar = 1014,
        ERRnoStrEnd = 1015,
        ERRnoCmtEnd = 1016,

        ERRbadReturn = 1018,
        ERRbadBreak = 1019,
        ERRbadContinue = 1020,

        ERRbadHexDigit = 1023,
        ERRnoWhile = 1024,
        ERRbadLabel = 1025,
        ERRnoLabel = 1026,
        ERRdupDefault = 1027,
        ERRnoMemberIdent = 1028,
        ERRnoCcEnd = 1029,
        ERRccOff = 1030,
        ERRnotConst = 1031,
        ERRnoAt = 1032,
        ERRnoCatch = 1033,
        ERRnoVar = 1034,
        ERRdanglingThrow = 1035,

        ERRWithNotInCP = 1036,
        ERRES5NoWith = 1037,
        ERRES5ArgSame = 1038,
        ERRES5NoOctal = 1039,
        ERREvalUsage = 1041,
        ERRArgsUsage = 1042,
        ERRInvalidDelete = 1045,
        ERRDupeObjLit = 1046,
        ERRFncDeclNotSourceElement = 1047,
        ERRKeywordNotId = 1048,
        ERRFutureReservedWordNotId = 1049,
        ERRFutureReservedWordInStrictModeNotId = 1050,
        ERRSetterMustHaveOneArgument = 1051,
        ERRRedeclaration = 1052,
        ERRUninitializedConst = 1053,
        ERRDeclOutOfStmt = 1054,
        ERRAssignmentToConst = 1055,
        ERRUnicodeOutOfRange = 1056,
        ERRInvalidSpreadUse = 1057,
        ERRInvalidSuper = 1058,
        ERRInvalidSuperScope = 1059,
        ERRSuperInIndirectEval = 1060,
        ERRSuperInGlobalEval = 1061,
        ERRnoDArrow = 1062,
        ERRInvalidCodePoint = 1063,
        ERRMissingCurlyBrace = 1064,
        ERRMethodFormalRedeclaration = 1065,
        ERRNonSimpleParameterRedeclaration = 1066,
        ERRRestLastArg = 1067,
        ERRRestWithDefault = 1068,
        ERRUnexpectedDotDotDot = 1069,

        ERRRegExpSyntax = 5017,
        ERRRegExpBadQuant = 5018,
        ERRRegExpNoBracket = 5019,
        ERRRegExpNoParen = 5020,
        ERRRegExpBadRange = 5021
    }

    // Represents a set of messages from the parser including errors and warnings.
    [Guid(GuidStrings.IAuthorMessageSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorMessageSet
    {
        // Return the total number of messages
        int Count { get; }

        // Return count errors starting at error index.
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] AuthorFileMessage[] messages);
    }

    // Flags that indicates information about the reference found. Note that some flags are mutually exclusive
    // such as asrfAuthoritative and asrfSuggestive, and asrfLocalReference, asrfGlobalReference and 
    // asrfMemberReference. At least one flag must be set of a valid AuthorSymbolReferenceFlags value.
    public enum AuthorSymbolReferenceFlags
    {
        // The symbol is known to be an unambiguous reference to the set identifier such as in the case of a parameter, var, 
        // let or const in a closure scope that do not contain a direct eval and the reference is not in a with scope.
        asrfAuthoritative = 0x0001,

        // The symbol might be a reference to the set identifier such as a member reference or a reference when an eval
        // or with statement might affect the scope.
        asrfSuggestive = 0x0002,

        // The symbol is used in the left-hand side of an expression (it is assigned to).
        asrfLValue = 0x0004,

        // The symbol is used in the right-hand side of an expression (it is read from).
        asrfRValue = 0x0008,

        // The symbol is a reference to a parameter, variable, let or const in a closure scope (e.g. not a global reference)
        asrfLocalReference = 0x0010,

        // The symbol is a reference to a variable in global scope.
        asrfGlobalReference = 0x0020,

        // The symbol is used to reference a member such as after a '.' or in a object literal.
        asrfMemberReference = 0x0040,

        // The symbol is referenced in the context of a with statement or an direct eval which might affect the scope.
        asrfScopeInDoubt = 0x0080,
    }

    // A struct containing the location information of a reference as well as descriptive flags about the kind of reference.
    public struct AuthorSymbolReference
    {
        public AuthorSymbolReferenceFlags Flags;

        // The count of UTF-16 code-units from the beginning of the file where the reference is located
        public int Position;

        // The number of UTF-16 code-units that make up the reference. This is included because the reference might include escapes 
        // and might not match the identifier length.
        public int Length;
    }

    // A set of references to an identifier supplied to the method creating this result.
    [Guid(GuidStrings.IAuthorReferenceSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorReferenceSet
    {
        // The identifier referenced
        string Identifier { get; }

        // Return the total number of symbol references in set
        int Count { get; }

        // Return count references starting at startIndex.
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] AuthorSymbolReference[] references);
    }

    // Represents a change to an authoring file.
    public struct AuthorFileChange
    {
        // The number of UTF-16 code-points from the beginning of the file where the modification occured.
        public int Offset;

        // The number of UTF-16 code-points removed (which could be 0 for insert operations).
        public int CharactersRemoved;

        // The number of UTF-16 code-points inserted (which could be 0 for delete operations).
        public int CharactersInserted;
    }

    // Represents a file for use in authoring.
    [Guid(GuidStrings.IAuthorFileGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorFile
    {
        // Returns a name that would be suitable for including in a message returnend in a IAuthorMessageSet. This
        // value does not need to be unique among all IAuthorFile's created and is only used for including in
        // a message set.
        string DisplayName { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Returns the length of the file.
        int Length { get; }

        // Retrieve a reader for the current content of the file.
        IAuthorFileReader GetReader();

        // Called each time after the file has been parsed to indicate the status of the file.
        void StatusChanged(AuthorFileStatus newStatus);
    }

    // Represents a file registered with an instance IAuthoringServices for use in producing an 
    // IAuthorFileAuthoring service.
    [Guid(GuidStrings.IAuthorFileHandleGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorFileHandle
    {
        // Must be called whenever the file has been modified with an accurate list of the changes made.
        // If accurate change inforamtion cannot be determiend the offset in the AuthorFileChange struct
        // should be -1 and the charactersRemoved and charactersInserted will be ignored.
        void FileChanged(int changeCount, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 0)] AuthorFileChange[] changes);

        // Returns a set of messages produced about the file including errors and warnings.
        IAuthorMessageSet GetMessageSet();

        // Close the file. This will cause the corresponding IAuthorFile to be released by authoring services.
        // All subsequent calls on to FileChanged will be ignored and GetMessageSet will return and empty set.
        void Close();

        // Returns a unique long for the file handle that is unique for every IAuthorFileHandle instance.
        long FileId { get; }
    }

    // Indicates the phases a file authoring can be in while servicing a request.
    public enum AuthorFileAuthoringPhase
    {
        // The file authoring is not analyzing.
        afpDormant,

        // The file authoring is parsing a file.
        afpParsing,

        // The file authoring is preparing a file for execution.
        afpPreparing,

        // The file is executing a file. This is the only phase in which calling Hurry() is meaningful.
        afpExecuting
    }

    // Indicates the status of a context file requested through an asynchronous script.
    public enum AuthorFileAsynchronousScriptStatus
    {
        // The asynchronous script is already in the current context.
        afassLoaded,

        // The asynchronous script was not in the current script context, but has been added.
        afassAdded,

        // The asynchronous script is not in the current script, and should be ignored.
        afassIgnored
    }

    // Indicates the type of the host for a specific IAuthorFileContext.
    public enum AuthorHostType
    {
        // Currently this implies enabled legacy language features, use it for IE.
        ahtBrowser,

        // Currently this implies legacy-free language features, use it for AppHost.
        ahtApplication
    }

    // Represents a context for a single JavaScript file. The order of the context files returned by GetContextFiles()
    // is assumed to be the order in which the will be executed at runtime. If, for example, two files define the same
    // global symbol, the file later in the context will obscure the earlier file.
    [Guid(GuidStrings.IAuthorFileContextGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorFileContext
    {
        // Called to retrieve the file handle for which this is a context for. This IAuthorFileHandle must be a
        // handle returned by calling RegisterFile() on the same IAuthoringServices this was passed instance
        // was passed as a parameter to GetFileAuthoring().
        IAuthorFileHandle GetPrimaryFile();

        // Retrieve the number of context files.
        int ContextFileCount { get; }

        // Called to retrieve the file handles for files that should be considered to already have executed prior 
        // to the file for which this context represents. The IAuthorFileHandle's must be instances returned by 
        // calling RegisterFile() on the same IAuthoringServices this was passed instance was passed as a parameter 
        // to GetFileAuthoring().
        void GetContextFiles(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] IAuthorFileHandle[] files);

        // Retrieve the type of the host for this IAuthorFileContext instance. The host type will be used to initialize
        // the JavaScript engine enabling or disabling certain features.
        AuthorHostType HostType { get; }

        // Called when at the start of a new phase or called with afpDormant if the current last phase is completed.
        // The phaseId must be used in calls to FileAuthoring::Hurry(). All Hurry calls will be ignored if they
        // are not for the current phase.
        void FileAuthoringPhaseChanged(AuthorFileAuthoringPhase phase, int phaseId, IAuthorFileHandle executingFile);

        // Called when a extension of the language service wishes to display a log message to the user.
        void LogMessage([MarshalAs(UnmanagedType.BStr)] string message);

        // Called to retrieve check if a script loaded asynchronously is part of the current context or not.
        // If the script is "Loaded" the onload event on the script request will be executed.
        // If the script is "Missing" current engine call will terminate unsuccessfully as the context is incomplete.
        // If the script is "Ignored" it will be skipped, and no events will be executed.
        // If insertBeforeSourceFile is set to true, the requested script needs to be loaded for the source file to execute correctly
        // otherwise, the script should be added after the source file in the context list.
        AuthorFileAsynchronousScriptStatus VerifyScriptInContext(
                [MarshalAs(UnmanagedType.BStr)] string url,
                [MarshalAs(UnmanagedType.BStr)] string charset,
                [MarshalAs(UnmanagedType.BStr)] string type,
                IAuthorFileHandle sourceFile,
                [MarshalAs(UnmanagedType.VariantBool)] bool insertBeforeSourceFile);

        // Retrieve the target script version for this set of files.
        AuthorScriptVersion GetScriptVersion();
    }

    // Represents a region of code-points from an authoring file.
    public struct AuthorFileRegion
    {
        // The count of UTF-16 code-points from the beginning of the file where the region begins.
        public int Offset;

        // The number of UTF-16 code-points in the region.
        public int Length;
    }

    // Represents a task comment from an authoring file.
    public struct AuthorFileTaskComment
    {
        // The count of UTF-16 code-points from the beginning of the file where the task comment begins.
        public int Offset;

        // The number of UTF-16 code-points in the task comment.
        public int Length;

        // priority of the task comment prefix
        public AuthorTaskCommentPriority Priority;
    }

    // Represent a prefix for finding task comment
    public struct AuthorTaskCommentPrefix
    {
        // text for the task comment prefix
        [MarshalAs(UnmanagedType.LPWStr)]
        public string TaskCommentPrefixText;

        // length of the task comment prefix in characters
        public int TaskCommentPrefixLength;

        // priority of the task comment prefix
        public AuthorTaskCommentPriority Priority;
    }

    // Indicates the kind of completion returned in a completion set.
    public enum AuthorCompletionKind
    {
        // The completion is a reference to a function or method (a slot containing a function).
        ackMethod,

        // The completion is a reference to a field (a slot not containing a function).
        ackField,

        // The completion is a reference to a ES5 property.
        ackProperty,

        // The completion is a reserved word.
        ackReservedWord,

        // The completion is a simple identifier.
        ackIdentifier,

        // The completion is a reference to a parameter.
        ackParameter,

        // The completion is a reference to a variable.
        ackVariable,

        // The completion is a reference to a label.
        ackLabel

    }

    // Specifies flags that can be used to obtain completions.
    [Flags]
    public enum AuthorCompletionFlags
    {
        // Return members or symbols from the current scope.
        acfMembersFilter = 0x0001,

        // Return all identifiers in the current file.
        acfFileIdentifiersFilter = 0x0002,

        // Return reserved words.
        acfSyntaxElementsFilter = 0x0004,

        // Return all available completions
        acfAny = 0x00FF,

        // Only return completions for locations in the file that an implicit completion would be appropriate 
        // (e.g. after a dot operartor but not a period in a comment or string).
        acfImplicitRequest = 0x0100,
    }

    // Specifies flags that can be used to provide diagnostic info about a completion session.
    [Flags]
    public enum AuthorCompletionDiagnosticFlags
    {
        // Default value with no flags set.
        acdfNone = 0,

        // Execution of the primary file was stopped.
        acdfPrimaryFileHalted = 0x0001,

        // Execution of a context file was stopped.
        acdfContextFileHalted = 0x0002,

        // Type information for the object was inferred from doc comments.
        acdfObjectTypeInferred = 0x0004,

        // Type information for a member of the completion list was inferred.
        acdfMemberTypeInferred = 0x0008,

        // Indicates successful execution to the breakpoint. 
        acdfSuccessfulExecution = 0x0100,
    }

    // Specific flags that can be used to obtain function help
    [Flags]
    public enum AuthorFunctionHelpFlags
    {
        // Default request.
        afhfDefault = 0,

        // Only return function help for locations in the file that an implicit function help request would be appropriate
        // (e.g. not in a string literal, comment, etc.).
        afhfImplicitRequest = 0x0100
    }

    // Indicates what kind of completion set this represents.
    public enum AuthorCompletionSetKind
    {
        // The completion set is completing the right side of a dot operator.
        acskMember,

        // The completion set complets an element in statement scope.
        acskStatement
    }

    // Indicates the kind of object, if any,  the completion list was generated for.
    public enum AuthorCompletionObjectKind
    {
        // The completion list is not for an object (e.g. a statement completions, reserved word list, etc).
        acokNone,

        // The member in the completion list represent members of a normal JavaScript dynamic object.
        acokDynamic,

        // The object was determined to be the undefined object.
        acokUndefined,

        // The object was determined to be the null object.
        acokNull,

        // The object was determined is a dummy object that was generated as a result of an exception.
        acokError
    }

    // Represents a completion.
    public struct AuthorCompletion
    {
        // The kind of completion this is.
        public AuthorCompletionKind Kind;

        // A cookie used to identify this completion. This should be treated as an opaque value used 
        // used to identify the completion.
        public int CompletionCookie;

        // The name of the completion. If completions are sorted this is the key that should be used.
        [MarshalAs(UnmanagedType.BStr)]
        public string Name;

        // If this completion is selected, this is the text that should replace the text indicated by
        // the extent of the completion set.
        [MarshalAs(UnmanagedType.BStr)]
        public string InsertionText;

        // This is the text that should be displayed to indicate to the user what the completion 
        // represents.
        [MarshalAs(UnmanagedType.BStr)]
        public string DisplayText;

        // This is indicates which filter this completion is a part of. A completion could be part
        // of more than one group (e.g. when a function is defined in the file it will be part of 
        // members group and in the file identifiers for statement level completions).
        public AuthorCompletionFlags Group;

        [MarshalAs(UnmanagedType.BStr)]
        public string Glyph;
    }

    // Represents diagnostic information that can be used to diagnose why an authoring request failed
    // to produce a result (but returned S_OK).
    public enum AuthorDiagStatus
    {
        // No error was encountered.
        adsSuccess,

        // The offset passed does not refer to a location that has information requested.
        adsOperationNotApplicableAtLocation,

        // The offset was not code.
        adsCodeLocationUnreachable,

        // The source location was not found in the file.
        adsCouldNotFindSourceAtLocation,

        // The evaluation of the expression at offset failed to produce a result.
        adsNoEvaluationResult,

        // The evaluation failed unexpectedly.
        adsUnexpectedEvaluationResult,

        // No function declaration could be found for the function at offset.
        adsFunctionDeclarationUnavailable,

        // An public parser error occurred.
        adsInvalidAST,

        // An public parser error occurred while parsing function XML doc comments.
        adsInvalidDocComment
    }

    // Represents a parameter help info
    [Guid(GuidStrings.IAuthorParameterGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorParameter
    {
        // Parameter name, cannot be empty. This value comes from xml doc <param name=""> attribute 
        // or from JS function definition if xml doc info is not available.
        string Name { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Parameter type, can be empty if not specified in xml doc
        string Type { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Parameter description, can be empty. The value is coming from <param> tag contents or from <summary> tag under <param> tag. 
        string Description { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The member id used to locate the block of XML which is then processed to find any description information associated with 
        // fields, can be empty.
        string Locid { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The type of the element is Array, this specifies the type of the elements in the array, can be empty.
        string ElementType { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Specifies whether the parameter needs to be specified by the caller. Default to "false" if not provided, or value is not equal to "true".
        bool Optional { [return: MarshalAs(UnmanagedType.VariantBool)]get; }

        // Optional, is provided when paramType="Function" and the <param> element has a child <signature> tag.
        IAuthorSignature FunctionParamSignature { get; }
    }

    // Represents a set of parameters
    [Guid(GuidStrings.IAuthorParameterSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorParameterSet
    {
        // The number of function signatures.
        int Count { get; }

        // Retrieves count number of IParamInfo's starting at startIndex.
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] IAuthorParameter[] parameters);
    }

    // Represents a return value help info
    [Guid(GuidStrings.IAuthorReturnValueGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorReturnValue
    {
        // Function return type. Will have a non-empty value if return type is specified in xml doc comments via <returns> tag.
        string Type { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Function return value description. Will have a non-empty value if return value description is specified in xml doc comments via <returns> tag.
        string Description { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The member id used to locate the block of XML which is then processed to find any description information associated with 
        // fields, can be empty.
        string Locid { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The type of the element is Array, this specifies the type of the elements in the array, can be empty.
        string ElementType { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Specifies the keyword to use when invoking F1 help in the editor, can be empty.
        string HelpKeyword { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The name of the file in which to find the external id. 
        string ExternalFile { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The member id used to locate the relevant xml associated with this signature, can be empty. Unlike locid on the 
        // elements themselves, this id indicates that all tags from the member with this id should be loaded and where 
        // applicable description information should be merged into the tags specified within the signature.
        string Externalid { [return: MarshalAs(UnmanagedType.BStr)] get; }

    }

    // Represents a deprecated attribute help info
    [Guid(GuidStrings.IAuthorDeprecatedInfoGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorDeprecatedInfo
    {
        // Deprecated type. Will have a non-empty value if deprecated type is specified in xml doc comments via <deprecated> tag.
        // Valid types are "deprecate" and "remove".
        string Type { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Deprecated message. Will have a non-empty value if deprecated message is specified in xml doc comments via <deprecated> tag.
        string Message { [return: MarshalAs(UnmanagedType.BStr)] get; }
    }

    // Represents a compatibleWith attribute
    [Guid(GuidStrings.IAuthorCompatibleWithInfoGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorCompatibleWithInfo
    {
        // Platform. Will have non-empty value if platform is specified in xml doc comments via <compatibleWith> tag.
        // Valid types are "Windows" and "WindowsPhone".
        string Platform { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Minimum version. Will have non-empty value if minVersion is specified in xml doc comments via <compatibleWith> tag.
        // Version is of format "{major}.{minor}.{build}.{revision}".
        string MinVersion { [return: MarshalAs(UnmanagedType.BStr)] get; }
    }

    // Represents a set of compatibleWith attributes
    [Guid(GuidStrings.IAuthorCompatibleWithSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorCompatibleWithSet
    {
        // The number of compatibleWith attributes
        int Count { get; }

        // Retrieves count number of IAuthorCompatibleWithInfo's starting at startIndex.
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] IAuthorCompatibleWithInfo[] parameters);
    }

    // Represents a function signature
    [Guid(GuidStrings.IAuthorSignatureGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorSignature
    {
        // Function description, can be empty. The value comes from xml doc comments <summary> tag.
        // May contain free text as well as xml tags such as <para>. May contain a reference to an external vsdoc file
        // containing the value, in this case the value will have the following format: [<file name>:<member specifier>]
        string Description { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // LocId associated with the function, can be empty. The value comes from xml doc comments field "locid" on the 
        // signature or summary tags.
        string Locid { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The name of the file in which to find the external id. 
        string ExternalFile { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The member id used to locate the relevant xml associated with this signature, can be empty. Unlike locid on the 
        // elements themselves, this id indicates that all tags from the member with this id should be loaded and where 
        // applicable description information should be merged into the tags specified within the signature.
        string Externalid { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Specifies the keyword to use when invoking F1 help in the editor, can be empty.
        string HelpKeyword { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Parameters collection. The information comes from <param> tags under a <signature> tag if available,
        // and from JS function defintion otherwise.
        IAuthorParameterSet GetParameters();

        // Function return type. Will have a non-empty value if return type is specified in xml doc comments via <returns> tag.
        IAuthorReturnValue GetReturnValue();

        // Deprecated information. Will have a non-empty value if deprecated is specified in xml doc comments via <deprecated> tag.
        IAuthorDeprecatedInfo GetDeprecated();

        // Platform compatibility information. Will have a non-empty value if platform compatibility is specified in xml doc comments via <compatibleWith> tag.
        IAuthorCompatibleWithSet GetCompatibleWith();
    }

    // Represents a set of function signatures
    [Guid(GuidStrings.IAuthorSignatureSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorSignatureSet
    {
        // The number of function signatures. 
        int Count { get; }

        // Retrieves count signatures starting at startIndex.
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] IAuthorSignature[] signatures);
    }

    // Represents a function help information
    [Guid(GuidStrings.IAuthorFunctionHelpGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorFunctionHelp
    {
        // Function name, cannot be empty. The value comes from JS function definition.
        string FunctionName { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The signature information comes from the target function's JS definition or xml doc comments if available.
        // If one or more signature entries are specified in xml doc comments, function definition is ignored. 
        // Otherwise, if no signatures are defined in xml doc comments, there will be one entry reflecting JS function definition. 
        IAuthorSignatureSet GetSignatures();

        // File handle of the source file from which the documentation originates.
        IAuthorFileHandle SourceFileHandle { get; }
    }

    public enum AuthorType
    {
        atUnknown = 0,
        atBoolean = 2,
        atNumber = 4,
        atString = 7,
        atSymbol = 11,
        atObject = 13,
        atFunction = 14,
        atArray = 15,
        atDate = 16,
        atRegEx = 17
    }

    public enum AuthorScope
    {
        ascopeUnknown = 0,
        ascopeGlobal = 1,
        ascopeClosure = 2,
        ascopeLocal = 3,
        ascopeParameter = 4,
        ascopeMember = 5
    }

    [Guid(GuidStrings.IAuthorSymbolHelpGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorSymbolHelp
    {
        // The symbol type. 
        AuthorType Type { get; }

        // The symbol scope. 
        AuthorScope Scope { get; }

        // When Type=atObject, and the object was created using constructor, will contain the constructor name.           
        string TypeName { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Provides the identifier text.  
        string Name { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Initialized when Type=atFunction and the target function declaration is available.
        IAuthorFunctionHelp GetFunctionHelp();

        // Symbol description, can be empty. The value is coming from <field> or <var> tag contents.
        string Description { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The member id used to locate the block of XML which is then processed to find any description information associated with 
        // fields, can be empty.
        string Locid { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The type of the element is Array, this specifies the type of the elements in the array, can be empty.
        string ElementType { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Specifies the keyword to use when invoking F1 help in the editor, can be empty.
        string HelpKeyword { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // File handle of the source file from which the documentation originates.
        IAuthorFileHandle SourceFileHandle { get; }

        // The name of the file in which to find the external id. 
        string ExternalFile { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // The member id used to locate the relevant xml associated with this signature, can be empty. Unlike locid on the 
        // elements themselves, this id indicates that all tags from the member with this id should be loaded and where 
        // applicable description information should be merged into the tags specified within the signature.
        string Externalid { [return: MarshalAs(UnmanagedType.BStr)] get; }

        // Deprecated information. Will have a non-empty value if deprecated is specified in xml doc comments via <deprecated> tag.
        IAuthorDeprecatedInfo GetDeprecated();

        // Platform compatibility information. Will have a non-empty value if platform compatibility is specified in xml doc comments via <compatibleWith> tag.
        IAuthorCompatibleWithSet GetCompatibleWith();
    }

    // Represents a set of regions in a file.
    [Guid(GuidStrings.IAuthorRegionSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorRegionSet
    {
        // Returns the total number of regions
        int Count { get; }

        // Retrieves count regions staring at start index
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] AuthorFileRegion[] regions);
    }

    // Represents a set of task comments in a file.
    [Guid(GuidStrings.IAuthorTaskCommentSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorTaskCommentSet
    {
        // Returns the total number of task comments
        int Count { get; }

        // Retrieves count task comments staring at start index
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] AuthorFileTaskComment[] taskComments);
    }

    [Guid(GuidStrings.IAuthorCompletionSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorCompletionSet
    {
        // Returns the total number of completions
        int Count { get; }

        // Retrieves count completions starting at startIndex.
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] AuthorCompletion[] completions);

        // Retrieves the replace extent of the the completion. If a completion is selected this range should be replaced by the 
        // InsertText of the completion result.
        AuthorFileRegion Extent { get; }

        // Retrieve the hint for the given completion in the list.        
        IAuthorSymbolHelp GetHintFor(int index);

        // Retrieve the set kind
        AuthorCompletionSetKind Kind { get; }

        // Retrieve what kind of object, if any, the completion set is being produced for.
        AuthorCompletionObjectKind ObjectKind { get; }

        // Retrieve the diagnostic flags from the completion session.
        AuthorCompletionDiagnosticFlags DiagnosticFlags { get; }
    }

    // Indicates the kind of the parse node.
    public enum AuthorParseNodeKind
    {
        apnkEmptyNode,
        apnkNone,
        apnkName,
        apnkInt,
        apnkFlt,
        apnkStr,
        apnkRegExp,
        apnkThis,
        apnkNull,
        apnkFalse,
        apnkTrue,
        apnkEmpty,
        apnkLdFncSlot,
        apnkArgRef,
        apnkHelperCall3,
        apnkNot,
        apnkNeg,
        apnkPos,
        apnkLogNot,
        apnkSpread,
        apnkIncPost,
        apnkDecPost,
        apnkIncPre,
        apnkDecPre,
        apnkTypeof,
        apnkVoid,
        apnkDelete,
        apnkArray,
        apnkObject,
        apnkTempRef,
        apnkComputedName,
        apnkYieldLeaf,
        apnkYield,
        apnkYieldStar,
        apnkStFncSlot,
        apnkAdd,
        apnkSub,
        apnkMul,
        apnkDiv,
        apnkMod,
        apnkOr,
        apnkXor,
        apnkAnd,
        apnkEq,
        apnkNe,
        apnkLt,
        apnkLe,
        apnkGe,
        apnkGt,
        apnkCall,
        apnkDot,
        apnkAsg,
        apnkInstOf,
        apnkIn,
        apnkEqv,
        apnkNEqv,
        apnkComma,
        apnkLogOr,
        apnkLogAnd,
        apnkLsh,
        apnkRsh,
        apnkRs2,
        apnkNew,
        apnkIndex,
        apnkQmark,
        apnkAsgAdd,
        apnkAsgSub,
        apnkAsgMul,
        apnkAsgDiv,
        apnkAsgMod,
        apnkAsgAnd,
        apnkAsgXor,
        apnkAsgOr,
        apnkAsgLsh,
        apnkAsgRsh,
        apnkAsgRs2,
        apnkScope,
        apnkMember,
        apnkSetMember,
        apnkGetMember,
        apnkList,
        apnkVarDecl,
        apnkTemp,
        apnkFncDecl,
        apnkClassDecl,
        apnkProg,
        apnkEndCode,
        apnkDebugger,
        apnkFor,
        apnkIf,
        apnkWhile,
        apnkDoWhile,
        apnkForIn,
        apnkForOf,
        apnkBlock,
        apnkStrTemplate,
        apnkWith,
        apnkBreak,
        apnkContinue,
        apnkLabel,
        apnkSwitch,
        apnkCase,
        apnkTryCatch,
        apnkCatch,
        apnkReturn,
        apnkTry,
        apnkThrow,
        apnkFinally,
        apnkTryFinally,
        apnkStruct,
        apnkEnum,
        apnkTyped,
        apnkVarDeclList,
        apnkDefaultCase,
        apnkConstDecl,
        apnkLetDecl,
        apnkConstDeclList,
        apnkLetDeclList,
        apnkSuper
    }

    // Indicates the relationship to between a parent and a child nodes
    public enum AuthorParseNodeEdge
    {
        // Node has no relationship to it parent
        // Applicable for: root node
        apneNone,

        // Node is an operand
        // Applicable for: 
        //                  apnkNot,
        //                  apnkNeg,
        //                  apnkPos,
        //                  apnkLogNot,
        //                  apnkSpread,
        //                  apnkIncPre,
        //                  apnkDecPre,
        //                  apnkTypeof,
        //                  apnkVoid,
        //                  apnkDelete,
        //                  apnkIncPost,
        //                  apnkDecPost,
        //                  apnkYieldLeaf,
        //                  apnkYield,
        //                  apnkYieldStar,
        apneOperand,

        // Node is a left child of a binary expression
        // Applicable for:
        //                  apnkAdd
        //                  apnkSub
        //                  apnkMul
        //                  apnkDiv
        //                  apnkMod
        //                  apnkOr
        //                  apnkXor
        //                  apnkAnd
        //                  apnkEq
        //                  apnkNe
        //                  apnkLt
        //                  apnkLe
        //                  apnkGe
        //                  apnkGt
        //                  apnkDot
        //                  apnkAsg
        //                  apnkInstOf
        //                  apnkIn
        //                  apnkEqv
        //                  apnkNEqv
        //                  apnkComma
        //                  apnkLogOr
        //                  apnkLogAnd
        //                  apnkLsh
        //                  apnkRsh
        //                  apnkRs2
        //                  apnkAsgAdd
        //                  apnkAsgSub
        //                  apnkAsgMul
        //                  apnkAsgDiv
        //                  apnkAsgMod
        //                  apnkAsgAnd
        //                  apnkAsgXor
        //                  apnkAsgOr
        //                  apnkAsgLsh
        //                  apnkAsgRsh
        //                  apnkAsgRs2
        //                  apnkScope
        apneLeft,

        // Node is a right child of a binary expression
        // Applicable for:
        //                  apnkAdd
        //                  apnkSub
        //                  apnkMul
        //                  apnkDiv
        //                  apnkMod
        //                  apnkOr
        //                  apnkXor
        //                  apnkAnd
        //                  apnkEq
        //                  apnkNe
        //                  apnkLt
        //                  apnkLe
        //                  apnkGe
        //                  apnkGt
        //                  apnkDot
        //                  apnkAsg
        //                  apnkInstOf
        //                  apnkIn
        //                  apnkEqv
        //                  apnkNEqv
        //                  apnkComma
        //                  apnkLogOr
        //                  apnkLogAnd
        //                  apnkLsh
        //                  apnkRsh
        //                  apnkRs2
        //                  apnkAsgAdd
        //                  apnkAsgSub
        //                  apnkAsgMul
        //                  apnkAsgDiv
        //                  apnkAsgMod
        //                  apnkAsgAnd
        //                  apnkAsgXor
        //                  apnkAsgOr
        //                  apnkAsgLsh
        //                  apnkAsgRsh
        //                  apnkAsgRs2
        //                  apnkScope
        apneRight,

        // Node is a condition in a statement
        // Applicable for:
        //                  apnkQmark
        //                  apnkIf
        //                  apnkFor
        //                  apnkWhile
        //                  apnkDoWhile
        apneCondition,

        // Node is the statements on the true branch of and branch statement
        // Applicable for:
        //                  apnkQmark
        //                  apnkIf
        apneThen,

        // Node is the statements on the false branch of and branch statement
        // Applicable for:
        //                  apnkQmark
        //                  apnkIf
        apneElse,

        // Node is the initialization in a var declaration or of a for statement
        // Applicable for:
        //                  apnkFor
        //                  apnkVarDecl
        apneInitialization,

        // Node is the increment in a for statement
        // Applicable for:
        //                  apnkFor
        apneIncrement,

        // Node is the statement of a body of a container statement
        // Applicable for:
        //                  apnkProg
        //                  apnkFncDecl
        //                  apnkFor
        //                  apnkForIn
        //                  apnkWhile
        //                  apnkDoWhile
        //                  apnkWith
        //                  apnkCase
        //                  apnkTry
        //                  apnkCatch
        //                  apnkFinally
        apneBody,

        // Node is the statement of a body of a block
        // Applicable for:
        //                  apnkBlock
        apneBlockBody,

        // Node is a value of in a statement 
        // Applicable for:
        //                  apnkReturn
        //                  apnkSwitch
        //                  apnkCase
        //                  apnkThrow
        //                  apnkGetMember
        //                  apnkSetMember
        //                  apnkTyped
        //                  apnkIndex
        apneValue,

        // Node is a target of a call statement 
        // Applicable for:
        //                  apnkCall
        //                  apnkNew
        //                  apnkMember
        apneTarget,

        // Node is an argument in a function declaration node (formal)
        // Applicable for:
        //                  apnkFncDecl
        apneArgument,

        // Node is a list of function arguments (actual)
        // Applicable for:
        //                  apnkCall
        //                  apnkNew
        apneArguments,

        // Node is a list of member of an object or an enum
        // Applicable for:
        //                  apnkEnum
        //                  apnkStruct
        //                  apnkObject
        //                  apnkClassDecl
        apneMembers,

        // Node is a variable in a catch or a forin statement 
        // Applicable for:
        //                  apnkCatch
        //                  apnkForIn
        apneVariable,

        // Node is an object in a forin or with statement 
        // Applicable for:
        //                  apnkWith
        //                  apnkForIn
        apneObject,

        // Node is a try statement  in a try-finally or a try-catch node
        // Applicable for:
        //                  apnkTryFinally
        //                  apnkTryCatch
        apneTry,

        // Node is a catch statement in a try-finally or a try-catch node
        // Applicable for:
        //                  apnkTryCatch
        apneCatch,

        // Node is a finally statement in a try-finally or a try-catch node
        // Applicable for:
        //                  apnkTryFinally
        apneFinally,

        // Node is a case statement in a switch node
        // Applicable for:
        //                  apnkSwitch
        apneCase,

        // Node is the default case statement in a switch node
        // Applicable for:
        //                  apnkSwitch
        apneDefaultCase,

        // Node is a member of an array
        // Applicable for:
        //                  apnkArray
        apneElements,

        // Node is a member in a list of statements
        // Applicable for:
        //                  apnkList
        apneListItem,

        // Node is a member in an object
        // Applicable for:
        //                  apnkMember
        apneMember,

        // Node is the type in a typed expression
        // Applicable for:
        //                  apnkTyped
        apneType,

        // Node is the extends expression
        // Applicable for:
        //                  apnkClassDecl
        apneExtends,

        // Node is the class constructor
        // Applicable for:
        //                  apnkClassDecl
        apneCtor,

        // Node is a list of static members for a class
        // Applicable for:
        //                  apnkClassDecl
        apneStaticMembers,

        // Node is a list of strings for a string template
        // Applicable for:
        //                  apnkStrTemplate
        apneStringLiterals,

        // Node is a list of substitution expression for a string template
        // Applicable for:
        //                  apnkStrTemplate

        apneSubstitutionExpression,

        // Node is a list of string raw literals for a string template
        // Applicable for:
        //                  apnkStrTemplate
        apneStringRawLiterals,
    }

    // Used by calls to GetNodeProperty to specify which node property to retrieve
    public enum AuthorParseNodeProperty
    {
        // Starting offset of the left curly braces in a statement
        apnpLCurlyMin,

        // Starting offset of the right curly braces in a statement
        apnpRCurlyMin,

        // Starting offset of the left parenthesis in a statement
        apnpLParenMin,

        // Starting offset of the right parenthesis in a statement
        apnpRParenMin,

        // Starting offset of the left square bracket in an index statement
        apnpLBrackMin,

        // Starting offset of the right square bracket in an index statement
        apnpRBrackMin,

        // Starting offset of the identifier name of a function, parameter, or variable declaration.
        apnpIdentifierMin,

        // Starting offset of the function keyword
        apnpFunctionKeywordMin
    }

    // Used to report when semicolons were automatically inserted to complete the node or when block was 
    // optional in the syntax but was included explicitly
    [Flags]
    public enum AuthorParseNodeFlags
    {
        // No flags set
        apnfNone = 0x0000,

        // Statement terminated by an explicit semicolon
        apnfExplicitSimicolon = 0x0010,

        // Statement terminated by an automatic semicolon
        apnfAutomaticSimicolon = 0x0020,

        // Statement missing terminating semicolon, and is not applicable for automatic semicolon insersion
        apnfMissingSimicolon = 0x0040,

        // Node is added by the parser or does it represent actual user code
        apnfSyntheticNode = 0x0100,

        // Function is subsumed in a call and was hoisted out
        apnfSubsumedFunction = 0x0200
    }

    // Indicates what options are set on a regular expression object.
    [Flags]
    public enum AuthorRegExpOptions
    {
        // No options specified
        areoNone = 0x0000,

        // Global is specified ('g')
        areoGlobal = 0x0001,

        // IgnoreCase is specified ('i')
        areoIgnoreCase = 0x0002,

        // Multiline is specified ('m')
        areoMultiline = 0x0004
    }

    // Represents a parse node in the AST
    public struct AuthorParseNodeDetails
    {
        // The kind of parse node this is
        public AuthorParseNodeKind Kind;

        // The starting offset of this node
        public int StartOffset;

        // The ending offset of this node
        public int EndOffset;

        // Flags associated with the current node
        public AuthorParseNodeFlags Flags;
    }

    // Represents a parse node object in a serialized list
    public struct AuthorParseNode
    {
        // The parse node details
        public AuthorParseNodeDetails Details;

        // The name associated with this node if applicable 
        // (e.g. function, variable or jump target for jump nodes)
        // In order to retrieve the actual string value, use IAuthorParseNodeCursor.GetPropertyById(<name>)
        // on the cursor object that created this node object.
        public int Name;

        // The label associated with this node if exists
        // In order to retrieve the actual string value, use IAuthorParseNodeCursor.GetPropertyById(<label>)
        // on the cursor object that created this node object.
        public int Label;

        // The edge label defining the relationship to the parent node. 
        // The root node will have a apneNone value. 
        public AuthorParseNodeEdge EdgeLabel;

        // The level of the current node in the tree relative to the root node
        public int Level;
    }

    // Represents a serialized sub tree
    [Guid(GuidStrings.IAuthorParseNodeSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorParseNodeSet
    {
        // Returns the total number of nodes
        int Count { get; }

        // Retrieves count nodes staring at start index
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] AuthorParseNode[] nodes);
    }

    // Represents an Cursor object accessing the AST
    [Guid(GuidStrings.IAuthorParseNodeCursorGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorParseNodeCursor
    {
        // Get the node the cursor is pointing at. If the current node does not exist an empty node (kind: apnkEmptyNode) 
        // is returned. Initial value is the root of the tree.
        AuthorParseNodeDetails Current();

        // Get the parent of the current node the cursor is pointing at. If the parent node does not exist an empty node (kind: apnkEmptyNode) 
        // is returned.
        AuthorParseNodeDetails Parent();

        // Get the one of the children of the current node without moving the cursor. The child is selected based on the label 
        // defined by edgeLabel. If the current node does not support the specific edge label the call will fail with E_INVALIDARG.
        // Use index to select which child to use in case of list nodes. The value of index is ignored for all other node
        // types.
        // If the child specified by edgeLabel does not exist (e.g. else for an if statement), an empty node (kind: apnkEmptyNode)
        // will be returned. 
        // In order to invoke any of the cursor methods on the child node, the cursor needs to be moved to the child node first.
        // Use this method to investigate the basic properties of a child node without moving the cursor.
        AuthorParseNodeDetails Child(AuthorParseNodeEdge edgeLabel, int index);

        // Move the cursor to one of the children of the current node. The child is selected based on the label defined 
        // by edgeLabel. If the current node does not support the specific edge label the call will fail with E_INVALIDARG.
        // Use index to select which child to use in case of list nodes. The value of index is ignored for all other node
        // types.
        // If the child specified by edgeLabel does not exist (e.g. else for an if statement), the cursor will be moved to 
        // point to an empty node (kind: apnkEmptyNode); use MoveUp to revert the cursor to the parent node.
        AuthorParseNodeDetails MoveToChild(AuthorParseNodeEdge edgeLabel, int index);

        // Move the cursor to the parent of the current node. If the current node is the root, NULL is returned
        AuthorParseNodeDetails MoveUp();

        // Move the cursor to the inner most node with range containing offset. Set excludeEndOffset to true to exclude nodes
        // that end at offset from the search.
        AuthorParseNodeDetails SeekToOffset(int offset, [MarshalAs(UnmanagedType.VariantBool)] bool excludeEndOffset = false);

        // Moves the cursor to the closest node that encloses the range defined by startOffset and endOffset. startOffset 
        // is expected to be less than or equal to endOffset; if not the call will fail. 
        // Set excludeEndOffset to true to exclude nodes that end at either startOffset and endOffset from the search.
        AuthorParseNodeDetails MoveToEnclosingNode(int startOffset, int endOffset, [MarshalAs(UnmanagedType.VariantBool)] bool excludeEndOffset = false);

        // Serialize the sub tree rooted at the current node into a list.
        // The list will always begin with the current node. Each node will be followed by its children. Use Level field 
        // in IAuthorParesNode struct to identify children of a node. Each node pears an edge label that defines its 
        // relationship to the parent. Only edges with nonempty nodes will be serialized (e.g. if without an else will only 
        // have two child nodes: condition and then). 
        // Use depth to control what portion of the tree will be serialized; depth = 0 will return the current node only, 
        // depth = 1 will return the current node and its children and so on. If depth < 0 all the nodes in the tree will be 
        // serialized.
        IAuthorParseNodeSet GetSubTree(int depth);

        // Get the value of a property on the current node. If the property value is not supported on the current node, 0 is 
        // returned.
        int GetNodeProperty(AuthorParseNodeProperty nodeProperty);

        // Get property string using an id. GetSubTree call will serialize names and string literals as id's. Use this method
        // to retrieve the actual text. Call will fail if the id is invalid.
        [return: MarshalAs(UnmanagedType.BStr)]
        string GetPropertyById(int propertyId);

        // Get the string value associated with the current node. This call is applicable to string literal nodes and name
        // nodes; calling this method on other node types will fail.
        [return: MarshalAs(UnmanagedType.BStr)]
        string GetStringValue();

        // Get the value of the current integer literal. If the current node is not a integer literal the call will fail.
        int GetIntValue();

        // Get the value of the current float literal. If the current node is not a float literal the call will fail.
        double GetFloatValue();

        // Get the value of the current regular expression node. If the current node is not a RegExp the call will fail.
        [return: MarshalAs(UnmanagedType.BStr)]
        void GetRegExpValue(out string result, out AuthorRegExpOptions options);

        // Get the label text associated with the current statement. If the current statement does not have a label or 
        // current node is not a statement null is returned.
        [return: MarshalAs(UnmanagedType.BStr)]
        string GetStatementLabel();

        // Gets the string value of labels of jump statements. Statements that support this call are Break and Continue.
        // If the current jump node does not have a target label null is returned.
        [return: MarshalAs(UnmanagedType.BStr)]
        string GetTargetLabel();

        // Get the span of the current node. The statement span is the start and end offsets of a statement as the debugger would
        // show them (e.g. statement span of if statement is the if keyword and the condition, similarlly that of a do while loop is
        // the while keyword and condition.
        void GetStatementSpan(out int startOffset, out int endOffset);

        // Checks if the offset lies within a comment regardless of the cursor location.
        [return: MarshalAs(UnmanagedType.VariantBool)]
        bool OffsetInComment(int offset);

        // Gets the edge label of the current node with respect to its parent.
        AuthorParseNodeEdge GetEdgeLabel();
    }

    public enum AuthorStructureNodeKind : uint
    {
        // A node not covered by the pre-defined types below. The actual kind is found in the customKind field of AuthorStructureNode
        asnkCustom,

        // The node represents the global scope.
        asnkGlobal,

        // The node represents an object literal. An object literal will typically only appear in the tree if it contains a function that must
        // be in the tree.
        asnkObjectLiteral,

        // The node represents a function not otherwise determined to be a class or module. If it is contained in a class it can be assumed to be a method.
        asnkFunction,

        // The node repesents a variable
        asnkVariable,

        // The node represents a field of a class
        asnkField,

        // The node represents a class. This is typically a function.
        asnkClass,

        // The node represents the definition of a namespace. This is typically a variable.
        asnkNamespace,

        // The node represents a module. This is typically an immediately invoked function expression.
        asnkModule,

        // A region annotation of the parent node
        asnkRegion,
    }

    public struct AuthorStructureNode
    {
        // A unique number in the IAuthorStructureSet used to identify the node and to identify the node container.
        // This value should be considered an opaque value to the caller.
        public int Key;

        // The key of the node that contains this node as one of its children.
        public int Container;

        // The kind of the node or asnkCustom if the customKind field should be used instead.
        public AuthorStructureNodeKind Kind;

        // This is true if a node, in the full set of nodes, has this node as a container.
        [MarshalAs(UnmanagedType.Bool)]
        public bool HasChildren;

        // The display name of the node when it is displayed as a container. This is the name as it appears, for example,
        // in the container drop-down list. If an node has a containerName it can be assumed to be a container.
        // A node mgiht be both a container and an item (such as a function which contains nested functions).
        [MarshalAs(UnmanagedType.BStr)]
        public string ContainerName;

        // The display name of the node when it is displayed as an item. This is the name as it appears, for example,
        // in the item drop-down list.
        [MarshalAs(UnmanagedType.BStr)]
        public string ItemName;

        // The kind if not one of the pre-defined kinds. This should be NULL and ignored if kind is not asnkCustom.
        [MarshalAs(UnmanagedType.BStr)]
        public string CustomKind;

        // The location of the node in the primary file.
        public AuthorFileRegion Region;

        // The glyph name if set by extensions code. May be null or empty.
        [MarshalAs(UnmanagedType.BStr)]
        public string Glyph;
    }

    [Guid(GuidStrings.IAuthorStructureNodeSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorStructureNodeSet
    {
        // Returns the total number of nodes
        int Count { get; }

        // Retrieves count completions starting at startIndex.
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] AuthorStructureNode[] nodes);
    }

    // Represents a tree of nodes describing the structure of a JavaScript file. This structure is produced by a combination of the AST
    // and interpretation of globally visible changes made by executing the JavaScript file. The set is accessed as an array of nodes
    // and the tree is formed by "container" edge, children point to their containers. Each call will produce an IAuthorStructureNodeSet.
    [Guid(GuidStrings.IAuthorStructureGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorStructure
    {
        // Retrieve the full set of nodes that make up the structure of the document
        IAuthorStructureNodeSet GetAllNodes();

        // Retrieve the set of nodes for which isContainer is true.
        IAuthorStructureNodeSet GetContainerNodes();

        // Retrieve the set of nodes for which have the key parameter value as the their container value.
        // The key parameter must be a value returned as a set from same instance of IAuthorStructure.
        IAuthorStructureNodeSet GetChildrenOf(int key);
    }

    // Memory allocation information
    [Guid(GuidStrings.IAuthorAllocInfoGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorAllocInfo
    {
        // A tag string for the allocated memory. For example, ArenaAllocator name.
        string Tag { get; }

        // The number of bytes allocated.
        int Size { get; }

        // The outstanding allocations count.
        int Count { get; }

        // The category name (Arena, Heap, Object etc).
        string Category { get; }
    }

    // Represents a set of AuthorAllocInfo objects
    [Guid(GuidStrings.IAuthorAllocInfoSetGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorAllocInfoSet
    {
        // Return the total number of AuthorAllocInfo objects
        int Count { get; }

        // Returns AuthorAllocInfo objects
        void GetItems(int startIndex, int count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1), Out] IAuthorAllocInfo[] authorAllocInfos);
    }

    // Debug-only diagnostics interface. Allows to get engine diagnostics information.
    [Guid(GuidStrings.IAuthorDiagnosticsGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorDiagnostics
    {
        // Returns memory allocation statistics.
        IAuthorAllocInfoSet AllocStats { get; }

        // Forces recycler to collect
        void ForceGC();
    }

    // A callback object that allows the host to take action at the point of completion failure.
    [Guid(GuidStrings.IAuthorCompletionDiagnosticCallbackGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorCompletionDiagnosticCallback
    {
        // Called just before a completion session fails (when execution is about to stop without a value)
        void Invoke();
    }

    // Per-file authoring service for a JavaScript file.
    [Guid(GuidStrings.IAuthorFileAuthoringGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorFileAuthoring
    {
        // Update the state of the authoring service. This has the side-effect of causing the IAuthorFile's 
        // StatusChanged() of files in the context to be invoked with the status of the file. StatusChanged() 
        // might also be called by other methods but this method will force it to be called.
        void Update();

        // Get the regions that can be used for collapsible regions. This primarily contains the locations of all function
        // code blocks.
        IAuthorRegionSet GetRegions();

        // Get the comments that starts with specified prefixes such as TODO. These comments are called task comments
        IAuthorTaskCommentSet GetTaskComments([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] AuthorTaskCommentPrefix[] prefixes, int count);

        // Get the completion list for the given offset in the file for which this is an authoring service.
        IAuthorCompletionSet GetCompletions(int offset, AuthorCompletionFlags filter);

        // Get quick info for the symbol at the given offset. The extent returned is the range of source for which this hint
        // applies.
        IAuthorSymbolHelp GetQuickInfo(int offset, out AuthorFileRegion extent);

        // Retrieve an abstract syntax tree (AST) of the primary file as a JSON string.
        [return: MarshalAs(UnmanagedType.BStr)]
        string GetASTAsJSON();

        // Gets function help information for the given offset in the file. currentParameterIndex will be set to
        // to be zero based index of the paramter of the call at offset. extent is set to the region containing the 
        // parameters of the call. diagStatus helps diagnose why funcHelpInfo is NULL. funcHelpInfo is help information 
        // for the call at offset or NULL if there is no call at offset or the call information could not be determined.
        IAuthorFunctionHelp GetFunctionHelp(int offset, AuthorFunctionHelpFlags flags, out int currentParameterIndex, out AuthorFileRegion extent, out AuthorDiagStatus diagStatus);

        // Must be called whenever the information returned by IAuthorFileContext is modified. For example if 
        // files were added or removed from the context causing GetContextFiles() to return a different value.
        void ContextChanged();

        // Tell the authoring service it is taking too long for the current operation. The authoring service uses this an
        // indicator that it should stop it current algorithm to collect information from the authoring file and try
        // a faster, less accurate alternative. Abort() and Hurry() must be called from a separate thread from all other 
        // methods on this interface. The phaseId parameter must be from a IAuthorFileContext.FileAuthoringPhaseChanged()
        // call. Hurry() is ignored if the file authoring is not in the phase with the given phaseId.
        void Hurry(int phaseId);

        // Tell the authoring service to abort the currently active call. This should be called when the results of
        // the current active call is no longer needed (e.g. the file as been modified in a way that clearly 
        // renders the information the current active all would provide invalid) or is taking an inordant amount 
        // of time and calls to Hurry() have not caused to operation to complete. This might, but is not guarenteed 
        // to, cause the active call to return E_ABORT. Abort() must be called from a separate thread from all other 
        // methods on this interface.
        void Abort();

        // Retrieve an abstract syntax tree (AST) of the primary file.
        IAuthorParseNodeCursor GetASTCursor();

        // Request the location of where a symbol is defined. The fileHandle is the handled of the file retrieved by
        // IAuthorFileContext::GetContextFiles() or IAuthorFileContext::GetPrimaryFile() and is a value that was 
        // previously returned by IAuthorServices::RegisterFile. The location is the offset of where the identifier is 
        // defined. If the location of the symbol could not be found the fileHandle is set to NULL and location is
        // set to 0.
        void GetDefinitionLocation(int offset, out IAuthorFileHandle fileHandle, out int location);

        // Returns a set of references in the file to the identifier at offset. If there is no identifier at offset 
        // then an empty reference set is returned with a null identifier.
        IAuthorReferenceSet GetReferences(int offset);

        // Retrieve structure information about the primary file.
        IAuthorStructure GetStructure();

        // Set a callback for completions failures.
        void SetCompletionDiagnosticCallback(IAuthorCompletionDiagnosticCallback acdCallback);

        // Returns diagnostics interface (available in CHK only)
        IAuthorDiagnostics GetDiagnostics();
    }

    // An optional host object.
    [Guid(GuidStrings.IAuthorServiceHostGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorServiceHost
    {
        // Requests the host to call the IAuthorServices::Work() method on the same thread that constructed
        // the IAuthorServices object. This method is never called on the IAuthorServices thread and the 
        // host object needs to be prepared to receive calls to RequestWork() on any thread.    
        void RequestWork();
    }

    // Represents the a factory for producing colorization and authoring services for a set of related JavaScript files.
    // Unless otherise inidcated, all method of this interface and returned interfaces must be called on the same
    // thread that this service was created on. The service has thread affinity with thread is was created on.
    [Guid(GuidStrings.IAuthorServicesGuid)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAuthorServices
    {
        // Retrieve a text colorizing service. The colorizer is stateless and can be used from any thread but only by one
        // thread at a time (rental-threading).
        IAuthorColorizeText GetColorizer();

        // Register a file with the authoring services to obtain a handle for use in authoring. This handle can be
        // used any number of times in a context to create IAuthorFileAuthoring. Each registered file is considered
        // a different file even if the name and content is identical to some other file registered. 
        IAuthorFileHandle RegisterFile(IAuthorFile file);

        // For a give file context retrieve an authoring service. The context specifies information about the file
        // for which subsequent requests will be made.
        IAuthorFileAuthoring GetFileAuthoring(IAuthorFileContext context);

        // Invoke the engine's public cleanup routine. This forces GC to kick in and collect any unused objects resulting
        // from previous executions. Setting exhaustive to true will force a more complete gargage collection pass that 
        // ensures all memory references, weak reference handles in particular, to be collected. An exhaustive clean-up
        // is expensive and should be used rarely and only when it is clear the authoring services will not be used for an 
        // extended period of time. Calling exhaustive, however, helps track down memory leaks as it ensures all non-live
        // memory has been released.
        // memory has been released. By default, Cleanup() is performed in thread. Non-exhaustive cleanup can be performed
        // concurrently if a host object is supplied that allows the concurrent cleanup to request time on the
        // IAuthorServices thread.
        void Cleanup([MarshalAs(UnmanagedType.VariantBool)] bool exhaustive);

        // Set the optional services host object. This object is used by authoring services to request various services
        // from the host. If no host object is supplied the services are assumed to be unavailable.
        void SetHost(IAuthorServiceHost host);

        // This method must be called, in response to RequestWork() on the host object, on the same thread that created 
        // the IAuthorService. This is used by the authoring services to perform incremental and concurrent work such
        // as concurrent garbage collection. 
        void Work();
    }
}

#pragma warning restore 649
