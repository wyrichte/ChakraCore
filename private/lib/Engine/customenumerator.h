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
        virtual Var GetCurrentIndex() override;
        virtual BOOL MoveNext(Js::PropertyAttributes* attributes = nullptr) override;
        virtual void Reset() override;
    protected:
        DEFINE_VTABLE_CTOR(CustomEnumerator, JavascriptEnumerator);        
        virtual void MarshalToScriptContext(Js::ScriptContext * scriptContext) override
        {
            AssertMsg(false, "CustomEnumerator should never get marshaled"); 
        }  
    private:
        CustomExternalObject* customObject;
        IVarEnumerator* varEnumerator;
    };

}
