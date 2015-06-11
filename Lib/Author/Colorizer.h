//
//    Copyright (C) Microsoft.  All rights reserved.
//
namespace  Authoring
{
    class Colorizer : public SimpleComObject<IAuthorColorizeText>
    {
    private:
        Js::ScriptContext* m_context;

    public:
        Colorizer(Js::ScriptContext* context): m_context(context) { }

        // *** IColorizeText ***
        STDMETHOD(Colorize)( 
                /* [in] */ const wchar_t *text,
                /* [in] */ int length,
                /* [in] */ AuthorSourceState state,
                /* [out] */ IAuthorTokenEnumerator **result);

        STDMETHOD(GetStateForText)( 
                /* [in] */ const wchar_t *text,
                /* [in] */ int length,
                /* [in] */ AuthorSourceState state,
                /* [out] */ AuthorSourceState *result);

        STDMETHOD(GetMultilineTokenKind)(/* in */ AuthorSourceState state, /* out */ AuthorMultilineTokenKind *result);
    };

}
