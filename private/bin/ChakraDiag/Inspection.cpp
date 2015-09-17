//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

// REVIEW: I get unresolved externals without these declarations (which duplicate JavascriptArray.cpp).
// Are they necessary, and is there another place where such things are collected in JscriptDiag?
#if _M_X64
const Js::Var Js::JavascriptArray::MissingItem = (Js::Var)0x8000000280000002;
#else
const Js::Var Js::JavascriptArray::MissingItem = (Js::Var)0x80000002;
#endif
const int32 Js::JavascriptNativeIntArray::MissingItem = 0x80000002;
static const uint64 FloatMissingItemPattern = 0x8000000280000002ull;
const double Js::JavascriptNativeFloatArray::MissingItem = *(double*)&FloatMissingItemPattern;


namespace JsDiag
{
    static const CString s_lengthPropertyName(L"length");
    const CString MethodGroupProperty::s_name(L"[Methods]");

    int __cdecl ComparePropertyInfo(_In_ void* context, _In_ const void* item1, _In_ const void* item2)
    {
        const PROPERTY_INFO* p1 = reinterpret_cast<const PROPERTY_INFO*>(item1);
        const PROPERTY_INFO* p2 = reinterpret_cast<const PROPERTY_INFO*>(item2);

        // Do the natural comparison, for example test2 comes before test11.
        return StrCmpLogicalW(p1->name, p2->name);
    }

    template<class T>
    void ObjectWalker<T>::Init(InspectionContext* context, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty)
    {
        __super::Init(context, var, ownerDebugProperty);

        RemoteDynamicObject obj(context->GetReader(), var);
        if (obj.HasObjectArray(context))
        {
            Js::Var arr = obj->GetObjectArrayOrFlagsAsArray();
            CComPtr<IJsDebugPropertyInternal> prop;
            context->CreateDebugProperty(PROPERTY_INFO(arr), /*parent*/nullptr, &prop);
            if (!prop->TryGetWalker(&m_internalArrayWalker))
            {
                DiagException::Throw(E_UNEXPECTED, DiagErrorCode::GET_INTERNALARRAY_WALKER);
            }
            m_internalArrayWalker->SetIsInternalArray(GetOwnerDebugProperty());
        }
    }

    template<class T>
    uint ObjectWalker<T>::GetInternalArrayCount() const
    {
        return m_internalArrayWalker ? m_internalArrayWalker->GetCount() : 0;
    }

    template<class T>
    bool_result ObjectWalker<T>::TryGetInternalArrayItem(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
    {
        return m_internalArrayWalker ? m_internalArrayWalker->TryGetProperty(index, ppDebugProperty) : false;
    }

    // Supports WalkerPolicy
    template<class T>
    bool_result ObjectWalker<T>::TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        return m_internalArrayWalker ? m_internalArrayWalker->TryGetItem(index, ppDebugProperty) : false;
    }

    CString JavascriptObjectProperty::GetTypeString() const
    {
        PROPERTY_INFO constructor;
        if (m_context->GetProperty(m_info.data, PropertyIds::constructor, &constructor)
            && constructor.HasData()
            && m_context->GetTypeId(constructor.data) == Js::TypeIds_Function)
        {
            CString name;
            if (RemoteJavascriptFunction::TryReadDisplayName(m_context->GetReader(), constructor.data, &name))
            {
                return L"Object, (" + name + L")";
            }
        }

        return __super::GetTypeString();
    }

    OriginalObjectInfo::OriginalObjectInfo(InspectionContext* context, const RecyclableObject* instance, Js::TypeId typeId):
        instance(instance),
        typeId(static_cast<::JavascriptTypeId>(typeId))
    {
        extensionObject = context->IsMshtmlObject(typeId) ?
            RemoteCustomExternalObject(context->GetReader(), static_cast<const CustomExternalObject*>(instance)).ReadExtensionObject() : nullptr;
    }

    void JavascriptProxyWalker::InsertProperties(const DynamicObject* var)
    {
        RemoteJavascriptProxyObject remoteObject(m_context->GetReader(), static_cast<const JavascriptProxy*>(var));
        PROPERTY_INFO info;
        info.type = PROPERTY_INFO::DATA_PROPERTY;
        info.attr = JS_PROPERTY_READONLY | JS_PROPERTY_FAKE;
        info.name = L"[handler]";
        info.pointer = remoteObject->GetHandler();
        pThis()->InsertItem(info);

        info.name = L"[target]";
        info.pointer = remoteObject->GetTarget();
        pThis()->InsertItem(info);
    }

    void ExternalObjectWalker::InsertProperties(const DynamicObject* var)
    {
        RemoteForInObjectEnumerator forIn(m_context, var);
        forIn.Enumerate([this](const PROPERTY_INFO& info)
        {
            pThis()->InsertItem(info);
            return true;
        });
        RemoteExternalObject externalObject(m_context->GetReader(), static_cast<const ExternalObject*>(var));
        if(externalObject.IsProjectionObjectInstance(m_context->GetDebugClient()))
        {
            PROPERTY_INFO  info;
            info.type = PROPERTY_INFO::POINTER_VALUE;
            info.name = L"[Native Object]";
            info.attr = JS_PROPERTY_NATIVE_WINRT_POINTER;
            RemoteProjectionObjectInstance instance(m_context->GetReader(),  static_cast<const Projection::ProjectionObjectInstance*>(var));
            info.pointer = instance->unknown;
            pThis()->InsertItem(info);
        }
    }

    void ExternalObjectProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, Js::TypeId typeId, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        __super::Init(context, info, parent);

        m_typeId = typeId;
    }

    CString ExternalObjectProperty::GetTypeString() const
    {
        CString name = m_context->ReadPropertyName(
            RemoteExternalObject(m_context->GetReader(), static_cast<ExternalObject*>(m_info.data)).GetClassName());
        return L"[Object, " + name + L"]";
    }

    void ExternalObjectProperty::EnumNonIndexProperties(IPropertyListener* listener, const OriginalObjectInfo& originalObject, bool requireEnumerable) const
    {
        if (m_context->IsMshtmlObject(m_typeId))
        {
            MshtmlObjectEnumerator::Enumerate(m_context, GetInstance(), m_typeId, originalObject, [=](const PROPERTY_INFO& prop) -> bool
            {
                return listener->EnumProperty(Js::Constants::NoProperty, prop);
            });
        }

        __super::EnumNonIndexProperties(listener, originalObject, requireEnumerable);
    }

    bool_result SimpleProperty::TryToIndex(_Out_ UINT* index, _Out_ CString* name)
    {
        // This works for null/undefined/true/false
        *index = DiagConstants::InvalidIndexPropertyName;
        *name = GetValue(10);
        return true;
    }

    bool_result UnknownProperty::TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        m_context->ThrowEvaluateNotSupported();
    }

    void JavascriptBooleanProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, bool value, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        __super::Init(context, info, L"Boolean", GetString(value), parent);
    }

    void JavascriptBooleanProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        bool value = RemoteJavascriptBoolean::GetValue(context->GetReader(), reinterpret_cast<JavascriptBoolean*>(info.data));
        Init(context, info, value, parent);
    }

    LPCWSTR JavascriptBooleanProperty::GetString(bool b)
    {
        return b ? L"true" : L"false";
    }

    bool_result JavascriptBooleanProperty::TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        // ToObject is only used for strict mode "this" inspection. It has no parent.
        CreateComObject<FakeToObjectProperty<JavascriptBooleanProperty, JavascriptBooleanObjectProperty>>(
            m_context, this, /*parent*/nullptr, ppDebugProperty);
        return true;
    }

    void JavascriptSymbolProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, CString value, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        __super::Init(context, info, parent);
        m_value = value;
    }

    void JavascriptSymbolProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        CString value = RemoteJavascriptSymbol::GetValue(context->GetReader(), reinterpret_cast<JavascriptSymbol*>(info.data));
        Init(context, info, value, parent);
    }

    bool_result JavascriptSymbolProperty::TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        // ToObject is only used for strict mode "this" inspection. It has no parent.
        CreateComObject<FakeToObjectProperty<JavascriptSymbolProperty, JavascriptSymbolObjectProperty>>(
            m_context, this, /*parent*/nullptr, ppDebugProperty);
        return true;
    }

    void NumberProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, double value, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        __super::Init(context, info, parent);
        m_value = value;
    }

    void NumberProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        GetValueBSTR(m_value, nRadix, pValue);
    }

    void NumberProperty::GetValueBSTR(double value, UINT radix, _Out_ BSTR* pValue)
    {
        // For fractional values, radix is ignored.
        long l = (long)value;
        bool isZero = JavascriptNumber::IsZero(value - (double)l);

        if (radix == 10 || !isZero)
        {
            *pValue = ToStringRadix10(value);
        }
        else if (radix >= 2 && radix <= 36)
        {
            if (radix == 16 && value < 0)
            {
                // On the tools side we show unsigned value.
                ulong ul = (ulong)(long)value; // ARM: casting negative value to ulong gives 0
                value = (double)ul;
            }

            *pValue = ToStringRadixHelper(value, radix);
        }
    }

    BSTR NumberProperty::ToStringRadix10(double value)
    {
        BSTR bstr = ToStringNanOrInfiniteOrZero(value, 10);
        if (bstr == nullptr)
        {
            static const int bufSize = 256;
            wchar_t szBuffer[bufSize]; //TODO: This seems overly generous

            if(Js::NumberUtilities::FNonZeroFiniteDblToStr(value, szBuffer, bufSize))
            {
                bstr = ::SysAllocString(szBuffer);
            }
        }

        if (bstr == nullptr)
        {
            DiagException::ThrowOOM();
        }
        return bstr;
    }

    BSTR NumberProperty::ToStringRadixHelper(double value, int radix)
    {
        Assert(radix != 10);
        Assert(radix >= 2 && radix <= 36);

        BSTR bstr = ToStringNanOrInfiniteOrZero(value, radix);
        if (bstr == nullptr)
        {
            static const int bufSize = 256;
            wchar_t szBuffer[bufSize];

            WCHAR* psz = szBuffer;
            int len = bufSize;
            if (radix == 16)
            {
                static const WCHAR PREFIX16[] = L"0x";
                static const int PREFIX_LEN = _countof(PREFIX16) - 1;
                wcscpy_s(szBuffer, PREFIX16);
                psz += PREFIX_LEN;
                len -= PREFIX_LEN;
            }

            if (Js::NumberUtilities::FNonZeroFiniteDblToStr(value, radix, psz, len))
            {
                bstr = ::SysAllocString(szBuffer);
            }
        }

        if (bstr == nullptr)
        {
            DiagException::ThrowOOM();
        }
        return bstr;
    }

    BSTR NumberProperty::ToStringNanOrInfiniteOrZero(double value, int radix)
    {
        DiagBSTRLib lib;

        BSTR nanF;
        if (nullptr != (nanF = JavascriptNumber::ToStringNanOrInfinite(value, lib)))
        {
            return nanF;
        }

        if (JavascriptNumber::IsZero(value))
        {
            if (radix == 16)
            {
                return lib.CreateStringFromCppLiteral(L"0x0");
            }
            else
            {
                if (radix == 10 && Js::JavascriptNumber::IsNegZero(value))
                {
                    // In debugger, we wanted to show negative zero explicitly (only for radix 10)
                    return lib.CreateStringFromCppLiteral(L"-0");
                }
                else
                {
                    return lib.CreateStringFromCppLiteral(L"0");
                }
            }
        }

        return nullptr;
    }

    bool_result NumberProperty::TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        // ToObject is only used for strict mode "this" inspection. It has no parent.
        CreateComObject<FakeToObjectProperty<NumberProperty, JavascriptNumberObjectProperty>>(
            m_context, this, /*parent*/nullptr, ppDebugProperty);
        return true;
    }

    void TaggedIntProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, int value, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        __super::Init(context, info, value, parent);
    }

    void TaggedIntProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        __super::Init(context, info, TaggedInt::ToDouble(info.data), parent);
    }

    bool_result TaggedIntProperty::TryToIndex(_Out_ UINT* index, _Out_ CString* name)
    {
        long value = static_cast<long>(GetValue());
        if (value >= 0)
        {
            *index = static_cast<UINT>(value);
            return true;
        }

        // We could try to support negative int as index property name, but don't bother with corner case.
        return false;
    }

    void JavascriptNumberProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        double value = RemoteJavascriptNumber::GetValue(context->GetReader(), info.data);
        __super::Init(context, info, value, parent);
    }

    template <class T>
    void JavascriptTypedNumberProperty<T>::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        RemoteData<T> number(m_context->GetReader(), reinterpret_cast<T*>(m_info.data));
        double value = static_cast<double>(number->GetValue());
        NumberProperty::GetValueBSTR(value, nRadix, pValue);
    }

    void JavascriptPointerProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        WCHAR valueString[70]; // Max is size 64 for base 2, and 1 for NULL, and 2 more prefix
        int prefixLen = 0;
        if(nRadix == 16)
        {
            valueString[0] = '0';
            valueString[1] = 'x';
            prefixLen = 2;
        }
        errno_t result = _ui64tow_s((UINT64)m_info.pointer, (valueString + prefixLen), (_countof(valueString) - prefixLen), nRadix);
        if(result != 0) // failed
        {
            Assert(false);
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::UI64_TO_STRING);
        }
        *pValue = SysAllocString(valueString);
        if(*pValue == nullptr)
        {
            DiagException::ThrowOOM();
        }
    }

    void JavascriptStringProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        __super::Init(context, info, parent);

        if (info.type == PROPERTY_INFO::STRING_VALUE)
        {
            m_str = L'\"' + info.strValue + L'\"';
        }
        else
        {
            Assert(info.HasData());
            JavascriptString* var = static_cast<JavascriptString*>(info.data);
            charcount_t len = RemoteJavascriptString(context, var).GetLength();

            if (len >= JavascriptString::MaxCharLength - 2)
            {
                DiagException::ThrowOOM(DiagErrorCode::REMOTE_STRING_TOO_LONG);
            }
            len += 2;

            LPWSTR buf = m_str.GetBufferSetLength(len);
            buf[0] = L'\"';
            charcount_t actual;
            context->ReadString(var, &buf[1], len - 2, &actual);
            buf[actual + 1] = L'\"';
            m_str.ReleaseBuffer(actual + 2);
        }
    }

    bool_result JavascriptStringProperty::TryToIndex(_Out_ UINT* index, _Out_ CString* name)
    {
        PCWSTR s = m_str.GetString();
        const UINT len = GetStringLength();

        uint32 n;
        if (NumberUtilities::TryConvertToUInt32(s + 1, len, &n)
            && n != DiagConstants::InvalidIndexPropertyName)
        {
            *index = static_cast<UINT>(n);
        }
        else
        {
            *index = DiagConstants::InvalidIndexPropertyName;
            *name = m_str.Mid(1, len);
        }

        return true;
    }

    bool_result JavascriptStringProperty::TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        // ToObject is only used for strict mode "this" inspection. It has no parent.
        CreateComObject<FakeToObjectProperty<JavascriptStringProperty, JavascriptStringObjectProperty>>(
            m_context, this, /*parent*/nullptr, ppDebugProperty);
        return true;
    }

    bool_result JavascriptStringProperty::TryGetBuiltInProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        if (name == s_lengthPropertyName)
        {
            const UINT len = GetStringLength();
            m_context->CreateDebugProperty(PROPERTY_INFO(name, len), nullptr, ppDebugProperty);
            return true;
        }
        return false;
    }

    bool_result JavascriptStringProperty::TryGetItemDirect(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        const UINT len = GetStringLength();
        if (index < len)
        {
            WCHAR ch = m_str[1 + index]; // skip start quote
            m_context->CreateDebugProperty(PROPERTY_INFO(index, CString(ch)), nullptr, ppDebugProperty);
            return true;
        }
        return false;
    }

    LPCWSTR JavascriptBooleanObjectProperty::GetValue(UINT nRadix)
    {
        bool value = RemoteJavascriptBooleanObject::GetValue(m_context->GetReader(), m_info.data);
        return JavascriptBooleanProperty::GetString(value);
    }

    LPCWSTR JavascriptSymbolObjectProperty::GetValue(UINT nRadix)
    {
        return RemoteJavascriptSymbolObject::GetValue(m_context->GetReader(), m_info.data);
    }

    void JavascriptNumberObjectProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        double value = RemoteJavascriptNumberObject::GetValue(m_context->GetReader(), m_info.data);
        NumberProperty::GetValueBSTR(value, nRadix, pValue);
    }

    void JavascriptStringObjectProperty::Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        __super::Init(context, info, parent);

        Assert(info.HasData());
        JavascriptString* str = RemoteJavascriptStringObject(
            context->GetReader(), static_cast<JavascriptStringObject*>(info.data)).GetValue();
        if (str)
        {
            CComPtr<JavascriptStringProperty> prop;
            CreateComObject(&prop);
            prop->Init(context, PROPERTY_INFO(CString(), str), parent);

            m_value = prop;
        }
    }

    void JavascriptStringObjectProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        if (m_value)
        {
            m_value->GetValueBSTR(nRadix, pValue);
        }
        else
        {
            ToBSTR(L"\"\"", pValue);
        }
    }

    bool_result JavascriptStringObjectProperty::TryGetBuiltInProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        return m_value ? m_value->TryGetBuiltInProperty(name, ppDebugProperty) : false;
    }

    bool_result JavascriptStringObjectProperty::TryGetItemDirect(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        return m_value ? m_value->TryGetItemDirect(index, ppDebugProperty) : false;
    }

    void JavascriptStringObjectProperty::EnumItems(IPropertyListener* listener, uint start, uint end, bool requireEnumerable) const
    {
        if (m_value)
        {
            const CString& str = m_value->GetValue();

            // str has extra leading and ending " char
            Assert(str.GetLength() >= 2);
            end = min(end, static_cast<uint>(str.GetLength()) - 2);
            ++start;
            ++end;

            for (uint i = start; i < end; i++)
            {
                wchar_t ch = str[i];
                if (!listener->EnumItem(i, PROPERTY_INFO(i, CString(ch))))
                {
                    break;
                }
            }
        }
    }

    template <class T>
    void BaseArrayWalker<T>::InsertSpecialProperties(const DynamicObject* var)
    {
        AssertMsg(var, "The array var must be specified to insert special properties.");
        auto reader = m_context->GetReader();
        RemoteJavascriptArray arr(reader, static_cast<const JavascriptArray*>(var));
        RemoteScriptContext scriptContext(reader, var);
        RemoteThreadContext threadContext(reader, scriptContext.GetThreadContext());

        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::length, arr->length);

        __super::InsertSpecialProperties(var);
    }

    template<class Data, bool clamped, class RemoteArray, class Trace>
    void TypedArrayWalker<Data, clamped, RemoteArray, Trace>::InsertSpecialProperties(const DynamicObject* var)
    {
        AssertMsg(var, "The array var must be specified to insert special properties.");
        auto reader = m_context->GetReader();
        RemoteScriptContext scriptContext(reader, var);
        RemoteScriptConfiguration scriptConfiguration(reader, scriptContext.GetConfig());
        if (!scriptConfiguration.IsES6TypedArrayExtensionsEnabled())
        {
            RemoteArray arr = RemoteArray(reader, static_cast<const TargetType*>(var));
            RemoteThreadContext threadContext(reader, scriptContext.GetThreadContext());

            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::length, arr->GetLength());
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::buffer, arr->GetArrayBuffer());
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::byteOffset, arr->GetByteOffset());
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::byteLength, arr->GetByteLength());
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::BYTES_PER_ELEMENT, arr->GetBytesPerElement());
        }

        // Explicitly leave out the super call as TypedArrayWalker inserts length
        // in a different way than its BaseArrayWalker parent.
        //__super::InsertSpecialProperties(var);
    }

    void ArrayBufferWalker::InsertSpecialProperties(const DynamicObject* var)
    {
        AssertMsg(var, "The array buffer var must be specified to insert special properties.");
        auto reader = m_context->GetReader();
        RemoteScriptContext scriptContext(reader, var);
        RemoteScriptConfiguration scriptConfiguration(reader, scriptContext.GetConfig());
        if (!scriptConfiguration.IsES6TypedArrayExtensionsEnabled())
        {
            RemoteArrayBuffer arrayBuffer = RemoteArrayBuffer(reader, static_cast<const ArrayBuffer*>(var));
            RemoteThreadContext threadContext(reader, scriptContext.GetThreadContext());

            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::byteLength, arrayBuffer->GetByteLength());
        }
        __super::InsertSpecialProperties(var);
    }

    void JavascriptRegExpConstructorWalker::InsertSpecialProperties(const DynamicObject* var)
    {
        AssertMsg(var, "The regular expression contructor var must be specified to insert special properties.");
        auto reader = m_context->GetReader();
        RemoteJavascriptRegExpConstructor arrayBuffer = RemoteJavascriptRegExpConstructor(reader, static_cast<const JavascriptRegExpConstructor*>(var));
        RemoteScriptContext scriptContext(reader, var);
        RemoteThreadContext threadContext(reader, scriptContext.GetThreadContext());

        Js::Var lastInput = arrayBuffer->lastInput;
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::input, lastInput);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$_, lastInput);

        Js::Var lastMatch = arrayBuffer->captures[0];
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::lastMatch, lastMatch);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$Ampersand, lastMatch);

        Js::Var lastParen = arrayBuffer->lastParen;
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::lastParen, lastParen);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$Plus, lastParen);

        Js::Var leftContext = arrayBuffer->leftContext;
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::leftContext, leftContext);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$BackTick, leftContext);

        Js::Var rightContext = arrayBuffer->rightContext;
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::rightContext, rightContext);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$Tick, rightContext);

        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$1, arrayBuffer->captures[1]);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$2, arrayBuffer->captures[2]);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$3, arrayBuffer->captures[3]);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$4, arrayBuffer->captures[4]);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$5, arrayBuffer->captures[5]);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$6, arrayBuffer->captures[6]);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$7, arrayBuffer->captures[7]);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$8, arrayBuffer->captures[8]);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::$9, arrayBuffer->captures[9]);

        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::index, arrayBuffer->index);
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::lastIndex, arrayBuffer->lastIndex);

        __super::InsertSpecialProperties(var);
    }

    void JavascriptRegExpWalker::InsertSpecialProperties(const DynamicObject* var)
    {
        AssertMsg(var, "The regular expression var must be specified to insert special properties.");
        auto reader = m_context->GetReader();
        RemoteJavascriptRegExp regularExpression = RemoteJavascriptRegExp(reader, static_cast<const JavascriptRegExp*>(var));
        RemoteScriptContext scriptContext(reader, var);
        RemoteThreadContext threadContext(reader, scriptContext.GetThreadContext());
        RemoteScriptConfiguration scriptConfiguration(reader, scriptContext.GetConfig());

        Js::Var lastIndex = regularExpression.GetLastIndex();
        if (lastIndex)
        {
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::lastIndex, lastIndex);
        }
        else
        {
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::lastIndex, 0);
        }

        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::global, regularExpression.IsGlobal());
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::multiline, regularExpression.IsMultiline());
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::ignoreCase, regularExpression.IsIgnoreCase());
        if (scriptConfiguration.IsES6UnicodeExtensionsEnabled())
        {
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::unicode, regularExpression.IsUnicode());
        }
        if (scriptConfiguration.IsES6RegExStickyEnabled())
        {
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::sticky, regularExpression.IsSticky());
        }
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::source, regularExpression.GetSource());
        pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::options, regularExpression.GetOptions(scriptConfiguration.IsES6UnicodeExtensionsEnabled(), scriptConfiguration.IsES6RegExStickyEnabled()));

        __super::InsertSpecialProperties(var);
    }

    void JavascriptFunctionWalker::InsertSpecialProperties(const DynamicObject* var)
    {
        AssertMsg(var, "The function var must be specified to insert special properties.");
        auto reader = m_context->GetReader();
        RemoteJavascriptFunction function = RemoteJavascriptFunction(reader, static_cast<const JavascriptFunction*>(var));
        RemoteScriptContext scriptContext(reader, var);
        RemoteThreadContext threadContext(reader, scriptContext.GetThreadContext());

        Js::Var value;
        CString error;

        if (function.HasRestrictedProperties())
        {
            if (function.GetCaller(m_context, &value, error))
            {
                pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::caller, value);
            }
            else
            {
                pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::caller, error);
            }

            CComPtr<IJsDebugPropertyInternal> pDebugProperty;
            if (function.GetArguments(m_context, &value, &pDebugProperty, error))
            {
                if (pDebugProperty)
                {
                    pThis()->InsertItem(PROPERTY_INFO(pDebugProperty));
                }
                else
                {
                    pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::arguments, value);
                }
            }
            else
            {
                pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::arguments, error);
            }
        }

        if (function.IsScriptFunction())
        {
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::length, function.GetLength());
        } 
        else if (function.IsBoundFunction(m_context))
        {
            PROPERTY_INFO propInfo;
            RemoteBoundFunction boundFunction = RemoteBoundFunction(reader, static_cast<const BoundFunction*>(var));
            uint16 value = boundFunction.GetLength(m_context, &propInfo);
            if (value == RemoteBoundFunction::TARGETS_RUNTIME_FUNCTION)
            {
                // it's a runtimefunction
                propInfo.name = s_lengthPropertyName;
                pThis()->InsertItem(propInfo);
            }
            else
            {
                pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::length, value);
            }
        }

        __super::InsertSpecialProperties(var);
    }

    void JavascriptStringObjectWalker::InsertSpecialProperties(const DynamicObject* var)
    {
        AssertMsg(var, "The string object var must be specified to insert special properties.");
        auto reader = m_context->GetReader();
        RemoteJavascriptStringObject stringObject = RemoteJavascriptStringObject(reader, static_cast<const JavascriptStringObject*>(var));
        RemoteScriptContext scriptContext(reader, var);
        RemoteThreadContext threadContext(reader, scriptContext.GetThreadContext());

        Js::JavascriptString* javascriptString = stringObject.GetValue();
        if (javascriptString)
        {
            RemoteJavascriptString remoteJavascriptString = RemoteJavascriptString(m_context, javascriptString);
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::length, remoteJavascriptString.GetLength());
        }
        else
        {
            pThis()->InsertSpecialProperty(&threadContext, var, Js::PropertyIds::length, 0);
        }

        __super::InsertSpecialProperties(var);
    }

    void ArgumentsObjectWalker::InsertSpecialProperties(const DynamicObject* var)
    {
        AssertMsg(var, "The arguments object var must be specified to insert special properties.");
        auto reader = m_context->GetReader();
        RemoteArgumentsObject arguments(reader, static_cast<const ArgumentsObject*>(var));
        RemoteScriptContext scriptContext(reader, var);
        RemoteScriptConfiguration scriptConfiguration(reader, scriptContext.GetConfig());

        __super::InsertSpecialProperties(var);
    }

    static CString GetShortFunctionDisplayName(const CString& name)
    {
        int i = name.ReverseFind(L'.');
        return i >= 0 ? name.Mid(i + 1) : name;
    }

    void JavascriptFunctionProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        auto reader = m_context->GetReader();
        RemoteJavascriptFunction func(reader, GetInstance());
        DiagScriptContext fakeScriptContext;
        CString str;

        FunctionBody* pFuncBody = func.GetFunction();
        if (pFuncBody)
        {
            RemoteFunctionBody body(reader, pFuncBody);
            RemoteUtf8SourceInfo sourceInfo(reader, body->GetUtf8SourceInfo());
            if (sourceInfo->GetIsLibraryCode())
            {
                CString displayName;
                if (body.TryReadDisplayName(&displayName))
                {
                    str = JavascriptFunction::GetLibraryCodeDisplayStringCommon<DiagStringBuilder, CString>(&fakeScriptContext, GetShortFunctionDisplayName(displayName));
                }
            }
            else
            {
                if (body->IsDefaultConstructor())
                {
                    str = (body->HasSuperReference())
                          ? JS_DEFAULT_EXTENDS_CTOR_DISPLAY_STRING
                          : JS_DEFAULT_CTOR_DISPLAY_STRING;
                }
                else
                {
                    size_t count = min(JavascriptFunction::DIAG_MAX_FUNCTION_STRING, body->LengthInChars());
                    size_t bytes = min(count * 3, body->LengthInBytes());

                    AutoArrayPtr<BYTE> pSource = VirtualReader::ReadBuffer(reader, sourceInfo.GetDebugModeSource() + body->m_cbStartOffset, bytes);
                    utf8::DecodeInto(str.GetBufferSetLength(count), pSource, count, utf8::doAllowThreeByteSurrogates);
                    str.ReleaseBufferSetLength(count);
                }
            }
        }
        else
        {
            Js::Var sourceString = func.GetSourceString(m_context);
            if (sourceString != nullptr)
            {
                Js::TypeId typeId = m_context->GetTypeId(sourceString);
                switch (typeId)
                {
                case Js::TypeIds_Integer:
                    {
                        RemoteScriptContext scriptContext(reader, GetInstance());
                        Js::PropertyId nameId = TaggedInt::ToInt32(sourceString);
                        str = JavascriptFunction::GetNativeFunctionDisplayStringCommon<DiagStringBuilder>(
                            &fakeScriptContext, m_context->ReadPropertyName(scriptContext.GetPropertyName(nameId)));
                    }
                    break;

                case Js::TypeIds_String:
                    {
                        str = m_context->ReadString((JavascriptString*)sourceString);
                    }
                    break;
                }
            }
        }

        if (str.IsEmpty())
        {
            str = fakeScriptContext.GetLibrary()->GetFunctionDisplayString();
        }

        *pValue = str.AllocSysString();
    }

    void JavascriptErrorProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        auto errorObject = GetInstance();

        PROPERTY_INFO description;
        if (m_context->GetProperty((Js::Var)errorObject, Js::PropertyIds::description, &description)
            && description.HasData()
            && m_context->GetTypeId(description.data) == Js::TypeIds_String)
        {
            *pValue = m_context->ReadString(static_cast<JavascriptString*>(description.data)).AllocSysString();
            return;
        }

        auto reader = m_context->GetReader();
        LPCWSTR originalRuntimeErrorMessage = RemoteJavascriptError(reader, errorObject)->originalRuntimeErrorMessage;
        if (originalRuntimeErrorMessage)
        {
            ToBSTR(originalRuntimeErrorMessage, pValue);
            return;
        }

        ToBSTR(L"", pValue); // Display an empty string
    }

    //
    // Helper to support a few ScriptContext fields to help get date value display with shared runtime code.
    //
    class FakeDateImplementationScriptContext
    {
    private:
        RemoteData<ScriptConfiguration> m_config;               // remote scriptContext->config
        RemoteData<DaylightTimeHelper> m_daylightTimeHelper;    // remote scriptContext->daylightTimeHelper
        TIME_ZONE_INFORMATION m_timeZoneInfo;                   // local

    public:
        FakeDateImplementationScriptContext(const RemoteScriptContext& scriptContext):
            m_config(scriptContext.GetReader(), scriptContext.GetConfig()),
            m_daylightTimeHelper(scriptContext.GetReader(), scriptContext.GetDaylightTimeHelper())
        {
        }

        const ScriptConfiguration* GetConfig() const { return m_config.ToTargetPtr(); }
        DaylightTimeHelper* GetDaylightTimeHelper() { return m_daylightTimeHelper.ToTargetPtr(); }

        TIME_ZONE_INFORMATION* GetTimeZoneInfo()
        {
            GetTimeZoneInformation(&m_timeZoneInfo);
            return &m_timeZoneInfo;
        }
    };

    void JavascriptDateProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        auto reader = m_context->GetReader();
        RemoteJavascriptDate instance(reader, GetInstance());
        DateImplementation& date = instance->m_date; // Now this is in local memory inside "instance"

        FakeDateImplementationScriptContext fakeScriptContext(RemoteScriptContext(reader, date.m_scriptContext));
        DiagStringBuilder bs;
        date.GetDiagValueString<DiagStringBuilder>(&fakeScriptContext,
            [&](int capacity) -> DiagStringBuilder*
        {
            return &bs;
        });

        *pValue = bs.GetString().AllocSysString();
    }

    void JavascriptVariantDateProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        auto reader = m_context->GetReader();
        Assert(m_info.HasData());
        RemoteJavascriptVariantDate variantDate(reader, reinterpret_cast<JavascriptVariantDate*>(m_info.data));

        FakeDateImplementationScriptContext fakeScriptContext(RemoteScriptContext(reader, variantDate.GetScriptContext()));
        DiagStringBuilder bs;
        DateImplementation::ConvertVariantDateToString<DiagStringBuilder>(variantDate->GetValue(), &fakeScriptContext,
            [&](int capacity) -> DiagStringBuilder*
        {
            return &bs;
        });

        *pValue = bs.GetString().AllocSysString();
    }

    void JavascriptRegExpProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        IVirtualReader* reader = m_context->GetReader();

        PCWSTR source;
        charcount_t len;
        RemoteJavascriptRegExp(reader, GetInstance()).GetSource(&source, &len);

        CString s = InspectionContext::ReadStringLen(reader, source, len);
        *pValue = s.AllocSysString();
    }

    template <class T, class Array, template <bool requireEnumerable> class Enumerator>
    void EnumeratorArrayWalker<T, Array, Enumerator>::Init(InspectionContext* context, const DynamicObject* var, _In_ IJsDebugPropertyInternal* ownerDebugProperty)
    {
        __super::Init(context, var, ownerDebugProperty);

        const Array* arr = static_cast<const Array*>(var);
        m_arrayItemsCount = 0;

        // Walk the array to figure out real array items count
        m_iter.Init(context, arr);
        while (m_iter.MoveNext())
        {
            ++m_arrayItemsCount;
        }

        m_next = m_arrayItemsCount; // mark position out of bound
    }

    template <class T, class Array, template <bool requireEnumerable> class Enumerator>
    bool EnumeratorArrayWalker<T, Array, Enumerator>::GetNextArrayItem(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
    {
        return __super::GetNextArrayItem(index, [=]
        {
            if (m_next != index)
            {
                m_iter.Reset();
                for (m_next = 0; m_next < index; m_next++)
                {
                    m_iter.MoveNext();
                }
            }

            Assert(m_next == index);
            m_next++;
            if (!m_iter.MoveNext())
            {
                DiagException::Throw(E_UNEXPECTED, DiagErrorCode::ARRAY_ITER_MOVENEXT);
            }

            m_context->CreateDebugProperty(m_iter.GetPropertyInfo(), GetOwnerDebugProperty(), ppDebugProperty);
        });
    }

    template <class T, class Array, template <bool requireEnumerable> class Enumerator>
    bool_result EnumeratorArrayWalker<T, Array, Enumerator>::TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        m_iter.Reset();
        m_next = 0;

        while (m_iter.MoveNext())
        {
            m_next++;

            if (m_iter.GetIndex() == index)
            {
                m_context->CreateDebugProperty(m_iter.GetPropertyInfo(), GetOwnerDebugProperty(), ppDebugProperty);
                return true;
            }
        }
        return false;
    }

    template <class T, class RemoteType, class Walker>
    template <bool requireEnumerable>
    void BaseArrayProperty<T, RemoteType, Walker>::EnumItems(IPropertyListener* listener, uint start, uint end) const
    {
        Walker::ArrayEnumerator<requireEnumerable>::EnumeratorType e;
        e.Init(m_context, GetInstance(), start, end);

        while (e.MoveNext())
        {
            if (!listener->EnumItem(e.GetIndex(), e.GetPropertyInfo()))
            {
                break;
            }
        }
    }

    template <class T, class RemoteType, class Walker>
    void BaseArrayProperty<T, RemoteType, Walker>::EnumItems(IPropertyListener* listener, uint start, uint end, bool requireEnumerable) const
    {
        if (requireEnumerable)
        {
            EnumItems</*requireEnumerable*/true>(listener, start, end);
        }
        else
        {
            EnumItems</*requireEnumerable*/false>(listener, start, end);
        }
    }

    SmallArrayListener::SmallArrayListener(uint length):
        count(0),
        length(length)
    {
    }

    bool SmallArrayListener::EnumItem(uint index, const PROPERTY_INFO& info)
    {
        if (props[index].IsEmpty())
        {
            props[index] = info;
            ++count;
        }
        return count < length;
    }

    void SmallArrayListener::FillFromPrototypes(InspectionContext* context, const RecyclableObject* instance)
    {
        const RecyclableObject* obj = instance;
        while (count < length)
        {
            obj = RemoteRecyclableObject(context->GetReader(), obj).GetPrototype();
            if (context->GetTypeId((Js::Var)obj) == Js::TypeIds_Null)
            {
                break;
            }

            CComPtr<IJsDebugPropertyInternal> prop;
            context->CreateDebugProperty(PROPERTY_INFO((Js::Var)obj), /*parent*/nullptr, &prop);
            prop->EnumItems(this, 0, length, /*requireEnumerable*/false);
        }
    }

    CString SmallArrayListener::Join(InspectionContext* context)
    {
        CString str;
        for (uint32 i = 0; i < length; i++)
        {
            if (i > 0)
            {
                str += L",";
            }

            const PROPERTY_INFO& prop = props[i];
            if (prop.IsEmpty() || prop.type == PROPERTY_INFO::ACCESSOR_PROPERTY)
            {
                // Empty/accessor properties not handled.
                continue;
            }
            if (prop.HasData())
            {
                // Var property: check for null/undefined.
                Js::Var var = prop.data;
                auto typeId = context->GetTypeId(var);
                if (typeId == Js::TypeIds_Null || typeId == Js::TypeIds_Undefined)
                {
                    continue;
                }
            }

            CComPtr<IJsDebugPropertyInternal> dbgProp;
            context->CreateDebugProperty(prop, /*parent*/nullptr, (IJsDebugProperty**)&dbgProp);

            CComBSTR bstr;
            dbgProp->GetDisplayValue(10, &bstr);

            str.Append(bstr, ::SysStringLen(bstr));
        }
        return str;
    }

    template <class T, class RemoteType, class Walker>
    CString BaseArrayProperty<T, RemoteType, Walker>::GetValueString() const
    {
        RemoteType arr(m_context->GetReader(), GetInstance());
        uint length = arr->length;

        if (length >= SmallArrayListener::MAX_SMALL_ARRAY_SIZE)
        {
            return L"[...]";
        }

        CString str = L"[";
        Js::Var instance = (Js::Var)GetInstance();
        if (!m_context->CheckObject(instance)) // prevent loop
        {
            InspectionContext::AutoOpStackObject autoOpStackObject(m_context, instance); // prevent loop
            SmallArrayListener items(length);

            pThis()->EnumItems(&items, 0, length, /*requireEnumerable*/false);
            if (items.count < length)
            {
                items.FillFromPrototypes(m_context, GetInstance());
            }

            str += items.Join(m_context);
        }
        str += L"]";

        return str;
    }

    template<typename T>
    void RemoteArrayElementEnumerator<T>::Init(InspectionContext* context, const JavascriptArray* instance, uint32 start, uint32 end)
    {
        auto reader = context->GetReader();
        RemoteJavascriptArray arr(reader, instance);

        m_context = context;
        m_arr = instance;
        m_start = start;
        m_end = min(end, arr->length);
        m_seg = nullptr;
        m_item = NULL;

        // Find start segment
        auto segment = (Js::SparseArraySegment<T>*)arr->head;
        while (segment)
        {
            RemoteSparseArraySegment<T> seg(reader, segment);
            if (seg->left + seg->length <= m_start)
            {
                segment = (Js::SparseArraySegment<T>*)seg->next;
                continue;
            }

            if (seg->left < m_end)
            {
                // Found segment. Set start index and endIndex
                m_seg = new(oomthrow) RemoteSparseArraySegment<T>(reader, segment);

                // set index to be at target index - 1, so MoveNext will move to target
                m_left = seg->left;
                m_index = max(m_left, m_start) - m_left - 1;
                m_endIndex = min(m_end - m_left, seg->length);
            }

            break;
        }
    }

    template<typename T>
    void RemoteArrayElementEnumerator<T>::Reset()
    {
        Init(m_context, m_arr, m_start, m_end); // Re-Init to correct position
    }

    template<typename T>
    bool RemoteArrayElementEnumerator<T>::MoveNext()
    {
        while (m_seg)
        {
            // Look for next non-null item in current segment
            while (++m_index < m_endIndex)
            {
                m_item = m_seg->Item(m_index);
                if (!SparseArraySegment<T>::IsMissingItem(&m_item))
                {
                    return true;
                }
            }

            // Move to next segment
            auto next = (Js::SparseArraySegment<T>*)m_seg->ToTargetPtr()->next;
            if (next)
            {
                m_seg = new(oomthrow) RemoteSparseArraySegment<T>(m_seg->GetReader(), next);
                m_left = m_seg->ToTargetPtr()->left;
                if (m_left < m_end)
                {
                    m_index = static_cast<uint32>(-1);
                    m_endIndex = min(m_end - m_left, m_seg->ToTargetPtr()->length);
                    continue; // Found next segment
                }
            }

            m_seg = nullptr; // DONE
            m_item = NULL;
        }

        return false;
    }

    template<typename T>
    uint32 RemoteArrayElementEnumerator<T>::GetIndex() const
    {
        return m_left + m_index;
    }

    template<typename T>
    T RemoteArrayElementEnumerator<T>::GetItem() const
    {
        return m_item;
    }

    void RemoteIndexPropertyMapEnumerator::Init(IVirtualReader* reader, const IndexPropertyDescriptorMap* map, uint start, uint end)
    {
        RemoteData<IndexPropertyDescriptorMap> imap(reader, map);
        RemoteDictionary<IndexPropertyDescriptorMap::InnerMap> indexPropertyMap(reader, imap->indexPropertyMap);

        uint count = static_cast<uint>(indexPropertyMap->Count());
        if (!m_items.SetCount(count))
        {
            DiagException::ThrowOOM();
        }

        for (uint i = 0; i < count; i++)
        {
            m_items[i] = indexPropertyMap.Item(i);
        }

        qsort_s(m_items.GetData(), m_items.GetCount(), sizeof(EntryType), CompareEntry, nullptr);

        // Find start/end index. CONSIDER: binary search
        uint i = 0;
        while (i < count && m_items[i].Key() < start)
        {
            i++;
        }

        if (i < count)
        {
            m_startIndex = i;

            while (i < count && m_items[i].Key() < end)
            {
                i++;
            }
            m_endIndex = i;
        }
        else
        {
            m_endIndex = 0; // none
        }

        Reset();
    }

    void RemoteIndexPropertyMapEnumerator::Reset()
    {
        m_cur = m_startIndex - 1;
    }

    bool RemoteIndexPropertyMapEnumerator::MoveNext()
    {
        return ++m_cur < m_endIndex;
    }

    uint32 RemoteIndexPropertyMapEnumerator::GetIndex() const
    {
        return m_items[m_cur].Key();
    }

    const IndexPropertyDescriptor& RemoteIndexPropertyMapEnumerator::GetItem() const
    {
        return m_items[m_cur].Value();
    }

    PROPERTY_INFO RemoteIndexPropertyMapEnumerator::GetPropertyInfo() const
    {
        auto item = GetItem();
        return PROPERTY_INFO(GetIndex(), item.Getter, item.Setter);
    }

    int __cdecl RemoteIndexPropertyMapEnumerator::CompareEntry(_In_ void* context, _In_ const void* item1, _In_ const void* item2)
    {
        const EntryType* p1 = reinterpret_cast<const EntryType*>(item1);
        const EntryType* p2 = reinterpret_cast<const EntryType*>(item2);

        Assert(p1->Key() != p2->Key());
        return p1->Key() < p2->Key() ? -1 : 1;
    }

    template <bool requireEnumerable>
    void RemoteES5ArrayItemEnumerator<requireEnumerable>::Init(InspectionContext* context, const ES5Array* arr, uint start, uint end)
    {
        auto reader = context->GetReader();
        end = min(end, RemoteJavascriptArray(reader, arr)->length);

        m_dataEnumerator.Init(context, arr, start, end);

        RemoteDynamicObject obj(reader, arr);
        RemoteData<DynamicType> type(reader, obj->GetDynamicType());
        RemoteData<ES5ArrayTypeHandler> typeHandler(reader, (ES5ArrayTypeHandler*)type->GetTypeHandler()); //CONSIDER: small/big ES5ArrayTypeHandler
        m_descriptorEnumerator.Init(reader, typeHandler->indexPropertyMap, start, end);

        // Only place not doing fullReset, because the two enumerators are just created
        Reset(/*fullReset*/false);
    }

    template <bool requireEnumerable>
    void RemoteES5ArrayItemEnumerator<requireEnumerable>::Reset(bool fullReset)
    {
        if (fullReset)
        {
            m_dataEnumerator.Reset();
            m_descriptorEnumerator.Reset();
        }

        m_dataIndex = JavascriptArray::InvalidIndex;
        m_descriptorIndex = JavascriptArray::InvalidIndex;
        m_index = JavascriptArray::InvalidIndex;
    }

    template <bool requireEnumerable>
    bool RemoteES5ArrayItemEnumerator<requireEnumerable>::MoveNext()
    {
        while (true)
        {
            Assert(m_index == min(m_dataIndex, m_descriptorIndex));

            if (m_index == m_dataIndex)
            {
                m_dataIndex = m_dataEnumerator.MoveNext() ? m_dataEnumerator.GetIndex() : JavascriptArray::InvalidIndex;
            }

            if (m_index == m_descriptorIndex)
            {
                m_descriptorIndex = m_descriptorEnumerator.MoveNext() ? m_descriptorEnumerator.GetIndex() : JavascriptArray::InvalidIndex;
            }

            m_index = min(m_dataIndex, m_descriptorIndex);
            if (m_index == JavascriptArray::InvalidIndex)
            {
                break;
            }

            if (m_index < m_descriptorIndex
                || (!requireEnumerable || (m_descriptorEnumerator.GetItem().Attributes & PropertyEnumerable)))
            {
                return true;
            }
        }

        return false;
    }

    template <bool requireEnumerable>
    uint32 RemoteES5ArrayItemEnumerator<requireEnumerable>::GetIndex() const
    {
        return m_index;
    }

    template <bool requireEnumerable>
    PROPERTY_INFO RemoteES5ArrayItemEnumerator<requireEnumerable>::GetPropertyInfo() const
    {
        return m_index == m_dataIndex ?
            m_dataEnumerator.GetPropertyInfo() : m_descriptorEnumerator.GetPropertyInfo();
    }

    void RemoteHeapArgumentsObject::GetNamedItems(InspectionContext* context, CAtlArray<PROPERTY_INFO>& arr)
    {
        uint count = min(ToTargetPtr()->formalCount, ToTargetPtr()->numOfArguments);
        if (count == 0)
        {
            return;
        }

        if (!arr.SetCount(count))
        {
            DiagException::ThrowOOM();
        }

        const DynamicObject* frameObject = GetFrameObject();
        const DynamicTypeHandler* typeHandler = RemoteDynamicObject(m_reader, frameObject).GetTypeHandler();
        RemoteDynamicTypeHandler frame(context, typeHandler, frameObject);

        for (uint i = 0; i < count; i++)
        {
            if (!m_deletedArgs || !m_deletedArgs->Test(i))
            {
                Js::Var var = frame.ReadSlot(i);
                if (var)
                {
                    arr[i] = PROPERTY_INFO(i, var);
                }
            }
        }
    }

    void ArgumentsObjectWalker::Init(InspectionContext* context, const DynamicObject* var, IJsDebugPropertyInternal* prop)
    {
        __super::Init(context, var, prop);

        CreateComObject(context, prop, &m_args);
        auto listener = MakePropertyListener([=](const PROPERTY_INFO& info, const Js::PropertyId propertyId) -> bool
        {
            m_args->InsertItem(info);
            return true;
        });
        prop->EnumItems(&listener, 0, JavascriptArray::InvalidIndex, /*requireEnumerable*/false);
    }

    uint ArgumentsObjectWalker::GetInternalArrayCount() const
    {
        return m_args->GetCount();
    }

    bool ArgumentsObjectWalker::TryGetInternalArrayItem(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
    {
        return m_args->GetNextProperty(index, ppDebugProperty);
    }

    bool_result ArgumentsObjectWalker::TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        // m_args may have deleted arguments, and also sparse internal array items. So looking for name instead.
        CString name = PROPERTY_INFO::FromUINT(index);
        return m_args->TryGetProperty(name, ppDebugProperty);
    }

    void ArgumentsObjectProperty::EnumItems(IPropertyListener* listener, uint start, uint end, bool requireEnumerable) const
    {
        auto reader = m_context->GetReader();
        const HeapArgumentsObject* instance = static_cast<const HeapArgumentsObject*>(GetInstance());
        CAtlArray<PROPERTY_INFO> args;

        // Collect named args from frame
        RemoteHeapArgumentsObject(reader, instance).GetNamedItems(m_context, args);

        const uint namedCount = static_cast<uint>(args.GetCount());
        bool hasEnumNamed = false;
        auto enumNamed = [&]() -> bool
        {
            if (!hasEnumNamed)
            {
                hasEnumNamed = true;
                for (uint i = start; i < min(namedCount, end); i++)
                {
                    if (!args[i].IsEmpty())
                    {
                        if (!listener->EnumItem(i, args[i]))
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        };

        // Collect disconnected args from object
        RemoteDynamicObject obj(reader, instance);
        if (obj.HasObjectArray(m_context))
        {
            auto localListener = MakeItemListener([&](uint i, const PROPERTY_INFO& info) -> bool
            {
                if (i < namedCount)
                {
                    if (args[i].IsEmpty())
                    {
                        args[i] = info;
                    }

                    return true;
                }
                else
                {
                    return enumNamed() && listener->EnumItem(i, info);
                }
            });

            Js::Var arr = obj->GetObjectArrayOrFlagsAsArray();
            CComPtr<IJsDebugPropertyInternal> prop;
            m_context->CreateDebugProperty(PROPERTY_INFO(arr), /*parent*/nullptr, &prop);
            prop->EnumItems(&localListener, start, end, requireEnumerable);
        }

        // Ensure enumerated named args if not already
        enumNamed();
    }

    void FakeObjectProperty::Init(InspectionContext* context, const CString& displayName, LPCWSTR displayType, RecyclableObject* prototype, _In_opt_ IJsDebugPropertyInternal* parent)
    {
        __super::Init(context, PROPERTY_INFO(displayName, (Js::Var)nullptr), parent);
        __super::SetTypeString(displayType);
        m_prototype = prototype;
    }

    void FakeObjectProperty::Init(const CString& newName, const FakeObjectProperty& other)
    {
        Init(other.m_context, newName, other.GetTypeDirect(), other.m_prototype, nullptr);
        m_properties.Copy(other.m_properties);
        m_items.Copy(other.m_items);
    }

    bool_result FakeObjectProperty::TryCloneImpl(const CString& newName, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) const
    {
        CreateComObject<FakeObjectProperty>(newName, *this, ppDebugProperty);
        return true;
    }

    bool_result FakeObjectProperty::TryCreateWalker(_Outptr_ WalkerType** ppWalker)
    {
        CreateComObject(m_context, this, ppWalker);
        return true;
    }

    void FakeObjectWalker::Init(InspectionContext* context, FakeObjectProperty* fakeObject)
    {
        m_fakeObject = fakeObject;
        __super::Init(context, nullptr, fakeObject);
    }

    RecyclableObject* FakeObjectWalker::GetPrototype(const DynamicObject* var) const
    {
        Assert(var == nullptr);
        return m_fakeObject->GetPrototype();
    }

    void FakeObjectWalker::InsertProperties(const DynamicObject* var)
    {
        Assert(var == nullptr);
        const CAtlArray<PROPERTY_INFO>& properties = m_fakeObject->GetProperties();

        // Although owner FakeObject already does book keeping of all its properties, we still need to
        // make a copy here because walker may reclassify them, e.g. putting functions into [Methods].
        for (uint i = 0; i < properties.GetCount(); i++)
        {
            __super::InsertItem(properties[i]);
        }
    }

    uint FakeObjectWalker::GetInternalArrayCount() const
    {
        const CAtlArray<PROPERTY_INFO>& items = m_fakeObject->GetItems();
        return items.GetCount();
    }

    bool_result FakeObjectWalker::TryGetInternalArrayItem(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty)
    {
        if (index < GetInternalArrayCount())
        {
            m_context->CreateDebugProperty(m_fakeObject->GetItems()[index], GetOwnerDebugProperty(), ppDebugProperty);
            return true;
        }
        return false;
    }

    bool_result FakeObjectWalker::TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        const CAtlArray<PROPERTY_INFO>& items = m_fakeObject->GetItems();
        return FindProperty(items, PROPERTY_INFO::FromUINT(index), [=](const PROPERTY_INFO& prop)
        {
            m_context->CreateDebugProperty(prop, GetOwnerDebugProperty(), ppDebugProperty);
        });
    }

    void KeyValueWalker::Init(InspectionContext* context, Js::Var key, Js::Var value)
    {
        PROPERTY_INFO infoKey(L"key", key);
        PROPERTY_INFO infoValue(L"value", value);

        context->CreateDebugProperty(infoKey, /*parent*/nullptr, &m_key);
        context->CreateDebugProperty(infoValue, /*parent*/nullptr, &m_value);
    }

    bool_result KeyValueWalker::GetNextProperty(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty)
    {
        if (index == 0)
        {
            *ppDebugProperty = GetKeyProperty();
            (*ppDebugProperty)->AddRef();
            return true;
        }
        else if (index == 1)
        {
            *ppDebugProperty = GetValueProperty();
            (*ppDebugProperty)->AddRef();
            return true;
        }

        return false;
    }

    void KeyValueProperty::Init(InspectionContext* context, const CString& name, Js::Var key, Js::Var value)
    {
        m_name = name;
        CreateComObject(context, key, value, &m_walker);
    }

    void KeyValueProperty::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        CString valueString;
        valueString.Append(L"[");

        CComBSTR bstrKey;
        CComBSTR bstrValue;

        m_walker->GetKeyProperty()->GetDisplayValue(nRadix, &bstrKey);
        valueString.Append(bstrKey, ::SysStringLen(bstrKey));

        valueString.Append(L", ");

        m_walker->GetValueProperty()->GetDisplayValue(nRadix, &bstrValue);
        valueString.Append(bstrValue, ::SysStringLen(bstrValue));

        valueString.Append(L"]");

        *pValue = valueString.AllocSysString();
    }

    void MapWalker::InsertProperties(const Js::JavascriptMap* map)
    {
        RemoteData<const Js::JavascriptMap> remoteMap(m_context->GetReader(), map);
        Js::JavascriptMap::MapDataNode* node = remoteMap->list.first;
        ULONG index = 0;

        while (node != nullptr)
        {
            RemoteData<const Js::JavascriptMap::MapDataNode> remoteNode(m_context->GetReader(), node);

            CString name;
            name.Format(L"%u", index);

            CComPtr<KeyValueProperty> keyValueProperty;
            CreateComObject(m_context, name, remoteNode->data.Key(), remoteNode->data.Value(), &keyValueProperty);

            pThis()->InsertItem(keyValueProperty);

            node = remoteNode->next;
            index += 1;
        }
    }

    void SetWalker::InsertProperties(const Js::JavascriptSet* set)
    {
        RemoteData<const Js::JavascriptSet> remoteSet(m_context->GetReader(), set);
        Js::JavascriptSet::SetDataNode* node = remoteSet->list.first;
        UINT index = 0;

        while (node != nullptr)
        {
            RemoteData<const Js::JavascriptSet::SetDataNode> remoteNode(m_context->GetReader(), node);

            CString name;
            name.Format(L"%u", index);

            PROPERTY_INFO info(name, remoteNode->data);
            pThis()->InsertItem(info);

            node = remoteNode->next;
            index += 1;
        }
    }

    void WeakMapWalker::InsertProperties(const Js::JavascriptWeakMap* weakMap)
    {
        RemoteWeaklyReferencedKeyDictionary<Js::JavascriptWeakMap::KeySet> remoteKeySet(m_context->GetReader(), &weakMap->keySet);
        UINT index = 0;

        remoteKeySet.Map([&] (const DynamicObject* key, bool)
        {
            const Js::JavascriptWeakMap::WeakMapKeyMap* keyMap = GetWeakMapKeyMapFromKey(key);

            if (keyMap != nullptr)
            {
                RemoteDictionary<Js::JavascriptWeakMap::WeakMapKeyMap> remoteWeakMap(m_context->GetReader(), keyMap);
                Js::Var value;

                // Note: GetWeakMapId uses the this pointer value of the JavascriptWeakMap object
                // to compute the ID, thus, it is correct to call it on the remote process's pointer
                // not the RemoteData object, remoteWeakMap.
                if (!remoteWeakMap.TryGetValue(weakMap->GetWeakMapId(), &value))
                {
                    DiagException::Throw(E_UNEXPECTED, DiagErrorCode::RUNTIME_WEAKMAP_GETVALUE);
                }

                CString name;
                name.Format(L"%u", index);

                CComPtr<KeyValueProperty> keyValueProperty;
                CreateComObject(m_context, name, (Js::Var)key, value, &keyValueProperty);

                pThis()->InsertItem(keyValueProperty);

                index += 1;
            }
        });
    }

    const Js::JavascriptWeakMap::WeakMapKeyMap* WeakMapWalker::GetWeakMapKeyMapFromKey(const DynamicObject* key)
    {
        AutoPtr<ITypeHandler> typeHandler = m_context->CreateTypeHandler(key);

        PROPERTY_INFO prop;
        if (typeHandler != nullptr &&
            typeHandler->GetInternalProperty(m_context->GetInternalPropertyRecord(InternalPropertyIds::WeakMapKeyMap), &prop))
        {
            Assert(prop.type == PROPERTY_INFO::DATA_PROPERTY);
            return static_cast<const Js::JavascriptWeakMap::WeakMapKeyMap*>(prop.data);
        }

        return nullptr;
    }

    void WeakSetWalker::InsertProperties(const Js::JavascriptWeakSet* weakSet)
    {
        RemoteWeaklyReferencedKeyDictionary<Js::JavascriptWeakSet::KeySet> remoteKeySet(m_context->GetReader(), &weakSet->keySet);
        UINT index = 0;

        remoteKeySet.Map([&] (const DynamicObject* key, bool)
        {
            CString name;
            name.Format(L"%u", index);

            PROPERTY_INFO info(name, (Js::Var)key);
            pThis()->InsertItem(info);

            index += 1;
        });
    }

    void InspectionContext::Init(JsDebugProcess* process)
    {
        m_debugProcess = process;

        const VTABLE_PTR* vtables;
        ULONG vtablesSize;
        m_debugProcess->GetDebugClient()->GetVTables(&vtables, &vtablesSize);

        InitVTables(vtables, vtablesSize);
    }

    //
    // Read a null-terminated string
    //
    CString InspectionContext::ReadString(IVirtualReader* reader, LPCWSTR addr, UINT maxLen)
    {
        CString str;
        LPWSTR buf = str.GetBuffer(maxLen);
        HRESULT hr = reader->ReadString(addr, buf, maxLen);
        CheckHR(hr, DiagErrorCode::READ_STRING);
        str.ReleaseBuffer();

        return str;
    }

    //
    // Read a string of known length into a buffer
    //
    void InspectionContext::ReadStringLen(IVirtualReader* reader, const wchar_t* addr, _Out_writes_all_(len) wchar_t* buf, charcount_t len)
    {
        // NOTE: Javascript string can contain embeded NULL. Do not handle it as LPCWSTR.
        // Read memory directly for string content since we know exact string length.

        ULONG bytes = len * sizeof(wchar_t);
        ULONG bytesRead;
        HRESULT hr = reader->ReadVirtual(addr, buf, bytes, &bytesRead);
        CheckHR(hr, DiagErrorCode::READ_STRING);
        if (bytesRead != bytes)
        {
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::READ_STRING_MISMATCH);
        }
    }

    //
    // Read a string of known length
    //
    CString InspectionContext::ReadStringLen(IVirtualReader* reader, const wchar_t* addr, charcount_t len)
    {
        if (len >= JavascriptString::MaxCharLength)
        {
            DiagException::ThrowOOM(DiagErrorCode::REMOTE_STRING_TOO_LONG);
        }

        CString str;
        LPWSTR buf = str.GetBufferSetLength(len);
        ReadStringLen(reader, addr, buf, len);
        str.ReleaseBuffer(len);
        return str;
    }

    CString InspectionContext::ReadPropertyName(IVirtualReader* reader, const PropertyRecord* propertyRecord)
    {
        RemoteData<PropertyRecord> prop(reader, propertyRecord);

        if (prop->GetPropertyId() == PropertyIds::_superReferenceSymbol)
        {
            return CString("super");
        }

        return ReadStringLen(reader, propertyRecord->GetBuffer(), prop->GetLength());
    }

    CString InspectionContext::ReadPropertyName(const PropertyRecord* propertyRecord) const
    {
        return ReadPropertyName(this->GetReader(), propertyRecord);
    }

    bool InspectionContext::ReadString(JavascriptString* s, _Out_writes_to_(bufLen, *actual) wchar_t* buf, _In_ charcount_t bufLen, _Out_ charcount_t* actual)
    {
        IRemoteStringFactory* fac = GetRemoteStringFactory(s);
        if (fac != nullptr)
        {
            fac->Read(this, s, buf, bufLen, actual);
            return true;
        }
        else
        {
            *actual = 0; // Unknown string (NYI?)
            return false;
        }
    }

    CString InspectionContext::ReadString(JavascriptString* s)
    {
        charcount_t len = RemoteJavascriptString(this, s).GetLength();
        charcount_t actual;

        if (len >= JavascriptString::MaxCharLength)
        {
            DiagException::ThrowOOM(DiagErrorCode::REMOTE_STRING_TOO_LONG);
        }

        CString str;
        LPWSTR buf = str.GetBufferSetLength(len);
        ReadString(s, buf, len, &actual);
        str.ReleaseBuffer(actual);
        return str;
    }

    const RemoteJavascriptLibrary& InspectionContext::GetJavascriptLibrary() const
    {
        Assert(m_javascriptLibrary);
        return *m_javascriptLibrary;
    }

    void InspectionContext::GetUndefinedProperty(_Outptr_ IJsDebugPropertyInternal** ppUndefined)
    {
        CreateDebugProperty(PROPERTY_INFO(m_javascriptLibrary->GetUndefined()), /*parent*/nullptr, ppUndefined);
    }

    void InspectionContext::InitLocalsContext(const ScriptContext* scriptContext)
    {
        IVirtualReader* reader = GetReader();

        m_javascriptLibrary = new(oomthrow) RemoteJavascriptLibrary(reader, scriptContext);

        RemoteData<ObjectPrototypeObject> objectPrototype(reader, m_javascriptLibrary->GetObjectPrototype());
        __proto__state = objectPrototype->is__proto__Enabled() ? ENABLED : DISABLED;
    }

    bool InspectionContext::is__proto__Enabled() const
    {
        return __proto__state == ENABLED;
    }

    const PCWSTR InspectionContext::GetPrototypeDisplay() const
    {
        return is__proto__Enabled() ? L"__proto__" : L"[prototype]";
    }

    void InspectionContext::TryResolve__proto__Value(const RecyclableObject* instance, CAtlArray<PROPERTY_INFO>& items)
    {
        //
        // If __proto__ enabled and instance is Object.prototype, we know Object.prototype.__proto__ has value null.
        //
        if (is__proto__Enabled())
        {
            if (instance == m_javascriptLibrary->GetObjectPrototype())
            {
                CString __proto__(L"__proto__");
                for (uint i = 0; i < static_cast<uint>(items.GetCount()); i++)
                {
                    if (items[i].type == PROPERTY_INFO::ACCESSOR_PROPERTY
                        && items[i].name == __proto__)
                    {
                        items[i].data = m_javascriptLibrary->GetNull();
                        items[i].type = PROPERTY_INFO::DATA_PROPERTY;
                        break;
                    }
                }
            }
        }
    }

    void InspectionContext::PushObject(Js::Var var)
    {
        m_opStack.AddTail(var);
    }

    Js::Var InspectionContext::PopObject()
    {
        return m_opStack.RemoveTail();
    }

    bool InspectionContext::CheckObject(Js::Var var)
    {
        return m_opStack.Find(var) != nullptr;
    }

    InspectionContext::AutoOpStackObject::AutoOpStackObject(InspectionContext* context, Js::Var var):
        m_context(context)
    {
        m_context->PushObject(var);
#if DBG
        m_var = var;
#endif
    }

    InspectionContext::AutoOpStackObject::~AutoOpStackObject()
    {
        Js::Var var = m_context->PopObject();
        Assert(var == m_var);
    }

    static ITypeHandlerFactory* s_typeHandlerFactory[] = {
#define TYPEHANDLER_ENTRY(s) &TypeHandlerFactory<##s##, Remote##s##>::s_instance,
#include "DiagVTableList.h"
    };

    static IRemoteStringFactory* m_remoteStringFactory[] = {
#define STRING_ENTRY(s) &RemoteStringFactory<##s##, Remote##s##>::s_instance,
#include "DiagVTableList.h"
    };

    void InspectionContext::InitVTables(_In_reads_(vtablesSize) const VTABLE_PTR* vtables, ULONG vtablesSize)
    {
        Assert(vtablesSize == Diag_MaxVTable);

        for (ULONG i = Diag_FirstTypeHandler; i <= Diag_LastTypeHandler; i++)
        {
            m_typeHandlerMap[vtables[i]] = s_typeHandlerFactory[i - Diag_FirstTypeHandler];
        }

        for (ULONG i = Diag_FirstString; i <= Diag_LastString; i++)
        {
            m_remoteStringMap[vtables[i]] = m_remoteStringFactory[i - Diag_FirstString];
        }
    }

    //
    // Read vtable of a Var (must be a RecyclableObject).
    //
    const void* InspectionContext::ReadVTable(Js::Var var) const
    {
        return RemoteRecyclableObject(GetReader(), reinterpret_cast<RecyclableObject*>(var)).ReadVTable();
    }

    //
    // Check if a given vtable is within a range (inclusive).
    //
    bool InspectionContext::IsVTable(const void* vtable, Diag_VTableType first, Diag_VTableType last) const
    {
        return GetDebugClient()->IsVTable(vtable, first, last);
    }

    //
    // Match the vtable of a Var (must be a RecyclableObject) against a known vtable type.
    //
    bool InspectionContext::MatchVTable(Js::Var var, Diag_VTableType vtable) const
    {
        return ReadVTable(var) == GetDebugClient()->GetVTable(vtable);
    }

    ITypeHandler* InspectionContext::CreateTypeHandler(const DynamicObject* var)
    {
        auto reader = GetReader();
        RemoteDynamicObject obj(reader, var);
        RemoteData<DynamicType> type(reader, obj->GetDynamicType());
        RemoteData<DynamicTypeHandler> typeHandler(reader, type->GetTypeHandler());

        VTABLE_PTR vtable = typeHandler.ReadVTable();
        ITypeHandlerFactory* fac;
        if (m_typeHandlerMap.Lookup(vtable, fac))
        {
            return fac->Create(this, typeHandler.GetRemoteAddr(), var);
        }

        return nullptr; // Failed to identify remote type handler
    }

    //
    // Get the factory for a remote string, throw if can't recognize the remote string type.
    //
    IRemoteStringFactory* InspectionContext::GetRemoteStringFactory(Js::Var var)
    {
        Assert(GetTypeId(var) == Js::TypeIds_String);

        VTABLE_PTR vtable = ReadVTable(var);

        IRemoteStringFactory* fac;
        if (m_remoteStringMap.Lookup(vtable, fac))
        {
            return fac;
        }

        AssertMsg(false, "Unknown string type?");
        DiagException::Throw(E_UNEXPECTED, DiagErrorCode::UNKNOWN_REMOTESTRING_TYPE);
    }

    Js::TypeId InspectionContext::GetTypeId(Js::Var var) const
    {
        if (Js::TaggedInt::Is(var))
        {
            return Js::TypeIds_Integer;
        }
#if FLOATVAR
        else if (Js::JavascriptNumber::Is_NoTaggedIntCheck(var))
        {
            return Js::TypeIds_Number;
        }
#endif

        RemoteRecyclableObject obj(this->GetReader(), reinterpret_cast<const RecyclableObject*>(var));
        RemoteType type(this->GetReader(), obj->GetType());
        return type->GetTypeId();
    }

    bool InspectionContext::IsStaticType(Js::TypeId typeId)
    {
        return typeId <= Js::TypeIds_LastStaticType;
    }

    bool InspectionContext::IsDynamicType(Js::TypeId typeId)
    {
        return !IsStaticType(typeId);
    }

    bool InspectionContext::IsObjectType(Js::TypeId typeId)
    {
        return typeId > Js::TypeIds_LastJavascriptPrimitiveType;
    }

    bool InspectionContext::GetProperty(const Js::Var instance, Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop)
    {
        RecyclableObject* object = static_cast<RecyclableObject*>(instance); // Will guard by GetTypeId
        Js::TypeId typeId;

        while ((typeId = GetTypeId(object)) != Js::TypeIds_Null)
        {
            if (IsDynamicType(typeId))
            {
                AutoPtr<ITypeHandler> typeHandler = CreateTypeHandler(static_cast<DynamicObject*>(object));
                if (typeHandler && typeHandler->GetProperty(propertyId, prop))
                {
                    return true;
                }
            }

            object = RemoteRecyclableObject(GetReader(), object).GetPrototype();
        }

        return false;
    }

    void InspectionContext::CreateDataDebugProperty(
        const PROPERTY_INFO& info,
        _In_opt_ IJsDebugPropertyInternal* parent,
        _Out_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        AssertMsg(info.data, "Got a nullptr Var?");
        Js::TypeId typeId = GetTypeId(info.data);

        switch (typeId)
        {
        case Js::TypeIds_Undefined:
            CreateComObject<SimpleProperty>(this, info, L"Undefined", L"undefined", parent, ppDebugProperty);
            return;

        case Js::TypeIds_Null:
            CreateComObject<SimpleProperty>(this, info, L"Null", L"null", parent, ppDebugProperty);
            return;

        case Js::TypeIds_Boolean:
            CreateComObject<JavascriptBooleanProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Symbol:
            CreateComObject<JavascriptSymbolProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Integer:
            CreateComObject<TaggedIntProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Number:
            CreateComObject<JavascriptNumberProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Int64Number:
            CreateComObject<JavascriptInt64NumberProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_UInt64Number:
            CreateComObject<JavascriptUInt64NumberProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_String:
            CreateComObject<JavascriptStringProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_VariantDate:
            CreateComObject<JavascriptVariantDateProperty>(this, info, parent, ppDebugProperty);
            return;
        case Js::TypeIds_WithScopeObject:
            {
               CreateDataDebugProperty(PROPERTY_INFO(RemoteData<WithScopeObject>(this->GetReader(), reinterpret_cast<const WithScopeObject*>(info.data))->GetWrappedObject()), parent, ppDebugProperty);
                return;
            }
        case Js::TypeIds_Function:
            if (MatchVTable(info.data, Diag_JavascriptRegExpConstructor))
            {
                CreateComObject<JavascriptRegExpConstructorProperty>(this, info, parent, ppDebugProperty);
            }
            else
            {
                CreateComObject<JavascriptFunctionProperty>(this, info, parent, ppDebugProperty);
            }
            return;

        case Js::TypeIds_Array:
            CreateComObject<JavascriptArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_NativeIntArray:
            CreateComObject<JavascriptNativeIntArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_NativeFloatArray:
            CreateComObject<JavascriptNativeFloatArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Date:
        case Js::TypeIds_WinRTDate:
            CreateComObject<JavascriptDateProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_RegEx:
            CreateComObject<JavascriptRegExpProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Error:
            CreateComObject<JavascriptErrorProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_BooleanObject:
            CreateComObject<JavascriptBooleanObjectProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_SymbolObject:
            CreateComObject<JavascriptSymbolObjectProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_NumberObject:
            CreateComObject<JavascriptNumberObjectProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_StringObject:
            CreateComObject<JavascriptStringObjectProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Arguments:
            CreateComObject<ArgumentsObjectProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_ES5Array:
            CreateComObject<ES5ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Int8Array:
            CreateComObject<Int8ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Uint8Array:
            CreateComObject<Uint8ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Uint8ClampedArray:
            CreateComObject<Uint8ClampedArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Int16Array:
            CreateComObject<Int16ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Uint16Array:
            CreateComObject<Uint16ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Int32Array:
            CreateComObject<Int32ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Uint32Array:
            CreateComObject<Uint32ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Float32Array:
            CreateComObject<Float32ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Float64Array:
            CreateComObject<Float64ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Int64Array:
            CreateComObject<Int64ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Uint64Array:
            CreateComObject<Uint64ArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_CharArray:
            CreateComObject<CharArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_BoolArray:
            CreateComObject<BoolArrayProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_ArrayBuffer:
            CreateComObject<ArrayBufferProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Map:
            CreateComObject<JavascriptMapProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Set:
            CreateComObject<JavascriptSetProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_WeakMap:
            CreateComObject<JavascriptWeakMapProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_WeakSet:
            CreateComObject<JavascriptWeakSetProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Proxy:
            CreateComObject<JavascriptProxyProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_Promise:
            CreateComObject<JavascriptPromiseProperty>(this, info, parent, ppDebugProperty);
            return;

        case Js::TypeIds_CopyOnAccessNativeIntArray:
            Assert(false);
            // fall-through

        default:
            if (typeId >= Js::TypeIds_Limit)
            {
                CreateComObject<ExternalObjectProperty>(this, info, typeId, parent, ppDebugProperty);
            }
            else if (typeId >= Js::TypeIds_Object)
            {
                CreateComObject<JavascriptObjectProperty>(this, info, parent, ppDebugProperty);
            }
            else
            {
                // Here are TypeIds that don't have inspection support. Assert if not intentionally unsupported.
                if (typeId != Js::TypeIds_HostDispatch)
                {
                    AssertMsg(false, "TypeId missing inspection?");
                }
                CreateComObject<UnknownProperty>(this, info, L"[Unrecognized Type]", L"{...}", parent, ppDebugProperty);
            }
            break;
        }
    }

    void InspectionContext::CreateDebugProperty(
        const PROPERTY_INFO& info,
        _In_opt_ IJsDebugPropertyInternal* parent,
        _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        switch (info.type)
        {
        case PROPERTY_INFO::DATA_PROPERTY:
            CreateDataDebugProperty(info, parent, ppDebugProperty);
            return;

        case PROPERTY_INFO::BOOL_VALUE:
            CreateComObject<JavascriptBooleanProperty>(this, info, info.boolValue, parent, ppDebugProperty);
            return;

        case PROPERTY_INFO::INT_VALUE:
            CreateComObject<TaggedIntProperty>(this, info, info.i4Value, parent, ppDebugProperty);
            return;

        case PROPERTY_INFO::UINT_VALUE:
            if (info.ui4Value <= INT_MAX)
            {
                CreateComObject<TaggedIntProperty>(this, info, static_cast<int>(info.ui4Value), parent, ppDebugProperty);
            }
            else
            {
                CreateComObject<NumberProperty>(this, info, info.ui4Value, parent, ppDebugProperty);
            }
            return;

        case PROPERTY_INFO::DOUBLE_VALUE:
            CreateComObject<NumberProperty>(this, info, info.dblValue, parent, ppDebugProperty);
            return;

        case PROPERTY_INFO::STRING_VALUE:
            CreateComObject<JavascriptStringProperty>(this, info, parent, ppDebugProperty);
            return;

        case PROPERTY_INFO::POINTER_VALUE:
            CreateComObject<JavascriptPointerProperty>(this, info, parent, ppDebugProperty);
            return;

        case PROPERTY_INFO::DEBUG_PROPERTY:
            *ppDebugProperty = info.debugProperty;
            (*ppDebugProperty)->AddRef();
            (*ppDebugProperty)->SetParent(parent); //fix parent node
            return;

        default:
            Assert(info.type == PROPERTY_INFO::ACCESSOR_PROPERTY);
            CreateComObject<UnknownProperty>(this, info, L"", GetDebugClient()->GetErrorString(DIAGERR_FunctionCallNotSupported), parent, ppDebugProperty);
        }
    }

    void InspectionContext::CreateDebugProperty(
        const PROPERTY_INFO& info,
        _In_opt_ IJsDebugPropertyInternal* parent,
        _Outptr_ IJsDebugProperty** ppDebugProperty)
    {
        return CreateDebugProperty(info, parent, reinterpret_cast<IJsDebugPropertyInternal**>(ppDebugProperty));
    }

    bool InspectionContext::IsMshtmlObject(Js::TypeId typeId)
    {
        // OS 2113442 - EdgeHtmlDac.dll is not present currently. Stop inspecting html objects for now and return false from here.
        // We have removed the call to EnsureReadyIfHybridDebugging from JavascriptLibrary::CreateExternalConstructor because
        // calling EnsureReadyIfHybridDebugging will call EnsureObjectReady which try to create the object and go back to trident.
        // Since the call CreateExternalConstructor would have not returned to trident there is no constructor on trident side and it can't create the object
        // When we fix the EdgeHtmlDac.dll load issue we need to check if HTML objects are inspected properly and if not we need to fix them by deferring the HTML objects somehow
#if DBG
        if (typeId >= Js::TypeIds_Limit)
        {
            if (!m_mshtmlDac)
            {
                try
                {
                    m_mshtmlDacLib.Load(L"EdgeHtmlDac.dll");
                    AssertMsg(false, "EdgeHtmlDac.dll found. Time to fix html inspection scenarios in hybrid debugging");
                }
                catch(const DiagException&)
                {
                    return false; //TODO: the dll may not exist
                }

                // Needs to be enabled when EdgeHtmlDac.dll is available
                //CComPtr<IMshtmlDac> dac;
                //CheckHR(PrivateCoCreate(m_mshtmlDacLib.Handle, /*CLSID_MshtmlDac*/__uuidof(MshtmlDac), IID_PPV_ARGS(&dac)));
                //CheckHR(dac->SetDebugDataTarget(m_debugProcess->GetDebugDataTarget()));

                //::JavascriptTypeId minTypeId, maxTypeId;
                //CheckHR(dac->GetJavascriptTypeIdRange(&minTypeId, &maxTypeId));

                //m_minMshtmlTypeId = static_cast<Js::TypeId>(minTypeId);
                //m_maxMshtmlTypeId = static_cast<Js::TypeId>(maxTypeId);
                //m_mshtmlDac = dac;
            }

            //return typeId >= m_minMshtmlTypeId && typeId <= m_maxMshtmlTypeId;
        }
#endif
        return false;
    }

    __declspec(noreturn) void InspectionContext::ThrowEvaluateNotSupported()
    {
        PCWSTR error = GetDebugClient()->GetErrorString(DIAGERR_EvaluateNotSupported);
        EvaluateException::Throw(error);
    }

    const PropertyRecord* InspectionContext::GetInternalPropertyRecord(Js::PropertyId propertyId)
    {
        switch (propertyId)
        {
#define INTERNALPROPERTY(name) \
            case InternalPropertyIds::name: \
                return GetDebugClient()->GetGlobalPointer<const PropertyRecord>(Globals::Globals_InternalProperty_##name);
#include "InternalPropertyList.h"
        }

        DiagException::Throw(E_UNEXPECTED, DiagErrorCode::UNKNOWN_INTERNALPROPERTYRECORD);
    }

    HRESULT PrivateCoCreate(HMODULE hModule, REFCLSID clsid, REFIID riid, LPVOID* ppUnk)
    {
        typedef HRESULT (STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);

        FN_DllGetClassObject proc = (FN_DllGetClassObject)::GetProcAddress(hModule, "DllGetClassObject");
        if (proc == nullptr)
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }

        CComPtr <IClassFactory> classFactory;
        HRESULT hr = proc(clsid, IID_PPV_ARGS(&classFactory));
        if (SUCCEEDED(hr))
        {
            hr = classFactory->CreateInstance(nullptr, riid, ppUnk);
        }

        return hr;
    }

} // namespace JsDiag.

// shared static data
#include "DateImplementationData.h"

// stubs
namespace Js
{
    bool DaylightTimeHelper::ForceOldDateAPIFlag()
    {
        return false;
    }

#if DBG
    bool Throw::ReportAssert(LPSTR fileName, uint lineNumber, LPSTR error, LPSTR message)
    {
        AssertMsg(false, "Runtime assertion");
        return false;
    }

    void Throw::LogAssert()
    {
        AssertMsg(false, "Runtime assertion");
    }
#endif

    void Throw::FatalInternalError()
    {
        AssertMsg(false, "Runtime fatal error");
    }

#if defined(PROFILE_RECYCLER_ALLOC) && defined(RECYCLER_DUMP_OBJECT_GRAPH)
    bool RecyclableObject::DumpObjectFunction(type_info const * typeinfo, bool isArray, void * objectAddress) { return true; }
    bool Type::DumpObjectFunction(type_info const * typeinfo, bool isArray, void * objectAddress) { return true; }
#endif
}

#if defined(PROFILE_RECYCLER_ALLOC) && defined(RECYCLER_DUMP_OBJECT_GRAPH)
void RecyclerObjectDumper::RegisterDumper(type_info const * typeinfo, DumpFunction dumperFunction) {}
#endif
