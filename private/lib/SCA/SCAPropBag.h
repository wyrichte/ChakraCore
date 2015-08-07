//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
    //
    // Implements ISCAPropBag.
    //
    class SCAPropBag sealed :
        public ScriptContextHolder,
        public ISCAPropBag
    {
        typedef JsUtil::BaseDictionary<InternalString, Var, RecyclerNonLeafAllocator, PowerOf2SizePolicy,
            DefaultComparer, JsUtil::DictionaryEntry> PropertyDictionary;

    private:
        RecyclerRootPtr<PropertyDictionary> m_properties;
        ULONG m_refCount;

        SCAPropBag(ScriptContext* scriptContext);
        HRESULT InternalAdd(LPCWSTR name, charcount_t len, Var value);
        HRESULT InternalAddNoCopy(LPCWSTR name, charcount_t len, Var value);

    public:
        ~SCAPropBag();
        static void CreateInstance(ScriptContext* scriptContext, SCAPropBag** ppInstance);

        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

        STDMETHODIMP Add(LPCWSTR name, Var value);
        STDMETHODIMP Get(LPCWSTR name, Var* pValue);

        //
        // PropBag property enumerator for WriteObjectProperties.
        //
        class PropBagEnumerator
        {
        private:
            PropertyDictionary* m_properties;
            int m_curIndex;

        public:
            PropBagEnumerator(SCAPropBag* propBag)
                : m_properties(propBag->m_properties), m_curIndex(-1)
            {
            }

            bool MoveNext();

            const wchar_t* GetNameString() const
            {
                return m_properties->GetKeyAt(m_curIndex).GetBuffer();
            }

            charcount_t GetNameLength() const
            {
                return m_properties->GetKeyAt(m_curIndex).GetLength();
            }

            Var GetValue() const
            {
                return m_properties->GetValueAt(m_curIndex);
            }
        };

        //
        // PropBag property sink for ReadObjectProperties.
        //
        class PropBagSink
        {
        private:
            SCAPropBag* m_propbag;

        public:
            PropBagSink(SCAPropBag* propbag)
                : m_propbag(propbag)
            {
            }

            void SetProperty(const wchar_t* name, charcount_t len, Var value)
            {
                HRESULT hr = m_propbag->InternalAddNoCopy(name, len, value);
                m_propbag->ThrowIfFailed(hr);
            }
        };
    };
}