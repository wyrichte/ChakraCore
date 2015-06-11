//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: helpers

#pragma once

namespace Authoring
{
    struct void_t 
    {
        void_t() {}
        void_t(decltype(nullptr)) {}
        operator Js::Var () { return nullptr; }
    };

    // Represents any JS type - almost the same as Js::Var but allows to use the type system as opposed to 
    // Js::Var is defined as void* which disables the type system.
    struct any_t 
    {
        any_t() {}
        any_t(decltype(nullptr)) {}
        operator Js::Var () { return static_cast<Js::Var>(this); }
    };

    struct IRecyclableObjectConvertible
    {
        virtual Js::RecyclableObject* AsRecyclableObject() = 0;
    };

    class ScriptOperationHelpers
    {
    public:
        template<typename TOperation> static void ScriptOperation(Js::ScriptContext* scriptContext, TOperation operation)
        {
            Assert(scriptContext);
            if(!scriptContext->GetThreadContext()->IsScriptActive())
            {
                BEGIN_ENTER_SCRIPT(scriptContext, false, false, false);
                {
                    operation();
                }
                END_ENTER_SCRIPT;
            }
            else
            {
                operation();
            }
        }
    };

    struct Convert
    {
        static Js::Var ToVar(LPCWSTR value, Js::ScriptContext* scriptContext) 
        {
            Assert(scriptContext);
            return Js::JavascriptString::NewCopySz(value ? value : L"", scriptContext);
        }

        static Js::Var ToVar(Js::RecyclableObject* value, Js::ScriptContext*) { return value; }
        static Js::Var ToVar(IRecyclableObjectConvertible* value, Js::ScriptContext*) { return value->AsRecyclableObject(); }
        static Js::Var ToVar(const void_t&, Js::ScriptContext*) { return nullptr; }
        static Js::Var ToVar(any_t* value, Js::ScriptContext*) { return value; }
        static Js::Var ToVar(uint value, Js::ScriptContext*) { return Js::TaggedInt::ToVarUnchecked(value); }
        static Js::Var ToVar(int value, Js::ScriptContext*) { return Js::TaggedInt::ToVarUnchecked(value); }
        static Js::Var ToVar(bool value, Js::ScriptContext* scriptContext) { return value ? scriptContext->GetLibrary()->GetTrue() : scriptContext->GetLibrary()->GetFalse(); }

        static bool FromVar(ArenaAllocator* alloc, Js::Var var, __out Js::Var &targetValue) 
        {
            targetValue = var;
            return true;
        }

        static bool FromVar(ArenaAllocator* alloc, Js::Var var, __out LPCWSTR& targetValue)
        {
            Assert(alloc);
            targetValue = nullptr;
            auto jsString = FromVar<Js::JavascriptString, Js::TypeIds_String>(var);
            if(jsString) 
            {
                auto internalString =  Js::InternalString::New(alloc, jsString->GetSz(), jsString->GetLength());
                targetValue = internalString->GetBuffer();
            }
            return targetValue != nullptr;
        }

        static bool FromVar(ArenaAllocator* alloc, Js::Var var, __out Js::InternalString * &targetValue)
        {
            Assert(alloc);
            targetValue = nullptr;
            auto jsString = FromVar<Js::JavascriptString, Js::TypeIds_String>(var);
            if(jsString) 
            {
                targetValue =  Js::InternalString::New(alloc, jsString->GetSz(), jsString->GetLength());
            }
            return targetValue != nullptr;
        }

        static bool FromVar(ArenaAllocator*, Js::Var var, __out Js::JavascriptFunction*& targetValue)
        {
            targetValue = FromVar<Js::JavascriptFunction, Js::TypeIds_Function>(var);
            return targetValue != nullptr;
        }

        static bool FromVar(ArenaAllocator*, Js::Var var, __out Js::JavascriptArray*& targetValue)
        {
            targetValue = nullptr;
            Js::RecyclableObject* obj = FromVar<Js::RecyclableObject>(var);
            if(obj)
            {
                // Force Detach in CopyOnWrite case. Otherwise the object may have Object type instead of an Array type.
                // Wrap in ScriptOperation call as the call will be executing code in the engine.
                ScriptOperationHelpers::ScriptOperation(obj->GetScriptContext(), [&] () 
                {
                    obj->GetPropertyCount();
                });

                targetValue = FromVar<Js::JavascriptArray, Js::TypeIds_Array>(var);
            }
            return targetValue != nullptr;
        }

        static bool FromVar(ArenaAllocator*, Js::Var var, __out Js::RecyclableObject*& targetValue)
        {
            targetValue = FromVar<Js::RecyclableObject>(var);
            return targetValue != nullptr;
        }

        static bool FromVar(ArenaAllocator*, Js::Var var, __out uint& targetValue)
        {
            targetValue = 0;
            if(Js::TaggedInt::Is(var))
            {
                targetValue = Js::TaggedInt::ToUInt32(var);
                return true;
            }
            return false;
        }

        static bool FromVar(ArenaAllocator*, Js::Var var, __out int& targetValue)
        {
            targetValue = 0;
            if(Js::TaggedInt::Is(var))
            {
                targetValue = Js::TaggedInt::ToInt32(var);
                return true;
            }
            return false;
        }

        static bool FromVar(ArenaAllocator*, Js::Var var, __out bool& targetValue)
        {
            targetValue = false;
            auto value = FromVar<Js::JavascriptBoolean>(var);
            if(value)
            {
                targetValue = value->GetValue() ? true : false;
                return true;
            }
            return false;
        }

        static bool FromVar(ArenaAllocator*, Js::Var var, __out void_t&) { return true; }

        static bool FromVar(ArenaAllocator*, Js::Var var, __out any_t*& targetValue) 
        { 
            targetValue = static_cast<any_t*>(var);
            return true;
        }

        template<typename TType, Js::TypeId typeId>
        static TType* FromVar(Js::Var var)
        {
            TType* targetValue = FromVar<TType>(var);
            if(targetValue && targetValue->GetTypeId() == typeId)
                return targetValue;
            return nullptr;
        }

        template<typename TType>
        static TType* FromVar(Js::Var var)
        {
            TType* targetValue = nullptr;
            if(TType::Is(var))
                targetValue = TType::FromVar(var);
            return targetValue;
        }
    };

   class JsHelpers
    {
    public:

        static bool IsNullOrUndefined(Js::Var value)
        {
            return Js::JavascriptOperators::IsUndefinedOrNullType(Js::JavascriptOperators::GetTypeId(value)) ? true : false;
        }

        static bool IsTrackingKeyValue(Js::ScriptContext *scriptContext, Js::Var value)
        {
            if (value)
            {
                switch (Js::JavascriptOperators::GetTypeId(value))
                {
                case Js::TypeIds_Undefined:
                    return value != scriptContext->GetLibrary()->GetUndefined();
                case Js::TypeIds_Null:
                    return value != scriptContext->GetLibrary()->GetNull();
                }
            }
            return false;
        }

        static bool IsExceptionObject(ArenaAllocator *alloc, Js::ScriptContext* scriptContext, Js::Var& var)
        {
            // The interpreter thunk will create an empty object for undefined values as it is recovering from exceptions. 
            // The objects created as part of exception recovery will have _$isExceptionObject property set to true.
            if (var && Js::RecyclableObject::Is(var))
            {
                Js::RecyclableObject* obj = Js::RecyclableObject::FromVar(var);
                auto isExceptionObject = JsHelpers::GetProperty<Js::RecyclableObject*>(obj, Names::isExceptionObject, alloc, scriptContext, true, true);
                return (isExceptionObject == scriptContext->GetLibrary()->GetTrue());
            }
            return false;
        }

        static Js::RecyclableObject* CreateObject(Js::ScriptContext* scriptContext, Js::RecyclableObject* prototype = nullptr)
        {
            Assert(scriptContext);
            Js::RecyclableObject* obj = nullptr;
            if(prototype) 
            {
                obj = scriptContext->GetLibrary()->CreateObject(prototype);
            }
            else
            {
                obj = scriptContext->GetLibrary()->CreateObject();
            }
            JsHelpers::PreventRecycling(scriptContext, obj);
            return obj;
        }

        static Js::JavascriptArray* CreateArray(uint count, Js::ScriptContext* scriptContext)
        {
            Assert(scriptContext);
            auto array = scriptContext->GetLibrary()->CreateArray(count);
            JsHelpers::PreventRecycling(scriptContext, array);
            return array;
        }

        template<typename TElementAt>
        static Js::JavascriptArray* CreateArray(uint count, TElementAt elementAt, Js::ScriptContext* scriptContext)
        {
            Assert(scriptContext);
            auto array = CreateArray(count, scriptContext);
            for(uint i = 0; i < count; i++)
            {
                array->DirectSetItemAt(i, Convert::ToVar(elementAt(i), scriptContext));
            }
            return array;
        }

        template<typename TType, typename THandler>
        static void ForEach(Js::JavascriptArray* array, ArenaAllocator* alloc, THandler handler)
        {
            Assert(array);
            Assert(alloc);
            auto length = array->GetLength();
            for (uint index = 0; index < length; index++)
            {
                handler(index, GetArrayElement<TType>(array, index, alloc));
            }
        }

        template<typename T>
        static T GetArrayElement(Js::JavascriptArray* array, uint index, ArenaAllocator* alloc)
        {
            Assert(array);
            Assert(alloc);
            Js::Var value = nullptr;
            array->DirectGetItemAt(index, &value);
            T result = T();
            if (value)
                Convert::FromVar(alloc, value, result);
            return result;
        }

        static void Push(Js::JavascriptArray* array, Js::Var value)
        {
            Assert(array);
            auto len = array->GetLength();
            array->DirectSetItemAt(len, value);
        }

        template<typename TType>
        static void SetField(Js::RecyclableObject* obj, LPCWSTR name, TType value, Js::ScriptContext* scriptContext, bool enumerable = true)
        {
            Assert(obj);
            Assert(scriptContext);
            auto var = Convert::ToVar(value, scriptContext);
            if (var)
                SetFieldVar(obj, name, var, scriptContext, enumerable);
        }

        static void SetFieldVar(Js::RecyclableObject* obj, LPCWSTR name, Js::Var value, Js::ScriptContext* scriptContext, bool enumerable = true)
        {
            Assert(obj);
            Assert(scriptContext);
            if (value)
            {
                Js::PropertyRecord const * propertyRecord;
                scriptContext->GetOrAddPropertyRecord(name, (int)wcslen(name), &propertyRecord);
                auto id = propertyRecord->GetPropertyId();
                obj->SetProperty(id, value, Js::PropertyOperation_None, nullptr);
                if (!enumerable)
                    obj->SetEnumerable(id, false);
            }
        }

        template<typename T>
        static T GetProperty(Js::RecyclableObject* obj, LPCWSTR name, ArenaAllocator* alloc, Js::ScriptContext* scriptContext, bool getOwnProperty = false, bool ignoreExceptions = false)
        {
            Assert(obj);
            Assert(scriptContext);
            T value = T();
            try
            {
                Js::Var var = GetPropertyVar(obj, name, scriptContext, getOwnProperty);
                if (var)
                    Convert::FromVar(alloc, var, value);
            }
            catch (Js::JavascriptExceptionObject*)
            {
                if (!ignoreExceptions)
                    throw;
            }

            return value;
        }

        static Js::Var GetPropertyVar(Js::RecyclableObject* obj, LPCWSTR name, Js::ScriptContext* scriptContext, bool getOwnProperty = false)
        {
            Assert(obj);
            Assert(scriptContext);
            
            Js::Var var = nullptr;
            // Note: JavascriptVariantDate doesn't support GetProperty
            if(obj->GetTypeId() != Js::TypeIds_VariantDate) 
            {
                Js::PropertyRecord const * propertyRecord;
                scriptContext->GetOrAddPropertyRecord(name, (int)wcslen(name), &propertyRecord);  
                auto id = propertyRecord->GetPropertyId();
                ScriptOperationHelpers::ScriptOperation(scriptContext, [&] () 
                {
                    if (getOwnProperty)
                    {
                        Js::JavascriptOperators::GetOwnProperty(obj, id, &var, scriptContext);
                    }
                    else if (Js::RootObjectBase::Is(obj))
                    {
                        var = Js::JavascriptOperators::GetRootProperty(obj, id, scriptContext, nullptr);
                    }
                    else
                    {
                        var = Js::JavascriptOperators::GetProperty(obj, id, scriptContext, nullptr);
                    }
                });
            }
            return var;
        }

        template <typename TOperation>
        static void WithArguments(TOperation operation, Js::ScriptContext* scriptContext, Js::RecyclableObject* thisObj, Js::Var arg0 = nullptr, Js::Var arg1 = nullptr, Js::Var arg2 = nullptr, Js::Var arg3 = nullptr)
        {
            Js::Var args[5];
            unsigned short argsCount = 1;
            args[0] = thisObj != nullptr ? thisObj : scriptContext->GetGlobalObject();
            args[1] = arg0;
            args[2] = arg1;
            args[3] = arg2;
            args[4] = arg3;
            for(int i = 1; i <= 4; i++)
            {
                if(!args[i])
                    break;
                argsCount++;
            }
            Js::CallInfo callInfo(Js::CallFlags_Value, argsCount);
            auto arguments = Js::Arguments(callInfo, args);
            operation(arguments);
        }

        static Js::Var CallFunction(Js::ScriptContext* scriptContext, Js::RecyclableObject* obj, LPCWSTR method, Js::Var arg0 = nullptr, Js::Var arg1 = nullptr, Js::Var arg2 = nullptr, Js::Var arg3 = nullptr)
        {
            Assert(scriptContext);
            Assert(obj);
            Assert(method);           
            auto func = JsHelpers::GetProperty<Js::JavascriptFunction*>(obj, method, nullptr, scriptContext);
            if (func)
                return CallFunction(func, obj, scriptContext, arg0, arg1, arg2, arg3);
            return scriptContext->GetLibrary()->GetUndefined();
        }

        static Js::Var CallFunction(Js::JavascriptFunction* func, Js::RecyclableObject* thisObj, Js::ScriptContext* scriptContext, Js::Var arg0 = nullptr, Js::Var arg1 = nullptr, Js::Var arg2 = nullptr, Js::Var arg3 = nullptr)
        {
            Js::Var result = nullptr;
            WithArguments([&] (Js::Arguments& arguments) {
                result = func->CallRootFunction(arguments, scriptContext);
            },
            scriptContext, thisObj, arg0, arg1, arg2, arg3);
            return result;
        }

        static void PreventRecycling(Js::ScriptContext* scriptContext, Js::Var value)
        {
            Assert(scriptContext);
            if(!value) return;
        
            auto valueType = Js::JavascriptOperators::GetTypeId(value);
            switch (valueType)
            {
                case Js::TypeIds_Enumerator:
                case Js::TypeIds_ExtensionEnumerator:
                case Js::TypeIds_HostDispatch:
                case Js::TypeIds_Boolean:
                case Js::TypeIds_Integer:
                case Js::TypeIds_Int64Number:
                case Js::TypeIds_UInt64Number:
                case Js::TypeIds_Null:
                case Js::TypeIds_Undefined:
                    // No need to protect these.
                    break;
                case Js::TypeIds_Number:
                    if (Js::TaggedNumber::Is(value))
                        break;
                    
                    // intentional fall-through
                    __fallthrough;

                default:
                    auto object = Js::RecyclableObject::FromVar(value);
                    // TODO: Change this to use its own hash table in AuthoringScriptContext once we have an AuthoringScriptContext.
                    scriptContext->RecordCopyOnWrite(object, object);
            }
        }
    };

    template <typename TReturn = void_t, typename TArg0 = void_t, typename TArg1 = void_t>
    struct FuncSignature
    {
        static TReturn Return;
        static TArg0 Arg0;
        static TArg1 Arg1;
    };

    struct Functor
    {
        template <typename TFunctor>
        static void Call(TFunctor functor, void_t&, void_t, void_t)
        {
            functor();
        }

        template <typename TFunctor, typename TArg0>
        static void Call(TFunctor functor, void_t&, TArg0 arg0, void_t)
        {
            functor(arg0);
        }

        template <typename TFunctor, typename TArg0, typename TArg1>
        static void Call(TFunctor functor, void_t&, TArg0 arg0, TArg1 arg1)
        {
            functor(arg0, arg1);
        }

        template <typename TFunctor, typename TReturn>
        static void Call(TFunctor functor, TReturn& result, void_t, void_t)
        {
            result = functor();
        }

        template <typename TFunctor, typename TReturn, typename TArg0>
        static void Call(TFunctor functor, TReturn& result, TArg0 arg0, void_t)
        {
            result = functor(arg0);
        }

        template <typename TFunctor, typename TReturn, typename TArg0, typename TArg1>
        static void Call(TFunctor functor, TReturn& result, TArg0 arg0, TArg1 arg1)
        {
            result = functor(arg0, arg1);
        }
    };

    template <typename T>
    static T* arena_new (ArenaAllocator* alloc)
    {
        return new (alloc->Alloc(sizeof(T))) T();
    }

    template <typename T, typename TArg0, typename TArg1, typename TArg2>
    static T* arena_new (ArenaAllocator* alloc, TArg0 arg0, TArg1 arg1, TArg2 arg2)
    {
        return new(alloc->Alloc(sizeof(T))) T(arg0, arg1, arg2);
    }

    template <typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
    static T* arena_new (ArenaAllocator* alloc, TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3)
    {
        return new (alloc->Alloc(sizeof(T))) T(arg0, arg1, arg2, arg3);
    }

    template <typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4>
    static T* arena_new (ArenaAllocator* alloc, TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4)
    {
        return new (alloc->Alloc(sizeof(T))) T(arg0, arg1, arg2, arg3, arg4);
    }

    template <typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5>
    static T* arena_new (ArenaAllocator* alloc, TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5)
    {
        return new (alloc->Alloc(sizeof(T))) T(arg0, arg1, arg2, arg3, arg4, arg5);
    }

    template <typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5, typename TArg6>
    static T* arena_new (ArenaAllocator* alloc, TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6)
    {
        return new (alloc->Alloc(sizeof(T))) T(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    template<typename TTarget, typename TSource>
    TTarget force_cast(TSource value) { return reinterpret_cast<TTarget>((void*)value); }

    template<typename TAction>
    void IgnoreJsExceptions(TAction action) 
    {  
        try
        {
            action();
        }
        catch(Js::JavascriptExceptionObject*)
        {
        }
        catch(Js::OutOfMemoryException) 
        {
        }
        catch(Js::InternalErrorException) 
        {
        }
        catch(Js::NotImplementedException) 
        {  
        }
        catch (Js::ScriptAbortException)
        {
        }
    }

    class ListOperations
    {
    public:
        template<typename TType, typename THandler>
        static void ForEach(JsUtil::List<TType, ArenaAllocator>* list, THandler handler)
        {
            Assert(list);
            list->Map(handler);            
        }
    };
}
