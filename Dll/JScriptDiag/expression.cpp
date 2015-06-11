//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"

namespace JsDiag
{
    // escape, comment, operators
    static PWSTR UNSUPPORTED_CHARS = L"\\/+-*?:{};()=|&^%";

    SimpleExpressionEvaluator::SimpleExpressionEvaluator(
        InspectionContext* context, _In_ IJsDebugPropertyInternal* root) :        
        m_context(context),
        m_root(root)
    {
        const RemoteJavascriptLibrary& lib = context->GetJavascriptLibrary();
        InitConst(context, L"undefined", lib.GetUndefined(), &m_undefined);
        InitConst(context, L"null", lib.GetNull());
        InitConst(context, L"true", lib.GetTrue());
        InitConst(context, L"false", lib.GetFalse());
    }

    bool_result SimpleExpressionEvaluator::TryEvaluate(
        const CString& expression,
        _Outptr_ IJsDebugProperty** ppResult,
        _Out_ CString& error)
    {
        m_expression = expression;
        m_expression.Trim();
        m_cur = 0;

        try
        {
            // string, escape, comment, operator
            if (m_expression.FindOneOf(UNSUPPORTED_CHARS) >= 0)
            {
                ThrowNotSupported();
            }

            CComPtr<IJsDebugPropertyInternal> result;

            if (m_expression.IsEmpty())
            {
                CheckHR(m_undefined.CopyTo(&result));
            }
            else
            {
                Scan();
                result = ParseTerm();
                CheckToken(TokenType::END);
            }

            // Clone the result property to "root" at expression instead of original hierarchy.
            if (!result->TryClone(expression, (IJsDebugPropertyInternal**)ppResult))
            {
                ThrowNotSupported();
            }
            return true;
        }
        catch(const EvaluateException& e)
        {
            error = e.message;
            return false;
        }
    }

    void SimpleExpressionEvaluator::InitConst(
        InspectionContext* context, PCWSTR name, Js::Var var, _Outptr_opt_ IJsDebugPropertyInternal** ppDebugProperty /*= nullptr*/)
    {
        CComPtr<IJsDebugPropertyInternal> prop;
        context->CreateDebugProperty(PROPERTY_INFO(var), /*parent*/nullptr, &prop);
        m_constants[name] = prop;
        if (ppDebugProperty)
        {
            CheckHR(prop.CopyTo(ppDebugProperty));
        }
    }

    //
    // Scan next token into m_token, return the token type
    //
    SimpleExpressionEvaluator::TokenType SimpleExpressionEvaluator::Scan()
    {
        PCWSTR expression = m_expression;
        const int length = m_expression.GetLength();

        if (m_cur >= length)
        {
            return m_token.type = TokenType::END;
        }

        // Scan till a delimiter
        TokenType tk = TokenType::NONE;
        int i = m_cur;
        for (; i < length; i++)
        {
            const WCHAR ch = expression[i];
            switch (ch)
            {
            case L'.':
                tk = TokenType::DOT;
                break;

            case L'[':
                tk = TokenType::LBRACKET;
                break;

            case L']':
                tk = TokenType::RBRACKET;
                break;

            case L'\'':
            case L'\"':
                tk = TokenType::STRING;
                break;
            }

            if (tk != TokenType::NONE)
            {
                break;
            }
        }

        // Now check what we've got from [m_cur, i)
        if (i > m_cur)
        {
            CString text = m_expression.Mid(m_cur, i - m_cur).Trim();
            if (!text.IsEmpty())
            {
                m_cur = i; // Move cur past token

                uint32 value;
                if (NumberUtilities::TryConvertToUInt32(text.GetString(), text.GetLength(), &value))
                {
                    m_token.uintVal = value;
                    return m_token.type = TokenType::NUMBER;
                }
                else
                {
                    m_token.text = text;
                    return m_token.type = TokenType::IDENTIFIER;
                }
            }
        }

        // It's empty before i, so the delimiter at i is the new token. It may be a single char token or a string literal.
        if (tk == TokenType::STRING)
        {
            const WCHAR ch = expression[i];
            int last = m_expression.Find(ch, i + 1);
            if (last < 0)
            {
                ThrowNotSupported(); // Unclosed string literal
            }

            m_token.text = m_expression.Mid(i + 1, last - (i + 1));
            m_cur = last + 1; // move cur past closing " or '
        }
        else
        {
            m_cur = i + 1; // move cur past single char token i
        }
        return m_token.type = tk;
    }

    //
    // Validate current token is of given type, and scan next token.
    //
    void SimpleExpressionEvaluator::CheckToken(TokenType tk)
    {
        if (m_token.type != tk)
        {
            ThrowNotSupported(); // may be syntax error, but may be something we don't support
        }

        Scan();
    }

    CComPtr<IJsDebugPropertyInternal> SimpleExpressionEvaluator::ParseTerm()
    {
        CComPtr<IJsDebugPropertyInternal> term;

        const CString& text = m_token.text;
        switch (m_token.type)
        {
        case TokenType::NUMBER:
            m_context->CreateDebugProperty(PROPERTY_INFO(m_expression, m_token.uintVal), /*parent*/nullptr, &term);
            break;

        case TokenType::STRING:
            m_context->CreateDebugProperty(PROPERTY_INFO(m_expression, m_token.text), /*parent*/nullptr, &term);
            break;

        case TokenType::IDENTIFIER:
            if (m_constants.Lookup(text, term)          // if it is a constant
                || TryGetProperty(m_root, text, &term))    // or a root variable
            {
                break;
            }
            // else fallthrough

        default:
            ThrowNotSupported();
            break;
        }

        Scan();
        return ParseSuffix(term);
    }

    CComPtr<IJsDebugPropertyInternal> SimpleExpressionEvaluator::ParseSuffix(CComPtr<IJsDebugPropertyInternal> term)
    {
        bool done = false;

        while (!done)
        {
            switch (m_token.type)
            {
            case JsDiag::SimpleExpressionEvaluator::DOT:
                {
                    Scan();
                    if (m_token.type != TokenType::IDENTIFIER)
                    {
                        ThrowNotSupported();
                    }

                    CComPtr<IJsDebugPropertyInternal> value;
                    term = TryGetProperty(term, m_token.text, &value) ? value : m_undefined;

                    Scan();
                }
                break;

            case JsDiag::SimpleExpressionEvaluator::LBRACKET:
                {
                    Scan();
                    CComPtr<IJsDebugPropertyInternal> index = ParseTerm();

                    CComPtr<IJsDebugPropertyInternal> value;
                    term = TryGetProperty(term, index, &value) ? value : m_undefined;

                    CheckToken(TokenType::RBRACKET);                    
                }
                break;

            case JsDiag::SimpleExpressionEvaluator::RBRACKET:
            case JsDiag::SimpleExpressionEvaluator::END:
                done = true;
                break;

            default:
                ThrowNotSupported(); // may be syntax error, but may be something we don't support
                break;
            }
        }

        return term;
    }

    template <class TryGetPropertyFunc, class FormatMessageFunc>
    bool_result SimpleExpressionEvaluator::TryGetPropertyCommon(IJsDebugPropertyInternal* object, TryGetPropertyFunc tryGetProperty, FormatMessageFunc formatMessage)
    {
        CComPtr<IJsDebugPropertyInternal> obj;
        if (!object->TryToObject(&obj))
        {
            CString message;
            PCWSTR fmt = m_context->GetDebugClient()->GetErrorString(JSERR_Property_CannotGet_NullOrUndefined);
            formatMessage(message, fmt);
            EvaluateException::Throw(message);
        }

        while (true)
        {
            if (tryGetProperty(obj))
            {
                return true;
            }

            CComPtr<IJsDebugPropertyInternal> proto;
            if (!obj->TryGetPrototype(&proto))
            {
                break;
            }

            obj = proto;
        }

        return false;
    }

    // Consider: Should this be in inspectionContext?
    bool_result SimpleExpressionEvaluator::TryGetProperty(IJsDebugPropertyInternal* object, const CString& name, _Outptr_ IJsDebugPropertyInternal** ppResult)
    {
        return TryGetPropertyCommon(object, [&](IJsDebugPropertyInternal* obj) -> bool_result
        {
            return obj->TryGetProperty(name, ppResult);
        },
            [&](CString& message, PCWSTR format)
        {
            message.Format(format, name.GetString());
        });
    }

    bool_result SimpleExpressionEvaluator::TryGetItem(IJsDebugPropertyInternal* object, UINT index, _Outptr_ IJsDebugPropertyInternal** ppResult)
    {
        return TryGetPropertyCommon(object, [&](IJsDebugPropertyInternal* obj) -> bool_result
        {
            return obj->TryGetItem(index, ppResult);
        },
            [&](CString& message, PCWSTR format)
        {
            WCHAR name[32];
            _ui64tow_s(index, name, _countof(name), 10);
            message.Format(format, name);
        });
    }

    bool_result SimpleExpressionEvaluator::TryGetProperty(IJsDebugPropertyInternal* object, IJsDebugPropertyInternal* index, _Outptr_ IJsDebugPropertyInternal** ppResult)
    {
        UINT n;
        CString name;

        if (!index->TryToIndex(&n, &name))
        {
            ThrowNotSupported();
        }

        return n != DiagConstants::InvalidIndexPropertyName ? TryGetItem(object, n, ppResult) : TryGetProperty(object, name, ppResult);
    }

    _declspec(noreturn) void SimpleExpressionEvaluator::ThrowNotSupported()
    {
        m_context->ThrowEvaluateNotSupported();
    }

    _declspec(noreturn) void EvaluateException::Throw(const CString& message)
    {
        throw EvaluateException(message);
    }
}
