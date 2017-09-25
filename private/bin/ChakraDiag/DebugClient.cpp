//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "PathCch.h"

// Reuse this GUID to identify build
#include "ByteCodeCacheReleaseFileVersion.h"

extern HANDLE g_hInstance;

namespace JsDiag
{
    const char16* DebugClient::c_chakraModuleName = CHAKRA_MODULE_NAME;
    const char16* DebugClient::c_chakraFileName = CHAKRA_MODULE_NAME _u(".dll");
    const char16* DebugClient::c_chakraTestModuleName = CHAKRA_MODULE_NAME _u("test");
    const char16* DebugClient::c_chakraTestFileName = CHAKRA_MODULE_NAME _u("test.dll");

    const LibraryName DebugClient::c_librariesToTry[] = {
        { DebugClient::c_chakraModuleName, DebugClient::c_chakraFileName },
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        // ENABLE_DEBUG_CONFIG_OPTIONS is to make sure we don't ship with check for ChakraTest.dll.
        { DebugClient::c_chakraTestModuleName, DebugClient::c_chakraTestFileName }
#endif
    };

    DebugClient::DebugClient(DiagProvider* diagProvider) : 
        m_diagProvider(diagProvider),
        m_chakraBaseAddr(0),
        m_chakraHighAddr(0),
        m_globals(),
        m_hasTargetHooks(false)
    {
        Assert(diagProvider);
    }

    DebugClient::~DebugClient()
    {
    }

    void DebugClient::EnsureTargetHooks()
    {
        if(!m_hasTargetHooks)
        {
            this->InitializeTargetHooks();
        }
    }

    void DebugClient::InitializeTargetHooks()
    {
        this->LoadTargetAndExecute([&](const CStringA& diagInfo, ULONG64 moduleBaseAddr64) {
            this->GetGlobals(diagInfo, moduleBaseAddr64);
            m_hasTargetHooks = true;
        });
    }

    template <typename T>
    void DebugClient::LoadTargetAndExecute(T operation)
    {
        // Load the library (try Chakra.dll, then ChakraTest.dll).
        int tryCount = sizeof(c_librariesToTry) / sizeof(LibraryName);
        for (int i = 0; i < tryCount; ++i)
        {
            LibraryName libraryName = c_librariesToTry[i];

            // Get the module base from target process.
            Assert(m_chakraBaseAddr == 0);
            bool isSuccess = m_diagProvider->TryGetTargetModuleBaseAddr(libraryName.ModuleName, &m_chakraBaseAddr);
            if (!isSuccess)
            {
                continue;   // The specified module should not be loaded in target process, try next module.
            }
            void* moduleBaseAddr = reinterpret_cast<void*>(m_chakraBaseAddr);
            Assert((ULONG64)moduleBaseAddr == m_chakraBaseAddr);

            AutoHandle hChildStdOut_Read, hChildStdOut_Write;
            {
                SECURITY_ATTRIBUTES saAttr;
                saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
                saAttr.bInheritHandle = TRUE;
                saAttr.lpSecurityDescriptor = NULL;
                if (!CreatePipe(&hChildStdOut_Read, &hChildStdOut_Write, &saAttr, 0))
                {
                    DiagException::Throw(E_FAIL, DiagErrorCode::DIAG_CREATEPIPE);
                }
                if (!SetHandleInformation(hChildStdOut_Read, HANDLE_FLAG_INHERIT, 0))
                {
                    DiagException::Throw(E_FAIL, DiagErrorCode::DIAG_CREATEPIPE);
                }
            }

            char16 sysDir[MAX_PATH];
            {
                UINT len = GetSystemDirectory(sysDir, _countof(sysDir));
                if (len == 0 || len >= _countof(sysDir))
                {
                    DiagException::Throw(E_UNEXPECTED);
                }
            }

            const char16* dllDir = sysDir;
            const char16* dllName = libraryName.FileName;
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            // Try local Chakra.dll if exists, to support jdtest.exe
            char16 localPath[MAX_PATH];
            DWORD localPathLen = GetModuleFileName(NULL, localPath, _countof(localPath));
            if (localPathLen > 0
                && SUCCEEDED(PathCchRemoveFileSpec(localPath, localPathLen))
                && SUCCEEDED(PathCchAppend(localPath, _countof(localPath), dllName))
                && PathFileExists(localPath)
                && SUCCEEDED(PathCchRemoveFileSpec(localPath, localPathLen)))
            {
                dllDir = localPath;
            }
#endif
            char16 szCmdline[MAX_PATH];
            if (swprintf_s(szCmdline,
                _u("\"%s\\rundll32.exe\" \"%s\\%s\",DumpDiagInfo"), sysDir, dllDir, dllName) < 0)
            {
                DiagException::Throw(E_UNEXPECTED);
            }
            PROCESS_INFORMATION piProcInfo = {};
            STARTUPINFO siStartInfo = {};
            siStartInfo.cb = sizeof(STARTUPINFO);
            siStartInfo.hStdError = hChildStdOut_Write;
            siStartInfo.hStdOutput = hChildStdOut_Write;
            siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

            if (!CreateProcess(NULL,
                szCmdline,     // command line
                NULL,          // process security attributes
                NULL,          // primary thread security attributes
                TRUE,          // handles are inherited
                0,             // creation flags
                NULL,          // use parent's environment
                NULL,          // use parent's current directory
                &siStartInfo,  // STARTUPINFO pointer
                &piProcInfo))  // receives PROCESS_INFORMATION
            {
                DiagException::Throw(E_FAIL, DiagErrorCode::DIAG_CREATEPROCESS);
            }

            AutoHandle hThread(piProcInfo.hThread), hProcess(piProcInfo.hProcess);
            hChildStdOut_Write.Close();

            CStringA s;
            {
                char buf[256];
                for (;;)
                {
                    DWORD dwRead;
                    if (!ReadFile(hChildStdOut_Read, buf, _countof(buf), &dwRead, NULL) || dwRead == 0)
                    {
                        break;
                    }
                    s.Append(buf, dwRead);
                }
            }

            operation(s, m_chakraBaseAddr);
            return;
        }

        // Could not load any of supported libraries.
        DiagException::Throw(E_INVALIDARG, DiagErrorCode::CHAKRA_MODULE_NOTFOUND);
    }

    void DebugClient::GetGlobals(const CStringA& diagInfo, ULONG64 moduleBaseAddr)
    {
        static const char* LINE = "\r\n";
        static const char* SPACE = " \t";
        static const char* NAMES[] =
        {
#define ENTRY(field, name) #name,
#include "DiagGlobalList.h"
#undef ENTRY
        };

        int i = 0, matchCount = 0;
        while (i >= 0)  // -1 when Tokenize reaches end of string
        {
            CStringA line = diagInfo.Tokenize(LINE, i);
            if (line.GetLength() == 0) {
                continue;
            }

            int j = 0;
            CStringA name = line.Tokenize(SPACE, j);
            CStringA value = line.Tokenize(SPACE, j);

            if (name == "Build")
            {
                REFIID buildGuid = byteCodeCacheReleaseFileVersion;
                CA2WEX<> wValue(value);
                IID guid;
                if (FAILED(IIDFromString(wValue, &guid))
                    || !IsEqualGUID(guid, buildGuid))
                {
                    DiagException::Throw(E_FAIL, DiagErrorCode::DIAG_INFO_BUILD_MISMATCH);
                }
                continue;
            }

            bool match = false;
            for (int k = 0; k < _countof(NAMES) && !match; k++)
            {
                if (name == NAMES[k])
                {
                    size_t offset;
                    if (sscanf_s(value, "%Ix", &offset) != 1)
                    {
                        DiagException::Throw(E_FAIL, DiagErrorCode::DIAG_INFO_BAD_VALUE);
                    }
                    m_globals[k] = reinterpret_cast<LPVOID>(moduleBaseAddr + offset);
                    match = true;
                }
            }

            if (match)
            {
                matchCount++;
            }
            else
            {
                DiagException::Throw(E_FAIL, DiagErrorCode::DIAG_INFO_BAD_NAME);
            }
        }

        if (matchCount != Globals_Count)
        {
            DiagException::Throw(E_FAIL, DiagErrorCode::DIAG_INFO_COUNT_MISMATCH);
        }
    }

    bool DebugClient::IsJsModuleAddress(void* address)
    {
        if(!m_chakraHighAddr)
        {
            EnsureTargetHooks();
            m_chakraHighAddr = (UINT64)(this->GetGlobalValue<UINT_PTR>(Globals_DllHighAddress));
            Assert(this->GetGlobalValue<UINT_PTR>(Globals_DllBaseAddress) == m_chakraBaseAddr);
            Assert(m_chakraHighAddr != NULL);
        }
        if(address >= (void*)m_chakraBaseAddr && address <= (void*)m_chakraHighAddr)
        {
            return true;
        }
        return false;
    }

     RemoteStackFrameEnumerator* DebugClient::CreateStackFrameEnumerator(ULONG threadId, void* advanceToAddr)
     {
         return m_diagProvider->CreateStackFrameEnumerator(threadId, advanceToAddr);
     }

     IVirtualReader* DebugClient::GetReader() const
     { 
         return m_diagProvider->GetReader(); 
     }

} // namespace JsDiag.
