//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#if DBG_DUMP
void PrintPnodeWIndent(ParseNode *pnode,int indentAmt);
#endif
const uint HASH_TABLE_SIZE = 256;

const char* nopNames[knopLim]= {
#define PTNODE(nop,sn,pc,nk,grfnop,json,apnk) sn,
#include "ptlist.h"
};
void printNop(int nop) {
  printf("%s\n",nopNames[nop]);
}

const uint ParseNode::mpnopgrfnop[knopLim] =
{
#define PTNODE(nop,sn,pc,nk,grfnop,json,apnk) grfnop,
#include "ptlist.h"
};

bool Parser::BindDeferredPidRefs() const
{
    return m_scriptContext->GetConfig()->BindDeferredPidRefs();
}

#if ERROR_RECOVERY
static ErrorRecoverySet ErrorRecoverySetOf(tokens token)
{
    switch (token)
    {
    case tkComma:       return ersComma;
    case tkDArrow:      return ersDArrow;
    case tkSColon:      return ersSColon;
    case tkAsg:         return ersAsg;
    case tkLsh:
    case tkRsh:
    case tkRs2:
    case tkLE:
    case tkGE:
    case tkINSTANCEOF:
    case tkEQ:
    case tkNE:
    case tkEqv:
    case tkNEqv:
    case tkLogAnd:
    case tkLogOr:
    case tkAsgMul:
    case tkAsgDiv:
    case tkAsgMod:
    case tkAsgAdd:
    case tkAsgSub:
    case tkAsgLsh:
    case tkAsgRsh:
    case tkAsgRs2:
    case tkAsgAnd:
    case tkAsgXor:
    case tkAsgOr:
    case tkQMark:
    case tkStar:
    case tkDiv:
    case tkPct:
    case tkGT:
    case tkLT:
    case tkAnd:
    case tkXor:
    case tkOr:          return ersBinOp;
    case tkRBrack:      return ersRBrack;
    case tkRCurly:      return ersRCurly;
    case tkRParen:      return ersRParen;
    case tkDot:         return ersDot;
    case tkColon:       return ersColon;
    case tkAdd:
    case tkSub:         return ersAddOp;
    case tkLCurly:      return ersLCurly;
    case tkLParen:      return ersLParen;
    case tkLBrack:      return ersLBrack;
    case tkScope:       return ersScope;
    case tkIN:          return ersIn;
    case tkCASE:
    case tkDEFAULT:     return ersSCase;
    case tkELSE:        return ersElse;
    case tkCATCH:
    case tkFINALLY:     return ersCatch;
    case tkVAR:         return ersVar;
    case tkBREAK:
    case tkRETURN:
    case tkTHROW:
    case tkDEBUGGER:
    case tkFOR:
    case tkSWITCH:
    case tkDO:
    case tkIF:
    case tkTRY:
    case tkWITH:        return ersStmt;
    case tkWHILE:       return ersWhile;
    case tkID:          return ersID;
    case tkFUNCTION:    return ersFunc;
    // PreOp
    case tkTilde:
    case tkBang:
    case tkInc:
    case tkDec:
    case tkEllipsis:
    // RegExp
    case tkRegExp:
    // Prefix
    case tkVOID:
    case tkDELETE:
    case tkTYPEOF:
    case tkYIELD:
    // RLit
    case tkTHIS:
    case tkTRUE:
    case tkFALSE:
    case tkNULL:
    // Literal
    case tkIntCon:
    case tkFltCon:
    case tkStrCon:
    // Class
    case tkCLASS:       return ersExpr;
    case tkEOF:
    case tkExternalSourceStart:
    case tkExternalSourceEnd:
                        return ersEOF;
    }
    return ersNone;
}
#endif

#if DEBUG
Parser::Parser(Js::ScriptContext* scriptContext, BOOL strictMode, PageAllocator *alloc, bool isBackground, size_t size)
#else
Parser::Parser(Js::ScriptContext* scriptContext, BOOL strictMode, PageAllocator *alloc, bool isBackground)
#endif
    : m_nodeAllocator(L"Parser", alloc ? alloc : scriptContext->GetThreadContext()->GetPageAllocator(), Parser::OutOfMemory),
    // use the GuestArena directly for keeping the RegexPattern* alive during byte code generation
    m_registeredRegexPatterns(scriptContext->GetGuestArena())
{
    AssertMsg(size == sizeof(Parser), "verify conditionals affecting the size of Parser agree");
    Assert(scriptContext != null);
    m_isInBackground = isBackground;
    m_phtbl = NULL;
    m_pscan = NULL;
    m_deferringAST = FALSE;
    m_stoppedDeferredParse = FALSE;
    m_hasParallelJob = false;
    m_doingFastScan = false;
    m_scriptContext = scriptContext;
    m_pCurrentAstSize = NULL;
    m_parsingDuplicate = 0;
    m_exprDepth = 0;
    m_arrayDepth = 0;
    m_funcInArrayDepth = 0;
    m_parenDepth = 0;
    m_funcInArray = 0;
    m_tryCatchOrFinallyDepth = 0;
    m_UsesArgumentsAtGlobal = false;
    m_currentNodeFunc = NULL;
    m_currentNodeDeferredFunc = NULL;
    m_currentNodeProg = NULL;
    m_currDeferredStub = NULL;
    m_pstmtCur = NULL;
    m_currentBlockInfo = NULL;
    m_currentScope = NULL;
    m_currentDynamicBlock = NULL;
    m_catchPidRefList = nullptr;
    m_grfscr = fscrNil;
    m_length = 0;
    m_originalLength = 0;
    m_nextFunctionId = null;
    m_errorCallback = NULL;
    m_commentCallback = NULL;
    m_uncertainStructure = FALSE;
    m_hasSubsumedFunction = FALSE;
    currBackgroundParseItem = nullptr;
    backgroundParseItems = nullptr;
    fastScannedRegExpNodes = nullptr;

#if ECMACP
    m_fECMACP = FALSE;
#endif // ECMACP
    m_fUseStrictMode = strictMode;
    m_InAsmMode = false;
    m_deferAsmJs = true;
    m_isAsmJsDisabled = false;
    m_scopeCountNoAst = 0;
    m_fExpectExternalSource = 0;

    m_languageServiceExtension = NULL;
    m_parseType = ParseType_Upfront;

    m_deferEllipsisError = false;
}

Parser::~Parser(void)
{
    if (m_scriptContext == nullptr || m_scriptContext->GetGuestArena() == nullptr)
    {
        // If the scriptcontext or guestArena have gone away, there is no point clearing each item of this list.
        // Just reset it so that dtor of the SList will be no-op
        m_registeredRegexPatterns.Reset();
    }

    if (this->m_hasParallelJob)
    {
        // Let the background threads know that they can decommit their arena pages.
        BackgroundParser *bgp = m_scriptContext->GetBackgroundParser();
        Assert(bgp);
        if (bgp->Processor()->ProcessesInBackground())
        {
            JsUtil::BackgroundJobProcessor *processor = static_cast<JsUtil::BackgroundJobProcessor*>(bgp->Processor());

            bool result = processor->IterateBackgroundThreads([&](JsUtil::ParallelThreadData *threadData)->bool {
                threadData->canDecommit = true;
                return false;
            });
            Assert(result);
        }
    }

    Release();

#if PARSENODE_EXTENSIONS
    if(NULL != m_languageServiceExtension)
    {
        m_languageServiceExtension->Clear();
        m_languageServiceExtension = NULL;
    }
#endif
}

void Parser::OutOfMemory()
{
    throw ParseExceptionObject(ERRnoMemory);
}

void Parser::Error(HRESULT hr)
{
    Assert(FAILED(hr));
    m_err.Throw(hr);
#if ERROR_RECOVERY
    AssertMsg(LanguageServiceMode(), "why did Throw return?");
    if (hr == ERRnoLcurly || hr == ERRnoRcurly || m_token.tk == tkFUNCTION) m_uncertainStructure = TRUE;
#else
    AssertMsg(false, "why did Throw return?");
#endif
}

void Parser::Error(HRESULT hr, ParseNodePtr pnode)
{
    if (pnode && pnode->ichLim)
    {
        Error(hr, pnode->ichMin, pnode->ichLim);
    }
    else
    {
        Error(hr);
    }
}

void Parser::Error(HRESULT hr, charcount_t ichMin, charcount_t ichLim)
{
    m_pscan->SetErrorPosition(ichMin, ichLim);
    Error(hr);
}

void Parser::IdentifierExpectedError(const Token& token)
{
    Assert(token.tk != tkID);

    HRESULT hr;
    if (token.IsReservedWord())
    {
        if (token.IsKeyword())
        {
            hr = ERRKeywordNotId;
        }
        else
        {
            Assert(token.IsFutureReservedWord(true));
            if (token.IsFutureReservedWord(false))
            {
                // Future reserved word in strict and non-strict modes
                hr = ERRFutureReservedWordNotId;
            }
            else
            {
                // Future reserved word only in strict mode. The token would have been converted to tkID by the scanner if not
                // in strict mode.
                Assert(IsStrictMode());
                hr = ERRFutureReservedWordInStrictModeNotId;
            }
        }
    }
    else
    {
        hr = ERRnoIdent;
    }

    Error(hr);
}

#if ERROR_RECOVERY
void Parser::Skip(ERROR_RECOVERY_FORMAL)
{
    Assert(LanguageServiceMode());

    charcount_t errRangeMin = m_pscan->IchMinTok();
    charcount_t errRangeLim = 0;
    // Add a completion dead zone if the error is unexpected dot after a keyword. e.g. while. but allow multiple dots..
    bool bCompletionDeadZone = (m_token.tk == tkDot && m_pscan->m_tkPrevious != tkDot);

    ers = ErrorRecoverySet(ers | ersEOF); // Since tkEOF is guarenteed elsewhere to be the last token, this ensures we terminate.

    tokens previous = tkLim;
    tokens previous2;
    while ((ErrorRecoverySetOf(m_token.tk) & ers) == 0)
    {
        previous2 = previous;
        previous = m_token.tk;
        errRangeLim = m_pscan->IchLimTok();
        m_pscan->Scan();
        switch (m_token.tk)
        {
        case tkDiv:
        case tkAsgDiv:
            // The token might really be a regular expression. A regular expression is legal anywhere the / or /= operators
            // are not expected. To accurately determine if a / or /= is expected requires a valid syntax. If we are here
            // the syntax is not valid so we don't know. The only tokens that can preceed a / or /= in a valid syntax Javascript
            // prefix are a simple term (tkID, tkStrCon, tkIntCon, tkFltCon, tkTHIS, tkRegExp) or a postfix operator or operator
            // tail (tkInc, tkDec, tkRParen, tkRBrack, and tkRCurly) or a reserved word if it is immediately preceeded by a tkDot.
            // (Below tkRCurly is ignored because it is most-likely a false positive.) If the previous is a token that can preceed
            // a / or /= then we assume it is not a regular expression. Otherwise, we assume it is and try to convert it.
            switch (previous)
            {
            case tkID:
            case tkStrCon:
            case tkIntCon:
            case tkFltCon:
            case tkTHIS:
            case tkRegExp:
            case tkInc:
            case tkDec:
            case tkRParen:
            case tkRBrack:
                continue;
            default:
                // Handles the "a.<reserved-word> /= 5" case.
                if (previous2 == tkDot && m_token.IsReservedWord())
                    continue;
            }
            m_pscan->TryRescanRegExp();
            break;
        case tkLCurly:
        case tkRCurly:
            m_uncertainStructure = TRUE;
            break;
        }
    }

    if(LanguageServiceMode() && bCompletionDeadZone && errRangeLim >= errRangeMin)
    {
        m_languageServiceExtension->SetCompletionRange(errRangeMin, errRangeLim, LanguageServiceExtension::CompletionRangeMode::Others);
    }
}
#else
void Parser::Skip(ERROR_RECOVERY_FORMAL) { }
#endif

CatchPidRefList *Parser::EnsureCatchPidRefList()
{
    if (this->m_catchPidRefList == nullptr)
    {
        this->m_catchPidRefList = Anew(&m_nodeAllocator, CatchPidRefList);
    }
    return this->m_catchPidRefList;
}

/***************************************************************************
Get colorization information for the source.
***************************************************************************/
#if DEBUG
BOOL  g_fMapAttrib = FALSE;  // Set to true to get good colors for DepScan
DWORD g_dwTextAttribFlags = (DWORD)-1; // Set to force a particular set of flags.
#endif // DEBUG

HRESULT Parser::GetTextAttribs(LPCOLESTR pszSrc, size_t cEncoded,
        SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr)
{
    return GetTextAttribsImpl<NullTerminatedUnicodeEncodingPolicy>(pszSrc, cEncoded, prgsta, cch, dwFlags, grfscr);
}

HRESULT Parser::GetTextAttribsUTF8(LPCUTF8 pszSrc, size_t cEncoded,
        SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr)
{
    return GetTextAttribsImpl<NullTerminatedUTF8EncodingPolicy>(pszSrc, cEncoded, prgsta, cch, dwFlags, grfscr);
}

template <typename EncodingPolicy>
HRESULT Parser::GetTextAttribsImpl(typename EncodingPolicy::EncodedCharPtr pszSrc, size_t cEncoded,
                               SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr)
{
    AssertArrMemR(pszSrc, cch);
    AssertArrMem(prgsta, cch);

    EncodingPolicy::EncodedChar*    psz = NULL;

    if (NULL == pszSrc || NULL == prgsta || 0 == cch)
        return NOERROR;

    // this avoids an overflow such as when cch == (int)-1
    if ( cch > INT_MAX)
        return E_FAIL ;

    if (pszSrc[cEncoded - 1] != 0)
    {
        // We have to duplicate the text.
        psz = (EncodingPolicy::EncodedChar *)malloc((cEncoded + 1) * sizeof(EncodingPolicy::EncodedChar));
        IFNULLMEMRET(psz);
        js_memcpy_s(psz, (cEncoded + 1) * sizeof(EncodingPolicy::EncodedChar), pszSrc, cEncoded * sizeof(EncodingPolicy::EncodedChar));
        psz[cEncoded] = 0;
        pszSrc = psz;
    }
    memset(prgsta, 0, sizeof(prgsta[0]) * cch);

    DebugOnly( m_err.fInited = TRUE; )

    Scanner<EncodingPolicy> *scanner = null;
#if ERROR_RECOVERY
    m_err.m_callback = NULL;
#endif
    try
    {
        // Create the hash table.
        if (NULL == (m_phtbl = HashTbl::Create(HASH_TABLE_SIZE, &m_err)))
            Error(ERRnoMemory);

        // Create the scanner.

        if (NULL == (scanner = Scanner<EncodingPolicy>::Create(this, m_phtbl, &m_token, &m_err, m_scriptContext)))
            Error(ERRnoMemory);

        // Give the scanner the source.
        scanner->SetText(pszSrc, 0, cch, 0, grfscr | fscrSyntaxColor);

#if DEBUG
        if (-1 != g_dwTextAttribFlags)
            dwFlags = g_dwTextAttribFlags;
#endif // DEBUG

        GetTextAttribsImpl(scanner, pszSrc, dwFlags, prgsta, cch);
    }
    catch(ParseExceptionObject& e)
    {
      m_err.m_hr = e.GetError();
    }
    RELEASEPTR(scanner);
    FREEPTR(psz);
    return m_err.m_hr;
}

template <typename EncodingPolicy>
void Parser::GetTextAttribsImpl(Scanner< EncodingPolicy > *scanner, typename EncodingPolicy::EncodedCharPtr pstr, DWORD dwFlags,
                                  __ecount(cch) SOURCE_TEXT_ATTR *prgsta, ulong cch)
{
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        switch (dwFlags & 0xFF)
        {
        case GETATTRTYPE_NORMAL:
            GetNormalTextAttribs(scanner, pstr, dwFlags, prgsta, cch);
            break;
        case GETATTRTYPE_DEPSCAN:
            GetDepScanTextAttribs(scanner, pstr, dwFlags, prgsta, cch);
            break;
        default:
            // Don't understand flags -> no attributes assigned.
            break;
        }
        m_err.m_hr = NOERROR;
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(m_err.m_hr);
}

template <typename EncodingPolicy>
void Parser::GetNormalTextAttribs(Scanner< EncodingPolicy > *scanner, typename EncodingPolicy::EncodedCharPtr pstr, DWORD dwFlags,
                                  __out_ecount(cch) SOURCE_TEXT_ATTR *prgsta, ulong cch)
{
    // Scan the tokens gathering attrib information.
    SOURCE_TEXT_ATTR sta;
    charcount_t istaMin, istaLim;
    BOOL fSetHumanText = (0 != (dwFlags & GETATTRFLAG_HUMANTEXT));
    tokens prevtk = tkNone; // track previous token for regex's
    while (scanner->Scan() != tkEOF)
    {
        sta = 0;
        switch (m_token.tk)
        {

        case tkComment:
            sta = SOURCETEXT_ATTR_COMMENT;
            break;

        case tkIntCon:
        case tkFltCon:

            sta = SOURCETEXT_ATTR_NUMBER;
            break;

        case tkStrCon:
            sta = SOURCETEXT_ATTR_STRING;
            break;

        case tkID:
            sta = SOURCETEXT_ATTR_IDENTIFIER;
            break;

        case tkDiv:
        case tkAsgDiv:
            // differentiate between div and regex by checking
            // for characters that cannot precede a regex
            switch(prevtk)
            {
            case tkID: case tkStrCon: case tkIntCon: case tkFltCon:
            case tkRegExp: case tkTHIS: case tkSUPER: case tkInc: case tkDec:
            case tkRParen: case tkRBrack: case tkRCurly:
            case tkTRUE: case tkFALSE:
                sta = SOURCETEXT_ATTR_OPERATOR;
                break;

            default:
                if(scanner->RescanRegExpNoAST() != tkScanError)
                    sta = SOURCETEXT_ATTR_STRING;
                else
                    sta = SOURCETEXT_ATTR_OPERATOR;
                break;
            }
            break;

        default:
            if (m_token.IsOperator())
                sta = SOURCETEXT_ATTR_OPERATOR;
            else if (m_token.IsReservedWord())
            {
                // dot followed by keyword might be an ecmascript5 identifier
                if (prevtk == tkDot)
                {
                    m_token.tk = tkID;
                    sta = SOURCETEXT_ATTR_IDENTIFIER;
                }
                else
                {
                    sta = SOURCETEXT_ATTR_KEYWORD;
                    if (dwFlags & GETATTRFLAG_THIS && tkTHIS == m_token.tk)
                        sta |= SOURCETEXT_ATTR_THIS;
                }
            }
            break;
        }

        if (sta != 0)
        {
            istaMin = scanner->IchMinTok();
            istaLim = scanner->IchLimTok();
            Assert(istaLim >= istaMin);
            AssertArrMem(prgsta, istaLim);
            while (istaMin < istaLim && istaLim <= cch)
            {
                prgsta[istaMin++] = sta;
            }
        }

        if (fSetHumanText)
            SetHumanTextForCurrentToken<EncodingPolicy>(scanner, pstr, prgsta);

        prevtk = m_token.tk;
    }
}

template <typename EncodingPolicy>
void Parser::GetDepScanTextAttribs(Scanner< EncodingPolicy >* scanner, typename EncodingPolicy::EncodedCharPtr pstr, DWORD dwFlags,
                                   __out_ecount(cch) SOURCE_TEXT_ATTR *prgsta, ulong cch)
{
    // Scan the tokens gathering attrib information.
    SOURCE_TEXT_ATTR sta;
    charcount_t istaMin, istaLim;
    BOOL fSetHumanText = (0 != (dwFlags & GETATTRFLAG_HUMANTEXT));
    while (scanner->Scan() != tkEOF)
    {
        switch (m_token.tk)
        {
        case tkDot:
            sta = SOURCETEXT_ATTR_MEMBERLOOKUP;
            break;
        case tkID:
            sta = SOURCETEXT_ATTR_IDENTIFIER;
            break;
        case tkTHIS:
            if (dwFlags & GETATTRFLAG_THIS)
            {
                sta = SOURCETEXT_ATTR_THIS;
                break;
            }
            goto LSetHumanText;

        default:
            // Skip - not an interesting token.
            goto LSetHumanText;
        }

#if DEBUG
        if (g_fMapAttrib)
        {
            // Map to coler indexes we can see.
            if (SOURCETEXT_ATTR_MEMBERLOOKUP == sta)
                sta = SOURCETEXT_ATTR_KEYWORD;
            else if (SOURCETEXT_ATTR_IDENTIFIER == sta)
                sta = SOURCETEXT_ATTR_COMMENT;
        }
#endif // DEBUG
        istaMin = scanner->IchMinTok();
        istaLim = scanner->IchLimTok();
        Assert(istaLim >= istaMin);
        AssertArrMem(prgsta, istaLim);
        while (istaMin < istaLim && istaLim <= cch)
        {
            prgsta[istaMin++] = sta;
        }

LSetHumanText:
        if (fSetHumanText)
            SetHumanTextForCurrentToken<EncodingPolicy>(scanner, pstr, prgsta);
    }
}

template <typename EncodingPolicy>
void Parser::SetHumanTextForCurrentToken(Scanner< EncodingPolicy >* scanner, typename EncodingPolicy::EncodedCharPtr pstr, SOURCE_TEXT_ATTR *prgsta)
{
    // Scan the tokens gathering attrib information.
    charcount_t istaMin, istaLim;
    size_t iuMin, iuLim;
    switch (m_token.tk)
    {
    case tkComment:
        // The JS comment can be a single line comment or a multiline
        // comment. The second character of the comment will determine.
        istaMin = scanner->IchMinTok()+2; // Both types start with two char
        istaLim = scanner->IchLimTok();
        iuMin = scanner->IecpMinTok()+2;
        iuLim = scanner->IecpLimTok();
        Assert('/' == pstr[iuMin-2]);
        if ('*' == pstr[iuMin-1])
        {
            // Multiline comment. This may not be terminated - if for
            // instance the comment ended at an EOF. We adjust the
            // end of the token only if the end comment matches.
            if ('/' == pstr[iuLim-1] && '*' == pstr[iuLim-2])
                istaLim -= 2;
        }
        else
        {
            // A single line comment.
            Assert('/' == pstr[iuMin-1]);
        }
        break;

    case tkStrCon:
        // Strip the strings quotes.
        istaMin = m_pscan->IchMinTok()+1;
        istaLim = m_pscan->IchLimTok()-1;
        break;

    default:
        // Skip - not an interesting token.
        return;
    }

    Assert(istaLim >= istaMin);
    AssertArrMem(prgsta, istaLim);
    while (istaMin < istaLim)
        prgsta[istaMin++] |= SOURCETEXT_ATTR_HUMANTEXT;
}

HRESULT Parser::ValidateSyntax(LPCUTF8 pszSrc, size_t encodedCharCount, bool isGenerator, CompileScriptException *pse, void (Parser::*validateFunction)())
{
    AssertPsz(pszSrc);
    AssertMemN(pse);

    if (this->IsBackgroundParser())
    {
        PROBE_STACK_NO_DISPOSE(m_scriptContext, Js::Constants::MinStackDefault);
    }
    else
    {
        PROBE_STACK(m_scriptContext, Js::Constants::MinStackDefault);
    }

    HRESULT hr;
    SmartFPUControl smartFpuControl;

    DebugOnly( m_err.fInited = TRUE; )
    BOOL fDeferSave = m_deferringAST;
#if ERROR_RECOVERY
    m_err.m_callback = NULL;
#endif
    try
    {
        hr = NOERROR;

        this->PrepareScanner(false);

        m_length = encodedCharCount;
        m_originalLength = encodedCharCount;

        // make sure deferred parsing is turned off
        ULONG grfscr = fscrNil;

        // Give the scanner the source and get the first token
        m_pscan->SetText(pszSrc, 0, encodedCharCount, 0, grfscr);
        m_pscan->SetYieldIsKeyword(isGenerator);
        m_pscan->Scan();

        uint nestedCount = 0;
        m_pnestedCount = &nestedCount;

        ParseNodePtr pnodeScope = nullptr;
        m_ppnodeScope = &pnodeScope;
        m_ppnodeExprScope = nullptr;

        uint nextFunctionId = 0;
        m_nextFunctionId = &nextFunctionId;

        m_inDeferredNestedFunc = false;
        m_deferringAST = true;

        if (m_scriptContext->authoringData != nullptr)
        {
            // If authoringData is set this is a deferred parse request in the context of the language serivce, start
            // parsing as if in statement completion mode.
            m_grfscr |= fscrStmtCompletion;
        }

        m_nextBlockId = 0;
        if (this->BindDeferredPidRefs())
        {            
            ParseNode *pnodeFnc = CreateNode(knopFncDecl);
            pnodeFnc->sxFnc.ClearFlags();
            pnodeFnc->sxFnc.SetDeclaration(false);
            pnodeFnc->sxFnc.astSize    = 0;
            pnodeFnc->sxFnc.pnodeVars  = nullptr;
            pnodeFnc->sxFnc.pnodeArgs  = nullptr;
            pnodeFnc->sxFnc.pnodeBody  = nullptr;
            pnodeFnc->sxFnc.pnodeNames = nullptr;
            pnodeFnc->sxFnc.pnodeRest  = nullptr;
            pnodeFnc->sxFnc.deferredStub = nullptr;
            pnodeFnc->sxFnc.SetIsGenerator(isGenerator);
            m_ppnodeVar = &pnodeFnc->sxFnc.pnodeVars;
            m_currentNodeFunc = pnodeFnc;
            m_currentNodeDeferredFunc = NULL;
            AssertMsg(m_pstmtCur == NULL, "Statement stack should be empty when we start parse function body");

            ParseNodePtr block = StartParseBlock<false>(PnodeBlockType::Function, ScopeType_FunctionBody);
            (this->*validateFunction)();
            FinishParseBlock(block);

            pnodeFnc->ichLim = m_pscan->IchLimTok();
            pnodeFnc->sxFnc.cbLim = m_pscan->IecpLimTok();
            pnodeFnc->sxFnc.pnodeVars = nullptr;

            if (m_asgToConst)
            {
                Error(ERRAssignmentToConst, m_asgToConst.GetIchMin(), m_asgToConst.GetIchLim());
            }
        }
        else
        {
            (this->*validateFunction)();
        }
        // there should be nothing after successful parsing for a given construct
        if (m_token.tk != tkEOF)
            Error(ERRsyntax);

        RELEASEPTR(m_pscan);
        m_deferringAST = fDeferSave;
    }
    catch(ParseExceptionObject& e)
    {
        m_deferringAST = fDeferSave;
        m_err.m_hr = e.GetError();
        hr = pse->ProcessError( m_pscan,  m_err.m_hr, /* pnodeBase */ NULL);
    }

    return hr;
}

#if ERROR_RECOVERY
void errorHandler(void *data, HRESULT hr)
{
    if (data)
    {
        Parser *parser = static_cast<Parser *>(data);
        parser->ReportError(hr);
    }
}

void Parser::ReportError(HRESULT hr)
{
    if (m_errorCallback)
    {
        m_errorCallback(m_errorCallbackData, m_pscan->IchMinError(), m_pscan->IchLimError() - m_pscan->IchMinError(), hr);
        m_pscan->SetErrorPosition(0, 0);
    }
}
#endif

HRESULT Parser::ParseSourceInternal(
    __out ParseNodePtr* parseTree, LPCUTF8 pszSrc, size_t offsetInBytes, size_t encodedCharCount, charcount_t offsetInChars,
    bool fromExternal, ULONG grfscr, CompileScriptException *pse, Js::LocalFunctionId * nextFunctionId, ULONG lineNumber, SourceContextInfo * sourceContextInfo)
{
    AssertMem(parseTree);
    AssertPsz(pszSrc);
    AssertMemN(pse);

    double startTime = m_scriptContext->GetThreadContext()->ParserTelemetry.Now();

    if (this->IsBackgroundParser())
    {
        PROBE_STACK_NO_DISPOSE(m_scriptContext, Js::Constants::MinStackDefault);
    }
    else
    {
        PROBE_STACK(m_scriptContext, Js::Constants::MinStackDefault);
    }

#ifdef PROFILE_EXEC
    m_scriptContext->ProfileBegin(Js::ParsePhase);
#endif
    JSETW(EventWriteJSCRIPT_PARSE_START(m_scriptContext,0));

    *parseTree = NULL;
    m_sourceLim = 0;

    m_grfscr = grfscr;
    m_sourceContextInfo = sourceContextInfo;

    ParseNodePtr pnodeBase = NULL;
    HRESULT hr;
    SmartFPUControl smartFpuControl;

#if ECMACP
    m_fECMACP = pos->IsECMACP();
#endif // ECMACP

    DebugOnly( m_err.fInited = TRUE; )

#if ERROR_RECOVERY
    if (LanguageServiceMode() && m_errorCallback)
    {
        m_err.m_data = this;
        m_err.m_callback = errorHandler;
    }
    else m_err.m_callback = null;
#endif

    try
    {
        this->PrepareScanner(fromExternal);

#if ERROR_RECOVERY
        // If there is a comment callback tell the scanner.
        if (m_commentCallback)
            m_pscan->SetCommentCallback(m_commentCallback, m_commentCallbackData);
#endif

        // parse the source
        pnodeBase = Parse(pszSrc, offsetInBytes, encodedCharCount, offsetInChars, grfscr, lineNumber, nextFunctionId, pse);

        AssertNodeMem(pnodeBase);

        // Record the actual number of words parsed.
        m_sourceLim = pnodeBase->ichLim - offsetInChars;

        // TODO: The assert can be false possitive in some scenarios and chuckj to fix it later
        // Assert(utf8::ByteIndexIntoCharacterIndex(pszSrc + offsetInBytes, encodedCharCount, fromExternal ? utf8::doDefault : utf8::doAllowThreeByteSurrogates) == m_sourceLim);

#if DBG_DUMP
        if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::ParsePhase))
        {
            PrintPnodeWIndent(pnodeBase,4);
            fflush(stdout);
        }
#endif

        *parseTree = pnodeBase;

        hr = NOERROR;
    }
    catch(ParseExceptionObject& e)
    {
#if PARSENODE_EXTENSIONS
        // Clearing the node allocator invalidates the language service extensions
        m_languageServiceExtension = NULL;
#endif

        m_err.m_hr = e.GetError();
        hr = pse->ProcessError( m_pscan, m_err.m_hr, pnodeBase);
#if LANGUAGE_SERVICE
        // In the language service we can only get here as a result of a syntax error and we are
        // not performing error recovery. This only happens during eval() during execution. This
        // ensures that we restore the execution state since execution will start again once the
        // exception is thrown.
        if (m_scriptContext->authoringData && m_scriptContext->authoringData->Callbacks())
            m_scriptContext->authoringData->Callbacks()->Executing();
#endif
    }

    if (this->m_hasParallelJob)
    {
        ///// Wait here for remaining jobs to finish. Then look for errors, do final const bindings.
        // pleath TODO: If there are remaining jobs, let the main thread help finish them.
        BackgroundParser *bgp = m_scriptContext->GetBackgroundParser();
        Assert(bgp);

        CompileScriptException se;
        this->WaitForBackgroundJobs(bgp, &se);

        BackgroundParseItem *failedItem = bgp->GetFailedBackgroundParseItem();
        if (failedItem)
        {
            CompileScriptException *bgPse = failedItem->GetPSE();
            Assert(bgPse);
            *pse = *bgPse;
            hr = failedItem->GetHR();
            bgp->SetFailedBackgroundParseItem(nullptr);
        }

        if (this->fastScannedRegExpNodes != nullptr)
        {
            this->FinishBackgroundRegExpNodes();
        }

        for (BackgroundParseItem *item = this->backgroundParseItems; item; item = item->GetNext())
        {
            Parser *parser = item->GetParser();
            parser->FinishBackgroundPidRefs(item, this != parser);
        }
    }

#if ERROR_RECOVERY
    if ( !LanguageServiceMode())
    {
#endif
        // done with the scanner
        RELEASEPTR(m_pscan);
#if ERROR_RECOVERY
    }
#endif

#ifdef PROFILE_EXEC
    m_scriptContext->ProfileEnd(Js::ParsePhase);
#endif
    JSETW(EventWriteJSCRIPT_PARSE_STOP(m_scriptContext, 0));

    ThreadContext *threadContext = m_scriptContext->GetThreadContext();
    threadContext->ParserTelemetry.LogTime(threadContext->ParserTelemetry.Now() - startTime);

    return hr;
}

void Parser::WaitForBackgroundJobs(BackgroundParser *bgp, CompileScriptException *pse)
{
    // The scan of the script is done, but there may be unfinished background jobs in the queue.
    // Enlist the main thread to help with those.
    BackgroundParseItem *item;
    if (!*bgp->GetPendingBackgroundItemsPtr())
    {
        // We're done.
        return;
    }

    // Save parser state, since we'll need to restore it in order to bind references correctly later.
    this->m_isInBackground = true;
    this->SetCurrBackgroundParseItem(nullptr);
    uint blockIdSave = this->m_nextBlockId;
    uint functionIdSave = *this->m_nextFunctionId;
    StmtNest *pstmtSave = this->m_pstmtCur;

    if (!bgp->Processor()->ProcessesInBackground())
    {
        // No background thread. Just walk the jobs with no locking and process them.
        for (item = bgp->GetNextUnprocessedItem(); item; item = bgp->GetNextUnprocessedItem())
        {
            bgp->Processor()->RemoveJob(item);
            bool succeeded = bgp->Process(item, this, pse);
            bgp->JobProcessed(item, succeeded);
        }
        Assert(!*bgp->GetPendingBackgroundItemsPtr());
    }
    else
    {
        // Background threads. We need to have the critical section in order to:
        // - Check for unprocessed jobs;
        // - Remove jobs from the processor queue;
        // - Do JobsProcessed work (such as removing jobs from the BackgroundParser's unprocessed list).
        CriticalSection *pcs = static_cast<JsUtil::BackgroundJobProcessor*>(bgp->Processor())->GetCriticalSection();
        pcs->Enter();
        for (;;)
        {
            // Grab a job (in lock)
            item = bgp->GetNextUnprocessedItem();
            if (item == nullptr)
            {
                break;
            }
            bgp->Processor()->RemoveJob(item);
            pcs->Leave();

            // Process job (if there is one) (outside lock)
            bool succeeded = bgp->Process(item, this, pse);

            pcs->Enter();
            bgp->JobProcessed(item, succeeded);
        }
        pcs->Leave();

        // Wait for the background threads to finish jobs they're already processing (if any).
        // TODO: Replace with a proper semaphore.
        while(*bgp->GetPendingBackgroundItemsPtr());
    }

    Assert(!*bgp->GetPendingBackgroundItemsPtr());

    // Restore parser state.
    this->m_pstmtCur = pstmtSave;
    this->m_isInBackground = false;
    this->m_nextBlockId = blockIdSave;
    *this->m_nextFunctionId = functionIdSave;
}

void Parser::FinishBackgroundPidRefs(BackgroundParseItem *item, bool isOtherParser)
{
    for (BlockInfoStack *blockInfo = item->GetParseContext()->currentBlockInfo; blockInfo; blockInfo = blockInfo->pBlockInfoOuter)
    {
        if (isOtherParser)
        {
            this->BindPidRefs<true>(blockInfo, item->GetMaxBlockId());
        }
        else
        {
            this->BindPidRefs<false>(blockInfo, item->GetMaxBlockId());
        }
    }
}

void Parser::FinishBackgroundRegExpNodes()
{
    // We have a list of regexp nodes that we saw on the UI thread in functions we're parallel parsing,
    // and for each background job we have a list of regexp nodes for which we couldn't allocate patterns.
    // We need to copy the pattern pointers from the UI thread nodes to the corresponding nodes on the
    // background nodes.
    // There may be UI thread nodes for which there are no background thread equivalents, because the UI thread
    // has to assume that the background thread won't defer anything.

    // Note that because these lists (and the list of background jobs) are SList's built by prepending, they are
    // all in reverse lexical order.

    Assert(!this->IsBackgroundParser());
    Assert(this->fastScannedRegExpNodes);
    Assert(this->backgroundParseItems != nullptr);

    BackgroundParseItem *currBackgroundItem;

#if DBG
    for (currBackgroundItem = this->backgroundParseItems;
         currBackgroundItem;
         currBackgroundItem = currBackgroundItem->GetNext())
    {
        if (currBackgroundItem->RegExpNodeList())
        {
            FOREACH_DLIST_ENTRY(ParseNodePtr, ArenaAllocator, pnode, currBackgroundItem->RegExpNodeList())
            {
                Assert(pnode->sxPid.regexPattern == nullptr);
            }
            NEXT_DLIST_ENTRY;
        }
    }
#endif

    // Hook up the patterns allocated on the main thread to the nodes created on the background thread.
    // Walk the list of foreground nodes, advancing through the work items and looking up each item.
    // Note that the background thread may have chosen to defer a given regex literal, so not every foreground
    // node will have a matching background node. Doesn't matter for correctness.
    // (It's inefficient, of course, to have to restart the inner loop from the beginning of the work item's
    // list, but it should be unusual to have many regexes in a single work item's chunk of code. Figure out how
    // to start the inner loop from a known internal node within the list if that turns out to be important.)
    currBackgroundItem = this->backgroundParseItems;
    FOREACH_DLIST_ENTRY(ParseNodePtr, ArenaAllocator, pnodeFgnd, this->fastScannedRegExpNodes)
    {
        Assert(pnodeFgnd->nop == knopRegExp);
        Assert(pnodeFgnd->sxPid.regexPattern != nullptr);
        bool quit = false;

        while (!quit)
        {
            // Find the next work item with a regex in it.
            while (currBackgroundItem && currBackgroundItem->RegExpNodeList() == nullptr)
            {
                currBackgroundItem = currBackgroundItem->GetNext();
            }
            if (!currBackgroundItem)
            {
                break;
            }

            // Walk the regexes in the work item.
            FOREACH_DLIST_ENTRY(ParseNodePtr, ArenaAllocator, pnodeBgnd, currBackgroundItem->RegExpNodeList())
            {
                Assert(pnodeBgnd->nop == knopRegExp);

                if (pnodeFgnd->ichMin <= pnodeBgnd->ichMin)
                {
                    // Either we found a match, or the next background node is past the foreground node.
                    // In any case, we can stop searching.
                    if (pnodeFgnd->ichMin == pnodeBgnd->ichMin)
                    {
                        Assert(pnodeFgnd->ichLim == pnodeBgnd->ichLim);
                        pnodeBgnd->sxPid.regexPattern = pnodeFgnd->sxPid.regexPattern;
                    }
                    quit = true;
                    break;
                }
            }
            NEXT_DLIST_ENTRY;

            if (!quit)
            {
                // Need to advance to the next work item.
                currBackgroundItem = currBackgroundItem->GetNext();
            }
        }
    }
    NEXT_DLIST_ENTRY;

#if DBG
    for (currBackgroundItem = this->backgroundParseItems;
         currBackgroundItem;
         currBackgroundItem = currBackgroundItem->GetNext())
    {
        if (currBackgroundItem->RegExpNodeList())
        {
            FOREACH_DLIST_ENTRY(ParseNodePtr, ArenaAllocator, pnode, currBackgroundItem->RegExpNodeList())
            {
                Assert(pnode->sxPid.regexPattern != nullptr);
            }
            NEXT_DLIST_ENTRY;
        }
    }
#endif
}

LabelId* Parser::CreateLabelId(IdentToken* pToken)
{
    LabelId* pLabelId;

    pLabelId = (LabelId*)m_nodeAllocator.Alloc(sizeof(LabelId));
    if (NULL == pLabelId)
        Error(ERRnoMemory);
    pLabelId->pid = pToken->pid;
    pLabelId->next = NULL;

    return pLabelId;
}

/*****************************************************************************
The following set of routines allocate parse tree nodes of various kinds.
They catch an exception on out of mem.
*****************************************************************************/
static const int g_mpnopcbNode[] =
{
#define PTNODE(nop,sn,pc,nk,ok,json,apnk) kcbPn##nk,
#include "ptlist.h"
};

const Js::RegSlot NoRegister = (Js::RegSlot)-1;
const Js::RegSlot OneByteRegister = (Js::RegSlot_OneByte)-1;

void Parser::InitNode(OpCode nop,ParseNodePtr pnode) {
    pnode->nop = nop;
    pnode->grfpn = PNodeFlags::fpnNone;
    pnode->location = NoRegister;
    pnode->emitLabels = false;
    pnode->isUsed = true;
    pnode->notEscapedUse = false;
    pnode->isInList = false;
    pnode->isCallApplyTargetLoad = false;
}

// Create nodes using Arena
template <OpCode nop>
ParseNodePtr Parser::StaticCreateNodeT(ArenaAllocator* alloc, charcount_t ichMin, charcount_t ichLim)
{
    ParseNodePtr pnode = StaticAllocNode<nop>(alloc);
    InitNode(nop,pnode);
    // default - may be changed
    pnode->ichMin = ichMin;
    pnode->ichLim = ichLim;

    return pnode;
}

ParseNodePtr
Parser::StaticCreateBlockNode(ArenaAllocator* alloc, charcount_t ichMin , charcount_t ichLim, int blockId, PnodeBlockType blockType)
{
    ParseNodePtr pnode = StaticCreateNodeT<knopBlock>(alloc, ichMin, ichLim);
    InitBlockNode(pnode, blockId, blockType);
    return pnode;
}

void Parser::InitBlockNode(ParseNodePtr pnode, int blockId, PnodeBlockType blockType)
{
    Assert(pnode->nop == knopBlock);
    pnode->sxBlock.pnodeScopes = nullptr;
    pnode->sxBlock.pnodeNext = nullptr;
    pnode->sxBlock.scope = nullptr;
    pnode->sxBlock.enclosingBlock = nullptr;
    pnode->sxBlock.pnodeLexVars = nullptr;
    pnode->sxBlock.pnodeStmt = nullptr;
    pnode->sxBlock.pnodeLastValStmt = nullptr;

    pnode->sxBlock.callsEval = false;
    pnode->sxBlock.childCallsEval = false;
    pnode->sxBlock.blockType = blockType;
    pnode->sxBlock.blockId = blockId;

    if (blockType != PnodeBlockType::Regular)
    {
        pnode->grfpn |= PNodeFlags::fpnSyntheticNode;
    }
}

// Create Node with limit
template <OpCode nop>
ParseNodePtr Parser::CreateNodeT(charcount_t ichMin,charcount_t ichLim)
{
    Assert(!this->m_deferringAST);
    ParseNodePtr pnode = StaticCreateNodeT<nop>(&m_nodeAllocator, ichMin, ichLim);

    Assert(m_pCurrentAstSize != NULL);
    *m_pCurrentAstSize += GetNodeSize<nop>();

    return pnode;
}

ParseNodePtr Parser::CreateDeclNode(OpCode nop, IdentPtr pid, SymbolType symbolType, bool errorOnRedecl)
{
    ParseNodePtr pnode = CreateNode(nop);

    pnode->sxVar.InitDeclNode(pid, NULL);

    if (symbolType != STUnknown)
    {
        pnode->sxVar.sym = AddDeclForPid(pnode, pid, symbolType, errorOnRedecl);
    }

    return pnode;
}

Symbol* Parser::AddDeclForPid(ParseNodePtr pnode, IdentPtr pid, SymbolType symbolType, bool errorOnRedecl)
{
    Assert(pnode->nop == knopLetDecl || pnode->nop == knopConstDecl || pnode->nop == knopVarDecl);

    PidRefStack *refForUse = nullptr, *refForDecl = nullptr;

    BlockInfoStack *blockInfo;
    bool fBlockScope = false;
    if (m_scriptContext->GetConfig()->IsBlockScopeEnabled() &&
        (pnode->nop != knopVarDecl || symbolType == STFunction))
    {
        Assert(m_pstmtCur);
        if (m_pstmtCur->isDeferred)
        {
            // Deferred parsing: there's no pnodeStmt node, only an opcode on the Stmt struct.
            if (m_pstmtCur->op != knopBlock)
            {
                // Let/const declared in a bare statement context.
                Error(ERRDeclOutOfStmt);
            }

            if (m_pstmtCur->pstmtOuter && m_pstmtCur->pstmtOuter->op == knopSwitch)
            {
                // Let/const declared inside a switch block (requiring conservative use-before-decl check).
                pnode->sxVar.isSwitchStmtDecl = true;
            }
        }
        else
        {
            if (m_pstmtCur->pnodeStmt->nop != knopBlock)
            {
                // Let/const declared in a bare statement context.
                Error(ERRDeclOutOfStmt);
            }

            if (m_pstmtCur->pstmtOuter && m_pstmtCur->pstmtOuter->pnodeStmt->nop == knopSwitch)
            {
                // Let/const declared inside a switch block (requiring conservative use-before-decl check).
                pnode->sxVar.isSwitchStmtDecl = true;
            }
        }

        fBlockScope = pnode->nop != knopVarDecl ||
            (
                !GetCurrentBlockInfo()->pnodeBlock->sxBlock.scope ||
                GetCurrentBlockInfo()->pnodeBlock->sxBlock.scope->GetScopeType() != ScopeType_GlobalEvalBlock
                );
    }
    if (fBlockScope)
    {
        blockInfo = GetCurrentBlockInfo();
    }
    else
    {
        blockInfo = GetCurrentFunctionBlockInfo();
    }

    // If we are creating an 'arguments' sym at function block scope, create it in
    // the parameter scope instead. That way, if we need to reuse the sym for the
    // actual arguments object at the end of the function, we don't need to move it
    // into the parameter scope.
    if (pid == wellKnownPropertyPids.arguments
        && pnode->nop == knopVarDecl
        && blockInfo->pnodeBlock->sxBlock.blockType == PnodeBlockType::Function
        && blockInfo->pBlockInfoOuter != nullptr
        && blockInfo->pBlockInfoOuter->pnodeBlock->sxBlock.blockType == PnodeBlockType::Parameter)
    {
        blockInfo = blockInfo->pBlockInfoOuter;
    }

    int maxScopeId = blockInfo->pnodeBlock->sxBlock.blockId;

    if (blockInfo->pnodeBlock->sxBlock.scope != nullptr && blockInfo->pnodeBlock->sxBlock.scope->GetScopeType() == ScopeType_FunctionBody)
    {
        // Check if there is a parameter scope and try to get it first.
        BlockInfoStack *outerBlockInfo = blockInfo->pBlockInfoOuter;
        if (outerBlockInfo != nullptr && outerBlockInfo->pnodeBlock->sxBlock.blockType == PnodeBlockType::Parameter)
        {
            maxScopeId = outerBlockInfo->pnodeBlock->sxBlock.blockId;
        }
    }

    refForDecl = this->FindOrAddPidRef(pid, blockInfo->pnodeBlock->sxBlock.blockId, maxScopeId);

    if (refForDecl == nullptr)
    {
        Error(ERRnoMemory);
    }
    if (blockInfo == GetCurrentBlockInfo())
    {
        refForUse = refForDecl;
    }
    else
    {
        refForUse = this->PushPidRef(pid);
    }
    pnode->sxVar.symRef = refForUse->GetSymRef();
    Symbol *sym = refForDecl->GetSym();
    if (sym != null)
    {
        // Multiple declarations in the same scope. 3 possibilities: error, existing one wins, new one wins.
        switch (pnode->nop)
        {
        case knopLetDecl:
        case knopConstDecl:
            if (!sym->GetDecl()->sxVar.isBlockScopeFncDeclVar)
            {
                Assert(errorOnRedecl);
                // Redeclaration error.
                Error(ERRRedeclaration);
            }
            else
            {
                // (New) let/const hides the (old) var
                sym->SetSymbolType(symbolType);
                sym->SetDecl(pnode);
            }
            break;
        case knopVarDecl:
            if (sym->GetDecl() == null)
            {
                Assert(symbolType == STFunction);
                sym->SetDecl(pnode);
                break;
            }
            switch (sym->GetDecl()->nop)
            {
            case knopLetDecl:
            case knopConstDecl:
                if (errorOnRedecl)
                {
                    Error(ERRRedeclaration);
                }
                // If !errorOnRedecl, (old) let/const hides the (new) var, so do nothing.
                break;
            case knopVarDecl:
                // Legal redeclaration. Who wins?
                if (errorOnRedecl || sym->GetDecl()->sxVar.isBlockScopeFncDeclVar)
                {
                    if (symbolType == STFormal ||
                        (symbolType == STFunction && sym->GetSymbolType() != STFormal) ||
                        sym->GetSymbolType() == STVariable)
                    {
                        // New decl wins.
                        sym->SetSymbolType(symbolType);
                        sym->SetDecl(pnode);
                    }
                }
                break;
            }
            break;
        }
    }
    else
    {
        Scope *scope = blockInfo->pnodeBlock->sxBlock.scope;
        if (scope == nullptr)
        {
            Assert(blockInfo->pnodeBlock->sxBlock.blockType == PnodeBlockType::Regular &&
                   m_scriptContext->GetConfig()->IsBlockScopeEnabled());
            scope = Anew(&m_nodeAllocator, Scope, &m_nodeAllocator, ScopeType_Block);
            blockInfo->pnodeBlock->sxBlock.scope = scope;
            PushScope(scope);
        }

        if (scope->GetScopeType() == ScopeType_GlobalEvalBlock)
        {
            Assert(fBlockScope);
            Assert(scope->GetEnclosingScope() == m_currentNodeProg->sxProg.scope);
            // Check for same-named decl in Global scope.
            PidRefStack *pidRefOld = pid->GetPidRefForScopeId(0);
            if (pidRefOld && pidRefOld->GetSym())
            {
                Error(ERRRedeclaration);
            }
        }
        else if (scope->GetScopeType() == ScopeType_Global && (this->m_grfscr & fscrEvalCode) &&
                 !(m_functionBody && m_functionBody->GetScopeInfo()))
        {
            // Check for same-named decl in GlobalEvalBlock scope. Note that this is not necessary
            // if we're compiling a deferred nested function and the global scope was restored from cached info,
            // because in that case we don't need a GlobalEvalScope.
            Assert(!fBlockScope);
            PidRefStack *pidRefOld = pid->GetPidRefForScopeId(1);
            if (pidRefOld && pidRefOld->GetSym())
            {
                Error(ERRRedeclaration);
            }
        }

        if ((scope->GetScopeType() == ScopeType_FunctionBody || scope->GetScopeType() == ScopeType_Parameter) && symbolType != STFunction)
        {
            ParseNodePtr pnodeFnc = GetCurrentFunctionNode();
            Assert(pnodeFnc);
            if (pnodeFnc->sxFnc.pnodeNames &&
                pnodeFnc->sxFnc.pnodeNames->nop == knopVarDecl &&
                pnodeFnc->sxFnc.pnodeNames->sxVar.pid == pid)
            {
                // Named function expression has its name hidden by a local declaration.
                // This is important to know if we don't know whether nested deferred functions refer to it,
                // because if the name has a non-local reference then we have to create a scope object.
                m_currentNodeFunc->sxFnc.SetNameIsHidden();
            }
        }

        if (!sym)
        {
            const wchar_t *name = reinterpret_cast<const wchar_t*>(pid->Psz());
            int nameLength = pid->Cch();
            SymbolName const symName(name, nameLength);

            Assert(!scope->FindLocalSymbol(symName));
            sym = Anew(&m_nodeAllocator, Symbol, symName, pnode, symbolType);
            scope->AddNewSymbol(sym);
            sym->SetPid(pid);
        }
        refForDecl->SetSym(sym);
    }
    return sym;
}

void Parser::RestorePidRefForSym(Symbol *sym)
{
    IdentPtr pid = m_pscan->m_phtbl->PidHashNameLen(sym->GetName().GetBuffer(), sym->GetName().GetLength());
    Assert(pid);
    sym->SetPid(pid);
    PidRefStack *ref = this->PushPidRef(pid);
    ref->SetSym(sym);
}

IdentPtr Parser::GenerateIdentPtr(wchar_t* name,long len) {
  return m_phtbl->PidHashNameLen(name,len);
}

/*static*/
LPCOLESTR Parser::GetClassName(PnClass *pClass)
{
    Assert(pClass != nullptr);
    AssertMsg(pClass->pnodeConstructor != nullptr, "Every class should have ctor");
    if (pClass->pnodeConstructor != nullptr)
    {
        Assert(pClass->pnodeConstructor->nop == knopFncDecl);
        if (pClass->pnodeConstructor->sxFnc.hint != nullptr)
        {
            return pClass->pnodeConstructor->sxFnc.hint;
        }
        else if (pClass->pnodeConstructor->sxFnc.pid != nullptr)
        {
            return pClass->pnodeConstructor->sxFnc.pid->Psz();
        }
        else
        {
            return Js::Constants::AnonymousClass;
        }
    }
    else if (pClass->pnodeName != nullptr)
    {
        return pClass->pnodeName->sxVar.pid->Psz();
    }

    return nullptr;
}

IdentPtr Parser::PidFromNode(ParseNodePtr pnode)
{
    for (;;)
    {
        switch (pnode->nop)
        {
        case knopName:
            return pnode->sxPid.pid;

        case knopVarDecl:
            return pnode->sxVar.pid;

        case knopDot:
            Assert(pnode->sxBin.pnode2->nop == knopName);
            return pnode->sxBin.pnode2->sxPid.pid;

        case knopComma:
            // Advance to the RHS and iterate.
            pnode = pnode->sxBin.pnode2;
            break;

        default:
            return NULL;
        }
    }
}

#if DBG
void VerifyNodeSize(OpCode nop, int size)
{
    Assert(nop >= 0 && nop < knopLim);
    __analysis_assume(nop < knopLim);
    Assert(g_mpnopcbNode[nop] == size);
}
#endif

ParseNodePtr Parser::StaticCreateBinNode(OpCode nop, ParseNodePtr pnode1,
                                   ParseNodePtr pnode2,ArenaAllocator* alloc)
{
    DebugOnly(VerifyNodeSize(nop, kcbPnBin));
    ParseNodePtr pnode = (ParseNodePtr)alloc->Alloc(kcbPnBin);
    InitNode(nop, pnode);

    pnode->sxBin.pnodeNext = NULL;
    pnode->sxBin.pnode1 = pnode1;
    pnode->sxBin.pnode2 = pnode2;

    // Statically detect if the add is a concat
    if (!PHASE_OFF1(Js::ByteCodeConcatExprOptPhase))
    {
        // We can't flatten the concat expression if the LHS is not a flatten concat already
        // e.g.  a + (<str> + b)
        //      Side effect of ToStr(b) need to happen first before ToStr(a)
        //      If we flatten the concat expr, we will do ToStr(a) before ToStr(b)
        if ((nop == knopAdd) && (pnode1->CanFlattenConcatExpr() || pnode2->nop == knopStr))
        {
            pnode->grfpn |= fpnCanFlattenConcatExpr;
        }
    }

    return pnode;
}

// Create nodes using parser allocator

ParseNodePtr Parser::CreateNode(OpCode nop, charcount_t ichMin)
{
    bool nodeAllowed = IsNodeAllowedForDeferParse(nop)
#if ERROR_RECOVERY
        // The language service needs to be able to complete a defer parse for syntax validation, including creating dummy nodes.
        || (this->m_deferringAST && LanguageServiceMode())
#endif
    ;
    Assert(nodeAllowed);

    Assert(nop >= 0 && nop < knopLim);
    ParseNodePtr pnode;
    int cb = (nop >= knopNone && nop < knopLim) ? g_mpnopcbNode[nop] : g_mpnopcbNode[knopEmpty];

    pnode = (ParseNodePtr)m_nodeAllocator.Alloc(cb);
    Assert(pnode != null);

    if (!m_deferringAST)
    {
        Assert(m_pCurrentAstSize != NULL);
        *m_pCurrentAstSize += cb;
    }

    InitNode(nop,pnode);

    // default - may be changed
    pnode->ichMin = ichMin;
    if (m_pscan!=NULL) {
      pnode->ichLim = m_pscan->IchLimTok();
    }
    else pnode->ichLim=0;

    return pnode;
}

ParseNodePtr Parser::CreateUniNode(OpCode nop, ParseNodePtr pnode1)
{
    Assert(!this->m_deferringAST);
    DebugOnly(VerifyNodeSize(nop, kcbPnUni));
    ParseNodePtr pnode = (ParseNodePtr)m_nodeAllocator.Alloc(kcbPnUni);

    Assert(m_pCurrentAstSize != NULL);
    *m_pCurrentAstSize += kcbPnUni;

    InitNode(nop, pnode);

    pnode->sxUni.pnode1 = pnode1;
    if (NULL == pnode1)
    {
        // no ops
        pnode->ichMin = m_pscan->IchMinTok();
        pnode->ichLim = m_pscan->IchLimTok();
    }
    else
    {
        // 1 op
        pnode->ichMin = pnode1->ichMin;
        pnode->ichLim = pnode1->ichLim;
        this->CheckArguments(pnode);
    }
    return pnode;
}

ParseNodePtr Parser::CreateBinNode(OpCode nop, ParseNodePtr pnode1, ParseNodePtr pnode2)
{
    Assert(!this->m_deferringAST);
    charcount_t ichMin;
    charcount_t ichLim;

    if (NULL == pnode1)
    {
        // no ops
        Assert(NULL == pnode2);
        ichMin = m_pscan->IchMinTok();
        ichLim = m_pscan->IchLimTok();
    }
    else
    {
        if (NULL == pnode2)
        {
            // 1 op
            ichMin = pnode1->ichMin;
            ichLim = pnode1->ichLim;
        }
        else
        {
            // 2 ops
            ichMin = pnode1->ichMin;
            ichLim = pnode2->ichLim;
            if (nop != knopDot && nop != knopIndex)
            {
                this->CheckArguments(pnode2);
            }
        }
        if (nop != knopDot && nop != knopIndex)
        {
            this->CheckArguments(pnode1);
        }
    }

    return CreateBinNode(nop, pnode1, pnode2, ichMin, ichLim);
}

ParseNodePtr Parser::CreateTriNode(OpCode nop, ParseNodePtr pnode1,
                                   ParseNodePtr pnode2, ParseNodePtr pnode3)
{
    charcount_t ichMin;
    charcount_t ichLim;

    if (NULL == pnode1)
    {
        // no ops
        Assert(NULL == pnode2);
        Assert(NULL == pnode3);
        ichMin = m_pscan->IchMinTok();
        ichLim = m_pscan->IchLimTok();
    }
    else if (NULL == pnode2)
    {
        // 1 op
        Assert(NULL == pnode3);
        ichMin = pnode1->ichMin;
        ichLim = pnode1->ichLim;
    }
    else if (NULL == pnode3)
    {
        // 2 op
        ichMin = pnode1->ichMin;
        ichLim = pnode2->ichLim;
    }
    else
    {
        // 3 ops
        ichMin = pnode1->ichMin;
        ichLim = pnode3->ichLim;
    }

    return CreateTriNode(nop, pnode1, pnode2, pnode3, ichMin, ichLim);
}

ParseNodePtr Parser::CreateBlockNode(charcount_t ichMin,charcount_t ichLim, PnodeBlockType blockType)
{
    return StaticCreateBlockNode(&m_nodeAllocator, ichMin, ichLim, this->m_nextBlockId++, blockType);
}

ParseNodePtr
Parser::CreateCallNode(OpCode nop, ParseNodePtr pnode1, ParseNodePtr pnode2,charcount_t ichMin,charcount_t ichLim)
{
    Assert(!this->m_deferringAST);
    DebugOnly(VerifyNodeSize(nop, kcbPnCall));
    ParseNodePtr pnode = (ParseNodePtr)m_nodeAllocator.Alloc(kcbPnCall);

    Assert(m_pCurrentAstSize != NULL);
    *m_pCurrentAstSize += kcbPnCall;

    InitNode(nop, pnode);

    pnode->sxCall.pnodeTarget = pnode1;
    pnode->sxCall.pnodeArgs = pnode2;
    pnode->sxCall.argCount = 0;
    pnode->sxCall.spreadArgCount = 0;
    pnode->sxCall.callOfConstants = false;
    pnode->sxCall.isApplyCall = false;
    pnode->sxCall.isEvalCall = false;

    pnode->ichMin = ichMin;
    pnode->ichLim = ichLim;

    return pnode;
}

ParseNodePtr Parser::CreateStrNode(IdentPtr pid)
{
    Assert(!this->m_deferringAST);

    ParseNodePtr pnode = CreateNode(knopStr);
    pnode->sxPid.pid=pid;
    pnode->grfpn |= PNodeFlags::fpnCanFlattenConcatExpr;
    return pnode;
}

ParseNodePtr Parser::CreateIntNode(long lw)
{
    ParseNodePtr pnode = CreateNode(knopInt);
    pnode->sxInt.lw = lw;
    return pnode;
}

// Create Node with scanner limit
template <OpCode nop>
ParseNodePtr Parser::CreateNodeWithScanner()
{
    Assert(m_pscan != null);
    return CreateNodeWithScanner<nop>(m_pscan->IchMinTok());
}

template <OpCode nop>
ParseNodePtr Parser::CreateNodeWithScanner(charcount_t ichMin)
{
    Assert(m_pscan != null);
    return CreateNodeT<nop>(ichMin, m_pscan->IchLimTok());
}

ParseNodePtr Parser::CreateCallNode(OpCode nop, ParseNodePtr pnode1, ParseNodePtr pnode2)
{
    charcount_t ichMin;
    charcount_t ichLim;

    if (NULL == pnode1)
    {
        Assert(NULL == pnode2);
        ichMin = m_pscan->IchMinTok();
        ichLim = m_pscan->IchLimTok();
    }
    else
    {
        if (NULL == pnode2)
        {
            ichMin = pnode1->ichMin;
            ichLim = pnode1->ichLim;
        }
        else
        {
            ichMin = pnode1->ichMin;
            ichLim = pnode2->ichLim;
        }
        if (pnode1->nop == knopDot || pnode1->nop == knopIndex)
        {
            this->CheckArguments(pnode1->sxBin.pnode1);
        }
    }
    return CreateCallNode(nop, pnode1, pnode2, ichMin, ichLim);
}

ParseNodePtr Parser::CreateStrNodeWithScanner(IdentPtr pid)
{
    Assert(!this->m_deferringAST);

    ParseNodePtr pnode = CreateNodeWithScanner<knopStr>();
    pnode->sxPid.pid=pid;
    pnode->grfpn |= PNodeFlags::fpnCanFlattenConcatExpr;
    return pnode;
}

ParseNodePtr Parser::CreateIntNodeWithScanner(long lw)
{
    Assert(!this->m_deferringAST);
    ParseNodePtr pnode = CreateNodeWithScanner<knopInt>();
    pnode->sxInt.lw = lw;
    return pnode;
}

ParseNodePtr Parser::CreateTempNode(ParseNode* initExpr) {
  ParseNodePtr pnode = CreateNode(knopTemp,(charcount_t)0);
  pnode->sxVar.pnodeInit=initExpr;
  pnode->sxVar.pnodeNext=NULL;
  return pnode;
}

ParseNodePtr Parser::CreateTempRef(ParseNode* tempNode) {
  ParseNodePtr pnode = CreateUniNode(knopTempRef,tempNode);
  return pnode;
}

void Parser::CheckPidIsValid(IdentPtr pid, bool autoArgumentsObject)
{
    if (IsStrictMode())
    {
        // in strict mode, variable named 'eval' cannot be created
        if (pid == wellKnownPropertyPids.eval)
        {
            Error(ERREvalUsage);
        }
        else if (pid == wellKnownPropertyPids.arguments && !autoArgumentsObject)
        {
            Error(ERRArgsUsage);
        }
    }
}

// CreateVarDecl needs m_ppnodeVar to be pointing to the right function.
// Post-parsing rewriting during bytecode gen may have m_ppnodeVar pointing to the last parsed function.
// This function sets up m_ppnodeVar to point to the given pnodeFnc and creates the new var declaration.
// This prevents accidentally adding var declarations to the last parsed function.
ParseNodePtr Parser::AddVarDeclNode(IdentPtr pid, ParseNodePtr pnodeFnc)
{
    Assert(pnodeFnc);

    ParseNodePtr *const ppnodeVarSave = m_ppnodeVar;
    m_ppnodeVar = &pnodeFnc->sxFnc.pnodeVars;

    ParseNodePtr pnode = CreateVarDeclNode(pid, STUnknown, false, 0, /* checkReDecl = */ false);

    m_ppnodeVar = ppnodeVarSave;

    return pnode;
}

ParseNodePtr Parser::CreateVarDeclNode(IdentPtr pid, SymbolType symbolType, bool autoArgumentsObject, ParseNodePtr pnodeFnc, bool errorOnRedecl)
{
    ParseNodePtr pnode = CreateDeclNode(knopVarDecl, pid, symbolType, errorOnRedecl);

#if PARSENODE_EXTENSIONS
    // Place the definition of the implicit arguments array at the function name or function if there is no name.
    if (LanguageServiceMode() && autoArgumentsObject && pnodeFnc)
    {
        ParseNodePtr definitionNode = pnodeFnc->sxFnc.pnodeNames;
        if (!definitionNode)
            definitionNode = pnodeFnc;
        pnode->ichMin = definitionNode->ichMin;
        pnode->ichLim = definitionNode->ichLim;
    }
#endif

    // Append the variable to the end of the current variable list.
    AssertMem(m_ppnodeVar);
    pnode->sxVar.pnodeNext = *m_ppnodeVar;
    *m_ppnodeVar = pnode;
    if (NULL != pid)
    {
        // this is not a temp - make sure temps go after this node
        AssertMem(pid);
        m_ppnodeVar = &pnode->sxVar.pnodeNext;
        CheckPidIsValid(pid, autoArgumentsObject);
    }

    return pnode;
}

ParseNodePtr Parser::CreateBlockScopedDeclNode(IdentPtr pid, OpCode nodeType)
{
    Assert(nodeType == knopConstDecl || nodeType == knopLetDecl);

    ParseNodePtr pnode = CreateDeclNode(nodeType, pid, STVariable, true);

    if (NULL != pid)
    {
        AssertMem(pid);
        pid->SetIsLetOrConst();
        AddVarDeclToBlock(pnode);
        CheckPidIsValid(pid);
    }

    return pnode;
}

void Parser::AddVarDeclToBlock(ParseNode *pnode)
{
    Assert(pnode->nop == knopConstDecl || pnode->nop == knopLetDecl);

    // Maintain a combined list of let and const decls to keep
    // track of declaration order.

    AssertMem(m_currentBlockInfo->m_ppnodeLex);
    *m_currentBlockInfo->m_ppnodeLex = pnode;
    m_currentBlockInfo->m_ppnodeLex = &pnode->sxVar.pnodeNext;
    pnode->sxVar.pnodeNext = NULL;
}

void Parser::SetCurrentStatement(StmtNest *stmt)
{
    m_pstmtCur = stmt;
}

template<bool buildAST>
ParseNodePtr Parser::StartParseBlockWithCapacity(PnodeBlockType blockType, ScopeType scopeType, int capacity)
{
    Scope *scope = null;
    // Block scopes are created lazily when we discover block-scoped content.
    if (scopeType != ScopeType_Unknown && scopeType != ScopeType_Block)
    {
        scope = Anew(&m_nodeAllocator, Scope, &m_nodeAllocator, scopeType, PHASE_OFF1(Js::ParserBindPhase), capacity);
        PushScope(scope);
    }

    return StartParseBlockHelper<buildAST>(blockType, scope, null, null);
}

template<bool buildAST>
ParseNodePtr Parser::StartParseBlock(PnodeBlockType blockType, ScopeType scopeType, ParseNodePtr pnodeLabel, LabelId* pLabelId)
{
    Scope *scope = null;
    // Block scopes are created lazily when we discover block-scoped content.
    if (scopeType != ScopeType_Unknown && scopeType != ScopeType_Block)
    {
        scope = Anew(&m_nodeAllocator, Scope, &m_nodeAllocator, scopeType);
        PushScope(scope);
    }

    return StartParseBlockHelper<buildAST>(blockType, scope, pnodeLabel, pLabelId);
}

template<bool buildAST>
ParseNodePtr Parser::StartParseBlockHelper(PnodeBlockType blockType, Scope *scope, ParseNodePtr pnodeLabel, LabelId* pLabelId)
{
    ParseNodePtr pnodeBlock = CreateBlockNode(blockType);
    pnodeBlock->sxBlock.scope = scope;
    BlockInfoStack *newBlockInfo = PushBlockInfo(pnodeBlock);

    PushStmt<buildAST>(&newBlockInfo->pstmt, pnodeBlock, knopBlock, pnodeLabel, pLabelId);

    return pnodeBlock;
}

void Parser::PushScope(Scope *scope)
{
    Assert(scope);
    scope->SetEnclosingScope(m_currentScope);
    m_currentScope = scope;
}

void Parser::PopScope(Scope *scope)
{
    Assert(scope == m_currentScope);
    m_currentScope = scope->GetEnclosingScope();
    scope->SetEnclosingScope(null);
}

void Parser::PushFuncBlockScope(ParseNodePtr pnodeBlock, ParseNodePtr **ppnodeScopeSave, ParseNodePtr **ppnodeExprScopeSave)
{
    bool blockHasScope = m_scriptContext->GetConfig()->IsBlockScopeEnabled();
    if (blockHasScope)
    {
        // Maintain the scope tree.

        pnodeBlock->sxBlock.pnodeScopes = NULL;
        pnodeBlock->sxBlock.pnodeNext = NULL;

        // Insert this block into the active list of scopes (m_ppnodeExprScope or m_ppnodeScope).
        // Save the current block's "next" pointer as the new endpoint of that list.
        if (m_ppnodeExprScope)
        {
            *ppnodeScopeSave = m_ppnodeScope;

            Assert(*m_ppnodeExprScope == NULL);
            *m_ppnodeExprScope = pnodeBlock;
            *ppnodeExprScopeSave = &pnodeBlock->sxBlock.pnodeNext;
        }
        else
        {
            Assert(m_ppnodeScope);
            Assert(*m_ppnodeScope == NULL);
            *m_ppnodeScope = pnodeBlock;
            *ppnodeScopeSave = &pnodeBlock->sxBlock.pnodeNext;

            *ppnodeExprScopeSave = m_ppnodeExprScope;
        }

        // Advance the global scope list pointer to the new block's child list.
        m_ppnodeScope = &pnodeBlock->sxBlock.pnodeScopes;
        // Set m_ppnodeExprScope to NULL to make that list inactive.
        m_ppnodeExprScope = NULL;
    }
}

void Parser::PopFuncBlockScope(ParseNodePtr *ppnodeScopeSave, ParseNodePtr *ppnodeExprScopeSave)
{
    bool blockHasScope = m_scriptContext->GetConfig()->IsBlockScopeEnabled();
    if (blockHasScope)
    {
        Assert(m_ppnodeExprScope == NULL || *m_ppnodeExprScope == NULL);
        m_ppnodeExprScope = ppnodeExprScopeSave;

        AssertMem(m_ppnodeScope);
        Assert(NULL == *m_ppnodeScope);
        m_ppnodeScope = ppnodeScopeSave;
    }
}

template<bool buildAST>
ParseNodePtr Parser::ParseBlock(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeLabel, LabelId* pLabelId)
{    
    StmtNest stmt;
    ParseNodePtr pnodeBlock = NULL;
    ParseNodePtr *ppnodeScopeSave = NULL;
    ParseNodePtr *ppnodeExprScopeSave = NULL;

#if PARSENODE_EXTENSIONS
    charcount_t ichLCurlyMin = 0;
    charcount_t ichRCurlyMin = 0;
#endif

    if (buildAST || BindDeferredPidRefs())
    {
        pnodeBlock = StartParseBlock<buildAST>(PnodeBlockType::Regular, ScopeType_Block, pnodeLabel, pLabelId);
    }
    else
    {
        PushStmt<buildAST>(&stmt, NULL, knopBlock, pnodeLabel, pLabelId);
    }

    ChkCurTok(tkLCurly, ERRnoLcurly _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLCurlyMin));

#if PARSENODE_EXTENSIONS
    // Keep ichLCurlyMin if needed
    if (buildAST && LanguageServiceMode())
    {
        m_languageServiceExtension->SetLCurly(pnodeBlock, ichLCurlyMin);
    }
#endif

    if (buildAST)
    {
        PushFuncBlockScope(pnodeBlock, &ppnodeScopeSave, &ppnodeExprScopeSave);
    }

    ParseStmtList<buildAST>(ERROR_RECOVERY_ACTUAL_(ersRCurly | ers) &pnodeBlock->sxBlock.pnodeStmt);

    if (buildAST)
    {
        PopFuncBlockScope(ppnodeScopeSave, ppnodeExprScopeSave);
    }

    if (buildAST || BindDeferredPidRefs())
    {
        FinishParseBlock(pnodeBlock);
    }
    else
    {
        PopStmt(&stmt);
    }

    ChkCurTok(tkRCurly, ERRnoRcurly _ERROR_RECOVERY_ACTUAL(ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRCurlyMin));

#if PARSENODE_EXTENSIONS
    // Keep ichRCurlyMin if needed
    if (buildAST && LanguageServiceMode())
    {
        m_languageServiceExtension->SetRCurly(pnodeBlock, ichRCurlyMin);
    }
#endif

    return pnodeBlock;
}

void Parser::FinishParseBlock(ParseNode *pnodeBlock, bool needScanRCurly)
{
    Assert(m_currentBlockInfo != NULL && pnodeBlock == m_currentBlockInfo->pnodeBlock);

    if (needScanRCurly)
    {
        // Only update the ichLim if we were expecting an RCurly. If there is an
        // expression body without a necessary RCurly, the correct ichLim will
        // have been set already.
        pnodeBlock->ichLim = m_pscan->IchLimTok();
    }
    
    BindPidRefs<false>(GetCurrentBlockInfo(), m_nextBlockId - 1);

    PopStmt(&m_currentBlockInfo->pstmt);

    PopBlockInfo();

    Scope *scope = pnodeBlock->sxBlock.scope;
    if (scope)
    {
        PopScope(scope);
    }
}

void Parser::FinishParseFncExprScope(ParseNodePtr pnodeFnc, ParseNodePtr pnodeFncExprScope)
{
    int fncExprScopeId = pnodeFncExprScope->sxBlock.blockId;
    ParseNodePtr pnodeName = pnodeFnc->sxFnc.pnodeNames;
    if (pnodeName)
    {
        Assert(pnodeName->nop == knopVarDecl);
        BindPidRefsInScope(pnodeName->sxVar.pid, pnodeName->sxVar.sym, fncExprScopeId);
    }
    FinishParseBlock(pnodeFncExprScope);
}

template <const bool backgroundPidRef>
void Parser::BindPidRefs(BlockInfoStack *blockInfo, uint maxBlockId)
{
    // We need to bind all assignments in order to emit assignment to 'const' error
    int blockId = blockInfo->pnodeBlock->sxBlock.blockId;

    Scope *scope = blockInfo->pnodeBlock->sxBlock.scope;
    if (scope)
    {
        auto bindPidRefs = [blockId, maxBlockId, this](Symbol *sym)
        {
            ParseNodePtr pnode = sym->GetDecl();
            IdentPtr pid;
#if PROFILE_DICTIONARY
            int depth = 0;
#endif
            Assert(pnode);
            switch (pnode->nop)
            {
            case knopLetDecl:
            case knopVarDecl:
                pid = pnode->sxVar.pid;
                if (backgroundPidRef)
                {
                    pid = this->m_pscan->m_phtbl->FindExistingPid(pid->Psz(), pid->Cch(), pid->Hash(), nullptr, nullptr
#if PROFILE_DICTIONARY
                                                                  , depth
#endif
                        );
                    if (pid == nullptr)
                    {
                        break;
                    }
                }
                this->BindPidRefsInScope(pid, sym, blockId, maxBlockId);
                break;
            case knopConstDecl:
                pid = pnode->sxVar.pid;
                if (backgroundPidRef)
                {
                    pid = this->m_pscan->m_phtbl->FindExistingPid(pid->Psz(), pid->Cch(), pid->Hash(), nullptr, nullptr
#if PROFILE_DICTIONARY
                                                                  , depth
#endif
                        );
                    if (pid == nullptr)
                    {
                        break;
                    }
                }
                this->BindConstPidRefsInScope(pid, sym, blockId, maxBlockId);
                break;
            case knopName:
                pid = pnode->sxPid.pid;
                if (backgroundPidRef)
                {
                    pid = this->m_pscan->m_phtbl->FindExistingPid(pid->Psz(), pid->Cch(), pid->Hash(), nullptr, nullptr
#if PROFILE_DICTIONARY
                                                                  , depth
#endif
                        );
                    if (pid == nullptr)
                    {
                        break;
                    }
                }
                this->BindPidRefsInScope(pid, sym, blockId, maxBlockId);
                break;
            default:
                Assert(0);
                break;
            }
        };

        scope->ForEachSymbol(bindPidRefs);
    }
}

void Parser::BindPidRefsInScope(IdentPtr pid, Symbol *sym, int blockId, uint maxBlockId)
{
    this->BindPidRefsInScopeImpl<false>(pid, sym, blockId, maxBlockId);
}

void Parser::BindConstPidRefsInScope(IdentPtr pid, Symbol *sym, int blockId, uint maxBlockId)
{
    this->BindPidRefsInScopeImpl<true>(pid, sym, blockId, maxBlockId);
}

template<const bool isConstBinding>
void Parser::BindPidRefsInScopeImpl(IdentPtr pid, Symbol *sym, int blockId, uint maxBlockId)
{
    PidRefStack *ref, *nextRef, *lastRef = nullptr;
    Assert(sym);

    for (ref = pid->GetTopRef(); ref && ref->GetScopeId() >= blockId; ref = nextRef)
    {
        // Fix up sym* on pid ref.
        Assert(!ref->GetSym() || ref->GetSym() == sym);
        nextRef = ref->prev;
        Assert(ref->GetScopeId() >= 0);
        if ((uint)ref->GetScopeId() > maxBlockId)
        {
            lastRef = ref;
            continue;
        }
        ref->SetSym(sym);
        if (isConstBinding && ref->IsAssignment() && !ref->IsDynamicBinding())
        {
            if (pid->GetTopIchMin() < this->m_asgToConst.GetIchMin())
            {
                this->m_asgToConst.Set(pid->GetTopIchMin(), pid->GetTopIchLim());
            }
        }
        this->RemovePrevPidRef(pid, lastRef);

        if (ref->IsAssignment())
        {
            sym->PromoteAssignmentState();
        }

        if (ref->GetScopeId() == blockId)
        {
            break;
        }
    }
}

BlockInfoStack *Parser::PushBlockInfo(ParseNodePtr pnodeBlock)
{
    BlockInfoStack *newBlockInfo = (BlockInfoStack *)m_nodeAllocator.Alloc(sizeof(BlockInfoStack));
    Assert(NULL != newBlockInfo);

    newBlockInfo->pnodeBlock = pnodeBlock;
    newBlockInfo->pBlockInfoOuter = m_currentBlockInfo;
    newBlockInfo->m_ppnodeLex = &pnodeBlock->sxBlock.pnodeLexVars;

    if (pnodeBlock->sxBlock.blockType != PnodeBlockType::Regular)
    {
        newBlockInfo->pBlockInfoFunction = newBlockInfo;
    }
    else
    {
        Assert(m_currentBlockInfo);
        newBlockInfo->pBlockInfoFunction = m_currentBlockInfo->pBlockInfoFunction;
    }

    m_currentBlockInfo = newBlockInfo;
    return newBlockInfo;
}

void Parser::PopBlockInfo()
{
    Assert(m_currentBlockInfo);
    PopDynamicBlock();
    m_currentBlockInfo = m_currentBlockInfo->pBlockInfoOuter;
}

void Parser::PushDynamicBlock()
{
    if (!m_scriptContext->GetConfig()->IsLetAndConstEnabled())
    {
        // Shortcut: we only need to track dynamically-bound blocks for const reassignment.
        return;
    }

    Assert(GetCurrentBlock());
    int blockId = GetCurrentBlock()->sxBlock.blockId;
    if (m_currentDynamicBlock && m_currentDynamicBlock->id == blockId)
    {
        return;
    }
    BlockIdsStack *info = (BlockIdsStack *)m_nodeAllocator.Alloc(sizeof(BlockIdsStack));
    if (NULL == info)
    {
        Error(ERRnoMemory);
    }

    info->id = blockId;
    info->prev = m_currentDynamicBlock;
    m_currentDynamicBlock = info;
}

void Parser::PopDynamicBlock()
{
    int blockId = GetCurrentDynamicBlockId();
    if (GetCurrentBlock()->sxBlock.blockId != blockId || blockId == -1)
    {
        return;
    }
    Assert(m_currentDynamicBlock);
    AssertMsg(m_scriptContext->GetConfig()->IsLetAndConstEnabled(), "Should only do this if let/const is enabled since only needed for const reassignment error checking");
    for (BlockInfoStack *blockInfo = m_currentBlockInfo; blockInfo; blockInfo = blockInfo->pBlockInfoOuter)
    {
        for (ParseNodePtr pnodeDecl = blockInfo->pnodeBlock->sxBlock.pnodeLexVars;
             pnodeDecl;
             pnodeDecl = pnodeDecl->sxVar.pnodeNext)
        {
            this->SetPidRefsInScopeDynamic(pnodeDecl->sxVar.pid, blockId);
        }
    }

    m_currentDynamicBlock = m_currentDynamicBlock->prev;
}

int Parser::GetCurrentDynamicBlockId() const
{
    return m_currentDynamicBlock ? m_currentDynamicBlock->id : -1;
}

ParseNode *Parser::GetCurrentFunctionNode()
{
    if (m_currentNodeDeferredFunc != NULL)
    {
        return m_currentNodeDeferredFunc;
    }
    else if (m_currentNodeFunc != NULL)
    {
        return m_currentNodeFunc;
    }
    else
    {
        AssertMsg(GetFunctionBlock()->sxBlock.blockType == PnodeBlockType::Global, "Most likely we are trying to fing a syntax error, related to 'let' or 'const' in deferred parsing mode with disabled support of 'let' and 'const'");
        return m_currentNodeProg;
    }
}

void Parser::RegisterRegexPattern(UnifiedRegex::RegexPattern *const regexPattern)
{
    Assert(regexPattern);

    // ensure a no-throw add behavior here, to catch out of memory exceptions, using the guest arena allocator
    if (!m_registeredRegexPatterns.PrependNoThrow(m_scriptContext->GetGuestArena(), regexPattern))
    {
        Parser::Error(ERRnoMemory);
    }
}

void Parser::AddToNodeListEscapedUse(ParseNode ** ppnodeList, ParseNode *** pppnodeLast,
                           ParseNode * pnodeAdd)
{
    AddToNodeList(ppnodeList, pppnodeLast, pnodeAdd);
    pnodeAdd->SetIsInList();
}

void Parser::AddToNodeList(ParseNode ** ppnodeList, ParseNode *** pppnodeLast,
                           ParseNode * pnodeAdd)
{
    Assert(!this->m_deferringAST);
    if (NULL == *pppnodeLast)
    {
        // should be an empty list
        Assert(NULL == *ppnodeList);

        *ppnodeList = pnodeAdd;
        *pppnodeLast = ppnodeList;
    }
    else
    {
        //
        AssertNodeMem(*ppnodeList);
        AssertNodeMem(**pppnodeLast);

        ParseNode *pnodeT = CreateBinNode(knopList, **pppnodeLast, pnodeAdd);
        **pppnodeLast = pnodeT;
        *pppnodeLast = &pnodeT->sxBin.pnode2;
    }
}

#ifdef LANGUAGE_SERVICE
void Parser::AppendToNodeList(ParseNode **ppnodeList, ParseNode ***pppnodeLast,
                             ParseNode *pnodeAddList, ParseNode ***pppnodeAddLast)
{
    if (pnodeAddList)
    {
        AddToNodeList(ppnodeList, pppnodeLast, pnodeAddList);
        if (**pppnodeAddLast != pnodeAddList)
            // If we adding a single node then **pppnodeAddLast will equal pnodeAddList. In this case
            // pppnodeAddLast will point to the lastNodeRef temporary variable so we need to ignore it
            // Otherwise, it points to the end of the list being appended so copy it to ppnodeLast.
            *pppnodeLast = *pppnodeAddLast;
    }
}
#endif

// Check reference to "arguments" that indicates the object may escape.
void Parser::CheckArguments(ParseNodePtr pnode)
{
    if (m_currentNodeFunc && this->NodeIsIdent(pnode, wellKnownPropertyPids.arguments))
    {
        m_currentNodeFunc->sxFnc.SetHasHeapArguments();
    }
}

// Check use of "arguments" that requires instantiation of the object.
void Parser::CheckArgumentsUse(IdentPtr pid, ParseNodePtr pnodeFnc)
{
    if (pid == wellKnownPropertyPids.arguments)
    {
        if (pnodeFnc != NULL)
        {
            pnodeFnc->sxFnc.SetUsesArguments(TRUE);
        }
        else
        {
            m_UsesArgumentsAtGlobal = true;
        }
    }
}

void Parser::CheckStrictModeEvalArgumentsUsage(IdentPtr pid, ParseNodePtr pnode)
{
    if (pid != NULL)
    {
        // In strict mode, 'eval' / 'arguments' cannot be assigned to.
        if ( pid == wellKnownPropertyPids.eval)
        {
            Error(ERREvalUsage, pnode);
        }

        if (pid == wellKnownPropertyPids.arguments)
        {
            Error(ERRArgsUsage, pnode);
        }
    }
}

void Parser::CheckStrictModeFncDeclNotSourceElement(const bool isSourceElement, const BOOL isDeclaration)
{
    // In strict mode, only a SourceElement can expand to a FunctionDeclaration; a Statement cannot. That means a function
    // declaration may only appear as a top-level statement in a program or function body, and otherwise may not be nested
    // inside another statement or block.
    //
    // The only difference between a SourceElement and a Statement is that a SourceElement can include a FunctionDeclaration, so
    // we just use ParseStmtList and ParseStatement and pass in a flag indicating whether the statements are source elements.
    Assert(!(isSourceElement && !isDeclaration));
    if(IsStrictMode() && !isSourceElement && isDeclaration &&
       !this->GetScriptContext()->GetConfig()->IsBlockScopeEnabled())
    {
        Error(ERRFncDeclNotSourceElement);
    }
}

void Parser::ReduceDeferredScriptLength(ULONG chars)
{
    // If we're in deferred mode, subtract the given char count from the total length,
    // and see if this puts us under the deferral threshold.
    if (m_grfscr & fscrDeferFncParse)
    {
        if (m_length > chars)
        {
            m_length -= chars;
        }
        else
        {
            m_length = 0;
        }
        if (m_length < Parser::GetDeferralThreshold(this->m_sourceContextInfo->sourceDynamicProfileManager))
        {
            // Stop deferring.
            m_grfscr &= ~fscrDeferFncParse;
            m_stoppedDeferredParse = TRUE;
        }
    }
}

/***************************************************************************
Look for an existing label with the given name.
***************************************************************************/
BOOL Parser::PnodeLabelNoAST(IdentToken* pToken, LabelId* pLabelIdList)
{
    StmtNest* pStmt;
    LabelId* pLabelId;

    // Look in the label stack.
    for (pStmt = m_pstmtCur; pStmt != NULL; pStmt = pStmt->pstmtOuter)
    {
        for (pLabelId = pStmt->pLabelId; pLabelId != NULL; pLabelId = pLabelId->next)
        {
            if (pLabelId->pid == pToken->pid)
                return TRUE;
        }
    }

    // Also look in the pnodeLabels list.
    for (pLabelId = pLabelIdList; pLabelId != NULL; pLabelId = pLabelId->next)
    {
        if (pLabelId->pid == pToken->pid)
            return TRUE;
    }

    return FALSE;
}

void Parser::EnsureStackAvailable()
{
    if (!m_scriptContext->GetThreadContext()->IsStackAvailable(Js::Constants::MinStackCompile))
    {
#ifdef LANGUAGE_SERVICE
        Error(E_FAIL);
#else
        Error(ERRnoMemory);
#endif
    }
}

/***************************************************************************
Parse an expression term.
***************************************************************************/
template<bool buildAST>
ParseNodePtr Parser::ParseTerm(ERROR_RECOVERY_FORMAL_ BOOL fAllowCall, LPCOLESTR pNameHint, ulong *pHintLength,  _Inout_opt_ IdentToken* pToken/*= NULL*/, bool fUnaryOrParen)
{
    ParseNodePtr pnode = NULL;
    charcount_t ichMin;
    size_t iuMin;
    IdentToken term;
    BOOL fInNew = FALSE;
    Assert(pToken == NULL || pToken->tk == tkNone); // Must be empty initially

    if (this->IsBackgroundParser())
    {
        PROBE_STACK_NO_DISPOSE(m_scriptContext, Js::Constants::MinStackParseOneTerm);
    }
    else
    {
        PROBE_STACK(m_scriptContext, Js::Constants::MinStackParseOneTerm);
    }

    switch (m_token.tk)
    {
    case tkID:
    {
        PidRefStack *ref = null;
        IdentPtr pid = m_token.GetIdentifier(m_phtbl);
        if (buildAST || BindDeferredPidRefs())
        {
            ref = this->PushPidRef(pid);
        }
        if (buildAST)
        {
            pnode = CreateNameNode(pid);
            pnode->sxPid.SetSymRef(ref);
            CheckArgumentsUse(pid, m_currentNodeFunc);
        }
        else
        {
            // Remember the identifier start and end in case it turns out to be a statement label.
            term.tk = tkID;
            term.pid = pid; // Record the identifier for detection of eval
            term.ichMin = static_cast<charcount_t>(m_pscan->IecpMinTok());
            term.ichLim = static_cast<charcount_t>(m_pscan->IecpLimTok());
        }
        m_pscan->Scan();
        break;
    }

    case tkTHIS:
        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopThis>();
        }
        m_pscan->Scan();
        break;

    case tkLParen:
        ichMin = m_pscan->IchMinTok();
        iuMin = m_pscan->IecpMinTok();
        m_pscan->Scan();
        if (m_token.tk == tkRParen)
        {
            // Empty parens can only be legal as an empty parameter list to a lambda declaration.
            // We're in a lambda if the next token is =>.
            fAllowCall = FALSE;
            m_pscan->Scan();

            // If the token after the right paren is not => or if there was a newline between () and => this is a syntax error
            if (!m_doingFastScan && (m_token.tk != tkDArrow || m_pscan->FHadNewLine()))
            {
                Error(ERRsyntax);
#if ERROR_RECOVERY
                // No need to recover here as we can just assume that this was (<ident>) instead of ().
                pnode = CreateErrorNameNode();
                pnode->ichMin = ichMin;
                pnode->ichLim = m_pscan->IchLimTok();
                break;
#endif
            }

            if (buildAST)
            {
                pnode = CreateNodeWithScanner<knopEmpty>();
            }
            break;
        }

        // Decrement expression depth so that we don't count nesting of parentheses.
        // Restore decremented depth after the expression contained in parentheses is finished.
        this->m_exprDepth--;
        this->m_parenDepth++;
        pnode = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersRParen | ers) koplNo, nullptr, TRUE, FALSE, nullptr, nullptr /*nameLength*/, &term, true);
        this->m_parenDepth--;
        this->m_exprDepth++;

#if PARSENODE_EXTENSIONS
        if(buildAST && LanguageServiceMode())
        {
            if (pnode && pnode->nop == knopName)
            {
                // Remember the actual min an lim of the identifier for extent calculations in the language service.
                m_languageServiceExtension->SetLParen(pnode, pnode->ichMin);
                m_languageServiceExtension->SetRParen(pnode, pnode->ichLim);
            }

            // Legacy behavior was to include the parentheses in the source code the expression node maps to. So for instance,
            // if the expression inside parentheses is a function, the function node's source code would include the parentheses
            // and a toString later on the function object would include the parentheses in the string.

            pnode->ichMin = ichMin;
            pnode->ichLim = m_pscan->IchLimTok();
            if (pnode->nop == knopFncDecl || pnode->nop == knopProg)
            {
                // If it is a function declaration or the program scope update the unit indexes as well.
                pnode->sxFnc.cbMin = iuMin;
                pnode->sxFnc.cbLim = m_pscan->IecpLimTok();
            }
        }
#endif
        ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ers));
#if PARSENODE_EXTENSIONS
        if (LanguageServiceMode() && buildAST && pnode != nullptr)
            m_languageServiceExtension->IncrementParenthesesCount(pnode);
#endif
        // Emit a deferred ... error if one was parsed.
        if (m_deferEllipsisError && m_token.tk != tkDArrow)
        {
            m_pscan->SeekTo(m_EllipsisErrLoc);
            Error(ERRInvalidSpreadUse);
#if ERROR_RECOVERY
            m_deferEllipsisError = false;
#endif
        }
        else
        {
            m_deferEllipsisError = false;
        }
        break;

    case tkIntCon:
        if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
        {
            Error(ERRES5NoOctal);
        }

        if (buildAST)
        {
            pnode = CreateIntNodeWithScanner(m_token.GetLong());
        }
        m_pscan->Scan();
        break;

    case tkFltCon:
        if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
        {
            Error(ERRES5NoOctal);
        }

        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopFlt>();
            pnode->sxFlt.dbl = m_token.GetDouble();
            pnode->sxFlt.maybeInt = m_token.GetDoubleMayBeInt();
        }
        m_pscan->Scan();
        break;

    case tkStrCon:
        if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
        {
            Error(ERRES5NoOctal);
        }

        if (buildAST)
        {
            pnode = CreateStrNodeWithScanner(m_token.GetStr());
        }
        else
        {
            // Subtract the string literal length from the total char count for the purpose
            // of deciding whether to defer parsing and byte code generation.
            this->ReduceDeferredScriptLength(m_pscan->IchLimTok() - m_pscan->IchMinTok());
        }
        m_pscan->Scan();
        break;

    case tkTRUE:
        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopTrue>();
        }
        m_pscan->Scan();
        break;

    case tkFALSE:
        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopFalse>();
        }
        m_pscan->Scan();
        break;

    case tkNULL:
        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopNull>();
        }
        m_pscan->Scan();
        break;

    case tkDiv:
    case tkAsgDiv:
        pnode = ParseRegExp<buildAST>();
        m_pscan->Scan();
        break;

    case tkNEW:
    {
        ichMin = m_pscan->IchMinTok();
        m_pscan->Scan();
        ParseNodePtr pnodeExpr = ParseTerm<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) FALSE, pNameHint, pHintLength);
        if (buildAST)
        {
            pnode = CreateCallNode(knopNew, pnodeExpr, NULL);
            pnode->ichMin = ichMin;
        }
        fInNew = TRUE;
        break;
    }

    case tkLBrack:
    {
#if PARSENODE_EXTENSIONS
        charcount_t ichRBrackMin = 0;
#endif
        ichMin = m_pscan->IchMinTok();
        m_pscan->Scan();
        pnode = ParseArrayLiteral<buildAST>(ERROR_RECOVERY_ACTUAL(ersRBrack | ers));
        if (buildAST)
        {
            pnode->ichMin = ichMin;
            pnode->ichLim = m_pscan->IchLimTok();
        }

        if (this->m_arrayDepth == 0)
        {
            Assert(m_pscan->IchLimTok() - ichMin > m_funcInArray);
            this->ReduceDeferredScriptLength(m_pscan->IchLimTok() - ichMin - this->m_funcInArray);
            this->m_funcInArray = 0;
            this->m_funcInArrayDepth = 0;
        }
        ChkCurTok(tkRBrack, ERRnoRbrack _ERROR_RECOVERY_ACTUAL(ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRBrackMin));

#if PARSENODE_EXTENSIONS
        // Keep ichLBrackMin and ichRBrackMin if needed
        if (buildAST && LanguageServiceMode())
        {
            m_languageServiceExtension->SetLBrack(pnode, ichMin);
            m_languageServiceExtension->SetRBrack(pnode, ichRBrackMin);
        }
#endif
    }
        break;

    case tkLCurly:
    {
#if PARSENODE_EXTENSIONS
        charcount_t ichRCurlyMin = 0;
#endif
        ichMin = m_pscan->IchMinTok();
        m_pscan->ScanForcingPid();
        ParseNodePtr pnodeMemberList = ParseMemberList<buildAST>(ERROR_RECOVERY_ACTUAL_(ersRCurly | ers) pNameHint, pHintLength);
        if (buildAST)
        {
            pnode = CreateUniNode(knopObject, pnodeMemberList);
            pnode->ichMin = ichMin;
            pnode->ichLim = m_pscan->IchLimTok();
        }
        ChkCurTok(tkRCurly, ERRnoRcurly _ERROR_RECOVERY_ACTUAL(ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRCurlyMin));

#if PARSENODE_EXTENSIONS
        // Keep ichLCurlyMin and ichRCurlyMin if needed
        if (buildAST && LanguageServiceMode())
        {
            m_languageServiceExtension->SetLCurly(pnode, ichMin);
            m_languageServiceExtension->SetRCurly(pnode, ichRCurlyMin);
        }
#endif
    }
        break;

    case tkFUNCTION:
    {
        if (m_grfscr & fscrDeferredFncExpression)
        {
            // The top-level deferred function body was defined by a function expression whose parsing was deferred. We are now
            // parsing it, so unset the flag so that any nested functions are parsed normally. This flag is only applicable the
            // first time we see it.
            //
            // Normally, deferred functions will be parsed in ParseStatement upon encountering the 'function' token. The first
            // token of the source code of the function may not a 'function' token though, so we still need to reset this flag
            // for the first function we parse. This can happen in compat modes, for instance, for a function expression encosed
            // in parentheses, where the legacy behavior was to include the parentheses in the function's source code.
            m_grfscr &= ~fscrDeferredFncExpression;
        }
#if PARSENODE_EXTENSIONS
        charcount_t ichtkFunctionMin = m_pscan->IchMinTok();
#endif
        pnode = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) fFncNoFlgs, pNameHint, false, false, fUnaryOrParen);

#if PARSENODE_EXTENSIONS
        // Keep ichtkFunctionMin if needed
        if (buildAST && LanguageServiceMode())
        {
            m_languageServiceExtension->SetTkFunctionMin(pnode, ichtkFunctionMin);
        }
#endif
        break;
    }

    case tkCLASS:
        fAllowCall = FALSE;
        if (m_scriptContext->GetConfig()->IsES6ClassAndExtendsEnabled())
        {
            pnode = ParseClassDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) FALSE, pNameHint, pHintLength);
        }
        else
        {
            goto LUnknown;
        }
        break;

    case tkStrTmplBasic:
    case tkStrTmplBegin:
        Assert(m_scriptContext->GetConfig()->IsES6StringTemplateEnabled());

        pnode = ParseStringTemplateDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) nullptr);
        break;

    case tkSUPER:
        if (m_scriptContext->GetConfig()->IsES6SuperEnabled())
        {
            pnode = ParseSuper<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) pnode);
        }
        else
        {
            goto LUnknown;
        }
        break;

    case tkCASE:
    {
        if (!m_doingFastScan)
        {
            goto LUnknown;
        }
        ParseNodePtr pnodeUnused;
        pnode = ParseCase<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) &pnodeUnused);
        break;
    }

    case tkELSE:
        if (!m_doingFastScan)
        {
            goto LUnknown;
        }
        m_pscan->Scan();
        ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
        break;

    default:
    LUnknown :
        Error(ERRsyntax);
#if ERROR_RECOVERY
        {
            SKIP(ERROR_RECOVERY_ACTUAL((ersID | ersPostfix) | ers));

            // Create an error node (or use the next tkID we happen to find).
            IdentPtr pid;
            if (m_token.tk != tkID)
            {
                pid = m_pidError;
            }
            else
            {
                pid = m_token.GetIdentifier(m_phtbl);
                m_pscan->Scan();
            }
            pnode = CreateNameNode(pid);
            PidRefStack *ref = this->PushPidRef(pid);
            pnode->sxPid.SetSymRef(ref);
        }
#endif
        break;
    }

    pnode = ParsePostfixOperators<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) pnode, fAllowCall, fInNew, &term);

    // Pass back identifier if requested
    if (pToken && term.tk == tkID)
    {
        *pToken = term;
    }

    return pnode;
}

template <bool buildAST>
ParseNodePtr Parser::ParseRegExp()
{
    ParseNodePtr pnode = nullptr;

    if (buildAST || m_doingFastScan)
    {
        m_pscan->RescanRegExp();

        BOOL saveDeferringAST = this->m_deferringAST;
        if (m_doingFastScan)
        {
            this->m_deferringAST = false;
        }
        pnode = CreateNodeWithScanner<knopRegExp>();
        pnode->sxPid.regexPattern = m_token.GetRegex();
        if (m_doingFastScan)
        {
            this->m_deferringAST = saveDeferringAST;
            this->AddFastScannedRegExpNode(pnode);
            if (!buildAST)
            {
                pnode = nullptr;
            }
        }
        else if (this->IsBackgroundParser())
        {
            Assert(pnode->sxPid.regexPattern == nullptr);
            this->AddBackgroundRegExpNode(pnode);
        }
    }
    else
    {
        m_pscan->RescanRegExpNoAST();
    }
    Assert(m_token.tk == tkRegExp || PerformingErrorRecovery());

    return pnode;
}

BOOL Parser::NodeIsEvalName(ParseNodePtr pnode)
{
    //WOOB 1107758 Special case of indirect eval binds to local scope in standards mode
    return pnode->nop == knopName && (pnode->sxPid.pid == wellKnownPropertyPids.eval);
}

BOOL Parser::NodeEqualsName(ParseNodePtr pnode, LPCOLESTR sz, ulong cch)
{
    return pnode->nop == knopName &&
        pnode->sxPid.pid->Cch() == cch &&
        !wmemcmp(pnode->sxPid.pid->Psz(), sz, cch);
}

BOOL Parser::NodeIsIdent(ParseNodePtr pnode, IdentPtr pid)
{
    for (;;)
    {
        switch (pnode->nop)
        {
        case knopName:
            return (pnode->sxPid.pid == pid);

        case knopComma:
            pnode = pnode->sxBin.pnode2;
            break;

        default:
            return FALSE;
        }
    }
}

template<bool buildAST>
ParseNodePtr Parser::ParsePostfixOperators(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnode,
                                           BOOL fAllowCall, BOOL fInNew, _Inout_ IdentToken* pToken)
{
    uint16 count = 0;
#if PARSENODE_EXTENSIONS
    charcount_t ichMin = 0;
#endif
    bool callOfConstants = false;

    for (;;)
    {
        uint16 spreadArgCount = 0;
        switch (m_token.tk)
        {
        case tkLParen:
            {
#if PARSENODE_EXTENSIONS
                charcount_t ichLParen = m_pscan->IchMinTok();
#endif
                if (fInNew)
                {
                    ParseNodePtr pnodeArgs = ParseArgList<buildAST>(ERROR_RECOVERY_ACTUAL_(ersRParen | ers) &callOfConstants, &spreadArgCount, &count);
                    if (buildAST)
                    {
                        Assert(pnode->nop == knopNew);
                        Assert(pnode->sxCall.pnodeArgs == NULL);
#if PARSENODE_EXTENSIONS
                        if(LanguageServiceMode())
                        {
                            m_languageServiceExtension->SetLParen(pnode, ichLParen);
                        }
#endif
                        pnode->sxCall.pnodeArgs = pnodeArgs;
                        pnode->sxCall.callOfConstants = callOfConstants;
                        pnode->sxCall.isApplyCall = false;
                        pnode->sxCall.isEvalCall = false;
                        pnode->sxCall.argCount = count;
                        pnode->sxCall.spreadArgCount = spreadArgCount;
                        pnode->ichLim = m_pscan->IchLimTok();
                    }
                    else
                    {
                        pToken->tk = tkNone; // This is no longer an identifier
                    }
                    fInNew = FALSE;
                }
                else
                {
                    bool fCallIsEval = false;
                    if (!fAllowCall)
                    {
                        return pnode;
                    }

                    ParseNodePtr pnodeArgs = ParseArgList<buildAST>(ERROR_RECOVERY_ACTUAL_(ersRParen | ers) &callOfConstants, &spreadArgCount, &count);
                    // We used to undefer a deferred function body here if it was called as part of the expression that declared it.
                    // We now detect this case up front in ParseFncDecl, which is cheaper and simpler.
                    if (buildAST)
                    {
                        pnode = CreateCallNode(knopCall, pnode, pnodeArgs);
#if PARSENODE_EXTENSIONS
                        if(LanguageServiceMode())
                        {
                            m_languageServiceExtension->SetLParen(pnode, ichLParen);
                        }
#endif
                        // Detect call to "eval" and record it on the function.
                        // Note: we used to leave it up to the byte code generator to detect eval calls
                        // at global scope, but now it relies on the flag the parser sets, so set it here.
                        if (pnode != NULL)
                        {
                            if (count > 0 && this->NodeIsEvalName(pnode->sxCall.pnodeTarget))
                            {
                                this->MarkEvalCaller();
                                fCallIsEval = true;
                            }
                        }

                        pnode->sxCall.callOfConstants = callOfConstants;
                        pnode->sxCall.spreadArgCount = spreadArgCount;
                        pnode->sxCall.isApplyCall = false;
                        pnode->sxCall.isEvalCall = fCallIsEval;
                        pnode->sxCall.argCount = count;
                        pnode->ichLim = m_pscan->IchLimTok();
                    }
                    else
                    {
                        if (pToken->tk == tkID && pToken->pid == wellKnownPropertyPids.eval) // Detect eval
                        {
                            this->MarkEvalCaller();
                        }
                        pToken->tk = tkNone; // This is no longer an identifier
                    }
                }
                ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ers) _PARSENODE_EXTENSIONS_ACTUAL(ichMin));
#if PARSENODE_EXTENSIONS
                if(buildAST && LanguageServiceMode())
                {
                    m_languageServiceExtension->SetRParen(pnode, ichMin);
                }
#endif
                break;
            }
        case tkLBrack:
            {
#if PARSENODE_EXTENSIONS
                charcount_t ichLBrackMin = m_pscan->IchMinTok();
                charcount_t ichRBrackMin = 0;
#endif
                m_pscan->Scan();
                ParseNodePtr pnodeExpr = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ersRBrack | ers));
                if (buildAST)
                {
                    pnode = CreateBinNode(knopIndex, pnode, pnodeExpr);
                    pnode->ichLim = m_pscan->IchLimTok();
                }
                else
                {
                    pToken->tk = tkNone; // This is no longer an identifier
                }
                ChkCurTok(tkRBrack, ERRnoRbrack _ERROR_RECOVERY_ACTUAL(ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRBrackMin));

#if PARSENODE_EXTENSIONS
                // Keep ichLBrackMin and ichRBrackMin if needed
                if (buildAST && LanguageServiceMode())
                {
                    m_languageServiceExtension->SetLBrack(pnode, ichLBrackMin);
                    m_languageServiceExtension->SetRBrack(pnode, ichRBrackMin);
                }
#endif

                if (!buildAST)
                {
                    break;
                }

                bool shouldConvertToDot = false;
                if (pnode->sxBin.pnode2->nop == knopStr)
                {
#if PARSENODE_EXTENSIONS
                    if (LanguageServiceMode())
                    {
                        shouldConvertToDot = true;
                    }
                    else
#endif
                    {
                        // if the string is empty or contains escape character, we will not convert them to dot node
                        shouldConvertToDot = pnode->sxBin.pnode2->sxPid.pid->Cch() > 0 && !m_pscan->IsEscapeOnLastTkStrCon();
                    }
                }

                if (shouldConvertToDot)
                {
                    LPCOLESTR str = pnode->sxBin.pnode2->sxPid.pid->Psz();
                    // See if we can convert o["p"] into o.p and o["0"] into o[0] since they're equivalent and the latter forms
                    // are faster
                    uint32 uintValue;
                    if(Js::JavascriptOperators::TryConvertToUInt32(
                           str,
                           pnode->sxBin.pnode2->sxPid.pid->Cch(),
                           &uintValue) &&
                       !Js::TaggedInt::IsOverflow(uintValue)) // the optimization is not very useful if the number can't be represented as an TaggedInt
                    {
                        // No need to verify that uintValue != JavascriptArray::InvalidIndex since all nonnegative TaggedInts are valid indexes
                        auto intNode = CreateIntNodeWithScanner(uintValue); // implicit conversion from uint32 to long
#if PARSENODE_EXTENSIONS
                        if (LanguageServiceMode())
                        {
                            intNode->ichMin = pnode->sxBin.pnode2->ichMin;
                            intNode->ichLim = pnode->sxBin.pnode2->ichLim;
                        }
#endif
                        pnode->sxBin.pnode2 = intNode;
                    }
                    // Field optimization (see GlobOpt::KillLiveElems) checks for value being a Number,
                    // and since NaN/Infinity is a number it won't kill o.NaN/o.Infinity which would cause a problem
                    // if we decide to hoist o.NaN/o.Infinity.
                    // We need to keep o["NaN"] and o["+/-Infinity"] as array element access (we don't hoist that but we may hoist field access),
                    // so no matter if it's killed by o[x] inside a loop, we make sure that we never hoist these.
                    // We need to follow same logic for strings that convert to a floating point number.
                    else
                    {
                        bool doConvertToProperty = false;    // Convert a["x"] -> a.x.
#if LANGUAGE_SERVICE
                        if (LanguageServiceMode())
                        {
                            doConvertToProperty = true;
                        }
                        else
#endif
                        {
                            if (!Parser::IsNaNOrInfinityLiteral<true>(str))
                            {
                                const OLECHAR* terminalChar;
                                double dbl = StrToDbl(str, &terminalChar, m_scriptContext);
                                bool convertsToFloat = !Js::NumberUtilities::IsNan(dbl);
                                doConvertToProperty = !convertsToFloat;
                            }
                        }

                        if (doConvertToProperty)
                        {
                            pnode->sxBin.pnode2->nop = knopName;
                            pnode->nop = knopDot;
                            pnode->grfpn |= PNodeFlags::fpnIndexOperator;
                        }
                    }
                }
            }
            break;

        case tkDot:
            {
            ParseNodePtr name = NULL;
#if ERROR_RECOVERY
            bool consumeIdent = true;
            charcount_t dotLocation = m_pscan->IchLimTok();
#endif
            OpCode opCode = knopDot;

            m_pscan->Scan();
            if (!m_token.IsIdentifier())
            {
                //allow reserved words in ES5 mode (but if not immediately after a new line in LanguageServiceMode)
                if (!(m_token.IsReservedWord())
#if ERROR_RECOVERY
                    || (buildAST && LanguageServiceMode() && m_pscan->FHadNewLine())
#endif
                    )
                {
#if ERROR_RECOVERY                    
                    name = CreateNameNode(m_pidError);
                    name->ichLim = name->ichMin = dotLocation;
#endif
                    IdentifierExpectedError(m_token);
#if ERROR_RECOVERY
                    SKIP(ERROR_RECOVERY_ACTUAL(ers | ersStmtStart));
                    consumeIdent = false;
#endif
                }
            }
            // Note: see comment above about field optimization WRT NaN/Infinity/-Infinity.
            // Convert a.Nan, a.Infinity into a["NaN"], a["Infinity"].
            // We don't care about -Infinity case here because x.-Infinity is invalid in javascript.
            // Both NaN and Infinity are identifiers.
            else if (buildAST &&
#if LANGUAGE_SERVICE
                    !LanguageServiceMode() &&
#endif
                    Parser::IsNaNOrInfinityLiteral<false>(m_token.GetIdentifier(m_phtbl)->Psz()))
            {
                opCode = knopIndex;
            }

            if (buildAST)
            {
#if ERROR_RECOVERY
                if (!name)
#endif
                {
                    if (opCode == knopDot)
                    {
                        name = CreateNameNode(m_token.GetIdentifier(m_phtbl));
                    }
                    else
                    {
                        Assert(opCode == knopIndex);
                        name = CreateStrNodeWithScanner(m_token.GetIdentifier(m_phtbl));
                    }
                }
                pnode = CreateBinNode(opCode, pnode, name);
            }
            else
            {
                pToken->tk = tkNone;
            }

            
#if ERROR_RECOVERY
            if (consumeIdent)
#endif
                m_pscan->Scan();

            break;
            }

        case tkStrTmplBasic:
        case tkStrTmplBegin:
            {
                Assert(m_scriptContext->GetConfig()->IsES6StringTemplateEnabled());

                ParseNode* templateNode = ParseStringTemplateDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) pnode);

                if (!buildAST)
                {
                    pToken->tk = tkNone; // This is no longer an identifier
                }

                pnode = templateNode;

                break;
            }
        default:
            return pnode;
        }
    }
}

/***************************************************************************
Look for an existing label with the given name.
***************************************************************************/
ParseNodePtr Parser::PnodeLabel(IdentPtr pid, ParseNodePtr pnodeLabels)
{
    AssertMem(pid);
    AssertNodeMemN(pnodeLabels);

    StmtNest *pstmt;
    ParseNodePtr pnodeT;

    // Look in the statement stack.
    for (pstmt = m_pstmtCur; NULL != pstmt; pstmt = pstmt->pstmtOuter)
    {
        AssertNodeMem(pstmt->pnodeStmt);
        AssertNodeMemN(pstmt->pnodeLab);

        for (pnodeT = pstmt->pnodeLab; NULL != pnodeT;
            pnodeT = pnodeT->sxLabel.pnodeNext)
        {
            Assert(knopLabel == pnodeT->nop);
            if (pid == pnodeT->sxLabel.pid)
                return pnodeT;
        }
    }

    // Also look in the pnodeLabels list.
    for (pnodeT = pnodeLabels; NULL != pnodeT;
        pnodeT = pnodeT->sxLabel.pnodeNext)
    {
        Assert(knopLabel == pnodeT->nop);
        if (pid == pnodeT->sxLabel.pid)
            return pnodeT;
    }

    return NULL;
}

// Currently only ints/floats are treated as constants in function call
// TODO: Check if we need for other constants as well
BOOL Parser::IsConstantInFunctionCall(ParseNodePtr pnode)
{
    if (pnode->nop == knopInt && !Js::TaggedInt::IsOverflow(pnode->sxInt.lw))
    {
        return TRUE;
    }

    if (pnode->nop == knopFlt)
    {
        return TRUE;
    }

    return FALSE;
}

/***************************************************************************
Parse a list of arguments.
***************************************************************************/
template<bool buildAST>
ParseNodePtr Parser::ParseArgList(ERROR_RECOVERY_FORMAL_  bool *pCallOfConstants, uint16 *pSpreadArgCount, uint16 * pCount)
{
    ParseNodePtr pnodeArg;
    ParseNodePtr pnodeList = NULL;
    ParseNodePtr *lastNodeRef = NULL;

    // Check for an empty list
    Assert(m_token.tk == tkLParen);

    if (m_pscan->Scan() == tkRParen)
    {
        return NULL;
    }

    *pCallOfConstants = true;
    *pSpreadArgCount = 0;

    int count=0;
    while (true)
    {
        // the count of arguments has to fit in an unsigned short
        if (count > 0xffffU)
            Error(ERRnoMemory);
        // Allow spread in argument lists.
        pnodeArg = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) koplCma, NULL, TRUE, /* fAllowEllipsis */TRUE);

        if (buildAST)
        {
#if PARSENODE_EXTENSIONS
            if(LanguageServiceMode())
            {
                // Store the full argument span to be used in finding paramter index
                m_languageServiceExtension->SetArgLim(pnodeArg, m_pscan->IchMinTok());
            }
#endif
            this->CheckArguments(pnodeArg);

            if (*pCallOfConstants && !IsConstantInFunctionCall(pnodeArg))
            {
                *pCallOfConstants = false;
            }

            if (pnodeArg->nop == knopEllipsis)
            {
                (*pSpreadArgCount)++;
            }

            ++count;
            AddToNodeListEscapedUse(&pnodeList, &lastNodeRef, pnodeArg);
        }
        if (m_token.tk != tkComma)
        {
            break;
        }
        m_pscan->Scan();
    }

    if (pSpreadArgCount!=nullptr && (*pSpreadArgCount) > 0){
        CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(SpreadFeatureCount, m_scriptContext);
    }

    if (buildAST)
    {
#if ERROR_RECOVERY
        if (LanguageServiceMode() && m_token.tk != tkRParen)
        {
            // If we will report an error record if the last parameter is a function declaration.
            // This notifies the language service it might want to hoist function declarations out
            // of the parameter list if they are marked IsSubsumed(). The actual hoisting isn't
            // performed here because this call might be in a deeply nested expression, and the
            // level it needs to be hoisted is not easily accessible here and making it easily
            // hoisted would slow down non-language service uses; plus the langauge service already
            // has the machinary to rewrite  the tree that the parser otherwise doesn't need.
            if (pnodeArg->nop == knopFncDecl)
            {
                pnodeArg->sxFnc.SetSubsumed();
                m_hasSubsumedFunction = true;
            }
        }
#endif
        *pCount = (uint16)count;
        AssertMem(lastNodeRef);
        AssertNodeMem(*lastNodeRef);
        pnodeList->ichLim = (*lastNodeRef)->ichLim;
    }

    return pnodeList;
}

// Currently only ints are treated as constants in ArrayLiterals
BOOL Parser::IsConstantInArrayLiteral(ParseNodePtr pnode)
{
    if (pnode->nop == knopInt && !Js::TaggedInt::IsOverflow(pnode->sxInt.lw))
    {
        return TRUE;
    }
    return FALSE;
}

template<bool buildAST>
ParseNodePtr Parser::ParseArrayLiteral(ERROR_RECOVERY_FORMAL)
{
    ParseNodePtr pnode = NULL;
    bool arrayOfTaggedInts = false;
    bool arrayOfInts = false;
    bool arrayOfNumbers = false;
    bool hasMissingValues = false;
    uint count = 0;
    uint spreadCount = 0;

    ParseNodePtr pnode1 = ParseArrayList<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) &arrayOfTaggedInts, &arrayOfInts, &arrayOfNumbers, &hasMissingValues, &count, &spreadCount);

    if (buildAST)
    {
        pnode = CreateNodeWithScanner<knopArray>();
        pnode->sxArrLit.pnode1 = pnode1;
        pnode->sxArrLit.arrayOfTaggedInts = arrayOfTaggedInts;
        pnode->sxArrLit.arrayOfInts = arrayOfInts;
        pnode->sxArrLit.arrayOfNumbers = arrayOfNumbers;
        pnode->sxArrLit.hasMissingValues = hasMissingValues;
        pnode->sxArrLit.count = count;
        pnode->sxArrLit.spreadCount = spreadCount;

        if (pnode->sxArrLit.pnode1)
        {
            this->CheckArguments(pnode->sxArrLit.pnode1);
        }
    }

    return pnode;
}

/***************************************************************************
Create a ArrayLiteral node
Parse a list of array elements. [ a, b, , c, ]
***************************************************************************/
template<bool buildAST>
ParseNodePtr Parser::ParseArrayList(ERROR_RECOVERY_FORMAL_ bool *pArrayOfTaggedInts, bool *pArrayOfInts, bool *pArrayOfNumbers, bool *pHasMissingValues, uint *count, uint *spreadCount)
{
    ParseNodePtr pnodeArg = NULL;
    ParseNodePtr pnodeList = NULL;
    ParseNodePtr *lastNodeRef = NULL;

    *count = 0;

    // Check for an empty list
    if (tkRBrack == m_token.tk)
    {
        return NULL;
    }

    this->m_arrayDepth++;
    bool arrayOfTaggedInts = buildAST;
    bool arrayOfInts = buildAST;
    bool arrayOfNumbers = buildAST;
    bool arrayOfVarInts = false;
    bool hasMissingValues = false;

    for (;;)
    {
        (*count)++;
        if (tkComma == m_token.tk || tkRBrack == m_token.tk)
        {
            hasMissingValues = true;
            arrayOfTaggedInts = false;
            arrayOfInts = false;
            arrayOfNumbers = false;
            if (buildAST)
            {
                pnodeArg = CreateNodeWithScanner<knopEmpty>();
            }
        }
        else
        {
            // Allow Spread in array literals.
            pnodeArg = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) koplCma, NULL, TRUE, /* fAllowEllipsis */ TRUE);
            if (buildAST)
            {
                if (pnodeArg->nop == knopEllipsis)
                {
                    (*spreadCount)++;
                }
                this->CheckArguments(pnodeArg);
            }
        }

#if DEBUG
        if(m_grfscr & fscrEnforceJSON && !IsJSONValid(pnodeArg))
        {
            Error(ERRsyntax);
        }
#endif

        if (buildAST)
        {
            if (arrayOfNumbers)
            {
                if (pnodeArg->nop != knopInt)
                {
                    arrayOfTaggedInts = false;
                    if (pnodeArg->nop != knopFlt)
                    {
                        // Not an array of constants.
                        arrayOfInts = false;
                        arrayOfNumbers = false;
                    }
                    else if (arrayOfInts && Js::JavascriptNumber::IsInt32OrUInt32(pnodeArg->sxFlt.dbl) && (!Js::JavascriptNumber::IsInt32(pnodeArg->sxFlt.dbl) || pnodeArg->sxFlt.dbl == -2147483648.0))
                    {
                        // We've seen nothing but ints, and this is a uint32 but not an int32.
                        // Unless we see an actual float at some point, we want an array of vars
                        // so we can work with tagged ints.
                        arrayOfVarInts = true;
                    }
                    else
                    {
                        // Not an int array, but it may still be a float array.
                        arrayOfInts = false;
                    }
                }
                else
                {
                    if (Js::SparseArraySegment<int32>::IsMissingItem((int32*)&pnodeArg->sxInt.lw))
                    {
                        arrayOfInts = false;
                    }
                    if (Js::TaggedInt::IsOverflow(pnodeArg->sxInt.lw))
                    {
                        arrayOfTaggedInts = false;
                    }
                }
            }
            AddToNodeListEscapedUse(&pnodeList, &lastNodeRef, pnodeArg);
        }

        if (tkComma != m_token.tk)
        {
            break;
        }
        m_pscan->Scan();

        if((tkRBrack == m_token.tk))
        {
            break;
        }
    }

    if (spreadCount != nullptr && *spreadCount > 0){
        CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(SpreadFeatureCount, m_scriptContext);
    }

    if (buildAST)
    {
        AssertMem(lastNodeRef);
        AssertNodeMem(*lastNodeRef);
        pnodeList->ichLim = (*lastNodeRef)->ichLim;

        if (arrayOfVarInts && arrayOfInts)
        {
            arrayOfInts = false;
            arrayOfNumbers = false;
        }
        *pArrayOfTaggedInts = arrayOfTaggedInts;
        *pArrayOfInts = arrayOfInts;
        *pArrayOfNumbers = arrayOfNumbers;
        *pHasMissingValues = hasMissingValues;
    }
    this->m_arrayDepth--;
    return pnodeList;
}

Parser::MemberNameToTypeMap* Parser::CreateMemberNameMap(ArenaAllocator* pAllocator)
{
    Assert(pAllocator);
    return Anew(pAllocator, MemberNameToTypeMap, pAllocator, 5);
}

template<bool buildAST> void Parser::ParseComputedName(ERROR_RECOVERY_FORMAL_ ParseNodePtr* ppnodeName, LPCOLESTR* ppNameHint, LPCOLESTR* ppFullNameHint, ulong *returnLength)
{
    m_pscan->Scan();
    ParseNodePtr pnodeNameExpr = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) koplNo, nullptr, TRUE, FALSE, *ppNameHint, returnLength);
    if (buildAST)
    {
        *ppnodeName = CreateNodeT<knopComputedName>(pnodeNameExpr->ichMin, pnodeNameExpr->ichLim);
        (*ppnodeName)->sxUni.pnode1 = pnodeNameExpr;
    }

    if (ppFullNameHint && buildAST && CONFIG_FLAG(UseFullName))
    {
        *ppFullNameHint = FormatPropertyString(*ppNameHint, pnodeNameExpr, returnLength);
    }

    ChkCurTokNoScan(tkRBrack, ERRsyntax _ERROR_RECOVERY_ACTUAL(ers));
}

/***************************************************************************
    Parse a list of object set\get members. e.g. { get foo(){ ... }, set bar(arg) { ... } }
***************************************************************************/
template<bool buildAST>
ParseNodePtr Parser::ParseMemberGetSet(ERROR_RECOVERY_FORMAL_ OpCode nop, LPCOLESTR* ppNameHint)
{
    ParseNodePtr pnodeName = nullptr;
    Assert(nop == knopGetMember || nop == knopSetMember);
    AssertMem(ppNameHint);
    IdentPtr pid = nullptr;
    bool isComputedName = false;

    *ppNameHint=nullptr;

    switch(m_token.tk)
    {
    default:
        if (!m_token.IsReservedWord())
        {
            Error(ERRnoMemberIdent);
#if ERROR_RECOVERY
RecoverFromERRnoMemberIdent:
            SKIP(ERROR_RECOVERY_ACTUAL( (ersLParen | ers) & ~ersBinOp));
            pnodeName = CreateStrNodeWithScanner(m_pidError);
            pid = m_pidError;
            *ppNameHint = pid->Psz();
            break;
#endif
        }
        // fall through
    case tkID:
        pid = m_token.GetIdentifier(m_phtbl);
        *ppNameHint = pid->Psz();
        if (buildAST)
        {
            pnodeName = CreateStrNodeWithScanner(pid);
        }
        break;
    case tkStrCon:
        if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
        {
            Error(ERRES5NoOctal);
        }
        pid = m_token.GetStr();
        *ppNameHint = pid->Psz();
        if (buildAST)
        {
            pnodeName = CreateStrNodeWithScanner(pid);
        }
        break;

    case tkIntCon:
        if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
        {
            Error(ERRES5NoOctal);
        }

        pid = m_pscan->PidFromLong(m_token.GetLong());
        if (buildAST)
        {
            pnodeName = CreateStrNodeWithScanner(pid);
        }
        break;

    case tkFltCon:
        if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
        {
            Error(ERRES5NoOctal);
        }

        pid = m_pscan->PidFromDbl(m_token.GetDouble());
        if (buildAST)
        {
            pnodeName = CreateStrNodeWithScanner(pid);
        }
        break;

    case tkLBrack:
        // Computed property name: get|set [expr] () {  }
        if (!m_scriptContext->GetConfig()->IsES6ObjectLiteralsEnabled())
        {
            Error(ERRnoMemberIdent);
#if ERROR_RECOVERY
            goto RecoverFromERRnoMemberIdent;
#endif
        }
        LPCOLESTR emptyHint = nullptr;
        ParseComputedName<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) &pnodeName, &emptyHint, ppNameHint);

        isComputedName = true;
        break;
    }

    MemberType memberType;
    ushort flags;
    if(nop == knopGetMember)
    {
        memberType = MemberTypeGetter;
        flags = fFncNoArg | fFncNoName;
    }
    else
    {
        Assert(nop == knopSetMember);
        memberType = MemberTypeSetter;
        flags = fFncOneArg | fFncNoName;
    }

    ParseNodePtr pnodeFnc = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) flags | fFncMethod | (nop == knopSetMember ? fFncSetter : fFncNoFlgs), *ppNameHint);

    if (buildAST)
    {
        pnodeFnc->sxFnc.SetIsAccessor();
        return CreateBinNode(nop, pnodeName, pnodeFnc);
    }
    else
    {
        return NULL;
    }
}

/***************************************************************************
Parse a list of object members. e.g. { x:foo, 'y me':bar }
***************************************************************************/
template<bool buildAST>
ParseNodePtr Parser::ParseMemberList(ERROR_RECOVERY_FORMAL_ LPCOLESTR pNameHint, ulong* pNameHintLength)
{
    ParseNodePtr pnodeArg;
    ParseNodePtr pnodeName = NULL;
    ParseNodePtr pnodeList = NULL;
    ParseNodePtr *lastNodeRef = NULL;   
    LPCOLESTR pFullNameHint = NULL;       // A calculated fullname
    ulong fullNameHintLength = pNameHintLength ? *pNameHintLength : 0;
    bool isProtoDeclared = false;

    // Check for an empty list
    if (tkRCurly == m_token.tk)
    {
        return NULL;
    }
#if PARSENODE_EXTENSIONS
    charcount_t ichDeadRangeMin = m_pscan->IchMinTok();
    charcount_t ichDeadRangeLim = 0;
#endif

    ArenaAllocator tempAllocator(L"MemberNames", m_nodeAllocator.GetPageAllocator(), Parser::OutOfMemory);

    for (;;)
    {
        bool isComputedName = false;
#if DEBUG
        if((m_grfscr & fscrEnforceJSON) && (tkStrCon != m_token.tk || !(m_pscan->IsDoubleQuoteOnLastTkStrCon())))
        {
            Error(ERRsyntax);
        }
#endif
        bool isGenerator = m_scriptContext->GetConfig()->IsES6GeneratorsEnabled() &&
                           m_token.tk == tkStar;
        ushort fncDeclFlags = fFncNoName | fFncMethod;
        if (isGenerator)
        {
            m_pscan->ScanForcingPid();
            fncDeclFlags |= fFncGenerator;
        }

        IdentPtr pidHint = NULL;              // A name scoped to current expression
        Token tkHint = m_token;
        charcount_t idHintIchMin = static_cast<charcount_t>(m_pscan->IecpMinTok());
        charcount_t idHintIchLim = static_cast< charcount_t >(m_pscan->IecpLimTok());
        bool wrapInBrackets = false;
        switch (m_token.tk)
        {
        default:
            if (!m_token.IsReservedWord())
            {
                Error(ERRnoMemberIdent);
#if ERROR_RECOVERY
RecoverFromERRnoMemberIdent:
                SKIP(ERROR_RECOVERY_ACTUAL(ersColon | (ers & ~ersBinOp)));
                pnodeName = CreateStrNodeWithScanner(m_pidError);
                pidHint = m_pidError;
                break;
#endif
            }
            // allow reserved words
            wrapInBrackets = true;
            // fall thru
        case tkID:
            pidHint = m_token.GetIdentifier(m_phtbl);            
            if (buildAST)
            {
                pnodeName = CreateStrNodeWithScanner(pidHint);
            }
            break;
        case tkStrCon:
            if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
            {
                Error(ERRES5NoOctal);
            }
            wrapInBrackets = true;
            pidHint = m_token.GetStr();            
            if (buildAST)
            {
                pnodeName = CreateStrNodeWithScanner(pidHint);
            }
            break;      
        case tkIntCon:
            // Object initializers with numeric labels allowed in JS6
            if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
            {
                Error(ERRES5NoOctal);
            }

            pidHint = m_pscan->PidFromLong(m_token.GetLong());
            if (buildAST)
            {
                pnodeName = CreateStrNodeWithScanner(pidHint);
            }
            break;

        case tkFltCon:
            if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
            {
                Error(ERRES5NoOctal);
            }

            pidHint = m_pscan->PidFromDbl(m_token.GetDouble());
            if (buildAST)
            {
                pnodeName = CreateStrNodeWithScanner(pidHint);
            }
            wrapInBrackets = true;
            break;

        case tkLBrack:
            // Computed property name: [expr] : value
            if (!m_scriptContext->GetConfig()->IsES6ObjectLiteralsEnabled())
            {
                Error(ERRnoMemberIdent);
#if ERROR_RECOVERY
                goto RecoverFromERRnoMemberIdent;
#endif
            }

            ParseComputedName<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) &pnodeName, &pNameHint, &pFullNameHint, &fullNameHintLength);

            isComputedName = true;
            break;
        }

        if (pFullNameHint == nullptr)
        {
            if (CONFIG_FLAG(UseFullName))
            {
                if ((pidHint == wellKnownPropertyPids.getter || pidHint == wellKnownPropertyPids.setter) && tkHint.tk == tkID &&
                    m_scriptContext->GetConfig()->IsES6FunctionNameEnabled()
                   )
                {
                    pFullNameHint = AppendNameHints(pidHint, pNameHint, &fullNameHintLength, true);
                }
                else
                {
                    pFullNameHint = AppendNameHints(pNameHint, pidHint, &fullNameHintLength, false, wrapInBrackets);
                }
            }
            else
            {
                pFullNameHint = pidHint? pidHint->Psz() : nullptr;
                fullNameHintLength = pidHint ? pidHint->Cch() : 0;
            }
        }

        RestorePoint atPid;
        m_pscan->Capture(&atPid);

#if ERROR_RECOVERY
        if (m_token.tk != tkColon)
#endif
            m_pscan->ScanForcingPid();

        if (isGenerator && m_token.tk != tkLParen)
        {
            Error(ERRnoLparen);
        }

        if (tkColon == m_token.tk)
        {
#if PARSENODE_EXTENSIONS
            ichDeadRangeLim = m_pscan->IchMinTok();
#endif

            // It is a syntax error is the production of the form __proto__ : <> occurs more than once. From B.3.1 in spec.
            // Note that previous scan is important because only after that we can determine we have a variable.
            if (!isComputedName && pidHint == wellKnownPropertyPids.__proto__)
            {
                if (isProtoDeclared)
                {
                    Error(ERRsyntax);
#if ERROR_RECOVERY
                    pnodeName = CreateStrNodeWithScanner(m_pidError);
#endif
                }
                else
                {
                    isProtoDeclared = true;
                }
            }

            m_pscan->Scan();
            ParseNodePtr pnodeExpr = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) koplCma, NULL, TRUE, FALSE, pFullNameHint, &fullNameHintLength);
#if DEBUG
            if((m_grfscr & fscrEnforceJSON) && !IsJSONValid(pnodeExpr))
            {
                Error(ERRsyntax);
            }
#endif
            if (buildAST)
            {
                pnodeArg = CreateBinNode(knopMember, pnodeName, pnodeExpr);
                if (pnodeArg->sxBin.pnode1->nop == knopStr)
                {
                    pnodeArg->sxBin.pnode1->sxPid.pid->PromoteAssignmentState();
                }
            }
        }
        else if (m_token.tk == tkLParen && m_scriptContext->GetConfig()->IsES6ObjectLiteralsEnabled())
        {
            // Shorthand syntax: foo() {} -> foo: function() {}

            // Rewind to the PID and parse a function expression.
            m_pscan->SeekTo(atPid);
            ParseNodePtr pnodeFunc = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) fncDeclFlags, pFullNameHint);

            if (buildAST)
            {
                pnodeArg = CreateBinNode(knopMember, pnodeName, pnodeFunc);
            }
        }
        else if (NULL != pidHint) //Its either tkID/tkStrCon/tkFloatCon/tkIntCon
        {
            Assert(pidHint->Psz() != NULL);
            if (pidHint == wellKnownPropertyPids.getter && tkHint.tk == tkID)
            {
#if PARSENODE_EXTENSIONS
                ichDeadRangeLim = m_pscan->IchLimTok();
#endif
                LPCOLESTR pNameGet = nullptr;
                pnodeArg = ParseMemberGetSet<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) knopGetMember, &pNameGet);
                if (CONFIG_FLAG(UseFullName) && buildAST && pnodeArg->sxBin.pnode2->nop == knopFncDecl)
                {
                    if (m_scriptContext->GetConfig()->IsES6FunctionNameEnabled())
                    {
                        // displays as get object.funcname
                        pFullNameHint = AppendNameHints(wellKnownPropertyPids.getter, AppendNameHints(pNameHint, pNameGet, &fullNameHintLength), &fullNameHintLength, true);
                    }
                    else
                    {
                        // displays as object.funcname.get
                        pFullNameHint = AppendNameHints(pNameHint, AppendNameHints(pNameGet, wellKnownPropertyPids.getter, &fullNameHintLength), &fullNameHintLength);
                    }
                }
#if PARSENODE_EXTENSIONS
                if (buildAST && LanguageServiceMode())
                    pnodeArg->ichMin = pnodeName->ichMin;
#endif
            }
            else if (pidHint == wellKnownPropertyPids.setter && tkHint.tk == tkID)
            {
#if PARSENODE_EXTENSIONS
                ichDeadRangeLim = m_pscan->IchLimTok();
#endif
                LPCOLESTR pNameSet = nullptr;
                pnodeArg = ParseMemberGetSet<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) knopSetMember, &pNameSet);
                if (CONFIG_FLAG(UseFullName) && buildAST && pnodeArg->sxBin.pnode2->nop == knopFncDecl)
                {
                    if (m_scriptContext->GetConfig()->IsES6FunctionNameEnabled())
                    {
                        // displays as set object.funcname
                        pFullNameHint = AppendNameHints(wellKnownPropertyPids.setter, AppendNameHints(pNameHint, pNameSet, &fullNameHintLength), &fullNameHintLength, true);
                    }
                    else
                    {
                        // displays as object.funcname.set
                        pFullNameHint = AppendNameHints(pNameHint, AppendNameHints(pNameSet, wellKnownPropertyPids.setter, &fullNameHintLength), &fullNameHintLength);
                    }
                }

#if PARSENODE_EXTENSIONS
                if (buildAST && LanguageServiceMode())
                    pnodeArg->ichMin = pnodeName->ichMin;
#endif
            }
            else if (
#if ERROR_RECOVERY
                pidHint != m_pidError && 
#endif
                (m_token.tk == tkRCurly || m_token.tk == tkComma) && m_scriptContext->GetConfig()->IsES6ObjectLiteralsEnabled()) {
                // Shorthand {foo} -> {foo:foo} syntax.
#if PARSENODE_EXTENSIONS
                ichDeadRangeLim = m_pscan->IchMinTok();
#endif

                if (tkHint.tk != tkID)
                {
                    Assert(tkHint.IsReservedWord() 
                        || tkHint.tk == tkIntCon || tkHint.tk == tkFltCon || tkHint.tk == tkStrCon);
                    // All keywords are banned in non-strict mode.
                    // Future reserved words are banned in strict mode.
                    if (IsStrictMode() || !tkHint.IsFutureReservedWord(true))
                    {
                        IdentifierExpectedError(tkHint);
                    }
                }

                if (buildAST)
                {
                    CheckArgumentsUse(pidHint, GetCurrentFunctionNode());

                    ParseNodePtr pnodeIdent = CreateNameNode(pidHint, idHintIchMin, idHintIchLim);
                    BlockInfoStack *blockInfo = GetCurrentFunctionBlockInfo();
                    PidRefStack *ref = this->FindOrAddPidRef(pidHint, blockInfo->pnodeBlock->sxBlock.blockId);
                    pnodeIdent->sxPid.SetSymRef(ref);

                    pnodeArg = CreateBinNode(knopMemberShort, pnodeName, pnodeIdent);
                }
            }
            else
            {
                Error(ERRnoColon);
#if ERROR_RECOVERY
                SKIP(ERROR_RECOVERY_ACTUAL(ersExprStart | ers));
                pnodeArg = CreateBinNode(knopMember, pnodeName, ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) koplCma));
#endif
#if PARSENODE_EXTENSIONS
                ichDeadRangeLim = m_pscan->IchLimTok();
#endif
            }
        }
        else
        {
            Error(ERRnoColon);
#if ERROR_RECOVERY
            SKIP(ERROR_RECOVERY_ACTUAL(ersExprStart | ers));
            pnodeArg = CreateBinNode(knopMember, pnodeName, ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) koplCma));
#endif
        }

        if (buildAST)
        {
#if PARSENODE_EXTENSIONS
            if (LanguageServiceMode() && ichDeadRangeLim > 0)
            {
                m_languageServiceExtension->SetCompletionRange(ichDeadRangeMin, ichDeadRangeLim, LanguageServiceExtension::CompletionRangeMode::ObjectLiteralNames);
                ichDeadRangeMin = m_pscan->IchLimTok();
                ichDeadRangeLim = 0;
            }
#endif
            Assert(pnodeArg->sxBin.pnode2 != NULL);
            if (pnodeArg->sxBin.pnode2->nop == knopFncDecl)
            {
                pnodeArg->sxBin.pnode2->sxFnc.hint = pFullNameHint;
                pnodeArg->sxBin.pnode2->sxFnc.hintLength = fullNameHintLength;
            }
            AddToNodeListEscapedUse(&pnodeList, &lastNodeRef, pnodeArg);
        }
        pidHint = NULL;
        pFullNameHint = NULL;
        if (tkComma != m_token.tk)
        {
            break;
        }
        m_pscan->ScanForcingPid();
        if (tkRCurly == m_token.tk)
        {
            break;
        }
    }

    if (buildAST)
    {
        AssertMem(lastNodeRef);
        AssertNodeMem(*lastNodeRef);
        pnodeList->ichLim = (*lastNodeRef)->ichLim;
    }

    return pnodeList;
}

BOOL Parser::DeferredParse(Js::LocalFunctionId functionId)
{
    if ((m_grfscr & fscrDeferFncParse) != 0)
    {
        if (m_stoppedDeferredParse)
        {
            return false;
        }
        if (PHASE_OFF_RAW(Js::DeferParsePhase, m_sourceContextInfo->sourceContextId, functionId))
        {
            return false;
        }
        if (PHASE_FORCE_RAW(Js::DeferParsePhase, m_sourceContextInfo->sourceContextId, functionId))
        {
            return true;
        }
#ifndef DISABLE_DYNAMIC_PROFILE_DEFER_PARSE
        if (m_sourceContextInfo->sourceDynamicProfileManager != null)
        {
            Js::ExecutionFlags flags = m_sourceContextInfo->sourceDynamicProfileManager->IsFunctionExecuted(functionId);
            return flags != Js::ExecutionFlags_Executed;
        }
#endif
        return true;
    }

    return false;
}

//
// Call this in ParseFncDecl only to check (and reset) if ParseFncDecl is reparsing a deferred
// func body. If a deferred func is called and being reparsed, it shouldn't be deferred again.
//
BOOL Parser::IsDeferredFnc()
{
    if (m_grfscr & fscrDeferredFnc)
    {
        m_grfscr &= ~fscrDeferredFnc;
        return true;
    }

    return false;
}

template<bool buildAST>
ParseNodePtr Parser::ParseFncDecl(ERROR_RECOVERY_FORMAL_ ushort flags, LPCOLESTR pNameHint, const bool isSourceElement, const bool needsPIDOnRCurlyScan, bool fUnaryOrParen)
{
    ParseNodePtr pnodeFnc = nullptr;
    ParseNodePtr *ppnodeVarSave = nullptr;
    ParseNodePtr pnodeFncSave = nullptr;
    ParseNodePtr pnodeFncBlockScope = nullptr;
    ParseNodePtr *ppnodeScopeSave = nullptr;
    ParseNodePtr *ppnodeExprScopeSave = nullptr;
    bool funcHasName = false;
    bool fDeclaration = flags & fFncDeclaration;
    charcount_t ichMin = this->m_pscan->IchMinTok();
    bool wasInDeferredNestedFunc = false;
    CatchPidRefList *catchPidRefList = nullptr;

    uint tryCatchOrFinallyDepthSave = this->m_tryCatchOrFinallyDepth;
    this->m_tryCatchOrFinallyDepth = 0;

    CheckStrictModeFncDeclNotSourceElement(isSourceElement, fDeclaration);

    if (this->m_arrayDepth)
    {
        this->m_funcInArrayDepth++; // Count func depth within array literal
    }

    Js::LocalFunctionId nextFunctionIdSave = 0;
    if (!buildAST && m_nextFunctionId != nullptr)
    {
        nextFunctionIdSave = *m_nextFunctionId;
    }

    // Update the count of functions nested in the current parent.
    Assert(m_pnestedCount || !buildAST);
    uint *pnestedCountSave = m_pnestedCount;
    if (buildAST || m_pnestedCount)
    {
        (*m_pnestedCount)++;
    }

    uint scopeCountNoAstSave = m_scopeCountNoAst;
    m_scopeCountNoAst = 0;

    long* pAstSizeSave = m_pCurrentAstSize;

    if (buildAST || BindDeferredPidRefs())
    {
        if (fDeclaration && m_scriptContext->GetConfig()->IsBlockScopeEnabled())
        {
            bool needsBlockNode =
                (m_pstmtCur->isDeferred && m_pstmtCur->op != knopBlock) ||
                (!m_pstmtCur->isDeferred && m_pstmtCur->pnodeStmt->nop != knopBlock);

            if (needsBlockNode)
            {
                // Well, hell. We have a function declaration like "if (a) function f() {}". We didn't see
                // a block scope on the way in, so we need to pretend we did.
                pnodeFncBlockScope = StartParseBlock<buildAST>(PnodeBlockType::Regular, ScopeType_Block);
                if (buildAST)
                {
                    PushFuncBlockScope(pnodeFncBlockScope, &ppnodeScopeSave, &ppnodeExprScopeSave);
                }
            }
        }

        // Create the node.
        pnodeFnc = CreateNode(knopFncDecl);
        pnodeFnc->sxFnc.ClearFlags();
        pnodeFnc->sxFnc.SetDeclaration(fDeclaration);
        pnodeFnc->sxFnc.astSize             = 0;
        pnodeFnc->sxFnc.pnodeNames          = nullptr;
        pnodeFnc->sxFnc.pnodeScopes         = nullptr;
        pnodeFnc->sxFnc.pnodeRest           = nullptr;
        pnodeFnc->sxFnc.pid                 = nullptr;
        pnodeFnc->sxFnc.hint                = nullptr;
        pnodeFnc->sxFnc.hintLength          = 0;
        pnodeFnc->sxFnc.isNameIdentifierRef = true;
        pnodeFnc->sxFnc.pnodeNext           = nullptr;
        pnodeFnc->sxFnc.pnodeArgs           = nullptr;
        pnodeFnc->sxFnc.pnodeVars           = nullptr;
        pnodeFnc->sxFnc.funcInfo            = nullptr;
        pnodeFnc->sxFnc.deferredStub        = nullptr;
        pnodeFnc->sxFnc.nestedCount         = 0;
        pnodeFnc->sxFnc.cbMin = m_pscan->IecpMinTok();
        pnodeFnc->sxFnc.functionId = (*m_nextFunctionId)++;

        // Push new parser state with this new function node

        AppendFunctionToScopeList(fDeclaration, pnodeFnc);

        // Start the argument list.
        ppnodeVarSave = m_ppnodeVar;
    }
    else
    {
        (*m_nextFunctionId)++;
    }

    if (buildAST)
    {
        pnodeFnc->sxFnc.lineNumber = m_pscan->LineCur();
        pnodeFnc->sxFnc.columnNumber = CalculateFunctionColumnNumber();
        pnodeFnc->sxFnc.SetNested(m_currentNodeFunc != nullptr); // If there is a current function, then we're a nested function.
        pnodeFnc->sxFnc.SetStrictMode(IsStrictMode()); // Inherit current strict mode -- may be overridden by the function itself if it contains a strict mode directive.
        pnodeFnc->sxFnc.firstDefaultArg = 0;

        m_pCurrentAstSize = &pnodeFnc->sxFnc.astSize;

        // Make this the current function and start its sub-function list.
        pnodeFncSave = m_currentNodeFunc;
        m_currentNodeFunc = pnodeFnc;

        m_pnestedCount = &pnodeFnc->sxFnc.nestedCount;

        catchPidRefList = this->GetCatchPidRefList();
        if (catchPidRefList)
        {
            Assert(!m_scriptContext->GetConfig()->IsBlockScopeEnabled());
            if (fDeclaration)
            {
                // We're starting a function declaration, and we're inside some number
                // of catches, and the catch has its own scope but the function gets hoisted
                // outside it. We have to fiddle with the PidRefStack's to simulate hoisting.
                // For each catch object in scope here, do the following:
                // - Remove the portion of the pid ref stack that holds references inside the catch.
                // - Save that portion of the stack in the catchPidRef list entry. Do this by:
                //     - Letting the list entry point to the current top of the pid ref stack;
                //     - Setting the prev pointer of the pid ref at the bottom of the removed portion to null.
                // Now we can accumulate references inside the function declaration without getting them
                // interspersed with the references that should bind to the catch variable.
                FOREACH_SLISTBASE_ENTRY(CatchPidRef, catchPidRef, catchPidRefList)
                {
                    IdentPtr pidCatch = catchPidRef.pid;
                    PidRefStack *topRef = pidCatch->GetTopRef();
                    PidRefStack *catchScopeRef = catchPidRef.ref;
                    catchPidRef.ref = topRef;
                    pidCatch->SetTopRef(catchScopeRef->prev);
                    catchScopeRef->prev = nullptr;
                }
                NEXT_SLISTBASE_ENTRY;
                catchPidRefList->Reverse();
            }

            this->SetCatchPidRefList(nullptr);
        }
    }
    else // if !buildAST
    {
        wasInDeferredNestedFunc = m_inDeferredNestedFunc;
        m_inDeferredNestedFunc = true;

        if (BindDeferredPidRefs())
        {
            pnodeFncSave = m_currentNodeDeferredFunc;
            m_currentNodeDeferredFunc = pnodeFnc;

            m_pnestedCount = &pnodeFnc->sxFnc.nestedCount;
        }
        else
        {
            m_pnestedCount = nullptr;
        }
    }

    if (buildAST || BindDeferredPidRefs())
    {
        pnodeFnc->sxFnc.SetIsLambda((flags & fFncLambda) != 0);
        pnodeFnc->sxFnc.SetIsMethod((flags & fFncMethod) != 0);
        pnodeFnc->sxFnc.SetIsClassMember((flags & fFncClassMember) != 0);
    }

    bool needScanRCurly = true;
    bool result = ParseFncDeclHelper<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) pnodeFnc, pnodeFncSave, pNameHint, flags, &funcHasName, isSourceElement, fUnaryOrParen, &needScanRCurly);
    if (!result)
    {
        Assert(!pnodeFncBlockScope);

#if ERROR_RECOVERY
        Assert((m_grfscr & fscrFunctionHeaderOnly) != 0);
        if (buildAST || BindDeferredPidRefs())
        {
            if (buildAST)
            {
                m_currentNodeFunc = pnodeFncSave;
            }
            else
            {
                m_currentNodeDeferredFunc = pnodeFncSave;
            }
        }
#endif

        return pnodeFnc;
    }

    if (buildAST || BindDeferredPidRefs())
    {
        *m_ppnodeVar = nullptr;
        m_ppnodeVar = ppnodeVarSave;

        // Restore the current function.
        if (buildAST)
        {
            Assert(pnodeFnc == m_currentNodeFunc);

            m_currentNodeFunc = pnodeFncSave;
            m_pCurrentAstSize = pAstSizeSave;
        }
        else
        {
            Assert(pnodeFnc == m_currentNodeDeferredFunc);

            m_currentNodeDeferredFunc = pnodeFncSave;
            if (m_currentNodeFunc && pnodeFnc->sxFnc.HasWithStmt())
            {
                GetCurrentFunctionNode()->sxFnc.SetHasWithStmt(true);
            }
        }
        if (m_currentNodeFunc && (pnodeFnc->sxFnc.CallsEval() || pnodeFnc->sxFnc.ChildCallsEval()))
        {
            GetCurrentFunctionNode()->sxFnc.SetChildCallsEval(true);
        }

        // Lambdas do not have "arguments" and instead capture their parent's
        // binding of "arguments.  To ensure the arguments object of the enclosing
        // non-lambda function is loaded propagate the UsesArguments flag up to
        // the parent function
        if ((flags & fFncLambda) != 0 && pnodeFnc->sxFnc.UsesArguments())
        {
            if (pnodeFncSave != nullptr)
            {
                pnodeFncSave->sxFnc.SetUsesArguments();
            }
            else
            {
                m_UsesArgumentsAtGlobal = true;
            }
        }
    }

    if (needScanRCurly
#if LANGUAGE_SERVICE
        && m_token.tk != tkExternalSourceEnd
#endif
        )
    {
        // Consume the next token now that we're back in the enclosing function (whose strictness may be
        // different from the function we just finished).
#if DBG
        bool expectedTokenValid = m_token.tk == tkRCurly;
#ifdef LANGUAGE_SERVICE
        expectedTokenValid |= m_token.tk == tkEOF;
#endif
        AssertMsg(expectedTokenValid, "Invalid token expected for RCurly match");
#endif
        // The next token may need to have a PID created in !buildAST mode, as we may be parsing a method with a string name.
        if (needsPIDOnRCurlyScan)
        {
            m_pscan->ScanForcingPid();
        }
        else
        {
            m_pscan->Scan();
        }
    }

    m_pnestedCount = pnestedCountSave;
    Assert(!buildAST || !wasInDeferredNestedFunc);
    m_inDeferredNestedFunc = wasInDeferredNestedFunc;

    if (this->m_arrayDepth)
    {
        this->m_funcInArrayDepth--;
        if (this->m_funcInArrayDepth == 0 && !this->m_parsingDuplicate)
        {
            // We disable deferred parsing if array literals dominate.
            // But don't do this if the array literal is dominated by function bodies.
            if (flags & (fFncMethod | fFncClassMember) && m_token.tk != tkSColon
#if ERROR_RECOVERY
                && m_token.tk != tkEOF
#endif
                )
            {
                // Class member methods have optional separators. We need to check whether we are
                // getting the IchLim of the correct token.
                Assert(m_pscan->m_tkPrevious == tkRCurly && needScanRCurly);

                this->m_funcInArray += m_pscan->IchMinTok() - /*tkRCurly*/ 1 - ichMin;
            }
            else
            {
                this->m_funcInArray += m_pscan->IchLimTok() - ichMin;
            }
        }
    }

    m_scopeCountNoAst = scopeCountNoAstSave;

    if (buildAST)
    {
        if (catchPidRefList)
        {
            if (this->GetCatchPidRefList())
            {
                // We may have had catches inside the functon we just finished. If so, we should be done
                // with them all (so the ref list should be empty), and we can throw away the list.
                Assert(this->GetCatchPidRefList()->Empty());
                Adelete(&m_nodeAllocator, this->GetCatchPidRefList());
            }
            this->SetCatchPidRefList(catchPidRefList);

            if (fDeclaration)
            {
                // We're finishing a function declaration inside a catch. For each catch variable that's in
                // scope here, put the portion of the pid ref stack that we removed and saved back on the top
                // of the stack. When we finish the catch, the references in this restored portion of the stack
                // will be bound to the catch variable, but those that belong the function body will
                // be left behind to be bound to the context outside the catch.
                FOREACH_SLISTBASE_ENTRY(CatchPidRef, catchPidRef, catchPidRefList)
                {
                    IdentPtr pidCatch = catchPidRef.pid;
                    PidRefStack *oldTopRef = pidCatch->GetTopRef();
                    PidRefStack *ref = catchPidRef.ref;
                    pidCatch->SetTopRef(ref);
                    while (ref->prev)
                    {
                        ref = ref->prev;
                    }
                    ref->prev = oldTopRef;
                    catchPidRef.ref = ref;
                }
                NEXT_SLISTBASE_ENTRY;
                catchPidRefList->Reverse();
            }
        }
    }

    if (buildAST && fDeclaration && m_scriptContext->GetConfig()->IsBlockScopeEnabled() && !IsStrictMode())
    {
        if (pnodeFnc->sxFnc.pnodeNames != nullptr && pnodeFnc->sxFnc.pnodeNames->nop == knopVarDecl &&
            GetCurrentBlock()->sxBlock.blockType == PnodeBlockType::Regular)
        {
            // Add a function scoped var decl with the same name as the function for
            // back compat with pre-ES6 code that declares functions in blocks. The
            // idea is that the last executed declaration wins at the function scope
            // level and we accomplish this by having each block scoped function
            // declaration assign to both the block scoped "let" binding, as well
            // as the function scoped "var" binding.
            ParseNodePtr vardecl = CreateVarDeclNode(pnodeFnc->sxFnc.pnodeNames->sxVar.pid, STVariable, false, nullptr, false);
            vardecl->sxVar.isBlockScopeFncDeclVar = true;
        }
    }

    if (pnodeFncBlockScope)
    {
        Assert(pnodeFncBlockScope->sxBlock.pnodeStmt == nullptr);
        pnodeFncBlockScope->sxBlock.pnodeStmt = pnodeFnc;
        if (buildAST)
        {
            PopFuncBlockScope(ppnodeScopeSave, ppnodeExprScopeSave);
        }
        FinishParseBlock(pnodeFncBlockScope);
        return pnodeFncBlockScope;
    }

    this->m_tryCatchOrFinallyDepth = tryCatchOrFinallyDepthSave;

    return pnodeFnc;
}

uint Parser::CalculateFunctionColumnNumber()
{
    uint columnNumber;

    if (m_pscan->IchMinTok() >= m_pscan->IchMinLine())
    {
        // In scenarios involving defer parse IchMinLine() can be incorrect for the first line after defer parse
        columnNumber = m_pscan->IchMinTok() - m_pscan->IchMinLine();
        if (m_functionBody != nullptr && m_functionBody->GetRelativeLineNumber() == m_pscan->LineCur())
        {
            // Adjust the column if it falls on the first line, where the reparse is happening.
            columnNumber += m_functionBody->GetRelativeColumnNumber();
        }
    }
    else if (m_currentNodeFunc)
    {
        // For the first line after defer parse, compute the column relative to the column number
        // of the lexically parent function.
        ULONG offsetFromCurrentFunction = m_pscan->IchMinTok() - m_currentNodeFunc->ichMin;
        columnNumber = m_currentNodeFunc->sxFnc.columnNumber + offsetFromCurrentFunction ;
    }
    else
    {
        // if there is no current function, lets give a default of 0.
        columnNumber = 0;
    }

    return columnNumber;
}

void Parser::AppendFunctionToScopeList(bool fDeclaration, ParseNodePtr pnodeFnc)
{
    if (!fDeclaration && m_ppnodeExprScope)
    {
        // We're tracking function expressions separately from declarations in this scope
        // (e.g., inside a catch scope in standards mode).
        Assert(*m_ppnodeExprScope == nullptr);
        *m_ppnodeExprScope = pnodeFnc;
        m_ppnodeExprScope = &pnodeFnc->sxFnc.pnodeNext;
    }
    else
    {
        Assert(*m_ppnodeScope == nullptr);
        *m_ppnodeScope = pnodeFnc;
        m_ppnodeScope = &pnodeFnc->sxFnc.pnodeNext;
    }
}

/***************************************************************************
Parse a function definition.
***************************************************************************/
template<bool buildAST>
bool Parser::ParseFncDeclHelper(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, ParseNodePtr pnodeFncParent, LPCOLESTR pNameHint, ushort flags, bool *pHasName, bool isSourceElement, bool fUnaryOrParen, bool *pNeedScanRCurly)
{
    bool fDeclaration = (flags & fFncDeclaration) != 0;
    bool fLambda = (flags & fFncLambda) != 0;
    bool fDeferred = false;
    StmtNest *pstmtSave;
    ParseNodePtr *lastNodeRef = nullptr;
#if PARSENODE_EXTENSIONS
    charcount_t ichDeadRangeMin = m_pscan->IchLimTok();
#endif
    bool fFunctionInBlock = false;
    if (buildAST)
    {
        fFunctionInBlock = GetCurrentBlockInfo() != GetCurrentFunctionBlockInfo() &&
            (GetCurrentBlockInfo()->pnodeBlock->sxBlock.scope == nullptr ||
             GetCurrentBlockInfo()->pnodeBlock->sxBlock.scope->GetScopeType() != ScopeType_GlobalEvalBlock);
    }

    // Save the position of the scanner incase we need to inspect the name hint later
    RestorePoint beginNameHint;
    m_pscan->Capture(&beginNameHint);

    ParseNodePtr pnodeFncExprScope = nullptr;
    Scope *fncExprScope = nullptr;
    if ((buildAST || BindDeferredPidRefs()) &&
        !fDeclaration)
    {
        pnodeFncExprScope = StartParseBlock<buildAST>(PnodeBlockType::Function, ScopeType_FuncExpr);
        fncExprScope = pnodeFncExprScope->sxBlock.scope;
    }

    *pHasName = !fLambda && this->ParseFncNames<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) pnodeFnc, pnodeFncParent, flags, isSourceElement, &lastNodeRef);

    // switch scanner to treat 'yield' as keyword in generator functions
    // or as an identifier in non-generator functions
    bool fPreviousYieldIsKeyword = m_pscan->SetYieldIsKeyword(pnodeFnc && pnodeFnc->sxFnc.IsGenerator());

    if (pnodeFnc && pnodeFnc->sxFnc.IsGenerator())
    {
        CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(GeneratorCount, m_scriptContext);
    }

    if (fncExprScope && !*pHasName)
    {
#if ERROR_RECOVERY == 0
        // Error recovery during name parsing can add to the scope.
        Assert(fncExprScope->Count() == 0);
#endif
        FinishParseBlock(pnodeFncExprScope);
        m_nextBlockId--;
        Adelete(&m_nodeAllocator, fncExprScope);
        fncExprScope = nullptr;
        pnodeFncExprScope = nullptr;
    }
    if (pnodeFnc)
    {
        pnodeFnc->sxFnc.scope = fncExprScope;
    }

    // Start a new statement stack.
    pstmtSave = m_pstmtCur;
    SetCurrentStatement(nullptr);

    RestorePoint beginFormals;
    m_pscan->Capture(&beginFormals);
    BOOL fWasAlreadyStrictMode = IsStrictMode();
    BOOL oldStrictMode = this->m_fUseStrictMode;

    if (fLambda)
    {
        // lambda formals are parsed in strict mode always
        m_fUseStrictMode = TRUE;
        CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(LambdaCount, m_scriptContext);
    }

    uint uDeferSave = m_grfscr & fscrDeferFncParse;
    if ((!fDeclaration && m_ppnodeExprScope) ||
        (m_scriptContext->GetConfig()->IsBlockScopeEnabled() && fFunctionInBlock) ||
        (flags & (fFncNoName | fFncLambda)))
    {
        // NOTE: Don't defer if this is a function expression inside a construct that induces
        // a scope nested within the current function (like a with, or a catch in ES5 mode, or
        // any function declared inside a nested lexical block in ES6 mode).
        // We won't be able to reconstruct the scope chain properly when we come back and
        // try to compile just the function expression.
        // Also shut off deferring on getter/setter or other construct with unusual text bounds
        // (fFncNoName|fFncLambda) as these are usually trivial, and re-parsing is problematic.
        m_grfscr &= ~fscrDeferFncParse;
    }


    bool isTopLevelDeferredFunc = false;

    struct AutoFastScanFlag {
        bool savedDoingFastScan;
        AutoFastScanFlag(Parser *parser) : m_parser(parser) { savedDoingFastScan = m_parser->m_doingFastScan; }
        ~AutoFastScanFlag() { m_parser->m_doingFastScan = savedDoingFastScan; }
        Parser *m_parser;
    } flag(this);

    bool doParallel = false;
    bool parallelJobStarted = false;
    if (buildAST)
    {
        bool isLikelyModulePattern =
            !fDeclaration && pnodeFnc && pnodeFnc->sxFnc.pnodeNames == nullptr && fUnaryOrParen;

        BOOL isDeferredFnc = IsDeferredFnc();
        isTopLevelDeferredFunc = (!isDeferredFnc
            && DeferredParse(pnodeFnc->sxFnc.functionId)
            && (!pnodeFnc->sxFnc.IsNested() || CONFIG_FLAG(DeferNested))
            // Don't defer if this is a function expression not contained in a statement or other expression.
            // Assume it will be called as part of this expression.
            // Note that in compat mode a named function expression has to be treated like a declaration statement.
            && (fDeclaration
            || (this->m_pstmtCur != nullptr &&
            (this->m_pstmtCur->pnodeStmt->nop != knopBlock ||
            (this->m_pstmtCur->pnodeStmt->sxBlock.blockType != PnodeBlockType::Function &&
            this->m_pstmtCur->pnodeStmt->sxBlock.blockType != PnodeBlockType::Global)))
            || this->m_exprDepth > 1)
            && !m_InAsmMode
            );

        if (!fLambda &&
            !isDeferredFnc &&
            !isLikelyModulePattern &&
            !this->IsBackgroundParser() &&
            !this->m_doingFastScan &&
            !(pnodeFncParent && m_currDeferredStub) &&
            !(this->m_parseType == ParseType_Deferred && this->m_functionBody && this->m_functionBody->GetScopeInfo() && !isTopLevelDeferredFunc))
        {
            doParallel = DoParallelParse(pnodeFnc);
            if (doParallel)
            {
                BackgroundParser *bgp = m_scriptContext->GetBackgroundParser();
                Assert(bgp);
                if (bgp->HasFailedBackgroundParseItem())
                {
                    Error(ERRsyntax);
                }
                doParallel = bgp->ParseBackgroundItem(this, pnodeFnc, isTopLevelDeferredFunc);
                if (doParallel)
                {
                    parallelJobStarted = true;
                    this->m_hasParallelJob = true;
                    this->m_doingFastScan = true;
                    doParallel = FastScanFormalsAndBody();
                    if (doParallel)
                    {
                        // Let the foreground thread take care of marking the limit on the function node,
                        // because in some cases this function's caller will want to change that limit,
                        // so we don't want the background thread to try and touch it.
                        pnodeFnc->ichLim = m_pscan->IchLimTok();
                        pnodeFnc->sxFnc.cbLim = m_pscan->IecpLimTok();
                    }
                }
            }
        }
    }

    if (!doParallel)
    {
        // We don't want to, or couldn't, let the main thread scan past this function body, so parse
        // it for real.
        ParseNodePtr pnodeRealFnc = pnodeFnc;
        if (parallelJobStarted)
        {
            // We have to deal with a failure to fast-scan the function (due to syntax error? "/"?) when
            // a background thread may already have begun to work on the job. Both threads can't be allowed to
            // operate on the same node.
            pnodeFnc = CreateDummyFuncNode(fDeclaration);
        }

        ParseNodePtr pnodeBlock = nullptr;
        if (buildAST || BindDeferredPidRefs())
        {
            pnodeBlock = StartParseBlock<buildAST>(PnodeBlockType::Parameter, ScopeType_Parameter);
            pnodeFnc->sxFnc.pnodeScopes = pnodeBlock;
            m_ppnodeVar = &pnodeFnc->sxFnc.pnodeArgs;
        }

        ParseNodePtr *ppnodeScopeSave = nullptr;
        ParseNodePtr *ppnodeExprScopeSave = nullptr;

        ppnodeScopeSave = m_ppnodeScope;
        if (pnodeBlock)
        {
            // This synthetic block scope will contain all the nested scopes.
            m_ppnodeScope = &pnodeBlock->sxBlock.pnodeScopes;
            pnodeBlock->sxBlock.pnodeStmt = pnodeFnc;
        }

        // Keep nested function declarations and expressions in the same list at function scope.
        // (Indicate this by nulling out the current function expressions list.)
        ppnodeExprScopeSave = m_ppnodeExprScope;
        m_ppnodeExprScope = nullptr;

        this->ParseFncFormals<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) pnodeFnc, flags);
        m_fUseStrictMode = oldStrictMode;

#if PARSENODE_EXTENSIONS
        if (buildAST && LanguageServiceMode())
        {
            // TODO: What about lambda's with their => token?
            m_languageServiceExtension->SetCompletionRange(ichDeadRangeMin + 1, m_pscan->IchMinTok(), LanguageServiceExtension::CompletionRangeMode::Others);
        }
#endif

#if ERROR_RECOVERY
        if (!fLambda && (m_grfscr & fscrFunctionHeaderOnly) != 0)
        {
#if PARSENODE_EXTENSIONS
            if (m_token.tk == tkRParen && buildAST && LanguageServiceMode())
            {
                // The language service needs to where the '{' is.
                m_pscan->Scan();
                if (m_token.tk == tkLCurly)
                {
                    m_languageServiceExtension->SetLCurly(pnodeFnc, m_pscan->IchMinTok());

                    // Language service needs to know where the beginning of the first token after the '{' is
                    // in order to know when to stop looking for meta-data comments for the function.
                    m_pscan->Scan();
                    pnodeFnc->ichLim = m_pscan->IchMinTok();
                }
            }
#endif
            m_token.tk = tkEOF; // Stop all parsing
            if (pnodeBlock)
            {
                FinishParseBlock(pnodeBlock);
            }

            SetCurrentStatement(pstmtSave);

            if (pnodeFncExprScope)
            {
                FinishParseFncExprScope(pnodeFnc, pnodeFncExprScope);
            }
            return false;
        }
#endif

        // Create function body scope
        ParseNodePtr pnodeInnerBlock = nullptr;
        if (buildAST || BindDeferredPidRefs())
        {
            pnodeInnerBlock = StartParseBlock<buildAST>(PnodeBlockType::Function, ScopeType_FunctionBody);
            // Set the parameter block's child to the function body block.
            *m_ppnodeScope = pnodeInnerBlock;
            pnodeFnc->sxFnc.pnodeBodyScope = pnodeInnerBlock;

            // This synthetic block scope will contain all the nested scopes.
            m_ppnodeScope = &pnodeInnerBlock->sxBlock.pnodeScopes;
            pnodeInnerBlock->sxBlock.pnodeStmt = pnodeFnc;
        }

        // Avoing below code under Language Service mode - in order to quick avoid huge perf regression.

        // DEFER: Begin deferral here (after names are parsed and name nodes created).
        // Create no more AST nodes until we're done.

        // Try to defer this func if all these are true:
        //  0. We are not already in deferred parsing (i.e. buildAST is true)
        //  1. We are not reparsing a deferred func which is being invoked.
        //  2. Dynamic profile suggests this func can be deferred (and deferred parse is on).
        //  3. This func is top level or defer nested func is on.
        //  4. The function is non-nested and not in eval, or the deferral decision was based on cached profile info,
        //     or the function is sufficiently long. (I.e., don't defer little nested functions unless we're
        //     confident they'll never be executed, because undeferring nested functions is more expensive.)

        // We will also temporarily defer all asm.js functions, except for the asm.js
        // module itself, which we will never defer
        bool strictModeTurnedOn = false;

#ifndef LANGUAGE_SERVICE
        if (isTopLevelDeferredFunc &&
            !(this->m_grfscr & fscrEvalCode) &&
            pnodeFnc->sxFnc.IsNested() &&
#ifndef DISABLE_DYNAMIC_PROFILE_DEFER_PARSE
            m_sourceContextInfo->sourceDynamicProfileManager == nullptr &&
#endif
            !PHASE_OFF_RAW(Js::ScanAheadPhase, m_sourceContextInfo->sourceContextId, pnodeFnc->sxFnc.functionId) &&
            (
                !PHASE_FORCE_RAW(Js::DeferParsePhase, m_sourceContextInfo->sourceContextId, pnodeFnc->sxFnc.functionId) ||
                PHASE_FORCE_RAW(Js::ScanAheadPhase, m_sourceContextInfo->sourceContextId, pnodeFnc->sxFnc.functionId)
            ))
        {
            // Try to scan ahead to the end of the function. If we get there before we've scanned a minimum
            // number of tokens, don't bother deferring, because it's too small.
            if (this->ScanAheadToFunctionEnd(CONFIG_FLAG(MinDeferredFuncTokenCount)))
            {
                isTopLevelDeferredFunc = false;
            }
        }
#endif

        if (isTopLevelDeferredFunc || (m_InAsmMode && m_deferAsmJs))
        {
            AssertMsg(!fLambda, "Deferring function parsing of a function does not handle lambda syntax");
            fDeferred = true;

            this->ParseTopLevelDeferredFunc(ERROR_RECOVERY_ACTUAL_(ers) pnodeFnc, pnodeFncParent, pNameHint);

#if PARSENODE_EXTENSIONS
            if(buildAST && LanguageServiceMode())
            {
                // Preserve the nested count
                m_languageServiceExtension->SetNestedCount(pnodeFnc, pnodeFnc->sxFnc.nestedCount);
            }
#endif
        }
        else
        {
#if PARSENODE_EXTENSIONS
            // Keep ichRParenMin if needed
            if (buildAST && LanguageServiceMode() && m_token.tk == tkRParen)
                m_languageServiceExtension->SetRParen(pnodeFnc, m_pscan->IchMinTok());
#endif

            if (m_token.tk == tkRParen) // This might be false due to error recovery or lambda.
            {
                m_pscan->Scan();
            }

            if (fLambda)
            {
                BOOL hadNewLine = m_pscan->FHadNewLine();

                // it can be the case we do not have a fat arrow here if there is a valid expression on the left hand side
                // of the fat arrow, but that expression does not parse as a parameter list.  E.g.
                //    a.x => { }
                // Therefore check for it and error if not found.
                // LS Mode : since this is a lambda we supposed to get the fat arrow, if not we will skip till we get that fat arrow.
                ChkCurTok(tkDArrow, ERRnoDArrow _ERROR_RECOVERY_ACTUAL(ersDArrow));

                // Newline character between arrow parameters and fat arrow is a syntax error but we want to check for
                // this after verifying there was a => token. Otherwise we would throw the wrong error.
                if (hadNewLine)
                {
                    Error(ERRsyntax);
                }

#if ERROR_RECOVERY
                if (m_token.tk == tkDArrow)
                {
                    m_pscan->Scan();
                }
#endif
            }

            if (buildAST || BindDeferredPidRefs())
            {
                // Shouldn't be any temps in the arg list.
                Assert(*m_ppnodeVar == nullptr);

                // Start the var list.
                pnodeFnc->sxFnc.pnodeVars = nullptr;
                m_ppnodeVar = &pnodeFnc->sxFnc.pnodeVars;
            }

            // Keep nested function declarations and expressions in the same list at function scope.
            // (Indicate this by nulling out the current function expressions list.)
            m_ppnodeExprScope = nullptr;

            if (buildAST)
            {
                DeferredFunctionStub *saveCurrentStub = m_currDeferredStub;
                if (pnodeFncParent && m_currDeferredStub)
                {
                    m_currDeferredStub = (m_currDeferredStub + (pnodeFncParent->sxFnc.nestedCount - 1))->deferredStubs;
                }

                if (m_token.tk != tkLCurly && fLambda)
                {
                    ParseExpressionLambdaBody<true>(ERROR_RECOVERY_ACTUAL_(ers) pnodeFnc);
                    *pNeedScanRCurly = false;
                }
                else
                {
                    this->FinishFncDecl(ERROR_RECOVERY_ACTUAL_(ers) pnodeFnc, pNameHint, lastNodeRef);
                }
                m_currDeferredStub = saveCurrentStub;
            }
            else
            {
                this->ParseNestedDeferredFunc(ERROR_RECOVERY_ACTUAL_(ers) pnodeFnc, fLambda, pNeedScanRCurly, &strictModeTurnedOn);
            }
        }

        if (pnodeInnerBlock)
        {
            FinishParseBlock(pnodeInnerBlock, *pNeedScanRCurly);
        }

        if ((buildAST || BindDeferredPidRefs()) && !(m_token.tk != tkLCurly && fLambda))
        {
            this->AddArgumentsNodeToVars(pnodeFnc);
        }

            // Restore the lists of scopes that contain function expressions.

#if PARSENODE_EXTENSIONS
        if (buildAST && LanguageServiceMode())
        {
            m_languageServiceExtension->SetNestedCount(pnodeFnc, pnodeFnc->sxFnc.nestedCount);
        }
#endif

        Assert(m_ppnodeExprScope == nullptr || *m_ppnodeExprScope == nullptr);
        m_ppnodeExprScope = ppnodeExprScopeSave;

        AssertMem(m_ppnodeScope);
        Assert(nullptr == *m_ppnodeScope);
        m_ppnodeScope = ppnodeScopeSave;

        if (pnodeBlock)
        {
            FinishParseBlock(pnodeBlock, *pNeedScanRCurly);
        }

        if (IsStrictMode() || strictModeTurnedOn)
        {
            this->m_fUseStrictMode = TRUE; // Now we know this function is in strict mode

            if (!fLambda && !fWasAlreadyStrictMode)
            {
                // If this function turned on strict mode then we didn't check the formal
                // parameters or function name hint for future reserved word usage. So do that now.
                // Except for lambdas which always treat formal parameters as strict and do not have
                // a name.
                RestorePoint afterFnc;
                m_pscan->Capture(&afterFnc);

                if (*pHasName)
                {
                    // Rewind to the function name hint and check if the token is a reserved word.
                    m_pscan->SeekTo(beginNameHint);
                    m_pscan->Scan();
                    if (pnodeFnc->sxFnc.IsGenerator())
                    {
                        Assert(m_token.tk == tkStar);
                        Assert(m_scriptContext->GetConfig()->IsES6GeneratorsEnabled());
                        Assert(!(flags & fFncClassMember));
                        m_pscan->Scan();
                    }
                    if (m_token.IsReservedWord())
                    {
                        IdentifierExpectedError(m_token);
                    }
                    CheckStrictModeEvalArgumentsUsage(m_token.GetIdentifier(m_phtbl));
                }

                // Fast forward to formal parameter list, check for future reserved words,
                // then restore scanner as it was.
                m_pscan->SeekTo(beginFormals);
                CheckStrictFormalParameters(ERROR_RECOVERY_ACTUAL(ers));
                m_pscan->SeekTo(afterFnc);
            }

            // REVIEW: Is it enough that we checked in the above if already?
            if (buildAST)
            {
                // TODO REVIEW: There is a bug here, deferred parsing of function eval::x() { "use strict"; } won't check for the illegal usage of 'eval'
                if (pnodeFnc->sxFnc.pnodeNames != nullptr && knopVarDecl == pnodeFnc->sxFnc.pnodeNames->nop)
                {
                    CheckStrictModeEvalArgumentsUsage(pnodeFnc->sxFnc.pnodeNames->sxVar.pid, pnodeFnc->sxFnc.pnodeNames);
                }
            }

            this->m_fUseStrictMode = oldStrictMode;
            CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(StrictModeFunctionCount, m_scriptContext);
        }

        if (fDeferred)
        {
            pnodeFnc->sxFnc.pnodeVars = nullptr;
        }

        if (parallelJobStarted)
        {
            pnodeFnc = pnodeRealFnc;
            m_currentNodeFunc = pnodeRealFnc;

            // Let the foreground thread take care of marking the limit on the function node,
            // because in some cases this function's caller will want to change that limit,
            // so we don't want the background thread to try and touch it.
            pnodeFnc->ichLim = m_pscan->IchLimTok();
            pnodeFnc->sxFnc.cbLim = m_pscan->IecpLimTok();
        }
    }

    // after parsing asm.js module, we want to reset asm.js state before continuing
    if (pnodeFnc->sxFnc.GetAsmjsMode())
    {
        m_InAsmMode = false;
    }

    // Restore the statement stack.
    Assert(nullptr == m_pstmtCur);
    SetCurrentStatement(pstmtSave);

    if (pnodeFncExprScope)
    {
        FinishParseFncExprScope(pnodeFnc, pnodeFncExprScope);
    }
    if (!m_stoppedDeferredParse)
    {
        m_grfscr |= uDeferSave;
    }


    m_pscan->SetYieldIsKeyword(fPreviousYieldIsKeyword);

    return true;
}

void Parser::ParseTopLevelDeferredFunc(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, ParseNodePtr pnodeFncParent, LPCOLESTR pNameHint)
{
    // Parse a function body that is a transition point from building AST to doing fast syntax check.

    pnodeFnc->sxFnc.pnodeVars = nullptr;
    pnodeFnc->sxFnc.pnodeBody = nullptr;

    this->m_deferringAST = TRUE;

    // Put the scanner into "no hashing" mode.
    BYTE deferFlags = m_pscan->SetDeferredParse(TRUE);

#if ERROR_RECOVERY
    if (m_token.tk == tkRParen) // This might be false due to error recovery.
#endif
        m_pscan->Scan();

    ChkCurTok(tkLCurly, ERRnoLcurly _ERROR_RECOVERY_ACTUAL(ers));

    ParseNodePtr *ppnodeVarSave = m_ppnodeVar;

    if (BindDeferredPidRefs())
    {
        m_ppnodeVar = &pnodeFnc->sxFnc.pnodeVars;

    }

    if (pnodeFncParent != nullptr
        && m_currDeferredStub != nullptr
        // We don't create stubs for function bodies in parameter scope.
        && pnodeFnc->sxFnc.pnodeScopes->sxBlock.blockType != PnodeBlockType::Parameter)
    {
        // We've already parsed this function body for syntax errors on the initial parse of the script.
        // We have information that allows us to skip it, so do so.

        DeferredFunctionStub *stub = m_currDeferredStub + (pnodeFncParent->sxFnc.nestedCount - 1);
        Assert(pnodeFnc->ichMin == stub->ichMin);
        if (stub->fncFlags & kFunctionCallsEval)
        {
            this->MarkEvalCaller();
        }
        if (stub->fncFlags & kFunctionChildCallsEval)
        {
            pnodeFnc->sxFnc.SetChildCallsEval(true);
        }
        if (stub->fncFlags & kFunctionHasWithStmt)
        {
            pnodeFnc->sxFnc.SetHasWithStmt(true);
        }

        PHASE_PRINT_TRACE1(
            Js::SkipNestedDeferredPhase,
            L"Skipping nested deferred function %d. %s: %d...%d\n",
            pnodeFnc->sxFnc.functionId, GetFunctionName(pnodeFnc, pNameHint), pnodeFnc->ichMin, stub->restorePoint.m_ichMinTok);

        m_pscan->SeekTo(stub->restorePoint, m_nextFunctionId);
        pnodeFnc->sxFnc.nestedCount = stub->nestedCount;
        pnodeFnc->sxFnc.deferredStub = stub->deferredStubs;
        if (stub->fncFlags & kFunctionStrictMode)
        {
            pnodeFnc->sxFnc.SetStrictMode(true);
        }
    }
    else
    {
        ParseStmtList<false>(ERROR_RECOVERY_ACTUAL_(ers) nullptr, nullptr, SM_DeferedParse, true /* isSourceElementList */);
    }

    pnodeFnc->ichLim = m_pscan->IchLimTok();
    pnodeFnc->sxFnc.cbLim = m_pscan->IecpLimTok();

    if (BindDeferredPidRefs())
    {
        m_ppnodeVar = ppnodeVarSave;
    }

    // Restore the scanner's default hashing mode.
    // Do this before we consume the next token.
    m_pscan->SetDeferredParseFlags(deferFlags);

    ChkCurTokNoScan(tkRCurly, ERRnoRcurly _ERROR_RECOVERY_ACTUAL(ers));

#if DBG
    pnodeFnc->sxFnc.deferredParseNextFunctionId = *this->m_nextFunctionId;
#endif
    this->m_deferringAST = FALSE;
}

bool Parser::DoParallelParse(ParseNodePtr pnodeFnc) const
{
#if ERROR_RECOVERY
    return false;
#else

    if (!PHASE_ON_RAW(Js::ParallelParsePhase, m_sourceContextInfo->sourceContextId, pnodeFnc->sxFnc.functionId))
    {
        return false;
    }

    BackgroundParser *bgp = m_scriptContext->GetBackgroundParser();
    return bgp != nullptr;

#endif
}

bool Parser::ScanAheadToFunctionEnd(uint count)
{
    bool found = false;
    uint curlyDepth = 0;

    RestorePoint funcStart;
    m_pscan->Capture(&funcStart);

    for (uint i = 0; i < count; i++)
    {
        switch (m_token.tk)
        {
            case tkStrTmplBegin:
            case tkStrTmplMid:
            case tkStrTmplEnd:
            case tkDiv:
            case tkAsgDiv:
            case tkScanError:
            case tkEOF:
                goto LEnd;

            case tkLCurly:
                UInt32Math::Inc(curlyDepth, Parser::OutOfMemory);
                break;

            case tkRCurly:
                if (curlyDepth == 1)
                {
                    found = true;
                    goto LEnd;
                }
                if (curlyDepth == 0)
                {
                    goto LEnd;
                }
                curlyDepth--;
                break;
        }

        m_pscan->ScanAhead();
    }

 LEnd:
    m_pscan->SeekTo(funcStart);
    return found;
}

bool Parser::FastScanFormalsAndBody()
{
    // The scanner is currently pointing just past the name of a function.
    // The idea here is to find the end of the function body as quickly as possible,
    // by tokenizing and tracking {}'s if possible.
    // String templates require some extra logic but can be handled.

    // The real wrinkle is "/" and "/=", which may indicate either a regex literal or a division, depending
    // on the context.
    // To handle this with minimal work, keep track of the last ";" seen at each {} depth. If we see one of the
    // difficult tokens, rewind to the last ";" at the current {} depth and parse statements until we pass the
    // point where we had to rewind. This will process the "/" as required.

    RestorePoint funcStart;
    m_pscan->Capture(&funcStart);

    const int maxRestorePointDepth = 16;
    struct FastScanRestorePoint
    {
        RestorePoint restorePoint;
        uint parenDepth;
        Js::LocalFunctionId functionId;
        int blockId;

        FastScanRestorePoint() : restorePoint(), parenDepth(0) {};
    };
    FastScanRestorePoint lastSColonAtCurlyDepth[maxRestorePointDepth];

    charcount_t ichStart = m_pscan->IchMinTok();
    uint blockIdSave = m_nextBlockId;
    uint functionIdSave = *m_nextFunctionId;
    uint curlyDepth = 0;
    uint strTmplDepth = 0;
    for (;;)
    {
        switch (m_token.tk)
        {
            case tkStrTmplBegin:
                UInt32Math::Inc(strTmplDepth, Parser::OutOfMemory);
                // Fall through

            case tkStrTmplMid:
            case tkLCurly:
                UInt32Math::Inc(curlyDepth, Parser::OutOfMemory);
                Int32Math::Inc(m_nextBlockId, &m_nextBlockId);
                break;

            case tkStrTmplEnd:
                // We can assert here, because the scanner will only return this token if we've told it we're
                // in a string template.
                Assert(strTmplDepth > 0);
                strTmplDepth--;
                break;

            case tkRCurly:
                if (curlyDepth == 1)
                {
                    Assert(strTmplDepth == 0);
                    if (PHASE_TRACE1(Js::ParallelParsePhase))
                    {
                        Output::Print(L"Finished fast seek: %d. %s -- %d...%d\n",
                                      m_currentNodeFunc->sxFnc.functionId,
                                      GetFunctionName(m_currentNodeFunc, m_currentNodeFunc->sxFnc.hint),
                                      ichStart, m_pscan->IchLimTok());
                    }
                    return true;
                }
                if (curlyDepth < maxRestorePointDepth)
                {
                    lastSColonAtCurlyDepth[curlyDepth].restorePoint.m_ichMinTok = (uint)-1;
                }
                curlyDepth--;
                if (strTmplDepth > 0)
                {
                    m_pscan->SetScanState(Scanner_t::ScanState::ScanStateStringTemplateMiddleOrEnd);
                }
                break;

            case tkSColon:
                // Track the location of the ";" (if it's outside parens, as we don't, for instance, want
                // to track the ";"'s in a for-loop header. If we find it's important to rewind within a paren
                // expression, we can do something more sophisticated.)
                if (curlyDepth < maxRestorePointDepth && lastSColonAtCurlyDepth[curlyDepth].parenDepth == 0)
                {
                    m_pscan->Capture(&lastSColonAtCurlyDepth[curlyDepth].restorePoint);
                    lastSColonAtCurlyDepth[curlyDepth].functionId = *this->m_nextFunctionId;
                    lastSColonAtCurlyDepth[curlyDepth].blockId = m_nextBlockId;
                }
                break;

            case tkLParen:
                if (curlyDepth < maxRestorePointDepth)
                {
                    UInt32Math::Inc(lastSColonAtCurlyDepth[curlyDepth].parenDepth);
                }
                break;

            case tkRParen:
                if (curlyDepth < maxRestorePointDepth)
                {
                    Assert(lastSColonAtCurlyDepth[curlyDepth].parenDepth != 0);
                    lastSColonAtCurlyDepth[curlyDepth].parenDepth--;
                }
                break;

            case tkID:
            {
                charcount_t tokLength = m_pscan->IchLimTok() - m_pscan->IchMinTok();
                // Detect the function and class keywords so we can track function ID's.
                // (In fast mode, the scanner doesn't distinguish keywords and doesn't point the token
                // to a PID.)
                // Detect try/catch/for to increment block count for them.
                switch (tokLength)
                {
                case 3:
                    if (!memcmp(m_pscan->PchMinTok(), "try", 3) || !memcmp(m_pscan->PchMinTok(), "for", 3))
                    {
                        Int32Math::Inc(m_nextBlockId, &m_nextBlockId);
                    }
                    break;
                case 5:
                    if (!memcmp(m_pscan->PchMinTok(), "catch", 5))
                    {
                        Int32Math::Inc(m_nextBlockId, &m_nextBlockId);
                    }
                    else if (!memcmp(m_pscan->PchMinTok(), "class", 5))
                    {
                        Int32Math::Inc(m_nextBlockId, &m_nextBlockId);
                        Int32Math::Inc(*this->m_nextFunctionId, (int*)this->m_nextFunctionId);
                    }
                    break;
                case 8:
                    if (!memcmp(m_pscan->PchMinTok(), "function", 8))
                    {
                        // Account for the possible func expr scope or dummy block for missing {}'s around a declaration
                        Int32Math::Inc(m_nextBlockId, &m_nextBlockId);
                        Int32Math::Inc(*this->m_nextFunctionId, (int*)this->m_nextFunctionId);
                    }
                    break;
                }
                break;
            }

            case tkDArrow:
                Int32Math::Inc(m_nextBlockId, &m_nextBlockId);
                Int32Math::Inc(*this->m_nextFunctionId, (int*)this->m_nextFunctionId);
                break;

            case tkDiv:
            case tkAsgDiv:
            {
                int opl;
                OpCode nop;
                tokens tkPrev = m_pscan->m_tkPrevious;
                if ((m_pscan->m_phtbl->TokIsBinop(tkPrev, &opl, &nop) && nop != knopNone) ||
                    (m_pscan->m_phtbl->TokIsUnop(tkPrev, &opl, &nop) &&
                     nop != knopNone &&
                     tkPrev != tkInc &&
                     tkPrev != tkDec) ||
                    tkPrev == tkColon ||
                    tkPrev == tkLParen ||
                    tkPrev == tkLBrack ||
                    tkPrev == tkRETURN)
                {
                    // Previous token indicates that we're starting an expression here and can't have a
                    // binary operator now.
                    // Assume this is a regex.
                    ParseRegExp<false>();
                    break;
                }
                uint tempCurlyDepth = curlyDepth < maxRestorePointDepth ? curlyDepth : maxRestorePointDepth - 1;
                for (; tempCurlyDepth != (uint)-1; tempCurlyDepth--)
                {
                    // We don't know whether we've got a regex or a divide. Rewind to the last safe ";"
                    // if we can and parse statements until we pass this point.
                    if (lastSColonAtCurlyDepth[tempCurlyDepth].restorePoint.m_ichMinTok != -1)
                    {
                        break;
                    }
                }
                if (tempCurlyDepth != (uint)-1)
                {
                    ParseNodePtr pnodeFncSave = m_currentNodeFunc;
                    long *pastSizeSave = m_pCurrentAstSize;
                    uint *pnestedCountSave = m_pnestedCount;
                    ParseNodePtr *ppnodeScopeSave = m_ppnodeScope;
                    ParseNodePtr *ppnodeExprScopeSave = m_ppnodeExprScope;

                    ParseNodePtr pnodeFnc = CreateDummyFuncNode(true);
                    m_ppnodeScope = &pnodeFnc->sxFnc.pnodeScopes;
                    m_ppnodeExprScope = nullptr;

                    charcount_t ichStop = m_pscan->IchLimTok();
                    curlyDepth = tempCurlyDepth;
                    m_pscan->SeekTo(lastSColonAtCurlyDepth[tempCurlyDepth].restorePoint);
                    m_nextBlockId = lastSColonAtCurlyDepth[tempCurlyDepth].blockId;
                    *this->m_nextFunctionId = lastSColonAtCurlyDepth[tempCurlyDepth].functionId;

                    ParseNodePtr pnodeBlock = StartParseBlock<true>(PnodeBlockType::Function, ScopeType_FunctionBody);

                    m_pscan->Scan();
                    do
                    {
                        ParseStatement<false>(ERROR_RECOVERY_ACTUAL_(0) true);
                    }
                    while(m_pscan->IchMinTok() < ichStop);

                    FinishParseBlock(pnodeBlock);

                    m_currentNodeFunc = pnodeFncSave;
                    m_pCurrentAstSize = pastSizeSave;
                    m_pnestedCount = pnestedCountSave;
                    m_ppnodeScope = ppnodeScopeSave;
                    m_ppnodeExprScope = ppnodeExprScopeSave;

                    // We've already consumed the first token of the next statement, so just continue
                    // without a further scan.
                    continue;
                }
            }

                // fall through to rewind to function start
            case tkScanError:
            case tkEOF:
                // Weirdness. Quit.
                if (PHASE_TRACE1(Js::ParallelParsePhase))
                {
                    Output::Print(L"Failed fast seek: %d. %s -- %d...%d\n",
                                  m_currentNodeFunc->sxFnc.functionId,
                                  GetFunctionName(m_currentNodeFunc, m_currentNodeFunc->sxFnc.hint),
                                  ichStart, m_pscan->IchLimTok());
                }
                m_nextBlockId = blockIdSave;
                *m_nextFunctionId = functionIdSave;
                m_pscan->SeekTo(funcStart);
                return false;
        }

        m_pscan->ScanNoKeywords();
    }
}

ParseNodePtr Parser::CreateDummyFuncNode(bool fDeclaration)
{
    // Create a dummy node and make it look like the current function declaration.
    // Do this in situations where we want to parse statements without impacting
    // the state of the "real" AST.

    ParseNodePtr pnodeFnc = CreateNode(knopFncDecl);
    pnodeFnc->sxFnc.ClearFlags();
    pnodeFnc->sxFnc.SetDeclaration(fDeclaration);
    pnodeFnc->sxFnc.astSize             = 0;
    pnodeFnc->sxFnc.pnodeNames          = nullptr;
    pnodeFnc->sxFnc.pnodeScopes         = nullptr;
    pnodeFnc->sxFnc.pnodeRest           = nullptr;
    pnodeFnc->sxFnc.pid                 = nullptr;
    pnodeFnc->sxFnc.hint                = nullptr;
    pnodeFnc->sxFnc.hintLength          = 0;
    pnodeFnc->sxFnc.isNameIdentifierRef = true;
    pnodeFnc->sxFnc.pnodeNext           = nullptr;
    pnodeFnc->sxFnc.pnodeArgs           = nullptr;
    pnodeFnc->sxFnc.pnodeVars           = nullptr;
    pnodeFnc->sxFnc.funcInfo            = nullptr;
    pnodeFnc->sxFnc.deferredStub        = nullptr;
    pnodeFnc->sxFnc.nestedCount         = 0;
    pnodeFnc->sxFnc.SetNested(m_currentNodeFunc != nullptr); // If there is a current function, then we're a nested function.
    pnodeFnc->sxFnc.SetStrictMode(IsStrictMode()); // Inherit current strict mode -- may be overridden by the function itself if it contains a strict mode directive.
    pnodeFnc->sxFnc.firstDefaultArg = 0;

    m_pCurrentAstSize = &pnodeFnc->sxFnc.astSize;
    m_currentNodeFunc = pnodeFnc;
    m_pnestedCount = &pnodeFnc->sxFnc.nestedCount;

    return pnodeFnc;
}

void Parser::ParseNestedDeferredFunc(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, bool fLambda, bool *pNeedScanRCurly, bool *pStrictModeTurnedOn)
{
    // Parse a function nested inside another deferred function.

    size_t lengthBeforeBody = this->GetSourceLength();

    if (m_token.tk != tkLCurly && fLambda)
    {
        ParseExpressionLambdaBody<false>(ERROR_RECOVERY_ACTUAL_(ers) pnodeFnc);
        *pNeedScanRCurly = false;
    }
    else
    {
        ChkCurTok(tkLCurly, ERRnoLcurly _ERROR_RECOVERY_ACTUAL(ers));

        bool* detectStrictModeOn = IsStrictMode() ? nullptr : pStrictModeTurnedOn;
        if (BindDeferredPidRefs())
        {
            m_ppnodeVar = &m_currentNodeDeferredFunc->sxFnc.pnodeVars;
        }

        ParseStmtList<false>(ERROR_RECOVERY_ACTUAL_(0) nullptr, nullptr, SM_DeferedParse, true /* isSourceElementList */, detectStrictModeOn);

        ChkCurTokNoScan(tkRCurly, ERRnoRcurly _ERROR_RECOVERY_ACTUAL(ers));
    }

    if (BindDeferredPidRefs())
    {
        pnodeFnc->ichLim = m_pscan->IchLimTok();
        pnodeFnc->sxFnc.cbLim = m_pscan->IecpLimTok();
        if (*pStrictModeTurnedOn)
        {
            pnodeFnc->sxFnc.SetStrictMode(true);
        }

        if (!PHASE_OFF1(Js::SkipNestedDeferredPhase))
        {
            // Record the end of the function and the function ID increment that happens inside the function.
            // Byte code gen will use this to build stub information to allow us to skip this function when the
            // enclosing function is fully parsed.
            RestorePoint *restorePoint = Anew(&m_nodeAllocator, RestorePoint);
            m_pscan->Capture(restorePoint,
                             *m_nextFunctionId - pnodeFnc->sxFnc.functionId - 1,
                             lengthBeforeBody - this->GetSourceLength());
            pnodeFnc->sxFnc.pRestorePoint = restorePoint;
        }
    }
}

template<bool buildAST>
bool Parser::ParseFncNames(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, ParseNodePtr pnodeFncParent, ushort flags, bool isSourceElement, ParseNodePtr **pLastNodeRef)
{
    BOOL fDeclaration = flags & fFncDeclaration;
    ParseNodePtr pnodeT;
    charcount_t ichMinNames, ichLimNames;

    // Get the names to bind to.
    /*
    * KaushiS [5/15/08]:
    * ECMAScript defines a FunctionExpression as follows:
    *
    * "function" [Identifier] ( [FormalParameterList] ) { FunctionBody }
    *
    * The function name being optional is omitted by most real world
    * code that uses a FunctionExpression to define a function. This however
    * is problematic for tools because there isn't a function name that
    * the runtime can provide.
    *
    * To fix this (primarily for the profiler), I'm adding simple, static
    * name inferencing logic to the parser. When it encounters the following
    * productions
    *
    *   "var" Identifier "=" FunctionExpression
    *   "var" IdentifierA.IdentifierB...Identifier "=" FunctionExpression
    *   Identifier = FunctionExpression
    *   "{" Identifier: FunctionExpression "}"
    *
    * it associates Identifier with the function created by the
    * FunctionExpression. This identifier is *not* the function's name. It
    * is ignored by the runtime and is only an additional piece of information
    * about the function (function name hint) that tools could opt to
    * surface.
    */

#if ERROR_RECOVERY
    if (m_token.tk != tkLParen) // This would only be false if error correction occurred because, otherwise it would be function or an identifier.
#endif
    {
        m_pscan->Scan();
    }

    // If generators are enabled then we are in a recent enough version
    // that deferred parsing will create a parse node for pnodeFnc and
    // it is safe to assume it is not null.
    if (flags & fFncGenerator)
    {
        Assert(m_scriptContext->GetConfig()->IsES6GeneratorsEnabled());
        pnodeFnc->sxFnc.SetIsGenerator();
    }
    else if (m_scriptContext->GetConfig()->IsES6GeneratorsEnabled() &&
        m_token.tk == tkStar &&
        !(flags & fFncClassMember))
    {
        if (!fDeclaration)
        {
            bool fPreviousYieldIsKeyword = m_pscan->SetYieldIsKeyword(!fDeclaration);
            m_pscan->Scan();
            m_pscan->SetYieldIsKeyword(fPreviousYieldIsKeyword);
        }
        else
        {
            m_pscan->Scan();
        }

        pnodeFnc->sxFnc.SetIsGenerator();
    }

    if (pnodeFnc)
    {
        pnodeFnc->sxFnc.pnodeNames = nullptr;
    }

    if (m_token.tk != tkID || flags & fFncNoName)
    {
        if (fDeclaration  ||
            ( m_token.IsReservedWord()) )  // For example:  var x = (function break(){});
        {
            IdentifierExpectedError(m_token);
#if ERROR_RECOVERY
            if (buildAST)
            {
                // No skip or recovery necessary since we can just proceed with non-ES5 syntax.
                pnodeFnc->sxFnc.pid = m_pidError;
                pnodeFnc->sxFnc.pnodeNames = CreateDeclNode(knopVarDecl, m_pidError, STFunction);
            }
#endif
        }
        return false;
    }

    ichMinNames = m_pscan->IchMinTok();


    Assert(m_token.tk == tkID);
    
    if (IsStrictMode())
    {        
        CheckStrictModeEvalArgumentsUsage(m_token.GetIdentifier(m_phtbl));
    }    
    Token tokenBase = m_token;
    charcount_t ichMinBase = m_pscan->IchMinTok();
    charcount_t ichLimBase = m_pscan->IchLimTok();
    
    m_pscan->Scan();

    if ((tkScope == m_token.tk || tkDot == m_token.tk) && fDeclaration)
    {
        IdentPtr pidBase = tokenBase.GetIdentifier(m_phtbl);
        CheckArgumentsUse(pidBase, pnodeFncParent);

        if (buildAST)
        {            
            pnodeT = CreateNameNode(pidBase, ichMinBase, ichLimBase);
            BlockInfoStack *blockInfo = GetCurrentFunctionBlockInfo();
            PidRefStack *ref = this->FindOrAddPidRef(pidBase, blockInfo->pnodeBlock->sxBlock.blockId);
            pnodeT->sxPid.SetSymRef(ref);
        }

        // Allow for id1.id2...idn::fn
        // Event handler. Accept this in IE9 mode for declarations only.
        while (tkDot == m_token.tk)
        {
            m_pscan->Scan();
#if ERROR_RECOVERY
            tokens tkErrorSave = tkID;
#endif
            if (m_token.tk != tkID)
            {
                IdentifierExpectedError(m_token);
#if ERROR_RECOVERY
                SKIP(ERROR_RECOVERY_ACTUAL((ersDot | ersComma | ersRParen | ersScope) | ers));
                if (m_token.tk != tkID)
                {
                    m_token.SetIdentifier(m_pidError);   
                    tkErrorSave = m_token.tk;
                    m_token.tk = tkID;
                }
#endif
            }
            if (buildAST)
            {
                pnodeT = CreateBinNode(knopDot, pnodeT, CreateNameNode(m_token.GetIdentifier(m_phtbl)));
            }
#if ERROR_RECOVERY
            if (tkErrorSave != tkID)
            {
                m_token.tk = tkErrorSave;
            }
            else
#endif
            {
                m_pscan->Scan();
            }
        }
        if (m_token.tk != tkScope)
        {
            // No separate error code available for missing ::, throw a syntax error
            Error(ERRsyntax);
        }
        m_pscan->Scan();
#if ERROR_RECOVERY
        tokens tkErrorSave = tkID;
#endif
        if (m_token.tk != tkID)
        {
            IdentifierExpectedError(m_token);
#if ERROR_RECOVERY
            SKIP(ERROR_RECOVERY_ACTUAL((ersDot | ersComma | ersRParen | ersScope) | ers));
            if (m_token.tk != tkID)
            {
                m_token.SetIdentifier(m_pidError);   
                tkErrorSave = m_token.tk;
                m_token.tk = tkID;
            }
#endif
        }

        if (!isSourceElement)
        {
            // For ES6, disallow scope operator in nested block/statement to avoid confusion
            // with block-scoping rules.
            Error(ERRsyntax);
        }
        if (buildAST)
        {
            pnodeT = CreateBinNode(knopScope, pnodeT, CreateNameNode(m_token.GetIdentifier(m_phtbl)));
        }
#if ERROR_RECOVERY
        if (tkErrorSave != tkID)
        {
            m_token.tk = tkErrorSave;
        }
        else
#endif
        {
            m_pscan->Scan();
        }
    }
    else if (buildAST || BindDeferredPidRefs())
    {
        IdentPtr pidBase = tokenBase.GetIdentifier(m_phtbl);
        pnodeT = CreateDeclNode(knopVarDecl, pidBase, STFunction);
        pnodeT->ichMin = ichMinBase;
        pnodeT->ichLim = ichLimBase;

        if (fDeclaration &&
            pnodeFncParent &&
            pnodeFncParent->sxFnc.pnodeNames &&
            pnodeFncParent->sxFnc.pnodeNames->nop == knopVarDecl &&
            pnodeFncParent->sxFnc.pnodeNames->sxVar.pid == pidBase)
        {
            pnodeFncParent->sxFnc.SetNameIsHidden();
        }
    }

    if (buildAST)
    {
        ichLimNames = pnodeT->ichLim;
        AddToNodeList(&pnodeFnc->sxFnc.pnodeNames, pLastNodeRef, pnodeT);

        pnodeFnc->sxFnc.pnodeNames->ichMin = ichMinNames;
        pnodeFnc->sxFnc.pnodeNames->ichLim = ichLimNames;
        if (knopVarDecl == pnodeFnc->sxFnc.pnodeNames->nop)
        {
            // Only one name (the common case).
            pnodeFnc->sxFnc.pid = pnodeFnc->sxFnc.pnodeNames->sxVar.pid;
        }
        else
        {
            // Multiple names. Turn the source into an IdentPtr.
            pnodeFnc->sxFnc.pid = m_phtbl->PidHashNameLen(
                m_pscan->PchBase() + ichMinNames, ichLimNames - ichMinNames);
        }

#if PARSENODE_EXTENSIONS
        if (LanguageServiceMode())
        {
            // store the min and lim of the function name
            m_languageServiceExtension->SetIdentMin(pnodeFnc, ichMinNames);
            m_languageServiceExtension->SetIdentLim(pnodeFnc, ichLimNames);
        }
#endif

        if(pnodeFnc->sxFnc.pid == wellKnownPropertyPids.arguments && fDeclaration && pnodeFncParent)
        {
            // This function declaration (or function expression in compat modes) overrides the built-in arguments object of the
            // parent function
            pnodeFncParent->grfpn |= PNodeFlags::fpnArguments_overriddenByDecl;
        }
    }

    return true;
}

void Parser::ValidateFormals()
{
    ParseFncFormals<false>(ERROR_RECOVERY_ACTUAL_(0) NULL, fFncNoFlgs);
    // Eat the tkRParen. The ParseFncDeclHelper caller expects to see it.
    m_pscan->Scan();
}

void Parser::ValidateSourceElementList()
{
    ParseStmtList<false>(ERROR_RECOVERY_ACTUAL_(0) NULL, NULL, SM_NotUsed, true);
}

template<bool buildAST>
void Parser::ParseFncFormals(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, ushort flags)
{
    // In strict mode we need to detect duplicated formals so force PID creation (unless the function should take 0 or 1 arg).
    BOOL forcePid = IsStrictMode() && ((flags & (fFncNoArg | fFncOneArg)) == 0);
    AutoTempForcePid autoForcePid(m_pscan, forcePid);

    // Lambda's allow single formal specified by a single binding identifier without parentheses, special case it.
    if (m_token.tk == tkID && (flags & fFncLambda))
    {
        if (buildAST || BindDeferredPidRefs())
        {
            IdentPtr pid = m_token.GetIdentifier(m_phtbl);
#if PARSENODE_EXTENSIONS
            ParseNodePtr pnodeT = 
#endif
                CreateVarDeclNode(pid, STFormal, false, nullptr, false);

#if PARSENODE_EXTENSIONS
            if (buildAST && LanguageServiceMode())
            {
                // store the min and lim of the parameter name
                m_languageServiceExtension->SetIdentMin(pnodeT, m_pscan->IchMinTok());
                m_languageServiceExtension->SetIdentLim(pnodeT, m_pscan->IchLimTok());
            }
#endif

            CheckPidIsValid(pid);
            
            m_pscan->Scan();

            if (m_token.tk != tkDArrow)
            {
                Error(ERRsyntax, m_pscan->IchMinTok(), m_pscan->IchLimTok());

                // In this case, we just skip any token until we reach the DArrow so that this is still a valid single formal
                SKIP(ERROR_RECOVERY_ACTUAL(ersDArrow));
            }

            return;
        }
    }

#if PARSENODE_EXTENSIONS
    charcount_t ichLParenMin = 0;
#endif

    // Otherwise, must have a parameter list within parens.
    ChkCurTok(tkLParen, ERRnoLparen _ERROR_RECOVERY_ACTUAL(ersRParen | ersLCurly | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLParenMin));

#if PARSENODE_EXTENSIONS
    // Keep ichLParenMin if needed
    if (buildAST && LanguageServiceMode())
        m_languageServiceExtension->SetLParen(pnodeFnc, ichLParenMin);
#endif

    // Now parse the list of arguments, if present
    Assert((flags & (fFncNoArg | fFncOneArg)) != (fFncNoArg | fFncOneArg)); // fFncNoArg and fFncOneArg can never be at same time.
    if (m_token.tk == tkRParen)
    {
        if (flags & fFncOneArg)
        {
            Error(ERRSetterMustHaveOneArgument);
            SKIP(ERROR_RECOVERY_ACTUAL((ersLCurly | ersRParen) | ers));
        }
    }
    else
    {
        if (flags & fFncNoArg)
        {
            Error(ERRnoRparen); //enforce no arguments
            // No recovery necessary since this is a semantic, not structural, error
        }
        SList<IdentPtr> formals(&m_nodeAllocator);
        ParseNodePtr pnodeT = nullptr;
        bool seenRestParameter = false;
        bool isNonSimpleParameterList = false;
        for (uint argPos = 0; ; ++argPos)
        {
            if (m_scriptContext->GetConfig()->IsES6RestEnabled() && m_token.tk == tkEllipsis)
            {
                // Possible rest parameter
                m_pscan->Scan();
                seenRestParameter = true;
            }
#if ERROR_RECOVERY
            tokens tkErrorSave = tkID;
#endif
            if (m_token.tk != tkID)
            {
                IdentifierExpectedError(m_token);
#if ERROR_RECOVERY
                SKIP(ERROR_RECOVERY_ACTUAL((ersComma | ersRParen) | ers));                
                if (m_token.tk != tkID)
                {
                    m_token.SetIdentifier(m_pidError);   
                    tkErrorSave = m_token.tk;
                    m_token.tk = tkID;
                }              
#endif
            }

            if (seenRestParameter)
            {
                if (flags & fFncSetter)
                {
                    // The parameter of a setter cannot be a rest parameter.
                    Error(ERRUnexpectedEllipsis);
                }
                if (buildAST || BindDeferredPidRefs())
                {
                    pnodeT = CreateDeclNode(knopVarDecl, m_token.GetIdentifier(m_phtbl), STFormal, false);
                    pnodeT->sxVar.sym->SetIsNonSimpleParameter(true);
                    if (buildAST)
                    {
                        // When only validating formals, we won't have a function node.
                        pnodeFnc->sxFnc.pnodeRest = pnodeT;
                    }
                    if (!isNonSimpleParameterList)
                    {
                        // This is the first non-simple parameter we've seen. We need to go back
                        // and set the syms of all previous parameters.
                        MapFormalsWithoutRest(m_currentNodeFunc, [&](ParseNodePtr pnodeArg) { pnodeArg->sxVar.sym->SetIsNonSimpleParameter(true); });
                    }
                }
                else
                {
                    isNonSimpleParameterList = true;
                }
            }
            else
            {
                if (buildAST || BindDeferredPidRefs())
                {
                    pnodeT = CreateVarDeclNode(m_token.GetIdentifier(m_phtbl), STFormal, false, nullptr, false);
                    if (isNonSimpleParameterList)
                    {
                        pnodeT->sxVar.sym->SetIsNonSimpleParameter(true);
                    }
                }
            }

#if PARSENODE_EXTENSIONS
            if (buildAST && LanguageServiceMode())
            {
                // store the min and lim of the parameter name
                m_languageServiceExtension->SetIdentMin(pnodeT, m_pscan->IchMinTok());
                m_languageServiceExtension->SetIdentLim(pnodeT, m_pscan->IchLimTok());
            }
#endif

            if (buildAST && m_token.GetIdentifier(m_phtbl) == wellKnownPropertyPids.arguments)
            {
                // This formal parameter overrides the built-in 'arguments' object
                m_currentNodeFunc->grfpn |= PNodeFlags::fpnArguments_overriddenByDecl;
            }

            if (IsStrictMode())
            {
                IdentPtr pid = m_token.GetIdentifier(m_phtbl);
                CheckStrictModeEvalArgumentsUsage(pid);
                if (formals.Has(pid))
                {
                    Error(ERRES5ArgSame);
                }
                else
                {
                    formals.Prepend(pid);
                }
            }

#if ERROR_RECOVERY
            if (tkErrorSave != tkID)
            {
                m_token.tk = tkErrorSave;
            }
            else
#endif
            {
                m_pscan->Scan();
            }

            if (seenRestParameter && m_token.tk != tkRParen && m_token.tk != tkAsg)
            {
                Error(ERRRestLastArg);
            }

            if (flags & fFncOneArg)
            {
                if (m_token.tk != tkRParen)
                {
                    Error(ERRSetterMustHaveOneArgument);
                    SKIP(ERROR_RECOVERY_ACTUAL((ersRParen | ersStmtStart) | ers));
                }
                break; //enforce only one arg
            }

            if (m_token.tk == tkAsg && m_scriptContext->GetConfig()->IsES6DefaultArgsEnabled())
            {
                if (seenRestParameter && m_scriptContext->GetConfig()->IsES6RestEnabled())
                {
                    Error(ERRRestWithDefault);
                }
                m_pscan->Scan();
                ParseNodePtr pnodeInit = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) koplCma);

                if (buildAST || BindDeferredPidRefs())
                {
                    pnodeT->sxVar.sym->SetIsNonSimpleParameter(true);
                    if (!isNonSimpleParameterList)
                    {
                        // This is the first non-simple parameter we've seen. We need to go back
                        // and set the syms of all previous parameters.
                        MapFormalsWithoutRest(m_currentNodeFunc, [&](ParseNodePtr pnodeArg) { pnodeArg->sxVar.sym->SetIsNonSimpleParameter(true); });

                        // There may be previous parameters that need to be checked for duplicates.
                        isNonSimpleParameterList = true;
                    }
                }


                if (buildAST)
                {
                    if (!m_currentNodeFunc->sxFnc.HasDefaultArguments())
                    {
                        m_currentNodeFunc->sxFnc.SetHasDefaultArguments();
                        m_currentNodeFunc->sxFnc.firstDefaultArg = argPos;
                        CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(DefaultArgFunctionCount, m_scriptContext);
                    }
                    pnodeT->sxVar.pnodeInit = pnodeInit;
                    pnodeT->ichLim = m_pscan->IchLimTok();
                }
            }

            if (m_token.tk != tkComma)
            {
                break;
            }

            m_pscan->Scan();
        }

        if (seenRestParameter)
        {
            CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(RestCount, m_scriptContext);
        }

        if (m_token.tk != tkRParen)
        {
            Error(ERRnoRparen);
            SKIP(ERROR_RECOVERY_ACTUAL((ersRParen | ersStmtStart) | ers));
        }
    }
    Assert(m_token.tk == tkRParen || PerformingErrorRecovery());
}

template<bool buildAST>
ParseNodePtr Parser::GenerateEmptyConstructor(bool extends)
{
    ParseNodePtr pnodeFnc;

    if (buildAST || BindDeferredPidRefs())
    {
        // Create the node.
        pnodeFnc = CreateNode(knopFncDecl);
        pnodeFnc->sxFnc.ClearFlags();
        pnodeFnc->sxFnc.SetNested(NULL != m_currentNodeFunc);
        pnodeFnc->sxFnc.SetStrictMode();
        pnodeFnc->sxFnc.SetDeclaration(TRUE);
        pnodeFnc->sxFnc.SetIsMethod(TRUE);
        pnodeFnc->sxFnc.SetIsClassMember(TRUE);
        pnodeFnc->sxFnc.SetIsClassConstructor(TRUE);
        pnodeFnc->sxFnc.SetIsGeneratedDefault(TRUE);

        pnodeFnc->ichLim = m_pscan->IchLimTok();
        pnodeFnc->ichMin = m_pscan->IchMinTok();
        pnodeFnc->sxFnc.cbLim = m_pscan->IecpLimTok();
        pnodeFnc->sxFnc.cbMin = m_pscan->IecpMinTok();
        pnodeFnc->sxFnc.astSize = 0;
        pnodeFnc->sxFnc.lineNumber = m_pscan->LineCur();

        pnodeFnc->sxFnc.functionId          = (*m_nextFunctionId);
        pnodeFnc->sxFnc.pid                 = nullptr;
        pnodeFnc->sxFnc.hint                = nullptr;
        pnodeFnc->sxFnc.hintLength          = 0;
        pnodeFnc->sxFnc.isNameIdentifierRef = true;
        pnodeFnc->sxFnc.pnodeNames          = nullptr;
        pnodeFnc->sxFnc.pnodeScopes         = nullptr;
        pnodeFnc->sxFnc.pnodeArgs           = nullptr;
        pnodeFnc->sxFnc.pnodeVars           = nullptr;
        pnodeFnc->sxFnc.pnodeBody           = nullptr;
        pnodeFnc->sxFnc.nestedCount         = 0;
        pnodeFnc->sxFnc.pnodeNext           = nullptr;
        pnodeFnc->sxFnc.pnodeRest           = nullptr;
        pnodeFnc->sxFnc.deferredStub        = nullptr;
        pnodeFnc->sxFnc.funcInfo            = nullptr;

#ifdef DBG
        pnodeFnc->sxFnc.deferredParseNextFunctionId = *(this->m_nextFunctionId);
#endif

        AppendFunctionToScopeList(true, pnodeFnc);
    }

    if (m_nextFunctionId)
    {
        (*m_nextFunctionId)++;
    }

    // Update the count of functions nested in the current parent.
    if (m_pnestedCount)
    {
        (*m_pnestedCount)++;
    }

    if (!buildAST)
    {
        return NULL;
    }

    if (m_pscan->IchMinTok() >= m_pscan->IchMinLine())
    {
        // In scenarios involving defer parse IchMinLine() can be incorrect for the first line after defer parse
        pnodeFnc->sxFnc.columnNumber = m_pscan->IchMinTok() - m_pscan->IchMinLine();
    }
    else if (m_currentNodeFunc)
    {
        // For the first line after defer parse, compute the column relative to the column number
        // of the lexically parent function.
        ULONG offsetFromCurrentFunction = m_pscan->IchMinTok() - m_currentNodeFunc->ichMin;
        pnodeFnc->sxFnc.columnNumber = m_currentNodeFunc->sxFnc.columnNumber + offsetFromCurrentFunction;
    }
    else
    {
        // if there is no current function, lets give a default of 0.
        pnodeFnc->sxFnc.columnNumber = 0;
    }

    long * pAstSizeSave = m_pCurrentAstSize;
    m_pCurrentAstSize = &(pnodeFnc->sxFnc.astSize);

    // Make this the current function.
    ParseNodePtr pnodeFncSave = m_currentNodeFunc;
    m_currentNodeFunc = pnodeFnc;

    ParseNodePtr pnodeBlock = StartParseBlock<buildAST>(PnodeBlockType::Parameter, ScopeType_Parameter);
    ParseNodePtr pnodeInnerBlock = StartParseBlock<buildAST>(PnodeBlockType::Function, ScopeType_FunctionBody);
    pnodeBlock->sxBlock.pnodeScopes = pnodeInnerBlock;
    pnodeFnc->sxFnc.pnodeBodyScope = pnodeInnerBlock;
    pnodeFnc->sxFnc.pnodeScopes = pnodeBlock;

    ParseNodePtr *lastNodeRef = nullptr;
    if (extends)
    {
        // constructor() { super(...arguments); } (equivalent to constructor(...args) { super(...args); } )
        PidRefStack *ref = this->PushPidRef(wellKnownPropertyPids.arguments);
        ParseNodePtr argumentsId = CreateNameNode(wellKnownPropertyPids.arguments, pnodeFnc->ichMin, pnodeFnc->ichLim);
        argumentsId->sxPid.symRef = ref->GetSymRef();
        pnodeFnc->sxFnc.SetUsesArguments(true);
        pnodeFnc->sxFnc.SetHasReferenceableBuiltInArguments(true);

        ParseNodePtr *const ppnodeVarSave = m_ppnodeVar;
        m_ppnodeVar = &pnodeFnc->sxFnc.pnodeVars;
        CreateVarDeclNode(wellKnownPropertyPids.arguments, STVariable, true, pnodeFnc)->grfpn |= PNodeFlags::fpnArguments;
        m_ppnodeVar = ppnodeVarSave;

        ParseNodePtr spreadArg = CreateUniNode(knopEllipsis, argumentsId, pnodeFnc->ichMin, pnodeFnc->ichLim);

        ParseNodePtr superRef = CreateNodeWithScanner<knopSuper>();
        pnodeFnc->sxFnc.SetHasSuperReference(TRUE);

        ParseNodePtr callNode = CreateCallNode(knopCall, superRef, spreadArg);
        callNode->sxCall.spreadArgCount = 1;
        AddToNodeList(&pnodeFnc->sxFnc.pnodeBody, &lastNodeRef, callNode);
    }

    AddToNodeList(&pnodeFnc->sxFnc.pnodeBody, &lastNodeRef, CreateNodeWithScanner<knopEndCode>());

    FinishParseBlock(pnodeInnerBlock);
    FinishParseBlock(pnodeBlock);

    m_currentNodeFunc = pnodeFncSave;
    m_pCurrentAstSize = pAstSizeSave;

    return pnodeFnc;
}

template<bool buildAST>
void Parser::ParseExpressionLambdaBody(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeLambda)
{
    ParseNodePtr *lastNodeRef = nullptr;

    // The lambda body is a single expression, the result of which is the return value.
    ParseNodePtr pnodeRet = nullptr;

    if (buildAST)
    {
        pnodeRet = CreateNodeWithScanner<knopReturn>();
        pnodeRet->grfpn |= PNodeFlags::fpnSyntheticNode;
        pnodeLambda->sxFnc.pnodeScopes->sxBlock.pnodeStmt = pnodeRet;
    }

    ParseNodePtr result = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) koplAsg, nullptr, TRUE, FALSE, nullptr);

    if (buildAST)
    {
        pnodeRet->sxReturn.pnodeExpr = result;

        pnodeRet->ichMin = pnodeRet->sxReturn.pnodeExpr->ichMin;
        pnodeRet->ichLim = pnodeRet->sxReturn.pnodeExpr->ichLim;

        // Pushing a statement node with PushStmt<>() normally does this initialization
        // but do it here manually since we know there is no outer statement node.
        pnodeRet->sxStmt.grfnop = 0;
        pnodeRet->sxStmt.pnodeOuter = nullptr;

        pnodeLambda->ichLim = pnodeRet->ichLim;
        pnodeLambda->sxFnc.cbLim = m_pscan->IecpLimTokPrevious();
        pnodeLambda->sxFnc.pnodeScopes->ichLim = pnodeRet->ichLim;

        pnodeLambda->sxFnc.pnodeBody = nullptr;
        AddToNodeList(&pnodeLambda->sxFnc.pnodeBody, &lastNodeRef, pnodeLambda->sxFnc.pnodeScopes);

        // Append an EndCode node.
        ParseNodePtr end = CreateNodeWithScanner<knopEndCode>(pnodeRet->ichLim);
        end->ichLim = end->ichMin; // make end code zero width at the immediate end of lambda body
        AddToNodeList(&pnodeLambda->sxFnc.pnodeBody, &lastNodeRef, end);

        // Lambda's do not have arguments binding
        pnodeLambda->sxFnc.SetHasReferenceableBuiltInArguments(false);
    }

#if PARSENODE_EXTENSIONS
    if (buildAST && LanguageServiceMode())
    {
        m_languageServiceExtension->SetNestedCount(pnodeLambda, pnodeLambda->sxFnc.nestedCount);
    }
#endif
}

void Parser::CheckStrictFormalParameters(ERROR_RECOVERY_FORMAL)
{
    Assert(m_token.tk == tkLParen);
    m_pscan->ScanForcingPid();

    if (m_token.tk != tkRParen)
    {
        SList<IdentPtr> formals(&m_nodeAllocator);
        for (;;)
        {
            if (m_token.tk != tkID)
            {
                IdentifierExpectedError(m_token);
            }

            IdentPtr pid = m_token.GetIdentifier(m_phtbl);
            CheckStrictModeEvalArgumentsUsage(pid);
            if (formals.Has(pid))
            {
                Error(ERRES5ArgSame, m_pscan->IchMinTok(), m_pscan->IchLimTok());
            }
            else
            {
                formals.Prepend(pid);
            }

            m_pscan->Scan();

            if (m_token.tk == tkAsg && m_scriptContext->GetConfig()->IsES6DefaultArgsEnabled())
            {
                m_pscan->Scan();
                // We can avoid building the AST since we are just checking the default expression.
                ParseNodePtr pnodeInit = ParseExpr<false>(ERROR_RECOVERY_ACTUAL_(ers) koplCma);
                Assert(pnodeInit == nullptr);
            }

            if (m_token.tk != tkComma)
            {
                break;
            }
            m_pscan->ScanForcingPid();
        }
    }
    Assert(m_token.tk == tkRParen);
}

void Parser::FinishFncNode(ParseNodePtr pnodeFnc)
{
    // Finish the AST for a function that was deferred earlier, but which we decided
    // to finish after the fact.
    // We assume that the name(s) and arg(s) have already got parse nodes, so
    // we just have to do the function body.

    // Save the current next function Id, and resume from the old one.
    Js::LocalFunctionId * nextFunctionIdSave = m_nextFunctionId;
    Js::LocalFunctionId tempNextFunctionId = pnodeFnc->sxFnc.functionId + 1;
    this->m_nextFunctionId = &tempNextFunctionId;

    ParseNodePtr pnodeFncSave = m_currentNodeFunc;
    uint *pnestedCountSave = m_pnestedCount;
    long* pAstSizeSave = m_pCurrentAstSize;

    m_currentNodeFunc = pnodeFnc;
    m_pCurrentAstSize = & (pnodeFnc->sxFnc.astSize);

    pnodeFnc->sxFnc.nestedCount = 0;
    m_pnestedCount = &pnodeFnc->sxFnc.nestedCount;

    // Cue up the parser to the start of the function body.
    if (pnodeFnc->sxFnc.pnodeNames)
    {
        // Skip the name(s).
        m_pscan->SetCurrentCharacter(pnodeFnc->sxFnc.pnodeNames->ichLim, pnodeFnc->sxFnc.lineNumber);
    }
    else
    {
        m_pscan->SetCurrentCharacter(pnodeFnc->ichMin, pnodeFnc->sxFnc.lineNumber);
        if (pnodeFnc->sxFnc.IsAccessor())
        {
            // Getter/setter. The node text starts with the name, so eat that.
            m_pscan->ScanNoKeywords();
        }
        else
        {
            // Anonymous function. Skip any leading "("'s and "function".
            for (;;)
            {
                m_pscan->Scan();
                if (m_token.tk == tkFUNCTION)
                {
                    break;
                }
                Assert(m_token.tk == tkLParen || m_token.tk == tkStar);
            }
        }
    }

    // switch scanner to treat 'yield' as keyword in generator functions
    // or as an identifier in non-generator functions
    bool fPreviousYieldIsKeyword = m_pscan->SetYieldIsKeyword(pnodeFnc && pnodeFnc->sxFnc.IsGenerator());

    // Skip the arg list.
    m_pscan->ScanNoKeywords();
    if (m_token.tk == tkStar)
    {
        Assert(pnodeFnc->sxFnc.IsGenerator());
        m_pscan->ScanNoKeywords();
    }
    Assert(m_token.tk == tkLParen);
    m_pscan->ScanNoKeywords();

    if (m_token.tk != tkRParen)
    {
        for (;;)
        {
            switch (m_token.tk)
            {
            case tkID:
                break;
            case tkEllipsis:
                m_pscan->ScanNoKeywords();
                break;

            default:
                AssertMsg(false, "Unexpected identifier prefix while fast-scanning formals");
            }
            m_pscan->ScanNoKeywords();

            if (m_token.tk == tkAsg)
            {
                // Eat the default expression
                m_pscan->ScanNoKeywords();
                ParseExpr<false>(ERROR_RECOVERY_ACTUAL_(ersNone) koplCma);
            }

            if (m_token.tk != tkComma)
            {
                break;
            }
            m_pscan->ScanNoKeywords();
        }
    }

#if PARSENODE_EXTENSIONS
    // Keep ichRParenMin if needed
    if (LanguageServiceMode() && m_token.tk == tkRParen)
        m_languageServiceExtension->SetRParen(pnodeFnc, m_pscan->IchMinTok());
#endif

    if (m_token.tk == tkRParen) // This might be false due to a lambda => token.
    {
        m_pscan->Scan();
    }

    // Finish the function body.
    {
        // Note that in IE8- modes, surrounding parentheses are considered part of function body. e.g. "( function x(){} )".
        // We lose that context here since we start from middle of function body. So save and restore source range info.
        ParseNodePtr* lastNodeRef = NULL;
        const charcount_t ichLim = pnodeFnc->ichLim;
        const size_t cbLim = pnodeFnc->sxFnc.cbLim;
        this->FinishFncDecl(ERROR_RECOVERY_ACTUAL_(ersEOF) pnodeFnc, NULL, lastNodeRef);

#if DBG
        // The pnode extent may not match the original extent.
        // We expect this to happen only when there are trailing ")"'s.
        // Consume them and make sure that's all we've got.
        if (pnodeFnc->ichLim != ichLim)
        {
            Assert(pnodeFnc->ichLim < ichLim);
            m_pscan->SetCurrentCharacter(pnodeFnc->ichLim);
            while (m_pscan->IchLimTok() != ichLim)
            {
                m_pscan->ScanNoKeywords();
                Assert(m_token.tk == tkRParen);
            }
        }
#endif
        pnodeFnc->ichLim = ichLim;
        pnodeFnc->sxFnc.cbLim = cbLim;
    }

    m_currentNodeFunc = pnodeFncSave;
    m_pCurrentAstSize = pAstSizeSave;
    m_pnestedCount = pnestedCountSave;
    Assert(m_pnestedCount);

    Assert(tempNextFunctionId == pnodeFnc->sxFnc.deferredParseNextFunctionId);
    this->m_nextFunctionId = nextFunctionIdSave;

    m_pscan->SetYieldIsKeyword(fPreviousYieldIsKeyword);
}

void Parser::FinishFncDecl(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeFnc, LPCOLESTR pNameHint, ParseNodePtr *lastNodeRef)
{
    LPCOLESTR name = NULL;
    long startAstSize = *m_pCurrentAstSize;
    if(EventEnabledJSCRIPT_PARSE_METHOD_START() || PHASE_TRACE1(Js::DeferParsePhase))
    {
        name = GetFunctionName(pnodeFnc, pNameHint);
        m_functionBody = NULL;  // for nested functions we do not want to get the name of the top deferred function return name;
        JSETW(EventWriteJSCRIPT_PARSE_METHOD_START(m_sourceContextInfo->dwHostSourceContext, GetScriptContext(), pnodeFnc->sxFnc.functionId, 0, m_parseType, name));
        OUTPUT_TRACE(Js::DeferParsePhase, L"Parsing function (%s) : %s (%d)\n", GetParseType(), name, pnodeFnc->sxFnc.functionId);
    }

    JSETW(EventWriteJSCRIPT_PARSE_FUNC(GetScriptContext(), pnodeFnc->sxFnc.functionId, /*Undefer*/FALSE));


    // Do the work of creating an AST for a function body.
    // This is common to the undeferred case and the case in which we undefer late in the game.

#if PARSENODE_EXTENSIONS
    charcount_t ichLCurlyMin = 0;
    charcount_t ichRCurlyMin = 0;
#endif

    Assert(pnodeFnc->nop == knopFncDecl);

    ChkCurTok(tkLCurly, ERRnoLcurly _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLCurlyMin));

#if PARSENODE_EXTENSIONS
    // Keep ichLCurlyMin if needed
    if (LanguageServiceMode())
    {
        m_languageServiceExtension->SetLCurly(pnodeFnc, ichLCurlyMin);
    }
#endif

    ParseStmtList<true>(ERROR_RECOVERY_ACTUAL_(ersRCurly | ers) &pnodeFnc->sxFnc.pnodeBody, &lastNodeRef, SM_OnFunctionCode, true /* isSourceElementList */);

#if LANGUAGE_SERVICE
    // If we don't have anything in the body, at least don't overlap with EndCode
    pnodeFnc->sxFnc.pnodeBodyScope->ichLim = m_pscan->IchMinTok();
#endif

    pnodeFnc->ichLim = m_pscan->IchLimTok();
    pnodeFnc->sxFnc.cbLim = m_pscan->IecpLimTok();

    // Append an EndCode node.
    AddToNodeList(&pnodeFnc->sxFnc.pnodeBody, &lastNodeRef, CreateNodeWithScanner<knopEndCode>());
    ChkCurTokNoScan(tkRCurly, ERRnoRcurly _ERROR_RECOVERY_ACTUAL(ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRCurlyMin));

#if PARSENODE_EXTENSIONS
    // Keep ichRCurlyMin if needed
    if (LanguageServiceMode())
    {
        m_languageServiceExtension->SetRCurly(pnodeFnc, ichRCurlyMin);
    }
#endif

    // Restore the lists of scopes that contain function expressions.

#if PARSENODE_EXTENSIONS
    if (LanguageServiceMode())
    {
        m_languageServiceExtension->SetNestedCount(pnodeFnc, pnodeFnc->sxFnc.nestedCount);
    }
#endif


    // Save the temps and restore the outer scope's list.
    // NOTE: Eze makes no use of this.
    //pnodeFnc->sxFnc.pnodeTmps = *m_ppnodeVar;

    long astSize = *m_pCurrentAstSize - startAstSize;
    JSETW(EventWriteJSCRIPT_PARSE_METHOD_STOP(m_sourceContextInfo->dwHostSourceContext, GetScriptContext(), pnodeFnc->sxFnc.functionId, astSize, m_parseType, name));
}

void Parser::AddArgumentsNodeToVars(ParseNodePtr pnodeFnc)
{
    if((pnodeFnc->grfpn & PNodeFlags::fpnArguments_overriddenByDecl) || pnodeFnc->sxFnc.IsLambda())
    {
        // In any of the following cases, there is no way to reference the built-in 'arguments' variable (in the order of checks
        // above):
        //     - A function parameter is named 'arguments'
        //     - There is a nested function declaration (or named function expression in compat modes) named 'arguments'
        //     - In compat modes, the function is named arguments, does not have a var declaration named 'arguments', and does
        //       not call 'eval'
        pnodeFnc->sxFnc.SetHasReferenceableBuiltInArguments(false);
    }
    else
    {
        if(m_ppnodeVar == &pnodeFnc->sxFnc.pnodeVars)
        {
            // There were no var declarations in the function
            CreateVarDeclNode(wellKnownPropertyPids.arguments, STVariable, true, pnodeFnc)->grfpn |= PNodeFlags::fpnArguments;
        }
        else
        {
            // There were var declarations in the function, so insert an 'arguments' local at the beginning of the var list.
            // This is done because the built-in 'arguments' variable overrides an 'arguments' var declaration until the
            // 'arguments' variable is assigned. By putting our built-in var declaration at the beginning, an 'arguments'
            // identifier will resolve to this symbol, which has the fpnArguments flag set, and will be the built-in arguments
            // object until it is replaced with something else.
            ParseNodePtr *const ppnodeVarSave = m_ppnodeVar;
            m_ppnodeVar = &pnodeFnc->sxFnc.pnodeVars;
            CreateVarDeclNode(wellKnownPropertyPids.arguments, STVariable, true, pnodeFnc)->grfpn |= PNodeFlags::fpnArguments;
            m_ppnodeVar = ppnodeVarSave;
        }

        pnodeFnc->sxFnc.SetHasReferenceableBuiltInArguments(true);
    }
}

LPCOLESTR Parser::GetFunctionName(ParseNodePtr pnodeFnc, LPCOLESTR pNameHint)
{
    LPCOLESTR name = NULL;
    if(pnodeFnc->sxFnc.pnodeNames != NULL && knopVarDecl == pnodeFnc->sxFnc.pnodeNames->nop)
    {
        name = pnodeFnc->sxFnc.pnodeNames->sxVar.pid->Psz();
    }
    if(name == NULL && pNameHint != null)
    {
        name = pNameHint;
    }
    if(name == NULL && m_functionBody != NULL)
    {
        name = m_functionBody->GetExternalDisplayName();
    }
    else if(name == NULL)
    {
        name = Js::Constants::AnonymousFunction;
    }
    return name;
}

IdentPtr Parser::ParseClassPropertyName(ERROR_RECOVERY_FORMAL_ IdentPtr * pidHint)
{
    if (m_token.tk == tkID || m_token.tk == tkStrCon || m_token.IsReservedWord())
    {
        IdentPtr pid;
        if (m_token.tk == tkStrCon)
        {
            if (m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
            {
                Error(ERRES5NoOctal);
#if ERROR_RECOVERY
                goto LErrorRecovery;
#endif
            }

            pid = m_token.GetStr();
        }
        else
        {
            pid = m_token.GetIdentifier(m_phtbl);
        }
        *pidHint = pid;
        return pid;
    }
    else if (m_token.tk == tkIntCon)
    {
        if (m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
        {
            Error(ERRES5NoOctal);
#if ERROR_RECOVERY
            goto LErrorRecovery;
#endif
        }

        return m_pscan->PidFromLong(m_token.GetLong());
    }
    else if (m_token.tk == tkFltCon)
    {
        if (m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
        {
            Error(ERRES5NoOctal);
#if ERROR_RECOVERY
            goto LErrorRecovery;
#endif
        }

        return m_pscan->PidFromDbl(m_token.GetDouble());
    }

    Error(ERRnoMemberIdent);
#if ERROR_RECOVERY
LErrorRecovery:
    SKIP(ERROR_RECOVERY_ACTUAL(ers));
    return m_pidError;
#endif
}

LPCOLESTR Parser::ConstructFinalHintNode(IdentPtr pClassName, IdentPtr pMemberName, IdentPtr pGetSet, bool isStatic, ulong* nameLength, bool isComputedName, LPCOLESTR pMemberNameHint)
{
    if ((pMemberName == nullptr && !isComputedName) ||
        (pMemberNameHint == nullptr && isComputedName) ||
        !CONFIG_FLAG(UseFullName))
    {
        return nullptr;
    }

    LPCOLESTR pFinalName = isComputedName? pMemberNameHint : pMemberName->Psz();
    ulong fullNameHintLength = 0;
    if (!isStatic)
    {
        // Add prototype.
        pFinalName = AppendNameHints(wellKnownPropertyPids.prototype, pFinalName, &fullNameHintLength);
    }

    if (pClassName)
    {
        pFinalName = AppendNameHints(pClassName, pFinalName, &fullNameHintLength);
    }

    if (pGetSet)
    {
        if (m_scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            // displays as get/set prototype.funcname
            pFinalName = AppendNameHints(pGetSet, pFinalName, &fullNameHintLength, true);
        }
        else
        {
            pFinalName = AppendNameHints(pFinalName, pGetSet, &fullNameHintLength);
        }
    }
    if (fullNameHintLength > *nameLength)
    {
        *nameLength = fullNameHintLength;
    }

    return pFinalName;
}

template<bool buildAST>
ParseNodePtr Parser::ParseClassDecl(ERROR_RECOVERY_FORMAL_ BOOL isDeclaration, LPCOLESTR pNameHint, ulong *pHintLength)
{
    bool hasConstructor = false;
    IdentPtr name = nullptr;
    ParseNodePtr pnodeName = nullptr;
    ParseNodePtr pnodeConstructor = nullptr;
    ParseNodePtr pnodeExtends = nullptr;
    ParseNodePtr pnodeMembers = nullptr;
    ParseNodePtr *lastMemberNodeRef = nullptr;
    ParseNodePtr pnodeStaticMembers = nullptr;
    ParseNodePtr *lastStaticMemberNodeRef = nullptr;
    ulong nameHintLength = pHintLength ? *pHintLength : 0;
#if PARSENODE_EXTENSIONS
    charcount_t ichDeadRangeMin = m_pscan->IchLimTok();
#endif

    ArenaAllocator tempAllocator(L"ClassMemberNames", m_nodeAllocator.GetPageAllocator(), Parser::OutOfMemory);

    ParseNodePtr pnodeClass = null;
    if (buildAST)
    {
        pnodeClass = CreateNode(knopClassDecl);

        CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(ClassCount, m_scriptContext);
#ifdef LANGUAGE_SERVICE
        pnodeClass->sxClass.isDeclaration = (isDeclaration != FALSE);
#endif
    }

#ifdef LANGUAGE_SERVICE
    uint identMin = 0;
    uint identLim = 0;
#endif

    m_pscan->Scan();
    if (m_token.tk == tkID)
    {
        name = m_token.GetIdentifier(m_phtbl);
#ifdef LANGUAGE_SERVICE
        identMin = m_pscan->IchMinTok();
        identLim = m_pscan->IchLimTok();
#endif
        m_pscan->Scan();
    }
    else if (isDeclaration)
    {
        IdentifierExpectedError(m_token);
#if ERROR_RECOVERY
        name = m_pidError;
        identMin = m_pscan->IchMinTok();
        identLim = identMin;
#endif
    }

    if (isDeclaration && name == wellKnownPropertyPids.arguments && GetCurrentBlockInfo()->pnodeBlock->sxBlock.blockType == Function)
    {
        GetCurrentFunctionNode()->grfpn |= PNodeFlags::fpnArguments_overriddenByDecl;
    }

#if PARSENODE_EXTENSIONS
    if (buildAST && LanguageServiceMode())
    {
        m_languageServiceExtension->SetCompletionRange(ichDeadRangeMin + 1, m_pscan->IchMinTok(), LanguageServiceExtension::CompletionRangeMode::Others);
    }
#endif

    if (m_token.tk == tkEXTENDS)
    {
        m_pscan->Scan();
        pnodeExtends = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
    }

    if (m_token.tk != tkLCurly)
    {
        Error(ERRnoLcurly);
        SKIP(ERROR_RECOVERY_ACTUAL(ersExprStart | ers));
    }
#if PARSENODE_EXTENSIONS
    // Keep ichLCurlyMin if needed
    if (buildAST && LanguageServiceMode() && m_token.tk == tkLCurly)
        m_languageServiceExtension->SetLCurly(pnodeClass, m_pscan->IchMinTok());
#endif

    OUTPUT_TRACE_DEBUGONLY(Js::ES6VerboseFlag, L"Parsing class (%s) : %s\n", GetParseType(), name ? name->Psz() : L"anonymous class");

    ParseNodePtr pnodeDeclName = nullptr;
    if (isDeclaration)
    {
        pnodeDeclName = CreateBlockScopedDeclNode(name, knopLetDecl);
#ifdef LANGUAGE_SERVICE
        pnodeDeclName->ichMin = identMin;
        pnodeDeclName->ichLim = identLim;
        if (buildAST && LanguageServiceMode())
        {
            m_languageServiceExtension->SetIdentMin(pnodeDeclName, identMin);
            m_languageServiceExtension->SetIdentLim(pnodeDeclName, identLim);
        }
#endif
    }

    ParseNodePtr *ppnodeScopeSave = NULL;
    ParseNodePtr *ppnodeExprScopeSave = NULL;

    ParseNodePtr pnodeBlock = StartParseBlock<buildAST>(PnodeBlockType::Regular, ScopeType_Block);
    if (buildAST)
    {
        PushFuncBlockScope(pnodeBlock, &ppnodeScopeSave, &ppnodeExprScopeSave);
        pnodeClass->sxClass.pnodeBlock = pnodeBlock;
    }

    if (name)
    {
        pnodeName = CreateBlockScopedDeclNode(name, knopConstDecl);
    }

    RestorePoint beginClass;
    m_pscan->Capture(&beginClass);

    BOOL strictSave = m_fUseStrictMode;
    m_fUseStrictMode = TRUE;

#if ERROR_RECOVERY
    if (m_token.tk == tkLCurly)
#endif
        m_pscan->ScanForcingPid();

    IdentPtr pClassNamePid = pnodeName ? pnodeName->sxVar.pid : nullptr;

    for (;;)
    {
#ifdef LANGUAGE_SERVICE
        if (m_token.tk == tkEOF || m_token.tk == tkExternalSourceEnd)
        {
            break;
        }
#endif
        if (m_token.tk == tkSColon)
        {
            m_pscan->ScanForcingPid();
            continue;
        }
        if (m_token.tk == tkRCurly)
        {
#if PARSENODE_EXTENSIONS
            // Keep ichRCurlyMin if needed
            if (buildAST && LanguageServiceMode())
                m_languageServiceExtension->SetRCurly(pnodeClass, m_pscan->IchMinTok());
#endif
            break;
        }

        bool isStatic = m_token.tk == tkSTATIC;
        if (isStatic)
        {
            m_pscan->ScanForcingPid();
        }

        ushort fncDeclFlags = fFncNoName | fFncMethod | fFncClassMember;

        bool isGenerator = m_scriptContext->GetConfig()->IsES6GeneratorsEnabled() &&
                           m_token.tk == tkStar;
        if (isGenerator)
        {
            fncDeclFlags |= fFncGenerator;
            m_pscan->ScanForcingPid();
        }

        ParseNodePtr pnodeMemberName = nullptr;
        IdentPtr pidHint = nullptr;
        IdentPtr memberPid = nullptr;
        LPCOLESTR pMemberNameHint = nullptr;
        ulong     memberNameHintLength = 0;
        bool isComputedName = false;

        if (m_token.tk == tkLBrack && m_scriptContext->GetConfig()->IsES6ObjectLiteralsEnabled())
        {
            // Computed member name: [expr] () { }
            LPCOLESTR emptyHint = nullptr;
            ParseComputedName<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) &pnodeMemberName, &emptyHint, &pMemberNameHint, &memberNameHintLength);
            isComputedName = true;
        }
        else // not computed name
        {
            memberPid = this->ParseClassPropertyName(ERROR_RECOVERY_ACTUAL_(ersExprStart | ers) &pidHint);
            if (pidHint)
            {
                pMemberNameHint = pidHint->Psz();
                memberNameHintLength = pidHint->Cch();
            }
        }

        if (buildAST && memberPid)
        {
            pnodeMemberName = CreateStrNodeWithScanner(memberPid);
        }

        if (!isStatic && memberPid == wellKnownPropertyPids.constructor)
        {
            if (hasConstructor)
            {
                Error(ERRsyntax);
#if ERROR_RECOVERY
                Assert(pnodeConstructor);
                if (buildAST)
                {
                    // Save previous constructor as a class member with the error identifier,
                    // so we can get intellisense inside the function body
                    ParseNodePtr errMemberName = CreateStrNodeWithScanner(m_pidError);
                    errMemberName->ichMin = errMemberName->ichLim = pnodeConstructor->ichMin;
                    ParseNodePtr pnodeMember = CreateBinNode(knopMember, errMemberName, pnodeConstructor);
                    pnodeMember->sxBin.pnode2->sxFnc.hint = NULL;
                    pnodeMember->sxBin.pnode2->sxFnc.hintLength = 0;
                    pnodeMember->sxBin.pnode2->sxFnc.pid  = m_pidError;

                    AddToNodeList(&pnodeMembers, &lastMemberNodeRef, pnodeMember);
                }
#endif
            }
            hasConstructor = true;
            LPCOLESTR pConstructorName = nullptr;
            uint  constructorNameLength = 0;
            if (pnodeName && pnodeName->sxVar.pid)
            {
                pConstructorName = pnodeName->sxVar.pid->Psz();
                constructorNameLength = pnodeName->sxVar.pid->Cch();
            }
            else
            {
                pConstructorName = pNameHint;
                constructorNameLength = nameHintLength;
            }
            
            pnodeConstructor = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) fncDeclFlags, pConstructorName, false, /* needsPIDOnRCurlyScan */ true);
            // The constructor function will get the same name as class.
            pnodeConstructor->sxFnc.hint = pConstructorName;
            pnodeConstructor->sxFnc.hintLength = constructorNameLength;
            pnodeConstructor->sxFnc.pid = pnodeName && pnodeName->sxVar.pid ? pnodeName->sxVar.pid : wellKnownPropertyPids.constructor;
            pnodeConstructor->sxFnc.SetIsClassConstructor(TRUE);
        }
        else
        {
            ParseNodePtr pnodeMember = NULL;

            bool isMemberNamedGetOrSet = false;
            RestorePoint beginMethodName;
            m_pscan->Capture(&beginMethodName);
            if (memberPid == wellKnownPropertyPids.getter || memberPid == wellKnownPropertyPids.setter)
            {
                m_pscan->ScanForcingPid();
            }
            if (m_token.tk == tkLParen)
            {
                m_pscan->SeekTo(beginMethodName);
                isMemberNamedGetOrSet = true;
            }

            if ((memberPid == wellKnownPropertyPids.getter || memberPid == wellKnownPropertyPids.setter) && !isMemberNamedGetOrSet)
            {
                bool isGetter = (memberPid == wellKnownPropertyPids.getter);

                if (m_token.tk == tkLBrack && m_scriptContext->GetConfig()->IsES6ObjectLiteralsEnabled())
                {
                    // Computed get/set member name: get|set [expr] () { }
                    LPCOLESTR emptyHint = nullptr;
                    ParseComputedName<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) &pnodeMemberName, &emptyHint, &pMemberNameHint, &memberNameHintLength);
                    isComputedName = true;
                }
                else // not computed name
                {
                    memberPid = this->ParseClassPropertyName(ERROR_RECOVERY_ACTUAL_(ersExprStart | ers) &pidHint);
                }

                if (isStatic ? (memberPid == wellKnownPropertyPids.prototype) : (memberPid == wellKnownPropertyPids.constructor))
                {
                    Error(ERRsyntax);
#if ERROR_RECOVERY
                    memberPid = m_pidDeclError;
                    pidHint = NULL;
                    pMemberNameHint = nullptr;
                    m_pscan->Scan();
#endif
                }
                if (buildAST && memberPid && !isComputedName)
                {
                    pnodeMemberName = CreateStrNodeWithScanner(memberPid);
                }

                ParseNodePtr pnodeFnc = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) (isGetter ? fFncNoArg : fFncSetter) | fncDeclFlags, pidHint ? pidHint->Psz() : nullptr, false, /* needsPIDOnRCurlyScan */ true);
                pnodeFnc->sxFnc.SetIsStaticMember(isStatic);

                if (buildAST)
                {
                    pnodeFnc->sxFnc.SetIsAccessor();
                    pnodeMember = CreateBinNode(isGetter ? knopGetMember : knopSetMember, pnodeMemberName, pnodeFnc);
                    pMemberNameHint = ConstructFinalHintNode(pClassNamePid, pidHint, isGetter ? wellKnownPropertyPids.getter : wellKnownPropertyPids.setter, isStatic, &memberNameHintLength, isComputedName, pMemberNameHint);
                }
            }
            else
            {
                if (isStatic && memberPid == wellKnownPropertyPids.prototype)
                {
                    Error(ERRsyntax);
#if ERROR_RECOVERY
                    memberPid = m_pidDeclError;
                    pidHint = NULL;
                    pMemberNameHint = nullptr;
                    m_pscan->Scan();
                    if (buildAST && memberPid)
                    {
                        pnodeMemberName = CreateStrNodeWithScanner(memberPid);
                    }
#endif
                }

                ParseNodePtr pnodeFunc = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) fncDeclFlags,  pidHint ? pidHint->Psz() : nullptr, false, /* needsPIDOnRCurlyScan */ true);
                pnodeFunc->sxFnc.SetIsStaticMember(isStatic);

                if (buildAST)
                {
                    pnodeMember = CreateBinNode(knopMember, pnodeMemberName, pnodeFunc);
                    pMemberNameHint = ConstructFinalHintNode(pClassNamePid, pidHint, nullptr /*pgetset*/, isStatic, &memberNameHintLength, isComputedName, pMemberNameHint);
                }
            }

            if (buildAST)
            {
                pnodeMember->sxBin.pnode2->sxFnc.hint = pMemberNameHint; // Fully qualified name
                pnodeMember->sxBin.pnode2->sxFnc.hintLength = memberNameHintLength;
                pnodeMember->sxBin.pnode2->sxFnc.pid  = memberPid; // Short name

                AddToNodeList(isStatic ? &pnodeStaticMembers : &pnodeMembers, isStatic ? &lastStaticMemberNodeRef : &lastMemberNodeRef, pnodeMember);
            }
        }
    }

    if (!hasConstructor)
    {
        OUTPUT_TRACE_DEBUGONLY(Js::ES6VerboseFlag, L"Generating contructor (%s) : %s\n", GetParseType(), name ? name->Psz() : L"anonymous class");

        RestorePoint endClass;
        m_pscan->Capture(&endClass);
        m_pscan->SeekTo(beginClass);

        pnodeConstructor = GenerateEmptyConstructor<buildAST>(pnodeExtends != nullptr);
        if (buildAST)
        {
            if (pClassNamePid)
            {
                pnodeConstructor->sxFnc.hint = pClassNamePid->Psz();
                pnodeConstructor->sxFnc.hintLength = pClassNamePid->Cch();
            }
            else
            {
                pnodeConstructor->sxFnc.hint = pNameHint;
                pnodeConstructor->sxFnc.hintLength = nameHintLength;
            }
            pnodeConstructor->sxFnc.pid = pClassNamePid;
        }

        m_pscan->SeekTo(endClass);
    }

    if (buildAST)
    {
        PopFuncBlockScope(ppnodeScopeSave, ppnodeExprScopeSave);

        pnodeClass->sxClass.pnodeDeclName = pnodeDeclName;
        pnodeClass->sxClass.pnodeName = pnodeName;
        pnodeClass->sxClass.pnodeConstructor = pnodeConstructor;
        pnodeClass->sxClass.pnodeExtends = pnodeExtends;
        pnodeClass->sxClass.pnodeMembers = pnodeMembers;
        pnodeClass->sxClass.pnodeStaticMembers = pnodeStaticMembers;
    }
    FinishParseBlock(pnodeBlock);

#ifdef LANGUAGE_SERVICE
    if (buildAST)
    {
        pnodeClass->ichLim = m_pscan->IchLimTok();
    }
#endif

    m_fUseStrictMode = strictSave;

    m_pscan->Scan();

    return pnodeClass;
}

template<bool buildAST>
ParseNodePtr Parser::ParseStringTemplateDecl(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnodeTagFnc)
{
    ParseNodePtr pnodeStringLiterals = nullptr;
    ParseNodePtr* lastStringLiteralNodeRef = nullptr;
    ParseNodePtr pnodeRawStringLiterals = nullptr;
    ParseNodePtr* lastRawStringLiteralNodeRef = nullptr;
    ParseNodePtr pnodeSubstitutionExpressions = nullptr;
    ParseNodePtr* lastSubstitutionExpressionNodeRef = nullptr;
    ParseNodePtr pnodeTagFncArgs = nullptr;
    ParseNodePtr* lastTagFncArgNodeRef = nullptr;
    ParseNodePtr stringLiteral = nullptr;
    ParseNodePtr stringLiteralRaw = nullptr;
    ParseNodePtr pnodeStringTemplate = nullptr;
    bool templateClosed = false;
    const bool isTagged = pnodeTagFnc != nullptr;
    uint16 stringConstantCount = 0;
    charcount_t ichMin = 0;

    Assert(m_token.tk == tkStrTmplBasic || m_token.tk == tkStrTmplBegin);

    if (buildAST)
    {
        pnodeStringTemplate = CreateNode(knopStrTemplate);
        pnodeStringTemplate->sxStrTemplate.countStringLiterals = 0;
        pnodeStringTemplate->sxStrTemplate.isTaggedTemplate = isTagged ? TRUE : FALSE;

        // If this is a tagged string template, we need to start building the arg list for the call
        if (isTagged)
        {
            ichMin = pnodeTagFnc->ichMin;
            AddToNodeListEscapedUse(&pnodeTagFncArgs, &lastTagFncArgNodeRef, pnodeStringTemplate);
        }

    }
    CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(StringTemplatesCount, m_scriptContext);

    OUTPUT_TRACE_DEBUGONLY(
        Js::StringTemplateParsePhase,
        L"Starting to parse a string template (%s)...\n\tis tagged = %s\n",
        GetParseType(),
        isTagged ? L"true" : L"false (Raw and cooked strings will not differ!)");

    // String template grammar
    // `...`   Simple string template
    // `...${  String template beginning
    // }...${  String template middle
    // }...`   String template end
    while (!templateClosed)
    {
        // First, extract the string constant part - we always have one
        if (IsStrictMode() && m_pscan->IsOctOrLeadingZeroOnLastTKNumber())
        {
            Error(ERRES5NoOctal);
        }

        // We are not able to pass more than a ushort worth of arguments to the tag
        // so use that as a logical limit on the number of string constant pieces.
        if (stringConstantCount >= USHORT_MAX)
        {
            Error(ERRnoMemory);
        }

        // Keep track of the string literal count (must be the same for raw strings)
        // We use this in code gen so we don't need to count the string literals list
        stringConstantCount++;

        // If we are not creating parse nodes, there is no need to create strings
        if (buildAST)
        {
            stringLiteral = CreateStrNodeWithScanner(m_token.GetStr());

            AddToNodeList(&pnodeStringLiterals, &lastStringLiteralNodeRef, stringLiteral);

            // We only need to collect a raw string when we are going to pass the string template to a tag
            if (isTagged)
            {
                // Make the scanner create a pid for the raw string constant for the preceding scan
                IdentPtr pid = m_pscan->GetSecondaryBufferAsPid();

                stringLiteralRaw = CreateStrNodeWithScanner(pid);

                // Should have gotten a raw string literal above
                AddToNodeList(&pnodeRawStringLiterals, &lastRawStringLiteralNodeRef, stringLiteralRaw);
            }
            else
            {
#if DBG
                // Assign the raw string for debug tracing below
                stringLiteralRaw = stringLiteral;
#endif
            }

            OUTPUT_TRACE_DEBUGONLY(
                Js::StringTemplateParsePhase,
                L"Parsed string constant: \n\tcooked = \"%s\" \n\traw = \"%s\" \n\tdiffer = %d\n",
                stringLiteral->sxPid.pid->Psz(),
                stringLiteralRaw->sxPid.pid->Psz(),
                stringLiteral->sxPid.pid->Psz() == stringLiteralRaw->sxPid.pid->Psz() ? 0 : 1);
        }

        switch (m_token.tk)
        {
        case tkStrTmplEnd:
        case tkStrTmplBasic:
            // We do not need to parse an expression for either the end or basic string template tokens
            templateClosed = true;
            break;
        case tkStrTmplBegin:
        case tkStrTmplMid:
            {
            // In the middle or begin string template token case, we need to parse an expression next
            m_pscan->Scan();

            // Parse the contents of the curly braces as an expression
            ParseNodePtr expression = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersRCurly | ers) 0);

            // After parsing expression, scan should leave us with an RCurly token.
            // Use the NoScan version so we do not automatically perform a scan - we need to
            // set the scan state before next scan but we don't want to set that state if
            // the token is not as expected since we'll error in that case.
            ChkCurTokNoScan(tkRCurly, ERRnoRcurly _ERROR_RECOVERY_ACTUAL(ers));

            // Notify the scanner that it should scan for a middle or end string template token
            m_pscan->SetScanState(Scanner_t::ScanState::ScanStateStringTemplateMiddleOrEnd);
            m_pscan->Scan();

            if (buildAST)
            {
                // If we are going to call the tag function, add this expression into the list of args
                if (isTagged)
                {
                    AddToNodeListEscapedUse(&pnodeTagFncArgs, &lastTagFncArgNodeRef, expression);
                }
                else
                {
                    // Otherwise add it to the substitution expression list
                    // TODO: Store the arguments and substitution expressions in a single list?
                    AddToNodeList(&pnodeSubstitutionExpressions, &lastSubstitutionExpressionNodeRef, expression);
                }
            }

            if (!(m_token.tk == tkStrTmplMid || m_token.tk == tkStrTmplEnd))
            {
                // Scan with ScanState ScanStateStringTemplateMiddleOrEnd should only return
                // tkStrTmpMid/End unless it is EOF or tkScanError
                Assert(m_token.tk == tkEOF || m_token.tk == tkScanError);
#if ERROR_RECOVERY                
                m_token.tk = tkStrTmplEnd;
                m_token.SetIdentifier(m_pidError);                                
#endif
                Error(ERRsyntax);
            }
            
            OUTPUT_TRACE_DEBUGONLY(Js::StringTemplateParsePhase, L"Parsed expression\n");
            }
            break;
        default:
            Assert(false);
            break;
        }
    }

    if (buildAST)
    {
        pnodeStringTemplate->sxStrTemplate.pnodeStringLiterals = pnodeStringLiterals;
        pnodeStringTemplate->sxStrTemplate.pnodeStringRawLiterals = pnodeRawStringLiterals;
        pnodeStringTemplate->sxStrTemplate.pnodeSubstitutionExpressions = pnodeSubstitutionExpressions;
        pnodeStringTemplate->sxStrTemplate.countStringLiterals = stringConstantCount;

        // We should still have the last string literal.
        // Use the char offset of the end of that constant as the end of the string template.
        pnodeStringTemplate->ichLim = stringLiteral->ichLim;

        // If this is a tagged template, we now have the argument list and can construct a call node
        if (isTagged)
        {
            // Return the call node here and let the byte code generator Emit the string template automagically
            pnodeStringTemplate = CreateCallNode(knopCall, pnodeTagFnc, pnodeTagFncArgs, ichMin, pnodeStringTemplate->ichLim);

            // We need to set the arg count explicitly
            pnodeStringTemplate->sxCall.argCount = stringConstantCount;
        }
    }

    m_pscan->Scan();

    return pnodeStringTemplate;
}

LPCOLESTR Parser::FormatPropertyString(LPCOLESTR propertyString, ParseNodePtr pNode, ulong *fullNameHintLength)
{
    // propertyString could be null, such as 'this.foo' =
    // propertyString could be empty, found in pattern as in (-1)[""][(x = z)]

    OpCode op = pNode->nop;
    LPCOLESTR rightNode = nullptr;
    if (propertyString == nullptr)
    {
        propertyString = L"";
    }

    if (op != knopInt && op != knopFlt && op != knopName && op != knopStr)
    {
        rightNode = L"";
    }
    else if (op == knopStr)
    {
        return AppendNameHints(propertyString, pNode->sxPid.pid, fullNameHintLength, false, true/*add brackets*/);
    }
    else if(op == knopFlt)
    {
        rightNode = m_pscan->StringFromDbl(pNode->sxFlt.dbl);
    }
    else
    {
        rightNode = op == knopInt ? m_pscan->StringFromLong(pNode->sxInt.lw)
            : pNode->sxPid.pid->Psz();
    }
   
    return AppendNameHints(propertyString, rightNode, fullNameHintLength, false, true/*add brackets*/);
}

LPCOLESTR Parser::ConstructNameHint(ParseNodePtr pNode, ulong* fullNameHintLength)
{
    Assert(pNode != nullptr);
    Assert(pNode->nop == knopDot || pNode->nop == knopIndex);
    LPCOLESTR leftNode = nullptr;
    if (pNode->sxBin.pnode1->nop == knopDot || pNode->sxBin.pnode1->nop == knopIndex)
    {
        leftNode = ConstructNameHint(pNode->sxBin.pnode1, fullNameHintLength);
    }
    else if (pNode->sxBin.pnode1->nop == knopName)
    {
        leftNode = pNode->sxBin.pnode1->sxPid.pid->Psz();
        *fullNameHintLength = pNode->sxBin.pnode1->sxPid.pid->Cch();
    }

    if (pNode->nop == knopIndex)
    {
        return FormatPropertyString(
            leftNode ? leftNode : Js::Constants::AnonymousFunction, // e.g. f()[0] = function () {}
            pNode->sxBin.pnode2, fullNameHintLength);
    }

    // REVIEW:Should the right side will always be dot or name?
    Assert(pNode->sxBin.pnode2->nop == knopDot || pNode->sxBin.pnode2->nop == knopName);

    LPCOLESTR rightNode = nullptr;
    bool wrapWithBrackets = false;
    if (pNode->sxBin.pnode2->nop == knopDot)
    {
        rightNode = ConstructNameHint(pNode->sxBin.pnode2, fullNameHintLength);
    }
    else
    {
        rightNode = pNode->sxBin.pnode2->sxPid.pid->Psz();
        wrapWithBrackets = PNodeFlags::fpnIndexOperator == (pNode->grfpn & PNodeFlags::fpnIndexOperator);
    }
    Assert(rightNode != nullptr);
    return AppendNameHints(leftNode, rightNode, fullNameHintLength, false, wrapWithBrackets);
}

LPCOLESTR Parser::AppendNameHints(LPCOLESTR leftStr, ulong leftLen, LPCOLESTR rightStr, ulong rightLen, ulong *returnLength, bool ignoreAddDotWithSpace, bool wrapInBrackets)
{
    Assert(rightStr != nullptr);
    Assert(leftLen  != 0 || wrapInBrackets);
    Assert(rightLen != 0 || wrapInBrackets);

    bool ignoreDot = rightStr[0] == L'[' && !wrapInBrackets;//if we wrap in brackets it can be a string literal which can have brackets at the first char
    ulong totalLength = leftLen + rightLen + ((ignoreDot) ? 1 : 2); // 1 (for dot or [) + 1 (for null termination)

    if (wrapInBrackets)
    {
        totalLength++; //1 for ']';
    }
    WCHAR * finalName = AllocateStringOfLength(totalLength);
    
    if (leftStr != nullptr && leftLen != 0)
    {
        wcscpy_s(finalName, leftLen + 1, leftStr);
    }

    if (ignoreAddDotWithSpace)
    {
        finalName[leftLen++] = (OLECHAR)L' ';
    }
    // mutually exclusive from ignoreAddDotWithSpace which is used for getters/setters

    else if (wrapInBrackets)
    {
        finalName[leftLen++] = (OLECHAR)L'[';
        finalName[totalLength-2] = (OLECHAR)L']';
    }
    else if (!ignoreDot)
    {
        finalName[leftLen++] = (OLECHAR)L'.';
    }
    //ignore case falls through
    js_wmemcpy_s(finalName + leftLen, rightLen, rightStr, rightLen);
    finalName[totalLength-1] = (OLECHAR)L'\0';
    
    if (returnLength != nullptr)
    {
        *returnLength = totalLength - 1;
    }

    return finalName;
}

WCHAR * Parser::AllocateStringOfLength(ulong length)
{
    Assert(length > 0);
    ULONG totalBytes;
    if (ULongMult(length, sizeof(OLECHAR), &totalBytes) != S_OK)
    {
        Error(ERRnoMemory);
    }
    WCHAR* finalName = (WCHAR*)m_phtbl->GetAllocator()->Alloc(totalBytes);
    if (finalName == nullptr)
    {
        Error(ERRnoMemory);
    }
    return finalName;
}

LPCOLESTR Parser::AppendNameHints(IdentPtr left, IdentPtr right, ulong *returnLength, bool ignoreAddDotWithSpace, bool wrapInBrackets)
{
    
    if (left == nullptr && !wrapInBrackets)
    {
        if (right)
        {
            *returnLength = right->Cch();
            return right->Psz();
        }
        return nullptr;
    }

    ulong leftLen = 0;
    LPCOLESTR leftStr = L"";

    if (left != nullptr) // if wrapInBrackets is true
    {
        leftStr = left->Psz();
        leftLen = left->Cch();    
    }

    if (right == nullptr)
    {
        *returnLength = leftLen;
        return left->Psz();
    }
    ulong rightLen = right->Cch();

    return AppendNameHints(leftStr, leftLen, right->Psz(), rightLen, returnLength, ignoreAddDotWithSpace, wrapInBrackets);
}

LPCOLESTR Parser::AppendNameHints(IdentPtr left, LPCOLESTR right, ulong *returnLength, bool ignoreAddDotWithSpace, bool wrapInBrackets)
{
    ulong leftLen = 0, rightLen = 0;

    if (left == nullptr && !wrapInBrackets)
    {
        *returnLength = right ? wcslen(right) : 0;
        return right;
    }

    LPCOLESTR leftStr = L"";

    if (left != nullptr) // if wrapInBrackets is true
    {
        leftStr = left->Psz();
        leftLen = left->Cch();
    }
    
    if ((right == nullptr || ((rightLen = wcslen(right)) == 0)) && !wrapInBrackets)
    {
        *returnLength = leftLen;
        return left->Psz();
    }

    return AppendNameHints(leftStr, leftLen, right, rightLen, returnLength, ignoreAddDotWithSpace, wrapInBrackets);
}

LPCOLESTR Parser::AppendNameHints(LPCOLESTR left, IdentPtr right, ulong *returnLength, bool ignoreAddDotWithSpace, bool wrapInBrackets)
{
    ulong leftLen = 0, rightLen = 0;
    if (left == nullptr || ((leftLen = wcslen(left)) == 0) && !wrapInBrackets)
    {
        if (right)
        {
            *returnLength = right->Cch();
            return right->Psz();
        }
        return nullptr;
    }

    if (right == nullptr)
    {
        *returnLength = leftLen;
        return left;
    }
    rightLen = right->Cch();
    
    return AppendNameHints(left, leftLen, right->Psz(), rightLen, returnLength, ignoreAddDotWithSpace, wrapInBrackets);
}


LPCOLESTR Parser::AppendNameHints(LPCOLESTR left, LPCOLESTR right, ulong *returnLength, bool ignoreAddDotWithSpace, bool wrapInBrackets)
{
    ulong leftLen = 0, rightLen = 0;
    if ((left == nullptr || ((leftLen = wcslen(left)) == 0)) && !wrapInBrackets)
    {
        *returnLength = right ? wcslen(right) : 0;
        return right;
    }

    if ((right == nullptr || ((rightLen = wcslen(right)) == 0)) && !wrapInBrackets)
    {
        *returnLength = leftLen;
        return left;
    }

    return AppendNameHints(left, leftLen, right, rightLen, returnLength, ignoreAddDotWithSpace, wrapInBrackets);
}

/**
 * Emits a spread error if there is no ambiguity, or marks defers the error for
 * when we can determine if it is a rest error or a spread error.
 *
 * The ambiguity arises when we are parsing a lambda parameter list but we have
 * not seen the => token. At this point, we are either in a parenthesized
 * expression or a parameter list, and cannot issue an error until the matching
 * RParen has been scanned.
 *
 * The actual emission of the error happens in ParseExpr, when we first know if
 * the expression is a lambda parameter list or not.
 *
 */
void Parser::DeferOrEmitPotentialSpreadError(ParseNodePtr pnodeT)
{
    if (m_parenDepth > 0)
    {
        if (m_token.tk == tkRParen)
        {
           if (!m_deferEllipsisError)
            {
                // Capture only the first error instance.
                m_pscan->Capture(&m_EllipsisErrLoc);
                m_deferEllipsisError = true;
            }
        }
        else
        {
            Error(ERRUnexpectedEllipsis);
        }
    }
    else
    {
        Error(ERRInvalidSpreadUse);
    }
}

/***************************************************************************
Parse an optional sub expression returning null if there was no expression.
Checks for no expression by looking for a token that can follow an
Expression grammar production.
***************************************************************************/
template<bool buildAST>
bool Parser::ParseOptionalExpr(ERROR_RECOVERY_FORMAL_ ParseNodePtr* pnode, int oplMin, BOOL *pfCanAssign, BOOL fAllowIn, BOOL fAllowEllipsis, LPCOLESTR pNameHint, ulong *pHintLength, _Inout_opt_ IdentToken* pToken)
{
    *pnode = nullptr;
    if (m_token.tk == tkRCurly ||
        m_token.tk == tkRBrack ||
        m_token.tk == tkRParen ||
        m_token.tk == tkSColon ||
        m_token.tk == tkColon ||
        m_token.tk == tkComma ||
        m_token.tk == tkLimKwd ||
        m_pscan->FHadNewLine())
    {
        return false;
    }

    *pnode = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) oplMin, pfCanAssign, fAllowIn, fAllowEllipsis, pNameHint, pHintLength, pToken);
    return true;
}

/***************************************************************************
Parse a sub expression.
'fAllowIn' indicates if the 'in' operator should be allowed in the initializing
expression ( it is not allowed in the context of the first expression in a  'for' loop).
***************************************************************************/
template<bool buildAST>
ParseNodePtr Parser::ParseExpr(ERROR_RECOVERY_FORMAL_ int oplMin, BOOL *pfCanAssign, BOOL fAllowIn, BOOL fAllowEllipsis, LPCOLESTR pNameHint, ulong *pHintLength, _Inout_opt_ IdentToken* pToken, bool fUnaryOrParen)
{
    Assert(pToken == nullptr || pToken->tk == tkNone); // Must be empty initially
    int opl;
    OpCode nop;
    charcount_t ichMin;
    ParseNodePtr pnode = nullptr;
    ParseNodePtr pnodeT = nullptr;
    BOOL fCanAssign = TRUE;
    bool assignmentStmt = false;
    IdentToken term;
    RestorePoint termStart;
    this->m_exprDepth++;
    ulong hintLength = 0;

    if (pHintLength != 0)
    {
        hintLength = *pHintLength;
    }

    EnsureStackAvailable();

    // Is the current token a unary operator?
    if (m_phtbl->TokIsUnop(m_token.tk, &opl, &nop) && nop != knopNone)
    {
        IdentToken operandToken;
        ichMin = m_pscan->IchMinTok();

        if (nop == knopYield)
        {
            if (!m_pscan->YieldIsKeyword() || oplMin > opl)
            {
                // The case where 'yield' is scanned as a keyword (tkYIELD) but the scanner
                // is not treating yield as a keyword (!m_pscan->YieldIsKeyword()) happens
                // in strict mode non-generator function contexts.
                //
                // That is, 'yield' is a keyword because of strict mode, but YieldExpression
                // is not a grammar production outside of generator functions.
                //
                // Otherwise it is an error for a yield to appear in the context of a higher level
                // binding operator, be it unary or binary.
                Error(ERRsyntax);

#if ERROR_RECOVERY
                nop = knopEmpty;
                if (buildAST) { pnode = CreateNodeWithScanner<knopEmpty>(); }
#endif
            }
        }

        m_pscan->Scan();
        fCanAssign = FALSE;

        if (nop == knopYield && !m_pscan->FHadNewLine() && m_token.tk == tkStar)
        {
            m_pscan->Scan();
            nop = knopYieldStar;
        }

        if (nop == knopYield)
        {
            if (!ParseOptionalExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersBinOp | ers) &pnodeT, opl, NULL, TRUE, fAllowEllipsis))
            {
                nop = knopYieldLeaf;
                if (buildAST)
                {
                    pnode = CreateNodeT<knopYieldLeaf>(ichMin, m_pscan->IchLimTok());
                }
            }
        }
        else
        {
            // Disallow spread after a Ellipsis token. This prevents chaining, and ensures spread is the top level expression.
            pnodeT = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersBinOp | ers) opl, NULL, TRUE, nop != knopEllipsis && fAllowEllipsis, nullptr /*hint*/, nullptr /*hintLength*/, &operandToken, true);
        }

        if (nop != knopYieldLeaf
#if ERROR_RECOVERY
            && nop != knopEmpty
#endif
            )
        {
            if (nop == knopIncPre || nop == knopDecPre)
            {
                TrackAssignment<buildAST>(pnodeT, &operandToken, ichMin, m_pscan->IchLimTok());
                if (buildAST)
                {
                    if (IsStrictMode() && pnodeT->nop == knopName)
                    {
                        CheckStrictModeEvalArgumentsUsage(pnodeT->sxPid.pid);
                    }
                }
                else
                {
                    if (IsStrictMode() && operandToken.tk == tkID)
                    {
                        CheckStrictModeEvalArgumentsUsage(operandToken.pid);
                    }
                }
            }
            else if (nop == knopEllipsis && !fAllowEllipsis)
            {
                DeferOrEmitPotentialSpreadError(pnodeT);
            }

            if (buildAST)
            {
                //Do not do the folding for Asm in case of KnopPos as we need this to determine the type
                if (nop == knopPos && (pnodeT->nop == knopInt || pnodeT->nop == knopFlt) && !this->m_InAsmMode)
                {
                    // Fold away a unary '+' on a number.
                    pnode = pnodeT;
                }
                else if (nop == knopNeg &&
                    ((pnodeT->nop == knopInt && pnodeT->sxInt.lw != 0) ||
                    (pnodeT->nop == knopFlt && (pnodeT->sxFlt.dbl != 0 || this->m_InAsmMode))))
                {
                    // Fold a unary '-' on a number into the value of the number itself.
                    pnode = pnodeT;
                    if (pnode->nop == knopInt)
                    {
                        pnode->sxInt.lw = -pnode->sxInt.lw;
                    }
                    else
                    {
                        pnode->sxFlt.dbl = -pnode->sxFlt.dbl;
                    }
                }
                else
                {
                    pnode = CreateUniNode(nop, pnodeT);
                    this->CheckArguments(pnode->sxUni.pnode1);
                }
                pnode->ichMin = ichMin;
            }

            if (nop == knopDelete)
            {
                if (IsStrictMode())
                {
                    if ((buildAST && pnode->sxUni.pnode1->nop == knopName) ||
                        (!buildAST && operandToken.tk == tkID))
                    {
                        Error(ERRInvalidDelete);
                    }
                }

                if (buildAST)
                {
                    ParseNodePtr pnode1 = pnode->sxUni.pnode1;
                    if (m_currentNodeFunc)
                    {
                        if (pnode1->nop == knopDot || pnode1->nop == knopIndex)
                        {
                            // If we delete an arguments property, use the conservative,
                            // heap-allocated arguments object.
                            this->CheckArguments(pnode1->sxBin.pnode1);
                        }
                    }
                }
            }
        }
    }
    else
    {
        tokens beforeToken = m_token.tk;
        m_pscan->Capture(&termStart);
        ichMin = m_pscan->IchMinTok();
        pnode = ParseTerm<buildAST>(ERROR_RECOVERY_ACTUAL_((ersBinOp | ersAddOp) | ers) TRUE, pNameHint, &hintLength, &term, fUnaryOrParen);

        if (m_scriptContext->GetConfig()->IsES6DestructuringEnabled() && beforeToken == tkLBrack && m_token.tk == tkAsg)
        {
            // Possible destructuring literal. Rewind and verify the parse tree.
            m_pscan->SeekTo(termStart);

            // No need to rebuild the nodes, we only need to verify the destructuring syntax rules.
            ParseDestructuredArrayLiteral<false>(ERROR_RECOVERY_ACTUAL_(ers) tkNone, false);
        }

        if (buildAST)
        {
            pNameHint = NULL;
            if (pnode->nop == knopName)
            {
                pNameHint = pnode->sxPid.pid->Psz();
                hintLength = pnode->sxPid.pid->Cch();
            }
            else if (pnode->nop == knopDot || pnode->nop == knopIndex)
            {
                if (CONFIG_FLAG(UseFullName))
                {
                    pNameHint = ConstructNameHint(pnode, &hintLength);
                }
                else
                {
                    ParseNodePtr pnodeName = pnode;
                    while (pnodeName->nop == knopDot)
                    {
                        pnodeName = pnodeName->sxBin.pnode2;
                    }

                    if (pnodeName->nop == knopName)
                    {
                        pNameHint = pnodeName->sxPid.pid->Psz();
                        hintLength = pnodeName->sxPid.pid->Cch();
                    }
                }
            }
        }

        // Check for postfix unary operators.
        if (!m_pscan->FHadNewLine() &&
            (tkInc == m_token.tk || tkDec == m_token.tk))
        {
            TrackAssignment<buildAST>(pnode, &term, ichMin, m_pscan->IchLimTok());
            fCanAssign = FALSE;
            if (buildAST)
            {
                if (IsStrictMode() && pnode->nop == knopName)
                {
                    CheckStrictModeEvalArgumentsUsage(pnode->sxPid.pid);
                }
                this->CheckArguments(pnode);
                pnode = CreateUniNode(tkInc == m_token.tk ? knopIncPost : knopDecPost, pnode);
                pnode->ichLim = m_pscan->IchLimTok();
            }
            else
            {
                if (IsStrictMode() && term.tk == tkID)
                {
                    CheckStrictModeEvalArgumentsUsage(term.pid);
                }
                // This expression is not an identifier
                term.tk = tkNone;
            }
            m_pscan->Scan();
        }
    }

    // Process a sequence of operators and operands.
    for (;;)
    {
        if (!m_phtbl->TokIsBinop(m_token.tk, &opl, &nop) || nop == knopNone)
        {
            break;
        }
        if ( ! fAllowIn && nop == knopIn )
        {
            break;
        }
        Assert(opl != koplNo);

        if (opl == koplAsg && m_token.tk != tkDArrow)
        {
            // Assignment operator. These are the only right associative
            // binary operators. We also need to special case the left
            // operand - it should only be a LeftHandSideExpression.
            Assert(ParseNode::Grfnop(nop) & fnopAsg || nop == knopFncDecl);
            TrackAssignment<buildAST>(pnode, &term, ichMin, m_pscan->IchLimTok());
            if (buildAST)
            {
                if (IsStrictMode() && pnode->nop == knopName)
                {
                    CheckStrictModeEvalArgumentsUsage(pnode->sxPid.pid);
                }

                // Assignment stmt of the form "this.<id> = <expr>"
                if (nop == knopAsg && pnode->nop == knopDot && pnode->sxBin.pnode1->nop == knopThis && pnode->sxBin.pnode2->nop == knopName)
                {
                    if (pnode->sxBin.pnode2->sxPid.pid != wellKnownPropertyPids.__proto__)
                    {
                        assignmentStmt = true;
                    }
#if PARSENODE_EXTENSIONS
                    if (LanguageServiceMode() && m_currentNodeFunc)
                        m_currentNodeFunc->sxFnc.SetHasThisStmt();
#endif
                }
            }
            else
            {
                if (IsStrictMode() && term.tk == tkID)
                {
                    CheckStrictModeEvalArgumentsUsage(term.pid);
                }
            }

            if (opl < oplMin)
            {
                break;
            }
            if (!fCanAssign)
            {
                Error(ERRsyntax);
                // No recovery necessary since this is a semantic, not structural, error.
            }
        }
        else if (opl <= oplMin && m_token.tk != tkDArrow)
        {
            break;
        }

        // This expression is not an identifier
        term.tk = tkNone;

        // Precedence is high enough. Consume the operator token.
        m_pscan->Scan();
        fCanAssign = FALSE;

        // Special case the "?:" operator
        if (nop == knopQmark)
        {
            pnodeT = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersColon | ers) koplAsg, NULL, fAllowIn);
            ChkCurTok(tkColon, ERRnoColon _ERROR_RECOVERY_ACTUAL(ersExprStart | ers));
            ParseNodePtr pnodeT2 = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersBinOp | ers) koplAsg, NULL, fAllowIn);
            if (buildAST)
            {
                pnode = CreateTriNode(nop, pnode, pnodeT, pnodeT2);
                this->CheckArguments(pnode->sxTri.pnode2);
                this->CheckArguments(pnode->sxTri.pnode3);
            }
        }
        else if (nop == knopFncDecl)
        {
            m_pscan->SeekTo(termStart);
            pnode = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) fFncLambda, nullptr, false);
        }
        else
        {
            // Parse the operand, make a new node, and look for more
            pnodeT = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersBinOp | ers) opl, NULL, fAllowIn, FALSE, pNameHint, &hintLength, nullptr);

            if (buildAST)
            {
                pnode = CreateBinNode(nop, pnode, pnodeT);
                Assert(pnode->sxBin.pnode2 != NULL);
                if (pnode->sxBin.pnode2->nop == knopFncDecl)
                {
                    pnode->sxBin.pnode2->sxFnc.hint = pNameHint;
                    pnode->sxBin.pnode2->sxFnc.hintLength = hintLength;
                    if (pnode->sxBin.pnode1->nop == knopDot)
                    {
                        pnode->sxBin.pnode2->sxFnc.isNameIdentifierRef  = false;
                    }
                }
                if (pnode->sxBin.pnode2->nop == knopClassDecl && pnode->sxBin.pnode1->nop == knopDot)
                {
                    Assert(pnode->sxBin.pnode2->sxClass.pnodeConstructor);
                    pnode->sxBin.pnode2->sxClass.pnodeConstructor->sxFnc.isNameIdentifierRef  = false;
                }
            }
            pNameHint = NULL;
        }
    }

    if (buildAST)
    {
        if (!assignmentStmt)
        {
            // Don't set the flag for following nodes
            switch (pnode->nop)
            {
            case knopName:
            case knopInt:
            case knopFlt:
            case knopStr:
            case knopRegExp:
            case knopNull:
            case knopFalse:
            case knopTrue:
                break;
            default:
                if (m_currentNodeFunc)
                {
                    m_currentNodeFunc->sxFnc.SetHasNonThisStmt();
                }
                else if (m_currentNodeProg)
                {
                    m_currentNodeProg->sxFnc.SetHasNonThisStmt();
                }
            }
        }
    }

    this->m_exprDepth--;

    if (NULL != pfCanAssign)
    {
        *pfCanAssign = fCanAssign;
    }

    // Pass back identifier if requested
    if (pToken && term.tk == tkID)
    {
        *pToken = term;
    }

    //Track "obj.a" assignment patterns here - Promote the Assignment state for the property's pid.
    // This includes =, += etc.
    if (pnode != NULL)
    {
        uint nodeType = ParseNode::Grfnop(pnode->nop);
        if (nodeType & fnopAsg)
        {
            if (nodeType & fnopBin)
            {
                ParseNodePtr lhs = pnode->sxBin.pnode1;

                Assert(lhs);
                if (lhs->nop == knopDot)
                {
                    ParseNodePtr propertyNode = lhs->sxBin.pnode2;
                    if (propertyNode->nop == knopName)
                    {
                        propertyNode->sxPid.pid->PromoteAssignmentState();
                    }
                }
            }
            else if (nodeType & fnopUni)
            {
                // cases like obj.a++, ++obj.a
                ParseNodePtr lhs = pnode->sxUni.pnode1;
                if (lhs->nop == knopDot)
                {
                    ParseNodePtr propertyNode = lhs->sxBin.pnode2;
                    if (propertyNode->nop == knopName)
                    {
                        propertyNode->sxPid.pid->PromoteAssignmentState();
                    }
                }
            }
        }
    }

    return pnode;
}

template<bool buildAST>
void Parser::TrackAssignment(ParseNodePtr pnodeT, IdentToken* pToken, charcount_t ichMin, charcount_t ichLim)
{
    if (buildAST)
    {
        Assert(pnodeT != NULL);
        if (pnodeT->nop == knopName)
        {
            PidRefStack *ref = pnodeT->sxPid.pid->GetTopRef();
            Assert(ref);
            ref->TrackAssignment(pnodeT->ichMin, pnodeT->ichLim);
        }
    }
    else
    {
        Assert(pToken != NULL);
        if (BindDeferredPidRefs() && pToken->tk == tkID)
        {
            PidRefStack *ref = pToken->pid->GetTopRef();
            Assert(ref);
            ref->TrackAssignment(ichMin, ichLim);
        }
    }
}

void PidRefStack::TrackAssignment(charcount_t ichMin, charcount_t ichLim)
{
    if (this->isAsg)
    {
        if (this->GetIchMin() <= ichMin)
        {
            return;
        }
        Assert(this->GetIchLim() >= ichLim);
    }

    this->isAsg = true;
    this->span.Set(ichMin, ichLim);
}

void PnPid::SetSymRef(PidRefStack *ref)
{
    Assert(symRef == null);
    this->symRef = ref->GetSymRef();
}

Js::PropertyId PnPid::PropertyIdFromNameNode() const
{
    Js::PropertyId propertyId;
    Symbol *sym = this->sym;
    if (sym)
    {
        propertyId = sym->GetPosition();
    }
    else
    {
        propertyId = this->pid->GetPropertyId();
    }
    return propertyId;
}

PidRefStack* Parser::PushPidRef(IdentPtr pid)
{
    if (PHASE_ON1(Js::ParallelParsePhase))
    {
        // NOTE: the phase check is here to protect perf. See OSG 1020424.
        // In some LS AST-rewrite cases we lose a lot of perf searching the pid ref stack rather
        // than just pushing on the top. This hasn't shown up as a perf issue in non-LS benchmarks.
        return pid->FindOrAddPidRef(&m_nodeAllocator, GetCurrentBlock()->sxBlock.blockId);
    }

    Assert(GetCurrentBlock() != nullptr);
    AssertMsg(pid != nullptr, "pid should be created");
    PidRefStack *ref = pid->GetTopRef();
    if (!ref || (ref->GetScopeId() < GetCurrentBlock()->sxBlock.blockId
                // We could have the ref from the parameter scope. In that case we can skip creating a new one.
                && !(m_currentBlockInfo->pBlockInfoOuter->pnodeBlock->sxBlock.blockType == PnodeBlockType::Parameter
                    && m_currentBlockInfo->pBlockInfoOuter->pnodeBlock->sxBlock.blockId == ref->GetScopeId())))
    {
        ref = Anew(&m_nodeAllocator, PidRefStack);
        if (ref == nullptr)
        {
            Error(ERRnoMemory);
        }
        pid->PushPidRef(GetCurrentBlock()->sxBlock.blockId, ref);
    }

    return ref;
}

PidRefStack* Parser::FindOrAddPidRef(IdentPtr pid, int scopeId, int maxScopeId)
{
    PidRefStack *ref = pid->FindOrAddPidRef(&m_nodeAllocator, scopeId, maxScopeId);
    if (ref == NULL)
    {
        Error(ERRnoMemory);
    }
    return ref;
}

void Parser::RemovePrevPidRef(IdentPtr pid, PidRefStack *ref)
{
    PidRefStack *prevRef = pid->RemovePrevPidRef(ref);
    Assert(prevRef);
    if (prevRef->GetSym() == null)
    {
        AllocatorDelete(ArenaAllocator, &m_nodeAllocator, prevRef);
    }
}

void Parser::SetPidRefsInScopeDynamic(IdentPtr pid, int blockId)
{
    PidRefStack *ref = pid->GetTopRef();
    while (ref && ref->GetScopeId() >= blockId)
    {
        ref->SetDynamicBinding();
        ref = ref->prev;
    }
}

ParseNode* Parser::GetFunctionBlock()
{
    Assert(m_currentBlockInfo != NULL);
    return m_currentBlockInfo->pBlockInfoFunction->pnodeBlock;
}


ParseNode* Parser::GetCurrentBlock()
{
    return m_currentBlockInfo != NULL ? m_currentBlockInfo->pnodeBlock : NULL;
}

BlockInfoStack* Parser::GetCurrentBlockInfo()
{
    return m_currentBlockInfo;
}

BlockInfoStack* Parser::GetCurrentFunctionBlockInfo()
{
    return m_currentBlockInfo->pBlockInfoFunction;
}

/***************************************************************************
Parse a variable declaration.
'fAllowIn' indicates if the 'in' operator should be allowed in the initializing
expression ( it is not allowed in the context of the first expression in a  'for' loop).
***************************************************************************/
template<bool buildAST>
ParseNodePtr Parser::ParseVariableDeclaration(
    ERROR_RECOVERY_FORMAL_ tokens declarationType, charcount_t ichMin,
#if PARSENODE_EXTENSIONS
    charcount_t ichDeadRangeMin,
#endif
    BOOL fAllowIn,
    BOOL* pfForInOk,
    BOOL singleDefOnly,
    BOOL allowInit)
{
    if (m_scriptContext->GetConfig()->IsES6DestructuringEnabled() && (m_token.tk == tkLBrack || m_token.tk == tkLCurly))
    {
        return ParseDestructuredArrayLiteral<buildAST>(ERROR_RECOVERY_ACTUAL_(ers | ersSColon | ersVar) declarationType, true);
    }

    ParseNodePtr pnodeThis = nullptr;
    ParseNodePtr pnodeInit;
    ParseNodePtr pnodeList = nullptr;
    ParseNodePtr *lastNodeRef = nullptr;
    LPCOLESTR pNameHint = nullptr;
    ulong     nameHintLength = 0;
    Assert(declarationType == tkVAR || declarationType == tkCONST || declarationType == tkLET);

    for (;;)
    {
#if ERROR_RECOVERY
        tokens tkErrorSave = tkID;
#endif
        if (m_token.tk != tkID)
        {
            IdentifierExpectedError(m_token);
#if ERROR_RECOVERY
            SKIP(ERROR_RECOVERY_ACTUAL((ersAsg | ersComma | ersStmtStart) | ers));
            if (m_token.tk != tkID)
            {
                m_token.SetIdentifier(m_pidError);
                tkErrorSave = m_token.tk;
                m_token.tk = tkID;
            }
#endif
        }

        IdentPtr pid = m_token.GetIdentifier(m_phtbl);
        Assert(pid);
        pNameHint = pid->Psz();
        nameHintLength = pid->Cch();

        if (buildAST || BindDeferredPidRefs())
        {
            if (declarationType == tkVAR)
            {
                pnodeThis = CreateVarDeclNode(pid, STVariable);
            }
            else if (declarationType == tkCONST)
            {
                pnodeThis = CreateBlockScopedDeclNode(pid, knopConstDecl);
                CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(ConstCount, m_scriptContext);
            }
            else
            {
                pnodeThis = CreateBlockScopedDeclNode(pid, knopLetDecl);
                CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(LetCount, m_scriptContext);
            }
        }
        else if (!buildAST)
        {
            CheckPidIsValid(pid);
        }

#if PARSENODE_EXTENSIONS
        if (buildAST && LanguageServiceMode())
        {
            // store the min and lim of the identifier name
            m_languageServiceExtension->SetIdentMin(pnodeThis, m_pscan->IchMinTok());
            m_languageServiceExtension->SetIdentLim(pnodeThis, m_pscan->IchLimTok());
        }
#endif
        if (pid == wellKnownPropertyPids.arguments && m_currentNodeFunc)
        {
            // This var declaration may change the way an 'arguments' identifier in the function is resolved
            if (declarationType == tkVAR)
            {
                m_currentNodeFunc->grfpn |= PNodeFlags::fpnArguments_varDeclaration;
            }
            else
            {
                if (GetCurrentBlockInfo()->pnodeBlock->sxBlock.blockType == Function)
                {
                    // Only override arguments if we are at the function block level.
                    m_currentNodeFunc->grfpn |= PNodeFlags::fpnArguments_overriddenByDecl;
                }
            }
        }

        if (pnodeThis)
        {
            pnodeThis->ichMin = ichMin;
        }

#if PARSENODE_EXTENSIONS
        charcount_t ichLimLastToken = m_pscan->IchLimTok();
#endif

#if ERROR_RECOVERY
        if (tkErrorSave != tkID) // This could be false due to error-recovery
        {
            m_token.tk = tkErrorSave;
        }
        else
#endif
        {
            m_pscan->Scan();
        }

        if (m_token.tk == tkAsg)
        {
            if (!allowInit)
            {
                Error(ERRUnexpectedDefault);
            }
#if PARSENODE_EXTENSIONS
            if (buildAST && LanguageServiceMode())
            {
                // Add a completion dead range one char after the var keyword to allow for completion on var => variable
                m_languageServiceExtension->SetCompletionRange(ichDeadRangeMin + 1, m_pscan->IchMinTok(), LanguageServiceExtension::CompletionRangeMode::Others);
            }
#endif
            if (pfForInOk && (declarationType == tkLET || declarationType == tkCONST))
            {
                *pfForInOk = FALSE;
            }

            m_pscan->Scan();
            pnodeInit = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) koplCma, NULL, fAllowIn, FALSE, pNameHint, &nameHintLength);
            if (buildAST)
            {
                pnodeThis->sxVar.pnodeInit = pnodeInit;
                pnodeThis->ichLim = pnodeInit->ichLim;

                if (pnodeInit->nop == knopFncDecl)
                {
                    pnodeInit->sxFnc.hint = pNameHint;
                    pnodeInit->sxFnc.hintLength = nameHintLength;
                }
                else
                {
                    this->CheckArguments(pnodeInit);
                }
                pNameHint = NULL;
#if PARSENODE_EXTENSIONS
                if (LanguageServiceMode())
                {
                    // Initialize the next dead range
                    ichDeadRangeMin = pnodeInit->ichLim;
                    ichLimLastToken = pnodeInit->ichLim;
                }
#endif
            }

            //Track var a =, let a= , const a =
            // This is for FixedFields Constant Heuristics
            if (((pnodeThis)->sxVar).pnodeInit != nullptr)
            {
                pnodeThis->sxVar.sym->PromoteAssignmentState();
            }
        }
        else if (declarationType == tkCONST /*pnodeThis->nop == knopConstDecl*/ && !singleDefOnly)
        {
            Error(ERRUninitializedConst);
#if ERROR_RECOVERY
            if (buildAST && LanguageServiceMode())
            {
                pnodeInit = CreateIntNode(0);
                pnodeThis->sxVar.pnodeInit = pnodeInit;
                pnodeThis->ichLim = pnodeInit->ichLim;
            }
#endif
        }

        if (singleDefOnly)
        {
            return pnodeThis;
        }

        if (buildAST)
        {
            AddToNodeListEscapedUse(&pnodeList, &lastNodeRef, pnodeThis);
#if PARSENODE_EXTENSIONS
            if (LanguageServiceMode() && pnodeList && pnodeList->nop == knopList)
            {
                pnodeList->ichLim = pnodeThis->ichLim;          // update the ichLim of the list
                pnodeList->grfpn |= PNodeFlags::fpnDclList;  // this is a var, let, or const decl list, mark it
            }
#endif
        }

        if (m_token.tk != tkComma)
        {
#if PARSENODE_EXTENSIONS
            if (buildAST && LanguageServiceMode())
            {
                // Add a completion dead range one char after the comma to allow for completion on the comma start offset
                // e.g. var x = |,
                m_languageServiceExtension->SetCompletionRange(ichDeadRangeMin + 1, ichLimLastToken, LanguageServiceExtension::CompletionRangeMode::Others);
            }
#endif
            return pnodeList;
        }

        if (pfForInOk)
        {
            // don't allow "for (var a, b in c)"
            *pfForInOk = FALSE;
        }
        m_pscan->Scan();
        ichMin = m_pscan->IchMinTok();
    }
}


/***************************************************************************
Parse try-catch-finally statement
***************************************************************************/

// Eze try-catch-finally tree nests the try-catch within a try-finally.
// This matches the new runtime implementation.
template<bool buildAST>
ParseNodePtr Parser::ParseTryCatchFinally(ERROR_RECOVERY_FORMAL)
{
    this->m_tryCatchOrFinallyDepth++;

    ParseNodePtr pnodeT = ParseTry<buildAST>(ERROR_RECOVERY_ACTUAL(ersCatch | ers));
    ParseNodePtr pnodeTC = null;
    StmtNest stmt;
    bool hasCatch = false;

    if (tkCATCH == m_token.tk)
    {
        hasCatch = true;
        if (buildAST)
        {
            pnodeTC = CreateNodeWithScanner<knopTryCatch>();
            pnodeT->sxStmt.pnodeOuter = pnodeTC;
            pnodeTC->sxTryCatch.pnodeTry = pnodeT;
        }
        PushStmt<buildAST>(&stmt, pnodeTC, knopTryCatch, NULL, NULL);

        ParseNodePtr pnodeCatch = ParseCatch<buildAST>(ERROR_RECOVERY_ACTUAL(ersCatch | ers));
        if (buildAST)
        {
            pnodeTC->sxTryCatch.pnodeCatch = pnodeCatch;
        }
        PopStmt(&stmt);
    }
    if (tkFINALLY != m_token.tk)
    {
        if (!hasCatch)
        {
            Error(ERRnoCatch);
#if ERROR_RECOVERY
            // No skipping necessary since the source is well formed (from an error correction stand-point)

            // Return a try-finally with an empty finally block.
            ParseNodePtr pnodeFTF = CreateNodeT<knopTryFinally>(pnodeT->ichMin, pnodeT->ichLim);
            PushStmt<true>(&stmt, pnodeFTF, knopFinally, NULL, NULL);
            pnodeFTF->sxTryFinally.pnodeTry = pnodeT;
            pnodeT->sxStmt.pnodeOuter = pnodeFTF;
            ParseNodePtr pnodeFinally = CreateNodeT<knopFinally>(pnodeT->ichLim, pnodeT->ichLim);
            pnodeFinally->grfpn |= PNodeFlags::fpnSyntheticNode;
            pnodeFTF->sxTryFinally.pnodeFinally = pnodeFinally;
            ParseNodePtr block = CreateBlockNode(pnodeT->ichLim, pnodeT->ichLim);
            StmtNest stmtBlock;
            PushStmt<true>(&stmtBlock, block, knopBlock, NULL,NULL);
            block->sxBlock.pnodeStmt = CreateNodeT<knopEmpty>(pnodeT->ichLim, pnodeT->ichLim);
            block->grfpn |= PNodeFlags::fpnSyntheticNode;
            PopStmt(&stmtBlock);
            pnodeFinally->sxFinally.pnodeBody = block;
            PopStmt(&stmt);
            return pnodeFTF;
#endif
        }
        Assert(!buildAST || pnodeTC);
        return pnodeTC;
    }

    ParseNodePtr pnodeTF = null;
    if (buildAST)
    {
        pnodeTF = CreateNode(knopTryFinally);
    }
    PushStmt<buildAST>(&stmt, pnodeTF, knopTryFinally, NULL, NULL);
    ParseNodePtr pnodeFinally = ParseFinally<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
    if (buildAST)
    {
        if (!hasCatch)
        {
            pnodeTF->sxTryFinally.pnodeTry = pnodeT;
            pnodeT->sxStmt.pnodeOuter = pnodeTF;
        }
        else
        {
            pnodeTF->sxTryFinally.pnodeTry = CreateNode(knopTry);
#if LANGUAGE_SERVICE_ONLY
            pnodeTF->sxTryFinally.pnodeTry->ichMin = pnodeTC->ichMin;
            pnodeTF->sxTryFinally.pnodeTry->ichLim = pnodeTC->ichLim;
#endif
            pnodeTF->sxTryFinally.pnodeTry->sxStmt.pnodeOuter = pnodeTF;
            pnodeTF->sxTryFinally.pnodeTry->sxTry.pnodeBody = pnodeTC;
            pnodeTC->sxStmt.pnodeOuter = pnodeTF->sxTryFinally.pnodeTry;
        }
        pnodeTF->sxTryFinally.pnodeFinally = pnodeFinally;
    }
    PopStmt(&stmt);
    this->m_tryCatchOrFinallyDepth--;
    return pnodeTF;
}

template<bool buildAST>
ParseNodePtr Parser::ParseTry(ERROR_RECOVERY_FORMAL)
{
    ParseNodePtr pnode = NULL;
    StmtNest stmt;
    Assert(tkTRY == m_token.tk);
    if (buildAST)
    {
        pnode = CreateNode(knopTry);
    }
    m_pscan->Scan();
    if (tkLCurly != m_token.tk)
    {
        Error(ERRnoLcurly);
        SKIP(ERROR_RECOVERY_ACTUAL((ersCatch | ersStmtStart) | ers));
    }
#if PARSENODE_EXTENSIONS
    else if (buildAST && LanguageServiceMode())
    {
        m_languageServiceExtension->SetLCurly(pnode, m_pscan->IchMinTok());
    }
#endif

    PushStmt<buildAST>(&stmt, pnode, knopTry, NULL, NULL);
    ParseNodePtr pnodeBody = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
    if (buildAST)
    {
        pnode->sxTry.pnodeBody = pnodeBody;
        if (pnode->sxTry.pnodeBody)
            pnode->ichLim = pnode->sxTry.pnodeBody->ichLim;
    }
    PopStmt(&stmt);
    return pnode;
}

template<bool buildAST>
ParseNodePtr Parser::ParseFinally(ERROR_RECOVERY_FORMAL)
{
    ParseNodePtr pnode = NULL;
    StmtNest stmt;
    Assert(tkFINALLY == m_token.tk);
    if (buildAST)
    {
        pnode = CreateNode(knopFinally);
    }
    m_pscan->Scan();
    if (tkLCurly != m_token.tk)
    {
        Error(ERRnoLcurly);
        SKIP(ERROR_RECOVERY_ACTUAL(ersStmtStart | ers));
    }
#if PARSENODE_EXTENSIONS
    else if (buildAST && LanguageServiceMode())
    {
        m_languageServiceExtension->SetLCurly(pnode, m_pscan->IchMinTok());
    }
#endif

    PushStmt<buildAST>(&stmt, pnode, knopFinally, NULL, NULL);
    ParseNodePtr pnodeBody = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
    if (buildAST)
    {
        pnode->sxFinally.pnodeBody = pnodeBody;
        if (!pnode->sxFinally.pnodeBody)
            // Will only occur due to error correction.
            pnode->sxFinally.pnodeBody = CreateNodeWithScanner<knopEmpty>();
        else
            pnode->ichLim = pnode->sxFinally.pnodeBody->ichLim;
    }
    PopStmt(&stmt);

    return pnode;
}

template<bool buildAST>
ParseNodePtr Parser::ParseCatch(ERROR_RECOVERY_FORMAL)
{
    ParseNodePtr rootNode = NULL;
    ParseNodePtr* ppnode = &rootNode;
    ParseNodePtr *ppnodeExprScopeSave = NULL;
    ParseNodePtr pnode = NULL;
    ParseNodePtr pnodeCatchScope = NULL;
    StmtNest stmt;
    IdentPtr pidCatch = NULL;
    //while (tkCATCH == m_token.tk)
    if (tkCATCH == m_token.tk)
    {
        charcount_t ichMin = m_pscan->IchMinTok();
#if PARSENODE_EXTENSIONS
        charcount_t ichTokenLim = m_pscan->IchLimTok();
        charcount_t ichLParenMin = 0;
        charcount_t ichRParenMin = 0;
#endif
        m_pscan->Scan(); //catch
        ChkCurTok(tkLParen, ERRnoLparen _ERROR_RECOVERY_ACTUAL(ersExprStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLParenMin)); //catch(

#if ERROR_RECOVERY
        tokens tkErrorSave = tkID;
#endif
        if (tkID != m_token.tk)
        {
            IdentifierExpectedError(m_token);
#if ERROR_RECOVERY
            SKIP(ERROR_RECOVERY_ACTUAL(ersExprStart | ers));
            if (m_token.tk != tkID)
            {
                m_token.SetIdentifier(m_pidError);
                tkErrorSave = m_token.tk;
                m_token.tk = tkID;
            }
#endif
        }

        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopCatch>(ichMin);
            PushStmt<buildAST>(&stmt, pnode, knopCatch, NULL, NULL);
            *ppnode = pnode;
            ppnode = &pnode->sxCatch.pnodeNext;
            *ppnode = NULL;
        }

        if (IsStrictMode())
        {
            IdentPtr pid = m_token.GetIdentifier(m_phtbl);
            if (pid == wellKnownPropertyPids.eval)
            {
                Error(ERREvalUsage);
            }
            else if (pid == wellKnownPropertyPids.arguments)
            {
                Error(ERRArgsUsage);
            }
        }

        if (buildAST)
        {
            pnodeCatchScope = StartParseBlock<buildAST>(PnodeBlockType::Regular, ScopeType_Catch);

            pidCatch = m_token.GetIdentifier(m_phtbl);
            PidRefStack *ref = this->PushPidRef(pidCatch);

            if (!m_scriptContext->GetConfig()->IsBlockScopeEnabled())
            {
                // Strange case: the catch adds a scope for the catch object, but function declarations
                // are hoisted out of the catch, so references within a function declaration to "x" do
                // not bind to "catch(x)". Extra bookkeeping is required.
                CatchPidRefList *list = this->EnsureCatchPidRefList();
                CatchPidRef *catchPidRef = list->PrependNode(&m_nodeAllocator);
                catchPidRef->pid = pidCatch;
                catchPidRef->ref = ref;
            }

            ParseNodePtr pnodeParam = CreateNameNode(pidCatch);
            pnodeParam->sxPid.symRef = ref->GetSymRef();
            pnode->sxCatch.pnodeParam = pnodeParam;

            const wchar_t *name = reinterpret_cast<const wchar_t*>(pidCatch->Psz());
            int nameLength = pidCatch->Cch();
            SymbolName const symName(name, nameLength);
            Symbol *sym = Anew(&m_nodeAllocator, Symbol, symName, pnodeParam, STVariable);
            sym->SetPid(pidCatch);
            if (sym == null)
            {
                Error(ERRnoMemory);
            }
            Assert(ref->GetSym() == null);
            ref->SetSym(sym);

            Scope *scope = pnodeCatchScope->sxBlock.scope;
            scope->AddNewSymbol(sym);
            pnode->sxCatch.scope = scope;

            // Add this catch to the current scope list.

            if (m_ppnodeExprScope)
            {
                Assert(*m_ppnodeExprScope == NULL);
                *m_ppnodeExprScope = pnode;
                m_ppnodeExprScope = &pnode->sxCatch.pnodeNext;
            }
            else
            {
                Assert(m_ppnodeScope);
                Assert(*m_ppnodeScope == NULL);
                *m_ppnodeScope = pnode;
                m_ppnodeScope = &pnode->sxCatch.pnodeNext;
            }

            // Keep a list of function expressions (not declarations) at this scope.

            ppnodeExprScopeSave = m_ppnodeExprScope;
            m_ppnodeExprScope = &pnode->sxCatch.pnodeScopes;
            pnode->sxCatch.pnodeScopes = NULL;
        }
#if ERROR_RECOVERY
        if (tkErrorSave != tkID)
        {
            m_token.tk = tkErrorSave;
        }
        else
#endif
        {
            m_pscan->Scan(); //catch(id
        }

        charcount_t ichLim = m_pscan->IchLimTok();
        ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRParenMin)); //catch(id[:expr])
#if ERROR_RECOVERY
        ParseNodePtr insertedBlock = nullptr;
#endif
#if PARSENODE_EXTENSIONS
        if (buildAST && LanguageServiceMode())
        {
            m_languageServiceExtension->SetCompletionRange(ichTokenLim + 1, m_pscan->IchMinTok(), LanguageServiceExtension::CompletionRangeMode::Others);
            m_languageServiceExtension->SetLParen(pnode, ichLParenMin);
            m_languageServiceExtension->SetRParen(pnode, ichRParenMin);
        }
#endif
        if (tkLCurly != m_token.tk)
        {
            Error(ERRnoLcurly);
#if ERROR_RECOVERY
            SKIP(ERROR_RECOVERY_ACTUAL(ersStmtStart | ers));
            insertedBlock = StartParseBlock<buildAST>(PnodeBlockType::Regular, ScopeType_Block);
            insertedBlock->grfpn |= PNodeFlags::fpnSyntheticNode;
#endif
        }
#if PARSENODE_EXTENSIONS
        else if (buildAST && LanguageServiceMode())
        {
            Assert(m_languageServiceExtension != NULL);
            m_languageServiceExtension->SetLCurly(pnode, m_pscan->IchMinTok());
        }
#endif
        ParseNodePtr pnodeBody = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ers));  //catch(id[:expr]) {block}
        if (buildAST)
        {
            pnode->sxCatch.pnodeBody = pnodeBody;
            pnode->ichLim = ichLim;
#if ERROR_RECOVERY
            if (insertedBlock)
            {
                FinishParseBlock(insertedBlock);
                insertedBlock->sxBlock.pnodeStmt = pnode->sxCatch.pnodeBody;
                pnode->sxCatch.pnodeBody = insertedBlock;
                insertedBlock->ichMin = pnode->sxCatch.pnodeBody->ichMin;
                insertedBlock->ichLim = pnode->sxCatch.pnodeBody->ichLim;
            }
#endif
        }

        if (pnodeCatchScope)
        {
            FinishParseBlock(pnodeCatchScope);
        }

        if (buildAST)
        {
            PopStmt(&stmt);

            // Restore the lists of function expression scopes.

            AssertMem(m_ppnodeExprScope);
            Assert(*m_ppnodeExprScope == NULL);
            m_ppnodeExprScope = ppnodeExprScopeSave;

            if (!m_scriptContext->GetConfig()->IsBlockScopeEnabled())
            {
                // Remove the catch object from the list.
                CatchPidRefList *list = this->GetCatchPidRefList();
                Assert(list);
                Assert(!list->Empty());
                Assert(list->Head().pid == pidCatch);
                list->RemoveHead(&m_nodeAllocator);
            }
        }
    }
    return rootNode;
}

template<bool buildAST>
ParseNodePtr Parser::ParseCase(ERROR_RECOVERY_FORMAL_ ParseNodePtr *ppnodeBody)
{
    ParseNodePtr pnodeT = nullptr;

    charcount_t ichMinT = m_pscan->IchMinTok();
    m_pscan->Scan();
    ParseNodePtr pnodeExpr = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ersColon | ers));
    charcount_t ichLim = m_pscan->IchLimTok();

    ChkCurTok(tkColon, ERRnoColon _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers));

    if (buildAST)
    {
        pnodeT = CreateNodeWithScanner<knopCase>(ichMinT);
        pnodeT->sxCase.pnodeExpr = pnodeExpr;
        pnodeT->ichLim = ichLim;
    }
    ParseStmtList<buildAST>(ERROR_RECOVERY_ACTUAL_(ersRCurly | ers) ppnodeBody);
#if PARSENODE_EXTENSIONS
    if (buildAST && LanguageServiceMode())
    {
        // Extend the case statement to include EOF token or Rcurly of parent switch statement
        if (m_token.tk == tkEOF)
            pnodeT->ichLim = m_pscan->IchLimTok();
        else if (m_token.tk == tkRCurly)
            pnodeT->ichLim = m_pscan->IchMinTok();
    }
#endif

    return pnodeT;
}

/***************************************************************************
Parse a single statement. Digest a trailing semicolon.
***************************************************************************/
template<bool buildAST>
ParseNodePtr Parser::ParseStatement(ERROR_RECOVERY_FORMAL_ bool isSourceElement)
{
    ParseNodePtr *ppnodeT;
    ParseNodePtr pnodeT;
    ParseNodePtr pnode = NULL;
    LabelId* pLabelIdList = NULL;
    charcount_t ichMin;
    StmtNest stmt;
    StmtNest *pstmt;
    BOOL fForInOrOfOkay;
    IdentPtr pid;
    uint fnop;
    ParseNodePtr pnodeLabel = NULL;
    bool expressionStmt = false;
    tokens tok;
#if EXCEPTION_RECOVERY
    ParseNodePtr pParentTryCatch = NULL;
    ParseNodePtr pTryBlock = NULL;
    ParseNodePtr pTry = NULL;
    ParseNodePtr pParentTryCatchBlock = NULL;

    StmtNest stmtTryCatchBlock;
    StmtNest stmtTryCatch;
    StmtNest stmtTry;
    StmtNest stmtTryBlock;
#endif

    // Reset the current expression depth so that it doesn't carry over across
    // function declarations, etc.
    uint oldExprDepth = this->m_exprDepth;
    this->m_exprDepth = 0;

#if PARSENODE_EXTENSIONS
    charcount_t ichLParen = 0;
    charcount_t ichRParen = 0;
    charcount_t ichLCurly = 0;
    charcount_t ichRCurly = 0;
    charcount_t ichWhileMin = 0;
    charcount_t ichDeadRangeMin;
#endif

    if (buildAST)
    {
#if EXCEPTION_RECOVERY
        if(Js::Configuration::Global.flags.SwallowExceptions)
        {
            // If we're swallowing exceptions, surround this statement with a try/catch block:
            //
            //   Before: x.y = 3;
            //   After:  try { x.y = 3; } catch(__ehobj) { }
            //
            // This is done to force the runtime to recover from exceptions at the most granular
            // possible point.  Recovering from EH dramatically improves coverage of testing via
            // fault injection.


            // create and push the try-catch node
            pParentTryCatchBlock = CreateBlockNode();
            PushStmt<buildAST>(&stmtTryCatchBlock, pParentTryCatchBlock, knopBlock, NULL, NULL);
            pParentTryCatch = CreateNodeWithScanner<knopTryCatch>();
            PushStmt<buildAST>(&stmtTryCatch, pParentTryCatch, knopTryCatch, NULL, NULL);

            // create and push a try node
            pTry = CreateNodeWithScanner<knopTry>();
            PushStmt<buildAST>(&stmtTry, pTry, knopTry, NULL, NULL);
            pTryBlock = CreateBlockNode();
            PushStmt<buildAST>(&stmtTryBlock, pTryBlock, knopBlock, NULL, NULL);
            // these nodes will be closed after the statement is parsed.
        }
#endif // EXCEPTION_RECOVERY
    }

    EnsureStackAvailable();

LRestart:
    tok = m_token.tk;
    switch (tok)
    {
    case tkEOF:
        if (buildAST)
        {
            pnode = NULL;
#if PARSENODE_EXTENSIONS
            if (LanguageServiceMode() && m_pstmtCur && m_pstmtCur->pnodeStmt)
            {
                // Extend the parent statement to include the EOF token
                m_pstmtCur->pnodeStmt->ichLim = m_pscan->IchLimTok();
            }
#endif
        }
        break;

    case tkFUNCTION:
    {
#if PARSENODE_EXTENSIONS
        charcount_t ichtkFunctionMin = m_pscan->IchMinTok();
#endif
        if (m_grfscr & fscrDeferredFncExpression)
        {
            // The top-level deferred function body was defined by a function expression whose parsing was deferred. We are now
            // parsing it, so unset the flag so that any nested functions are parsed normally. This flag is only applicable the
            // first time we see it.
            m_grfscr &= ~fscrDeferredFncExpression;
            pnode = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) fFncNoFlgs, nullptr, isSourceElement);
        }
        else
        {
            pnode = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) fFncDeclaration, nullptr, isSourceElement);
        }
#if PARSENODE_EXTENSIONS
        // Keep ichtkFunctionMin if needed
        if (buildAST && LanguageServiceMode())
        {
            m_languageServiceExtension->SetTkFunctionMin(pnode, ichtkFunctionMin);
        }
#endif
        break;
    }

    case tkCLASS:
        if (m_scriptContext->GetConfig()->IsES6ClassAndExtendsEnabled())
        {
            pnode = ParseClassDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) TRUE, nullptr, nullptr);
        }
        else
        {
            goto LDefaultToken;
        }
        break;

    case tkID:
        if (m_token.GetIdentifier(m_phtbl) == wellKnownPropertyPids.let && m_scriptContext->GetConfig()->IsLetAndConstEnabled())
        {
            // We see "let" at the start of a statement. This could either be a declaration or an identifier
            // reference. The next token determines which.
            RestorePoint parsedLet;
            m_pscan->Capture(&parsedLet);
            ichMin = m_pscan->IchMinTok();
#if PARSENODE_EXTENSIONS
            ichDeadRangeMin = m_pscan->IchLimTok();
#endif
            m_pscan->Scan();
            if (this->NextTokenConfirmsLetDecl())
            {
                pnode = ParseVariableDeclaration<buildAST>(ERROR_RECOVERY_ACTUAL_(ers | ersSColon | ersVar) tkLET, ichMin
#if PARSENODE_EXTENSIONS
                                                           , ichDeadRangeMin
#endif
                    );
                goto LNeedTerminator;
            }
            m_pscan->SeekTo(parsedLet);
        }
        goto LDefaultToken;

    case tkCONST:
    case tkLET:
        if (m_scriptContext->GetConfig()->IsLetAndConstEnabled())
        {
            ichMin = m_pscan->IchMinTok();
#if PARSENODE_EXTENSIONS
            ichDeadRangeMin = m_pscan->IchLimTok();
#endif
            m_pscan->Scan();
            pnode = ParseVariableDeclaration<buildAST>(ERROR_RECOVERY_ACTUAL_(ers | ersSColon | ersVar) tok, ichMin
#if PARSENODE_EXTENSIONS
                                                       , ichDeadRangeMin
#endif
                );
            goto LNeedTerminator;
        }
        else
        {
            goto LDefaultToken;
        }

    case tkVAR:
        ichMin = m_pscan->IchMinTok();
#if PARSENODE_EXTENSIONS
        ichDeadRangeMin = m_pscan->IchLimTok();
#endif
        m_pscan->Scan();
        pnode = ParseVariableDeclaration<buildAST>(ERROR_RECOVERY_ACTUAL_(ers | ersSColon | ersVar) tok, ichMin
#if PARSENODE_EXTENSIONS
                                                   , ichDeadRangeMin
#endif
                );
        goto LNeedTerminator;

    case tkFOR:
    {
        ParseNodePtr pnodeBlock = NULL;
        ParseNodePtr *ppnodeScopeSave = NULL;
        ParseNodePtr *ppnodeExprScopeSave = NULL;

        ichMin = m_pscan->IchMinTok();
        ChkNxtTok(tkLParen, ERRnoLparen _ERROR_RECOVERY_ACTUAL((ersExprStart | ersVar) | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLParen));
        if (buildAST || BindDeferredPidRefs())
        {
            pnodeBlock = StartParseBlock<buildAST>(PnodeBlockType::Regular, ScopeType_Block);
            if (buildAST)
            {
                PushFuncBlockScope(pnodeBlock, &ppnodeScopeSave, &ppnodeExprScopeSave);
            }
        }

        fForInOrOfOkay = TRUE;
        tok = m_token.tk;
        switch (tok)
        {
        case tkID:
            if (m_token.GetIdentifier(m_phtbl) == wellKnownPropertyPids.let && m_scriptContext->GetConfig()->IsLetAndConstEnabled())
            {
                // We see "let" in the init part of a for loop. This could either be a declaration or an identifier
                // reference. The next token determines which.
                RestorePoint parsedLet;
                m_pscan->Capture(&parsedLet);
                auto ichMin = m_pscan->IchMinTok();
#if PARSENODE_EXTENSIONS
                ichDeadRangeMin = m_pscan->IchLimTok();
#endif
                m_pscan->Scan();
                if (this->NextTokenConfirmsLetDecl() && m_token.tk != tkIN)
                {
                    if (m_scriptContext->GetConfig()->IsES6DestructuringEnabled() && (m_token.tk == tkLBrack || m_token.tk == tkLCurly))
                    {
                        pnodeT = ParseDestructuredArrayLiteral<buildAST>(ERROR_RECOVERY_ACTUAL_(ers | ersSColon | ersVar) tkLET, true);
                        break;
                    }
                    else
                    {
                        pnodeT = ParseVariableDeclaration<buildAST>(ERROR_RECOVERY_ACTUAL_((ersSColon | ersIn) | ers) tkLET, ichMin
#if PARSENODE_EXTENSIONS
                                                                    , ichDeadRangeMin
#endif
                                                                    , /*fAllowIn = */FALSE
                                                                    , /*pfForInOk = */&fForInOrOfOkay);
                        break;
                    }
                }
                m_pscan->SeekTo(parsedLet);
            }
            goto LDefaultTokenFor;
        case tkLET:
        case tkCONST:
            if (!m_scriptContext->GetConfig()->IsLetAndConstEnabled())
            {
                goto LDefaultTokenFor;
            }
        case tkVAR:
            {
                auto ichMin = m_pscan->IchMinTok();
#if PARSENODE_EXTENSIONS
                ichDeadRangeMin = m_pscan->IchLimTok();
#endif
                m_pscan->Scan();
                if (m_scriptContext->GetConfig()->IsES6DestructuringEnabled() && (m_token.tk == tkLBrack || m_token.tk == tkLCurly))
                {
                    pnodeT = ParseDestructuredArrayLiteral<buildAST>(ERROR_RECOVERY_ACTUAL_(ers | ersSColon | ersVar) tkVAR, true);
                    break;
                }
                else
                {
                    pnodeT = ParseVariableDeclaration<buildAST>(ERROR_RECOVERY_ACTUAL_((ersSColon | ersIn) | ers) tok, ichMin
#if PARSENODE_EXTENSIONS
                                                                , ichDeadRangeMin
#endif
                                                                , /*fAllowIn = */FALSE
                                                                , /*pfForInOk = */&fForInOrOfOkay);
                }
            }
            break;
        case tkSColon:
            pnodeT = NULL;
            fForInOrOfOkay = FALSE;
            break;
        default:
LDefaultTokenFor:
            pnodeT = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_((ersSColon | ersIn) | ers) koplNo, &fForInOrOfOkay, /*fAllowIn = */FALSE);
            if (buildAST)
            {
                pnodeT->isUsed=false;
            }
            break;
        }

        if (m_token.tk == tkIN || (m_scriptContext->GetConfig()->IsES6IteratorsEnabled() && m_token.tk == tkID && m_token.GetIdentifier(m_phtbl) == wellKnownPropertyPids.of))
        {
            bool isForOf = (m_token.tk != tkIN);
            Assert(!isForOf || (m_token.tk == tkID && m_token.GetIdentifier(m_phtbl) == wellKnownPropertyPids.of));

            if ((buildAST && NULL == pnodeT) || !fForInOrOfOkay)
            {
                Error(ERRsyntax);
#if ERROR_RECOVERY
                // No skipping necessary since this is a semantic, not structural, error
                if (buildAST && NULL == pnodeT)
                {
                    pnodeT = CreateNameNode(m_pidError);
                }
#endif
            }

            m_pscan->Scan();
            ParseNodePtr pnodeObj = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ersRParen | ers));
            charcount_t ichLim = m_pscan->IchLimTok();
            ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRParen));

            if (buildAST)
            {
                if (isForOf)
                {
                    pnode = CreateNodeWithScanner<knopForOf>(ichMin);
                }
                else
                {
                    pnode = CreateNodeWithScanner<knopForIn>(ichMin);
                }
                pnode->sxForInOrForOf.pnodeBlock = pnodeBlock;
                pnode->sxForInOrForOf.pnodeLval = pnodeT;
                pnode->sxForInOrForOf.pnodeObj = pnodeObj;
                pnode->ichLim = ichLim;
            }
            PushStmt<buildAST>(&stmt, pnode, isForOf ? knopForOf : knopForIn, pnodeLabel, pLabelIdList);
            ParseNodePtr pnodeBody = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ers));

            if (buildAST)
            {
                pnode->sxForInOrForOf.pnodeBody = pnodeBody;
            }
            PopStmt(&stmt);
        }
        else
        {
#if ERROR_RECOVERY
            bool scanBeforeRParen = true;
#endif
            ChkCurTok(tkSColon, ERRnoSemic _ERROR_RECOVERY_ACTUAL(ers));
            ParseNodePtr pnodeCond = NULL;
            if (m_token.tk != tkSColon)
            {
                pnodeCond = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL((ersSColon | ersRParen) | ers));
                if (m_token.tk != tkSColon)
                {
                    Error(ERRnoSemic);
                    SKIP(ERROR_RECOVERY_ACTUAL(ersRParen | ers));
#if ERROR_RECOVERY
                    scanBeforeRParen = false;
#endif
                }
            }

            tokens tk;
#if ERROR_RECOVERY
            if (scanBeforeRParen)
#endif
                tk = m_pscan->Scan();
#if ERROR_RECOVERY
            else tk = m_token.tk;
#endif

            ParseNodePtr pnodeIncr = NULL;
            if (tk != tkRParen)
            {
                pnodeIncr = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL((ersSColon | ersRParen) | ers));
                if(pnodeIncr)
                {
                    pnodeIncr->isUsed = false;
                }
            }

            charcount_t ichLim = m_pscan->IchLimTok();

            ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRParen));

            if (buildAST)
            {
                pnode = CreateNodeWithScanner<knopFor>(ichMin);
                pnode->sxFor.pnodeBlock = pnodeBlock;
                pnode->sxFor.pnodeInverted=NULL;
                pnode->sxFor.pnodeInit = pnodeT;
                pnode->sxFor.pnodeCond = pnodeCond;
                pnode->sxFor.pnodeIncr = pnodeIncr;
                pnode->ichLim = ichLim;
            }
            PushStmt<buildAST>(&stmt, pnode, knopFor, pnodeLabel, pLabelIdList);
            ParseNodePtr pnodeBody = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
            if (buildAST)
            {
                pnode->sxFor.pnodeBody = pnodeBody;
            }
            PopStmt(&stmt);
        }

        if (buildAST)
        {
            PopFuncBlockScope(ppnodeScopeSave, ppnodeExprScopeSave);
            FinishParseBlock(pnodeBlock);

#if PARSENODE_EXTENSIONS
            if (LanguageServiceMode())
            {
                m_languageServiceExtension->SetLParen(pnode, ichLParen);
                m_languageServiceExtension->SetRParen(pnode, ichRParen);
            }
#endif
        }
        else if (BindDeferredPidRefs())
        {
            FinishParseBlock(pnodeBlock);
        }
        break;
    }

    case tkSWITCH:
    {
        BOOL fSeenDefault = FALSE;
        StmtNest stmtBlock;
        ParseNodePtr pnodeBlock = NULL;
        ParseNodePtr *ppnodeScopeSave = NULL;
        ParseNodePtr *ppnodeExprScopeSave = NULL;

        ichMin = m_pscan->IchMinTok();
        ChkNxtTok(tkLParen, ERRnoLparen _ERROR_RECOVERY_ACTUAL(ersExprStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLParen));
        ParseNodePtr pnodeVal = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ersRParen | ers));
        charcount_t ichLim = m_pscan->IchLimTok();

        ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ersLCurly | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRParen));
        ChkCurTok(tkLCurly, ERRnoLcurly _ERROR_RECOVERY_ACTUAL(ersSCase | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLCurly));

        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopSwitch>(ichMin);
        }
        PushStmt<buildAST>(&stmt, pnode, knopSwitch, pnodeLabel, pLabelIdList);
        if (buildAST || BindDeferredPidRefs())
        {
            pnodeBlock = StartParseBlock<buildAST>(PnodeBlockType::Regular, ScopeType_Block, null, pLabelIdList);
        }
        else
        {
            PushStmt<buildAST>(&stmtBlock, NULL, knopBlock, NULL, pLabelIdList);
        }

        if (buildAST)
        {
            pnode->sxSwitch.pnodeVal = pnodeVal;
            pnode->sxSwitch.pnodeBlock = pnodeBlock;
            pnode->ichLim = ichLim;
            PushFuncBlockScope(pnode->sxSwitch.pnodeBlock, &ppnodeScopeSave, &ppnodeExprScopeSave);

            pnode->sxSwitch.pnodeDefault = NULL;
            ppnodeT = &pnode->sxSwitch.pnodeCases;
        }

#if PARSENODE_EXTENSIONS
        ParseNodePtr pnodeLastCase = NULL;
#endif
        for (;;)
        {
#if PARSENODE_EXTENSIONS
            if (LanguageServiceMode() && pnodeLastCase && pnodeLastCase->nop == knopCase)
            {
                // Extend the previous case statement to the beginning of the current one (-1 to avoid overlapping in SeekToOffset queries)
                if (m_token.tk == tkEOF)
                    pnodeLastCase->ichLim = m_pscan->IchLimTok();
                else if (m_token.tk == tkRCurly)
                    pnodeLastCase->ichLim = m_pscan->IchMinTok();
                else
                    pnodeLastCase->ichLim = m_pscan->IchMinTok() - 1;
            }
#endif

            ParseNodePtr pnodeBody = NULL;
            switch (m_token.tk)
            {
            default:
                goto LEndSwitch;
            case tkCASE:
            {
                pnodeT = this->ParseCase<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) &pnodeBody);
                break;
            }
            case tkDEFAULT:
                if (fSeenDefault)
                {
                    Error(ERRdupDefault);
                    // No recovery necessary since this is a semantic, not structural, error
#if ERROR_RECOVERY
                    if (buildAST)
                    {
                        // The original default is no-longer treated as the default and is, therefore, assumed to have an expression. Fill one in if one isn't there already.
                        if (!pnode->sxSwitch.pnodeDefault->sxCase.pnodeExpr)
                            pnode->sxSwitch.pnodeDefault->sxCase.pnodeExpr = CreateNameNode(m_pidError, pnode->sxSwitch.pnodeDefault->ichLim, pnode->sxSwitch.pnodeDefault->ichLim);
                    }
#endif
                }
                fSeenDefault = TRUE;
                charcount_t ichMinT = m_pscan->IchMinTok();
                m_pscan->Scan();
                charcount_t ichLim = m_pscan->IchLimTok();
                ChkCurTok(tkColon, ERRnoColon _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers));
                if (buildAST)
                {
                    pnodeT = CreateNodeWithScanner<knopCase>(ichMinT);
                    pnode->sxSwitch.pnodeDefault = pnodeT;
                    pnodeT->ichLim = ichLim;
                    pnodeT->sxCase.pnodeExpr = NULL;
                }
                ParseStmtList<buildAST>(ERROR_RECOVERY_ACTUAL_(ersRCurly | ers) &pnodeBody);
#if PARSENODE_EXTENSIONS
                if (buildAST && LanguageServiceMode())
                {
                    // Extend the default statement to include EOF token or Rcurly of parent switch statement
                    if (m_token.tk == tkEOF)
                        pnodeT->ichLim = m_pscan->IchLimTok();
                    else if (m_token.tk == tkRCurly)
                        pnodeT->ichLim = m_pscan->IchMinTok();
                }
#endif
                break;
            }
            if (buildAST)
            {
#if PARSENODE_EXTENSIONS
                pnodeLastCase = pnodeT;
#endif
                if (pnodeBody)
                {
                    // Create a block node to contain the statement list for this case.
                    // This helps us insert byte code to return the right value from
                    // global/eval code.
                    pnodeT->sxCase.pnodeBody = CreateBlockNode(pnodeT->ichMin, pnodeT->ichLim);
                    pnodeT->sxCase.pnodeBody->grfpn |= PNodeFlags::fpnSyntheticNode; // block is not a user specifier block
                    pnodeT->sxCase.pnodeBody->sxBlock.pnodeStmt = pnodeBody;
                }
                else
                {
                    pnodeT->sxCase.pnodeBody = NULL;
                }
                *ppnodeT = pnodeT;
                ppnodeT = &pnodeT->sxCase.pnodeNext;
            }
        }
LEndSwitch:
        ChkCurTok(tkRCurly, ERRnoRcurly _ERROR_RECOVERY_ACTUAL(ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRCurly));
        if (buildAST)
        {
            *ppnodeT = NULL;
            PopFuncBlockScope(ppnodeScopeSave, ppnodeExprScopeSave);
            FinishParseBlock(pnode->sxSwitch.pnodeBlock);
        }
        else
        {
            if (BindDeferredPidRefs())
            {
                FinishParseBlock(pnodeBlock);
            }
            else
            {
                PopStmt(&stmtBlock);
            }
        }
        PopStmt(&stmt);

#if PARSENODE_EXTENSIONS
        // Store LParen, RParen, LCurly, RCurly for the language service
        if (buildAST && LanguageServiceMode())
        {
            m_languageServiceExtension->SetLParen(pnode, ichLParen);
            m_languageServiceExtension->SetRParen(pnode, ichRParen);
            m_languageServiceExtension->SetLCurly(pnode, ichLCurly);
            m_languageServiceExtension->SetRCurly(pnode, ichRCurly);

            // store the node orignal lim for GetNodeSpan
            m_languageServiceExtension->SetSwitchLim(pnode, pnode->ichLim);

            // Set the node lim to RCurly position.
            if (pnode->ichLim < (ichRCurly + 1))
            {
                pnode->ichLim = (ichRCurly + 1);
            }
            else if (m_token.tk == tkEOF)
            {
                // If in a switch statement with no closing right curly and EOF is reached, extend it.
                pnode->ichLim = m_pscan->IchLimTok();
            }
            else if (ichRCurly <= 0)
            {
                // If switch is missing the closing curly, extend the swtich to the next node (-1 to avoid overlapping in SeekToOffset queries)
                pnode->ichLim = m_pscan->IchMinTok() - 1;
            }
        }
#endif
        break;
    }

    case tkWHILE:
    {
        ichMin = m_pscan->IchMinTok();
        ChkNxtTok(tkLParen, ERRnoLparen _ERROR_RECOVERY_ACTUAL(ersExprStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLParen));
        ParseNodePtr pnodeCond = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ersRParen | ers));
        charcount_t ichLim = m_pscan->IchLimTok();
        ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRParen));

        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopWhile>(ichMin);
            pnode->sxWhile.pnodeCond = pnodeCond;
            pnode->ichLim = ichLim;
        }
        PushStmt<buildAST>(&stmt, pnode, knopWhile, pnodeLabel, pLabelIdList);
        ParseNodePtr pnodeBody = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
        PopStmt(&stmt);

        if (buildAST)
        {
#if PARSENODE_EXTENSIONS
            // Store LParen, RParen for the language service
            if (LanguageServiceMode())
            {
                m_languageServiceExtension->SetLParen(pnode, ichLParen);
                m_languageServiceExtension->SetRParen(pnode, ichRParen);
            }
#endif

            pnode->sxWhile.pnodeBody = pnodeBody;
        }
        break;
    }

    case tkDO:
    {
        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopDoWhile>();
        }
        PushStmt<buildAST>(&stmt, pnode, knopDoWhile, pnodeLabel, pLabelIdList);
        m_pscan->Scan();
        ParseNodePtr pnodeBody = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ersWhile | ers));
        PopStmt(&stmt);
        // if not in the language service mode, set the begining to the while for debugging
        charcount_t ichMinT = m_pscan->IchMinTok();

#if ERROR_RECOVERY
        bool whileInExpectedLocation = m_token.tk == tkWHILE;
#endif
        ChkCurTok(tkWHILE, ERRnoWhile _ERROR_RECOVERY_ACTUAL(ersLParen | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichWhileMin));
        ChkCurTok(tkLParen, ERRnoLparen _ERROR_RECOVERY_ACTUAL(ersExprStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLParen));

        ParseNodePtr pnodeCond = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ersRParen | ers));
        charcount_t ichLim = m_pscan->IchLimTok();
        ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRParen));

        if (buildAST)
        {
#if PARSENODE_EXTENSIONS
            // Store LParen, RParen for the language service
            if (LanguageServiceMode())
            {
                m_languageServiceExtension->SetLParen(pnode, ichLParen);
                m_languageServiceExtension->SetRParen(pnode, ichRParen);
                m_languageServiceExtension->SetWhileMin(pnode, ichWhileMin);
            }
#endif
            pnode->sxWhile.pnodeBody = pnodeBody;
            pnode->sxWhile.pnodeCond = pnodeCond;
            pnode->ichLim = ichLim;

#if ERROR_RECOVERY
            if (!whileInExpectedLocation)
            {
                // If the while is not found then try shrink node to the minimum of what it contains instead to the current token
                // This is done because '}' can trigger formatting and formatting works better if "do { ... }" is treated as if
                // it was a complete statement.
                if (pnode->sxWhile.pnodeCond && !(pnode->sxWhile.pnodeCond->nop == knopName && pnode->sxWhile.pnodeCond->sxPid.pid == m_pidError))
                {
                    // If the expression exists and it is not an error symbol (inserted by error correction) then take the expression limit.
                    pnode->ichLim = pnode->sxWhile.pnodeCond->ichLim;
                }
                else if (pnode->sxWhile.pnodeBody)
                {
                    // Otherwise, take the body's limit as the limit.
                    pnode->ichLim = pnode->sxWhile.pnodeBody->ichLim;

                    // Move the error symbol, if there is one to the end of this node. This preserves the constraint that parent contain their children.
                    if (pnode->sxWhile.pnodeCond)
                    {
                        // This is checked above but asserted here to ensure modifying the above doesn't break this code.
                        Assert(pnode->sxWhile.pnodeCond->nop == knopName && pnode->sxWhile.pnodeCond->sxPid.pid == m_pidError);

                        pnode->sxWhile.pnodeCond->ichMin = pnode->ichLim;
                        pnode->sxWhile.pnodeCond->ichLim = pnode->ichLim;
                    }

                }
            }
#endif
#if PARSENODE_EXTENSIONS
            if (!LanguageServiceMode())
            {
#endif
                pnode->ichMin = ichMinT;
#if PARSENODE_EXTENSIONS
            }
#endif
        }

        // REVIEW: Allow do...while statements to be embedded in other compound statements like if..else, or do..while?
        //      goto LNeedTerminator;

        // For now just eat the trailing semicolon if present.
        if (m_token.tk == tkSColon)
        {
            if (pnode)
            {
                pnode->grfpn |= PNodeFlags::fpnExplicitSimicolon;
            }
            m_pscan->Scan();
        }
        else if (pnode)
        {
            pnode->grfpn |= PNodeFlags::fpnAutomaticSimicolon;
        }

        break;
    }

    case tkIF:
    {
        ichMin = m_pscan->IchMinTok();
        ChkNxtTok(tkLParen, ERRnoLparen _ERROR_RECOVERY_ACTUAL(ersExprStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLParen));
        ParseNodePtr pnodeCond = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ersLParen | ers));
        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopIf>(ichMin);
            pnode->ichLim = m_pscan->IchLimTok();
            pnode->sxIf.pnodeCond = pnodeCond;
        }
        ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRParen));

#if PARSENODE_EXTENSIONS
        // Store LParen, RParen for the language service
        if (buildAST && LanguageServiceMode())
        {
            m_languageServiceExtension->SetLParen(pnode, ichLParen);
            m_languageServiceExtension->SetRParen(pnode, ichRParen);
        }
#endif
        PushStmt<buildAST>(&stmt, pnode, knopIf, pnodeLabel, pLabelIdList);
        ParseNodePtr pnodeTrue = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ersElse | ers));
        ParseNodePtr pnodeFalse = NULL;
        if (m_token.tk == tkELSE)
        {
            m_pscan->Scan();
            pnodeFalse = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
        }
        if (buildAST)
        {
            pnode->sxIf.pnodeTrue = pnodeTrue;
            pnode->sxIf.pnodeFalse = pnodeFalse;
        }
        PopStmt(&stmt);
        break;
    }

    case tkTRY:
    {
        if (buildAST)
        {
            pnode = CreateBlockNode();
            pnode->grfpn |= PNodeFlags::fpnSyntheticNode; // block is not a user specifier block
        }
        PushStmt<buildAST>(&stmt, pnode, knopBlock, pnodeLabel, pLabelIdList);
        ParseNodePtr pnodeStmt = ParseTryCatchFinally<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
        if (buildAST)
        {
            pnode->sxBlock.pnodeStmt = pnodeStmt;
        }
        PopStmt(&stmt);
        break;
    }

    case tkWITH:
    {
#if ECMACP
        if (m_fECMACP)
            Error(ERRWithNotInCP);
#endif // ECMACP
        if ( IsStrictMode() )
        {
            Error(ERRES5NoWith);
        }
        if (m_currentNodeFunc)
        {
            GetCurrentFunctionNode()->sxFnc.SetHasWithStmt(); // Used by DeferNested
        }

        ichMin = m_pscan->IchMinTok();
        ChkNxtTok(tkLParen, ERRnoLparen _ERROR_RECOVERY_ACTUAL(ersExprStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichLParen));
        ParseNodePtr pnodeObj = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ersRParen | ers));
        if (!buildAST)
        {
            m_scopeCountNoAst++;
        }
        charcount_t ichLim = m_pscan->IchLimTok();
        ChkCurTok(tkRParen, ERRnoRparen _ERROR_RECOVERY_ACTUAL(ersStmtStart | ers) _PARSENODE_EXTENSIONS_ACTUAL(ichRParen));

        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopWith>(ichMin);
        }
        PushStmt<buildAST>(&stmt, pnode, knopWith, pnodeLabel, pLabelIdList);

        ParseNodePtr *ppnodeExprScopeSave = NULL;
        if (buildAST)
        {
            pnode->sxWith.pnodeObj = pnodeObj;
            this->CheckArguments(pnode->sxWith.pnodeObj);

            if (m_ppnodeExprScope)
            {
                Assert(*m_ppnodeExprScope == NULL);
                *m_ppnodeExprScope = pnode;
                m_ppnodeExprScope = &pnode->sxWith.pnodeNext;
            }
            else
            {
                Assert(m_ppnodeScope);
                Assert(*m_ppnodeScope == NULL);
                *m_ppnodeScope = pnode;
                m_ppnodeScope = &pnode->sxWith.pnodeNext;
            }
            pnode->sxWith.pnodeNext = NULL;
            pnode->sxWith.scope = NULL;

            ppnodeExprScopeSave = m_ppnodeExprScope;
            m_ppnodeExprScope = &pnode->sxWith.pnodeScopes;
            pnode->sxWith.pnodeScopes = NULL;

            pnode->ichLim = ichLim;

#if PARSENODE_EXTENSIONS
            // Store LParen, RParen for the language service
            if (LanguageServiceMode())
            {
                m_languageServiceExtension->SetLParen(pnode, ichLParen);
                m_languageServiceExtension->SetRParen(pnode, ichRParen);
            }
#endif
        }

        if (buildAST || BindDeferredPidRefs())
        {
            PushBlockInfo(CreateBlockNode());
            PushDynamicBlock();
        }
        ParseNodePtr pnodeBody = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL(ers));
        if (buildAST)
        {
            pnode->sxWith.pnodeBody = pnodeBody;
            m_ppnodeExprScope = ppnodeExprScopeSave;
        }
        else
        {
            m_scopeCountNoAst--;
        }
        if (buildAST || BindDeferredPidRefs())
        {
            // The dynamic block is not stored in the actual parse tree and so will not
            // be visited by the byte code generator.  Grab the calleval flag off it and
            // pass on to outer block in case of:
            // with (...) eval(...); // i.e. blockless form of with
            bool callsEval = GetCurrentBlock()->sxBlock.GetCallsEval();
            PopBlockInfo();
            if (callsEval)
            {
                // be careful not to overwrite an existing true with false
                GetCurrentBlock()->sxBlock.SetCallsEval(true);
            }
        }
        PopStmt(&stmt);
        break;
    }

    case tkLCurly:
        pnode = ParseBlock<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) pnodeLabel, pLabelIdList);
        break;

    case tkSColon:
        pnode = NULL;
#if PARSENODE_EXTENSIONS
        if (buildAST && LanguageServiceMode() && m_pstmtCur && m_pstmtCur->pnodeStmt && m_pstmtCur->pnodeStmt->ichLim <= m_pscan->IchMinTok())
        {
            // Extend the parent statement up to the semicolon token
            m_pstmtCur->pnodeStmt->ichLim = m_pscan->IchMinTok();
        }
#endif
        m_pscan->Scan();
        break;

    case tkBREAK:
        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopBreak>();
        }
        fnop = fnopBreak;
        goto LGetJumpStatement;

    case tkCONTINUE:
        if (buildAST)
        {
            pnode = CreateNode(knopContinue);
        }
        fnop = fnopContinue;

LGetJumpStatement:
        m_pscan->ScanForcingPid();
        if (tkID == m_token.tk && !m_pscan->FHadNewLine())
        {
            // Labeled break or continue.
            pid = m_token.GetIdentifier(m_phtbl);
            AssertMem(pid);
#if PARSENODE_EXTENSIONS
            ParseNodePtr labelNode = NULL;
#endif
            if (buildAST)
            {
#if PARSENODE_EXTENSIONS
                labelNode = CreateNameNode(pid);
#endif
                pnode->sxJump.hasExplicitTarget=true;
                pnode->ichLim = m_pscan->IchLimTok();

                m_pscan->Scan();
                PushStmt<buildAST>(&stmt, pnode, pnode->nop, pnodeLabel, null);
                Assert(pnode->sxStmt.grfnop == 0);
                for (pstmt = m_pstmtCur; NULL != pstmt; pstmt = pstmt->pstmtOuter)
                {
                    AssertNodeMem(pstmt->pnodeStmt);
                    AssertNodeMemN(pstmt->pnodeLab);
                    for (pnodeT = pstmt->pnodeLab; NULL != pnodeT;
                         pnodeT = pnodeT->sxLabel.pnodeNext)
                    {
                        Assert(knopLabel == pnodeT->nop);
                        if (pid == pnodeT->sxLabel.pid)
                        {
                            // Found the label. Make sure we can use it. We can
                            // break out of any statement, but we can only
                            // continue loops.
                            if (fnop == fnopContinue &&
                                !(pstmt->pnodeStmt->Grfnop() & fnop))
                            {
                                Error(ERRbadContinue);
#if ERROR_RECOVERY
                                // No recovery is necessary since this is a semantic, not structural, error
                                // Need to ensure this is not treated as a break during code generation
                                pnode->nop = knopEmpty;
                                pnode->grfpn |= PNodeFlags::fpnJumbStatement;
                                PopStmt(&stmt);
                                goto LNeedTerminator;
#endif
                            }
                            else
                            {
                                pstmt->pnodeStmt->sxStmt.grfnop |= fnop;
                                pnode->sxJump.pnodeTarget = pstmt->pnodeStmt;
                            }
                            PopStmt(&stmt);
                            goto LNeedTerminator;
                        }
                    }
                    pnode->sxStmt.grfnop |=
                        (pstmt->pnodeStmt->Grfnop() & fnopCleanup);
                }
            }
            else
            {
                m_pscan->Scan();
                for (pstmt = m_pstmtCur; pstmt; pstmt = pstmt->pstmtOuter)
                {
                    LabelId* pLabelId;
                    for (pLabelId = pstmt->pLabelId; pLabelId; pLabelId = pLabelId->next)
                    {

                        if (pid == pLabelId->pid)
                        {
                            // Found the label. Make sure we can use it. We can
                            // break out of any statement, but we can only
                            // continue loops.
                            if (fnop == fnopContinue &&
                                !(ParseNode::Grfnop(pstmt->op) & fnop))
                            {
                                Error(ERRbadContinue);
                            }
                            goto LNeedTerminator;
                        }
                    }
                }
            }
#if PARSENODE_EXTENSIONS
            Error(ERRnoLabel, labelNode);
#else
            Error(ERRnoLabel);
#endif
#if ERROR_RECOVERY
            if (buildAST)
            {
                // No recovery is necessary since this is a semantic, not structural, error
                // Need to ensure this is not treated as a break during code generation
                pnode->nop = knopEmpty;
                pnode->grfpn |= PNodeFlags::fpnJumbStatement;
                PopStmt(&stmt);
                goto LNeedTerminator;
            }
#endif
        }
        else
        {
            // If we're doing a fast scan, we're not tracking labels, so we can't accurately do this analysis.
            // Let the thread that's doing the full parse detect the error, if there is one.
            if (!this->m_doingFastScan)
            {
                // Unlabeled break or continue.
                if (buildAST)
                {
                    pnode->sxJump.hasExplicitTarget=false;
                    PushStmt<buildAST>(&stmt, pnode, pnode->nop, pnodeLabel, null);
                    Assert(pnode->sxStmt.grfnop == 0);
                }

                for (pstmt = m_pstmtCur; NULL != pstmt; pstmt = pstmt->pstmtOuter)
                {
                    if (buildAST)
                    {
                        AssertNodeMem(pstmt->pnodeStmt);
                        AssertNodeMemN(pstmt->pnodeLab);
                        if (pstmt->pnodeStmt->Grfnop() & fnop)
                        {
                            pstmt->pnodeStmt->sxStmt.grfnop |= fnop;
                            pnode->sxJump.pnodeTarget = pstmt->pnodeStmt;
                            PopStmt(&stmt);
                            goto LNeedTerminator;
                        }
                        pnode->sxStmt.grfnop |=
                            (pstmt->pnodeStmt->Grfnop() & fnopCleanup);
                    }
                    else
                    {
                        if (pstmt->isDeferred)
                        {
                            if (ParseNode::Grfnop(pstmt->op) & fnop)
                            {
                                goto LNeedTerminator;
                            }
                        }
                        else
                        {
                            AssertNodeMem(pstmt->pnodeStmt);
                            AssertNodeMemN(pstmt->pnodeLab);
                            if (pstmt->pnodeStmt->Grfnop() & fnop)
                            {
                                pstmt->pnodeStmt->sxStmt.grfnop |= fnop;
                                goto LNeedTerminator;
                            }
                        }
                    }
                }
                Error(fnop == fnopBreak ? ERRbadBreak : ERRbadContinue);
#if ERROR_RECOVERY
                if (buildAST)
                {
                    // No recovery is necessary since this is a semantic, not structural, error
                    // Need to ensure this is not treated as a break during code generation
                    pnode->nop = knopEmpty;
                    pnode->grfpn |= PNodeFlags::fpnJumbStatement;
                    PopStmt(&stmt);
                }
#endif
            }
            goto LNeedTerminator;
        }

    case tkRETURN:
    {
        if (buildAST)
        {
            if (NULL == m_currentNodeFunc)
            {
                Error(ERRbadReturn);
            }
            pnode = CreateNodeWithScanner<knopReturn>();
        }
        m_pscan->Scan();
        ParseNodePtr pnodeExpr = NULL;
        ParseOptionalExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersSColon | ers) &pnodeExpr);
        if (buildAST)
        {
            pnode->sxReturn.pnodeExpr = pnodeExpr;
            if (pnodeExpr)
            {
                this->CheckArguments(pnode->sxReturn.pnodeExpr);
                pnode->ichLim = pnode->sxReturn.pnodeExpr->ichLim;
            }
            // See if return should call finally
            PushStmt<buildAST>(&stmt, pnode, knopReturn, pnodeLabel, null);
            Assert(pnode->sxStmt.grfnop == 0);
            for (pstmt = m_pstmtCur; NULL != pstmt; pstmt = pstmt->pstmtOuter)
            {
                AssertNodeMem(pstmt->pnodeStmt);
                AssertNodeMemN(pstmt->pnodeLab);
                if (pstmt->pnodeStmt->Grfnop() & fnopCleanup)
                {
                    pnode->sxStmt.grfnop |= fnopCleanup;
                    break;
                }
            }
            PopStmt(&stmt);
        }
        goto LNeedTerminator;
    }

    case tkTHROW:
    {
        if (buildAST)
        {
            pnode = CreateUniNode(knopThrow, NULL);
        }
        m_pscan->Scan();
        ParseNodePtr pnode1 = NULL;
        if (m_token.tk != tkSColon &&
            m_token.tk != tkRCurly &&
            !m_pscan->FHadNewLine())
        {
            pnode1 = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL(ersSColon | ers));
        }
        else
        {
            Error(ERRdanglingThrow);
#if ERROR_RECOVERY
            if (buildAST)
            {
                // No recovery is necessary since this is a semantic, not structural, error

                // Put an error node in the tree.
                pnode->sxUni.pnode1 = CreateNameNode(m_pidError);
                pnode->ichLim = pnode->sxUni.pnode1->ichLim;
            }
#endif
        }

        if (buildAST)
        {
            pnode->sxUni.pnode1 = pnode1;
            if (pnode1)
            {
                this->CheckArguments(pnode->sxUni.pnode1);
                pnode->ichLim = pnode->sxUni.pnode1->ichLim;
            }
        }
        goto LNeedTerminator;
    }

    case tkDEBUGGER:
        if (buildAST)
        {
            pnode = CreateNodeWithScanner<knopDebugger>();
        }
        m_pscan->Scan();
        goto LNeedTerminator;

LDefaultToken:
    default:
    {
#if ERROR_RECOVERY
        ichMin = m_pscan->IchMinTok();
#endif

#if LANGUAGE_SERVICE
        // Special case class members when deferred
        if (m_grfscr & fscrDeferredClassMemberFnc)
        {
            IdentPtr pidHint = NULL;
            IdentPtr memberPid = this->ParseClassPropertyName(ERROR_RECOVERY_ACTUAL_(ersExprStart | ers) &pidHint);
            LPCOLESTR pNameHint =  nullptr;
            ulong nameHintLength = 0;
            if (pidHint)
            {
                pNameHint = pidHint->Psz();
                nameHintLength = pidHint->Cch();
            }
            pnode = ParseFncDecl<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) fFncNoName | fFncMethod | fFncClassMember, pNameHint, false, /* needsPIDOnRCurlyScan */ true);
            if (buildAST)
            {
                pnode->sxFnc.hint = pNameHint; // Fully qualified name
                pnode->sxFnc.hintLength = nameHintLength;
                pnode->sxFnc.pid = memberPid; // Short name
            }
            break;
        }
#endif

        // An expression statement or a label.
        IdentToken tok;
        pnode = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_((ersSColon | ersColon) | ers) koplNo, nullptr, TRUE, FALSE, nullptr, nullptr /*hintLength*/, &tok);

        if (buildAST)
        {
            // Check for a label.
            if (tkColon == m_token.tk &&
                NULL != pnode && knopName == pnode->nop)
            {
                // We have a label. See if it is already defined.
                if (NULL != PnodeLabel(pnode->sxPid.pid, pnodeLabel))
                {
                    Error(ERRbadLabel);
                    // No recovery is necessary since this is a semantic, not structural, error
                }
                pnodeT = CreateNodeWithScanner<knopLabel>();
                pnodeT->sxLabel.pid = pnode->sxPid.pid;
                pnodeT->sxLabel.pnodeNext = pnodeLabel;
                pnodeLabel = pnodeT;
                m_pscan->Scan();
                isSourceElement = false;
                goto LRestart;
            }

#if ERROR_RECOVERY
            if (ichMin == m_pscan->IchMinTok() && m_token.tk != tkRCurly)
                // This will happen if an error occured in ParseExpr() that was recovered but no tokens
                // were consumed. For example, if the curent token is in the error recovery set it will
                // not be skipped by Skip(). If it not a valid prefix to an expression it will not be
                // skipped by parsing either. Skipping a token here prevents an infinite loop being called
                // by ParseStmtList(). The error recovery set should be modified to avoid this by ensuring
                // the offending token is not in the set.
                //
                // We make an exception for '}'. Arbitrarily skipping an '}' leads to error recovery appearing
                // very broken in common scenarios. '}' will not cause an infinite loop because ParseStmtList()
                // specifically checks for the '}' to terminate the list.
                m_pscan->Scan();
#endif

            expressionStmt = true;

            pnode->isUsed=false;
        }
        else
        {
            // Check for a label.
            if (tkColon == m_token.tk && tok.tk == tkID)
            {
                tok.pid = m_pscan->PidAt(tok.ichMin, tok.ichLim);
                if (PnodeLabelNoAST(&tok, pLabelIdList))
                {
                    Error(ERRbadLabel);
                }
                LabelId* pLabelId = CreateLabelId(&tok);
                pLabelId->next = pLabelIdList;
                pLabelIdList = pLabelId;
                m_pscan->Scan();
                isSourceElement = false;
                goto LRestart;
            }
        }
    }

LNeedTerminator:
#if ECMACP
    if (m_fECMACP)
    {
        if (buildAST)
        {
            ChkCurTok(tkSColon, ERRnoSemic);
        }
        else
        {
            ChkCurTok(tkSColon, ERRnoSemic);
        }
    }
    else
#endif
    {
        // Need a semicolon, new-line, } or end-of-file.
        // We digest a semicolon if it's there.
        switch (m_token.tk)
        {
        case tkSColon:
            m_pscan->Scan();
            if (pnode!= nullptr) pnode->grfpn |= PNodeFlags::fpnExplicitSimicolon;
            break;
        case tkEOF:
        case tkRCurly:
            if (pnode!= nullptr) pnode->grfpn |= PNodeFlags::fpnAutomaticSimicolon;
            break;
        default:
            if (!m_pscan->FHadNewLine())
            {
                Error(ERRnoSemic);
#if ERROR_RECOVERY
                SKIP(ERROR_RECOVERY_ACTUAL(ersStmtStart | ers));
                if (pnode!= nullptr) pnode->grfpn |= PNodeFlags::fpnMissingSimicolon;
#endif
            }
            else
            {
                if (pnode!= nullptr) pnode->grfpn |= PNodeFlags::fpnAutomaticSimicolon;
            }
            break;
        }
        break;
    }
    }

    if (buildAST)
    {
        // All non expression stmts excluded from the "this.x" optimization
        // Another check while parsing expressions
        if (!expressionStmt)
        {
            if (m_currentNodeFunc)
            {
                m_currentNodeFunc->sxFnc.SetHasNonThisStmt();
            }
            else if (m_currentNodeProg)
            {
                m_currentNodeProg->sxFnc.SetHasNonThisStmt();
            }
        }

#if EXCEPTION_RECOVERY
        // close the try/catch block
        if(Js::Configuration::Global.flags.SwallowExceptions)
        {
            // pop the try block and fill in the body
            PopStmt(&stmtTryBlock);
            pTryBlock->sxBlock.pnodeStmt = pnode;
            PopStmt(&stmtTry);
            if(pnode != NULL)
            {
                pTry->ichLim = pnode->ichLim;
            }
            pTry->sxTry.pnodeBody = pTryBlock;


            // create a catch block with an empty body
            StmtNest stmtCatch;
            ParseNodePtr pCatch;
            pCatch = CreateNodeWithScanner<knopCatch>();
            PushStmt<buildAST>(&stmtCatch, pCatch, knopCatch, NULL, NULL);
            pCatch->sxCatch.pnodeBody = NULL;
            if(pnode != NULL)
            {
                pCatch->ichLim = pnode->ichLim;
            }
            pCatch->sxCatch.grfnop = 0;
            pCatch->sxCatch.pnodeNext = NULL;

            // create a fake name for the catch var.
            WCHAR *uniqueNameStr = L"__ehobj";
            IdentPtr uniqueName = m_phtbl->PidHashNameLen(uniqueNameStr, static_cast<long>(wcslen(uniqueNameStr)));

            pCatch->sxCatch.pnodeParam = CreateNameNode(uniqueName);

            // Add this catch to the current list. We don't bother adjusting the catch and func expr
            // lists here because the catch is just an empty statement.

            if (m_ppnodeExprScope)
            {
                Assert(*m_ppnodeExprScope == NULL);
                *m_ppnodeExprScope = pCatch;
                m_ppnodeExprScope = &pCatch->sxCatch.pnodeNext;
            }
            else
            {
                Assert(m_ppnodeScope);
                Assert(*m_ppnodeScope == NULL);
                *m_ppnodeScope = pCatch;
                m_ppnodeScope = &pCatch->sxCatch.pnodeNext;
            }

            pCatch->sxCatch.pnodeScopes = NULL;

            PopStmt(&stmtCatch);

            // fill in and pop the the try-catch
            pParentTryCatch->sxTryCatch.pnodeTry = pTry;
            pParentTryCatch->sxTryCatch.pnodeCatch = pCatch;
            PopStmt(&stmtTryCatch);
            PopStmt(&stmtTryCatchBlock);

            // replace the node that's being returned
            pParentTryCatchBlock->sxBlock.pnodeStmt = pParentTryCatch;
            pnode = pParentTryCatchBlock;
        }
#endif // EXCEPTION_RECOVERY

#if PARSENODE_EXTENSIONS
        if (LanguageServiceMode() && pnode!= nullptr && pnodeLabel != nullptr)
        {
            Assert(m_languageServiceExtension != NULL);
            if (pnode->nop != knopName || pnode->sxPid.pid != m_pidError)
            {
                m_languageServiceExtension->AddLabel(pnode, pnodeLabel);
            }
        }
#endif
    }

    // Restore saved expression depth.
    this->m_exprDepth = oldExprDepth;

    return pnode;
}

/***************************************************************************
Parse a sequence of statements.
***************************************************************************/
template<bool buildAST>
void Parser::ParseStmtList(ERROR_RECOVERY_FORMAL_ ParseNodePtr *ppnodeList, ParseNodePtr **pppnodeLast, StrictModeEnvironment smEnvironment, const bool isSourceElementList, _Out_opt_ bool* strictModeOn)
{
    BOOL doneDirectives = !isSourceElementList; // directives may only exist in a SourceElementList, not a StatementList
    BOOL seenDirectiveContainingOctal = false; // Have we seen an octal directive before a use strict directive?

    BOOL old_UseStrictMode = m_fUseStrictMode;

    ParseNodePtr pnodeStmt;
    ParseNodePtr *lastNodeRef = NULL;

    if (buildAST)
    {
        AssertMem(ppnodeList);
        AssertMemN(pppnodeLast);
        *ppnodeList = NULL;
    }

    if(CONFIG_FLAG(ForceStrictMode))
    {
        m_fUseStrictMode = TRUE;
    }

    for (;;)
    {
        switch (m_token.tk)
        {
        case tkCASE:
        case tkDEFAULT:
        case tkRCurly:
        case tkEOF:
#ifdef LANGUAGE_SERVICE
        case tkExternalSourceEnd:
#endif
            if (buildAST && NULL != pppnodeLast)
            {
                *pppnodeLast = lastNodeRef;
            }
            if (!buildAST)
            {
                m_fUseStrictMode = old_UseStrictMode;
            }
            return;
        }

        if (doneDirectives == FALSE)
        {
            bool isOctalInString = false;
            bool isUseStrictDirective = false;
            bool isUseAsmDirective = false;
            if (smEnvironment != SM_NotUsed && CheckForDirective(&isUseStrictDirective, &isUseAsmDirective, &isOctalInString))
            {
                if (isUseStrictDirective)
                {
                    if (seenDirectiveContainingOctal)
                    {
                        // Directives seen before a "use strict" cannot contain an octal.
                        Error(ERRES5NoOctal);
                    }
                    if (!buildAST)
                    {
                        // Turning on strict mode in deferred code.
                        m_fUseStrictMode = TRUE;
                        if (!m_inDeferredNestedFunc)
                        {
                            // Top-level deferred function, so there's a parse node
                            Assert(m_currentNodeFunc != NULL);
                            m_currentNodeFunc->sxFnc.SetStrictMode();
                        }
                        else if (strictModeOn)
                        {
                            // This turns on strict mode in a deferred function, we need to go back
                            // and re-check duplicated formals.
                            *strictModeOn = true;
                        }
                    }
                    else
                    {
                        if (smEnvironment == SM_OnGlobalCode)
                        {
                            // Turning on strict mode at the top level
                            m_fUseStrictMode = TRUE;
                        }
                        else
                        {
                            // ie. smEnvironment == SM_OnFunctionCode
                            Assert(m_currentNodeFunc != NULL);
                            m_currentNodeFunc->sxFnc.SetStrictMode();
                        }
                    }
                }
                else if (isUseAsmDirective)
                {
                    if (smEnvironment != SM_OnGlobalCode) //Top level use asm doesn't mean anything.
                    {
                        // ie. smEnvironment == SM_OnFunctionCode
                        Assert(m_currentNodeFunc != NULL);
                        m_currentNodeFunc->sxFnc.SetAsmjsMode();
                        m_InAsmMode = true;

                        CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(AsmJSFunctionCount, m_scriptContext);
                    }
                }
                else if (isOctalInString)
                {
                    seenDirectiveContainingOctal = TRUE;
                }
            }
            else
            {
                // The first time we see anything other than a directive we can have no more directives.
                doneDirectives = TRUE;
            }
        }

        // ersElse and ersRParen are removed from the recovery set because even though it would
        // continue a parent production it potentially causes an infinite loop if either token
        // are seen where a statement is expected since the statement would not skip the token
        // and the error recovery set prevents it from being skipped as well. An example of
        // which would be something like "{ else }". ParseStatement will correctly report this
        // as an error since else cannot begin a statement if not preceeded by an if statement.
        // However, if ersElse is in the recovery set it will not be skipped by Skip(). This
        // means the scanner will not advance and this loop will be called again with the
        // scanner still on "else". A similar situation arises with ')', ':', catch, and finally.
        if (NULL != (pnodeStmt = ParseStatement<buildAST>(ERROR_RECOVERY_ACTUAL_(ers & (~(ersElse | ersRParen | ersCatch | ersColon))) isSourceElementList)))
        {
            Assert(buildAST || BindDeferredPidRefs());
            if (buildAST)
            {
                AddToNodeList(ppnodeList, &lastNodeRef, pnodeStmt);
            }
        }
    }
}

void Parser::ParseLanguageServiceContent(ERROR_RECOVERY_FORMAL_ ParseNodePtr *ppnodeList, ParseNodePtr **pppnodeLast, StrictModeEnvironment smEnvironment, const bool isSourceElementList)
{
#ifdef LANGUAGE_SERVICE
    AssertMem(ppnodeList);
    AssertMemN(pppnodeLast);

    *ppnodeList = NULL;
    auto currentStrictMode = m_fUseStrictMode;

    for (;;)
    {
        ParseNodePtr *lastNodeRef = NULL;
        ParseNodePtr pnodeStatements = NULL;
        if (m_token.tk == tkExternalSourceStart)
        {
            m_pscan->Scan();

            ParseStmtList<true>(ERROR_RECOVERY_ACTUAL_(ers) &pnodeStatements, &lastNodeRef, smEnvironment, isSourceElementList);
            AppendToNodeList(ppnodeList, pppnodeLast, pnodeStatements, &lastNodeRef);
            if (m_token.tk == tkExternalSourceEnd)
            {
                m_pscan->Scan();

                // Reset strict mode
                m_fUseStrictMode = currentStrictMode;

                continue;
            }
        }
        else if (m_token.tk == tkExternalSourceEnd)
        {
            // This will only occur if we receive such a comment where we are not expected it. Instead of treating this as
            // an error, just skip it and start ignoring external source comments.
            m_fExpectExternalSource = 0; // Ignore all other external comments since we are in a malformed state.
            m_pscan->Scan();
        }
        m_fExpectExternalSource = 0; // We don't expect any more external source comments, ignore them in case the file is malformed.
        lastNodeRef = NULL;
        pnodeStatements = NULL;
        ParseStmtList<true>(ERROR_RECOVERY_ACTUAL_(ers) &pnodeStatements, &lastNodeRef, smEnvironment, isSourceElementList);
        AppendToNodeList(ppnodeList, pppnodeLast, pnodeStatements, &lastNodeRef);

        if (m_token.tk == tkEOF)
            break;

        auto ich = m_pscan->IchLimTok();
        Error(ERRsyntax);
        Skip(ErrorRecoverySet(ers | ersStmtStart));

        if (m_token.tk == tkEOF)
            break;

        if (ich >= m_pscan->IchLimTok())
        {
            AssertMsg(false, "Error recovery set contains the current token preventing skipping from making progress");
            break;
        }
    }
#else
    ParseStmtList<true>(ERROR_RECOVERY_ACTUAL_(ers) ppnodeList, pppnodeLast, smEnvironment, isSourceElementList);
#endif
}

template <class Fn>
void Parser::VisitFunctionsInScope(ParseNodePtr pnodeScopeList, Fn fn)
{
    ParseNodePtr pnodeScope;
    for (pnodeScope = pnodeScopeList; pnodeScope;)
    {
        switch (pnodeScope->nop)
        {
        case knopBlock:
            VisitFunctionsInScope(pnodeScope->sxBlock.pnodeScopes, fn);
            pnodeScope = pnodeScope->sxBlock.pnodeNext;
            break;

        case knopFncDecl:
            fn(pnodeScope);
            pnodeScope = pnodeScope->sxFnc.pnodeNext;
            break;

        case knopCatch:
            VisitFunctionsInScope(pnodeScope->sxCatch.pnodeScopes, fn);
            pnodeScope = pnodeScope->sxCatch.pnodeNext;
            break;

        case knopWith:
            VisitFunctionsInScope(pnodeScope->sxWith.pnodeScopes, fn);
            pnodeScope = pnodeScope->sxWith.pnodeNext;
            break;

        default:
            AssertMsg(false, "Unexpected node with scope list");
            return;
        }
    }
}

// Scripts above this size (minus string literals and comments) will have parsing of
// function bodies deferred.
ULONG Parser::GetDeferralThreshold(Js::SourceDynamicProfileManager* profileManager)
{
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    if (CONFIG_FLAG(ForceDeferParse) ||
        PHASE_FORCE1(Js::DeferParsePhase) ||
        Js::Configuration::Global.flags.IsEnabled(Js::ForceUndoDeferFlag))
    {
        return 0;
    }
    else if (Js::Configuration::Global.flags.IsEnabled(Js::DeferParseFlag))
    {
        return Js::Configuration::Global.flags.DeferParse;
    }
    else
#endif
    {
        if(profileManager != NULL && profileManager->IsProfileLoaded())
        {
            return DEFAULT_CONFIG_ProfileBasedDeferParseThreshold;
        }
        return DEFAULT_CONFIG_DeferParseThreshold;
    }
}

void Parser::FinishDeferredFunction(ParseNodePtr pnodeScopeList)
{
    VisitFunctionsInScope(pnodeScopeList,
        [this](ParseNodePtr pnodeFnc)
    {
        Assert(pnodeFnc->nop == knopFncDecl);

        if (pnodeFnc->sxFnc.pnodeBody == NULL)
        {
            // Go back and generate an AST for this function.
            JSETW(EventWriteJSCRIPT_PARSE_FUNC(this->GetScriptContext(), pnodeFnc->sxFnc.functionId, /*Undefer*/TRUE));

            ParseNodePtr pnodeFncSave = this->m_currentNodeFunc;
            this->m_currentNodeFunc = pnodeFnc;

            ParseNodePtr pnodeFncExprBlock = null;
            if (pnodeFnc->sxFnc.pnodeNames &&
                !pnodeFnc->sxFnc.IsDeclaration())
            {
                // Set up the named function expression symbol so references inside the function can be bound.
                ParseNodePtr pnodeName = pnodeFnc->sxFnc.pnodeNames;
                Assert(pnodeName->nop == knopVarDecl);
                Assert(pnodeName->sxVar.pnodeNext == null);

                pnodeFncExprBlock = this->StartParseBlock<true>(PnodeBlockType::Function, ScopeType_FuncExpr);
                PidRefStack *ref = this->PushPidRef(pnodeName->sxVar.pid);
                pnodeName->sxVar.symRef = ref->GetSymRef();
                ref->SetSym(pnodeName->sxVar.sym);

                Scope *fncExprScope = pnodeFncExprBlock->sxBlock.scope;
                fncExprScope->AddNewSymbol(pnodeName->sxVar.sym);
                pnodeFnc->sxFnc.scope = fncExprScope;
            }

            ParseNodePtr pnodeBlock = this->StartParseBlock<true>(PnodeBlockType::Parameter, ScopeType_Parameter);
            pnodeFnc->sxFnc.pnodeScopes = pnodeBlock;
            m_ppnodeScope = &pnodeBlock->sxBlock.pnodeScopes;
            pnodeBlock->sxBlock.pnodeStmt = pnodeFnc;

            // Add the args to the scope, since we won't re-parse those.
            Scope *scope = pnodeBlock->sxBlock.scope;
            auto addArgsToScope = [&](ParseNodePtr pnodeArg) {
                PidRefStack *ref = this->PushPidRef(pnodeArg->sxVar.pid);
                pnodeArg->sxVar.symRef = ref->GetSymRef();
                if (ref->GetSym() != nullptr)
                {
                    // Duplicate parameter in a configuration that allows them.
                    // The symbol is already in the scope, just point it to the right declaration.
                    Assert(ref->GetSym() == pnodeArg->sxVar.sym);
                    ref->GetSym()->SetDecl(pnodeArg);
                }
                else
                {
                    ref->SetSym(pnodeArg->sxVar.sym);
                    scope->AddNewSymbol(pnodeArg->sxVar.sym);
                }
            };
            MapFormals(pnodeFnc, addArgsToScope);

            ParseNodePtr pnodeInnerBlock = this->StartParseBlock<true>(PnodeBlockType::Function, ScopeType_FunctionBody);
            pnodeFnc->sxFnc.pnodeBodyScope = pnodeInnerBlock;

            // Set the parameter block's child to the function body block.
            *m_ppnodeScope = pnodeInnerBlock;

            ParseNodePtr *ppnodeScopeSave = nullptr;
            ParseNodePtr *ppnodeExprScopeSave = nullptr;

            ppnodeScopeSave = m_ppnodeScope;

            // This synthetic block scope will contain all the nested scopes.
            m_ppnodeScope = &pnodeInnerBlock->sxBlock.pnodeScopes;
            pnodeInnerBlock->sxBlock.pnodeStmt = pnodeFnc;

            // Keep nested function declarations and expressions in the same list at function scope.
            // (Indicate this by nulling out the current function expressions list.)
            ppnodeExprScopeSave = m_ppnodeExprScope;
            m_ppnodeExprScope = nullptr;

            // Shouldn't be any temps in the arg list.
            Assert(*m_ppnodeVar == nullptr);

            // Start the var list.
            pnodeFnc->sxFnc.pnodeVars = nullptr;
            m_ppnodeVar = &pnodeFnc->sxFnc.pnodeVars;

            this->FinishFncNode(pnodeFnc);

            m_ppnodeExprScope = ppnodeExprScopeSave;

            AssertMem(m_ppnodeScope);
            Assert(NULL == *m_ppnodeScope);
            m_ppnodeScope = ppnodeScopeSave;

            this->FinishParseBlock(pnodeInnerBlock);

            this->AddArgumentsNodeToVars(pnodeFnc);

            this->FinishParseBlock(pnodeBlock);
            if (pnodeFncExprBlock)
            {
                this->FinishParseBlock(pnodeFncExprBlock);
            }

            this->m_currentNodeFunc = pnodeFncSave;
        }
    });
}

void Parser::InitPids()
{
    AssertMemN(m_phtbl);
    wellKnownPropertyPids.arguments = m_phtbl->PidHashNameLen(g_ssym_arguments.sz, g_ssym_arguments.cch);
    wellKnownPropertyPids.eval = m_phtbl->PidHashNameLen(g_ssym_eval.sz, g_ssym_eval.cch);
    wellKnownPropertyPids.getter = m_phtbl->PidHashNameLen(g_ssym_get.sz, g_ssym_get.cch);
    wellKnownPropertyPids.setter = m_phtbl->PidHashNameLen(g_ssym_set.sz, g_ssym_set.cch);
    wellKnownPropertyPids.let = m_phtbl->PidHashNameLen(g_ssym_let.sz, g_ssym_let.cch);
    wellKnownPropertyPids.constructor = m_phtbl->PidHashNameLen(g_ssym_constructor.sz, g_ssym_constructor.cch);
    wellKnownPropertyPids.prototype = m_phtbl->PidHashNameLen(g_ssym_prototype.sz, g_ssym_prototype.cch);
    wellKnownPropertyPids.__proto__ = m_phtbl->PidHashNameLen(L"__proto__", sizeof("__proto__") - 1);
    wellKnownPropertyPids.of = m_phtbl->PidHashNameLen(L"of", sizeof("of") - 1);
#if ERROR_RECOVERY
    m_pidError = m_phtbl->PidHashNameLen(L"?", sizeof("?") - 1);
    m_pidDeclError = m_phtbl->PidHashNameLen(L"??", sizeof("??") - 1);
#endif
}

void Parser::RestoreScopeInfo(Js::FunctionBody* functionBody)
{
    if (!functionBody)
    {
        return;
    }

    Js::ScopeInfo* scopeInfo = functionBody->GetScopeInfo();
    if (!scopeInfo)
    {
        return;
    }

    if (this->IsBackgroundParser())
    {
        PROBE_STACK_NO_DISPOSE(m_scriptContext, Js::Constants::MinStackByteCodeVisitor);
    }
    else
    {
        PROBE_STACK(m_scriptContext, Js::Constants::MinStackByteCodeVisitor);
    }

    RestoreScopeInfo(scopeInfo->GetParent()); // Recursively restore outer func scope info

    Js::ScopeInfo* funcExprScopeInfo = scopeInfo->GetFuncExprScopeInfo();
    if (funcExprScopeInfo)
    {
        funcExprScopeInfo->SetScopeId(m_nextBlockId);
        ParseNodePtr pnodeFncExprScope = StartParseBlockWithCapacity<true>(PnodeBlockType::Function, ScopeType_FuncExpr, funcExprScopeInfo->GetSymbolCount());
        Scope *scope = pnodeFncExprScope->sxBlock.scope;
        funcExprScopeInfo->GetScopeInfo(this, nullptr, nullptr, scope);
    }

    Js::ScopeInfo* paramScopeInfo = scopeInfo->GetParamScopeInfo();
    if (paramScopeInfo)
    {
        paramScopeInfo->SetScopeId(m_nextBlockId);
        ParseNodePtr pnodeFncExprScope = StartParseBlockWithCapacity<true>(PnodeBlockType::Parameter, ScopeType_Parameter, paramScopeInfo->GetSymbolCount());
        Scope *scope = pnodeFncExprScope->sxBlock.scope;
        paramScopeInfo->GetScopeInfo(this, nullptr, nullptr, scope);
    }

    scopeInfo->SetScopeId(m_nextBlockId);
    ParseNodePtr pnodeFncScope = nullptr;
    if (scopeInfo->IsGlobalEval())
    {
        pnodeFncScope = StartParseBlockWithCapacity<true>(PnodeBlockType::Regular, ScopeType_GlobalEvalBlock, scopeInfo->GetSymbolCount());
    }
    else
    {
        pnodeFncScope = StartParseBlockWithCapacity<true>(PnodeBlockType::Function, ScopeType_FunctionBody, scopeInfo->GetSymbolCount());
    }
    Scope *scope = pnodeFncScope->sxBlock.scope;
    scopeInfo->GetScopeInfo(this, nullptr, nullptr, scope);
}

void Parser::FinishScopeInfo(Js::FunctionBody *functionBody)
{
    if (!functionBody)
    {
        return;
    }

    Js::ScopeInfo* scopeInfo = functionBody->GetScopeInfo();
    if (!scopeInfo)
    {
        return;
    }

    if (this->IsBackgroundParser())
    {
        PROBE_STACK_NO_DISPOSE(m_scriptContext, Js::Constants::MinStackByteCodeVisitor);
    }
    else
    {
        PROBE_STACK(m_scriptContext, Js::Constants::MinStackByteCodeVisitor);
    }

    int scopeId = scopeInfo->GetScopeId();

    scopeInfo->GetScope()->ForEachSymbol([this, scopeId](Symbol *sym)
    {
        this->BindPidRefsInScope(sym->GetPid(), sym, scopeId);
    });
    PopScope(scopeInfo->GetScope());
    PopStmt(&m_currentBlockInfo->pstmt);
    PopBlockInfo();

    Js::ScopeInfo *paramScopeInfo = scopeInfo->GetParamScopeInfo();
    if (paramScopeInfo)
    {
        scopeId = paramScopeInfo->GetScopeId();
        paramScopeInfo->GetScope()->ForEachSymbol([this, scopeId](Symbol *sym)
        {
            this->BindPidRefsInScope(sym->GetPid(), sym, scopeId);
        });
        PopScope(paramScopeInfo->GetScope());
        PopStmt(&m_currentBlockInfo->pstmt);
        PopBlockInfo();
    }

    Js::ScopeInfo *funcExprScopeInfo = scopeInfo->GetFuncExprScopeInfo();
    if (funcExprScopeInfo)
    {
        scopeId = funcExprScopeInfo->GetScopeId();
        funcExprScopeInfo->GetScope()->ForEachSymbol([this, scopeId](Symbol *sym)
        {
            this->BindPidRefsInScope(sym->GetPid(), sym, scopeId);
        });
        PopScope(funcExprScopeInfo->GetScope());
        PopStmt(&m_currentBlockInfo->pstmt);
        PopBlockInfo();
    }

    FinishScopeInfo(scopeInfo->GetParent());
}

/***************************************************************************
Parse the code.
***************************************************************************/
ParseNodePtr Parser::Parse(LPCUTF8 pszSrc, size_t offset, size_t length, charcount_t charOffset, ULONG grfscr, ULONG lineNumber, Js::LocalFunctionId * nextFunctionId, CompileScriptException *pse)
{
    ParseNodePtr pnodeProg;
    ParseNodePtr *lastNodeRef = NULL;

    m_nextBlockId = 0;

    // Scanner should run in Running mode and not syntax coloring mode
    grfscr &= ~fscrSyntaxColor;

    if ( (this->m_scriptContext->IsInDebugMode()
#if LANGUAGE_SERVICE_ONLY
          && (grfscr & fscrStmtCompletion) == 0
#endif
        ) || PHASE_OFF1(Js::Phase::DeferParsePhase)
#ifdef ENABLE_PREJIT
         || Js::Configuration::Global.flags.Prejit
#endif
         || ((grfscr & fscrNoDeferParse) != 0)
        )
    {
        // Don't do deferred parsing if debugger is attached or feature is disabled
        // by command-line switch.
        grfscr &= ~fscrDeferFncParse;
    }

    bool isDeferred = (grfscr & fscrDeferredFnc) != 0;

#if LANGUAGE_SERVICE_ONLY
    // If authoringData is set this is a deferred parse request in the context of the language serivce, start
    // parsing as if in statement completion mode.
    if (m_scriptContext->authoringData)
    {
        if (isDeferred)
            grfscr |= fscrStmtCompletion;
        if (m_scriptContext->authoringData->Callbacks())
            m_scriptContext->authoringData->Callbacks()->Parsing();
    }
    m_fExpectExternalSource = 1;
#endif

    m_grfscr = grfscr;
    m_length = length;
    m_originalLength = length;
    m_nextFunctionId = nextFunctionId;

    if(m_parseType != ParseType_Deferred)
    {
        JSETW(EventWriteJSCRIPT_PARSE_METHOD_START(m_sourceContextInfo->dwHostSourceContext, GetScriptContext(), *m_nextFunctionId, 0, m_parseType, Js::Constants::GlobalFunction));
        OUTPUT_TRACE(Js::DeferParsePhase, L"Parsing function (%s) : %s (%d)\n", GetParseType(), Js::Constants::GlobalFunction, *m_nextFunctionId);
    }

#if PARSENODE_EXTENSIONS
    // Create the companion lookup table for languageServiceExtension data
    if (LanguageServiceMode() || (isDeferred && m_scriptContext->authoringData))
    {
        m_languageServiceExtension = (LanguageServiceExtension*)m_nodeAllocator.Alloc(sizeof(LanguageServiceExtension));
        HRESULT hr = m_languageServiceExtension->Init(&m_nodeAllocator, m_scriptContext->GetThreadContext()->GetPageAllocator());
        if (S_OK != hr)
            Error(hr);
    }
#endif

    // Give the scanner the source and get the first token
    m_pscan->SetText(pszSrc, offset, length, charOffset, grfscr, lineNumber);
    m_pscan->Scan();

    // Make the main 'knopProg' node
    long initSize = 0;
    m_pCurrentAstSize = &initSize;
    pnodeProg = CreateNodeWithScanner<knopProg>();
    pnodeProg->grfpn = PNodeFlags::fpnNone;
    pnodeProg->sxFnc.pid = nullptr;
    pnodeProg->sxFnc.pnodeNames = nullptr;
    pnodeProg->sxFnc.pnodeRest = nullptr;
    pnodeProg->sxFnc.ClearFlags();
    pnodeProg->sxFnc.SetNested(FALSE);
    pnodeProg->sxFnc.astSize = 0;
    pnodeProg->sxFnc.cbMin = m_pscan->IecpMinTok();
    pnodeProg->sxFnc.lineNumber = lineNumber;
    pnodeProg->sxFnc.columnNumber = 0;

    if (!isDeferred || (isDeferred && grfscr & fscrGlobalCode))
    {
        // In the deferred case, if the global function is deferred parse (which is in no-refresh case), we will re-use the same function body, so start with the correct functionId
        pnodeProg->sxFnc.functionId = (*m_nextFunctionId)++;
    }
    else
    {
        pnodeProg->sxFnc.functionId = Js::Constants::NoFunctionId;
    }

    m_pCurrentAstSize = & (pnodeProg->sxFnc.astSize);

    pnodeProg->sxFnc.hint = NULL;
    pnodeProg->sxFnc.hintLength = 0;
    pnodeProg->sxFnc.isNameIdentifierRef = true;

    // initialize parsing variables
    pnodeProg->sxFnc.pnodeNext = NULL;

    m_currentNodeFunc = NULL;
    m_currentNodeDeferredFunc = NULL;
    m_currentNodeProg = pnodeProg;
    m_cactIdentToNodeLookup = 1;

    pnodeProg->sxFnc.nestedCount = 0;
    m_pnestedCount = &pnodeProg->sxFnc.nestedCount;
    m_inDeferredNestedFunc = false;

    pnodeProg->sxFnc.pnodeArgs = nullptr;
    pnodeProg->sxFnc.pnodeVars = nullptr;
    pnodeProg->sxFnc.pnodeRest = nullptr;
    m_ppnodeVar = &pnodeProg->sxFnc.pnodeVars;
    SetCurrentStatement(NULL);
    AssertMsg(m_pstmtCur == NULL, "Statement stack should be empty when we start parse global code");

    // Create block for const's and let
    ParseNodePtr pnodeGlobalBlock = StartParseBlock<true>(PnodeBlockType::Global, ScopeType_Global);
    pnodeProg->sxProg.scope = pnodeGlobalBlock->sxBlock.scope;
    ParseNodePtr pnodeGlobalEvalBlock = null;

    // Don't track function expressions separately from declarations at global scope.
    m_ppnodeExprScope = NULL;

    // This synthetic block scope will contain all the nested scopes.
    pnodeProg->sxFnc.pnodeBodyScope = nullptr;
    pnodeProg->sxFnc.pnodeScopes = pnodeGlobalBlock;
    m_ppnodeScope = &pnodeGlobalBlock->sxBlock.pnodeScopes;

    if ((this->m_grfscr & fscrEvalCode) &&
        m_scriptContext->GetConfig()->IsBlockScopeEnabled() &&
        !(this->m_functionBody && this->m_functionBody->GetScopeInfo()))
    {
        pnodeGlobalEvalBlock = StartParseBlock<true>(PnodeBlockType::Regular, ScopeType_GlobalEvalBlock);
        pnodeProg->sxFnc.pnodeScopes = pnodeGlobalEvalBlock;
        m_ppnodeScope = &pnodeGlobalEvalBlock->sxBlock.pnodeScopes;
    }

    Js::ScopeInfo *scopeInfo = null;
    if (m_parseType == ParseType_Deferred && m_functionBody)
    {
        // this->m_functionBody can be cleared during parsing, but we need access to the scope info later.
        scopeInfo = m_functionBody->GetScopeInfo();
        if (scopeInfo)
        {
            this->RestoreScopeInfo(scopeInfo->GetParent());
        }
    }

    // Process a sequence of statements/declarations
#ifdef LANGUAGE_SERVICE
    ParseLanguageServiceContent(
#else
    ParseStmtList<true>(
#endif
        ERROR_RECOVERY_ACTUAL_(ersEOF | ersFunc)
        &pnodeProg->sxFnc.pnodeBody,
        &lastNodeRef,
        SM_OnGlobalCode,
        !(m_grfscr & fscrDeferredFncExpression) /* isSourceElementList */);

    if (m_parseType == ParseType_Deferred)
    {
        if (scopeInfo)
        {
            this->FinishScopeInfo(scopeInfo->GetParent());
        }
    }

    pnodeProg->sxProg.m_UsesArgumentsAtGlobal = m_UsesArgumentsAtGlobal;

    if (IsStrictMode())
    {
        pnodeProg->sxFnc.SetStrictMode();
    }

#if DEBUG
    if(m_grfscr & fscrEnforceJSON && !IsJSONValid(pnodeProg->sxFnc.pnodeBody))
    {
        Error(ERRsyntax);
    }
#endif

    if (tkEOF != m_token.tk)
        Error(ERRsyntax);

    // Append an EndCode node. Give it a zero range so it generates
    // a Bos0.
    // the runtime (jscript!PcFindBos) expects an OP_Bos0, OP_FuncEnd
    // in the global scope
    AddToNodeList(&pnodeProg->sxFnc.pnodeBody, &lastNodeRef,
        CreateNodeWithScanner<knopEndCode>());
    AssertMem(lastNodeRef);
    AssertNodeMem(*lastNodeRef);
    Assert((*lastNodeRef)->nop == knopEndCode);
    (*lastNodeRef)->ichMin = 0;
    (*lastNodeRef)->ichLim = 0;

    // Get the extent of the code.
    pnodeProg->ichLim = m_pscan->IchLimTok();
    pnodeProg->sxFnc.cbLim = m_pscan->IecpLimTok();

    // save the temps and terminate the local list
    // NOTE: Eze makes no use of this.
    //pnodeProg->sxFnc.pnodeTmps = *m_ppnodeVar;
    *m_ppnodeVar = NULL;

    Assert(NULL == *m_ppnodeScope);
    Assert(NULL == pnodeProg->sxFnc.pnodeNext);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.IsEnabled(Js::ForceUndoDeferFlag))
    {
        m_stoppedDeferredParse = true;
    }
#endif

    if (m_stoppedDeferredParse)
    {
        if (this->m_hasParallelJob)
        {
            BackgroundParser *bgp = static_cast<BackgroundParser*>(m_scriptContext->GetBackgroundParser());
            Assert(bgp);
            this->WaitForBackgroundJobs(bgp, pse);
        }

        // Finally, see if there are any function bodies we now want to generate because we
        // decided to stop deferring.
        FinishDeferredFunction(pnodeProg->sxFnc.pnodeScopes);
    }

    if (pnodeGlobalEvalBlock)
    {
        FinishParseBlock(pnodeGlobalEvalBlock);
    }
    // Append block as body of pnodeProg
    FinishParseBlock(pnodeGlobalBlock);

    m_scriptContext->AddSourceSize(m_length);

#if LANGUAGE_SERVICE_ONLY
    if (m_scriptContext->authoringData && ((isDeferred && (grfscr & fscrFunctionHeaderOnly) == 0) || (grfscr & fscrDynamicCode) != 0))
    {
        auto callbackData = (RuntimeParseCallbackData *)m_scriptContext->authoringData;
        if (callbackData->dataType == RUNTIME_PARSE_CALLBACK_TYPE && callbackData->callback)
        {
            callbackData->callback(callbackData->context, this, pszSrc, offset, length, pnodeProg);
        }
    }
#endif

    if (m_asgToConst)
    {
        Error(ERRAssignmentToConst, m_asgToConst.GetIchMin(), m_asgToConst.GetIchLim());
    }

    if(!m_parseType != ParseType_Deferred)
    {
        JSETW(EventWriteJSCRIPT_PARSE_METHOD_STOP(m_sourceContextInfo->dwHostSourceContext, GetScriptContext(), pnodeProg->sxFnc.functionId, *m_pCurrentAstSize, false, Js::Constants::GlobalFunction));
    }
    return pnodeProg;
}


bool Parser::CheckForDirective(bool* pIsUseStrict, bool *pIsUseAsm, bool* pIsOctalInString)
{
    // A directive is a string constant followed by a statement terminating token
    if (m_token.tk != tkStrCon)
        return false;

    // Careful, need to check for octal before calling m_pscan->Scan()
    // because Scan() clears the "had octal" flag on the scanner and
    // m_pscan->Restore() does not restore this flag.
    if (pIsOctalInString != nullptr)
    {
        *pIsOctalInString = m_pscan->IsOctOrLeadingZeroOnLastTKNumber();
    }

    Ident* pidDirective = m_token.GetStr();
    RestorePoint start;
    m_pscan->Capture(&start);
    m_pscan->Scan();

    bool isDirective = true;

    switch (m_token.tk)
    {
    case tkSColon:
    case tkEOF:
    case tkLCurly:
    case tkRCurly:
        break;
    default:
        if (!m_pscan->FHadNewLine())
        {
            isDirective = false;
        }
        break;
    }

    if (isDirective)
    {
        if (pIsUseStrict != nullptr)
        {
            *pIsUseStrict = CheckStrictModeStrPid(pidDirective);
        }
        if (pIsUseAsm != nullptr)
        {
            *pIsUseAsm = CheckAsmjsModeStrPid(pidDirective);
        }
    }

    m_pscan->SeekTo(start);
    return isDirective;
}

bool Parser::CheckStrictModeStrPid(IdentPtr pid)
{
    // If we're already in strict mode, no need to check if the string would put us in strict mode. So, this function would only
    // return true if it detects a transition from non-strict to strict, which is what matters for callers.
    // This is a minor optimization to avoid redundant string comparisons of nested "use strict" directives.
    if (IsStrictMode())
    {
        return false;
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.NoStrictMode)
        return false;
#endif

    return pid != NULL &&
        pid->Cch() == 10 &&
        !m_pscan->IsEscapeOnLastTkStrCon() &&
        wcsncmp(pid->Psz(), L"use strict", 10) == 0;
}

bool Parser::CheckAsmjsModeStrPid(IdentPtr pid)
{
#ifdef ASMJS_PLAT
    //Two flags are ugly, though we don't have ON flag in release bits
    //Once asmjs switches to ON mode by default, replace two switches with single switch
    if (!(PHASE_ON1(Js::AsmjsPhase) || CONFIG_FLAG_RELEASE(Asmjs)))
    {
        return false;
    }

    bool isAsmCandidate = (pid != NULL &&
        AutoSystemInfo::Data.SSE2Available() &&
        pid->Cch() == 7 &&
        !m_pscan->IsEscapeOnLastTkStrCon() &&
        wcsncmp(pid->Psz(), L"use asm", 10) == 0);

    if (isAsmCandidate && m_scriptContext->IsInDebugMode())
    {
        // We would like to report this to debugger - they may choose to disable debugging.
        // TODO : localization of the string?
        m_scriptContext->RaiseMessageToDebugger(DEIT_ASMJS_IN_DEBUGGING, L"AsmJs initialization error - AsmJs disabled due to script debugger", !m_sourceContextInfo->IsDynamic() ? m_sourceContextInfo->url : nullptr);
        return false;
    }

    return isAsmCandidate && !m_isAsmJsDisabled;
#else
    return false;
#endif
}

HRESULT Parser::ParseUtf8Source(__out ParseNodePtr* parseTree, LPCUTF8 pSrc, size_t length, ULONG grfsrc, CompileScriptException *pse,
    Js::LocalFunctionId * nextFunctionId, SourceContextInfo * sourceContextInfo)
{
    m_functionBody = NULL;
    m_parseType = ParseType_Upfront;
    return ParseSourceInternal( parseTree, pSrc, 0, length, 0, true, grfsrc, pse, nextFunctionId, 0, sourceContextInfo);
}

HRESULT Parser::ParseCesu8Source(__out ParseNodePtr* parseTree, LPCUTF8 pSrc, size_t length, ULONG grfsrc, CompileScriptException *pse,
    Js::LocalFunctionId * nextFunctionId, SourceContextInfo * sourceContextInfo)
{
    m_functionBody = NULL;
    m_parseType = ParseType_Upfront;
    return ParseSourceInternal( parseTree, pSrc, 0, length, 0, false, grfsrc, pse, nextFunctionId, 0, sourceContextInfo);
}

void Parser::PrepareScanner(bool fromExternal)
{
    // NOTE: HashTbl and Scanner are currently allocated from the CRT heap. If we want to allocate them from the
    // parser arena, then we also need to change the way the HashTbl allocates PID's from its underlying
    // allocator (which also currently uses the CRT heap). This is not trivial, because we still need to support
    // heap allocation for the colorizer interface.

    // create the hash table and init pid members
    if (NULL == (m_phtbl = HashTbl::Create(HASH_TABLE_SIZE, &m_err)))
        Error(ERRnoMemory);
    InitPids();

    // create the scanner
    if (NULL == (m_pscan = Scanner_t::Create(this, m_phtbl, &m_token, &m_err, m_scriptContext)))
        Error(ERRnoMemory);

    if (fromExternal)
        m_pscan->FromExternalSource();
}

void Parser::PrepareForBackgroundParse()
{
    m_pscan->PrepareForBackgroundParse(m_scriptContext);
}

void Parser::AddBackgroundParseItem(BackgroundParseItem *const item)
{
    if (currBackgroundParseItem == nullptr)
    {
        backgroundParseItems = item;
    }
    else
    {
        currBackgroundParseItem->SetNext(item);
    }
    currBackgroundParseItem = item;
}

void Parser::AddFastScannedRegExpNode(ParseNodePtr const pnode)
{
    Assert(!IsBackgroundParser());
    Assert(m_doingFastScan);

    if (fastScannedRegExpNodes == nullptr)
    {
        fastScannedRegExpNodes = Anew(&m_nodeAllocator, NodeDList, &m_nodeAllocator);
    }
    fastScannedRegExpNodes->Append(pnode);
}

void Parser::AddBackgroundRegExpNode(ParseNodePtr const pnode)
{
    Assert(IsBackgroundParser());
    Assert(currBackgroundParseItem != nullptr);

    currBackgroundParseItem->AddRegExpNode(pnode, &m_nodeAllocator);
}

HRESULT Parser::ParseFunctionInBackground(ParseNodePtr pnodeFnc, ParseContext *parseContext, bool topLevelDeferred, CompileScriptException *pse)
{
    m_functionBody = NULL;
    m_parseType = ParseType_Upfront;
    HRESULT hr = S_OK;
    SmartFPUControl smartFpuControl;
    uint nextFunctionId = pnodeFnc->sxFnc.functionId + 1;

    this->RestoreContext(parseContext);
    DebugOnly( m_err.fInited = TRUE; )
    m_nextFunctionId = &nextFunctionId;
    m_deferringAST = topLevelDeferred;
    m_inDeferredNestedFunc = false;
    m_scopeCountNoAst = 0;

    SetCurrentStatement(NULL);

    pnodeFnc->sxFnc.pnodeVars = nullptr;
    pnodeFnc->sxFnc.pnodeArgs = nullptr;
    pnodeFnc->sxFnc.pnodeBody = nullptr;
    pnodeFnc->sxFnc.nestedCount = 0;

    m_currentNodeFunc = pnodeFnc;
    m_currentNodeDeferredFunc = NULL;
    m_ppnodeScope = NULL;
    m_ppnodeExprScope = NULL;

    m_pnestedCount = &pnodeFnc->sxFnc.nestedCount;
    m_pCurrentAstSize = &pnodeFnc->sxFnc.astSize;

    ParseNodePtr pnodeBlock = StartParseBlock<true>(PnodeBlockType::Function, ScopeType_FunctionBody);
    pnodeFnc->sxFnc.pnodeScopes = pnodeBlock;
    m_ppnodeScope = &pnodeBlock->sxBlock.pnodeScopes;

    uint uDeferSave = m_grfscr & fscrDeferFncParse;

    try
    {
        m_pscan->Scan();

        m_ppnodeVar = &pnodeFnc->sxFnc.pnodeArgs;
        this->ParseFncFormals<true>(ERROR_RECOVERY_ACTUAL_(0) pnodeFnc, fFncNoFlgs);

        if (m_token.tk == tkRParen)
        {
            m_pscan->Scan();
        }

        ChkCurTok(tkLCurly, ERRnoLcurly _ERROR_RECOVERY_ACTUAL(0));

        m_ppnodeVar = &pnodeFnc->sxFnc.pnodeVars;

        // Put the scanner into "no hashing" mode.
        BYTE deferFlags = m_pscan->SetDeferredParse(topLevelDeferred);

        // Process a sequence of statements/declarations
        if (topLevelDeferred)
        {
            ParseStmtList<false>(ERROR_RECOVERY_ACTUAL_(0) nullptr, nullptr, SM_DeferedParse, true);
        }
        else
        {
            ParseNodePtr *lastNodeRef = nullptr;
            ParseStmtList<true>(ERROR_RECOVERY_ACTUAL_(0) &pnodeFnc->sxFnc.pnodeBody, &lastNodeRef, SM_OnFunctionCode, true);
            AddArgumentsNodeToVars(pnodeFnc);
            // Append an EndCode node.
            AddToNodeList(&pnodeFnc->sxFnc.pnodeBody, &lastNodeRef, CreateNodeWithScanner<knopEndCode>());
        }

        // Restore the scanner's default hashing mode.
        m_pscan->SetDeferredParseFlags(deferFlags);

#if DBG
        pnodeFnc->sxFnc.deferredParseNextFunctionId = *this->m_nextFunctionId;
#endif
        this->m_deferringAST = FALSE;

        // Append block as body of pnodeProg
        FinishParseBlock(pnodeBlock);

        if (m_asgToConst)
        {
            Error(ERRAssignmentToConst, m_asgToConst.GetIchMin(), m_asgToConst.GetIchLim());
        }
    }
    catch(ParseExceptionObject& e)
    {
        m_err.m_hr = e.GetError();
        hr = pse->ProcessError( m_pscan, m_err.m_hr, nullptr);
    }

    if (IsStrictMode())
    {
        pnodeFnc->sxFnc.SetStrictMode();
    }

    if (topLevelDeferred)
    {
        pnodeFnc->sxFnc.pnodeVars = nullptr;
    }

    m_grfscr |= uDeferSave;

    Assert(NULL == *m_ppnodeScope);

    return hr;
}

HRESULT Parser::ParseSourceWithOffset(__out ParseNodePtr* parseTree, LPCUTF8 pSrc, size_t offset, size_t cbLength, charcount_t cchOffset,
        bool isCesu8, ULONG grfscr, CompileScriptException *pse, Js::LocalFunctionId * nextFunctionId, ULONG lineNumber, SourceContextInfo * sourceContextInfo,
        Js::ParseableFunctionInfo* functionInfo, bool isReparse, bool isAsmJsDisabled)
{
    m_functionBody = functionInfo;
    m_isAsmJsDisabled = isAsmJsDisabled;
    if (m_functionBody)
    {
        m_currDeferredStub = m_functionBody->GetDeferredStubs();
        m_InAsmMode = isAsmJsDisabled ? false : m_functionBody->GetIsAsmjsMode();
    }
    m_deferAsmJs = !m_InAsmMode;
    m_parseType = isReparse ? ParseType_Reparse : ParseType_Deferred;
    return ParseSourceInternal( parseTree, pSrc, offset, cbLength, cchOffset, !isCesu8, grfscr, pse, nextFunctionId, lineNumber, sourceContextInfo);
}

bool Parser::IsStrictMode() const
{
    return (m_fUseStrictMode ||
           (m_currentNodeFunc != nullptr && m_currentNodeFunc->sxFnc.GetStrictMode()));
}

BOOL Parser::ExpectingExternalSource()
{
    return m_fExpectExternalSource;
}

Symbol *PnFnc::GetFuncSymbol()
{
    if (pnodeNames &&
        pnodeNames->nop == knopVarDecl)
    {
        return pnodeNames->sxVar.sym;
    }
    return null;
}

void PnFnc::SetFuncSymbol(Symbol *sym)
{
    Assert(pnodeNames &&
           pnodeNames->nop == knopVarDecl);
    pnodeNames->sxVar.sym = sym;
}

ParseNodePtr PnFnc::GetParamScope() const
{
    if (this->pnodeScopes == nullptr)
    {
        return nullptr;
    }
    Assert(this->pnodeScopes->nop == knopBlock &&
           this->pnodeScopes->sxBlock.pnodeNext == nullptr);
    return this->pnodeScopes->sxBlock.pnodeScopes;
}

ParseNodePtr * PnFnc::GetParamScopeRef() const
{
    if (this->pnodeScopes == nullptr)
    {
        return nullptr;
    }
    Assert(this->pnodeScopes->nop == knopBlock &&
           this->pnodeScopes->sxBlock.pnodeNext == nullptr);
    return &this->pnodeScopes->sxBlock.pnodeScopes;
}

ParseNodePtr PnFnc::GetBodyScope() const
{
    if (this->pnodeBodyScope == nullptr)
    {
        return nullptr;
    }
    Assert(this->pnodeBodyScope->nop == knopBlock &&
           this->pnodeBodyScope->sxBlock.pnodeNext == nullptr);
    return this->pnodeBodyScope->sxBlock.pnodeScopes;
}

ParseNodePtr * PnFnc::GetBodyScopeRef() const
{
    if (this->pnodeBodyScope == nullptr)
    {
        return nullptr;
    }
    Assert(this->pnodeBodyScope->nop == knopBlock &&
           this->pnodeBodyScope->sxBlock.pnodeNext == nullptr);
    return &this->pnodeBodyScope->sxBlock.pnodeScopes;
}

// Create node versions with explicit token limits
ParseNodePtr Parser::CreateNode(OpCode nop, charcount_t ichMin, charcount_t ichLim)
{
    Assert(!this->m_deferringAST);
    Assert(nop >= 0 && nop < knopLim);
    ParseNodePtr pnode;
    __analysis_assume(nop < knopLim);
    int cb = nop >= 0 && nop < knopLim ? g_mpnopcbNode[nop] : kcbPnNone;

    pnode = (ParseNodePtr)m_nodeAllocator.Alloc(cb);
    Assert(pnode);

    Assert(m_pCurrentAstSize != NULL);
    *m_pCurrentAstSize += cb;

    InitNode(nop,pnode);

    pnode->ichMin = ichMin;
    pnode->ichLim = ichLim;

    return pnode;
}

ParseNodePtr Parser::CreateNameNode(IdentPtr pid,charcount_t ichMin,charcount_t ichLim) {
  ParseNodePtr pnode = CreateNodeT<knopName>(ichMin,ichLim);
  pnode->sxPid.pid = pid;
  pnode->sxPid.sym=NULL;
  pnode->sxPid.symRef=NULL;
  return pnode;
}

ParseNodePtr Parser::CreateUniNode(OpCode nop, ParseNodePtr pnode1, charcount_t ichMin,charcount_t ichLim)
{
    Assert(!this->m_deferringAST);
    DebugOnly(VerifyNodeSize(nop, kcbPnUni));

    ParseNodePtr pnode = (ParseNodePtr)m_nodeAllocator.Alloc(kcbPnUni);

    Assert(m_pCurrentAstSize != NULL);
    *m_pCurrentAstSize += kcbPnUni;

    InitNode(nop, pnode);

    pnode->sxUni.pnode1 = pnode1;

    pnode->ichMin = ichMin;
    pnode->ichLim = ichLim;

    return pnode;
}

ParseNodePtr Parser::CreateBinNode(OpCode nop, ParseNodePtr pnode1,
                                   ParseNodePtr pnode2,charcount_t ichMin,charcount_t ichLim)
{
    Assert(!this->m_deferringAST);
    ParseNodePtr pnode = StaticCreateBinNode(nop, pnode1, pnode2, &m_nodeAllocator);

    Assert(m_pCurrentAstSize != NULL);
    *m_pCurrentAstSize += kcbPnBin;

    pnode->ichMin = ichMin;
    pnode->ichLim = ichLim;

    return pnode;
}

ParseNodePtr Parser::CreateTriNode(OpCode nop, ParseNodePtr pnode1,
                                   ParseNodePtr pnode2, ParseNodePtr pnode3,
                                   charcount_t ichMin,charcount_t ichLim)
{
    Assert(!this->m_deferringAST);
    DebugOnly(VerifyNodeSize(nop, kcbPnTri));
    ParseNodePtr pnode = (ParseNodePtr)m_nodeAllocator.Alloc(kcbPnTri);

    Assert(m_pCurrentAstSize != NULL);
    *m_pCurrentAstSize += kcbPnTri;

    InitNode(nop, pnode);

    pnode->sxTri.pnodeNext = NULL;
    pnode->sxTri.pnode1 = pnode1;
    pnode->sxTri.pnode2 = pnode2;
    pnode->sxTri.pnode3 = pnode3;

    pnode->ichMin = ichMin;
    pnode->ichLim = ichLim;

    return pnode;
}

bool PnBlock::HasBlockScopedContent() const
{
    // A block has its own content if a let, const, or function is declared there.

    if (this->pnodeLexVars != nullptr || this->blockType == Parameter)
    {
        return true;
    }

    // The enclosing scopes can contain functions and other things, so walk the list
    // looking specifically for functions.

    for (ParseNodePtr pnode = this->pnodeScopes; pnode;)
    {
        switch (pnode->nop) {

        case knopFncDecl:
            return true;

        case knopBlock:
            pnode = pnode->sxBlock.pnodeNext;
            break;

        case knopCatch:
            pnode = pnode->sxCatch.pnodeNext;
            break;

        case knopWith:
            pnode = pnode->sxWith.pnodeNext;
            break;

        default:
            Assert(UNREACHED);
            return true;
        }
    }

    return false;
}

class ByteCodeGenerator;

// Copy AST; this works mostly on expressions for now
ParseNode* Parser::CopyPnode(ParseNode *pnode) {
    if (pnode==NULL)
        return NULL;
    switch (pnode->nop) {
        //PTNODE(knopName       , "name"        ,None    ,Pid  ,fnopLeaf)
    case knopName: {
      ParseNode* nameNode=CreateNameNode(pnode->sxPid.pid,pnode->ichMin,pnode->ichLim);
      nameNode->sxPid.sym=pnode->sxPid.sym;
      return nameNode;
    }
      //PTNODE(knopInt        , "int const"    ,None    ,Int  ,fnopLeaf|fnopConst)
  case knopInt:
    return pnode;
      //PTNODE(knopFlt        , "flt const"    ,None    ,Flt  ,fnopLeaf|fnopConst)
  case knopFlt:
    return pnode;
      //PTNODE(knopStr        , "str const"    ,None    ,Pid  ,fnopLeaf|fnopConst)
  case knopStr:
    return pnode;
      //PTNODE(knopRegExp     , "reg expr"    ,None    ,Pid  ,fnopLeaf|fnopConst)
  case knopRegExp:
    return pnode;
    break;
      //PTNODE(knopThis       , "this"        ,None    ,None ,fnopLeaf)
  case knopThis:
    return CreateNodeT<knopThis>(pnode->ichMin,pnode->ichLim);
      //PTNODE(knopNull       , "null"        ,Null    ,None ,fnopLeaf)
  case knopNull:
    return pnode;
      //PTNODE(knopFalse      , "false"        ,False   ,None ,fnopLeaf)
  case knopFalse:
    return CreateNodeT<knopFalse>(pnode->ichMin,pnode->ichLim);
      break;
      //PTNODE(knopTrue       , "true"        ,True    ,None ,fnopLeaf)
  case knopTrue:
    return CreateNodeT<knopTrue>(pnode->ichMin,pnode->ichLim);
      //PTNODE(knopEmpty      , "empty"        ,Empty   ,None ,fnopLeaf)
  case knopEmpty:
    return CreateNodeT<knopEmpty>(pnode->ichMin,pnode->ichLim);
      // Unary operators.
      //PTNODE(knopNot        , "~"            ,BitNot  ,Uni  ,fnopUni)
      //PTNODE(knopNeg        , "unary -"    ,Neg     ,Uni  ,fnopUni)
      //PTNODE(knopPos        , "unary +"    ,Pos     ,Uni  ,fnopUni)
      //PTNODE(knopLogNot     , "!"            ,LogNot  ,Uni  ,fnopUni)
      //PTNODE(knopEllipsis     , "..."       ,Spread  ,Uni    , fnopUni)
      //PTNODE(knopDecPost    , "-- post"    ,Dec     ,Uni  ,fnopUni|fnopAsg)
      //PTNODE(knopIncPre     , "++ pre"    ,Inc     ,Uni  ,fnopUni|fnopAsg)
      //PTNODE(knopDecPre     , "-- pre"    ,Dec     ,Uni  ,fnopUni|fnopAsg)
      //PTNODE(knopTypeof     , "typeof"    ,None    ,Uni  ,fnopUni)
      //PTNODE(knopVoid       , "void"        ,Void    ,Uni  ,fnopUni)
      //PTNODE(knopDelete     , "delete"    ,None    ,Uni  ,fnopUni)
  case knopNot:
  case knopNeg:
  case knopPos:
  case knopLogNot:
  case knopEllipsis:
  case knopIncPost:
  case knopDecPost:
  case knopIncPre:
  case knopDecPre:
  case knopTypeof:
  case knopVoid:
  case knopDelete:
    return CreateUniNode(pnode->nop,CopyPnode(pnode->sxUni.pnode1),pnode->ichMin,pnode->ichLim);
      //PTNODE(knopArray      , "arr cnst"    ,None    ,Uni  ,fnopUni)
      //PTNODE(knopObject     , "obj cnst"    ,None    ,Uni  ,fnopUni)
  case knopArray:
  case knopObject:
    // TODO: need to copy arr
    Assert(false);
    break;
      // Binary operators
      //PTNODE(knopAdd        , "+"            ,Add     ,Bin  ,fnopBin)
      //PTNODE(knopSub        , "-"            ,Sub     ,Bin  ,fnopBin)
      //PTNODE(knopMul        , "*"            ,Mul     ,Bin  ,fnopBin)
      //PTNODE(knopDiv        , "/"            ,Div     ,Bin  ,fnopBin)
      //PTNODE(knopMod        , "%"            ,Mod     ,Bin  ,fnopBin)
      //PTNODE(knopOr         , "|"            ,BitOr   ,Bin  ,fnopBin)
      //PTNODE(knopXor        , "^"            ,BitXor  ,Bin  ,fnopBin)
      //PTNODE(knopAnd        , "&"            ,BitAnd  ,Bin  ,fnopBin)
      //PTNODE(knopEq         , "=="        ,EQ      ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopNe         , "!="        ,NE      ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopLt         , "<"            ,LT      ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopLe         , "<="        ,LE      ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopGe         , ">="        ,GE      ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopGt         , ">"            ,GT      ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopEqv        , "==="        ,Eqv     ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopIn         , "in"        ,In      ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopInstOf     , "instanceof",InstOf  ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopNEqv       , "!=="        ,NEqv    ,Bin  ,fnopBin|fnopRel)
      //PTNODE(knopComma      , ","            ,None    ,Bin  ,fnopBin)
      //PTNODE(knopLogOr      , "||"        ,None    ,Bin  ,fnopBin)
      //PTNODE(knopLogAnd     , "&&"        ,None    ,Bin  ,fnopBin)
      //PTNODE(knopLsh        , "<<"        ,Lsh     ,Bin  ,fnopBin)
      //PTNODE(knopRsh        , ">>"        ,Rsh     ,Bin  ,fnopBin)
      //PTNODE(knopRs2        , ">>>"        ,Rs2     ,Bin  ,fnopBin)
  case knopAdd:
  case knopSub:
  case knopMul:
  case knopDiv:
  case knopMod:
  case knopOr:
  case knopXor:
  case knopAnd:
  case knopEq:
  case knopNe:
  case knopLt:
  case knopLe:
  case knopGe:
  case knopGt:
  case knopEqv:
  case knopIn:
  case knopInstOf:
  case knopNEqv:
  case knopComma:
  case knopLogOr:
  case knopLogAnd:
  case knopLsh:
  case knopRsh:
  case knopRs2:
      //PTNODE(knopAsg        , "="            ,None    ,Bin  ,fnopBin|fnopAsg)
  case knopAsg:
      //PTNODE(knopDot        , "."            ,None    ,Bin  ,fnopBin)
  case knopDot:
      //PTNODE(knopAsgAdd     , "+="        ,Add     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgAdd:
      //PTNODE(knopAsgSub     , "-="        ,Sub     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgSub:
      //PTNODE(knopAsgMul     , "*="        ,Mul     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgMul:
      //PTNODE(knopAsgDiv     , "/="        ,Div     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgDiv:
      //PTNODE(knopAsgMod     , "%="        ,Mod     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgMod:
      //PTNODE(knopAsgAnd     , "&="        ,BitAnd  ,Bin  ,fnopBin|fnopAsg)
  case knopAsgAnd:
      //PTNODE(knopAsgXor     , "^="        ,BitXor  ,Bin  ,fnopBin|fnopAsg)
  case knopAsgXor:
      //PTNODE(knopAsgOr      , "|="        ,BitOr   ,Bin  ,fnopBin|fnopAsg)
  case knopAsgOr:
      //PTNODE(knopAsgLsh     , "<<="        ,Lsh     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgLsh:
      //PTNODE(knopAsgRsh     , ">>="        ,Rsh     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgRsh:
      //PTNODE(knopAsgRs2     , ">>>="        ,Rs2     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgRs2:
      //PTNODE(knopMember     , ":"            ,None    ,Bin  ,fnopBin)
  case knopMember:
  case knopMemberShort:
      //PTNODE(knopIndex      , "[]"        ,None    ,Bin  ,fnopBin)
      //PTNODE(knopList       , "<list>"    ,None    ,Bin  ,fnopNone)

  case knopIndex:
  case knopList:
    return CreateBinNode(pnode->nop,CopyPnode(pnode->sxBin.pnode1),
                         CopyPnode(pnode->sxBin.pnode2),pnode->ichMin,pnode->ichLim);

      //PTNODE(knopCall       , "()"        ,None    ,Bin  ,fnopBin)
      //PTNODE(knopNew        , "new"        ,None    ,Bin  ,fnopBin)
  case knopNew:
  case knopCall:
    return CreateCallNode(pnode->nop,CopyPnode(pnode->sxBin.pnode1),
                         CopyPnode(pnode->sxBin.pnode2),pnode->ichMin,pnode->ichLim);
      //PTNODE(knopQmark      , "?"            ,None    ,Tri  ,fnopBin)
  case knopQmark:
    return CreateTriNode(pnode->nop,CopyPnode(pnode->sxTri.pnode1),
                         CopyPnode(pnode->sxTri.pnode2),CopyPnode(pnode->sxTri.pnode3),
                         pnode->ichMin,pnode->ichLim);
      //PTNODE(knopScope      , "::"        ,None    ,Bin  ,fnopBin)
  case knopScope:
    Assert(false);
    break;
      // General nodes.
      //PTNODE(knopVarDecl    , "varDcl"    ,None    ,Var  ,fnopNone)
    case knopVarDecl: {
      ParseNode* copyNode=CreateNodeT<knopVarDecl>(pnode->ichMin,pnode->ichLim);
      copyNode->sxVar.pnodeInit=CopyPnode(pnode->sxVar.pnodeInit);
      copyNode->sxVar.sym=pnode->sxVar.sym;
      // TODO: mult-decl
      Assert(pnode->sxVar.pnodeNext==NULL);
      copyNode->sxVar.pnodeNext=NULL;
      return copyNode;
    }
      //PTNODE(knopFncDecl    , "fncDcl"    ,None    ,Fnc  ,fnopLeaf)
      //PTNODE(knopProg       , "program"    ,None    ,Fnc  ,fnopNone)
  case knopFncDecl:
  case knopProg:
    Assert(false);
    break;
      //PTNODE(knopEndCode    , "<endcode>"    ,None    ,None ,fnopNone)
  case knopEndCode:
    break;
      //PTNODE(knopDebugger   , "debugger"    ,None    ,None ,fnopNone)
  case knopDebugger:
    break;
      //PTNODE(knopFor        , "for"        ,None    ,For  ,fnopBreak|fnopContinue)
    case knopFor: {
      ParseNode* copyNode=CreateNodeT<knopFor>(pnode->ichMin,pnode->ichLim);
      copyNode->sxFor.pnodeInverted=NULL;
      copyNode->sxFor.pnodeInit=CopyPnode(pnode->sxFor.pnodeInit);
      copyNode->sxFor.pnodeCond=CopyPnode(pnode->sxFor.pnodeCond);
      copyNode->sxFor.pnodeIncr=CopyPnode(pnode->sxFor.pnodeIncr);
      copyNode->sxFor.pnodeBody=CopyPnode(pnode->sxFor.pnodeBody);
      return copyNode;
    }
      //PTNODE(knopIf         , "if"        ,None    ,If   ,fnopNone)
  case knopIf:
    Assert(false);
    break;
      //PTNODE(knopWhile      , "while"        ,None    ,While,fnopBreak|fnopContinue)
  case knopWhile:
    Assert(false);
    break;
      //PTNODE(knopDoWhile    , "do-while"    ,None    ,While,fnopBreak|fnopContinue)
  case knopDoWhile:
    Assert(false);
    break;
      //PTNODE(knopForIn      , "for in"    ,None    ,ForIn,fnopBreak|fnopContinue|fnopCleanup)
  case knopForIn:
    Assert(false);
    break;
  case knopForOf:
    Assert(false);
    break;
      //PTNODE(knopReturn     , "return"    ,None    ,Uni  ,fnopNone)
  case knopReturn: {
    ParseNode* copyNode=CreateNodeT<knopReturn>(pnode->ichMin,pnode->ichLim);
    copyNode->sxReturn.pnodeExpr=CopyPnode(pnode->sxReturn.pnodeExpr);
    return copyNode;
  }
      //PTNODE(knopBlock      , "{}"        ,None    ,Block,fnopNone)
  case knopBlock: {
    ParseNode* copyNode=CreateBlockNode(pnode->ichMin,pnode->ichLim,pnode->sxBlock.blockType);
    if (pnode->grfpn & PNodeFlags::fpnSyntheticNode) {
        // fpnSyntheticNode is sometimes set on PnodeBlockType::Regular blocks which
        // CreateBlockNode() will not automatically set for us, so set it here if it's
        // specified on the source node.
        copyNode->grfpn |= PNodeFlags::fpnSyntheticNode;
    }
    copyNode->sxBlock.pnodeStmt=CopyPnode(pnode->sxBlock.pnodeStmt);
    return copyNode;
  }
      //PTNODE(knopWith       , "with"        ,None    ,With ,fnopCleanup)
  case knopWith:
    Assert(false);
    break;
      //PTNODE(knopBreak      , "break"        ,None    ,Jump ,fnopNone)
  case knopBreak:
    Assert(false);
    break;
      //PTNODE(knopContinue   , "continue"    ,None    ,Jump ,fnopNone)
  case knopContinue:
    Assert(false);
    break;
      //PTNODE(knopLabel      , "label"        ,None    ,Label,fnopNone)
  case knopLabel:
    Assert(false);
    break;
      //PTNODE(knopSwitch     , "switch"    ,None    ,Switch,fnopBreak)
  case knopSwitch:
    Assert(false);
    break;
      //PTNODE(knopCase       , "case"        ,None    ,Case ,fnopNone)
  case knopCase:
    Assert(false);
    break;
      //PTNODE(knopTryFinally,"try-finally",None,TryFinally,fnopCleanup)
  case knopTryFinally:
    Assert(false);
    break;
  case knopFinally:
    Assert(false);
    break;
      //PTNODE(knopCatch      , "catch"     ,None    ,Catch,fnopNone)
  case knopCatch:
    Assert(false);
    break;
      //PTNODE(knopTryCatch      , "try-catch" ,None    ,TryCatch  ,fnopCleanup)
  case knopTryCatch:
    Assert(false);
    break;
      //PTNODE(knopTry        , "try"       ,None    ,Try  ,fnopCleanup)
  case knopTry:
    Assert(false);
    break;
      //PTNODE(knopThrow      , "throw"     ,None    ,Uni  ,fnopNone)
  case knopThrow:
    Assert(false);
    break;
  default:
    Assert(false);
    break;
    }
    return NULL;
}

// Returns true when str is string for Nan, Inifinity or -Infinity.
// Does not check for double number value being in NaN/Infinity range.
// static
template<bool CheckForNegativeInfinity>
inline bool Parser::IsNaNOrInfinityLiteral(LPCOLESTR str)
{
    // Note: wcscmp crashes when one of the parameters is NULL.
    return str &&
           (wcscmp(L"NaN", str) == 0 ||
           wcscmp(L"Infinity", str) == 0 ||
           CheckForNegativeInfinity && wcscmp(L"-Infinity", str) == 0);
}

template <bool buildAST>
ParseNodePtr Parser::ParseSuper(ERROR_RECOVERY_FORMAL_ ParseNodePtr pnode)
{
    ParseNodePtr currentNodeFunc = GetCurrentFunctionNode();

    if (buildAST) {
        pnode = CreateNodeWithScanner<knopSuper>();
    }

    m_pscan->ScanForcingPid();

    switch (m_token.tk)
    {
    case tkDot:     // super.prop
    case tkLBrack:  // super[foo]
    case tkLParen:  // super(args)
        break;

    default:
        if (!m_pscan->FHadNewLine())
        {
#if ERROR_RECOVERY
            Error(ERRInvalidSuper, pnode);
            goto LErrorRecovery;
#else
            Error(ERRInvalidSuper);
#endif
        }
        break;
    }
    currentNodeFunc->sxFnc.SetHasSuperReference(TRUE);
    CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(SuperCount, m_scriptContext);
    return pnode;

#if ERROR_RECOVERY
LErrorRecovery:
    if (buildAST)
    {
        pnode = CreateErrorNameNode();
    }
    return pnode;
#endif
}

template <bool buildAST>
ParseNodePtr Parser::ParseDestructuredArrayLiteral(ERROR_RECOVERY_FORMAL_ tokens declarationType, bool isDecl, bool topLevel)
{
    Assert(m_token.tk == tkLBrack);

    m_pscan->Scan();

    ParseNodePtr pnodeDestructAsg = nullptr;
    ParseNodePtr pnodeDestructArr = nullptr;
    ParseNodePtr pnodeDefault = nullptr;

    ParseNodePtr pnodeList = nullptr;
    ParseNodePtr *lastNodeRef = nullptr;
    uint count = 0;
    int parenCount = 0;
    bool hasMissingValues = false;
    bool seenRest = false;
    charcount_t ichMin = m_pscan->IchMinTok();
    charcount_t ichLim;
#if PARSENODE_EXTENSIONS
    charcount_t ichDeadRangeMin;
#endif

    for (;;)
    {
        ParseNodePtr pnodeElem = nullptr;

        switch (m_token.tk)
        {
        case tkLParen:
            // Swallow parens, they have no effect.
            m_pscan->Scan();
            ++parenCount;
            continue;

        case tkRParen:
            m_pscan->Scan();
            --parenCount;
            continue;

        // Missing values
        case tkRBrack:
            if (count == 0)
            {
                break;
            }
            // fall through
        case tkComma:
            hasMissingValues = true;
            if (buildAST)
            {
                pnodeElem = CreateNodeWithScanner<knopEmpty>();
            }
            ++count;
            break;

        // Nested array literal
        case tkLBrack:
            if (isDecl)
            {
                pnodeElem = ParseDestructuredArrayLiteral<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) declarationType, isDecl, false);
            }
            else
            {
                pnodeElem = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) koplCma);
            }
            ++count;
            break;

        // Nested object literal
        case tkLCurly:
            AssertMsg(false, "Not implemented");
            break;

        case tkEllipsis:
            seenRest = true;
            m_pscan->Scan();
            if (m_token.tk != tkID && m_token.tk != tkSUPER)
            {
                Error(ERRsyntax);
#if ERROR_RECOVERY                
                m_token.SetIdentifier(m_pidError);
#endif
            }
            // fall through

        case tkSUPER:
        case tkID:
        {
            if (isDecl)
            {
                ichMin = m_pscan->IchMinTok();
#if PARSENODE_EXTENSIONS
                ichDeadRangeMin = m_pscan->IchLimTok();
#endif
                pnodeElem = ParseVariableDeclaration<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) declarationType, ichMin
#if PARSENODE_EXTENSIONS
                                                               , ichDeadRangeMin
#endif
                ,/* fAllowIn */false, /* pfForInOk */nullptr, /* singleDefOnly */true, /* allowInit */!seenRest);
            }
            else
            {
                // We aren't declaring anything, so scan the ID reference manually.
                pnodeElem = ParseTerm<buildAST>(ERROR_RECOVERY_ACTUAL_(ersAsg | ersComma | ers) /* fAllowCall */ m_token.tk != tkSUPER);

                // Swallow RParens before a default expression, if any.
                while (m_token.tk == tkRParen)
                {
                    m_pscan->Scan();
                    --parenCount;
                }

                if (m_token.tk != tkAsg)
                {
                    break;
                }

                // Parse the initializer.
                if (seenRest)
                {
                    Error(ERRRestWithDefault);
                }
                m_pscan->Scan();

                ParseNodePtr pnodeInit = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ersComma | ers) koplCma);

                if (buildAST)
                {
                    pnodeElem = CreateBinNode(knopAsg, pnodeElem, pnodeInit);
                }
            }
            ++count;
            break;
         }

        default:
            if (m_token.IsOperator())
            {
                Error(ERRDestructNoOper);
            }
            Error(ERRDestructIDRef);
        }

        if (buildAST && count > 0)
        {
            if (seenRest)
            {
                ParseNodePtr pnodeRest = CreateNodeWithScanner<knopEllipsis>();
                pnodeRest->sxUni.pnode1 = pnodeElem;
                pnodeElem = pnodeRest;
            }
            AddToNodeListEscapedUse(&pnodeList, &lastNodeRef, pnodeElem);
#if PARSENODE_EXTENSIONS
            if (LanguageServiceMode() && pnodeList && pnodeList->nop == knopList)
            {
                pnodeList->ichLim = pnodeElem->ichLim;          // update the ichLim of the list
                pnodeList->grfpn |= PNodeFlags::fpnDclList;  // this is a var, let, or const decl list, mark it
            }
#endif
        }

        // Swallow RParens.
        while (m_token.tk == tkRParen)
        {
            m_pscan->Scan();
            --parenCount;
        }
        if (parenCount != 0)
        {
            Error(ERRnoRparen);
        }

        if (m_token.tk == tkRBrack)
        {
            break;
        }

        // Rest must be in the last position.
        if (seenRest)
        {
            Error(ERRDestructRestLast);
        }

        if (m_token.tk != tkComma)
        {
            if (m_token.IsOperator())
            {
                Error(ERRDestructNoOper);
            }
            Error(ERRsyntax);
        }

        m_pscan->Scan();
    }

    ichLim = m_pscan->IchLimTok();

    if (buildAST)
    {
        pnodeDestructArr = CreateNodeWithScanner<knopArray>();
        pnodeDestructArr->sxArrLit.pnode1 = pnodeList;
        pnodeDestructArr->sxArrLit.arrayOfTaggedInts = false;
        pnodeDestructArr->sxArrLit.arrayOfInts = false;
        pnodeDestructArr->sxArrLit.arrayOfNumbers = false;
        pnodeDestructArr->sxArrLit.hasMissingValues = hasMissingValues;
        pnodeDestructArr->sxArrLit.count = count;
        pnodeDestructArr->sxArrLit.spreadCount = seenRest ? 1 : 0;
        pnodeDestructArr->ichMin = ichMin;
        pnodeDestructArr->ichLim = ichLim;

        if (pnodeDestructArr->sxArrLit.pnode1)
        {
            this->CheckArguments(pnodeDestructArr->sxArrLit.pnode1);
        }
    }

    m_pscan->Scan();

    if (topLevel
        && m_token.tk != tkAsg
        // Check for For-in or for-of
        && !(m_token.tk == tkIN || m_scriptContext->GetConfig()->IsES6IteratorsEnabled() && m_token.tk == tkID && m_token.GetIdentifier(m_phtbl) == wellKnownPropertyPids.of))
    {
        Error(ERRDestructInit);
    }

    if (!isDecl || (isDecl && m_token.tk != tkAsg))
    {
        return pnodeDestructArr;
    }

    m_pscan->Scan();
    pnodeDefault = ParseExpr<buildAST>(ERROR_RECOVERY_ACTUAL_(ers) koplCma);

    if (buildAST)
    {
        pnodeDestructAsg = CreateNodeWithScanner<knopAsg>();
        pnodeDestructAsg->sxBin.pnode1 = pnodeDestructArr;
        pnodeDestructAsg->sxBin.pnode2 = pnodeDefault;
        pnodeDestructAsg->ichMin = ichMin;
        pnodeDestructAsg->ichLim = m_pscan->IchLimTok();
    }

    return pnodeDestructAsg;
}

void Parser::CaptureContext(ParseContext *parseContext) const
{
    parseContext->pszSrc = m_pscan->PchBase();
    parseContext->offset = m_pscan->IchMinTok() + m_pscan->m_cMultiUnits;
    parseContext->length = this->m_originalLength;
    parseContext->characterOffset = parseContext->offset - m_pscan->m_cMultiUnits;
    parseContext->grfscr = this->m_grfscr;
    parseContext->lineNumber = m_pscan->LineCur();

    parseContext->pnodeProg = this->m_currentNodeProg;
    parseContext->fromExternal = m_pscan->IsFromExternalSource();
    parseContext->strictMode = this->IsStrictMode();
    parseContext->sourceContextInfo = this->m_sourceContextInfo;
    parseContext->currentBlockInfo = this->m_currentBlockInfo;
    parseContext->nextBlockId = this->m_nextBlockId;
}

void Parser::RestoreContext(ParseContext *const parseContext)
{
    m_sourceContextInfo = parseContext->sourceContextInfo;
    m_currentBlockInfo = parseContext->currentBlockInfo;
    m_nextBlockId = parseContext->nextBlockId;
    m_grfscr = parseContext->grfscr;
    m_length = parseContext->length;
    m_pscan->SetText(parseContext->pszSrc, parseContext->offset, parseContext->length, parseContext->characterOffset, parseContext->grfscr, parseContext->lineNumber);
    m_currentNodeProg = parseContext->pnodeProg;
    m_fUseStrictMode = parseContext->strictMode;
}

class ByteCodeGenerator;
#if DBG_DUMP

#define INDENT_SIZE 2

void PrintPnodeListWIndent(ParseNode *pnode,int indentAmt);



void Indent(int indentAmt) {
    for (int i=0;i<indentAmt;i++) {
        Output::Print(L" ");
    }
}

void PrintScopesWIndent(ParseNode *pnode,int indentAmt) {
    ParseNode *scope = nullptr;
    bool firstOnly = false;
    switch(pnode->nop)
    {
    case knopProg:
    case knopFncDecl: scope = pnode->sxFnc.pnodeScopes; break;
    case knopBlock: scope = pnode->sxBlock.pnodeScopes; break;
    case knopCatch: scope = pnode->sxCatch.pnodeScopes; break;
    case knopWith: scope = pnode->sxWith.pnodeScopes; break;
    case knopSwitch: scope = pnode->sxSwitch.pnodeBlock; firstOnly = true; break;
    case knopFor: scope = pnode->sxFor.pnodeBlock; firstOnly = true; break;
    case knopForIn: scope = pnode->sxForInOrForOf.pnodeBlock; firstOnly = true; break;
    case knopForOf: scope = pnode->sxForInOrForOf.pnodeBlock; firstOnly = true; break;
    }
    if (scope) {
        Indent(indentAmt);
        Output::Print(L"Scopes: ");
        ParseNode *next = nullptr;
        ParseNode *syntheticBlock = nullptr;
        while (scope) {
            switch (scope->nop) {
            case knopFncDecl: Output::Print(L"knopFncDecl"); next = scope->sxFnc.pnodeNext; break;
            case knopBlock: Output::Print(L"knopBlock"); next = scope->sxBlock.pnodeNext; break;
            case knopCatch: Output::Print(L"knopCatch"); next = scope->sxCatch.pnodeNext; break;
            case knopWith: Output::Print(L"knopWith"); next = scope->sxWith.pnodeNext; break;
            default: Output::Print(L"unknown"); break;
            }
            if (firstOnly) {
                next = nullptr;
                syntheticBlock = scope;
            }
            if (scope->grfpn & fpnSyntheticNode) {
                Output::Print(L" synthetic");
                if (scope->nop == knopBlock)
                    syntheticBlock = scope;
            }
            Output::Print(L" (%d-%d)", scope->ichMin, scope->ichLim);
            if (next) Output::Print(L", ");
            scope = next;
        }
        Output::Print(L"\n");
        if (syntheticBlock || firstOnly) {
            PrintScopesWIndent(syntheticBlock, indentAmt + INDENT_SIZE);
        }
    }
}

void PrintPnodeWIndent(ParseNode *pnode,int indentAmt) {
    if (pnode==NULL)
        return;

    Output::Print(L"[%d, %d): ", pnode->ichMin, pnode->ichLim);
    switch (pnode->nop) {
        //PTNODE(knopName       , "name"        ,None    ,Pid  ,fnopLeaf)
  case knopName:
      Indent(indentAmt);
      if (pnode->sxPid.pid!=NULL) {
        Output::Print(L"id: %s\n",pnode->sxPid.pid->Psz());
      }
      else {
        Output::Print(L"name node\n");
      }
      break;
      //PTNODE(knopInt        , "int const"    ,None    ,Int  ,fnopLeaf|fnopConst)
  case knopInt:
      Indent(indentAmt);
      Output::Print(L"%d\n",pnode->sxInt.lw);
      break;
      //PTNODE(knopFlt        , "flt const"    ,None    ,Flt  ,fnopLeaf|fnopConst)
  case knopFlt:
      Indent(indentAmt);
      Output::Print(L"%lf\n",pnode->sxFlt.dbl);
      break;
      //PTNODE(knopStr        , "str const"    ,None    ,Pid  ,fnopLeaf|fnopConst)
  case knopStr:
      Indent(indentAmt);
      Output::Print(L"\"%s\"\n",pnode->sxPid.pid->Psz());
      break;
      //PTNODE(knopRegExp     , "reg expr"    ,None    ,Pid  ,fnopLeaf|fnopConst)
  case knopRegExp:
      Indent(indentAmt);
      Output::Print(L"/%x/\n",pnode->sxPid.regexPattern);
      break;
      //PTNODE(knopThis       , "this"        ,None    ,None ,fnopLeaf)
  case knopThis:
      Indent(indentAmt);
      Output::Print(L"this\n");
      break;
      //PTNODE(knopSuper      , "super"       ,None    ,None ,fnopLeaf)
  case knopSuper:
      Indent(indentAmt);
      Output::Print(L"super\n");
      break;
      //PTNODE(knopNull       , "null"        ,Null    ,None ,fnopLeaf)
  case knopNull:
      Indent(indentAmt);
      Output::Print(L"null\n");
      break;
      //PTNODE(knopFalse      , "false"        ,False   ,None ,fnopLeaf)
  case knopFalse:
      Indent(indentAmt);
      Output::Print(L"false\n");
      break;
      //PTNODE(knopTrue       , "true"        ,True    ,None ,fnopLeaf)
  case knopTrue:
      Indent(indentAmt);
      Output::Print(L"true\n");
      break;
      //PTNODE(knopEmpty      , "empty"        ,Empty   ,None ,fnopLeaf)
  case knopEmpty:
      Indent(indentAmt);
      Output::Print(L"empty\n");
      break;
      // Unary operators.
      //PTNODE(knopNot        , "~"            ,BitNot  ,Uni  ,fnopUni)
  case knopNot:
      Indent(indentAmt);
      Output::Print(L"~\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopNeg        , "unary -"    ,Neg     ,Uni  ,fnopUni)
  case knopNeg:
      Indent(indentAmt);
      Output::Print(L"U-\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopPos        , "unary +"    ,Pos     ,Uni  ,fnopUni)
  case knopPos:
      Indent(indentAmt);
      Output::Print(L"U+\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopLogNot     , "!"            ,LogNot  ,Uni  ,fnopUni)
  case knopLogNot:
      Indent(indentAmt);
      Output::Print(L"!\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopEllipsis     , "..."       ,Spread  ,Uni    , fnopUni)
  case knopEllipsis:
      Indent(indentAmt);
      Output::Print(L"...<expr>\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopIncPost    , "++ post"    ,Inc     ,Uni  ,fnopUni|fnopAsg)
  case knopIncPost:
      Indent(indentAmt);
      Output::Print(L"<expr>++\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopDecPost    , "-- post"    ,Dec     ,Uni  ,fnopUni|fnopAsg)
  case knopDecPost:
      Indent(indentAmt);
      Output::Print(L"<expr>--\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopIncPre     , "++ pre"    ,Inc     ,Uni  ,fnopUni|fnopAsg)
  case knopIncPre:
      Indent(indentAmt);
      Output::Print(L"++<expr>\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopDecPre     , "-- pre"    ,Dec     ,Uni  ,fnopUni|fnopAsg)
  case knopDecPre:
      Indent(indentAmt);
      Output::Print(L"--<expr>\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopTypeof     , "typeof"    ,None    ,Uni  ,fnopUni)
  case knopTypeof:
      Indent(indentAmt);
      Output::Print(L"typeof\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopVoid       , "void"        ,Void    ,Uni  ,fnopUni)
  case knopVoid:
      Indent(indentAmt);
      Output::Print(L"void\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopDelete     , "delete"    ,None    ,Uni  ,fnopUni)
  case knopDelete:
      Indent(indentAmt);
      Output::Print(L"delete\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopArray      , "arr cnst"    ,None    ,Uni  ,fnopUni)
  case knopArray:
      Indent(indentAmt);
      Output::Print(L"Array Literal\n");
      PrintPnodeListWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopObject     , "obj cnst"    ,None    ,Uni  ,fnopUni)
  case knopObject:
      Indent(indentAmt);
      Output::Print(L"Object Literal\n");
      PrintPnodeListWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      // Binary and Ternary Operators
      //PTNODE(knopAdd        , "+"            ,Add     ,Bin  ,fnopBin)
  case knopAdd:
      Indent(indentAmt);
      Output::Print(L"+\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopSub        , "-"            ,Sub     ,Bin  ,fnopBin)
  case knopSub:
      Indent(indentAmt);
      Output::Print(L"-\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopMul        , "*"            ,Mul     ,Bin  ,fnopBin)
  case knopMul:
      Indent(indentAmt);
      Output::Print(L"*\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopDiv        , "/"            ,Div     ,Bin  ,fnopBin)
  case knopDiv:
      Indent(indentAmt);
      Output::Print(L"/\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopMod        , "%"            ,Mod     ,Bin  ,fnopBin)
  case knopMod:
      Indent(indentAmt);
      Output::Print(L"%\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopOr         , "|"            ,BitOr   ,Bin  ,fnopBin)
  case knopOr:
      Indent(indentAmt);
      Output::Print(L"|\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopXor        , "^"            ,BitXor  ,Bin  ,fnopBin)
  case knopXor:
      Indent(indentAmt);
      Output::Print(L"^\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAnd        , "&"            ,BitAnd  ,Bin  ,fnopBin)
  case knopAnd:
      Indent(indentAmt);
      Output::Print(L"&\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopEq         , "=="        ,EQ      ,Bin  ,fnopBin|fnopRel)
  case knopEq:
      Indent(indentAmt);
      Output::Print(L"==\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopNe         , "!="        ,NE      ,Bin  ,fnopBin|fnopRel)
  case knopNe:
      Indent(indentAmt);
      Output::Print(L"!=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopLt         , "<"            ,LT      ,Bin  ,fnopBin|fnopRel)
  case knopLt:
      Indent(indentAmt);
      Output::Print(L"<\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopLe         , "<="        ,LE      ,Bin  ,fnopBin|fnopRel)
  case knopLe:
      Indent(indentAmt);
      Output::Print(L"<=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopGe         , ">="        ,GE      ,Bin  ,fnopBin|fnopRel)
  case knopGe:
      Indent(indentAmt);
      Output::Print(L">=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopGt         , ">"            ,GT      ,Bin  ,fnopBin|fnopRel)
  case knopGt:
      Indent(indentAmt);
      Output::Print(L">\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopCall       , "()"        ,None    ,Bin  ,fnopBin)
  case knopCall:
      Indent(indentAmt);
      Output::Print(L"Call\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeListWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopDot        , "."            ,None    ,Bin  ,fnopBin)
  case knopDot:
      Indent(indentAmt);
      Output::Print(L".\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsg        , "="            ,None    ,Bin  ,fnopBin|fnopAsg)
  case knopAsg:
      Indent(indentAmt);
      Output::Print(L"=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopInstOf     , "instanceof",InstOf  ,Bin  ,fnopBin|fnopRel)
  case knopInstOf:
      Indent(indentAmt);
      Output::Print(L"instanceof\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopIn         , "in"        ,In      ,Bin  ,fnopBin|fnopRel)
  case knopIn:
      Indent(indentAmt);
      Output::Print(L"in\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopEqv        , "==="        ,Eqv     ,Bin  ,fnopBin|fnopRel)
  case knopEqv:
      Indent(indentAmt);
      Output::Print(L"===\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopNEqv       , "!=="        ,NEqv    ,Bin  ,fnopBin|fnopRel)
  case knopNEqv:
      Indent(indentAmt);
      Output::Print(L"!==\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopComma      , ","            ,None    ,Bin  ,fnopBin)
  case knopComma:
      Indent(indentAmt);
      Output::Print(L",\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopLogOr      , "||"        ,None    ,Bin  ,fnopBin)
  case knopLogOr:
      Indent(indentAmt);
      Output::Print(L"||\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopLogAnd     , "&&"        ,None    ,Bin  ,fnopBin)
  case knopLogAnd:
      Indent(indentAmt);
      Output::Print(L"&&\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopLsh        , "<<"        ,Lsh     ,Bin  ,fnopBin)
  case knopLsh:
      Indent(indentAmt);
      Output::Print(L"<<\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopRsh        , ">>"        ,Rsh     ,Bin  ,fnopBin)
  case knopRsh:
      Indent(indentAmt);
      Output::Print(L">>\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopRs2        , ">>>"        ,Rs2     ,Bin  ,fnopBin)
  case knopRs2:
      Indent(indentAmt);
      Output::Print(L">>>\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopNew        , "new"        ,None    ,Bin  ,fnopBin)
  case knopNew:
      Indent(indentAmt);
      Output::Print(L"new\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeListWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopIndex      , "[]"        ,None    ,Bin  ,fnopBin)
  case knopIndex:
      Indent(indentAmt);
      Output::Print(L"[]\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeListWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopQmark      , "?"            ,None    ,Tri  ,fnopBin)
  case knopQmark:
      Indent(indentAmt);
      Output::Print(L"?:\n");
      PrintPnodeWIndent(pnode->sxTri.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxTri.pnode2,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxTri.pnode3,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgAdd     , "+="        ,Add     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgAdd:
      Indent(indentAmt);
      Output::Print(L"+=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgSub     , "-="        ,Sub     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgSub:
      Indent(indentAmt);
      Output::Print(L"-=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgMul     , "*="        ,Mul     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgMul:
      Indent(indentAmt);
      Output::Print(L"*=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgDiv     , "/="        ,Div     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgDiv:
      Indent(indentAmt);
      Output::Print(L"/=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgMod     , "%="        ,Mod     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgMod:
      Indent(indentAmt);
      Output::Print(L"%=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgAnd     , "&="        ,BitAnd  ,Bin  ,fnopBin|fnopAsg)
  case knopAsgAnd:
      Indent(indentAmt);
      Output::Print(L"&=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgXor     , "^="        ,BitXor  ,Bin  ,fnopBin|fnopAsg)
  case knopAsgXor:
      Indent(indentAmt);
      Output::Print(L"^=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgOr      , "|="        ,BitOr   ,Bin  ,fnopBin|fnopAsg)
  case knopAsgOr:
      Indent(indentAmt);
      Output::Print(L"|=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgLsh     , "<<="        ,Lsh     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgLsh:
      Indent(indentAmt);
      Output::Print(L"<<=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgRsh     , ">>="        ,Rsh     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgRsh:
      Indent(indentAmt);
      Output::Print(L">>=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopAsgRs2     , ">>>="        ,Rs2     ,Bin  ,fnopBin|fnopAsg)
  case knopAsgRs2:
      Indent(indentAmt);
      Output::Print(L">>>=\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopScope      , "::"        ,None    ,Bin  ,fnopBin)
  case knopScope:
      Indent(indentAmt);
      Output::Print(L"::\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      // TODO: print associated identifier
      break;
      //PTNODE(knopMember     , ":"            ,None    ,Bin  ,fnopBin)
  case knopMember:
  case knopMemberShort:
      Indent(indentAmt);
      Output::Print(L":\n");
      PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxBin.pnode2,indentAmt+INDENT_SIZE);
      break;
      // General nodes.
      //PTNODE(knopList       , "<list>"    ,None    ,Bin  ,fnopNone)
  case knopList:
      Indent(indentAmt);
      Output::Print(L"List\n");
      PrintPnodeListWIndent(pnode,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopVarDecl    , "varDcl"    ,None    ,Var  ,fnopNone)
  case knopVarDecl:
      Indent(indentAmt);
      Output::Print(L"var %s\n",pnode->sxVar.pid->Psz());
      if (pnode->sxVar.pnodeInit!=NULL)
          PrintPnodeWIndent(pnode->sxVar.pnodeInit,indentAmt+INDENT_SIZE);
      break;
  case knopConstDecl:
      Indent(indentAmt);
      Output::Print(L"const %s\n",pnode->sxVar.pid->Psz());
      if (pnode->sxVar.pnodeInit!=NULL)
          PrintPnodeWIndent(pnode->sxVar.pnodeInit,indentAmt+INDENT_SIZE);
      break;
  case knopLetDecl:
      Indent(indentAmt);
      Output::Print(L"let %s\n",pnode->sxVar.pid->Psz());
      if (pnode->sxVar.pnodeInit!=NULL)
          PrintPnodeWIndent(pnode->sxVar.pnodeInit,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopFncDecl    , "fncDcl"    ,None    ,Fnc  ,fnopLeaf)
  case knopFncDecl:
      Indent(indentAmt);
      if (pnode->sxFnc.pid!=NULL)
      {
          Output::Print(L"fn decl %d nested %d name %s (%d-%d)\n",pnode->sxFnc.IsDeclaration(),pnode->sxFnc.IsNested(),
              pnode->sxFnc.pid->Psz(), pnode->ichMin, pnode->ichLim);
      }
      else
      {
          Output::Print(L"fn decl %d nested %d anonymous (%d-%d)\n",pnode->sxFnc.IsDeclaration(),pnode->sxFnc.IsNested(),pnode->ichMin,pnode->ichLim);
      }
      PrintScopesWIndent(pnode, indentAmt+INDENT_SIZE);
      PrintPnodeListWIndent(pnode->sxFnc.pnodeArgs, indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxFnc.pnodeRest, indentAmt + INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxFnc.pnodeBody, indentAmt + INDENT_SIZE);
      break;
      //PTNODE(knopProg       , "program"    ,None    ,Fnc  ,fnopNone)
  case knopProg:
      Indent(indentAmt);
      Output::Print(L"program\n");
      PrintScopesWIndent(pnode, indentAmt+INDENT_SIZE);
      PrintPnodeListWIndent(pnode->sxFnc.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopEndCode    , "<endcode>"    ,None    ,None ,fnopNone)
  case knopEndCode:
      Indent(indentAmt);
      Output::Print(L"<endcode>\n");
      break;
      //PTNODE(knopDebugger   , "debugger"    ,None    ,None ,fnopNone)
  case knopDebugger:
      Indent(indentAmt);
      Output::Print(L"<debugger>\n");
      break;
      //PTNODE(knopFor        , "for"        ,None    ,For  ,fnopBreak|fnopContinue)
  case knopFor:
      Indent(indentAmt);
      Output::Print(L"for\n");
      PrintScopesWIndent(pnode, indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxFor.pnodeInit,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxFor.pnodeCond,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxFor.pnodeIncr,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxFor.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopIf         , "if"        ,None    ,If   ,fnopNone)
  case knopIf:
      Indent(indentAmt);
      Output::Print(L"if\n");
      PrintPnodeWIndent(pnode->sxIf.pnodeCond,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxIf.pnodeTrue,indentAmt+INDENT_SIZE);
      if (pnode->sxIf.pnodeFalse!=NULL)
          PrintPnodeWIndent(pnode->sxIf.pnodeFalse,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopWhile      , "while"        ,None    ,While,fnopBreak|fnopContinue)
  case knopWhile:
      Indent(indentAmt);
      Output::Print(L"while\n");
      PrintPnodeWIndent(pnode->sxWhile.pnodeCond,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxWhile.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopDoWhile    , "do-while"    ,None    ,While,fnopBreak|fnopContinue)
  case knopDoWhile:
      Indent(indentAmt);
      Output::Print(L"do\n");
      PrintPnodeWIndent(pnode->sxWhile.pnodeCond,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxWhile.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopForIn      , "for in"    ,None    ,ForIn,fnopBreak|fnopContinue|fnopCleanup)
  case knopForIn:
      Indent(indentAmt);
      Output::Print(L"forIn\n");
      PrintScopesWIndent(pnode, indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxForInOrForOf.pnodeLval,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxForInOrForOf.pnodeObj,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxForInOrForOf.pnodeBody,indentAmt+INDENT_SIZE);
      break;
  case knopForOf:
      Indent(indentAmt);
      Output::Print(L"forOf\n");
      PrintScopesWIndent(pnode, indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxForInOrForOf.pnodeLval,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxForInOrForOf.pnodeObj,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxForInOrForOf.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopReturn     , "return"    ,None    ,Uni  ,fnopNone)
  case knopReturn:
      Indent(indentAmt);
      Output::Print(L"return\n");
      if (pnode->sxReturn.pnodeExpr!=NULL)
          PrintPnodeWIndent(pnode->sxReturn.pnodeExpr,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopBlock      , "{}"        ,None    ,Block,fnopNone)
  case knopBlock:
      Indent(indentAmt);
      Output::Print(L"block ");
      if (pnode->grfpn & fpnSyntheticNode)
          Output::Print(L"synthetic ");
      Output::Print(L"(%d-%d)\n",pnode->ichMin,pnode->ichLim);
      PrintScopesWIndent(pnode, indentAmt+INDENT_SIZE);
      if (pnode->sxBlock.pnodeStmt!=NULL)
          PrintPnodeWIndent(pnode->sxBlock.pnodeStmt,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopWith       , "with"        ,None    ,With ,fnopCleanup)
  case knopWith:
      Indent(indentAmt);
      Output::Print(L"with (%d-%d)\n", pnode->ichMin,pnode->ichLim);
      PrintScopesWIndent(pnode, indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxWith.pnodeObj,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxWith.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopBreak      , "break"        ,None    ,Jump ,fnopNone)
  case knopBreak:
      Indent(indentAmt);
      Output::Print(L"break\n");
      // TODO: some representation of target
      break;
      //PTNODE(knopContinue   , "continue"    ,None    ,Jump ,fnopNone)
  case knopContinue:
      Indent(indentAmt);
      Output::Print(L"continue\n");
      // TODO: some representation of target
      break;
      //PTNODE(knopLabel      , "label"        ,None    ,Label,fnopNone)
  case knopLabel:
      Indent(indentAmt);
      Output::Print(L"label %s",pnode->sxLabel.pid->Psz());
      // TODO: print labeled statement
      break;
      //PTNODE(knopSwitch     , "switch"    ,None    ,Switch,fnopBreak)
  case knopSwitch:
      Indent(indentAmt);
      Output::Print(L"switch\n");
      PrintScopesWIndent(pnode, indentAmt+INDENT_SIZE);
      for (ParseNode *pnodeT = pnode->sxSwitch.pnodeCases; NULL != pnodeT;pnodeT = pnodeT->sxCase.pnodeNext) {
          PrintPnodeWIndent(pnodeT,indentAmt+2);
      }
      break;
      //PTNODE(knopCase       , "case"        ,None    ,Case ,fnopNone)
  case knopCase:
      Indent(indentAmt);
      Output::Print(L"case\n");
      PrintPnodeWIndent(pnode->sxCase.pnodeExpr,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxCase.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopTryFinally,"try-finally",None,TryFinally,fnopCleanup)
  case knopTryFinally:
      PrintPnodeWIndent(pnode->sxTryFinally.pnodeTry,indentAmt);
      PrintPnodeWIndent(pnode->sxTryFinally.pnodeFinally,indentAmt);
      break;
  case knopFinally:
      Indent(indentAmt);
      Output::Print(L"finally\n");
      PrintPnodeWIndent(pnode->sxFinally.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopCatch      , "catch"     ,None    ,Catch,fnopNone)
  case knopCatch:
      Indent(indentAmt);
      Output::Print(L"catch (%d-%d)\n", pnode->ichMin,pnode->ichLim);
      PrintScopesWIndent(pnode, indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxCatch.pnodeParam,indentAmt+INDENT_SIZE);
//      if (pnode->sxCatch.pnodeGuard!=NULL)
//          PrintPnodeWIndent(pnode->sxCatch.pnodeGuard,indentAmt+INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxCatch.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopTryCatch      , "try-catch" ,None    ,TryCatch  ,fnopCleanup)
  case knopTryCatch:
      PrintPnodeWIndent(pnode->sxTryCatch.pnodeTry,indentAmt);
      PrintPnodeWIndent(pnode->sxTryCatch.pnodeCatch,indentAmt);
      break;
      //PTNODE(knopTry        , "try"       ,None    ,Try  ,fnopCleanup)
  case knopTry:
      Indent(indentAmt);
      Output::Print(L"try\n");
      PrintPnodeWIndent(pnode->sxTry.pnodeBody,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopThrow      , "throw"     ,None    ,Uni  ,fnopNone)
  case knopThrow:
      Indent(indentAmt);
      Output::Print(L"throw\n");
      PrintPnodeWIndent(pnode->sxUni.pnode1,indentAmt+INDENT_SIZE);
      break;
      //PTNODE(knopClassDecl, "classDecl", None , Class, fnopLeaf)
  case knopClassDecl:
      Indent(indentAmt);
      Output::Print(L"class %s", pnode->sxClass.pnodeName->sxVar.pid->Psz());
      if (pnode->sxClass.pnodeExtends != nullptr)
      {
          Output::Print(L" extends ");
          PrintPnodeWIndent(pnode->sxClass.pnodeExtends, 0);
      }
      else {
          Output::Print(L"\n");
      }

      PrintPnodeWIndent(pnode->sxClass.pnodeConstructor,   indentAmt + INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxClass.pnodeMembers,       indentAmt + INDENT_SIZE);
      PrintPnodeWIndent(pnode->sxClass.pnodeStaticMembers, indentAmt + INDENT_SIZE);
      break;
  case knopStrTemplate:
      Indent(indentAmt);
      Output::Print(L"string template\n");
      PrintPnodeListWIndent(pnode->sxStrTemplate.pnodeSubstitutionExpressions, indentAmt + INDENT_SIZE);
      break;
  case knopYieldStar:
      Indent(indentAmt);
      Output::Print(L"yield*\n");
      PrintPnodeListWIndent(pnode->sxUni.pnode1, indentAmt + INDENT_SIZE);
      break;
  case knopYield:
  case knopYieldLeaf:
      Indent(indentAmt);
      Output::Print(L"yield\n");
      PrintPnodeListWIndent(pnode->sxUni.pnode1, indentAmt + INDENT_SIZE);
      break;
  default:
      Output::Print(L"unhandled pnode op %d\n",pnode->nop);
      break;
    }
}

void PrintPnodeListWIndent(ParseNode *pnode,int indentAmt) {
    if (pnode!=NULL) {
        while(pnode->nop==knopList) {
            PrintPnodeWIndent(pnode->sxBin.pnode1,indentAmt);
            pnode = pnode->sxBin.pnode2;
        }
        PrintPnodeWIndent(pnode,indentAmt);
    }
}

void PrintPnode(ParseNode *pnode) {
    PrintPnodeWIndent(pnode,0);
}

void ParseNode::Dump()
{
    switch(nop)
    {
    case knopFncDecl:
    case knopProg:
        LPCOLESTR name = Js::Constants::AnonymousFunction;
        if(this->sxFnc.pnodeNames)
        {
            name = this->sxFnc.pnodeNames->sxVar.pid->Psz();
        }

        Output::Print(L"%s (%d) [%d, %d]:\n", name, this->sxFnc.functionId, this->sxFnc.lineNumber, this->sxFnc.columnNumber);
        Output::Print(L"hasArguments: %s callsEval:%s childCallsEval:%s HasReferenceableBuiltInArguments:%s ArgumentsObjectEscapes:%s HasWith:%s HasThis:%s HasOnlyThis:%s \n",
            IsTrueOrFalse(this->sxFnc.HasHeapArguments()),
            IsTrueOrFalse(this->sxFnc.CallsEval()),
            IsTrueOrFalse(this->sxFnc.ChildCallsEval()),
            IsTrueOrFalse(this->sxFnc.HasReferenceableBuiltInArguments()),
            IsTrueOrFalse(this->sxFnc.GetArgumentsObjectEscapes()),
            IsTrueOrFalse(this->sxFnc.HasWithStmt()),
            IsTrueOrFalse(this->sxFnc.HasThisStmt()),
            IsTrueOrFalse(this->sxFnc.HasOnlyThisStmts()));
        if(this->sxFnc.funcInfo)
        {
            this->sxFnc.funcInfo->Dump();
        }
        break;
    }
}
#endif

#if PARSENODE_EXTENSIONS
/* Language Service Extension */

LanguageServiceExtension* Parser::GetLanguageServiceExtension()
{
    return m_languageServiceExtension;
}

LanguageServiceExtension::ExtensionData* LanguageServiceExtension::NodeExtension(ParseNode* node, bool createIfNotExists)
{
    Assert(node != nullptr);
    Assert(m_parseNodeExtensions != nullptr);

    LanguageServiceExtension::ExtensionData* extensionData = NULL;
    m_parseNodeExtensions->TryGetValue(node, &extensionData);

    if(extensionData == nullptr && createIfNotExists)
    {
        extensionData = Anew(m_alloc, LanguageServiceExtension::ExtensionData);
        m_parseNodeExtensions->Item(node, extensionData);
    }

    return extensionData;
}

void LanguageServiceExtension::SetLCurly(ParseNodePtr node, uint ichLCurly)
{
    NodeExtension(node, true)->ichLCurly = ichLCurly;
}

void LanguageServiceExtension::SetRCurly(ParseNodePtr node, uint ichRCurly)
{
    NodeExtension(node, true)->ichRCurly = ichRCurly;
}

void LanguageServiceExtension::SetLParen(ParseNodePtr node, uint ichLParen)
{
    NodeExtension(node, true)->ichLParen = ichLParen;
}

void LanguageServiceExtension::SetRParen(ParseNodePtr node, uint ichRParen)
{
    NodeExtension(node, true)->ichRParen = ichRParen;
}

void LanguageServiceExtension::SetLBrack(ParseNodePtr node, uint ichLBrack)
{
    NodeExtension(node, true)->ichLBrack = ichLBrack;
}

void LanguageServiceExtension::SetRBrack(ParseNodePtr node, uint ichRBrack)
{
    NodeExtension(node, true)->ichRBrack = ichRBrack;
}

void LanguageServiceExtension::SetWhileMin(ParseNodePtr node, uint ichWhileMin)
{
    NodeExtension(node, true)->ichWhileMin = ichWhileMin;
}

void LanguageServiceExtension::SetSwitchLim(ParseNodePtr node, uint ichSwitchLim)
{
    NodeExtension(node, true)->ichSwitchLim = ichSwitchLim;
}

void LanguageServiceExtension::SetNestedCount(ParseNodePtr node, uint nestedCount)
{
    NodeExtension(node, true)->nestedCount = nestedCount;
}

void LanguageServiceExtension::SetIdentMin(ParseNodePtr node, uint ichIdentMin)
{
    NodeExtension(node, true)->ichIdentMin = ichIdentMin;
}

void LanguageServiceExtension::SetIdentLim(ParseNodePtr node, uint ichIdentLim)
{
    NodeExtension(node, true)->ichIdentLim = ichIdentLim;
}

void LanguageServiceExtension::SetArgLim(ParseNodePtr node, uint ichArgLim)
{
    NodeExtension(node, true)->ichArgLim = ichArgLim;
}

void LanguageServiceExtension::SetTkFunctionMin(ParseNodePtr node, uint ichtkFunctionMin)
{
    NodeExtension(node, true)->ichtkFunctionMin = ichtkFunctionMin;
}

#define ExtensionField(extensionData, field) \
    (extensionData != nullptr ? extensionData->field : 0)

uint LanguageServiceExtension::LCurly(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichLCurly);
}

uint LanguageServiceExtension::RCurly(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichRCurly);
}

uint LanguageServiceExtension::LParen(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichLParen);
}

uint LanguageServiceExtension::RParen(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichRParen);
}

uint LanguageServiceExtension::LBrack(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichLBrack);
}

uint LanguageServiceExtension::RBrack(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichRBrack);
}

uint LanguageServiceExtension::WhileMin(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichWhileMin);
}

uint LanguageServiceExtension::SwitchLim(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichSwitchLim);
}

uint LanguageServiceExtension::NestedCount(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), nestedCount);
}

uint LanguageServiceExtension::IdentMin(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichIdentMin);
}

uint LanguageServiceExtension::IdentLim(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichIdentLim);
}

uint LanguageServiceExtension::ArgLim(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichArgLim);
}

uint LanguageServiceExtension::TkFunctionMin(ParseNodePtr node)
{
    return ExtensionField(NodeExtension(node), ichtkFunctionMin);
}

void  LanguageServiceExtension::Clear()
{
    if (NULL != m_parseNodeExtensions)
    {
        m_parseNodeExtensions->Clear();
        m_parseNodeExtensions = NULL;
    }

    if (NULL != m_alloc)
    {
        m_alloc->Clear();
        m_alloc->~ArenaAllocator();
        m_alloc = NULL;
    }
}

HRESULT LanguageServiceExtension::Init(ParseNodeAllocator* nodeAllocator, PageAllocator * pageAllocator)
{
    typedef JsUtil::BaseDictionary<ParseNode*, LanguageServiceExtension::ExtensionData*, ArenaAllocator, PrimeSizePolicy> ParseNodeToExtMap;
    typedef JsUtil::BaseDictionary<ParseNode*, wchar_t*, ArenaAllocator, PrimeSizePolicy> ParseNodeToStringMap;
    typedef JsUtil::BaseDictionary<ParseNode*, int, ArenaAllocator, PrimeSizePolicy> ParseNodeToIntMap;

    void* allocBuffer = nodeAllocator->Alloc(sizeof(ArenaAllocator));
    if (NULL == allocBuffer)
        return ERRnoMemory;
    m_alloc = new (allocBuffer) ArenaAllocator (L"ls:NodeExtension", pageAllocator, Js::Throw::OutOfMemory);
    m_parseNodeExtensions = Anew(m_alloc,ParseNodeToExtMap, m_alloc);
    m_labels = Anew(m_alloc, ParseNodeToStringMap, m_alloc);
    m_parentheses = Anew(m_alloc, ParseNodeToIntMap, m_alloc);
    m_completionRanges = JsUtil::List<CompletionRange*, ArenaAllocator>::New(m_alloc);
    return S_OK;
}

void LanguageServiceExtension::AddLabel(ParseNodePtr node, ParseNodePtr pnodeLabel)
{
    Assert(node != nullptr);
    Assert(m_labels != nullptr);

    if (pnodeLabel != nullptr)
    {
        // create a copy of the label name
        Assert(pnodeLabel->nop == knopLabel && pnodeLabel->sxLabel.pid != nullptr);
        LPCWSTR labelName = pnodeLabel->sxLabel.pid->Psz();
        int len = pnodeLabel->sxLabel.pid->Cch();
        if (len > 0)
        {
            m_labels->Item(node, (wchar_t*)Js::InternalString::New(m_alloc, labelName, len)->GetBuffer());
        }
    }
}

LPCWSTR LanguageServiceExtension::GetLabel(ParseNode* node)
{
    Assert(node != nullptr);
    Assert(m_labels != nullptr);

    wchar_t* label = nullptr;
    m_labels->TryGetValue(node, &label);
    return label;
}

void LanguageServiceExtension::IncrementParenthesesCount(ParseNodePtr node)
{
    Assert(node != nullptr);
    Assert(m_parentheses != nullptr);

    int count = m_parentheses->Lookup(node, 0);
    m_parentheses->Item(node, count + 1);
}

int LanguageServiceExtension::GetParenthesesCount(ParseNode* node)
{
    Assert(node != nullptr);
    Assert(m_parentheses != nullptr);

    return m_parentheses->Lookup(node, 0);
}

void LanguageServiceExtension::SetCompletionRange(uint min, uint lim, CompletionRangeMode completionRangeMode)
{
    Assert(m_completionRanges != nullptr);

    // Ignore invalid ranges
    if (lim < min)
        return;

    // Almost all insertions will be in order, the exception is error ranges inside dead ranges.
    // e.g. "var. a;" a range will be added for the dot, and another for the "var a".
    int insertAt = m_completionRanges->Count();
    for (int i = m_completionRanges->Count() - 1; i >= 0; i--)
    {
        auto current = m_completionRanges->Item(i);

        if (min > current->ichLim)
        {
            // Common case, insert here.
            break;
        }

        insertAt = i;

        if (lim < current->ichMin)
        {
            // Not the correct location, keep going.
            continue;
        }
        else if (current->ichMin <= min && lim <= current->ichLim)
        {
            // Current includes the new range, no need to insert it.
            return;
        }
        else if (min <= current->ichMin && current->ichLim <= lim)
        {
           // The new range includes the current range, delete the current range and keep moving
           // backward to insert the current range
           m_completionRanges->RemoveAt(i);
           continue;
        }
        else if (current->ichMin <= min && lim > current->ichLim)
        {
            // Current range overlaps with the new range, but the new range extends to the right.
            // Update the current range, and return.
            current->ichLim = lim;
            return;
        }
        else if (min <= current->ichMin && current->ichLim > lim)
        {
           // Current range overlaps with the new range, but the current range extends to the right
           // The new range can cause more than one range to coalesce. Merge the two ranges into the new range,
           // delete the existing range, and keep moving to the left.
           lim = current->ichLim;
           m_completionRanges->RemoveAt(i);
           continue;
        }
    }

    // Insert at the specified location
    if (insertAt == m_completionRanges->Count())
    {
        m_completionRanges->Add(Anew(m_alloc, CompletionRange, min, lim, completionRangeMode));
    }
    else
    {
        // Extend the list by 1
        m_completionRanges->Add(nullptr);
        // move elements to free a location to insert the new one
        for (int i = m_completionRanges->Count() - 2 ; i >= insertAt; i--)
        {
            auto currentItem = m_completionRanges->Item(i);
            m_completionRanges->Item(i + 1, currentItem);
        }
        m_completionRanges->Item(insertAt, Anew(m_alloc, CompletionRange, min, lim, completionRangeMode));
    }
}

JsUtil::List<LanguageServiceExtension::CompletionRange*, ArenaAllocator>* LanguageServiceExtension::CompletionRanges()
{
    return m_completionRanges;
}

#endif
