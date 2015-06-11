//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class JavascriptArrayEnumerator : public JavascriptArrayEnumeratorBase
    {
    protected:
        DEFINE_VTABLE_CTOR(JavascriptArrayEnumerator, JavascriptArrayEnumeratorBase);
        DEFINE_MARSHAL_ENUMERATOR_TO_SCRIPT_CONTEXT(JavascriptArrayEnumerator);

    public:
        JavascriptArrayEnumerator(JavascriptArray* arrayObject, ScriptContext* scriptContext, BOOL enumNonEnumerable, bool enumSymbols = false);
        virtual void Reset() override;
        virtual Var GetCurrentAndMoveNext(PropertyId& propertyId, PropertyAttributes* attributes = nullptr) override;
    };
}
