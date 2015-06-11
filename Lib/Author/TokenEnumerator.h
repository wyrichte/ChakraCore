//
//    Copyright (C) Microsoft.  All rights reserved.
//
namespace Authoring 
{

    typedef Scanner<NullTerminatedUnicodeEncodingPolicy> ScannerT;

    class TokenEnumerator : public SimpleComObject<IAuthorTokenEnumerator>
    {

    private:
        ScannerT* m_scanner;
        HashTbl* m_hashTbl;
        LPOLESTR m_text;
        ErrHandler m_err;
        Token m_token;
        tokens m_previous;
        tokens m_previous2;
        tokens m_previous3;
        tokens m_previous4;

        // Below members are for providing classification for string template.
        int m_currentNumBraces;
        JsUtil::Stack<int> * m_braceStackStringTemplate;
        Js::ScriptContext * m_context;
        bool m_withinStringTemplate;

    public:
        TokenEnumerator(Js::ScriptContext* context, AuthorSourceState state, const wchar_t *text, int length);
        static ScannerT::ScanState GetScannerState(AuthorSourceState state);
        virtual void OnDelete() override;
    private:
        AuthorTokenKind TokenKind(tokens token);
        bool IsDivPrefix(tokens token);
        void EnsureAndPushCurrentBraceState();

    public:
        STDMETHOD(Next)(/* [out] */ AuthorTokenColorInfo *result);
    };

}