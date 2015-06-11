//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptWeakSet::JavascriptWeakSet(DynamicType* type)
        : DynamicObject(type),
        keySet(type->GetScriptContext()->GetRecycler())
    {
    }

    Var JavascriptWeakSet::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        Assert(!(callInfo.Flags & CallFlags_New) || args[0] == nullptr);
        CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(WeakSetCount);

        JavascriptWeakSet* weakSetObject = nullptr;

        if (callInfo.Flags & CallFlags_New)
        {
             weakSetObject = library->CreateWeakSet();
        }
        else
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"WeakSet", L"WeakSet");
        }
        Assert(weakSetObject != nullptr);

        Var iterable = (args.Info.Count > 1) ? args[1] : library->GetUndefined();

        RecyclableObject* iter = nullptr;
        RecyclableObject* adder = nullptr;

        if (JavascriptConversion::CheckObjectCoercible(iterable, scriptContext))
        {
            iter = JavascriptOperators::GetIterator(iterable, scriptContext);
            Var adderVar = JavascriptOperators::GetProperty(weakSetObject, PropertyIds::add, scriptContext);
            if (!JavascriptConversion::IsCallable(adderVar))
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
            }
            adder = RecyclableObject::FromVar(adderVar);
        }

        if (iter != nullptr)
        {
            Var nextItem;

            while (JavascriptOperators::IteratorStepAndValue(iter, scriptContext, &nextItem))
            {
                adder->GetEntryPoint()(adder, CallInfo(CallFlags_Value, 2), weakSetObject, nextItem);
            }
        }

        return weakSetObject;
    }

    Var JavascriptWeakSet::EntryAdd(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptWeakSet::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"WeakSet.prototype.add", L"WeakSet");
        }

        JavascriptWeakSet* weakSet = JavascriptWeakSet::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();

        if (!JavascriptOperators::IsObject(key) || JavascriptOperators::GetTypeId(key) == TypeIds_HostDispatch)
        {
            // HostDispatch is not expando so can't have internal property added to it.
            // TODO: Support HostDispatch as WeakSet key
            JavascriptError::ThrowTypeError(scriptContext, JSERR_WeakMapSetKeyNotAnObject, L"WeakSet.prototype.add");
        }

        DynamicObject* keyObj = DynamicObject::FromVar(key);

        weakSet->Add(keyObj);

        return weakSet;
    }

    Var JavascriptWeakSet::EntryDelete(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptWeakSet::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"WeakSet.prototype.delete", L"WeakSet");
        }

        JavascriptWeakSet* weakSet = JavascriptWeakSet::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();
        bool didDelete = false;

        if (JavascriptOperators::IsObject(key) && JavascriptOperators::GetTypeId(key) != TypeIds_HostDispatch)
        {
            DynamicObject* keyObj = DynamicObject::FromVar(key);

            didDelete = weakSet->Delete(keyObj);
        }

        return scriptContext->GetLibrary()->CreateBoolean(didDelete);
    }

    Var JavascriptWeakSet::EntryHas(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptWeakSet::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"WeakSet.prototype.has", L"WeakSet");
        }

        JavascriptWeakSet* weakSet = JavascriptWeakSet::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();
        bool hasValue = false;

        if (JavascriptOperators::IsObject(key) && JavascriptOperators::GetTypeId(key) != TypeIds_HostDispatch)
        {
            DynamicObject* keyObj = DynamicObject::FromVar(key);

            hasValue = weakSet->Has(keyObj);
        }

        return scriptContext->GetLibrary()->CreateBoolean(hasValue);
    }

    void JavascriptWeakSet::Add(DynamicObject* key)
    {  
        keySet.Item(key, true);
    }

    bool JavascriptWeakSet::Delete(DynamicObject* key)
    {
        bool unused = false;
        return keySet.TryGetValueAndRemove(key, &unused);
    }

    bool JavascriptWeakSet::Has(DynamicObject* key)
    {
        bool unused = false;
        return keySet.TryGetValue(key, &unused);
    }

    BOOL JavascriptWeakSet::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"WeakSet");
        return TRUE;
    }

    JavascriptWeakSet* JavascriptWeakSet::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler *recycler = scriptContext->GetRecycler();
        CopyOnWriteObject<JavascriptWeakSet> *result = RecyclerNew(recycler, CopyOnWriteObject<JavascriptWeakSet>, scriptContext->GetLibrary()->GetWeakSetType(), this, scriptContext);

        keySet.Map([&](DynamicObject* key, bool val, const RecyclerWeakReference<DynamicObject>* weakRef) {
            Var copyKey = scriptContext->CopyOnWrite(key);
            Assert(DynamicObject::Is(copyKey));
            result->Add(DynamicObject::FromVar(copyKey));
        });

        return result;
    }
}