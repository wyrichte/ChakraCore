//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class EnumVariantEnumerator : public Js::JavascriptEnumerator
{
public:
    EnumVariantEnumerator(ScriptSite* scriptSite, IEnumVARIANT *enumerator);

    // JavascriptEnumerator Methods
    virtual Var GetCurrentIndex() override;
    virtual Var GetCurrentValue() override;
    virtual BOOL MoveNext(Js::PropertyAttributes* attributes = nullptr) override;
    virtual void Reset() override; 

    virtual void Finalize(bool isShutdown) override;
    virtual void Dispose(bool isShutdown) override;
    
protected:
    virtual void MarshalToScriptContext(Js::ScriptContext * scriptContext) override
    {
        AssertMsg(false, "EnumVariantEnumerator should never get marshaled"); 
    }
private:
    ScriptSite* scriptSite;
    IEnumVARIANT *enumerator;
    Js::Var currentItem;
    BOOL fetched;
} ;
