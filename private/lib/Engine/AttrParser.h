//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "activdbg.h"
#include "activaut.h"

class AttrParser : public Parser
{
public:
    AttrParser(Js::ScriptContext* scriptContext) : Parser(scriptContext) {};

    HRESULT GetTextAttribs(LPCOLESTR pszSrc, size_t cEncoded,
        SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr);
    HRESULT GetTextAttribsUTF8(LPCUTF8 pszSrc, size_t cEncoded,
        SOURCE_TEXT_ATTR *prgsta, ulong cch, DWORD dwFlags, ulong grfscr);

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
};