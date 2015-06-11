//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class JavascriptArraySnapshotEnumerator : public JavascriptArrayEnumeratorBase
    {
    private:
        uint32 initialLength;

    protected:
        DEFINE_VTABLE_CTOR(JavascriptArraySnapshotEnumerator, JavascriptArrayEnumeratorBase);
        DEFINE_MARSHAL_ENUMERATOR_TO_SCRIPT_CONTEXT(JavascriptArraySnapshotEnumerator);

    public:
        JavascriptArraySnapshotEnumerator(JavascriptArray* arrayObject, ScriptContext* scriptContext, BOOL enumNonEnumerable, bool enumSymbols = false);
        virtual void Reset() override;
        virtual Var GetCurrentAndMoveNext(PropertyId& propertyId, PropertyAttributes* attributes = nullptr);
    };
}
