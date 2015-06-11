//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    // --------------------------------------------------------------------------------------------
    // SimpleExpressionEvaluator only supports very basic property access syntax: x.y, x[12], or
    // extended x.y[12].z
    // --------------------------------------------------------------------------------------------
    class SimpleExpressionEvaluator
    {
    private:
        enum TokenType { NONE, NUMBER, STRING, IDENTIFIER, DOT, LBRACKET, RBRACKET, END = NONE};

        struct Token
        {
            TokenType type;
            UINT uintVal;    // number value, only valid for NUMBER
            CString text;   // Trimmed text, only valid for STRING/IDENTIFIER
        };

        // Scanner
        CString m_expression;
        int m_cur;
        Token m_token;

        // Runtime
        CComPtr<InspectionContext> m_context;

        CComPtr<IJsDebugPropertyInternal> m_root; // stack frame Locals root
        CAtlMap<CString, CComPtr<IJsDebugPropertyInternal>> m_constants; // null, undefined, true...
        CComPtr<IJsDebugPropertyInternal> m_undefined;

    public:
        SimpleExpressionEvaluator(InspectionContext* context, _In_ IJsDebugPropertyInternal* root);
        
        bool_result TryEvaluate(
            const CString& expression,
            _Outptr_ IJsDebugProperty** ppResult,
            _Out_ CString& error);

    private:
        void InitConst(InspectionContext* context, PCWSTR name, Js::Var var, _Outptr_opt_ IJsDebugPropertyInternal** ppDebugProperty = nullptr);

        TokenType Scan();
        void CheckToken(TokenType tk);
        CComPtr<IJsDebugPropertyInternal> ParseTerm();
        CComPtr<IJsDebugPropertyInternal> ParseSuffix(CComPtr<IJsDebugPropertyInternal> term);

        __declspec(noreturn) void ThrowNotSupported();

        template <class TryGetPropertyFunc, class FormatMessageFunc>
        bool_result TryGetPropertyCommon(IJsDebugPropertyInternal* object, TryGetPropertyFunc tryGetProperty, FormatMessageFunc formatMessage);

        bool_result TryGetProperty(IJsDebugPropertyInternal* object, const CString& name, _Outptr_ IJsDebugPropertyInternal** ppResult);
        bool_result TryGetItem(IJsDebugPropertyInternal* object, UINT index, _Outptr_ IJsDebugPropertyInternal** ppResult);
        bool_result TryGetProperty(IJsDebugPropertyInternal* object, IJsDebugPropertyInternal* index, _Outptr_ IJsDebugPropertyInternal** ppResult);
    };

    // --------------------------------------------------------------------------------------------
    // Evaluation exception type, to identify and bail from scan/parse errors.
    // --------------------------------------------------------------------------------------------
    struct EvaluateException: public DiagException
    {
        CString message;

        EvaluateException(const CString& message, HRESULT hr = S_FALSE) :
            DiagException(hr),
            message(message)
        {}

        __declspec(noreturn) static void Throw(const CString& message);
    };
}
