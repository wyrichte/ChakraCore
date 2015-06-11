//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class HostDispatchEnumerator : public Js::JavascriptEnumerator
{
private:
    HostDispatch* hostDispatch;
    DISPID idCurrent;
protected:
    DEFINE_VTABLE_CTOR(HostDispatchEnumerator, Js::JavascriptEnumerator);
    virtual void MarshalToScriptContext(Js::ScriptContext * scriptContext) override
    {
        AssertMsg(false, "HostDispatchEnumerator should never get marshaled"); 
    }
public:
    HostDispatchEnumerator(HostDispatch*);
    virtual Var GetCurrentIndex() override;
    virtual Var GetCurrentValue() override { Js::Throw::NotImplemented(); }
    virtual BOOL MoveNext(Js::PropertyAttributes* attributes = nullptr) override;
    virtual void Reset() override;
};
