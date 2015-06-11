//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(TokenEnumerator, L"TokenEnumerator")

        TokenEnumerator::TokenEnumerator(Js::ScriptContext* context, AuthorSourceState state, const wchar_t *text, int length) 
        : m_previous2(tkLim), m_previous3(tkLim), m_previous4(tkLim), m_currentNumBraces(0), m_braceStackStringTemplate(nullptr), m_context(context), m_withinStringTemplate(false)
    {
        m_text = static_cast<LPOLESTR>(malloc(sizeof(wchar_t) * (length + 1)));
        if (!m_text) 
            Js::Throw::OutOfMemory();
        memcpy_s(m_text, sizeof(wchar_t) * (length + 1), text, sizeof(wchar_t) * length);
        m_text[length] = 0;
        m_hashTbl = HashTbl::Create(256, &m_err);
        m_scanner = ScannerT::Create(NULL, m_hashTbl, &m_token, &m_err, context);
        m_scanner->SetText(m_text, 0, length, 0, fscrSyntaxColor);
        m_scanner->SetScanState(GetScannerState(state));
        m_previous = state & (1 << 2) ? tkID : tkLim;
    }

    ScannerT::ScanState TokenEnumerator::GetScannerState(AuthorSourceState state)
    {
        return ScannerT::ScanState(state & 3);
    }

    void TokenEnumerator::OnDelete()
    {
        ReleasePtr(m_scanner);
        ReleasePtr(m_hashTbl);
        FreePtr(m_text);
    }

    AuthorTokenKind TokenEnumerator::TokenKind(tokens token)
    {
        switch (token)
        {
        // keywords
        case tkBREAK: return atkBreak;
        case tkCASE: return atkCase;
        case tkCATCH: return atkCatch;
        case tkCLASS: return atkClass;
        case tkCONST: return atkConst;
        case tkCONTINUE: return atkContinue;
        case tkDEBUGGER: return atkDebugger;
        case tkDEFAULT: return atkDefault;
        case tkDELETE: return atkDelete;
        case tkDO: return atkDo;
        case tkELSE: return atkElse;
        case tkENUM: return atkEnum;
        case tkEXPORT: return atkExport;
        case tkEXTENDS: return atkExtends;
        case tkFALSE: return atkFalse;
        case tkFINALLY: return atkFinally;
        case tkFOR: return atkFor;
        case tkFUNCTION: return atkFunction;
        case tkIF: return atkIf;
        case tkIMPORT: return atkImport;
        case tkIN: return atkIn;
        case tkINSTANCEOF: return atkInstanceof;
        case tkNEW: return atkNew;
        case tkNULL: return atkNull;
        case tkRETURN: return atkReturn;
        case tkSUPER: return atkSuper;
        case tkSWITCH: return atkSwitch;
        case tkTHIS: return atkThis;
        case tkTHROW: return atkThrow;
        case tkTRUE: return atkTrue;
        case tkTRY: return atkTry;
        case tkTYPEOF: return atkTypeof;
        case tkVAR: return atkVar;
        case tkVOID: return atkVoid;
        case tkWHILE: return atkWhile;
        case tkWITH: return atkWith;

        // operators
        case tkSColon: return atkSColon;
        case tkRParen: return atkRParen;
        case tkRBrack: return atkRBrack;
        case tkLCurly: return atkLCurly;
        case tkRCurly: return atkRCurly;
        case tkComma: return atkComma;
        case tkDArrow: return atkDArrow;
        case tkAsg: return atkAsg;
        case tkAsgAdd: return atkAsgAdd;
        case tkAsgSub: return atkAsgSub;
        case tkAsgMul: return atkAsgMul;
        case tkAsgDiv: return atkAsgDiv;
        case tkAsgMod: return atkAsgMod;
        case tkAsgAnd: return atkAsgAnd;
        case tkAsgXor: return atkAsgXor;
        case tkAsgOr: return atkAsgOr;
        case tkAsgLsh: return atkAsgLsh;
        case tkAsgRsh: return atkAsgRsh;
        case tkAsgRs2: return atkAsgRs2;
        case tkQMark: return atkQMark;
        case tkColon: return atkColon;
        case tkLogOr: return atkLogOr;
        case tkLogAnd: return atkLogAnd;
        case tkOr: return atkOr;
        case tkXor: return atkXor;
        case tkAnd: return atkAnd;
        case tkEQ: return atkEQ;
        case tkNE: return atkNE;
        case tkEqv: return atkEqv;
        case tkNEqv: return atkNEqv;
        case tkLT: return atkLT;
        case tkLE: return atkLE;
        case tkGT: return atkGT;
        case tkGE: return atkGE;
        case tkLsh: return atkLsh;
        case tkRsh: return atkRsh;
        case tkRs2: return atkRs2;
        case tkAdd: return atkAdd;
        case tkSub: return atkSub;
        case tkStar: return atkStar;
        case tkDiv: return atkDiv;
        case tkPct: return atkPct;
        case tkTilde: return atkTilde;
        case tkBang: return atkBang;
        case tkInc: return atkInc;
        case tkDec: return atkDec;
        case tkLParen: return atkLParen;
        case tkLBrack: return atkLBrack;
        case tkDot: return atkDot;
        case tkScope: return atkScope;

        // future reserved words
        case tkIMPLEMENTS: return atkImplements;
        case tkINTERFACE: return atkInterface;
        case tkLET: return atkLet;
        case tkPACKAGE: return atkPackage;
        case tkPRIVATE: return atkPrivate;
        case tkPROTECTED: return atkProtected;
        case tkPUBLIC: return atkPublic;
        case tkSTATIC: return atkStatic;
        case tkYIELD:  return atkYield;
        
        // pseudo-keywords
        case tkOF: return atkOf;

        case tkABSTRACT:
        case tkASSERT:
        case tkBOOLEAN:
        case tkBYTE:
        case tkCHAR:
        case tkDECIMAL:
        case tkDOUBLE:
        case tkFINAL:
        case tkFLOAT:
        case tkGET:
        case tkGOTO:
        case tkINT:
        case tkINTERNAL:
        case tkINVARIANT:
        case tkLONG:
        case tkNAMESPACE:
        case tkNATIVE:
        case tkREQUIRE:
        case tkSBYTE:
        case tkSET:
        case tkSHORT:
        case tkSYNCHRONIZED:
        case tkTHROWS:
        case tkTRANSIENT:
        case tkUINT:
        case tkULONG:
        case tkUSE:
        case tkUSHORT:
        case tkVOLATILE:
            return atkIdentifier;

        // string template tokens
        case tkStrTmplBasic:
        case tkStrTmplBegin:
        case tkStrTmplMid:
        case tkStrTmplEnd:
            return atkStrTemplate;

        // literals
        case tkFltCon:
        case tkIntCon: return atkNumber;
        case tkStrCon: return atkString;
        case tkRegExp: return atkRegexp;

        case tkScanError: return atkScanError;
        case tkComment: return atkComment;
        case tkEOF: return atkEnd;
        case tkID: return atkIdentifier;

        default:
            return atkText;
        }
    }

    bool TokenEnumerator::IsDivPrefix(tokens token)
    {
        switch (token)
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
            return true;
        }
        return false;
    }

    void TokenEnumerator::EnsureAndPushCurrentBraceState()
    {
        if (m_braceStackStringTemplate == nullptr)
        {
            m_braceStackStringTemplate = Anew(m_context->GeneralAllocator(), JsUtil::Stack<int>, m_context->GeneralAllocator());
        }
        m_withinStringTemplate = true;
        m_braceStackStringTemplate->Push(m_currentNumBraces);
    }

    STDMETHODIMP TokenEnumerator::Next(
            /* [out] */ AuthorTokenColorInfo *result)
    {
        STDMETHOD_PREFIX;
        bool wasErrorToken = false;
        m_scanner->Scan();
        switch (m_token.tk)
        {
        case tkDiv:
        case tkAsgDiv:
            // The token might really be a regular expression. A regular expression is legal anywhere the / or /= operators
            // are not expected. To accurately determine if a / or /= is expected requires a valid syntax. Since we are not
            // really parsing we don't know so we must guess. The only tokens that can preceed a / or /= in a valid syntax Javascript
            // prefix are a simple term (tkID, tkStrCon, tkIntCon, tkFltCon, tkTHIS, tkRegExp) or a postfix operator or operator 
            // tail (tkInc, tkDec, tkRParen, tkRBrack, and tkRCurly) or a reserved word if it is immediately preceeded by a tkDot.
            // (Below tkRCurly is treated special because it is most-likely a false positive.) If the previous is a token that can 
            // preceed a / or /= then we assume it is not a regular expression. Otherwise, we assume it is and try to convert it.
            switch (m_previous)
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
                break;
            case tkRCurly:
                if (!m_scanner->FHadNewLine())
                    // If we had a new-line after a } then a / or /= is valid but most-likely a mistake.
                    break;
                // fall-through
            default:
                // Handles the "a.<reserved-word> /= 5" case.
                if (m_previous2 == tkDot && m_token.IsReservedWord())
                    break;
                
                // A tkDiv nor tkAsgDiv can not validly appear here, it must be a regular-expression
                m_scanner->RescanRegExpTokenizer();
                if (m_token.tk == tkScanError)
                    m_token.tk = m_scanner->ErrorToken();
                break;
            }
            break;
        case tkScanError:
            wasErrorToken = true;
            m_token.tk = m_scanner->ErrorToken();
            break;

        case tkLCurly:
            if (m_withinStringTemplate)
            {
                m_currentNumBraces++;
            }
            break;

        case tkRCurly:
            if (m_withinStringTemplate)
            {
                Assert(m_braceStackStringTemplate->Count() > 0);
                --m_currentNumBraces;
                Assert(m_currentNumBraces >= 0);
                if (m_currentNumBraces == 0) // balanced
                {
                    m_scanner->SetScanState(ScannerT::ScanState::ScanStateStringTemplateMiddleOrEnd);
                }
            }
            break;

        case tkStrTmplEnd:
            {
                m_currentNumBraces = m_braceStackStringTemplate->Pop();
                m_withinStringTemplate = (m_braceStackStringTemplate->Count() > 0);
            }
            break;

        case tkStrTmplBegin:
            {
                EnsureAndPushCurrentBraceState();
                m_currentNumBraces = 1; // Accounting a brace in '${'
            }
            break;
        case tkStrTmplMid:
            {
                m_currentNumBraces = 1; // Accounting a brace in '${'
            }
            break;

        case tkOF:
            // if the token stream has not produced either of these two patterns
            //   tkFOR tkLParen tkID tkOF
            //   tkFOR tkLParen tkVAR|tkLET|tkCONST tkID tkOF
            // then convert the token into a plain tkID.  Otherwise, leave it as
            // tkOF so that it is colored like a keyword.
            if (!(m_previous == tkID &&
                ((m_previous2 == tkLParen && m_previous3 == tkFOR) ||
                ((m_previous2 == tkVAR || m_previous2 == tkLET || m_previous2 == tkCONST) && m_previous3 == tkLParen && m_previous4 == tkFOR))))
            {
                m_token.tk = tkID;
            }
            break;
        default:
            // Handles colorization of the reserved word in "a.<reserved-word>"
            if (m_token.IsReservedWord() && m_previous == tkDot)
                m_token.tk = tkID;
            break;
        }
        if (m_token.tk != tkComment)
        {
            m_previous4 = m_previous3;
            m_previous3 = m_previous2;
            m_previous2 = m_previous;
            m_previous = m_token.tk;
        }

        result->StartPosition = m_scanner->IchMinTok();
        result->EndPosition = m_scanner->IchLimTok();
        if (m_token.tk == tkStrTmplMid || m_token.tk == tkStrTmplBegin)
        {
            if (!wasErrorToken)
            {
                result->EndPosition -= 2; // we are removing '${'
            }
            Assert(result->EndPosition >= result->StartPosition);
        }
        result->State = AuthorSourceState(m_scanner->GetScanState() | (m_token.tk == tkEOF && IsDivPrefix(m_previous2) ? 1 << 2 : 0));
        result->Kind = TokenKind(m_token.tk);
        
        STDMETHOD_POSTFIX;
    }

}
