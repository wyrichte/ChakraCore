//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

//
// WER exports
//

//
// MiniDumpWriteDump calls this to collect JS stack info when target process has Chakra loaded.
// This exported function collects JS stack info and serilizes into the user dump stream.
//
HRESULT WINAPI GetDumpStreams(
    _In_ IStackProviderDataTarget* dataTarget,
    _In_ MINIDUMP_TYPE DumpType,
    _COM_Outptr_ PMINIDUMP_USER_STREAM_INFORMATION *ppUserStream);

//
// MiniDumpWriteDump calls this to free the user dump stream object created by above.
//
HRESULT WINAPI FreeDumpStreams(
    _In_ PMINIDUMP_USER_STREAM_INFORMATION pUserStream);

namespace JsDiag
{
    //
    // Shallow wrapper of MINIDUMP_USER_STREAM
    //
    class ScriptDumpStream: public MINIDUMP_USER_STREAM
    {
    public:
        ScriptDumpStream(LPVOID buffer, ULONG bufferSize);
        ~ScriptDumpStream();

        static bool Is(_In_ PMINIDUMP_USER_STREAM pUserStream);
        static ScriptDumpStream* From(_In_ PMINIDUMP_USER_STREAM pUserStream);
    };

    //
    // Shallow wrapper of MINIDUMP_USER_STREAM_INFORMATION
    //
    class ScriptDumpInfo: public MINIDUMP_USER_STREAM_INFORMATION
    {
    private:
        ScriptDumpStream m_scriptDumpStream;

    public:
        ScriptDumpInfo(LPVOID buffer, ULONG bufferSize);

        static bool Is(_In_ PMINIDUMP_USER_STREAM_INFORMATION pUserStreamInfo);
        static ScriptDumpInfo* From(_In_ PMINIDUMP_USER_STREAM_INFORMATION pUserStreamInfo);
    };

    //
    // An in-memory stream used with serialization.
    //
    class MemoryWriteStream: public ISerializationStream
    {
    private:
        CComPtr<IStream> m_stream;

    public:
        MemoryWriteStream();
        void GetContent(PVOID* ppBuf, ULONG* pSize);

        virtual void Write(const void* buf, size_t byteCount) override;
        virtual void Read(void* buf, size_t byteCount) override;
        virtual bool IsEOF() override;
    };

    class ScriptState
    {
    private:
        static void SaveState(IStackProviderDataTarget* dataTarget, AutoList<WerStack>& werStacks, MINIDUMP_TYPE dumpType);
        static void GetStream(WerMessage* message, PVOID* ppBuf, ULONG* pSize);

    public:
        static void SaveState(IStackProviderDataTarget* dataTarget, PMINIDUMP_USER_STREAM_INFORMATION *ppUserStream, MINIDUMP_TYPE dumpType);
    };
}
