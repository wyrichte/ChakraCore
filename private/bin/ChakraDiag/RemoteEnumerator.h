//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    //
    // Holds info of original object when enumerating on prototype chain.
    //
    struct OriginalObjectInfo
    {
        const RecyclableObject* instance;
        ::JavascriptTypeId typeId;
        void* extensionObject;  // used only for DOM objects

        OriginalObjectInfo(InspectionContext* context, const RecyclableObject* instance, Js::TypeId typeId);
    };

    //
    // Auto releases MSHTMLDAC_PROPERTY resources in destructor.
    //
    struct AutoMSHTMLDAC_PROPERTY: public MSHTMLDAC_PROPERTY
    {
    public:
        ~AutoMSHTMLDAC_PROPERTY()
        {
            ::SysFreeString(bstrName);
            if (dwType == MSHTMLDAC_PROPERTY_TYPE::MSHTMLDAC_PROPERTY_TYPE_BSTR)
            {
                ::SysFreeString(bstrValue);
            }
        }
    };

    //
    // Enumerates DOM managed properties through MshtmlDAC.
    //
    class MshtmlObjectEnumerator
    {
    public:
        template <class Fn>
        static void Enumerate(InspectionContext* context,
            const ExternalObject* object, Js::TypeId typeId, const OriginalObjectInfo& originalObject, Fn fn);
    };

    template <class Fn>
    void MshtmlObjectEnumerator::Enumerate(InspectionContext* context,
        const ExternalObject* object, Js::TypeId typeId, const OriginalObjectInfo& originalObject, Fn fn)
    {
        OriginalObjectInfo current(context, object, typeId);

        CComPtr<IMshtmlDebugProperty> domDebugProperty;
        CheckHR(context->GetMshtmlDac()->GetDebugProperty(
            current.extensionObject, current.typeId,
            originalObject.extensionObject, originalObject.typeId,
            &domDebugProperty));

        if (domDebugProperty)
        {
            CComPtr<IMshtmlDacPropertyIter> domPropertyIter;
            CheckHR(domDebugProperty->GetMembers(&domPropertyIter), DiagErrorCode::DOM_GETMEMBERS);

            for(;;)
            {
                AutoMSHTMLDAC_PROPERTY prop;

                HRESULT hr = domPropertyIter->Next(&prop);
                if (hr == S_FALSE)
                {
                    break;
                }
                CheckHR(hr, DiagErrorCode::DOM_NEXT);

                if (prop.dwType != MSHTMLDAC_PROPERTY_TYPE::MSHTMLDAC_PROPERTY_TYPE_UNDEFINED)
                {
                    if (!fn(PROPERTY_INFO(prop)))
                    {
                        break;
                    }
                }
            }
        }
    }

    //
    // Simulates runtime ForInObjectEnumerator to enumerate all properties of an object.
    //
    class RemoteForInObjectEnumerator
    {
    private:
        InspectionContext* m_context;
        const DynamicObject* m_object;
        CAtlMap<CString, int> m_props;

    public:
        RemoteForInObjectEnumerator(InspectionContext* context, const DynamicObject* object):
            m_context(context),
            m_object(object)
        {
        }

        template <class Fn>
        void Enumerate(Fn fn, bool requireEnumerable = true);
    };

    template <class Fn>
    void RemoteForInObjectEnumerator::Enumerate(Fn fn, bool requireEnumerable/*= true*/)
    {
        m_props.RemoveAll();

        auto listener = MakePropertyListener([=](const PROPERTY_INFO& prop, const Js::PropertyId propertyId) -> bool
        {
            if (!m_props.Lookup(prop.name))
            {
                // Record already enumerated properties so we won't enumerate them again if they are also on prototypes.
                m_props[prop.name] = 0;

                return fn(prop);
            }

            return true;
        });

        RecyclableObject* object = const_cast<DynamicObject*>(m_object);
        Js::TypeId typeId = m_context->GetTypeId(object);
        OriginalObjectInfo originalObject(m_context, m_object, typeId);

        while (typeId != Js::TypeIds_Null)
        {
            CComPtr<IJsDebugPropertyInternal> prop;
            m_context->CreateDebugProperty(PROPERTY_INFO(object), /*parent*/nullptr, &prop);

            // Enumerate properties on this node
            prop->EnumProperties(&listener, originalObject, requireEnumerable);

            object = RemoteRecyclableObject(m_context->GetReader(), object).GetPrototype();
            typeId = m_context->GetTypeId(object);
        }
    }

} // namespace JsDiag.
