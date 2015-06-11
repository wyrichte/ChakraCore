//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------
#pragma once

class Module : public ATL::CAtlDllModuleT<Module>
{
public:
    static HRESULT GetFileVersion(DWORD* majorVersion, DWORD* minorVersion);
    static HINSTANCE g_ModuleInstance;
    static HRESULT GetFileVersion(HANDLE process, HINSTANCE module, DWORD* majorVersion, DWORD* minorVersion);

#ifdef DEBUG
    virtual LONG Lock() throw() // Break point stub for leak investigation
    {
        return __super::Lock();
    }

    virtual LONG Unlock() throw()  // Break point stub for leak investigation
    {
        return __super::Unlock();
    }

    ~Module()
    {
        AssertMsg(GetLockCount() == 0, "Com object leaked?");
    }
#endif

private:
    static HRESULT GetVersionInfo(__in LPCWSTR pszPath, DWORD* majorVersion, DWORD* minorVersion);
    static DWORD majorVersion;
    static DWORD minorVersion;
    static const DWORD INVALID_VERSION = (DWORD)-1;
};
