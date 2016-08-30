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
public:
    HostDispatchEnumerator(HostDispatch*);
    virtual Var MoveAndGetNext(Js::PropertyId& propertyId, Js::PropertyAttributes* attributes = nullptr) override;
    virtual void Reset() override;
};
