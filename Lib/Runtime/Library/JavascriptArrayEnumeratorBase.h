//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class JavascriptArrayEnumeratorBase abstract : public JavascriptEnumerator
    {
    protected:
        JavascriptArray* arrayObject;
        uint32 index;     
        bool doneArray;
        bool doneObject;
        BOOL enumNonEnumerable;
        bool enumSymbols;
        JavascriptEnumerator* objectEnumerator;

        DEFINE_VTABLE_CTOR_ABSTRACT(JavascriptArrayEnumeratorBase, JavascriptEnumerator)

        JavascriptArrayEnumeratorBase(JavascriptArray* arrayObject, ScriptContext* scriptContext, BOOL enumNonEnumerable, bool enumSymbols = false);
        virtual Var GetCurrentIndex() override;
        virtual Var GetCurrentValue() override;
        virtual BOOL MoveNext(PropertyAttributes* attributes = nullptr) override;
        virtual uint32 GetCurrentItemIndex()  override { return index; }
        virtual BOOL GetCurrentPropertyId(PropertyId *propertyId) override;
    };
}
