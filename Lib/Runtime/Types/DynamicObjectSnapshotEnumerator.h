//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    template <typename T, bool enumNonEnumerable, bool enumSymbols>
    class DynamicObjectSnapshotEnumerator : public DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, /*snapShotSementics*/true>
    {
    protected:
        int initialPropertyCount;

        DynamicObjectSnapshotEnumerator(ScriptContext* scriptContext)
            : DynamicObjectEnumerator(scriptContext)
        {
        }

        DEFINE_VTABLE_CTOR(DynamicObjectSnapshotEnumerator, DynamicObjectEnumerator);
        DEFINE_MARSHAL_ENUMERATOR_TO_SCRIPT_CONTEXT(DynamicObjectSnapshotEnumerator);

        Var GetCurrentAndMoveNextFromArray(PropertyId& propertyId, PropertyAttributes* attributes);
        JavascriptString * GetCurrentAndMoveNextFromObject(T& index, PropertyId& propertyId, PropertyAttributes* attributes);

        DynamicObjectSnapshotEnumerator() { /* Do nothing, needed by the vtable ctor for ForInObjectEnumeratorWrapper */ }
        void Initialize(DynamicObject* object);

    public:
        static JavascriptEnumerator* New(ScriptContext* scriptContext, DynamicObject* object);

        virtual void Reset() override;
        virtual Var GetCurrentAndMoveNext(PropertyId& propertyId, PropertyAttributes* attributes = nullptr) override;
    };

} // namespace Js