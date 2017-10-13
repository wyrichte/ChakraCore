//----------------------------------------------------------------------------

// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class CustomExternalObject;

    class CustomEnumerator sealed : public JavascriptEnumerator
    {
    public:
        CustomEnumerator(ScriptContext* scriptContext, IVarEnumerator* varEnumerator, CustomExternalObject* customObject);
        virtual void Finalize(bool isShutdown) override {};
        virtual void Dispose(bool isShutdown) override;
        virtual JavascriptString* MoveAndGetNext(PropertyId& propertyId, PropertyAttributes* attributes = nullptr) override;
        virtual void Reset() override;
    protected:
        DEFINE_VTABLE_CTOR(CustomEnumerator, JavascriptEnumerator);
    private:
        CustomExternalObject* customObject;
        IVarEnumerator* varEnumerator;
    };

}
