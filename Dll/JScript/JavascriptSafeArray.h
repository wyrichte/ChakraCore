//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
class JavascriptSafeArray : public RecyclableObject
{
protected:
    DEFINE_VTABLE_CTOR(JavascriptSafeArray, RecyclableObject);

public:
    JavascriptSafeArray(ScriptSite* actsp, VARTYPE vt, SAFEARRAY * psa, StaticType * type) : 
      activeScriptSite(actsp), RecyclableObject(type)
    {
        Assert(type->GetTypeId() == TypeIds_SafeArray);
        m_vt = vt;
        SafeArrayCopy(psa, &m_safeArray);
    }

    static bool Is(Var aValue);
    static JavascriptSafeArray* FromVar(Var aValue);

    VARTYPE GetSafeArrayVarType() const { return m_vt; }
    SAFEARRAY* GetSafeArray(){return m_safeArray;}
    ScriptSite* GetScriptSite(){ return activeScriptSite;}
    uint GetSafeArrayLength();
    uint GetSafeArraySize();

    virtual RecyclableObject*  ToObject(ScriptContext * requestContext) override;
    virtual BOOL Equals(Var other, BOOL* value, ScriptContext * requestContext) override;
    virtual Var GetTypeOfString(ScriptContext * requestContext) override;
    virtual BOOL ToPrimitive(JavascriptHint hint, Var* value, ScriptContext * requestContext)override {*value = this; return true;}

    virtual void Finalize(bool isShutdown) override {}
    virtual void Dispose(bool isShutdown) override
    {
        if (!isShutdown)
        {
            SafeArrayDestroy(m_safeArray);
        }
        else
        {
            LEAK_REPORT_PRINT(L"JavascriptSafeArray %p: Finalize not called on shutdown (SafeArray %p)\n",
                this, m_safeArray);
        }
    }

    virtual RecyclableObject * CloneToScriptContext(ScriptContext* requestContext) override;

private:
    VARTYPE m_vt;
    SAFEARRAY* m_safeArray;
    ScriptSite* activeScriptSite;
};
}