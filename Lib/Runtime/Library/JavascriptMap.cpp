//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptMap::JavascriptMap(DynamicType* type)
        : DynamicObject(type)
    {
    }

    Var JavascriptMap::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();
        AUTO_TAG_NATIVE_LIBRARY_ENTRY(scriptContext, L"Map");

        Assert(!(callInfo.Flags & CallFlags_New) || args[0] == nullptr);
        CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(MapCount);

        JavascriptMap* mapObject = nullptr;

        if (callInfo.Flags & CallFlags_New)
        {
            mapObject = library->CreateMap();
        }
        else
        {
            // TODO: Implement this case to handle sublcassing feature

            // ES6 Spec changed since Map was implemented regarding subclassing; passed in objects to a Map call
            // must have [[MapData]] on them already, via @@create method.  We do not currently support @@create,
            // so we no longer support subclassing of Map for IE11 release.  Since a user cannot call @@create,
            // they cannot obtain an object with the [[MapData]] internal property, and thus they cannot provide
            // an object for the this argument here that would not cause the Map constructor to throw.  Therefore
            // we always throw for now.

            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map", L"Map");
        }
        Assert(mapObject != nullptr);
        
        Var iterable = (args.Info.Count > 1) ? args[1] : library->GetUndefined();

        RecyclableObject* iter = nullptr;
        RecyclableObject* adder = nullptr;
        
        if (JavascriptConversion::CheckObjectCoercible(iterable, scriptContext))
        {
            iter = JavascriptOperators::GetIterator(iterable, scriptContext);
            Var adderVar = JavascriptOperators::GetProperty(mapObject, PropertyIds::set, scriptContext);
            if (!JavascriptConversion::IsCallable(adderVar))
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
            }
            adder = RecyclableObject::FromVar(adderVar);
        }

        if (mapObject->map != nullptr)
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_ObjectIsAlreadyInitialized, L"Map", L"Map");
        }

        mapObject->map = RecyclerNew(scriptContext->GetRecycler(), MapDataMap, scriptContext->GetRecycler());

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

                adder->GetEntryPoint()(adder, CallInfo(CallFlags_Value, 3), mapObject, key, value);
            }
        }

        return mapObject;
    }

    Var JavascriptMap::EntryClear(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.clear", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);

        map->Clear();

        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var JavascriptMap::EntryDelete(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.delete", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();

        bool didDelete = map->Delete(key);

        return scriptContext->GetLibrary()->CreateBoolean(didDelete);
    }

    Var JavascriptMap::EntryForEach(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AUTO_TAG_NATIVE_LIBRARY_ENTRY(scriptContext, L"Map.prototype.forEach");

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.forEach", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);

        if (args.Info.Count < 2 || !JavascriptConversion::IsCallable(args[1]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedFunction, L"Map.prototype.forEach");
        }
        RecyclableObject* callBackFn = RecyclableObject::FromVar(args[1]);

        Var thisArg = (args.Info.Count > 2) ? args[2] : scriptContext->GetLibrary()->GetUndefined();

        auto iterator = map->GetIterator();

        while (iterator.Next())
        {
            Var key = iterator.Current().Key();
            Var value = iterator.Current().Value();

            callBackFn->GetEntryPoint()(callBackFn, CallInfo(CallFlags_Value, 4), thisArg, value, key, map);
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var JavascriptMap::EntryGet(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.get", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();
        Var value = null;

        if (map->Get(key, &value))
        {
            return value;
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var JavascriptMap::EntryHas(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.has", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();

        bool hasValue = map->Has(key);

        return scriptContext->GetLibrary()->CreateBoolean(hasValue);
    }

    Var JavascriptMap::EntrySet(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.set", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);

        Var key = (args.Info.Count > 1) ? args[1] : scriptContext->GetLibrary()->GetUndefined();
        Var value = (args.Info.Count > 2) ? args[2] : scriptContext->GetLibrary()->GetUndefined();

        if (JavascriptNumber::Is(key) && JavascriptNumber::IsNegZero(JavascriptNumber::GetValue(key)))
        {
            // Normalize -0 to +0
            key = JavascriptNumber::New(0.0, scriptContext);
        }

        map->Set(key, value);

        return map;
    }

    Var JavascriptMap::EntrySizeGetter(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.size", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);

        int size = map->Size();

        return JavascriptNumber::ToVar(size, scriptContext);
    }

    Var JavascriptMap::EntryEntries(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.entries", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);

        return scriptContext->GetLibrary()->CreateMapIterator(map, JavascriptMapIteratorKind::KeyAndValue);
    }

    Var JavascriptMap::EntryKeys(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.keys", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);

        return scriptContext->GetLibrary()->CreateMapIterator(map, JavascriptMapIteratorKind::Key);
    }

    Var JavascriptMap::EntryValues(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (!JavascriptMap::Is(args[0]))
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Map.prototype.values", L"Map");
        }

        JavascriptMap* map = JavascriptMap::FromVar(args[0]);
        return scriptContext->GetLibrary()->CreateMapIterator(map, JavascriptMapIteratorKind::Value);
    }

    void JavascriptMap::Clear()
    {
        // TODO: (Consider) Should we clear the map here and leave it as large as it has grown, or
        // toss it away and create a new empty map, letting it grow as needed?
        list.Clear();
        map->Clear();
    }

    bool JavascriptMap::Delete(Var key)
    {
        if (map->ContainsKey(key))
        {
            MapDataNode* node = map->Item(key);
            list.Remove(node);
            return map->Remove(key);
        }
        return false;
    }

    bool JavascriptMap::Get(Var key, Var* value)
    {
        if (map->ContainsKey(key))
        {
            MapDataNode* node = map->Item(key);
            *value = node->data.Value();
            return true;
        }
        return false;
    }

    bool JavascriptMap::Has(Var key)
    {
        return map->ContainsKey(key);
    }

    void JavascriptMap::Set(Var key, Var value)
    {
        if (map->ContainsKey(key))
        {
            MapDataNode* node = map->Item(key);
            node->data = MapDataKeyValuePair(key, value);
        }
        else
        {
            MapDataKeyValuePair pair(key, value);
            MapDataNode* node = list.Append(pair, GetScriptContext()->GetRecycler());
            map->Add(key, node);
        }
    }

    int JavascriptMap::Size()
    {
        return map->Count();
    }

    BOOL JavascriptMap::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Map");
        return TRUE;
    }

    JavascriptMap* JavascriptMap::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler *recycler = scriptContext->GetRecycler();
        CopyOnWriteObject<JavascriptMap> *result = RecyclerNew(recycler, CopyOnWriteObject<JavascriptMap>, scriptContext->GetLibrary()->GetMapType(), this, scriptContext);
        result->map = RecyclerNew(scriptContext->GetRecycler(), MapDataMap, scriptContext->GetRecycler());

        auto iterator = GetIterator();
        while (iterator.Next())
        {
            Var proxyKey = scriptContext->CopyOnWrite(iterator.Current().Key());
            auto proxyValue = scriptContext->CopyOnWrite(iterator.Current().Value());
            result->Set(proxyKey, proxyValue);
        }

        return result;
    }

    Var JavascriptMap::EntryGetterSymbolSpecies(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);

        Assert(args.Info.Count > 0);

        return args[0];
    }
}
