//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Authoring
{
    //
    //  Name: JsCallable 
    //  Summary: Represents a C++ native object which can be called from JavaScript.
    //
    class JsCallable : public IRecyclableObjectConvertible
    {
    private:
        template <typename TSignature, typename THandler>
        class JsDirectFunc
        {
            template <typename TType, int index>
            TType ArgValue(Js::Var* args, int argsCount)
            {
                TType value = TType();
                if(index < argsCount && args[index] != nullptr)
                {
                    Convert::FromVar(_alloc, args[index], value);
                }
                return value;
            }

            THandler                _handler;
            Js::PropertyId          _id;
            Js::JavascriptFunction* _func;
            Js::ScriptContext*      _scriptContext;
            ArenaAllocator*         _alloc;

            typedef JsDirectFunc<TSignature, THandler> JsDirectFunc_t;
        public:
            JsDirectFunc(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, LPCWSTR fieldName, THandler handler) 
                : _handler(handler), _scriptContext(scriptContext), _alloc(alloc), _func(nullptr) 
            { 
                _id = _scriptContext->GetOrAddPropertyIdTracked(fieldName, (int)wcslen(fieldName));
            }

            static JsDirectFunc<TSignature, THandler>* New(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, LPCWSTR fieldName, THandler handler)
            {
                return Anew(alloc, JsDirectFunc_t, alloc, scriptContext, fieldName, handler);
            }

            Js::JavascriptFunction* JsFunc()
            {
                if(_func == nullptr)
                {
                    _func = _scriptContext->GetLibrary()->CreateExternalFunction(Thunk, _id, this);
                    JsHelpers::PreventRecycling(_scriptContext, _func);
                }

                return _func;
            }

        private:

            Js::Var CallHandler(Js::Var* args, int argsCount)
            {
                HRESULT hr = S_OK;
                decltype(TSignature::Return) returnValue;
                Js::Var returnVar = nullptr;
                BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
                {
                    auto arg0 = ArgValue<decltype(TSignature::Arg0), 1>(args, argsCount);
                    auto arg1 = ArgValue<decltype(TSignature::Arg1), 2>(args, argsCount);
                    Functor::Call(_handler, returnValue, arg0, arg1);
                    returnVar = Convert::ToVar(returnValue, _scriptContext);
                }
                END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
                if(FAILED(hr))
                {
                    Js::JavascriptError::MapAndThrowError(_scriptContext, hr);
                }
                return returnVar;
            }

            static Js::Var Thunk (Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
            {
                HRESULT hr = S_OK;

                Js::JavascriptFunction* jsFunc = Js::JavascriptFunction::FromVar(jsFuncVar);

                if ((Js::JavascriptOperators::GetTypeId(jsFunc) != Js::TypeIds_Function) || !jsFunc->IsExternalFunction())
                {
                    return NULL;
                }

                auto jsExternalFunc = static_cast<Js::JavascriptExternalFunction*>(jsFuncVar);

                Js::ScriptContext* scriptContext = jsExternalFunc->GetType()->GetScriptContext();

                ARGUMENTS(args, callInfo);

                auto impl = static_cast<JsDirectFunc_t*>(jsExternalFunc->GetSignature());
                if (impl == nullptr)
                {
                    Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction /* TODO-ERROR: get arg name - args[0] */);
                }

                Js::Var result = scriptContext->GetLibrary()->GetUndefined();
                result = impl->CallHandler(args.Values, callInfo.Count);

                if(FAILED(hr))
                {
                    Js::JavascriptError::MapAndThrowError(scriptContext, hr);
                }

                return result;
            }
        }; // End JsDirectFunc class

    protected:
        Js::RecyclableObject*   _jsobj;
        Js::ScriptContext*      _scriptContext;
        ArenaAllocator*         _alloc;
    public:
        JsCallable(ArenaAllocator* alloc, Js::ScriptContext* scriptContext) 
            : _alloc(alloc), _scriptContext(scriptContext), _jsobj(nullptr)
        {
            Assert(_scriptContext != nullptr);
        }

        Js::RecyclableObject* AsRecyclableObject() override
        {
            if(_jsobj == nullptr)
            {
                _jsobj = CreateUnderlyingObject();
                Assert(_jsobj != nullptr);
                ExposeProperties();
            }
            return _jsobj;
        }

        Js::ScriptContext* GetScriptContext()
        {
            return _scriptContext;
        }

        ArenaAllocator* Alloc() 
        {
            return _alloc;
        }

        void AssignToProperty(Js::RecyclableObject* targetObj, LPCWSTR propName)
        {
            JsHelpers::SetField(targetObj, propName, this, _scriptContext);
        }

        template <typename TSignature, class THandler>
        static Js::JavascriptFunction* CreateExternalFunction(Js::ScriptContext *scriptContext, LPCWSTR name, THandler handler)
        {
            Assert(scriptContext);
            Assert(name);

            auto directFunc = JsDirectFunc<TSignature, THandler>::New(scriptContext->GetGuestArena(), scriptContext, name, handler);
            return directFunc->JsFunc();
        }

    protected:

        virtual Js::RecyclableObject* CreateUnderlyingObject()
        {
            return JsHelpers::CreateObject(_scriptContext);
        }

        virtual void ExposeProperties() { }

        template <typename TSetType, typename TGetType, typename TGetter, typename TSetter>
        void ExposeProperty(LPCWSTR name, TGetter getter, TSetter setter)
        {
            Js::PropertyRecord const * propertyRecord;
            _scriptContext->GetOrAddPropertyRecord(name, (int)wcslen(name), &propertyRecord);
            auto getterFunc = arena_new<JsDirectFunc< FuncSignature<TGetType>, TGetter>>(_alloc, _alloc, _scriptContext, name, getter);
            auto setterFunc = arena_new<JsDirectFunc< FuncSignature<void_t, TSetType>, TSetter>>(_alloc, _alloc, _scriptContext, name, setter);
            AsRecyclableObject()->SetAccessors(propertyRecord->GetPropertyId(), getterFunc->JsFunc(), setterFunc->JsFunc());
        }

        template <typename TSignature, class THandler>
        void ExposeFunction(LPCWSTR name, THandler handler)
        {
            auto directFunc = JsDirectFunc<TSignature, THandler>::New(_alloc, _scriptContext, name, handler);
            SetField(name, directFunc->JsFunc());
        }

        template<typename TType>
        void ExposeField(LPCWSTR name, TType value)
        {
            SetField<TType>(name, value);
        }

        template<typename T>
        T GetProperty(Js::RecyclableObject* obj, LPCWSTR name)
        {
            return JsHelpers::GetProperty<T>(obj, name, _alloc, _scriptContext);
        }

        // Allows to manage on demand property values which are cached in a private field.
        class OnDemandProperty
        {
        public:
            template<typename PropertyType>
            static PropertyType Get(LPCWSTR name, JsCallable* parent) 
            {
                Assert(parent);
                auto result = JsHelpers::GetProperty<PropertyType>(parent->AsRecyclableObject(), name, parent->Alloc(), parent->GetScriptContext());
                return (!result || JsHelpers::IsNullOrUndefined(result)) ? nullptr: result;
            }

            template<typename PropertyType>
            static void Set(LPCWSTR name, JsCallable* parent, PropertyType value)
            {
                Assert(parent);
                JsHelpers::SetField(parent->AsRecyclableObject(), name, value, parent->GetScriptContext());
            }

            template<typename PropertyType, typename TCreate>
            static PropertyType GetOrCreate(LPCWSTR name, JsCallable* parent, TCreate createHandler) 
            {
                Assert(parent);
                auto result = Get<PropertyType>(name, parent);
                if(!result)
                {
                    result = createHandler();
                    Set<PropertyType>(name, parent, result);
                }
                return result;
            }
        };

    private:

        template<typename TType>
        void SetField(LPCWSTR name, TType value)
        {
            JsHelpers::SetField<TType>(this->AsRecyclableObject(), name, value, _scriptContext);
        }
    };
}
