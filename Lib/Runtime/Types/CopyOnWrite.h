// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#define VERIFY_COPY_ON_WRITE_ENABLED() if (!BinaryFeatureControl::LanguageService() && !CONFIG_FLAG(CopyOnWriteTest)){ Assert(false); return; }
#define VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(b) if (!BinaryFeatureControl::LanguageService() && !CONFIG_FLAG(CopyOnWriteTest)){ Assert(false); return b; }
#define VERIFY_COPY_ON_WRITE_ENABLED_RET() VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(nullptr)

namespace Js {

    class NoSpecialProperties
    {
    public:
        static inline bool IsSpecialProperty(PropertyId id) { return false; }
    };

    template<class T, class SpecialProperty = NoSpecialProperties>
    class CopyOnWriteObject : public T {
        T* proxiedObject;

    protected:
        DEFINE_VTABLE_CTOR(CopyOnWriteObject, T);

    private:
        void DetachIfAccessor(PropertyId propertyId)
        {
            VERIFY_COPY_ON_WRITE_ENABLED();

            // Force detatch if the property is an getter or setter function
            if (proxiedObject && !proxiedObject->HasOnlyWritableDataProperties() && proxiedObject->GetPropertyIndex(propertyId) == Constants::NoSlot)
                Detach();
        }

    public:
        CopyOnWriteObject(DynamicType* type, T* proxiedObject, ScriptContext* scriptContext) : T(type),
            proxiedObject(proxiedObject)
        {
            VERIFY_COPY_ON_WRITE_ENABLED();
            if (!proxiedObject->HasOnlyWritableDataProperties())
            {
                this->GetTypeHandler()->ClearHasOnlyWritableDataProperties();
            }
        }

        static inline CopyOnWriteObject * New(Recycler * recycler, DynamicType * type, T * proxiedObject, ScriptContext * scriptContext)
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET();
            size_t inlineSlotsSize = type->GetTypeHandler()->GetInlineSlotsSize();
            if (inlineSlotsSize)
            {
                return RecyclerNewPlusZ(recycler, inlineSlotsSize, CopyOnWriteObject, type, proxiedObject, scriptContext);
            }
            else
            {
                return RecyclerNew(recycler, CopyOnWriteObject, type, proxiedObject, scriptContext);
            }
        }

        template<typename U>
        bool IsJavascriptProxy()
        {
            return false;
        }

        template<>
        bool IsJavascriptProxy<JavascriptProxy>()
        {
            return true;
        }

        friend class AutoProxyResetter;

        class AutoProxyResetter
        {
        public:
            AutoProxyResetter(CopyOnWriteObject<T, SpecialProperty>* parent) : m_parent(parent), m_savedProxiedObject(parent->proxiedObject)
            {
                this->m_parent->proxiedObject = nullptr;
            }
            ~AutoProxyResetter()
            {
                this->m_parent->proxiedObject = this->m_savedProxiedObject;
            }
            T* GetProxiedObject() { return this->m_savedProxiedObject; }
        private:
            CopyOnWriteObject<T, SpecialProperty>* m_parent;
            T* m_savedProxiedObject;
        };

        void Detach()
        {
            VERIFY_COPY_ON_WRITE_ENABLED();
            
            if (proxiedObject)
            {
                if (IsJavascriptProxy<T>())
                {
                    this->proxiedObject = nullptr;
                    return;
                }

                T::PrepareDetach(proxiedObject);
                
                //
                // Special processing logic for WeakMap
                //
                // In order to function, WeakMap requires all its key to have an internal property InternalPropertyIds::WeakMapKeyMap
                // We could not set that property during MakeCopyOnWriteObject because SetInternalProperty will detach and that may run setter.
                // So that work is postpone here, and this is the code to handle the postponed work
                //
                {
                    //
                    // GetInternalProperty/SetInternalProperty is going to Detach, if proxied object is still non null at point, we will run into infinte recursion.
                    // So we temporaily set it to nullptr at this point and restore it back once the operation is completed
                    //
                    AutoProxyResetter proxySaver(this);
                    Var sourceWeakMapKeyData = nullptr;
                    if (proxySaver.GetProxiedObject()->GetInternalProperty(proxySaver.GetProxiedObject(), InternalPropertyIds::WeakMapKeyMap, &sourceWeakMapKeyData, nullptr, nullptr))
                    {
                        Var targetWeakMapKeyData = JavascriptWeakMap::CopyWeakMapKeyMapForCopyOnWrite(this->GetScriptContext(), sourceWeakMapKeyData);
                        BOOL success = this->SetInternalProperty(InternalPropertyIds::WeakMapKeyMap, targetWeakMapKeyData, PropertyOperation_Force, nullptr);
                        Assert(success);
                    }
                }

                Var enumeratorVar;
                ScriptContext *scriptContext = GetScriptContext();
                if (proxiedObject->GetEnumerator(true, &enumeratorVar, scriptContext, false, true))
                {
                    JavascriptEnumerator* enumerator = (JavascriptEnumerator*)enumeratorVar;
                    T *originalObject = proxiedObject;
                    proxiedObject = nullptr;
                    while (enumerator->MoveNext())
                    {
                        Var index = enumerator->GetCurrentIndex();

                        PropertyId id;
                        if (enumerator->GetCurrentPropertyId(&id))
                        {
                            Var getter;
                            Var setter;

                            if (originalObject->GetAccessors(id, &getter, &setter, originalObject->GetScriptContext()))
                            {
                                // This is a getter and/or setter property.
                                Js::PropertyDescriptor propertyDescriptor;
                                propertyDescriptor.SetEnumerable(originalObject->IsEnumerable(id) ? true : false);
                                propertyDescriptor.SetConfigurable(originalObject->IsConfigurable(id) ? true : false);
                                // Ignore Writable because specifying turns the descriptor into a data descriptor
                                // which ignores the setter and getter.
                                if (getter)
                                    propertyDescriptor.SetGetter(scriptContext->CopyOnWrite(getter));
                                if (setter)
                                    propertyDescriptor.SetSetter(scriptContext->CopyOnWrite(setter));
                                JavascriptOperators::DefineOwnPropertyDescriptor(this, id, propertyDescriptor, false, GetScriptContext());
                                continue;
                            }
                            PropertyAttributes attributes =
                                (originalObject->IsEnumerable(id) ? PropertyEnumerable : PropertyNone) |
                                (originalObject->IsWritable(id) ? PropertyWritable : PropertyNone) |
                                (originalObject->IsConfigurable(id) ? PropertyConfigurable : PropertyNone);

                            Var value = scriptContext->CopyOnWrite(enumerator->GetCurrentValue());
                            this->SetPropertyWithAttributes(id, value, attributes, nullptr);
                            continue;
                        }

                        Var value = scriptContext->CopyOnWrite(enumerator->GetCurrentValue());
                        JavascriptOperators::OP_SetElementI(this, index, value, GetScriptContext());
                    }
                }

                proxiedObject = nullptr;
            }
        }

        virtual bool HasReadOnlyPropertiesInvisibleToTypeHandler() override { return true; }

        virtual BOOL HasProperty(PropertyId propertyId) override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject && !SpecialProperty::IsSpecialProperty(propertyId))
            {
                return proxiedObject->HasProperty(propertyId);
            }
            else return T::HasProperty(propertyId);
        }

        virtual BOOL HasOwnProperty(PropertyId propertyId) override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject && !SpecialProperty::IsSpecialProperty(propertyId))
            {
                return proxiedObject->HasOwnProperty(propertyId);
            }
            else return T::HasOwnProperty(propertyId);
        }

        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject)
            {
                auto result = proxiedObject->GetProperty(originalInstance, propertyNameString, value, info, requestContext);
                if (result)
                {
                    auto scriptContext = GetScriptContext();
                    if (scriptContext->GetCopyOnGetEnabled())
                    {
                        Detach();
                    }
                    *value = scriptContext->CopyOnWrite(*value);
                    PropertyValueInfo::SetNoCache(info, this);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                // TODO Windows 8 Bug 733223: Enable property get and set caching for detached copy-on-write objects.
                // It's ok to cache property reads from the instance after the copy-on-write object has been detached.
                // Even the special property (if any) is simply stored in one of the property slots.  It's not ok to cache
                // reads from the prototype, because the prototype itself may be a still attached copy-on-write object.
                // Care must also be taken when a detached copy-on-write object is used as prototype by a still attached
                // copy-on-write object.  For now we disable all caching for copy-on-write objects.
                auto result =  T::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
                PropertyValueInfo::SetNoCache(info, this);
                return result;
            }
        }

        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            DetachIfAccessor(propertyId);

            if (proxiedObject && !SpecialProperty::IsSpecialProperty(propertyId))
            {
                BOOL result = proxiedObject->GetProperty(originalInstance, propertyId, value, info, requestContext);
                if (result)
                {
                    ScriptContext *scriptContext = GetScriptContext();
                    if (scriptContext->GetCopyOnGetEnabled())
                    {
                        Detach();
                    }
                    *value = scriptContext->CopyOnWrite(*value);
                    PropertyValueInfo::SetNoCache(info, this);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                // TODO Windows 8 Bug 733223: Enable property get and set caching for detached copy-on-write objects.
                // It's ok to cache property reads from the instance after the copy-on-write object has been detached.
                // Even the special property (if any) is simply stored in one of the property slots.  It's not ok to cache
                // reads from the prototype, because the prototype itself may be a still attached copy-on-write object.
                // Care must also be taken when a detached copy-on-write object is used as prototype by a still attached
                // copy-on-write object.  For now we disable all caching for copy-on-write objects.
                const BOOL result = T::GetProperty(originalInstance, propertyId, value, info, requestContext);
                PropertyValueInfo::SetNoCache(info, this);
                return result;
            }
        }

        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            DetachIfAccessor(propertyId);

            if (proxiedObject && !SpecialProperty::IsSpecialProperty(propertyId))
            {
                BOOL result = proxiedObject->GetPropertyReference(originalInstance, propertyId, value, info, requestContext);
                if (result)
                {
                    ScriptContext *scriptContext = GetScriptContext();
                    if (scriptContext->GetCopyOnGetEnabled())
                    {
                        Detach();
                    }
                    *value = scriptContext->CopyOnWrite(*value);
                    PropertyValueInfo::SetNoCache(info, this);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                // TODO Windows 8 Bug 733223: Enable property get and set caching for detached copy-on-write objects.
                // It's ok to cache property reads from the instance after the copy-on-write object has been detached.
                // Even the special property (if any) is simply stored in one of the property slots.  It's not ok to cache
                // reads from the prototype, because the prototype itself may be a still attached copy-on-write object.
                // Care must also be taken when a detached copy-on-write object is used as prototype by a still attached
                // copy-on-write object.  For now we disable all caching for copy-on-write objects.
                const BOOL result = T::GetPropertyReference(originalInstance,propertyId,value,info,requestContext);
                PropertyValueInfo::SetNoCache(info, this);
                return result;
            }
        }

        virtual BOOL GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject)
            {
                BOOL result = proxiedObject->GetItem(originalInstance, index, value, requestContext);
                if (result)
                {
                    ScriptContext *scriptContext = GetScriptContext();
                    *value = scriptContext->CopyOnWrite(*value);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return T::GetItem(originalInstance,index,value,requestContext);
            }
        }

        virtual BOOL GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject)
            {
                BOOL result = proxiedObject->GetItemReference(originalInstance, index, value, requestContext);
                if (result)
                {
                    ScriptContext *scriptContext = GetScriptContext();
                    *value = scriptContext->CopyOnWrite(*value);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return T::GetItemReference(originalInstance,index, value, requestContext);
            }
        }

        virtual DescriptorFlags GetItemSetter(uint32 index, Var* setterValue, ScriptContext* requestContext) override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(DescriptorFlags::None);
            Detach();
            return T::GetItemSetter(index, setterValue, requestContext);
        }

        virtual BOOL ToPrimitive(JavascriptHint hint, Var* result, ScriptContext * requestContext) override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject)
            {
                BOOL res=proxiedObject->ToPrimitive(hint, result, requestContext);
                if (res)
                {
                    ScriptContext *scriptContext = GetScriptContext();
                    *result = scriptContext->CopyOnWrite(*result);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return T::ToPrimitive(hint, result, requestContext);
            }

        }

        virtual BOOL IsWritable(PropertyId propertyId) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject!=nullptr && !SpecialProperty::IsSpecialProperty(propertyId)) 
            {
                return proxiedObject->IsWritable(propertyId);
            }
            else return T::IsWritable(propertyId);
        }

        virtual BOOL IsConfigurable(PropertyId propertyId) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject!=nullptr && !SpecialProperty::IsSpecialProperty(propertyId)) 
            {
                return proxiedObject->IsConfigurable(propertyId);
            }
            else return T::IsConfigurable(propertyId);
        }

        virtual BOOL IsEnumerable(PropertyId propertyId) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject!=nullptr && !SpecialProperty::IsSpecialProperty(propertyId)) 
            {
                return proxiedObject->IsEnumerable(propertyId);
            }
            else return T::IsEnumerable(propertyId);
        }

        virtual BOOL GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::GetAccessors(propertyId,getter,setter,requestContext);
        }

        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            // TODO Windows 8 Bug 733223: Enable property get and set caching for detached copy-on-write objects.
            // It's ok to cache property sets after the copy-on-write object has been detached.  Even the special
            // property (if any) is simply stored in one of the property slots.  Care must be taken when setters
            // are involved.  Conversely, if this property set resulted in detachment (and a corresponding sequence
            // of type transitions), we cannot cache this change in the type-without-property cache, because
            // the properties added by Detach on the slow path would not get copied on the fast path.
            // For now we disable all caching for copy-on-write objects.
            BOOL result = T::SetProperty(propertyId,value,flags,info);
            PropertyValueInfo::SetNoCache(info, this);
            return result;
        }

        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            // See for justification for clearing the cache
            BOOL result = T::SetProperty(propertyNameString, value, flags, info);
            PropertyValueInfo::SetNoCache(info, this);
            return result;
        }

        virtual BOOL GetInternalProperty(Var instance, PropertyId internalPropertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (internalPropertyId == InternalPropertyIds::WeakMapKeyMap)
            {
                //
                // It is possible that a key passed to a WeakMap API (e.g. get) is not detached, in that case, the 
                // WeakMapKeyMap is not cloned (and translated).
                //
                // See Special processing logic for WeakMap for more information about this fix.
                // This only impact objects that are key for a WeakMap (or being checked whether it is one)
                //
                Detach();
            }

            return T::GetInternalProperty(instance, internalPropertyId, value, info, requestContext);
        }

        virtual BOOL SetInternalProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            // TODO Windows 8 Bug 733223: Enable property get and set caching for detached copy-on-write objects.
            // It's ok to cache property sets after the copy-on-write object has been detached.  Even the special
            // property (if any) is simply stored in one of the property slots.  Care must be taken when setters
            // are involved.  Conversely, if this property set resulted in detachment (and a corresponding sequence
            // of type transitions), we cannot cache this change in the type-without-property cache, because
            // the properties added by Detach on the slow path would not get copied on the fast path.
            // For now we disable all caching for copy-on-write objects.
            BOOL result = T::SetInternalProperty(propertyId, value, flags, info);
            PropertyValueInfo::SetNoCache(info, this);
            return result;
        }

        virtual BOOL InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info = nullptr) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::InitProperty(propertyId,value,flags,info);
        }

        virtual BOOL SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags = PropertyOperation_None, SideEffects possibleSideEffects = SideEffects_Any) override sealed 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::SetPropertyWithAttributes(propertyId,value,attributes,info,flags,possibleSideEffects);
        }

        virtual int GetPropertyCount() override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(0);
            Detach();
            return T::GetPropertyCount();
        }

        virtual PropertyId GetPropertyId(PropertyIndex index) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(Js::Constants::NoProperty);
            Detach();
            return T::GetPropertyId(index);
        }

        virtual PropertyId GetPropertyId(BigPropertyIndex index) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(Js::Constants::NoProperty);
            Detach();
            return T::GetPropertyId(index);
        }

        virtual DescriptorFlags GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(DescriptorFlags::None);
            Detach();
            return T::GetSetter(propertyId,setterValue,info,requestContext);
        }

        virtual DescriptorFlags GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(DescriptorFlags::None);
            Detach();
            return T::GetSetter(propertyNameString, setterValue, info, requestContext);
        }

        virtual BOOL DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::DeleteProperty(propertyId,flags);
        }

        virtual BOOL HasItem(uint32 index) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject)
            {
                return proxiedObject->HasItem(index);
            }
            else return T::HasItem(index);
        }

        virtual BOOL HasOwnItem(uint32 index) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            if (proxiedObject)
            {
                return proxiedObject->HasOwnItem(index);
            }
            else return T::HasOwnItem(index);
        }

        virtual BOOL SetItem(uint32 index, Var value, PropertyOperationFlags flags) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::SetItem(index,value,flags);
        }

        virtual BOOL DeleteItem(uint32 index, PropertyOperationFlags flags) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::DeleteItem(index,flags);
        }

        virtual BOOL GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext* scriptContext, bool preferSnapshotSemantics = true, bool enumSymbols = false) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::GetEnumerator(enumNonEnumerable, enumerator, scriptContext, preferSnapshotSemantics, enumSymbols);
        }

        virtual BOOL SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags = PropertyOperation_None) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::SetAccessors(propertyId,getter,setter,flags);
        }

        virtual BOOL SetEnumerable(PropertyId propertyId, BOOL value) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::SetEnumerable(propertyId,value);
        }

        virtual BOOL SetWritable(PropertyId propertyId, BOOL value) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::SetWritable(propertyId,value);
        }

        virtual BOOL SetConfigurable(PropertyId propertyId, BOOL value) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::SetConfigurable(propertyId,value);
        }

        virtual BOOL SetAttributes(PropertyId propertyId, PropertyAttributes attributes) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::SetAttributes(propertyId,attributes);
        }

        virtual BOOL IsExtensible() override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::IsExtensible();
        }

        virtual BOOL PreventExtensions() override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::PreventExtensions();
        }

        virtual BOOL Seal() override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::Seal();
        }

        virtual BOOL Freeze() override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::Freeze();
        }

        virtual BOOL IsSealed() override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::IsSealed();
        }

        virtual BOOL IsFrozen() override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::IsFrozen();
        }

        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::GetDiagValueString(stringBuilder, requestContext);
        }

        virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET_VALUE(false);
            Detach();
            return T::GetDiagTypeString(stringBuilder, requestContext);
        }

        virtual Var GetTypeOfString(ScriptContext * requestContext) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET();
            Detach();
            return T::GetTypeOfString(requestContext);
        }

        virtual void RemoveFromPrototype(ScriptContext * requestContext) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED();
            Detach();
            return T::RemoveFromPrototype(requestContext);
        }

        virtual void SetPrototype(RecyclableObject* newPrototype) override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED();
            Detach();
            return T::SetPrototype(newPrototype);
        }

        virtual RecyclableObject* GetProxiedObjectForHeapEnum() override 
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET();
            Assert(this->GetScriptContext()->IsHeapEnumInProgress());
            return proxiedObject;
        }

        virtual BOOL IsCopyOnWriteObject() override sealed
        {            
            return TRUE;
        }

        virtual BOOL IsCopyOnWriteProxy() override
        {         
            return proxiedObject != nullptr;
        }
    };
}