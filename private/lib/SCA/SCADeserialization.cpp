//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace Js
{
    template <class Reader>
    bool DeserializationCloner<Reader>::TryClonePrimitive(SrcTypeId typeId, Src src, Dst* dst)
    {
        if (!IsSCAPrimitive(typeId))
        {
            return false;
        }

        ScriptContext* scriptContext = GetScriptContext();
        JavascriptLibrary* lib = scriptContext->GetLibrary();

        switch (typeId)
        {
        case SCA_None:
            *dst = NULL;
            break;

        case SCA_Reference: // Handle reference explictly as a primitive
            {
                scaposition_t pos;
                m_reader->Read(&pos);
                if (!GetEngine()->TryGetClonedObject(pos, dst))
                {
                    ThrowSCADataCorrupt();
                }
            }
            break;

        case SCA_NullValue:
            *dst = lib->GetNull();
            break;

        case SCA_UndefinedValue:
            *dst = lib->GetUndefined();
            break;

        case SCA_TrueValue:
            *dst = lib->GetTrue();
            break;

        case SCA_FalseValue:
            *dst = lib->GetFalse();
            break;

        case SCA_Int32Value:
            {
                int32 n;
                m_reader->Read(&n);
                *dst = JavascriptNumber::ToVar(n, scriptContext);
            }
            break;

        case SCA_DoubleValue:
            {
                double dbl;
                m_reader->Read(&dbl);
                *dst = JavascriptNumber::ToVarWithCheck(dbl, scriptContext);
            }
            break;

        case SCA_Int64Value:
            {
                __int64 n;
                m_reader->Read(&n);
                *dst = JavascriptInt64Number::ToVar(n, scriptContext);
            }
            break;

        case SCA_Uint64Value:
            {
                unsigned __int64 n;
                m_reader->Read(&n);
                *dst = JavascriptUInt64Number::ToVar(n, scriptContext);
            }
            break;

        default:
            return false; // Not a recognized primitive type
        }

        return true;
    }

    template <class Reader>
    bool DeserializationCloner<Reader>::TryCloneObject(SrcTypeId typeId, Src src, Dst* dst, SCADeepCloneType* deepClone)
    {
        ScriptContext* scriptContext = GetScriptContext();
        JavascriptLibrary* lib = scriptContext->GetLibrary();
        *deepClone = SCADeepCloneType::None;
        bool isObject = true;

        if (typeId == SCA_Transferable)
        {
            scaposition_t pos;
            m_reader->Read(&pos);

            *dst = GetEngine()->ClaimTransferable(pos, lib);
            if (*dst == nullptr)
            {
                ThrowSCADataCorrupt();
            }

            return true;
        }

        if (IsSCAHostObject(typeId))
        {
            if (m_pSCAHost)
            {
                HRESULT hr = S_OK;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = m_pSCAHost->CreateObject(m_pSCAContext, typeId, dst);
                }
                END_LEAVE_SCRIPT(scriptContext);
                ThrowIfFailed(hr);

                *deepClone = SCADeepCloneType::Object;
                return true;
            }
            //Can't create host object
            ThrowSCAUnsupported();
        }

        switch (typeId)
        {
        case SCA_StringValue: // Clone string value as object type to resolve multiple references
            {
                const wchar_t* buf;
                charcount_t len;
                ReadString(&buf, &len);
                *dst = Js::JavascriptString::NewWithBuffer(buf, len, scriptContext);
                isObject = false;
            }
            break;

        case SCA_BooleanTrueObject:
            *dst = lib->CreateBooleanObject(TRUE);
            break;

        case SCA_BooleanFalseObject:
            *dst = lib->CreateBooleanObject(FALSE);
            break;

        case SCA_DateObject:
            {
                double dbl;
                m_reader->Read(&dbl);
                *dst = lib->CreateDate(dbl);
            }
            break;

        case SCA_NumberObject:
            {
                double dbl;
                m_reader->Read(&dbl);
                *dst = lib->CreateNumberObjectWithCheck(dbl);
            }
            break;

        case SCA_StringObject:
            {
                const wchar_t* buf;
                charcount_t len;
                ReadString(&buf, &len);
                *dst = lib->CreateStringObject(buf, len);
            }
            break;

        case SCA_RegExpObject:
            {
                const wchar_t* buf;
                charcount_t len;
                ReadString(&buf, &len);

                DWORD flags;
                m_reader->Read(&flags);
                *dst = JavascriptRegExp::CreateRegEx(buf, len,
                    static_cast<UnifiedRegex::RegexFlags>(flags), scriptContext);
            }
            break;

        case SCA_Object:
            {
                *dst = lib->CreateObject();
                *deepClone = SCADeepCloneType::Object;
            }
            break;

        case SCA_Map:
            {
                *dst = JavascriptMap::New(scriptContext);
                *deepClone = SCADeepCloneType::Map;
            }
            break;

        case SCA_Set:
            {
                *dst = JavascriptSet::New(scriptContext);
                *deepClone = SCADeepCloneType::Set;
            }
            break;

        case SCA_DenseArray:
        case SCA_SparseArray:
            {
                uint32 length;
                Read(&length);
                *dst = lib->CreateArray(length);
                *deepClone = SCADeepCloneType::Object;
            }
            break;

        case SCA_ArrayBuffer:
            {
                uint32 len;
                m_reader->Read(&len);
                ArrayBuffer* arrayBuffer = lib->CreateArrayBuffer(len);
                Read(arrayBuffer->GetBuffer(), arrayBuffer->GetByteLength());
                *dst = arrayBuffer;
            }
            break;

        case SCA_Uint8ClampedArray:
            // If Khronos Interop is not enabled, we don't have Uint8ClampedArray available.
            // This is a scenario where the source buffer was created in a newer document mode 
            // but needs to be deserialized in an older document mode. 
            // What we want to do is return the buffer as a CanvasPixelArray instead of 
            // Uint8ClampedArray since the older document mode knows what CanvasPixelArray is but
            // not what Uint8ClampedArray is.
            // We don't support pixelarray in edge anymore.
            // Intentionally fall through to default (TypedArray) label

        default:
            if (IsSCATypedArray(typeId) || typeId == SCA_DataView)
            {
                ReadTypedArray(typeId, dst);
                break;
            }
            return false; // Not a supported object type
        }

#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_RECYCLER_ALLOCATE_OBJECT() && isObject)
        {
            EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(*dst);
        }
#endif
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            *dst = JavascriptProxy::AutoProxyWrapper(*dst);
        }
#endif
        return true;
    }

    template <class Reader>
    void DeserializationCloner<Reader>::CloneProperties(SrcTypeId typeId, Src src, Dst dst)
    {
        ScriptContext* scriptContext = GetScriptContext();
        RecyclableObject* obj = RecyclableObject::FromVar(dst);

        if (obj->IsExternal()) // Read host object properties
        {
            AutoLeaveScriptPtr<SCAPropBag> pPropBag(scriptContext);
            SCAPropBag::CreateInstance(scriptContext, &pPropBag);

            SCAPropBag::PropBagSink sink(pPropBag);
            ReadObjectProperties(&sink);

            HRESULT hr = S_OK;
            {
                AutoLeaveScriptPtr<ISCASerializable> pSCASerializable(scriptContext);
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    IfFailGo(obj->QueryObjectInterface(__uuidof(ISCASerializable), (void**)&pSCASerializable));
                    IfFailGo(pSCASerializable->InitializeObject(m_pSCAContext, pPropBag));
                }
                END_LEAVE_SCRIPT(scriptContext);
Error:
                ; // Fall through
            }
            ThrowIfFailed(hr);
        }
        else // Read native object properties
        {
            // Read array index named properties
            if (typeId == SCA_DenseArray)
            {
                JavascriptArray* arr = JavascriptArray::FromAnyArray(obj); // (might be ES5Array if -ForceES5Array)
                uint32 length = arr->GetLength();
                for (uint32 i = 0; i < length; i++)
                {
                    Dst value = NULL;
                    GetEngine()->Clone(m_reader->GetPosition(), &value);
                    if (value)
                    {
                        arr->DirectSetItemAt(i, value); //Note: no prototype check
                    }
                }
            }
            else if (typeId == SCA_SparseArray)
            {
                JavascriptArray* arr = JavascriptArray::FromAnyArray(obj); // (might be ES5Array if -ForceES5Array)
                while (true)
                {
                    uint32 i;
                    Read(&i);
                    if (i == SCA_PROPERTY_TERMINATOR)
                    {
                        break;
                    }

                    Dst value = NULL;
                    GetEngine()->Clone(m_reader->GetPosition(), &value);
                    if (value == NULL)
                    {
                        ThrowSCADataCorrupt();
                    }

                    arr->DirectSetItemAt(i, value); //Note: no prototype check
                }
            }

            // Read non-index named properties
            ObjectPropertySink sink(scriptContext, obj);
            ReadObjectProperties(&sink);
        }
    }

    template <class Reader>
    void DeserializationCloner<Reader>::CloneMap(Src src, Dst dst)
    {
        JavascriptMap* map = JavascriptMap::FromVar(dst);

        int32 size;
        m_reader->Read(&size);

        for (int i = 0; i < size; i++)
        {
            Var key;
            Var value;

            GetEngine()->Clone(m_reader->GetPosition(), &key);
            if (!key)
            {
                ThrowSCADataCorrupt();
            }

            GetEngine()->Clone(m_reader->GetPosition(), &value);
            if (!value)
            {
                ThrowSCADataCorrupt();
            }

            map->Set(key, value);
        }
    }

    template <class Reader>
    void DeserializationCloner<Reader>::CloneSet(Src src, Dst dst)
    {
        JavascriptSet* set = JavascriptSet::FromVar(dst);

        int32 size;
        m_reader->Read(&size);

        for (int i = 0; i < size; i++)
        {
            Var value;

            GetEngine()->Clone(m_reader->GetPosition(), &value);
            if (!value)
            {
                ThrowSCADataCorrupt();
            }

            set->Add(value);
        }
    }

    template <class Reader>
    void DeserializationCloner<Reader>::CloneObjectReference(Src src, Dst dst)
    {
        Assert(FALSE); // Should never call this. Object reference handled explictly.
    }
    
    //
    // Try to read a SCAString layout in the form of: [byteLen] [string content] [padding].
    // SCAString is also used for property name in object layout. In case of property terminator,
    // SCA_PROPERTY_TERMINATOR will appear at the place of [byteLen]. Return false in this case.
    //
    template <class Reader>
    bool DeserializationCloner<Reader>::TryReadString(const wchar_t** str, charcount_t* len) const
    {
        uint32 byteLen;
        m_reader->Read(&byteLen);

        if (byteLen == SCA_PROPERTY_TERMINATOR)
        {
            return false;
        }
        else if (byteLen == 0)
        {
            static const wchar_t* emptyString = L"";
            *str = emptyString;
            *len = 0;
        }
        else
        {
            Recycler* recycler = GetScriptContext()->GetRecycler();

            *len = byteLen / sizeof(wchar_t);
            wchar_t* buf = RecyclerNewArrayLeaf(recycler, wchar_t, *len + 1);
            m_reader->Read(buf, byteLen);
            buf[*len] = NULL;

            uint32 unalignedLen = byteLen % sizeof(uint32);
            if (unalignedLen)
            {
                uint32 padding;
                m_reader->Read(&padding, sizeof(uint32) - unalignedLen);
            }

            *str = buf;
        }

        return true;
    }

    //
    // Read a SCAString value from layout: [byteLen] [string content] [padding].
    // Throw if seeing SCA_PROPERTY_TERMINATOR.
    //
    template <class Reader>
    void DeserializationCloner<Reader>::ReadString(const wchar_t** str, charcount_t* len) const
    {
        if (!TryReadString(str, len))
        {
            ThrowSCADataCorrupt();
        }
    }

    //
    // Read bytes data: [bytes] [padding]
    //
    template <class Reader>
    void DeserializationCloner<Reader>::Read(BYTE* buf, uint32 len) const
    {
        m_reader->Read(buf, len);

        uint32 unalignedLen = len % sizeof(uint32);
        if (unalignedLen)
        {
            uint32 padding;
            m_reader->Read(&padding, sizeof(uint32) - unalignedLen);
        }
    }

    //
    // Read a TypedArray or DataView.
    //
    template <class Reader>
    void DeserializationCloner<Reader>::ReadTypedArray(SrcTypeId typeId, Dst* dst) const
    {
        switch (typeId)
        {
        case SCA_Int8Array:
            ReadTypedArray<int8, false>(dst);
            break;

        case SCA_Uint8Array:
            ReadTypedArray<uint8, false>(dst);
            break;

        case SCA_Uint8ClampedArray:
            ReadTypedArray<uint8, true>(dst);
            break;

        case SCA_Int16Array:
            ReadTypedArray<int16, false>(dst);
            break;

        case SCA_Uint16Array:
            ReadTypedArray<uint16, false>(dst);
            break;

        case SCA_Int32Array:
            ReadTypedArray<int32, false>(dst);
            break;

        case SCA_Uint32Array:
            ReadTypedArray<uint32, false>(dst);
            break;

        case SCA_Float32Array:
            ReadTypedArray<float, false>(dst);
            break;

        case SCA_Float64Array:
            ReadTypedArray<double, false>(dst);
            break;

        case SCA_DataView:
            ReadTypedArray<DataView, false>(dst);
            break;

        default:
            Assert(false);
            break;
        }
    }

    template class DeserializationCloner<StreamReader>;

    void ObjectPropertySink::SetProperty(const wchar_t* name, charcount_t len, Var value)
    {
        ScriptContext* scriptContext = GetScriptContext();

        Js::PropertyRecord const * propertyRecord;
        scriptContext->GetOrAddPropertyRecord(name, len, &propertyRecord);
        m_obj->SetProperty(propertyRecord->GetPropertyId(), value, PropertyOperation_None, NULL); //Note: no prototype check
    }

    Var SCADeserializationEngine::Deserialize(ISCAHost* pSCAHost, ISCAContext* pSCAContext, StreamReader* reader, TransferablesHolder* transferableObjects)
    {
        ScriptContext* scriptContext = reader->GetScriptContext();
        StreamDeserializationCloner cloner(scriptContext, pSCAHost, pSCAContext, reader);

        // Read version
        uint32 version;
        reader->Read(&version);
        if (GetSCAMajor(version) > SCA_FORMAT_MAJOR)
        {
            cloner.ThrowSCANewVersion();
        }
        Var value = SCAEngine<scaposition_t, Var, StreamDeserializationCloner>::Clone(reader->GetPosition(), &cloner, transferableObjects, nullptr, 0);
        if (!value)
        {
            cloner.ThrowSCADataCorrupt();
        }

        return value;
    }
}
