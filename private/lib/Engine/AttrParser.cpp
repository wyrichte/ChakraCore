//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"
#include "AttrParser.h"

/***************************************************************************
Get colorization information for the source.
***************************************************************************/
#if DEBUG
BOOL  g_fMapAttrib = FALSE;  // Set to true to get good colors for DepScan
DWORD g_dwTextAttribFlags = (DWORD)-1; // Set to force a particular set of flags.
#endif // DEBUG

HRESULT AttrParser::GetTextAttribs(LPCOLESTR pszSrc, size_t cEncoded,
    SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr)
{
    return GetTextAttribsImpl<NullTerminatedUnicodeEncodingPolicy>(pszSrc, cEncoded, prgsta, cch, dwFlags, grfscr);
}

HRESULT AttrParser::GetTextAttribsUTF8(LPCUTF8 pszSrc, size_t cEncoded,
    SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr)
{
    return GetTextAttribsImpl<NullTerminatedUTF8EncodingPolicy>(pszSrc, cEncoded, prgsta, cch, dwFlags, grfscr);
}

template <typename EncodingPolicy>
HRESULT AttrParser::GetTextAttribsImpl(typename EncodingPolicy::EncodedCharPtr pszSrc, size_t cEncoded,
    SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr)
{
    AssertArrMemR(pszSrc, cch);
    AssertArrMem(prgsta, cch);

    EncodingPolicy::EncodedChar*    psz = NULL;

    if (NULL == pszSrc || NULL == prgsta || 0 == cch)
        return NOERROR;

    // this avoids an overflow such as when cch == (int)-1
    if (cch > INT_MAX)
        return E_FAIL;

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

    DebugOnly(m_err.fInited = TRUE; )

    Scanner<EncodingPolicy> *scanner = null;
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
    catch (ParseExceptionObject& e)
    {
        m_err.m_hr = e.GetError();
    }
    RELEASEPTR(scanner);
    FREEPTR(psz);
    return m_err.m_hr;
}

template <typename EncodingPolicy>
void AttrParser::GetTextAttribsImpl(Scanner< EncodingPolicy > *scanner, typename EncodingPolicy::EncodedCharPtr pstr, DWORD dwFlags,
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
void AttrParser::GetNormalTextAttribs(Scanner< EncodingPolicy > *scanner, typename EncodingPolicy::EncodedCharPtr pstr, DWORD dwFlags,
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
            switch (prevtk)
            {
            case tkID: case tkStrCon: case tkIntCon: case tkFltCon:
            case tkRegExp: case tkTHIS: case tkSUPER: case tkInc: case tkDec:
            case tkRParen: case tkRBrack: case tkRCurly:
            case tkTRUE: case tkFALSE:
                sta = SOURCETEXT_ATTR_OPERATOR;
                break;

            default:
                if (scanner->RescanRegExpNoAST() != tkScanError)
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
void AttrParser::GetDepScanTextAttribs(Scanner< EncodingPolicy >* scanner, typename EncodingPolicy::EncodedCharPtr pstr, DWORD dwFlags,
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
void AttrParser::SetHumanTextForCurrentToken(Scanner< EncodingPolicy >* scanner, typename EncodingPolicy::EncodedCharPtr pstr, SOURCE_TEXT_ATTR *prgsta)
{
    // Scan the tokens gathering attrib information.
    charcount_t istaMin, istaLim;
    size_t iuMin, iuLim;
    switch (m_token.tk)
    {
    case tkComment:
        // The JS comment can be a single line comment or a multiline
        // comment. The second character of the comment will determine.
        istaMin = scanner->IchMinTok() + 2; // Both types start with two char
        istaLim = scanner->IchLimTok();
        iuMin = scanner->IecpMinTok() + 2;
        iuLim = scanner->IecpLimTok();
        Assert('/' == pstr[iuMin - 2]);
        if ('*' == pstr[iuMin - 1])
        {
            // Multiline comment. This may not be terminated - if for
            // instance the comment ended at an EOF. We adjust the
            // end of the token only if the end comment matches.
            if ('/' == pstr[iuLim - 1] && '*' == pstr[iuLim - 2])
                istaLim -= 2;
        }
        else
        {
            // A single line comment.
            Assert('/' == pstr[iuMin - 1]);
        }
        break;

    case tkStrCon:
        // Strip the strings quotes.
        istaMin = m_pscan->IchMinTok() + 1;
        istaLim = m_pscan->IchLimTok() - 1;
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