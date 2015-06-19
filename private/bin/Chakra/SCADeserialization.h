//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
    //
    // DeserializationCloner helps clone a Var from a stream location.
    //
    template <class Reader>
    class DeserializationCloner:
        public ClonerBase<scaposition_t, Var, SCATypeId, DeserializationCloner<Reader> >
    {
    private:
        CComPtr<ISCAHost> m_pSCAHost;
        CComPtr<ISCAContext> m_pSCAContext;
        Reader* m_reader;

    private:
        SCATypeId ReadTypeId() const
        {
            uint32 typeId;
            m_reader->Read(&typeId);
            return static_cast<SCATypeId>(typeId);
        }

        void Read(uint32* value) const
        {
            m_reader->Read(value);
        }

        bool TryReadString(const wchar_t** str, charcount_t* len) const;
        void ReadString(const wchar_t** str, charcount_t* len) const;
        void Read(BYTE* buf, uint32 len) const;

        void ReadCanvasPixelArray(Dst* dst, JavascriptLibrary* javascriptLibrary) const;

        //
        // Read a TypedArray or DataView.
        //
        template <class T, bool clamped>
        void ReadTypedArray(Dst* dst) const
        {
            typedef TypedArrayTrace<T,clamped> trace_type;

            Dst arrayBuffer;
            GetEngine()->Clone(m_reader->GetPosition(), &arrayBuffer);
            if (!arrayBuffer || !ArrayBuffer::Is(arrayBuffer))
            {
                ThrowSCADataCorrupt();
            }

            uint32 byteOffset, length;
            Read(&byteOffset);
            Read(&length);
            *dst = trace_type::CreateTypedArray(
                ArrayBuffer::FromVar(arrayBuffer), byteOffset, length, GetScriptContext());
        }

        void ReadTypedArray(SrcTypeId typeId, Dst* dst) const;

        //
        // Read a SCAProperties section: {SCAPropertyName SCAValue} SCAPropertiesTerminator
        //
        template <class PropertySink>
        void ReadObjectProperties(PropertySink* sink)
        {
            for(;;)
            {
                const wchar_t* buf;
                charcount_t len;
                if (!TryReadString(&buf, &len))
                {
                    break;
                }

                Var value;
                GetEngine()->Clone(m_reader->GetPosition(), &value);
                if (!value)
                {
                    ThrowSCADataCorrupt();
                }

                sink->SetProperty(buf, len, value);
            }
        }

    public:
        DeserializationCloner(ScriptContext* scriptContext, ISCAHost* pSCAHost, ISCAContext* pSCAContext, Reader* reader)
            : ClonerBase(scriptContext), m_pSCAHost(pSCAHost), m_pSCAContext(pSCAContext), m_reader(reader)
        {
        }

        static bool ShouldLookupReference()
        {
            // Never lookup reference when cloning from stream location. DeserializationCloner
            // handles object reference lookup explictly when seeing a reference SCATypeId.
            return false;
        }

        SrcTypeId GetTypeId(Src src) const
        {
            Assert(m_reader->GetPosition() == src); // Only allow the current position
            return ReadTypeId();
        }

        void ThrowSCAUnsupported() const
        {
            // Unexpected SCATypeId indicates data corruption.
            ThrowSCADataCorrupt();
        }

        bool TryClonePrimitive(SrcTypeId typeId, Src src, Dst* dst);
        bool TryCloneObject(SrcTypeId typeId, Src src, Dst* dst, SCADeepCloneType* deepClone);
        void CloneProperties(SrcTypeId typeId, Src src, Dst dst);
        void CloneMap(Src src, Dst dst);
        void CloneSet(Src src, Dst dst);
        void CloneObjectReference(Src src, Dst dst);
    };

    //
    // Object property sink for ReadObjectProperties.
    //
    class ObjectPropertySink: public ScriptContextHolder
    {
    private:
        RecyclableObject* m_obj;

    public:
        ObjectPropertySink(ScriptContext* scriptContext, RecyclableObject* obj)
            : ScriptContextHolder(scriptContext), m_obj(obj)
        {
        }

        void SetProperty(const wchar_t* name, charcount_t len, Var value);
    };

    class SCADeserializationEngine
    {
        typedef DeserializationCloner<StreamReader> StreamDeserializationCloner;

    public:
        static Var Deserialize(ISCAHost* pSCAHost, ISCAContext* pSCAContext, StreamReader* reader, TransferablesHolder* transferrableHolder);
    };
}
