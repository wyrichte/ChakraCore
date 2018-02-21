//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "SCAPch.h"
#include "common\ByteSwap.h"
#include "Library\JavascriptNumberObject.h"
#include "Library\JavascriptStringObject.h"
#include "Library\JavascriptBooleanObject.h"
#include "Library\DateImplementation.h"
#include "Library\JavascriptDate.h"
#include "Library\dataview.h"
#include "Library\ES5Array.h"

#include "Types\PropertyIndexRanges.h"
#include "Types\DictionaryPropertyDescriptor.h"
#include "Types\DictionaryTypeHandler.h"
#include "Types\ES5ArrayTypeHandler.h"
#include "Library\JavascriptArrayIndexStaticEnumerator.h"
#include "Library\ES5ArrayIndexStaticEnumerator.h"

namespace Js
{
    template <class Writer>
    bool SerializationCloner<Writer>::TryClonePrimitive(SrcTypeId typeId, Src src, Dst* dst)
    {
        switch (typeId)
        {
        case TypeIds_Undefined:
            WriteTypeId(SCA_UndefinedValue);
            break;

        case TypeIds_Null:
            WriteTypeId(SCA_NullValue);
            break;

        case TypeIds_Boolean:
            WriteTypeId(
                JavascriptBoolean::FromVar(src)->GetValue() ? SCA_TrueValue : SCA_FalseValue);
            break;

        case TypeIds_Integer:
            {
                WriteTypeId(SCA_Int32Value);
                m_writer->Write(TaggedInt::ToInt32(src));
            }
            break;

        case TypeIds_Number:
            {
                WriteTypeId(SCA_DoubleValue);
                m_writer->Write(JavascriptNumber::GetValue(src));
            }
            break;

        case TypeIds_Int64Number:
            {
                WriteTypeId(SCA_Int64Value);
                m_writer->Write(JavascriptInt64Number::FromVar(src)->GetValue());
            }
            break;

        case TypeIds_UInt64Number:
            {
                WriteTypeId(SCA_Uint64Value);
                m_writer->Write(JavascriptUInt64Number::FromVar(src)->GetValue());
            }
            break;

        default:
            return false; // Not a recognized primitive type
        }

        return true;
    }

    template <class Writer>
    bool SerializationCloner<Writer>::TryCloneObject(SrcTypeId typeId, Src src, Dst* dst, SCADeepCloneType* deepClone)
    {
        RecyclableObject* obj = RecyclableObject::FromVar(src);
        scaposition_t beginPos = m_writer->GetPosition();
        *deepClone = SCADeepCloneType::None;

        size_t transferredIndex = 0;
        if (this->CanBeTransferred(typeId) && GetEngine()->TryGetTransferredOrShared(src, &transferredIndex))
        {
            WriteTypeId(SCA_Transferable);
            m_writer->Write((uint32)transferredIndex);
        }
        else if (JavascriptOperators::IsObjectDetached(src))
        {
            //Object is detached, throw error
            ThrowSCAObjectDetached();
        }
        else
        {
            switch (typeId)
            {
            case TypeIds_String: // Clone string value as object type to resolve multiple references
                {
                    JavascriptString* str = JavascriptString::FromVar(obj);
                    WriteTypeId(SCA_StringValue);
                    Write(str->GetString(), str->GetLength());
                }
                break;

            case TypeIds_Object:
                {
                    WriteTypeId(SCA_Object);
                    *deepClone = SCADeepCloneType::Object;
                }
                break;

            case TypeIds_Proxy:
                {
                // Currently SCA algorithm does not support proxy. We'll see 
                // if the spec will be updated. I don't support QueryObjectInterface in proxy
                // so we don't want to go through the default code path.
                    return false;
                }
            case TypeIds_Array:
            case TypeIds_ES5Array:
            case TypeIds_NativeIntArray:
            case TypeIds_NativeFloatArray:
                {
                    // Postpone writing to CloneProperties
                    *deepClone = SCADeepCloneType::Object;
                }
                break;

            case TypeIds_Date:
                {
                    WriteTypeId(SCA_DateObject);
                    m_writer->Write(JavascriptDate::FromVar(src)->GetTime());
                }
                break;

            case TypeIds_RegEx:
                {
                    JavascriptRegExp* regex = JavascriptRegExp::FromVar(src);
                    InternalString str = regex->GetSource();
                    DWORD flags = static_cast<DWORD>(regex->GetFlags());
                    WriteTypeId(SCA_RegExpObject);
                    Write(str.GetBuffer(), str.GetLength());
                    m_writer->Write(flags);
                }
                break;

            case TypeIds_BooleanObject:
                WriteTypeId(JavascriptBooleanObject::FromVar(src)->GetValue() ?
                    SCA_BooleanTrueObject : SCA_BooleanFalseObject);
                break;

            case TypeIds_NumberObject:
                {
                    WriteTypeId(SCA_NumberObject);
                    m_writer->Write(JavascriptNumberObject::FromVar(src)->GetValue());
                }
                break;

            case TypeIds_StringObject:
                {
                    JavascriptString* str = JavascriptStringObject::FromVar(src)->Unwrap();
                    WriteTypeId(SCA_StringObject);
                    Write(str->GetString(), str->GetLength());
                }
                break;

            case TypeIds_ArrayBuffer:
                {
                    ArrayBuffer* buf = ArrayBuffer::FromVar(src);
                    WriteTypeId(SCA_ArrayBuffer);
                    Write(buf->GetBuffer(), buf->GetByteLength());
                }
                break;

            case TypeIds_SharedArrayBuffer:
                {
                    SCAContextType contextType;
                    if (FAILED(this->m_pSCAContext->GetContext(&contextType)))
                    {
                        return false;
                    }
                    
                    if(contextType == SCAContext_CrossProcess || contextType == SCAContext_Persist)
                    {
                        return false;
                    }

            
                    SharedArrayBuffer* buf = SharedArrayBuffer::FromVar(src);
                    SharedContents* sharedContents = buf->GetSharedContents();
                    sharedContents->AddRef();
                    this->m_sharedContentsrList->Add(sharedContents);
                    WriteTypeId(SCA_SharedArrayBuffer);
                    m_writer->Write((intptr_t)sharedContents);
                }
                break;

            case TypeIds_Map:
                {
                    WriteTypeId(SCA_Map);
                    *deepClone = SCADeepCloneType::Map;
                }
                break;

            case TypeIds_Set:
                {
                    WriteTypeId(SCA_Set);
                    *deepClone = SCADeepCloneType::Set;
                }
                break;

#ifdef ENABLE_WASM
            case TypeIds_WebAssemblyModule:
                {
                    WebAssemblyModule* wasmModule = WebAssemblyModule::FromVar(src);
                    WriteTypeId(SCA_WebAssemblyModule);
                    Write(wasmModule->GetBinaryBuffer(), wasmModule->GetBinaryBufferLength());
                }
                break;
#endif

            case TypeIds_CopyOnAccessNativeIntArray:
                Assert(false);
                // fall-through

            default:
                if (IsTypedArray(typeId))
                {
                    WriteTypedArray(typeId, src);
                    break;
                }
                Js::ScriptContext* scriptContext = obj->GetScriptContext();
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    if (SUCCEEDED(obj->QueryObjectInterface(__uuidof(ISCASerializable), (void**)&m_pSCASerializable)))
                    {
                        *deepClone = SCADeepCloneType::Object;
                        break;
                    }
                }
                END_LEAVE_SCRIPT(scriptContext);
                return false; // Not a supported object type
            }
        }

        *dst = beginPos;
        return true;
    }

    template <class Writer>
    void SerializationCloner<Writer>::ClonePropertiesWithSCASerializable(SrcTypeId srcTypeId, Src src, Dst dst)
    {
        AutoCOMPtr<ISCASerializable> pSCASerializable = m_pSCASerializable;
        Assert(pSCASerializable != NULL);
        m_pSCASerializable = NULL; // Clear temp

        SCATypeId typeId;
        AutoCOMPtr<SCAPropBag> pPropBag;
        SCAPropBag::CreateInstance(GetScriptContext(), &pPropBag);

        ScriptContext* scriptContext = GetScriptContext();
        HRESULT hr = S_OK;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = pSCASerializable->GetObjectData(m_pSCAContext, &typeId, pPropBag);
        }
        END_LEAVE_SCRIPT(scriptContext);
        ThrowIfFailed(hr);

        SCAPropBag::PropBagEnumerator propBagEnumerator(pPropBag);
        WriteTypeId(typeId);
        WriteObjectProperties(&propBagEnumerator);
    }

    template <class Writer>
    void SerializationCloner<Writer>::ClonePropertiesWithoutSCASerializable(SrcTypeId srcTypeId, Src src, Dst dst)
    {
        RecyclableObject* obj = RecyclableObject::FromVar(src);
        // allocate the JavascriptStaticEnumerator on the heap to avoid blowing the stack
        JavascriptStaticEnumerator enumerator;
        ScriptContext* scriptContext = GetScriptContext();
        if (DynamicObject::IsAnyArrayTypeId(srcTypeId))
        {
            JavascriptArray* arr = JavascriptArray::FromAnyArray(src);
            bool isSparseArray = IsSparseArray(arr);

            WriteTypeId(isSparseArray ? SCA_SparseArray : SCA_DenseArray);
            Write(arr->GetLength());

            if (isSparseArray)
            {
                WriteSparseArrayIndexProperties(arr);
            }
            else
            {
                WriteDenseArrayIndexProperties(arr);
            }

            // Now we only need to write remaining non-index properties
            arr->GetNonIndexEnumerator(&enumerator, scriptContext);
        }
        else if (!obj->GetEnumerator(&enumerator, EnumeratorFlags::SnapShotSemantics, scriptContext))
        {
            // Mark property end if we don't have enumerator
            m_writer->Write(static_cast<uint32>(SCA_PROPERTY_TERMINATOR));
            return;
        }

        ObjectPropertyEnumerator propEnumerator(scriptContext, obj, &enumerator);
        WriteObjectProperties(&propEnumerator);
    }

    template <class Writer>
    void SerializationCloner<Writer>::CloneProperties(SrcTypeId srcTypeId, Src src, Dst dst)
    {
        if (m_pSCASerializable)
        {
            ClonePropertiesWithSCASerializable(srcTypeId, src, dst);
        }
        else
        {
            ClonePropertiesWithoutSCASerializable(srcTypeId, src, dst);
        }
    }

    template <class Writer>
    void SerializationCloner<Writer>::CloneMap(Src src, Dst dst)
    {
        JavascriptMap* map = JavascriptMap::FromVar(src);

        Write((int32)(map->Size()));

        JavascriptMap::MapDataList::Iterator iter = map->GetIterator();
        while (iter.Next())
        {
            const JavascriptMap::MapDataKeyValuePair& entry = iter.Current();
            GetEngine()->Clone(entry.Key());
            GetEngine()->Clone(entry.Value());
        }
    }

    template <class Writer>
    void SerializationCloner<Writer>::CloneSet(Src src, Dst dst)
    {
        JavascriptSet* set = JavascriptSet::FromVar(src);

        Write((int32)(set->Size()));

        JavascriptSet::SetDataList::Iterator iter = set->GetIterator();
        while (iter.Next())
        {
            GetEngine()->Clone(iter.Current());
        }
    }

    template <class Writer>
    void SerializationCloner<Writer>::CloneObjectReference(Src src, Dst dst)
    {
        WriteTypeId(SCA_Reference);
        m_writer->Write(dst);
    }

    //
    // Write layout: [byteLen] [string content] [padding]
    //
    template <class Writer>
    void SerializationCloner<Writer>::Write(const char16* str, charcount_t len) const
    {
        uint32 byteLen = static_cast<uint32>(sizeof(char16) * len);
        m_writer->Write(byteLen);
        m_writer->Write(str, byteLen);
        uint32 unalignedLen = byteLen % sizeof(uint32);
        if (unalignedLen)
        {
            uint32 padding = 0;
            m_writer->Write(&padding, sizeof(uint32) - unalignedLen);
        }
    }

    //
    // Write layout: [byteLen] [byte data] [padding]
    //
    template <class Writer>
    void SerializationCloner<Writer>::Write(const BYTE* bytes, uint32 len) const
    {
        m_writer->Write(len);
        m_writer->Write(bytes, len);
        uint32 unalignedLen = len % sizeof(uint32);
        if (unalignedLen)
        {
            uint32 padding = 0;
            m_writer->Write(&padding, sizeof(uint32) - unalignedLen);
        }
    }

    //
    // Check if a SrcTypeId is of a TypedArray or DataView.
    //
    template <class Writer>
    bool SerializationCloner<Writer>::IsTypedArray(SrcTypeId typeId)
    {
        return (typeId >= TypeIds_TypedArraySCAMin && typeId <= TypeIds_TypedArraySCAMax)
            || typeId == TypeIds_DataView;
    }

    //
    // Write a TypedArray or a DataView layout.
    //
    template <class Writer>
    void SerializationCloner<Writer>::WriteTypedArray(SrcTypeId typeId, Src src) const
    {
        switch (typeId)
        {
        case TypeIds_Int8Array:
            if (Int8VirtualArray::HasVirtualTableInfo(src))
            {
                WriteTypedArray<int8, false, true>(src);
            }
            else
            {
                WriteTypedArray<int8, false>(src);
            }
            break;

        case TypeIds_Uint8Array:
            if (Uint8VirtualArray::HasVirtualTableInfo(src))
            {
                WriteTypedArray<uint8, false, true>(src);
            }
            else
            {
                WriteTypedArray<uint8, false>(src);
            }
            break;

        case TypeIds_Uint8ClampedArray:
            if (Uint8ClampedVirtualArray::HasVirtualTableInfo(src))
            {
                WriteTypedArray<uint8, true, true>(src);
            }
            else
            {
                WriteTypedArray<uint8, true>(src);
            }
            break;

        case TypeIds_Int16Array:
            if (Int16VirtualArray::HasVirtualTableInfo(src))
            {
                WriteTypedArray<int16, false, true>(src);
            }
            else
            {
                WriteTypedArray<int16, false>(src);
            }
            break;

        case TypeIds_Uint16Array:
            if (Uint16VirtualArray::HasVirtualTableInfo(src))
            {
                WriteTypedArray<uint16, false, true>(src);
            }
            else
            {
                WriteTypedArray<uint16, false>(src);
            }
            break;

        case TypeIds_Int32Array:
            if (Int32VirtualArray::HasVirtualTableInfo(src))
            {
                WriteTypedArray<int32, false, true>(src);
            }
            else
            {
                WriteTypedArray<int32, false>(src);
            }
            break;

        case TypeIds_Uint32Array:
            if (Uint32VirtualArray::HasVirtualTableInfo(src))
            {
                WriteTypedArray<uint32, false, true>(src);
            }
            else
            {
                WriteTypedArray<uint32, false>(src);
            }
            break;

        case TypeIds_Float32Array:
            if (Float32VirtualArray::HasVirtualTableInfo(src))
            {
                WriteTypedArray<float, false, true>(src);
            }
            else
            {
                WriteTypedArray<float, false>(src);
            }
            break;

        case TypeIds_Float64Array:
            if (Float64VirtualArray::HasVirtualTableInfo(src))
            {
                WriteTypedArray<double, false, true>(src);
            }
            else
            {
                WriteTypedArray<double, false>(src);
            }
            break;

        case TypeIds_DataView:
            WriteTypedArray<DataView, false>(src);
            break;

        default:
            Assert(false);
        }
    }

    //
    // Do an arbitrary test to determine serializing a JavascriptArray as sparse array or not.
    //
    template <class Writer>
    bool SerializationCloner<Writer>::IsSparseArray(JavascriptArray* arr)
    {
        uint32 length = arr->GetLength();
        if (length > SparseArraySegmentBase::HEAD_CHUNK_SIZE)
        {
            // Consider it a sparse array if the array size is non-trivial, and there
            // are empty slots found by some arbitrary sampling.
            return !JavascriptOperators::HasOwnItem(arr, length / 4)
                || !JavascriptOperators::HasOwnItem(arr, length / 2)
                || !JavascriptOperators::HasOwnItem(arr, length / 2 + length / 4);
        }

        return false;
    }

    //
    // Write dense array index named properties.
    //
    template <class Writer>
    void SerializationCloner<Writer>::WriteDenseArrayIndexProperties(JavascriptArray* arr)
    {
        if (JavascriptArray::Is(arr))
        {
            if (!arr->IsCrossSiteObject())
            {
                WriteArrayIndexProperties<JavascriptArrayDirectItemAccessor>(arr);
            }
            else
            {
                WriteArrayIndexProperties<JavascriptArrayItemAccessor>(arr);
            }
        }
        else
        {
            WriteArrayIndexProperties<JavascriptArrayEnumerableItemAccessor>(arr);
        }
    }

    //
    // Write sparse array index named properties.
    //
    template <class Writer>
    void SerializationCloner<Writer>::WriteSparseArrayIndexProperties(JavascriptArray* arr)
    {
        if (JavascriptArray::Is(arr))
        {
            if (!arr->IsCrossSiteObject())
            {
                WriteSparseArrayIndexProperties<
                    JavascriptArrayIndexStaticEnumerator, JavascriptArrayDirectItemAccessor>(arr);
            }
            else
            {
                WriteSparseArrayIndexProperties<
                    JavascriptArrayIndexStaticEnumerator, JavascriptArrayItemAccessor>(arr);
            }
        }
        else
        {
            // We don't need JavascriptArrayEnumerableItemAccessor to check enumberable since we'll
            // enumerate enumerable index named properties through ES5ArrayIndexEnumerator. Just use
            // JavascriptArrayItemAccessor.
            WriteSparseArrayIndexProperties<
                ES5ArrayIndexStaticEnumerator<>, JavascriptArrayItemAccessor>(ES5Array::FromVar(arr));
        }
    }

    template class SerializationCloner<StreamWriter>;

    bool ObjectPropertyEnumerator::MoveNext()
    {
        if (m_innerEnumerator)
        {
            ScriptContext* scriptContext = GetScriptContext();
            Var undefined = scriptContext->GetLibrary()->GetUndefined();
            Var propertyName;
            PropertyId propertyId;

            while ((propertyName = m_innerEnumerator->MoveAndGetNext(propertyId)) != NULL)
            {
                if (propertyName != undefined) //There are some code paths in which GetCurrentIndex can return undefined
                {
                    m_name = JavascriptString::FromVar(propertyName);

                    if (propertyId != Constants::NoProperty)
                    {
                        m_value = JavascriptOperators::GetProperty(m_obj, propertyId, scriptContext);
                    }
                    else
                    {
                        m_value = JavascriptOperators::OP_GetElementI(m_obj, propertyName, scriptContext);
                    }

                    return true;
                }
            }

            // No more properties, mark end
            m_innerEnumerator = NULL;
        }

        return false;
    }

    void SCASerializationEngine::Serialize(ISCAContext* pSCAContext, Var root, StreamWriter* writer, Var* transferableVars, size_t cTransferableVars,
        JsUtil::List<Js::SharedContents*, HeapAllocator>* sharedContentsList)
    {
        ScriptContext* scriptContext = writer->GetScriptContext();

        // Write version
        writer->Write(static_cast<uint32>(SCA_FORMAT_VERSION));

        StreamSerializationCloner cloner(scriptContext, pSCAContext, writer, sharedContentsList);
        SCAEngine<Var, scaposition_t, StreamSerializationCloner>::Clone(root, &cloner, nullptr, transferableVars, cTransferableVars);
    }
}
