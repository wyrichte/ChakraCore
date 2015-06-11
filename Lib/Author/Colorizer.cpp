//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(Colorizer, L"Colorizer")

    // *** IColorizeText ***
    STDMETHODIMP Colorizer::Colorize( 
        /* [in] */ const wchar_t *text,
        /* [in] */ int length,
        /* [in] */ AuthorSourceState state,
        /* [out] */ IAuthorTokenEnumerator **result)
    {
        STDMETHOD_PREFIX;

        ValidateArg(text && result && length >= 0);

        *result = new TokenEnumerator(m_context, state, text, length);

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP Colorizer::GetStateForText( 
            /* [in] */ const wchar_t *text,
            /* [in] */ int length,
            /* [in] */ AuthorSourceState state,
            /* [out] */ AuthorSourceState *result)
    {
        IAuthorTokenEnumerator *enumerator = NULL;

        STDMETHOD_PREFIX;
        
        ValidateArg(text && result && length >= 0);

        enumerator = new TokenEnumerator(m_context, state, text, length);
        *result = AuthorSourceState(0);

        for(;;)
        {
            AuthorTokenColorInfo info;
            HRESULT hr = enumerator->Next(&info);
            if (FAILED(hr)) goto Error;
            if (info.Kind == atkEnd)
            {
                *result = info.State;
                break;
            }
        }

        STDMETHOD_POSTFIX_CLEAN_START;
        ReleasePtr(enumerator);
        STDMETHOD_POSTFIX_CLEAN_END;
    }

    STDMETHODIMP Colorizer::GetMultilineTokenKind(/* in */ AuthorSourceState state, /* out */ AuthorMultilineTokenKind *result)
    {
        STDMETHOD_PREFIX;

        ValidateArg(result);

        auto scanState = TokenEnumerator::GetScannerState(state);

        if (scanState == ScannerT::ScanState::ScanStateMultiLineComment)
            *result = AuthorMultilineTokenKind::amtkMultilineComment;
        else if (scanState == ScannerT::ScanState::ScanStateMultiLineDoubleQuoteString || scanState == ScannerT::ScanState::ScanStateMultiLineSingleQuoteString)
            *result = AuthorMultilineTokenKind::amtkMultilineString;
        else
            *result = AuthorMultilineTokenKind::amtkNone;

        STDMETHOD_POSTFIX;
    }

}