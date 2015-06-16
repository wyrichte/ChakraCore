//----------------------------------------------------------------------------

// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class JsrtExternalObject;

class JsrtCustomEnumerator sealed : public Js::JavascriptEnumerator
{
public:
    JsrtCustomEnumerator(Js::ScriptContext* scriptContext, bool enumNonEnumerable, bool enumSymbols, JsMoveNextCallback moveNext, JsGetCurrentCallback getCurrent, JsEndEnumerationCallback finish, void *data, JsrtExternalObject* externalObject);
    virtual void Finalize(bool isShutdown) override;
    virtual void Dispose(bool isShutdown) override {};
    virtual Var GetCurrentIndex() override;
    virtual Var GetCurrentValue() override;
    virtual BOOL MoveNext(Js::PropertyAttributes* attributes = nullptr) override;
    virtual void Reset() override;
    virtual Var GetCurrentAndMoveNext(PropertyId& propertyId, Js::PropertyAttributes* attributes = nullptr) override;
    virtual Var GetCurrentBothAndMoveNext(PropertyId& propertyId, Var* currentValueRef) override;
protected:
    DEFINE_VTABLE_CTOR(JsrtCustomEnumerator, JavascriptEnumerator);
    virtual void MarshalToScriptContext(Js::ScriptContext * scriptContext) override
    {
        AssertMsg(false, "JsrtCustomEnumerator should never get marshaled"); 
    }  
private:
    bool enumNonEnumerable;
    bool enumSymbols;
    JsrtExternalObject* externalObject;
    JsMoveNextCallback moveNext;
    JsGetCurrentCallback getCurrent;
    JsEndEnumerationCallback finish;
    void *data;

    PropertyId GetPropertyIdOfIndex(Var index);
};
