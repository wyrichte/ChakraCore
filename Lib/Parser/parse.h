//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#include <crtdefs.h>

// Operator precedence levels
enum
{
    koplNo,     // not an operator
    koplCma,    // ,
    koplSpr,    // ...
    koplAsg,    // = += etc
    koplQue,    // ?:
    koplLor,    // ||
    koplLan,    // &&
    koplBor,    // |
    koplXor,    // ^
    koplBan,    // &
    koplEqu,    // == !=
    koplCmp,    // < <= > >=
    koplShf,    // << >> >>>
    koplAdd,    // + -
    koplMul,    // * / %
    koplUni,    // unary operators
    koplLim
};
enum ParseType
{
    ParseType_Upfront,
    ParseType_Deferred,
    ParseType_Reparse
};

enum ScopeType;
enum SymbolType : byte;

#if ERROR_RECOVERY
// Error recovery sets
enum ErrorRecoverySet
{
    ersNone      = 0u,
    ersComma     = 1u <<  0, // tkComma
    ersSColon    = 1u <<  1, // tkSColon
    ersAsg       = 1u <<  2, // tkAsg
    ersBinOp     = 1u <<  3, // tkLsh, tkRsh, tkRs2, tkLe, tkGe, tkINSTANCEOF, tkEQ, tkNE, tkEqv, tkNEqv, tkLogAnd, tkLogOr, tkAsgMul, tkAsgDiv
                             // tkAsgMod, tkAsgAdd, tkAsgSub, tkAsgLsh, tkAsgRsh, tkAsgRs2, tkAsgAnd, tkAsgXor, tkAsgOr, tkQMark, tkStar, tkDiv,
                             // tkPct, tkGT, tkLT, tkAnd, tkXor, tkOr
    ersRBrack    = 1u <<  4, // tkRBrack
    ersRCurly    = 1u <<  5, // tkRCurly
    ersRParen    = 1u <<  6, // tkRParen
    ersDot       = 1u <<  7, // tkDot
    ersColon     = 1u <<  8, // tkColon
    ersGetSet    = 1u <<  9, // get/set
    ersAddOp     = 1u << 10, // tkAdd, tkSub
    ersLCurly    = 1u << 11, // tkLCurly
    ersLParen    = 1u << 12, // tkLParen
    ersLBrack    = 1u << 13, // tkLBrack
    ersScope     = 1u << 14, // tkScope
    ersIn        = 1u << 15, // tkIN
    ersSCase     = 1u << 16, // tkCASE, tkDEFAULT
    ersElse      = 1u << 17, // tkELSE
    ersCatch     = 1u << 18, // tkCATCH, tkFINALLY
    ersVar       = 1u << 19, // tkVAR
    ersStmt      = 1u << 20, // tkBREAK, tkRETURN, tkTHROW, tkDEBUGGER, tkFOR, tkSWITCH, tkDO, tkIF, tkTRY, tkWITH
    ersWhile     = 1u << 21, // tkWHILE
    ersID        = 1u << 22, // tkID
    ersFunc      = 1u << 23, // tkFUNCTION
    ersEOF       = 1u << 24, // tkEOF
    ersDArrow    = 1u << 25, // tkDArrow
    ersExpr      = 1u << 26, // tkTilde, tkBang, tkInc, tkDec, tkEllipsis, tkRegExp, tkVOID, tkDELETE, tkTYPEOF, tkIntCon, tkFltCon, tkStrCon
                             // tkTHIS, tkTRUE, tkFALSE, tkNULL, tkCLASS


    ersExprStart = ersSColon | ersAddOp | ersLCurly | ersLParen | ersLBrack | ersID |  ersFunc | ersExpr,
    ersStmtStart = ersExprStart | ersVar | ersStmt | ersWhile,
    ersPostfix   = ersDot | ersLParen | ersLBrack,
};

#define ERROR_RECOVERY_FORMAL ErrorRecoverySet ers
#define ERROR_RECOVERY_FORMAL_ ErrorRecoverySet ers,
#define _ERROR_RECOVERY_FORMAL , ErrorRecoverySet ers
#define ERROR_RECOVERY_ACTUAL(value) ErrorRecoverySet(value)
#define ERROR_RECOVERY_ACTUAL_(value) ErrorRecoverySet(value),
#define _ERROR_RECOVERY_ACTUAL(value) , ErrorRecoverySet(value)
#define SKIP(set) Skip(set)
#else
#define ERROR_RECOVERY_FORMAL
#define ERROR_RECOVERY_FORMAL_
#define _ERROR_RECOVERY_FORMAL
#define ERROR_RECOVERY_ACTUAL(value)
#define ERROR_RECOVERY_ACTUAL_(value)
#define _ERROR_RECOVERY_ACTUAL(value)
#define SKIP(set)
#endif

#if PARSENODE_EXTENSIONS
#define _PARSENODE_EXTENSIONS_ACTUAL(value) , value
#else
#define _PARSENODE_EXTENSIONS_ACTUAL(value)
#endif

// Representation of a label used when no AST is being built.
struct LabelId
{
    IdentPtr pid;
    struct LabelId* next;
};

typedef ArenaAllocator ParseNodeAllocator;

/***************************************************************************
Parser object.
***************************************************************************/
class CompileScriptException;
class Parser;
struct LanguageServiceExtension;
class SourceContextInfo;
struct BlockIdsStack;
class Span;
class BackgroundParser;
struct BackgroundParseItem;

class AuthoringCallbacks;

class AuthoringData
{
private:
    AuthoringCallbacks *callbacks;
public:
    AuthoringData(AuthoringCallbacks *callbacks): callbacks(callbacks) { }
    AuthoringCallbacks *Callbacks() { return callbacks; }
};

#if LANGUAGE_SERVICE_ONLY
    #define RUNTIME_PARSE_CALLBACK_TYPE 2
    typedef void (*RuntimeParseCallback)(void *context, Parser *parse, LPCUTF8 pszSrc, size_t offset, size_t length, ParseNodePtr pnodeProg);
    struct RuntimeParseCallbackData: public AuthoringData
    {
        int dataType;
        /* Authoring::RuntimeParsingContext* */ void *context;
        RuntimeParseCallback callback;
        RuntimeParseCallbackData(AuthoringCallbacks *callbacks, void *context, RuntimeParseCallback callback): AuthoringData(callbacks), dataType(RUNTIME_PARSE_CALLBACK_TYPE), context(context), callback(callback) { }
    };
#endif

typedef void (*ParseErrorCallback)(void *data, charcount_t position, charcount_t length, HRESULT hr);

struct CatchPidRef
{
    IdentPtr pid;
    PidRefStack *ref;
};

typedef SListBase<CatchPidRef> CatchPidRefList;

struct DeferredFunctionStub
{
    RestorePoint restorePoint;
    uint fncFlags;
    uint nestedCount;
    DeferredFunctionStub *deferredStubs;
#if DEBUG
    charcount_t ichMin;
#endif
};

struct StmtNest
{
    union
    {
        struct
        {
            ParseNodePtr pnodeStmt; // This statement node.
            ParseNodePtr pnodeLab;  // Labels for this statement.
        };
        struct
        {
            bool isDeferred:1;
            OpCode op;              // This statement operation.
            LabelId* pLabelId;      // Labels for this statement.
        };
    };
    StmtNest *pstmtOuter;           // Enclosing statement.
};

struct BlockInfoStack
{
    StmtNest pstmt;
    ParseNode *pnodeBlock;
    ParseNodePtr *m_ppnodeLex;              // lexical variable list tail
    BlockInfoStack *pBlockInfoOuter;        // containing block's BlockInfoStack
    BlockInfoStack *pBlockInfoFunction;     // nearest function's BlockInfoStack (if pnodeBlock is a function, this points to itself)
};

struct ParseContext
{
    LPCUTF8 pszSrc;
    size_t offset;
    size_t length;
    charcount_t characterOffset;
    int nextBlockId;
    ULONG grfscr;
    ULONG lineNumber;
    ParseNodePtr pnodeProg;
    SourceContextInfo* sourceContextInfo;
    BlockInfoStack* currentBlockInfo;
    bool strictMode;
    bool fromExternal;
};

class Parser
{
    typedef Scanner<NotNullTerminatedUTF8EncodingPolicy> Scanner_t;

private:

    template <OpCode nop> static int GetNodeSize();
#define PTNODE(nop,sn,pc,nk,ok,json,apnk) template <> static int GetNodeSize<nop>() { return kcbPn##nk; };
#include "ptlist.h"

    template <OpCode nop> static ParseNodePtr StaticAllocNode(ArenaAllocator * alloc)
    {
        ParseNodePtr pnode = (ParseNodePtr)alloc->Alloc(GetNodeSize<nop>());
        Assert(pnode != null);
        return pnode;
    }


public:
#if DEBUG
    Parser(Js::ScriptContext* scriptContext, BOOL strictMode = FALSE, PageAllocator *alloc = nullptr, bool isBackground = false, size_t size = sizeof(Parser));
#else
    Parser(Js::ScriptContext* scriptContext, BOOL strictMode = FALSE, PageAllocator *alloc = nullptr, bool isBackground = false);
#endif
    ~Parser(void);

    Js::ScriptContext* GetScriptContext() const { return m_scriptContext; }
    void ClearScriptContext() { m_scriptContext = nullptr; }

    bool IsBackgroundParser() const { return m_isInBackground; }
    bool IsDoingFastScan() const { return m_doingFastScan; }

    static IdentPtr PidFromNode(ParseNodePtr pnode);
    static LPCOLESTR GetClassName(PnClass *pClass);

    ParseNode* CopyPnode(ParseNode* pnode);
    IdentPtr GenerateIdentPtr(__ecount(len)wchar_t* name,long len);
    HRESULT GetTextAttribs(LPCOLESTR pszSrc, size_t cEncoded,
        SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr);
    HRESULT GetTextAttribsUTF8(LPCUTF8 pszSrc, size_t cEncoded,
        SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr);

    ArenaAllocator *GetAllocator() { return &m_nodeAllocator;}

    size_t GetSourceLength() { return m_length; }
    size_t GetOriginalSourceLength() { return m_originalLength; }
    static ULONG GetDeferralThreshold(Js::SourceDynamicProfileManager* profileManager);
    BOOL DeferredParse(Js::LocalFunctionId functionId);
    BOOL IsDeferredFnc();
    void ReduceDeferredScriptLength(ULONG chars);

    void RestorePidRefForSym(Symbol *sym);

    HRESULT ValidateSyntax(LPCUTF8 pszSrc, size_t encodedCharCount, bool isGenerator, CompileScriptException *pse, void (Parser::*validateFunction)());

#if ERROR_RECOVERY
    BOOL HasUncertainStructure() { return m_uncertainStructure; }
    BOOL HasSubsumedFunction() { return m_hasSubsumedFunction; };

    void SetErrorCallback(ParseErrorCallback callback, void *data) { m_errorCallback = callback; m_errorCallbackData = data; }

    void ReportError(HRESULT hr);

    void SetCommentCallback(CommentCallback callback, void *data)
    {
        if (m_pscan)
            m_pscan->SetCommentCallback(callback, data);
        else
        {
            m_commentCallback = callback;
            m_commentCallbackData = data;
        }
    }
#endif

    // Should be called when the UTF-8 source was produced from UTF-16. This is really CESU-8 source in that it encodes surragate pairs
    // as 2 three byte sequences instead of 4 bytes as required UTF-8. It also is is loss-less converison of invalid UTF-16 sequences.
    // This is important in Javascript because Javascript engines are required not report invalid UTF-16 sequences and to consider
    // the UTF-16 characters pre-canonacalized. Converting this UTF-16 with invalid sequences to valid UTF-8 and back would cause
    // all invalid UTF-16 seqences to be replace by one or more Unicode replacement characters (0xFFFD), losing the original
    // invalid sequences.
    HRESULT ParseCesu8Source(__out ParseNodePtr* parseTree, LPCUTF8 pSrc, size_t length, ULONG grfsrc, CompileScriptException *pse,
        Js::LocalFunctionId * nextFunctionId, SourceContextInfo * sourceContextInfo);

    // Should be called when the source is UTF-8 and invalid UTF-8 sequences should be replaced with the unicode replacement character
    // (0xFFFD). Security concerns require externally produced UTF-8 only allow valid UTF-8 otherwise an attacker could use invalid
    // UTF-8 sequences to fool a filter and cause Javascript to be executed that might otherwise have been rejected.
    HRESULT ParseUtf8Source(__out ParseNodePtr* parseTree, LPCUTF8 pSrc, size_t length, ULONG grfsrc, CompileScriptException *pse,
        Js::LocalFunctionId * nextFunctionId, SourceContextInfo * sourceContextInfo);

    // Used by deferred parsing to parse a deferred function.
    HRESULT ParseSourceWithOffset(__out ParseNodePtr* parseTree, LPCUTF8 pSrc, size_t offset, size_t cbLength, charcount_t cchOffset,
        bool isCesu8, ULONG grfscr, CompileScriptException *pse, Js::LocalFunctionId * nextFunctionId, ULONG lineNumber,
        SourceContextInfo * sourceContextInfo, Js::ParseableFunctionInfo* functionInfo, bool isReparse, bool isAsmJsDisabled);

protected:
    template <typename EncodingPolicy>
    HRESULT GetTextAttribsImpl(typename EncodingPolicy::EncodedCharPtr pszSrc, size_t cEncoded,
                               SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr);
    template <typename EncodingPolicy>
    void GetTextAttribsImpl(Scanner< EncodingPolicy > *scanner, typename EncodingPolicy::EncodedCharPtr pstr, DWORD dwFlags,
        __ecount(cch) SOURCE_TEXT_ATTR *prgsa, ulong cch);
    template <typename EncodingPolicy>
    void GetNormalTextAttribs(Scanner< EncodingPolicy > *scanner, typename EncodingPolicy::EncodedCharPtr pstr, DWORD dwFlags,
        __out_ecount(cch) SOURCE_TEXT_ATTR *prgsa, ulong cch);
    template <typename EncodingPolicy>
    void GetDepScanTextAttribs(Scanner< EncodingPolicy > *scanner, typename EncodingPolicy::EncodedCharPtr pstr, DWORD dwFlags,
        __out_ecount(cch) SOURCE_TEXT_ATTR *prgpsa, ulong cch);
    template <typename EncodingPolicy>
    void SetHumanTextForCurrentToken(Scanner< EncodingPolicy >* scanner, typename EncodingPolicy::EncodedCharPtr pstr,
        SOURCE_TEXT_ATTR *prgsa);

    HRESULT ParseSourceInternal(
        __out ParseNodePtr* parseTree, LPCUTF8 pszSrc, size_t offsetInBytes,
        size_t lengthInCodePoints, charcount_t offsetInChars, bool fromExternal,
        ULONG grfscr, CompileScriptException *pse, Js::LocalFunctionId * nextFunctionId, ULONG lineNumber, SourceContextInfo * sourceContextInfo);

    ParseNodePtr Parse(LPCUTF8 pszSrc, size_t offset, size_t length, charcount_t charOffset, ULONG grfscr, ULONG lineNumber,
        Js::LocalFunctionId * nextFunctionId, CompileScriptException *pse);

private:
    /***********************************************************************
    Core members.
    ***********************************************************************/
    HashTbl *   m_phtbl;
    ParseNodeAllocator m_nodeAllocator;
    ErrHandler  m_err;
    long        m_cactIdentToNodeLookup;
    ulong       m_grfscr;
    size_t      m_length;             // source length in characters excluding comments and literals
    size_t      m_originalLength;             // source length in characters excluding comments and literals
    Js::LocalFunctionId * m_nextFunctionId;
    SourceContextInfo*    m_sourceContextInfo;
    CatchPidRefList *m_catchPidRefList;

    ParseErrorCallback  m_errorCallback;
    void *              m_errorCallbackData;
    CommentCallback     m_commentCallback;
    void*               m_commentCallbackData;    
    BOOL                m_uncertainStructure;
    BOOL                m_hasSubsumedFunction;
    bool                m_hasParallelJob;
    bool                m_doingFastScan;    
    Span                m_asgToConst;
    int                 m_nextBlockId;

    // RegexPattern objects created for literal regexes are recycler-allocated and need to be kept alive until the function body
    // is created during byte code generation. The RegexPattern pointer is stored in the script context's guest
    // arena for that purpose. This list is then unregistered from the guest arena at the end of parsing/scanning.
    SList<UnifiedRegex::RegexPattern *, ArenaAllocator> m_registeredRegexPatterns;

    // Should be under #if ERROR_RECOVERY, but can't change the size of Parser for LS
    IdentPtr            m_pidError;
    IdentPtr            m_pidDeclError;     // Used for member declarations to avoid cases of infinite recursion due to interaction with missing ident errors

protected:
    Js::ScriptContext* m_scriptContext;

private:
    void GenerateCode(ParseNodePtr pnode, void *pvUser, long cbUser,
        LPCOLESTR pszSrc, long cchSrc, LPCOLESTR pszTitle);

#if ERROR_RECOVERY
    void Error(HRESULT hr);
    void Error(HRESULT hr, ParseNodePtr pnode);
    void Error(HRESULT hr, charcount_t ichMin, charcount_t ichLim);
#else
    __declspec(noreturn) void Error(HRESULT hr);
    __declspec(noreturn) void Error(HRESULT hr, ParseNodePtr pnode);
    __declspec(noreturn) void Error(HRESULT hr, charcount_t ichMin, charcount_t ichLim);
#endif
    __declspec(noreturn) static void OutOfMemory();


    void EnsureStackAvailable();

    void IdentifierExpectedError(const Token& token);

    void Skip(ERROR_RECOVERY_FORMAL);

    bool CheckForDirective(bool* pIsUseStrict, bool* pIsUseAsm, bool* pIsOctalInString);
    bool CheckStrictModeStrPid(IdentPtr pid);
    bool CheckAsmjsModeStrPid(IdentPtr pid);


    void InitPids();

    CatchPidRefList *GetCatchPidRefList() const { return m_catchPidRefList; }
    void SetCatchPidRefList(CatchPidRefList *list) { m_catchPidRefList = list; }
    CatchPidRefList *EnsureCatchPidRefList();

    // True if we need to create PID's and bind names to decls in deferred functions.
    // Do this if we need to support early let/const errors.
    bool BindDeferredPidRefs() const;

    /***********************************************************************
    Members needed just for parsing.
    ***********************************************************************/
    Token       m_token;

protected:
    Scanner_t*  m_pscan;

public:

    // create nodes using arena allocator; used by AST transformation
    template <OpCode nop>
    static ParseNodePtr StaticCreateNodeT(ArenaAllocator* alloc, charcount_t ichMin = 0, charcount_t ichLim = 0);
    static ParseNodePtr StaticCreateBinNode(OpCode nop, ParseNodePtr pnode1,ParseNodePtr pnode2,ArenaAllocator* alloc);
    static ParseNodePtr StaticCreateBlockNode(ArenaAllocator* alloc, charcount_t ichMin = 0, charcount_t ichLim = 0, int blockId = -1, PnodeBlockType blockType = PnodeBlockType::Regular);
    ParseNodePtr CreateNode(OpCode nop, charcount_t ichMin,charcount_t ichLim);
    ParseNodePtr CreateDummyFuncNode(bool fDeclaration);


    ParseNodePtr CreateTriNode(OpCode nop, ParseNodePtr pnode1,
                               ParseNodePtr pnode2, ParseNodePtr pnode3,
                               charcount_t ichMin,charcount_t ichLim);
    ParseNodePtr CreateTempNode(ParseNode* initExpr);
    ParseNodePtr CreateTempRef(ParseNode* tempNode);

    ParseNodePtr CreateNode(OpCode nop) { return CreateNode(nop, m_pscan? m_pscan->IchMinTok() : 0); }
    ParseNodePtr CreateDeclNode(OpCode nop, IdentPtr pid, SymbolType symbolType, bool errorOnRedecl = true);
    Symbol*      AddDeclForPid(ParseNodePtr pnode, IdentPtr pid, SymbolType symbolType, bool errorOnRedecl);
    ParseNodePtr CreateNameNode(IdentPtr pid)
    {
        ParseNodePtr pnode = CreateNode(knopName);
        pnode->sxPid.pid = pid;
        pnode->sxPid.sym=NULL;
        pnode->sxPid.symRef=NULL;
        return pnode;
    }
#if ERROR_RECOVERY
    ParseNodePtr CreateErrorNameNode()
    {
        ParseNodePtr pnode = CreateNameNode(m_pidError);
        PidRefStack *ref = this->PushPidRef(m_pidError);
        pnode->sxPid.SetSymRef(ref);

        return pnode;
    }
#endif
    ParseNodePtr CreateBlockNode(PnodeBlockType blockType = PnodeBlockType::Regular)
    {
        ParseNodePtr pnode = CreateNode(knopBlock);
        InitBlockNode(pnode, m_nextBlockId++, blockType);
        return pnode;
    }
    // Creating parse nodes.

    ParseNodePtr CreateNode(OpCode nop, charcount_t ichMin);
    ParseNodePtr CreateTriNode(OpCode nop, ParseNodePtr pnode1, ParseNodePtr pnode2, ParseNodePtr pnode3);
    ParseNodePtr CreateIntNode(long lw);
    ParseNodePtr CreateStrNode(IdentPtr pid);

    ParseNodePtr CreateUniNode(OpCode nop, ParseNodePtr pnodeOp);
    ParseNodePtr CreateBinNode(OpCode nop, ParseNodePtr pnode1, ParseNodePtr pnode2);
    ParseNodePtr CreateCallNode(OpCode nop, ParseNodePtr pnode1, ParseNodePtr pnode2);

    // Create parse node with token limis
    template <OpCode nop>
    ParseNodePtr CreateNodeT(charcount_t ichMin,charcount_t ichLim);
    ParseNodePtr CreateUniNode(OpCode nop, ParseNodePtr pnode1, charcount_t ichMin,charcount_t ichLim);
    ParseNodePtr CreateBlockNode(charcount_t ichMin,charcount_t ichLim, PnodeBlockType blockType = PnodeBlockType::Regular);
    ParseNodePtr CreateNameNode(IdentPtr pid,charcount_t ichMin,charcount_t ichLim);
    ParseNodePtr CreateBinNode(OpCode nop, ParseNodePtr pnode1, ParseNodePtr pnode2,
        charcount_t ichMin,charcount_t ichLim);
    ParseNodePtr CreateCallNode(OpCode nop, ParseNodePtr pnode1, ParseNodePtr pnode2,
        charcount_t ichMin,charcount_t ichLim);

    void PrepareScanner(bool fromExternal);
    void PrepareForBackgroundParse();
    void AddFastScannedRegExpNode(ParseNodePtr const pnode);
    void AddBackgroundRegExpNode(ParseNodePtr const pnode);
    void AddBackgroundParseItem(BackgroundParseItem *const item);
    void FinishBackgroundRegExpNodes();
    void FinishBackgroundPidRefs(BackgroundParseItem *const item, bool isOtherParser);
    void WaitForBackgroundJobs(BackgroundParser *bgp, CompileScriptException *pse);
    HRESULT ParseFunctionInBackground(ParseNodePtr pnodeFunc, ParseContext *parseContext, bool topLevelDeferred, CompileScriptException *pse);

    void CheckPidIsValid(IdentPtr pid, bool autoArgumentsObject = false);
    void AddVarDeclToBlock(ParseNode *pnode);
    // Add a var declaration. Only use while parsing. Assumes m_ppnodeVar is pointing to the right place already
    ParseNodePtr CreateVarDeclNode(IdentPtr pid, SymbolType symbolType, bool autoArgumentsObject = false, ParseNodePtr pnodeFnc = NULL, bool checkReDecl = true);
    // Add a var declaration, during parse tree rewriting. Will setup m_ppnodeVar for the given pnodeFnc
    ParseNodePtr AddVarDeclNode(IdentPtr pid, ParseNodePtr pnodeFnc);
    // Add a 'const' or 'let' declaration.
    ParseNodePtr CreateBlockScopedDeclNode(IdentPtr pid, OpCode nodeType);

    void RegisterRegexPattern(UnifiedRegex::RegexPattern *const regexPattern);
    bool IsReparsing() const { return m_parseType == ParseType_Reparse; }


#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    WCHAR* GetParseType() const
    {
        switch(m_parseType)
        {
            case ParseType_Upfront:
                return L"Upfront";
            case ParseType_Deferred:
                return L"Deferred";
            case ParseType_Reparse:
                return L"Reparse";
        }
        Assert(false);
        return NULL;
    }


#endif

    void CaptureContext(ParseContext *parseContext) const;
    void RestoreContext(ParseContext *const parseContext);
    int GetLastBlockId() const { Assert(m_nextBlockId > 0); return m_nextBlockId - 1; }

private:
    template <OpCode nop> ParseNodePtr CreateNodeWithScanner();
    template <OpCode nop> ParseNodePtr CreateNodeWithScanner(charcount_t ichMin);
    ParseNodePtr CreateStrNodeWithScanner(IdentPtr pid);
    ParseNodePtr CreateIntNodeWithScanner(long lw);

    static void InitNode(OpCode nop,ParseNodePtr pnode);
    static void InitBlockNode(ParseNodePtr pnode, int blockId, PnodeBlockType blockType);

private:
    ParseNodePtr m_currentNodeFunc; // current function or NULL
    ParseNodePtr m_currentNodeDeferredFunc; // current function or NULL
    ParseNodePtr m_currentNodeProg; // current programm
    DeferredFunctionStub *m_currDeferredStub;
    long * m_pCurrentAstSize;
    ParseNodePtr * m_ppnodeScope;  // function list tail
    ParseNodePtr * m_ppnodeExprScope; // function expression list tail
    ParseNodePtr * m_ppnodeVar;  // variable list tail
    bool m_inDeferredNestedFunc; // true if parsing a function in deferred mode, nested within the current node
    bool m_isInBackground;
    uint * m_pnestedCount; // count of functions nested at one level below the current node

    struct WellKnownPropertyPids
    {
        IdentPtr arguments; // m_pidArguments; // 'arguments' identifier
        IdentPtr eval; // m_pidEval;
        IdentPtr setter; // m_pidSetter;
        IdentPtr getter; // m_pidGetter;
        IdentPtr let; //m_pidLet;
        IdentPtr constructor; //m_pidConstructor;
        IdentPtr prototype; //m_pidPrototype;
        IdentPtr __proto__; // m_pid__proto__;
        IdentPtr of; //m_pidOf;
    };

    WellKnownPropertyPids wellKnownPropertyPids;

    charcount_t m_sourceLim; // The actual number of characters parsed.

    Js::ParseableFunctionInfo* m_functionBody; // For a deferred parsed function, the function body is non-null
    ParseType m_parseType;

    uint m_parsingDuplicate;
    uint m_exprDepth;
    uint m_arrayDepth;
    uint m_funcInArrayDepth; // Count func depth within array literal
    charcount_t m_funcInArray;
    uint m_scopeCountNoAst;

    // Used for issuing spread and rest errors when there is ambiguity with parameter list and parenthesized expressions
    uint m_parenDepth;
    bool m_deferEllipsisError;
    RestorePoint m_EllipsisErrLoc;

    uint m_tryCatchOrFinallyDepth;  // Used to determine if parsing is currently in a try/catch/finally block in order to throw error on yield expressions inside them

    StmtNest *m_pstmtCur; // current statement or NULL
    BlockInfoStack *m_currentBlockInfo;
    Scope *m_currentScope;

    BackgroundParseItem *currBackgroundParseItem;
    BackgroundParseItem *backgroundParseItems;
    typedef DList<ParseNodePtr, ArenaAllocator> NodeDList;
    NodeDList* fastScannedRegExpNodes;

    BlockIdsStack *m_currentDynamicBlock;
    int GetCurrentDynamicBlockId() const;

    void AppendFunctionToScopeList(bool fDeclaration, ParseNodePtr pnodeFnc);

    // block scoped content helpers
    void SetCurrentStatement(StmtNest *stmt);
    ParseNode* GetCurrentBlock();
    ParseNode* GetFunctionBlock();
    BlockInfoStack* GetCurrentBlockInfo();
    BlockInfoStack* GetCurrentFunctionBlockInfo();
    ParseNode *GetCurrentFunctionNode();
    bool IsNodeAllowedForDeferParse(OpCode op) {return !this->m_deferringAST ||
        (op == knopBlock || op == knopVarDecl || op == knopConstDecl || op == knopLetDecl || op == knopFncDecl); }
    bool NextTokenConfirmsLetDecl() const { return m_token.tk == tkID || m_token.tk == tkLBrack || m_token.tk == tkLCurly || m_token.IsReservedWord(); }

    template<bool buildAST>
    void PushStmt(StmtNest *pStmt, ParseNodePtr pnode, OpCode op, ParseNodePtr pnodeLab, LabelId* pLabelIdList)
    {
        AssertMem(pStmt);

        if (buildAST)
        {
            AssertNodeMem(pnode);
            AssertNodeMemN(pnodeLab);

            pnode->sxStmt.grfnop = 0;
            pnode->sxStmt.pnodeOuter = (NULL == m_pstmtCur) ? NULL : m_pstmtCur->pnodeStmt;

            pStmt->pnodeStmt = pnode;
            pStmt->pnodeLab = pnodeLab;
        }
        else
        {
            // Assign to pnodeStmt rather than op so that we initialize the whole field.
            pStmt->pnodeStmt = 0;
            pStmt->isDeferred = true;
            pStmt->op = op;
            pStmt->pLabelId = pLabelIdList;
        }
        pStmt->pstmtOuter = m_pstmtCur;
        SetCurrentStatement(pStmt);
    }

    void PopStmt(StmtNest *pStmt)
    {
        Assert(pStmt == m_pstmtCur);
        SetCurrentStatement(m_pstmtCur->pstmtOuter);
    }

    BlockInfoStack *PushBlockInfo(ParseNodePtr pnodeBlock);
    void PopBlockInfo();
    void PushDynamicBlock();
    void PopDynamicBlock();

    ParseNodePtr PnodeLabel(IdentPtr pid, ParseNodePtr pnodeLabels);

    void MarkEvalCaller()
    {
        if (m_currentNodeFunc)
        {
            ParseNodePtr pnodeFunc = GetCurrentFunctionNode();
            pnodeFunc->sxFnc.SetCallsEval(true);
        }
        ParseNode *pnodeBlock = GetCurrentBlock();
        if (pnodeBlock != NULL)
        {
            pnodeBlock->sxBlock.SetCallsEval(true);
            PushDynamicBlock();
        }
    }

public:
    WellKnownPropertyPids* names(){ return &wellKnownPropertyPids; }

    IdentPtr CreatePid(__in_ecount(len) LPCOLESTR name, charcount_t len)
    {
        return m_phtbl->PidHashNameLen(name, len);
    }

    bool KnownIdent(__in_ecount(len) LPCOLESTR name, charcount_t len)
    {
        return m_phtbl->Contains(name, len);
    }

private:
    struct IdentToken
    {
        tokens tk;
        IdentPtr pid;
        charcount_t ichMin;
        charcount_t ichLim;

        IdentToken()
            : tk(tkNone), pid(NULL)
        {
        }
    };

    void CheckArguments(ParseNodePtr pnode);
    void CheckArgumentsUse(IdentPtr pid, ParseNodePtr pnodeFnc);
    
    void CheckStrictModeEvalArgumentsUsage(IdentPtr pid, ParseNodePtr pnode = NULL);
    void CheckStrictModeFncDeclNotSourceElement(const bool isSourceElement, const BOOL isDeclaration);

    // environments on which the strict mode is set, if found
    enum StrictModeEnvironment
    {
        SM_NotUsed,         // StrictMode environment is don't care
        SM_OnGlobalCode,    // The current environment is a global code
        SM_OnFunctionCode,  // The current environment is a function code
        SM_DeferedParse     // StrictMode used in defered parse cases
    };

    template<bool buildAST> ParseNodePtr ParseArrayLiteral(ERROR_RECOVERY_FORMAL);

    template<bool buildAST> ParseNodePtr ParseStatement(ERROR_RECOVERY_FORMAL_ bool isSourceElement = false);
    template<bool buildAST> ParseNodePtr ParseVariableDeclaration(
        ERROR_RECOVERY_FORMAL_ tokens declarationType, charcount_t ichMin,
#if PARSENODE_EXTENSIONS
        charcount_t ichDeadRangeMin,
#endif
        BOOL fAllowIn = TRUE,
        BOOL* pfForInOk = nullptr,
        BOOL singleDefOnly = FALSE,
        BOOL allowInit = TRUE);

    template<bool buildAST>
    void ParseStmtList(
        ERROR_RECOVERY_FORMAL_
        ParseNodePtr *ppnodeList,
        ParseNodePtr **pppnodeLast = NULL,
        StrictModeEnvironment smEnvironment = SM_NotUsed,
        const bool isSourceElementList = false,
        _Out_opt_ bool* strictModeOn = NULL);
    bool FastScanFormalsAndBody();
    bool ScanAheadToFunctionEnd(uint count);

    bool DoParallelParse(ParseNodePtr pnodeFnc) const;

    // TODO: We should really call this StartScope and separate out the notion of scopes and blocks;
    // blocks refer to actual curly braced syntax, whereas scopes contain symbols.  All blocks have
    // a scope, but some statements like for loops or the with statement introduce a block-less scope.
    template<bool buildAST> ParseNodePtr StartParseBlock(PnodeBlockType blockType, ScopeType scopeType, ParseNodePtr pnodeLabel = NULL, LabelId* pLabelId = NULL);
    template<bool buildAST> ParseNodePtr StartParseBlockWithCapacity(PnodeBlockType blockType, ScopeType scopeType, int capacity);
    template<bool buildAST> ParseNodePtr StartParseBlockHelper(PnodeBlockType blockType, Scope *scope, ParseNodePtr pnodeLabel, LabelId* pLabelId);
    void PushFuncBlockScope(ParseNodePtr pnodeBlock, ParseNodePtr **ppnodeScopeSave, ParseNodePtr **ppnodeExprScopeSave);
    void PopFuncBlockScope(ParseNodePtr *ppnodeScopeSave, ParseNodePtr *ppnodeExprScopeSave);
    template<bool buildAST> ParseNodePtr ParseBlock(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeLabel, LabelId* pLabelId);
    void FinishParseBlock(ParseNode *pnodeBlock, bool needScanRCurly = true);
    void FinishParseFncExprScope(ParseNodePtr pnodeFnc, ParseNodePtr pnodeFncExprScope);

    template<const bool backgroundPidRefs>
    void BindPidRefs(BlockInfoStack *blockInfo, uint maxBlockId = (uint)-1);
    void BindPidRefsInScope(IdentPtr pid, Symbol *sym, int blockId, uint maxBlockId = (uint)-1);
    void BindConstPidRefsInScope(IdentPtr pid, Symbol *sym, int blockId, uint maxBlockId = (uint)-1);
    template<const bool constBinding>
    void BindPidRefsInScopeImpl(IdentPtr pid, Symbol *sym, int blockId, uint maxBlockId = (uint)-1);
    void PushScope(Scope *scope);
    void PopScope(Scope *scope);

    void ParseLanguageServiceContent(
        ERROR_RECOVERY_FORMAL_
        ParseNodePtr *ppnodeList,
        ParseNodePtr **pppnodeLast,
        StrictModeEnvironment smEnvironment,
        const bool isSourceElementList = false);
    template<bool buildAST> ParseNodePtr ParseArgList(ERROR_RECOVERY_FORMAL_ bool *pCallOfConstants, uint16 *pSpreadArgCount, uint16 * pCount);
    template<bool buildAST> ParseNodePtr ParseArrayList(ERROR_RECOVERY_FORMAL_ bool *pArrayOfTaggedInts, bool *pArrayOfInts, bool *pArrayOfNumbers, bool *pHasMissingValues, uint *count, uint *spreadCount);
    template<bool buildAST> ParseNodePtr ParseMemberList(ERROR_RECOVERY_FORMAL_ LPCOLESTR pNameHint, ulong *pHintLength);
    template<bool buildAST> ParseNodePtr ParseSuper(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnode);

    // Used to determine the type of JavaScript object member.
    // The values can be combined using bitwise OR.
    //       specifically, it is valid to have getter and setter at the same time.
    enum MemberType
    {
        MemberTypeDataProperty = 1 << 0, // { foo: 1 },
        MemberTypeGetter       = 1 << 1, // { get foo() }
        MemberTypeSetter       = 1 << 2, // { set foo(arg) {} }
        MemberTypeMethod       = 1 << 3, // { foo() {} }
        MemberTypeIdentifier   = 1 << 4  // { foo } (shorthand for { foo: foo })
    };

    // Used to map JavaScript object member name to member type.
    typedef JsUtil::BaseDictionary<WCHAR*, MemberType, ArenaAllocator, PrimeSizePolicy> MemberNameToTypeMap;

    static MemberNameToTypeMap* CreateMemberNameMap(ArenaAllocator* pAllocator);

    template<bool buildAST> void ParseComputedName(ERROR_RECOVERY_FORMAL_ ParseNodePtr* ppnodeName, LPCOLESTR* ppNameHint, LPCOLESTR* ppFullNameHint = nullptr, ulong *returnLength = nullptr);
    template<bool buildAST> ParseNodePtr ParseMemberGetSet(ERROR_RECOVERY_FORMAL_ OpCode nop, LPCOLESTR* ppNameHint);
    template<bool buildAST> ParseNodePtr ParseFncDecl(ERROR_RECOVERY_FORMAL_ ushort flags, LPCOLESTR pNameHint = NULL, const bool isSourceElement = false, const bool needsPIDOnRCurlyScan = false, bool fUnaryOrParen = false);
    template<bool buildAST> bool ParseFncNames(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, ParseNodePtr pnodeFncParent, ushort flags, bool isSourceElement, ParseNodePtr **pLastNodeRef);
    template<bool buildAST> void ParseFncFormals(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, ushort flags);
    template<bool buildAST> bool ParseFncDeclHelper(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, ParseNodePtr pnodeFncParent, LPCOLESTR pNameHint, ushort flags, bool *pHasName, bool isSourceElement, bool fUnaryOrParen, bool *pNeedScanRCurly);
    template<bool buildAST> void ParseExpressionLambdaBody(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc);
    void FinishFncDecl(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, LPCOLESTR pNameHint, ParseNodePtr *lastNodeRef);
    void ParseTopLevelDeferredFunc(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, ParseNodePtr pnodeFncParent, LPCOLESTR pNameHint);
    void ParseNestedDeferredFunc(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, bool fLambda, bool *pNeedScanRCurly, bool *pStrictModeTurnedOn);
    void CheckStrictFormalParameters(ERROR_RECOVERY_FORMAL);
    void AddArgumentsNodeToVars(ParseNodePtr pnodeFnc);

    LPCOLESTR GetFunctionName(ParseNodePtr pnodeFnc, LPCOLESTR pNameHint);
    uint CalculateFunctionColumnNumber();

    template<bool buildAST> ParseNodePtr GenerateEmptyConstructor(bool extends = false);

    IdentPtr ParseClassPropertyName(ERROR_RECOVERY_FORMAL_ IdentPtr * hint);
    template<bool buildAST> ParseNodePtr ParseClassDecl(ERROR_RECOVERY_FORMAL_ BOOL isDeclaration, LPCOLESTR pNameHint, ulong *pHintLength);

    template<bool buildAST> ParseNodePtr ParseStringTemplateDecl(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeTagFnc);

    // This is used in the es6 class pattern.
    LPCOLESTR ConstructFinalHintNode(IdentPtr pClassName, IdentPtr pMemberName, IdentPtr pGetSet, bool isStatic, ulong* nameLength, bool isComputedName = false, LPCOLESTR pMemberNameHint = nullptr);

    // Construct the name from the parse node.
    LPCOLESTR FormatPropertyString(LPCOLESTR propertyString, ParseNodePtr pNode, ulong *fullNameHintLength);
    LPCOLESTR ConstructNameHint(ParseNodePtr pNode, ulong* fullNameHintLength);
    LPCOLESTR AppendNameHints(IdentPtr  left, IdentPtr  right, ulong *returnLength, bool ignoreAddDotWithSpace = false, bool wrapInBrackets = false);
    LPCOLESTR AppendNameHints(IdentPtr  left, LPCOLESTR right, ulong *returnLength, bool ignoreAddDotWithSpace = false, bool wrapInBrackets = false);
    LPCOLESTR AppendNameHints(LPCOLESTR left, IdentPtr  right, ulong *returnLength, bool ignoreAddDotWithSpace = false, bool wrapInBrackets = false);
    LPCOLESTR AppendNameHints(LPCOLESTR left, LPCOLESTR right, ulong *returnLength, bool ignoreAddDotWithSpace = false, bool wrapInBrackets = false);
    LPCOLESTR AppendNameHints(LPCOLESTR leftStr, ulong leftLen, LPCOLESTR rightStr, ulong rightLen, ulong *returnLength, bool ignoreAddDotWithSpace = false, bool wrapInBrackets = false);
    WCHAR * AllocateStringOfLength(ulong length);

    void FinishFncNode(ParseNodePtr pnodeFnc);

    template<bool buildAST> bool ParseOptionalExpr(
        ERROR_RECOVERY_FORMAL_
        ParseNodePtr* pnode,
        int oplMin = koplNo,
        BOOL *pfCanAssign = NULL,
        BOOL fAllowIn = TRUE,
        BOOL fAllowEllipsis = FALSE,
        LPCOLESTR pHint = NULL,
        ulong *pHintLength = nullptr,
        _Inout_opt_ IdentToken* pToken = NULL);
    template<bool buildAST> ParseNodePtr ParseExpr(
        ERROR_RECOVERY_FORMAL_
        int oplMin = koplNo,
        BOOL *pfCanAssign = NULL,
        BOOL fAllowIn = TRUE,
        BOOL fAllowEllipsis = FALSE,
        LPCOLESTR pHint = NULL,
        ulong *pHintLength = nullptr,
        _Inout_opt_ IdentToken* pToken = NULL,
        bool fUnaryOrParen = false);
    template<bool buildAST> ParseNodePtr ParseTerm(
        ERROR_RECOVERY_FORMAL_
        BOOL fAllowCall = TRUE,
        LPCOLESTR pNameHint = NULL,
        ulong *pHintLength = nullptr,
        _Inout_opt_ IdentToken* pToken = NULL,
        bool fUnaryOrParen = false);
    template<bool buildAST> ParseNodePtr ParsePostfixOperators(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnode,
        BOOL fAllowCall, BOOL fInNew, _Inout_ IdentToken* pToken);

    BOOL NodeIsIdent(ParseNodePtr pnode, IdentPtr pid);
    BOOL NodeIsEvalName(ParseNodePtr pnode);        
    BOOL IsJSONValid(ParseNodePtr pnodeExpr)
    {
        OpCode jnop = (knopNeg == pnodeExpr->nop) ? pnodeExpr->sxUni.pnode1->nop : pnodeExpr->nop;
        if (knopNeg == pnodeExpr->nop)
        {
            return (knopInt == jnop ||  knopFlt == jnop);
        }
        else
        {
            return (knopInt == jnop ||  knopFlt == jnop ||
                knopStr == jnop ||  knopNull == jnop ||
                knopTrue == jnop || knopFalse == jnop ||
                knopObject == jnop || knopArray == jnop);
        }
    }

    BOOL IsConstantInFunctionCall(ParseNodePtr pnode);
    BOOL IsConstantInArrayLiteral(ParseNodePtr pnode);

    template<bool buildAST> ParseNodePtr ParseTryCatchFinally(ERROR_RECOVERY_FORMAL);
    template<bool buildAST> ParseNodePtr ParseTry(ERROR_RECOVERY_FORMAL);
    template<bool buildAST> ParseNodePtr ParseCatch(ERROR_RECOVERY_FORMAL);
    template<bool buildAST> ParseNodePtr ParseFinally(ERROR_RECOVERY_FORMAL);

    template<bool buildAST> ParseNodePtr ParseCase(ERROR_RECOVERY_FORMAL_ ParseNodePtr *ppnodeBody);
    template<bool buildAST> ParseNodePtr ParseRegExp();

    template <bool buildAST>
    ParseNodePtr ParseDestructuredArrayLiteral(ERROR_RECOVERY_FORMAL_ tokens declarationType, bool isDecl, bool topLevel = true);

    template<bool CheckForNegativeInfinity> static bool IsNaNOrInfinityLiteral(LPCOLESTR str);

public:
    void ValidateSourceElementList();
    void ValidateFormals();

    bool IsStrictMode() const;
    BOOL ExpectingExternalSource();

    IdentPtr GetArgumentsPid() const { return wellKnownPropertyPids.arguments; }
    IdentPtr GetEvalPid() const { return wellKnownPropertyPids.eval; }
#if ERROR_RECOVERY
    IdentPtr GetErrorPid() const { return m_pidError; }
    IdentPtr GetDeclErrorPid() const { return m_pidDeclError; }
#endif
    BackgroundParseItem *GetCurrBackgroundParseItem() const { return currBackgroundParseItem; }
    void SetCurrBackgroundParseItem(BackgroundParseItem *item) { currBackgroundParseItem = item; }

    void Release()
    {
        RELEASEPTR(m_pscan);
        RELEASEPTR(m_phtbl);
    }

private:
    void DeferOrEmitPotentialSpreadError(ParseNodePtr pnodeT);
    template<bool buildAST> void TrackAssignment(ParseNodePtr pnodeT, IdentToken* pToken, charcount_t ichMin, charcount_t ichLim);
    PidRefStack* PushPidRef(IdentPtr pid);    
    PidRefStack* FindOrAddPidRef(IdentPtr pid, int blockId, int maxScopeId = -1);    
    void RemovePrevPidRef(IdentPtr pid, PidRefStack *lastRef);
    void SetPidRefsInScopeDynamic(IdentPtr pid, int blockId);

    void RestoreScopeInfo(Js::FunctionBody* functionBody);
    void FinishScopeInfo(Js::FunctionBody* functionBody);

    BOOL PnodeLabelNoAST(IdentToken* pToken, LabelId* pLabelIdList);
    LabelId* CreateLabelId(IdentToken* pToken);

    void AddToNodeList(ParseNode ** ppnodeList, ParseNode *** pppnodeLast, ParseNode * pnodeAdd);
    void AddToNodeListEscapedUse(ParseNode ** ppnodeList, ParseNode *** pppnodeLast, ParseNode * pnodeAdd);

#ifdef LANGUAGE_SERVICE
    void AppendToNodeList(ParseNode ** ppnodeList, ParseNode *** pppnodeLast,
        ParseNode * pnodeAdd, ParseNode ***pppnodeAddLast);
#endif

    void ChkCurTokNoScan(int tk, int wErr _ERROR_RECOVERY_FORMAL)
    {
        if (m_token.tk != tk)
        {
            Error(wErr);
            SKIP(ERROR_RECOVERY_ACTUAL(ers));
        }
    }

    void ChkCurTok(int tk, int wErr _ERROR_RECOVERY_FORMAL)
    {
        if (m_token.tk != tk)
        {
            Error(wErr);
            SKIP(ERROR_RECOVERY_ACTUAL(ers));
        }
        else
        {
            m_pscan->Scan();
        }
    }
    void ChkNxtTok(int tk, int wErr _ERROR_RECOVERY_FORMAL)
    {
        m_pscan->Scan();
        ChkCurTok(tk, wErr _ERROR_RECOVERY_ACTUAL(ers));
    }
#if PARSENODE_EXTENSIONS
    void ChkCurTok(int tk, int wErr, ERROR_RECOVERY_FORMAL_ charcount_t& tkMin)
    {
        if (m_token.tk != tk)
        {
            tkMin = 0;
            Error(wErr);
            SKIP(ERROR_RECOVERY_ACTUAL(ers));
        }
        else
        {
            tkMin = m_pscan->IchMinTok();
            m_pscan->Scan();
        }
    }
    void ChkCurTokNoScan(int tk, int wErr, ERROR_RECOVERY_FORMAL_ charcount_t& tkMin)
    {
        if (m_token.tk != tk)
        {
            tkMin = 0;
            Error(wErr);
            SKIP(ERROR_RECOVERY_ACTUAL(ers));
        }
        else
        {
            tkMin = m_pscan->IchMinTok();
        }
    }
    void ChkNxtTok(int tk, int wErr, ERROR_RECOVERY_FORMAL_ charcount_t& tkMin)
    {
        m_pscan->Scan();
        ChkCurTok(tk, wErr, ERROR_RECOVERY_ACTUAL_(ers) tkMin);
    }
#endif

    template <class Fn>
    void VisitFunctionsInScope(ParseNodePtr pnodeScopeList, Fn fn);
    void FinishDeferredFunction(ParseNodePtr pnodeScopeList);



    /***********************************************************************
    Misc
    ***********************************************************************/
    bool        m_UsesArgumentsAtGlobal; // "arguments" used at global code.

#if ECMACP
    BOOL m_fECMACP;  // ECMA Compact Profile (ECMA 327)
#endif // ECMACP
    BOOL m_fUseStrictMode; // ES5 Use Strict mode. In AST mode this is a global flag; in NoAST mode it is pushed and popped.
    bool m_InAsmMode; // Currently parsing Asm.Js module
    bool m_isAsmJsDisabled; // we disable parsing as asm.js if we are reparsing due to link time validation failure or under debugger
    bool m_deferAsmJs;
    BOOL m_fExpectExternalSource;
    BOOL m_deferringAST;
    BOOL m_stoppedDeferredParse;

    enum FncDeclFlag : ushort
    {
        fFncNoFlgs      = 0,
        fFncDeclaration = 1 << 0,
        fFncNoArg       = 1 << 1,
        fFncOneArg      = 1 << 2, //Force exactly one argument.
        fFncNoName      = 1 << 3,
        fFncLambda      = 1 << 4,
        fFncMethod      = 1 << 5,
        fFncClassMember = 1 << 6,
        fFncGenerator   = 1 << 7,
        fFncSetter      = 1 << 8,
    };

    //
    // If we need the scanner to force PID creation temporarily, use this auto object
    // to turn scanner deferred parsing off temporarily and restore at destructor.
    //
    class AutoTempForcePid
    {
    private:
        Scanner_t* m_scanner;
        BOOL m_forcePid;
        BYTE m_oldScannerDeferredParseFlags;

    public:
        AutoTempForcePid(Scanner_t* scanner, BOOL forcePid)
            : m_scanner(scanner), m_forcePid(forcePid)
        {
            if (forcePid)
            {
                m_oldScannerDeferredParseFlags = scanner->SetDeferredParse(FALSE);
            }
        }

        ~AutoTempForcePid()
        {
            if (m_forcePid)
            {
                m_scanner->SetDeferredParseFlags(m_oldScannerDeferredParseFlags);
            }
        }
    };

public:
    charcount_t GetSourceIchLim() { return m_sourceLim; }
    static BOOL NodeEqualsName(ParseNodePtr pnode, LPCOLESTR sz, ulong cch);

private:
#if PARSENODE_EXTENSIONS | ERROR_RECOVERY
    inline bool LanguageServiceMode()
    {
        return (fscrStmtCompletion & m_grfscr) != 0;
    }

#endif

    inline bool PerformingErrorRecovery()
    {
#if ERROR_RECOVERY
        return LanguageServiceMode();
#else
        return false;
#endif
    }
private:
    LanguageServiceExtension* m_languageServiceExtension;
public:
    LanguageServiceExtension* GetLanguageServiceExtension();
};

#if PARSENODE_EXTENSIONS
struct LanguageServiceExtension
{
public:
    enum CompletionRangeMode : char
    {
        // A value reserved to mean it is not a completion dead range
        None,
        /*
         * Normally we don't want to provide intellisense for object literal names because they are no good information we can use for help
         * But if the literal is instantiated at an offset where we can make sense of the 'type' of the literal, then we should help.
         */
        ObjectLiteralNames,

        // TODO (andrewau) consider classify the other completion dead ranges so that we document them and potentially useful for future
        // scenarios
        Others
    };

    struct CompletionRange
    {
        charcount_t ichMin;
        charcount_t ichLim;
        CompletionRangeMode mode;

        CompletionRange(charcount_t ichMin, charcount_t ichLim, CompletionRangeMode mode)
            : ichMin(ichMin), ichLim(ichLim), mode(mode)
        {
        }

        bool Contains(charcount_t offset)
        {
            return offset >= ichMin && offset <= ichLim;
        }
    };

private:
    struct ExtensionData
    {
        uint ichAsg;
        uint ichLCurly;
        uint ichRCurly;
        uint ichLParen;
        uint ichRParen;
        uint ichLBrack;
        uint ichRBrack;
        uint ichWhileMin;
        uint ichSwitchLim;
        uint ichIdentMin;
        uint ichIdentLim;
        uint nestedCount;
        uint ichColon;
        uint ichArgLim;
        uint ichtkFunctionMin;
        ExtensionData() : ichAsg(0), ichLCurly(0), ichRCurly(0), ichLParen(0), ichRParen(0), ichWhileMin(0), ichSwitchLim(0), nestedCount(0), ichColon(0), ichIdentMin(0), ichIdentLim(0), ichArgLim(0), ichtkFunctionMin(0)  { }
    };

    ArenaAllocator* m_alloc;
    JsUtil::BaseDictionary<ParseNode*, ExtensionData*, ArenaAllocator, PrimeSizePolicy>* m_parseNodeExtensions;
    JsUtil::BaseDictionary<ParseNode*, wchar_t*, ArenaAllocator, PrimeSizePolicy>* m_labels;
    JsUtil::BaseDictionary<ParseNode*, int, ArenaAllocator, PrimeSizePolicy>* m_parentheses;
    JsUtil::List<CompletionRange*, ArenaAllocator>* m_completionRanges;

public:
    void AddLabel(ParseNodePtr pnodeJmpStatment, ParseNodePtr pnodeLabel);
    LPCWSTR GetLabel(ParseNode* node);

    void IncrementParenthesesCount(ParseNodePtr node);
    int GetParenthesesCount(ParseNode* node);

    HRESULT Init(ParseNodeAllocator* nodeAllocator, PageAllocator* pageAllocator);
    void Clear();

    void SetLCurly(ParseNodePtr node, uint ichLCurly);
    void SetRCurly(ParseNodePtr node, uint ichRCurly);
    void SetLParen(ParseNodePtr node, uint ichLParen);
    void SetRParen(ParseNodePtr node, uint ichRParen);
    void SetLBrack(ParseNodePtr node, uint ichLBrack);
    void SetRBrack(ParseNodePtr node, uint ichRBrack);
    void SetWhileMin(ParseNodePtr node, uint ichWhileMin);
    void SetSwitchLim(ParseNodePtr node, uint ichSwitchLim);
    void SetNestedCount(ParseNodePtr node, uint nestedCount);
    void SetIdentMin(ParseNodePtr node, uint ichIdentMin);
    void SetIdentLim(ParseNodePtr node, uint ichIdentLim);
    void SetArgLim(ParseNodePtr node, uint ichArgMin);
    void SetTkFunctionMin(ParseNodePtr node, uint ichtkFunctionMin);
    void SetCompletionRange(uint min, uint lim, CompletionRangeMode completionRangeMode);

    uint LCurly(ParseNodePtr node);
    uint RCurly(ParseNodePtr node);
    uint LParen(ParseNodePtr node);
    uint RParen(ParseNodePtr node);
    uint LBrack(ParseNodePtr node);
    uint RBrack(ParseNodePtr node);
    uint WhileMin(ParseNodePtr node);
    uint SwitchLim(ParseNodePtr node);
    uint NestedCount(ParseNodePtr node);
    uint IdentMin(ParseNodePtr node);
    uint IdentLim(ParseNodePtr node);
    uint ArgLim(ParseNodePtr node);
    uint TkFunctionMin(ParseNodePtr node);

    JsUtil::List<CompletionRange*, ArenaAllocator>* CompletionRanges();
private:
    ExtensionData* NodeExtension(ParseNode* node, bool createIfNotExists = false);
};
#endif
