//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

//
// Exports
//

//
// Debugger calls this when a JavaScriptDataStream type of user stream is found in a dump.
// This exported function reads and parses the given JS dump stream, prepares to answer JS
// stack info queries.
//
HRESULT WINAPI BeginThreadStackReconstruction(
    _In_ ULONG streamType,
    _In_ PVOID miniDumpStreamBuffer,
    _In_ ULONG bufferSize);

//
// Debugger calls this to query JS stack info of a given thread. Debugger would use the returned
// JS stack info to reconstruct full mixed stack.
//
HRESULT WINAPI ReconstructStack(
    _In_ ULONG systemThreadId,
    _In_ PDEBUG_STACK_FRAME_EX pNativeFrames,
    _In_ ULONG cNativeFrames,
    _Out_ PSTACK_SYM_FRAME_INFO *ppStackSymFrames,
    _Out_ ULONG *pStackSymFramesFilled);

//
// Releases JS stack info created by above.
//
HRESULT WINAPI FreeStackSymFrames(
    _In_ PSTACK_SYM_FRAME_INFO pStackSymFrames);

//
// Ends stack reconstruction session. Release all resources.
//
HRESULT WINAPI EndThreadStackReconstruction();

//
// Private test api for unit tests. Queries threadID at index starting from 0. Returns S_FALSE if no more threads.
//
HRESULT WINAPI PrivateGetStackThreadId(
    _In_ ULONG index,
    _Out_ ULONG *pSystemThreadId);

namespace JsDiag
{
    using namespace Serialization;
    typedef ExtensibleBinarySerializer Serializer;

    //
    // The main entry object to read JS dump stream and answer JS stack info queries.
    //
    // At BeginThreadStackReconstruction, a global instance of this class is created and initialized with JS dump stream.
    // This object reads JS stack info from the provided stream.
    //
    // At ReconstructStack, the object created above is used to lookup JS stack info for a given thread.
    //
    // At EndThreadStackReconstruction, the object is released.
    //
    class ATL_NO_VTABLE ScriptDumpReader:
        public CComObjectRoot,
        public IUnknown
    {
    private:
        AutoPtr<WerMessage> m_message;
        CAtlMap<ULONG, const WerStack*> m_stacks;

    public:
        DECLARE_NOT_AGGREGATABLE(ScriptDumpReader)
        BEGIN_COM_MAP(ScriptDumpReader)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        void Init(_In_ PVOID miniDumpStreamBuffer, _In_ ULONG bufferSize);
        void GetStack(_In_ ULONG systemThreadId, _Out_ PSTACK_SYM_FRAME_INFO *ppStackSymFrames, _Out_ ULONG *pStackSymFramesFilled) const;
        _Success_(return) bool GetStackThreadId(_In_ ULONG index, _Out_ ULONG *pSystemThreadId) const;

    private:
        static void ToStackInfo(const WerStack* werStack, _Out_ PSTACK_SYM_FRAME_INFO *ppStackSymFrames, _Out_ ULONG *pStackSymFramesFilled);
    };

    //
    // Read from a buffer as a stream. Used with deserialization.
    //
    class MemoryReadStream : public ISerializationStream
    {
        LPBYTE m_buffer;
        size_t m_length;
        size_t m_pos;

    public:
        MemoryReadStream(LPVOID buffer, ULONG bufferSize):
            m_buffer(reinterpret_cast<LPBYTE>(buffer)),
            m_length(bufferSize),
            m_pos(0)
        {}

        // Read from the stream into buf.
        virtual void Read(_Out_writes_bytes_all_(byteCount) void* buf, size_t byteCount) override
        {
            CheckForOverrun(byteCount);
            memcpy(buf, m_buffer + m_pos, byteCount);
            m_pos += byteCount;
        }

        virtual bool IsEOF() override
        {
            return m_pos >= m_length;
        }

        // Write content of the buf to the stream.
        virtual void Write(const void* /* buf */, size_t /* byteCount */) override
        {
            Assert(false); // should never be called
            DiagException::Throw(E_UNEXPECTED);
        }

    private:
        void CheckForOverrun(size_t byteCount)
        {
            if (m_pos + byteCount > m_length)
            {
                DiagException::Throw(E_INVALIDARG);
            }
        }
    }; // MemoryStream.
}
