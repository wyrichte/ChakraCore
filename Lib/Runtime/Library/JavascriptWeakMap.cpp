//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptWeakMap::JavascriptWeakMap(DynamicType* type)
        : DynamicObject(type),
        keySet(type->GetScriptContext()->GetRecycler())
    {
    }

    JavascriptWeakMap::WeakMapKeyMap* JavascriptWeakMap::GetWeakMapKeyMapFromKey(DynamicObject* key) const
    {
        Var weakMapKeyData = null;
        if (!key->GetInternalProperty(key, InternalPropertyIds::WeakMapKeyMap, &weakMapKeyData, null, null))
        {
            return null;
        }

        return static_cast<WeakMapKeyMap*>(weakMapKeyData);
    }

    JavascriptWeakMap::WeakMapKeyMap* JavascriptWeakMap::AddWeakMapKeyMapToKey(DynamicObject* key)
    {
        // The internal property may exist on an object that has had DynamicObject::ResetObject called on itself.
        // In that case the value stored in the property slot should be null.
        DebugOnly(Var unused = null);
        Assert(!key->GetInternalProperty(key, InternalPropertyIds::WeakMapKeyMap, &unused, null, null) || unused == null);

        WeakMapKeyMap* weakMapKeyData = RecyclerNew(GetScriptContext()->GetRecycler(), WeakMapKeyMap, GetScriptContext()->GetRecycler());
        BOOL success = key->SetInternalProperty(InternalPropertyIds::WeakMapKeyMap, weakMapKeyData, PropertyOperation_Force, null);
        Assert(success);

        return weakMapKeyData;
    }

    bool JavascriptWeakMap::KeyMapGet(WeakMapKeyMap* map, Var* value) const
    {
        if (map->ContainsKey(GetWeakMapId()))
        {
            *value = map->Item(GetWeakMapId());
            return true;
        }

        return false;
    }

    Var JavascriptWeakMap::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        Assert(!(callInfo.Flags & CallFlags_New) || args[0] == nullptr);
        CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(WeakMapCount);

        JavascriptWeakMap* weakMapObject = nullptr;

        if (callInfo.Flags & CallFlags_New)
        {
            weakMapObject = library->CreateWeakMap();
        }
        else
        {
            // TODO: Implement this case to handle subclassing feature

            // ES6 Spec changed since Map was implemented regarding subclassing; passed in objects to a Map call
            // must have [[MapData]] on them already, via @@create method.  We do not currently support @@create,
            // so we no longer support subclassing of Map for IE11 release.  Since a user cannot call @@create,
            // they cannot obtain an object with the [[MapData]] internal property, and thus they cannot provide
            // an object for the this argument here that would not cause the Map constructor to throw.  Therefore
            // we always throw for now.

            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"WeakMap", L"WeakMap");
        }
        Assert(weakMapObject != nullptr);

        Var iterable = (args.Info.Count > 1) ? args[1] : library->GetUndefined();

        RecyclableObject* iter = nullptr;
        RecyclableObject* adder = nullptr;
        
        if (JavascriptConversion::CheckObjectCoercible(iterable, scriptContext))
        {
            iter = JavascriptOperators::GetIterator(iterable, scriptContext);
            Var adderVar = JavascriptOperators::GetProperty(weakMapObject, PropertyIds::set, scriptContext);
            if (!JavascriptConversion::IsCallable(adderVar))
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
            }
            adder = RecyclableObject::FromVar(adderVar);
        }

        if (iter != nullptr)
        {
            Var nextItem;
            Var undefined = library->GetUndefined();

            while (JavascriptOperators::IteratorStepAndValue(iter, scriptContext, &nextItem))
            {
                if (!JavascriptOperators::IsObject(nextItem))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
                }

                RecyclableObject* obj = RecyclableObject::FromVar(nextItem);

                Var key, value;

                if (!JavascriptOperators::GetItem(obj, 0, &key, scriptContext))
                {
                    key = undefined;
                }

                if (!JavascriptOperators::GetItem(obj, 1, &value, scriptContext))
                {
                    value = undefined;
                }

                adder->GetEntryPoint()(adder, CallInfo(CallFlags_Value, 3), weakMapObject, key, value);
            }
        }

        return weakMapObject;
    }

    Var JavascriptWeakMap::EntryDelete(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptWeakMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"WeakMap.prototype.delete", L"WeakMap");
        }

        JavascriptWeakMap* weakMap = JavascriptWeakMap::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();
        bool didDelete = false;

        if (JavascriptOperators::IsObject(key) && JavascriptOperators::GetTypeId(key) != TypeIds_HostDispatch)
        {
            DynamicObject* keyObj = DynamicObject::FromVar(key);

            didDelete = weakMap->Delete(keyObj);
        }

        return scriptContext->GetLibrary()->CreateBoolean(didDelete);
    }

    Var JavascriptWeakMap::EntryGet(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptWeakMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"WeakMap.prototype.get", L"WeakMap");
        }

        JavascriptWeakMap* weakMap = JavascriptWeakMap::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();

        if (JavascriptOperators::IsObject(key) && JavascriptOperators::GetTypeId(key) != TypeIds_HostDispatch)
        {
            DynamicObject* keyObj = DynamicObject::FromVar(key);
            Var value = null;

            if (weakMap->Get(keyObj, &value))
            {
                return value;
            }
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var JavascriptWeakMap::EntryHas(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptWeakMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"WeakMap.prototype.has", L"WeakMap");
        }

        JavascriptWeakMap* weakMap = JavascriptWeakMap::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();
        bool hasValue = false;

        if (JavascriptOperators::IsObject(key) && JavascriptOperators::GetTypeId(key) != TypeIds_HostDispatch)
        {
            DynamicObject* keyObj = DynamicObject::FromVar(key);

            hasValue = weakMap->Has(keyObj);
        }

        return scriptContext->GetLibrary()->CreateBoolean(hasValue);
    }

    Var JavascriptWeakMap::EntrySet(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptWeakMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"WeakMap.prototype.set", L"WeakMap");
        }

        JavascriptWeakMap* weakMap = JavascriptWeakMap::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();
        Var value = (args.Info.Count > 2) ? args[2] : scriptContext->GetLibrary()->GetUndefined();

        if (!JavascriptOperators::IsObject(key) || JavascriptOperators::GetTypeId(key) == TypeIds_HostDispatch)
        {
            // HostDispatch is not expando so can't have internal property added to it.
            // TODO: Support HostDispatch as WeakMap key
            JavascriptError::ThrowTypeError(scriptContext, JSERR_WeakMapSetKeyNotAnObject, L"WeakMap.prototype.set");
        }

        DynamicObject* keyObj = DynamicObject::FromVar(key);

        weakMap->Set(keyObj, value);

        return weakMap;
    }

    void JavascriptWeakMap::Clear()
    {
        keySet.Map([&](DynamicObject* key, bool value, const RecyclerWeakReference<DynamicObject>* weakRef) {
            WeakMapKeyMap* keyMap = GetWeakMapKeyMapFromKey(key);

            // It may be the case that a CEO has been reset and the keyMap is now null.
            // Just ignore it in this case, the keyMap has already been collected.
            if (keyMap != nullptr)
            {
                // It may also be the case that a CEO has been reset and then added to a separate WeakMap,
                // creating a new WeakMapKeyMap on the CEO.  In this case GetWeakMapId() may not be in the
                // keyMap, so don't assert successful removal here.
                keyMap->Remove(GetWeakMapId());
            }
        });
        keySet.Clear();
    }

    bool JavascriptWeakMap::Delete(DynamicObject* key)
    {
        WeakMapKeyMap* keyMap = GetWeakMapKeyMapFromKey(key);

        if (keyMap != null)
        {
            bool unused = false;
            bool inSet = keySet.TryGetValueAndRemove(key, &unused);
            bool inData = keyMap->Remove(GetWeakMapId());
            Assert(inSet == inData);

            return inData;
        }

        return false;
    }

    bool JavascriptWeakMap::Get(DynamicObject* key, Var* value) const
    {
        WeakMapKeyMap* keyMap = GetWeakMapKeyMapFromKey(key);

        if (keyMap != null)
        {
            return KeyMapGet(keyMap, value);
        }

        return false;
    }

    bool JavascriptWeakMap::Has(DynamicObject* key) const
    {
        WeakMapKeyMap* keyMap = GetWeakMapKeyMapFromKey(key);

        if (keyMap != null)
        {
            return keyMap->ContainsKey(GetWeakMapId());
        }

        return false;
    }

    void JavascriptWeakMap::Set(DynamicObject* key, Var value)
    {
        WeakMapKeyMap* keyMap = GetWeakMapKeyMapFromKey(key);

        if (keyMap == null)
        {
            keyMap = AddWeakMapKeyMapToKey(key);
        }

        keyMap->Item(GetWeakMapId(), value);
        keySet.Item(key, true);
    }

    BOOL JavascriptWeakMap::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"WeakMap");
        return TRUE;
    }

    JavascriptWeakMap* JavascriptWeakMap::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler *recycler = scriptContext->GetRecycler();
        CopyOnWriteObject<JavascriptWeakMap> *result = RecyclerNew(recycler, CopyOnWriteObject<JavascriptWeakMap>, scriptContext->GetLibrary()->GetWeakMapType(), this, scriptContext);

        keySet.Map([&](DynamicObject* key, bool val, const RecyclerWeakReference<DynamicObject>* weakRef) {
            WeakMapKeyMap* keyMap = GetWeakMapKeyMapFromKey(key);

            if (keyMap != nullptr)
            {
                Var value = null;
                bool success = KeyMapGet(keyMap, &value);
                Assert(success);

                Var copyKey = scriptContext->CopyOnWrite(key);

                result->keySet.Item(DynamicObject::FromVar(copyKey), true);
            }
        });

        return result;
    }

    /* static */ Var JavascriptWeakMap::CopyWeakMapKeyMapForCopyOnWrite(ScriptContext* scriptContext, Var sourceWeakMapKeyDataVar)
    {
        WeakMapKeyMap* sourceWeakMapKeyData = static_cast<WeakMapKeyMap*>(sourceWeakMapKeyDataVar);
        WeakMapKeyMap* targetWeakMapKeyData = RecyclerNew(scriptContext->GetRecycler(), WeakMapKeyMap, scriptContext->GetRecycler());

        sourceWeakMapKeyData->Map([&](const WeakMapId& sourceKey, const Var& sourceValue)
        {
            JavascriptWeakMap* sourceJavascriptWeakMap = GetWeakMapFromId(sourceKey);
            JavascriptWeakMap* targetJavascriptWeakMap = static_cast<JavascriptWeakMap*>(scriptContext->CopyOnWrite(sourceJavascriptWeakMap));
            WeakMapId targetKey = targetJavascriptWeakMap->GetWeakMapId();
            Var targetValue = scriptContext->CopyOnWrite(sourceValue);
            targetWeakMapKeyData->Add(targetKey, targetValue);
        });

        return targetWeakMapKeyData;
    }
}
