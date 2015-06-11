//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class JavascriptPixelArrayEnumerator : public JavascriptEnumerator
    {
    private:
        JavascriptPixelArray* pixelArrayObject;
        int index;

    protected:
        DEFINE_VTABLE_CTOR(JavascriptPixelArrayEnumerator, JavascriptEnumerator);
        virtual void MarshalToScriptContext(Js::ScriptContext * scriptContext) override
        {
            AssertMsg(false, "JavascriptPixelArrayEnumerator should never get marshaled"); 
        }  

    public:
        JavascriptPixelArrayEnumerator(JavascriptPixelArray* pixelArrayObject);
        virtual Var GetCurrentIndex() override;
        virtual Var GetCurrentValue() override { Js::Throw::NotImplemented(); }
        virtual BOOL MoveNext(PropertyAttributes* attributes = nullptr) override;
        virtual void Reset() { index = -1; }

    };
}
