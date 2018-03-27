/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include <EnginePch.h>

#include "RegexFlags.h"

namespace Js
{
    C_ASSERT(::DescriptorFlags_None == None);
    C_ASSERT(::DescriptorFlags_Accessor == Accessor);
    C_ASSERT(::DescriptorFlags_Data == Data);
    C_ASSERT(::DescriptorFlags_Writable == Writable);

    C_ASSERT(::CallFlags_None == Js::CallFlags_None);
    C_ASSERT(::CallFlags_New == Js::CallFlags_New);
    C_ASSERT(::CallFlags_Value == Js::CallFlags_Value);
    C_ASSERT(::CallFlags_Eval == Js::CallFlags_Eval);
    C_ASSERT(::CallFlags_ExtraArg == Js::CallFlags_ExtraArg);
    C_ASSERT(::CallFlags_NotUsed == Js::CallFlags_NotUsed);
    C_ASSERT(::CallFlags_Wrapped == Js::CallFlags_Wrapped);
    C_ASSERT(::CallFlags_CallPut == Js::CallFlags_NewTarget);

    C_ASSERT(::RegexFlags_None == UnifiedRegex::NoRegexFlags);
    C_ASSERT(::RegexFlags_IgnoreCase == UnifiedRegex::IgnoreCaseRegexFlag);
    C_ASSERT(::RegexFlags_Global == UnifiedRegex::GlobalRegexFlag);
    C_ASSERT(::RegexFlags_Multiline == UnifiedRegex::MultilineRegexFlag);
    C_ASSERT(::RegexFlags_Unicode == UnifiedRegex::UnicodeRegexFlag);
    C_ASSERT(::RegexFlags_Sticky == UnifiedRegex::StickyRegexFlag);
    C_ASSERT(::RegexFlags_All == UnifiedRegex::AllRegexFlags);

    OperationUsage DefaultScriptOperations::defaultUsage =
    {
       OperationFlag_all,
       OperationFlag_none,
       OperationFlagsForNamespaceOrdering_none
    };

    DefaultScriptOperations DefaultScriptOperations::s_DefaultScriptOperations;

    DefaultScriptOperations::DefaultScriptOperations()
    {
    }

    DefaultScriptOperations::~DefaultScriptOperations()
    {
    }

    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        IfNullReturnError(ppvObject, E_INVALIDARG);
        *ppvObject = nullptr;

        if (riid == IID_IUnknown ||
            riid == _uuidof(ITypeOperations) ||
            riid == IID_IDefaultScriptOperations)
        {
            *ppvObject = (ITypeOperations*)this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE DefaultScriptOperations::AddRef(void)
    {
        return 1;
    }

    ULONG STDMETHODCALLTYPE DefaultScriptOperations::Release()
    {
        return 1;
    }

    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::GetOperationUsage(
        /* [out] */ OperationUsage *usageRef)
    {
        *usageRef = defaultUsage;
        return NOERROR;
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::HasOwnProperty(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)objInstance;
                *result = JavascriptConversion::PropertyQueryFlagsToBoolean(customExternalObject->ExternalObject::HasPropertyQuery(propertyId));
            }
            else
            {
                *result = objInstance->HasOwnProperty(propertyId);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, NOERROR);

    }


    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::GetOwnProperty(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent)
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(propertyPresent, E_INVALIDARG);
        *propertyPresent = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)objInstance;
                *propertyPresent = JavascriptConversion::PropertyQueryFlagsToBoolean(customExternalObject->ExternalObject::GetPropertyQuery(instance, propertyId, value, NULL, scriptContext));
            }
            else
            {
                *propertyPresent = objInstance->GetProperty(objInstance, propertyId, value, NULL, scriptContext);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, NOERROR);
    }

    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::GetPropertyReference(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent)
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(propertyPresent, E_INVALIDARG);
        *propertyPresent = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject* customExternalObject = (Js::CustomExternalObject*)instance;
                *propertyPresent = JavascriptConversion::PropertyQueryFlagsToBoolean(customExternalObject->ExternalObject::GetPropertyReferenceQuery(instance, propertyId, value, NULL, objInstance->GetScriptContext()));
            }
            else
            {
                *propertyPresent = objInstance->GetPropertyReference(instance, propertyId, value, NULL, objInstance->GetScriptContext());
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, NOERROR);
    }


    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::SetProperty(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var value,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *result = customExternalObject->ExternalObject::SetProperty(propertyId, value, PropertyOperation_NonFixedValue, NULL);
            }
            else
            {
                *result = objInstance->SetProperty(propertyId, value, PropertyOperation_None, NULL);
            }
            return NOERROR;
    };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, NOERROR);
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::SetPropertyWithAttributes(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var value,
        /* [in] */ ::PropertyAttributes attributes,
        /* [in] */ ::SideEffects effects,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *result = customExternalObject->ExternalObject::SetPropertyWithAttributes(propertyId, value, (Js::PropertyAttributes) attributes, NULL, PropertyOperation_NonFixedValue, (Js::SideEffects) effects);
            }
            else
            {
                *result = objInstance->SetPropertyWithAttributes(propertyId, value, (Js::PropertyAttributes) attributes, NULL, PropertyOperation_None, (Js::SideEffects) effects);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, NOERROR);
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::DeleteProperty(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *result = customExternalObject->ExternalObject::DeleteProperty(propertyId, PropertyOperation_None);
            }
            else
            {
                *result = objInstance->DeleteProperty(propertyId, PropertyOperation_None);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }


    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::HasOwnItem(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *result = JavascriptConversion::PropertyQueryFlagsToBoolean(customExternalObject->ExternalObject::HasItemQuery(JavascriptConversion::ToUInt32(index, scriptContext)));
            }
            else
            {
                *result = objInstance->HasOwnItem(JavascriptConversion::ToUInt32(index, scriptContext));
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, NOERROR);
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::GetOwnItem(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ Var *value,
        /* [out] */ BOOL *itemPresent)
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(itemPresent, E_INVALIDARG);
        *itemPresent = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *itemPresent = JavascriptConversion::PropertyQueryFlagsToBoolean(customExternalObject->ExternalObject::GetItemQuery(instance, JavascriptConversion::ToUInt32(index, scriptContext), value, scriptContext));
            }
            else
            {
                *itemPresent = objInstance->GetItem(instance, JavascriptConversion::ToUInt32(index, scriptContext), value, scriptContext);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, NOERROR);
    }
    
    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::SetItem(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [in] */ Var value,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *result = customExternalObject->ExternalObject::SetItem(JavascriptConversion::ToUInt32(index, scriptContext), value, PropertyOperation_None);
            }
            else
            {
                *result = objInstance->SetItem(JavascriptConversion::ToUInt32(index, scriptContext), value, PropertyOperation_None);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, NOERROR);
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::DeleteItem(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *result = customExternalObject->ExternalObject::DeleteItem(JavascriptConversion::ToUInt32(index, scriptContext), PropertyOperation_None);
            }
            else
            {
                *result = objInstance->DeleteItem(JavascriptConversion::ToUInt32(index, scriptContext), PropertyOperation_None);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, NOERROR);
    }

    class JavascriptEnumeratorWrapper : public Js::JavascriptStaticEnumerator
    {
    public:
        JavascriptEnumeratorWrapper() : currentIndex(nullptr), currentPropertyId(Constants::NoProperty) {};

        BOOL MoveNext(PropertyAttributes * attributes = nullptr)
        {
            currentIndex = this->MoveAndGetNext(currentPropertyId, attributes);
            if (currentIndex == nullptr)
            {
                currentPropertyId = Constants::NoProperty;
                return FALSE;
            }
            return true;
        }
        Var GetCurrentIndex()
        {
            return currentIndex;
        }
        bool GetCurrentPropertyId(PropertyId * propertyId)
        {
            *propertyId = currentPropertyId;
            return currentPropertyId != Constants::NoProperty;
        }
    private:
        Var currentIndex;
        PropertyId currentPropertyId;
    };

    // To avoid circularity, this returns an enumerator of only the internal properties of an object, even if the object has
    // an external type.  The IVarEnumerator must be released by its consumer.

    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::GetEnumerator(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ BOOL enumNonEnumerable,
        /* [in] */ BOOL enumSymbols,
        /* [out] */ IVarEnumerator **enumerator)
    {
        IfNullReturnError(enumerator, E_INVALIDARG);
        *enumerator = nullptr;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            HRESULT hrLocal = NOERROR;
            BOOL result;
            EnumeratorFlags flags = EnumeratorFlags::SnapShotSemantics;
            if (enumNonEnumerable)
            {
                flags |= EnumeratorFlags::EnumNonEnumerable;
            }
            if (enumSymbols)
            {
                flags |= EnumeratorFlags::EnumSymbols;
            }
            JavascriptEnumeratorWrapper * internalEnum = RecyclerNew(scriptContext->GetRecycler(), JavascriptEnumeratorWrapper);
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                result = customExternalObject->ExternalObject::GetEnumerator(internalEnum, flags, scriptContext);
            }
            else
            {
                result = objInstance->GetEnumerator(internalEnum, flags, scriptContext);
            }
            Assert(!result || (internalEnum != nullptr));
            if (result && !internalEnum->IsNullEnumerator())
            {
                CVarEnumerator * externalEnum = HeapNew(CVarEnumerator, internalEnum, scriptContext);
                hrLocal = externalEnum->QueryInterface(__uuidof(IVarEnumerator), (void**)enumerator);
                Assert(SUCCEEDED(hrLocal));
            }
            else
            {
                *enumerator = &CVarNullEnumerator::Instance;
            }
            return hrLocal;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::IsEnumerable(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *result = customExternalObject->ExternalObject::IsEnumerable(propertyId);
            }
            else
            {
                *result = objInstance->IsEnumerable(propertyId);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }
    
    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::IsWritable(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *result = customExternalObject->ExternalObject::IsWritable(propertyId);
            }
            else
            {
                *result = objInstance->IsWritable(propertyId);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }
    
    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::IsConfigurable(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *result = customExternalObject->ExternalObject::IsConfigurable(propertyId);
            }
            else
            {
                *result = objInstance->IsConfigurable(propertyId);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::SetEnumerable(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value)
    {
        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                customExternalObject->ExternalObject::SetEnumerable(propertyId, value);
            }
            else
            {
                objInstance->SetEnumerable(propertyId,value);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::SetWritable(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value)
    {
        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                customExternalObject->ExternalObject::SetWritable(propertyId, value);
            }
            else
            {
                objInstance->SetWritable(propertyId,value);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::SetConfigurable(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value)
    {
        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                customExternalObject->ExternalObject::SetConfigurable(propertyId, value);
            }
            else
            {
                objInstance->SetConfigurable(propertyId,value);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }
    
    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::SetAccessors(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var getter,
        /* [in] */ Var setter)
    {
        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                customExternalObject->ExternalObject::SetAccessors(propertyId, getter, setter, PropertyOperation_NonFixedValue);
            }
            else
            {
                objInstance->SetAccessors(propertyId, getter, setter);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }
    
    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::GetAccessors(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* getter,
        /* [out] */ Var* setter,
        /* [out] */ BOOL* result)
    {
        IfNullReturnError(getter, E_INVALIDARG);
        *getter = nullptr;
        IfNullReturnError(setter, E_INVALIDARG);
        *setter = nullptr;
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject* customExternalObject = (Js::CustomExternalObject*)instance;
                *result = customExternalObject->ExternalObject::GetAccessors(propertyId, getter, setter, scriptContext);
            }
            else
            {
                *result = objInstance->GetAccessors(propertyId, getter, setter, scriptContext);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }


    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::GetSetter(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* setter,
        /* [out] */ ::DescriptorFlags* flags)
    {
        IfNullReturnError(setter, E_INVALIDARG);
        *setter = nullptr;
        IfNullReturnError(flags, E_INVALIDARG);
        *flags = DescriptorFlags_None;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *flags = (::DescriptorFlags)customExternalObject->ExternalObject::GetSetter(propertyId, setter, NULL, scriptContext);
            }
            else
            {
                *flags = (::DescriptorFlags)objInstance->GetSetter(propertyId, setter, NULL, scriptContext);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::GetItemSetter(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ Var* setter,
        /* [out] */ ::DescriptorFlags* flags)
    {
        IfNullReturnError(setter, E_INVALIDARG);
        *setter = nullptr;
        IfNullReturnError(flags, E_INVALIDARG);
        *flags = DescriptorFlags_None;

        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *flags = (::DescriptorFlags)(customExternalObject->ExternalObject::GetItemSetter(
                    JavascriptConversion::ToUInt32(index, scriptContext), setter, scriptContext));
            }
            else
            {
                *flags = (::DescriptorFlags)(objInstance->GetItemSetter(
                    JavascriptConversion::ToUInt32(index, scriptContext), setter, scriptContext));
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }


    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::Equals(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var other,
        /* [in] */ BOOL* result)
    {
        if (TaggedNumber::Is(instance) || TaggedNumber::Is(other))
        {
            *result = (instance == other);
            return NOERROR;
        }
        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            BOOL ret = false;
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                ret = customExternalObject->ExternalObject::Equals(other, result, scriptContext);
            }
            else
            {
                ret = objInstance->Equals(other, result, scriptContext);
            }
            if (!ret)
            {
                return E_FAIL;
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }


    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::StrictEquals(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var other,
        /* [in] */ BOOL* result)
    {
        IfNullReturnError(instance, E_INVALIDARG);
        IfNullReturnError(other, E_INVALIDARG);
        if (TaggedNumber::Is(instance) || TaggedNumber::Is(other))
        {
            *result = (instance == other);
            return NOERROR;
        }
        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            BOOL ret = false;
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                ret = customExternalObject->ExternalObject::StrictEquals(other, result, scriptContext);
            }
            else
            {
                ret = objInstance->StrictEquals(other, result, scriptContext);
            }
            if (!ret)
            {
                return E_FAIL;
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }


    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::QueryObjectInterface(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ REFIID riid,
        /* [out] */ void **ppvObj)
    {
        IfNullReturnError(ppvObj, E_INVALIDARG);
        *ppvObj = nullptr;

        IfNullReturnError(instance, E_INVALIDARG);
        HRESULT hr = NOERROR;
        if (TaggedNumber::Is(instance))
        {
            return E_INVALIDARG;
        }

        RecyclableObject* objInstance = RecyclableObject::FromVar(instance);
        if (objInstance->IsExternal())
        {
            Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)objInstance;
            hr = customExternalObject->ExternalObject::QueryObjectInterface(riid, ppvObj);
        }
        else
        {
            // TODO: investigate whether to pass IDispatchEx through here
            hr = E_NOINTERFACE;
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::GetInitializer(
        /* [out] */ InitializeMethod * initializer,
        /* [out] */ int * initSlotCapacity,
        /* [out] */ BOOL * hasAccessors)
    {
        IfNullReturnError(initializer, E_INVALIDARG);
        *initializer = nullptr;
        IfNullReturnError(initSlotCapacity, E_INVALIDARG);
        *initSlotCapacity = 0;
        IfNullReturnError(hasAccessors, E_INVALIDARG);
        *hasAccessors = FALSE;

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::GetFinalizer(
        /* [out] */ FinalizeMethod * finalizer)
    {
        IfNullReturnError(finalizer, E_INVALIDARG);
        *finalizer = nullptr;

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE
    DefaultScriptOperations::HasInstance(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var constructor,
        /* [in] */ Var instance,
        /* [out] */ BOOL* result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        IfNullReturnError(constructor, E_INVALIDARG);
        IfNullReturnError(instance, E_INVALIDARG);
        RecyclableObject* objConstructor = RecyclableObject::FromVar(constructor);
        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objConstructor->IsExternal() && ExternalObject::FromVar(constructor)->IsCustomExternalObject())
            {
                Var funcPrototype = JavascriptOperators::GetProperty(objConstructor, PropertyIds::prototype, scriptContext);
                *result = JavascriptFunction::HasInstance(funcPrototype, instance, scriptContext, NULL, NULL);
            }
            else
            {
                *result = objConstructor->HasInstance(instance, scriptContext);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, constructor, fn, E_INVALIDARG);
    }

    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::GetNamespaceParent(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [out] */ Var* namespaceParent)
    {
        IfNullReturnError(namespaceParent, E_INVALIDARG);
        *namespaceParent = nullptr;

        AssertMsg(FALSE, "host shouldn't get back to jscript to resolve namespace parent");
        auto fn = [&] (Js::RecyclableObject* objInstance, Js::ScriptContext* scriptContext) -> HRESULT {
            if (objInstance->IsExternal())
            {
                Js::CustomExternalObject * customExternalObject = (Js::CustomExternalObject *)instance;
                *namespaceParent = customExternalObject->ExternalObject::GetNamespaceParent(objInstance);
            }
            else
            {
                *namespaceParent = objInstance->GetNamespaceParent(objInstance);
            }
            return NOERROR;
        };
        return DefaultOperationsWrapper(scriptDirect, instance, fn, E_INVALIDARG);
    }

    HRESULT STDMETHODCALLTYPE DefaultScriptOperations::CrossDomainCheck(
        /* [in] */ IActiveScriptDirect* scriptDirect,
        /* [in] */ Var instance,
        /* [out] */ BOOL* result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        IfNullReturnError(instance, E_INVALIDARG);

        return S_OK;
    }

   HRESULT STDMETHODCALLTYPE DefaultScriptOperations::GetHeapObjectInfo(
       /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance, 
        /* [in] */ ProfilerHeapObjectInfoFlags flags,
        /* [out] */ HostProfilerHeapObject** result,
        /* [out] */ HeapObjectInfoReturnResult* returnResult)
    {
       IfNullReturnError(result, E_INVALIDARG);
       *result = nullptr;
       IfNullReturnError(returnResult, E_INVALIDARG);
       *returnResult = HeapObjectInfoReturnResult_NoResult;

       IfNullReturnError(instance, E_INVALIDARG);

        return S_OK;
    }

 
    Js::ScriptContext *
    DefaultScriptOperations::GetCurrentScriptContext(IActiveScriptDirect* scriptDirect, HostScriptContext ** hostScriptContext)
    {
        Js::ScriptContext* scriptContext = nullptr;
        ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
        if (requestSite != nullptr)
        {
            scriptContext = requestSite->GetScriptSiteContext();
            if (scriptContext->GetThreadContext()->HasPreviousHostScriptContext())
            {
                *hostScriptContext = scriptContext->GetThreadContext()->GetPreviousHostScriptContext();
                scriptContext = (*hostScriptContext)->GetScriptContext();
            }
            else
            {
                *hostScriptContext = nullptr;
            }
        }
        else
        {
            OUTPUT_TRACE_DEBUGONLY(Js::RunPhase, _u("DefaultScriptOperations::GetCurrentScriptContext() - requestSite is null"));
            
#if DBG
            // verify the script state is closed
            SCRIPTSTATE scriptState;
            AssertMsg((static_cast<ScriptEngine*>(scriptDirect))->GetScriptState(&scriptState) == S_OK, "GetScriptState failed");
            AssertMsg(scriptState == SCRIPTSTATE_CLOSED, "Script state not closed");
#endif

            *hostScriptContext = nullptr;
        }

        return scriptContext;
    }

    STDMETHODIMP CVarEnumerator::MoveNext( /*[out]*/ BOOL* itemsAvailable, /*[out,optional]*/ ::PropertyAttributes* attributes)
    {
        IfNullReturnError(itemsAvailable, E_INVALIDARG);
        *itemsAvailable = FALSE;

        HRESULT hr = NOERROR;
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
        {
            *itemsAvailable = internalEnum->MoveNext((Js::PropertyAttributes*)attributes);
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
        VERIFYHRESULTBEFORERETURN(hr, scriptContext);
        return hr;
    }

    STDMETHODIMP CVarEnumerator::GetCurrentName( /*[out]*/ Var* item )
    {
        IfNullReturnError(item, E_INVALIDARG);
        *item = nullptr;

        HRESULT hr = NOERROR;
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
        {
            *item = internalEnum->GetCurrentIndex();
            if ((*item) == nullptr)
            {
                hr = S_FALSE;
            }
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
        VERIFYHRESULTBEFORERETURN(hr, scriptContext);
        return hr;
    }

    STDMETHODIMP CVarEnumerator::GetJavascriptEnumerator(/*[out]*/ Var * enumerator)
    {
        IfNullReturnError(enumerator, E_INVALIDARG);

        *enumerator = this->internalEnum;
        return S_OK;
    }

    CVarEnumerator::CVarEnumerator(Js::JavascriptEnumeratorWrapper * internalEnum, Js::ScriptContext* scriptContext) :
        scriptContext(scriptContext), refCount(0)
    {
        Recycler * recycler = scriptContext->GetRecycler();
        this->internalEnum.Root(internalEnum, recycler);
    }

    CVarEnumerator::~CVarEnumerator()
    {
        this->internalEnum.Unroot(scriptContext->GetRecycler());
    }

    CVarNullEnumerator CVarNullEnumerator::Instance;
}
