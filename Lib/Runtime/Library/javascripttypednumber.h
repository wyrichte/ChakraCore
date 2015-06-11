//---------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    template<typename T>
    class JavascriptTypedNumber : public RecyclableObject
    {
    private:
        T m_value;
        __inline JavascriptTypedNumber(T value, StaticType * type) : RecyclableObject(type), m_value(value)
        {
#if DBG
            AssertMsg(type->GetTypeId() == TypeIds_Int64Number ||
                type->GetTypeId() == TypeIds_UInt64Number, "invalid typed number");
#endif
        }

    protected:
        DEFINE_VTABLE_CTOR(JavascriptTypedNumber, RecyclableObject);

    public:

        T GetValue() const
        {   
            return m_value;
        }

        static Var ToVar(T nValue, ScriptContext* scriptContext);

        static JavascriptTypedNumber<T>* FromVar(Var value)
        {
#if DBG
            AssertMsg(JavascriptOperators::GetTypeId(value) == TypeIds_Int64Number ||
                JavascriptOperators::GetTypeId(value) == TypeIds_UInt64Number, "invalid typed number");
#endif
            return static_cast<JavascriptTypedNumber<T>*>(value);
        };

        static JavascriptString* ToString(Var value, ScriptContext* scriptContext);
        
        Var ToJavascriptNumber()
        {
            return JavascriptNumber::New((double)GetValue(), GetScriptContext());
        }

        RecyclableObject * CloneToScriptContext(ScriptContext* requestContext) override
        {
            return RecyclableObject::FromVar(JavascriptTypedNumber::ToVar(this->GetValue(), requestContext));
        }

        BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override
        {
            stringBuilder->AppendCppLiteral(L"Number, (Object)");
            return TRUE;
        }

        virtual RecyclableObject* ToObject(ScriptContext * requestContext) override
        {
            return requestContext->GetLibrary()->CreateNumberObjectWithCheck((double)m_value);
        }

        virtual Var GetTypeOfString(ScriptContext * requestContext) override {Assert(FALSE); return requestContext->GetLibrary()->GetNumberTypeDisplayString(); }
        virtual BOOL ToPrimitive(JavascriptHint hint, Var* value, ScriptContext * requestContext)override {Assert(false); *value = this; return true;}

    };

    typedef JavascriptTypedNumber<__int64> JavascriptInt64Number;
    typedef JavascriptTypedNumber<unsigned __int64> JavascriptUInt64Number;
}
