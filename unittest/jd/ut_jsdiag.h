//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class EXT_CLASS:
    public ExtExtension,
    public DummyTestGroup
{
private:
    bool m_unitTestMode; // Default true (Out $ut$ for unittest output filter). Toggled by !utmode.

public:
    EXT_CLASS();
    EXT_COMMAND_METHOD(echo);
    EXT_COMMAND_METHOD(utmode);
    EXT_COMMAND_METHOD(ldsym);

    // WER js stack dump
    EXT_COMMAND_METHOD(writedump);
    EXT_COMMAND_METHOD(readdump);
    EXT_COMMAND_METHOD(testdump);

    void Out(_In_ PCSTR fmt, ...);
    void Out(_In_ PCWSTR fmt, ...);

private:
    static HRESULT PrivateCoCreate(LPCWSTR strModule, REFCLSID rclsid, REFIID iid, LPVOID* ppunk);
    void IfFailThrow(HRESULT hr, PCSTR msg = NULL);
    bool SetupSymbolPath();

    template <class Func>
    void TestGetDump(Func func);
    void TestReadDump(LPVOID buffer, ULONG bufferSize, _In_opt_ const DbgEngDataTarget::MemoryRegionsType* memoryRegions, const ULONG maxFrames = ULONG_MAX);
};
