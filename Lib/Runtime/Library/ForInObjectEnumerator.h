//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class ForInObjectEnumerator : public WeakReferenceCache<ForInObjectEnumerator>
    {
    private:
        DynamicObjectSnapshotEnumeratorWPCache<BigPropertyIndex, true, false> embeddedEnumerator;
        JavascriptEnumerator * currentEnumerator;
        RecyclableObject *object;
        RecyclableObject *baseObject;
        BVSparse<Recycler>* propertyIds;
        RecyclableObject *firstPrototype;
        Var currentIndex;
        Type* baseObjectType;
        SListBase<Js::PropertyRecord const *> newPropertyStrings;
        ScriptContext * scriptContext;
        bool enumSymbols;

        BOOL TestAndSetEnumerated(PropertyId propertyId);
        BOOL GetCurrentEnumerator();

        // Only used by the vtable ctor for ForInObjectEnumeratorWrapper
        friend class ForInObjectEnumeratorWrapper;
        ForInObjectEnumerator() { /* do nothing */ }

    public:
        ForInObjectEnumerator(RecyclableObject* object, ScriptContext * requestContext, bool enumSymbols = false);
        ~ForInObjectEnumerator() { Clear(); }

        ScriptContext * GetScriptContext() const { return scriptContext; }
        BOOL CanBeReused();
        void Initialize(RecyclableObject* currentObject, ScriptContext * scriptContext);
        void Clear();
        Var GetCurrentIndex();
        Var GetCurrentValue();
        BOOL MoveNext();
        void Reset();
        Var GetCurrentBothAndMoveNext(PropertyId& propertyId, Var *currentValueRef);
        Var GetCurrentAndMoveNext(PropertyId& propertyId);

        static uint32 GetOffsetOfCurrentEnumerator() { return offsetof(ForInObjectEnumerator, currentEnumerator); }
        static uint32 GetOffsetOfFirstPrototype() { return offsetof(ForInObjectEnumerator, firstPrototype); }
        
        static RecyclableObject* GetFirstPrototypeWithEnumerableProperties(RecyclableObject* object);
    };

    // Use when we want to use the ForInObject as if they are normal javascript enumerator
    class ForInObjectEnumeratorWrapper : public JavascriptEnumerator
    {
    public:
        ForInObjectEnumeratorWrapper(RecyclableObject* object, ScriptContext * requestContext)
            : JavascriptEnumerator(requestContext), forInObjectEnumerator(object, requestContext)
        {
        }

        virtual Var GetCurrentIndex() override { return forInObjectEnumerator.GetCurrentIndex(); }
        virtual Var GetCurrentValue() override { return forInObjectEnumerator.GetCurrentValue(); }
        virtual BOOL MoveNext(PropertyAttributes* attributes = nullptr) override { return forInObjectEnumerator.MoveNext(); }
        virtual void Reset() override { forInObjectEnumerator.Reset(); }
        virtual Var GetCurrentBothAndMoveNext(PropertyId& propertyId, Var *currentValueRef) override 
        { 
            return forInObjectEnumerator.GetCurrentBothAndMoveNext(propertyId, currentValueRef); 
        }
        virtual Var GetCurrentAndMoveNext(PropertyId& propertyId, PropertyAttributes* attributes = nullptr)
        {
            return forInObjectEnumerator.GetCurrentAndMoveNext(propertyId);
        }
    protected:
        DEFINE_VTABLE_CTOR(ForInObjectEnumeratorWrapper, JavascriptEnumerator);
        virtual void MarshalToScriptContext(Js::ScriptContext * scriptContext) override
        {
            // Currently this wrapper is only used by ExtensionEnumeratorObject and doesn't support marshaling cross script context.
            AssertMsg(false, "ForInObjectEnumeratorWrapper should never get marshaled");
        }
    private:
        ForInObjectEnumerator forInObjectEnumerator;
    };

    // Enumerator for undefined/number/double values.
    class NullEnumerator : public JavascriptEnumerator
    {
    private:
        virtual Var GetCurrentIndex() override;
        virtual Var GetCurrentValue() override;
        virtual BOOL MoveNext(PropertyAttributes* attributes = nullptr) override;
        virtual void Reset() override;
        virtual Var GetCurrentAndMoveNext(PropertyId& propertyId, PropertyAttributes* attributes = nullptr) override;
        virtual BOOL GetCurrentPropertyId(PropertyId *propertyId) override;

    protected:
        DEFINE_VTABLE_CTOR(NullEnumerator, JavascriptEnumerator);

        // nothing to marshal
        virtual void MarshalToScriptContext(Js::ScriptContext * scriptContext) override {}
    public:
        NullEnumerator(ScriptContext* scriptContext) : JavascriptEnumerator(scriptContext) {}
    };
}
