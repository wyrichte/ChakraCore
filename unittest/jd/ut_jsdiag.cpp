//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "activaut.h"
#include <initguid.h>
#include "guids.h"

// EXT_DECLARE_GLOBALS must be used to instantiate
// the framework's assumed globals.
EXT_DECLARE_GLOBALS();

EXT_CLASS::EXT_CLASS():
    m_unitTestMode(true)
{
}

void EXT_CLASS::Out(_In_ PCSTR fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    std::string s;
    if (m_unitTestMode)
    {
        s = std::string("$ut$") + fmt;
        fmt = s.c_str();
    }

    m_Control->OutputVaList(DEBUG_OUTPUT_NORMAL, fmt, args);

    va_end(args);
}

void EXT_CLASS::Out(_In_ PCWSTR fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    std::wstring s;
    if (m_unitTestMode)
    {
        s = std::wstring(_u("$ut$")) + fmt;
        fmt = s.c_str();
    }

    m_Control4->OutputVaListWide(DEBUG_OUTPUT_NORMAL, fmt, args);

    va_end(args);
}

//
// We need this command to output from unit tests. To support baseline diff, "jdtest.exe -q" mode
// prohibits any debugger output. Unit tests need to directly output to stdout.
//
EXT_COMMAND(echo, "Echo to output", "{{custom}}")
{
    Out("%s\n", GetRawArgStr());
}

EXT_COMMAND(utmode,
            "Turn on/off unit test mode",
            "{;e,o,d=1;on;Use 1 to turn on, 0 to turn off}")
{
    m_unitTestMode = GetUnnamedArgU64(0) != 0;
}

EXT_COMMAND(ldsym,
    "Test load JavaScript symbols",
    "")
{
    bool sympathChanged = SetupSymbolPath();

    // Find jscript9 module.
    //      NOTE: If we JIT, finding symbol "!Js::ScriptContext" fails. Probably some debugger cache got invalidated.
    //      It will succeed after ".reload". To workaround, check both known jscript9test/jscript9 modules.
    ULONG index = DEBUG_ANY_ID;
    ULONG64 base;
    IfFailThrow(FindJScriptModuleByName</*IsPublic*/false>(m_Symbols, &index, &base),
        "Failed to find chakra module in the process");

    _TCHAR moduleName[MAX_PATH], imageName[MAX_PATH];
    IfFailThrow(m_Symbols3->GetModuleNameStringT(DEBUG_MODNAME_MODULE, index, base, moduleName, _countof(moduleName), NULL),
        "Failed to find chakra module name");
    IfFailThrow(m_Symbols3->GetModuleNameStringT(DEBUG_MODNAME_IMAGE, index, base, imageName, _countof(imageName), NULL),
        "Failed to find chakra module path");
    Verb(_u("Use %s\n"), imageName);
    HRESULT hr = S_OK;
    // .reload jscript9.dll, if sympath has changed. Without this FindSymbol will still fail if there was a previous failure (e.g., k) with missing symbols.
    if (sympathChanged)
    {
        CString module = CString(moduleName) + ".dll";
        IfFailedAssertReturn(m_Symbols3->ReloadT(module));
    }

    const CLSID CLSID_JScript9DAC = { 0x197060cb, 0x5efb, 0x4a53, 0xb0, 0x42, 0x93, 0x9d, 0xbb, 0x31, 0x62, 0x7c };
    CComPtr<IScriptDAC> pDAC;
    CComPtr<SinkDebugSite> pSinkDebugSite;

    IfFailedAssertReturn(PrivateCoCreate(imageName, CLSID_JScript9DAC, IID_PPV_ARGS(&pDAC)));

    IfFailedAssertReturn(CreateComObject(&pSinkDebugSite));
    IfFailedAssertReturn(pSinkDebugSite->Init(this, moduleName, m_Symbols3, m_Data4));

    IfFailedAssertReturn(pDAC->LoadScriptSymbols(pSinkDebugSite));

    // Print symbol names for baseline comparison
    pSinkDebugSite->PrintSymbolNames();
}

USE_DbgEngDataTarget(); // Use DbgEngDataTarget implementation

//
// GetDumpStreams on current target, and run "func" on the stream content
//
template <class Func>
void EXT_CLASS::TestGetDump(Func func)
{
    HRESULT hr = S_OK;
    //TODO: better logic to determine which dll to use
    HINSTANCE hInstance = LoadLibraryEx(_u("chakradiagtest.dll"), NULL, 0);
    if (!hInstance)
    {
        hInstance = LoadLibraryEx(_u("chakradiag.dll"), NULL, 0);
    }
    IfFalseAssertReturn(hInstance);

    PFN_GetDumpStreams GetDumpStreams = (PFN_GetDumpStreams)GetProcAddress(hInstance, "GetDumpStreams");
    PFN_FreeDumpStreams FreeDumpStreams = (PFN_FreeDumpStreams)GetProcAddress(hInstance, "FreeDumpStreams");
    IfFalseAssertReturn(GetDumpStreams);
    IfFalseAssertReturn(GetDumpStreams);
    
    CComPtr<DbgEngDataTarget> dataTarget;
    IfFailedAssertReturn(CreateComObject(&dataTarget));
    IfFailedAssertReturn(dataTarget->Init(m_Client));

    PMINIDUMP_USER_STREAM_INFORMATION pUserStream;
    IfFalseAssertReturn(GetDumpStreams(dataTarget, MINIDUMP_TYPE::MiniDumpWithoutAuxiliaryState, &pUserStream) == E_INVALIDARG);
    IfFalseAssertReturn(GetDumpStreams(NULL, MINIDUMP_TYPE::MiniDumpNormal, &pUserStream) == E_INVALIDARG);
    IfFalseAssertReturn(GetDumpStreams(dataTarget, MINIDUMP_TYPE::MiniDumpNormal, NULL) == E_POINTER);
    IfFailedAssertReturn(GetDumpStreams(dataTarget, MINIDUMP_TYPE::MiniDumpNormal, &pUserStream));

    if (pUserStream != NULL)
    {
        IfFalseAssertReturn(pUserStream->UserStreamCount == 1);
        IfFalseAssertReturn(pUserStream->UserStreamArray != NULL);

        PMINIDUMP_USER_STREAM pDump = pUserStream->UserStreamArray;
        IfFalseAssertReturn(pDump->Type == MINIDUMP_STREAM_TYPE::JavaScriptDataStream);

        func(pDump->Buffer, pDump->BufferSize, &dataTarget->GetMemoryRegions());

        // Release
        IfFailedAssertReturn(FreeDumpStreams(pUserStream));
    }

    FreeLibrary(hInstance);
}

// Get filename part of a path for baseline comparison
static string GetFileName(PCWSTR pPath)
{
    string path(pPath ? pPath : _u(""));
    string::size_type split = path.rfind(_T('\\'));
    return split != string::npos ? path.substr(split + 1) : path;
}

//
// Run jscript9diagdump to read the given dump stream content
//
void EXT_CLASS::TestReadDump(LPVOID buffer, ULONG bufferSize, _In_opt_ const DbgEngDataTarget::MemoryRegionsType* memoryRegions, const ULONG maxFrames)
{
    HRESULT hr = S_OK;
    HINSTANCE hInstance = LoadLibraryEx(_u("jscript9diagdump.dll"), NULL, 0);
    IfFalseAssertReturn(hInstance);

    PDEBUG_STACK_PROVIDER_BEGINTHREADSTACKRECONSTRUCTION BeginThreadStackReconstruction = (PDEBUG_STACK_PROVIDER_BEGINTHREADSTACKRECONSTRUCTION)GetProcAddress(hInstance, "BeginThreadStackReconstruction");
    PDEBUG_STACK_PROVIDER_RECONSTRUCTSTACK ReconstructStack = (PDEBUG_STACK_PROVIDER_RECONSTRUCTSTACK)GetProcAddress(hInstance, "ReconstructStack");
    PDEBUG_STACK_PROVIDER_FREESTACKSYMFRAMES FreeStackSymFrames = (PDEBUG_STACK_PROVIDER_FREESTACKSYMFRAMES)GetProcAddress(hInstance, "FreeStackSymFrames");
    PDEBUG_STACK_PROVIDER_ENDTHREADSTACKRECONSTRUCTION EndThreadStackReconstruction = (PDEBUG_STACK_PROVIDER_ENDTHREADSTACKRECONSTRUCTION)GetProcAddress(hInstance, "EndThreadStackReconstruction");
    JsDiag::PrivateGetStackThreadIdFunc* PrivateGetStackThreadId = (JsDiag::PrivateGetStackThreadIdFunc*)GetProcAddress(hInstance, "PrivateGetStackThreadId");
    IfFalseAssertReturn(BeginThreadStackReconstruction);
    IfFalseAssertReturn(ReconstructStack);
    IfFalseAssertReturn(FreeStackSymFrames);
    IfFalseAssertReturn(EndThreadStackReconstruction);
    IfFalseAssertReturn(PrivateGetStackThreadId);

    // Prepare
    PSTACK_SYM_FRAME_INFO pStackSymFrames;
    ULONG stackSymFramesFilled;
    ULONG noSuchThreadId = (ULONG)-1; // If non 0, no such threadId in the dump
    IfFalseAssertReturn(ReconstructStack(noSuchThreadId, NULL, 0, &pStackSymFrames, &stackSymFramesFilled) == E_UNEXPECTED); // Before Begin

    // Begin
    IfFalseAssertReturn(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::ExceptionStream, buffer, bufferSize) == E_INVALIDARG);               // wrong stream type
    IfFalseAssertReturn(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, NULL, bufferSize) == E_INVALIDARG);    // null data
    IfFalseAssertReturn(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, buffer, 0) == E_INVALIDARG);           // 0 size data
    ULONG bad_data[] = {0, 0, 0, 0};
    IfFalseAssertReturn(FAILED(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, bad_data, sizeof(bad_data))));  // bad data
    IfFailedAssertReturn(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, buffer, bufferSize));
    IfFalseAssertReturn(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, buffer, bufferSize) == E_UNEXPECTED);  // already begun

    // Test
    for (ULONG index = 0; ; index++)
    {
        ULONG threadId;
        if (PrivateGetStackThreadId(index, &threadId) == S_OK)
        {
            if (threadId == noSuchThreadId)
            {
                noSuchThreadId = 0; // Oops, found such a thread Id, don't use noSuchThreadId
            }

            IfFailedAssertReturn(ReconstructStack(threadId, NULL, 0, &pStackSymFrames, &stackSymFramesFilled));

            if (!m_unitTestMode)
            {
                Out(_u("Thread: 0x%x\n"), threadId);
#ifdef _M_X64
                Out(_u("%-17s %-17s %-17s\n"), _u("Child-SP"), _u("RetAddr"), _u("Inst"));
#else
                Out(_u("%-8s %-8s %-8s\n"),    _u("ChildEBP"), _u("RetAddr"), _u("Inst"));
#endif
            }

            BYTE nextInlineIndex = 0; // inline frame index starts from 0
            for (ULONG i = 0; i < std::min(stackSymFramesFilled, maxFrames); i++)
            {
                const STACK_SYM_FRAME_INFO& f = pStackSymFrames[i];

                BYTE inlineIndex;
                bool isInlineFrame = JsDiag::TryGetInlineFrameId(f.StackFrameEx.InlineFrameContext, &inlineIndex);
                if (isInlineFrame)
                {
                    IfFalseAssertReturn(inlineIndex == nextInlineIndex++);
                }
                else
                {
                    nextInlineIndex = 0;
                }

                if (!m_unitTestMode)
                {
                    Out(
#ifdef _M_X64
                        _u("%08x`%08x %08x`%08x %08x`%08x %s%s (%s:%d,%d)\n"),
                        HILONG(f.StackFrameEx.StackOffset), LOLONG(f.StackFrameEx.StackOffset),
                        HILONG(f.StackFrameEx.ReturnOffset), LOLONG(f.StackFrameEx.ReturnOffset),
                        HILONG(f.StackFrameEx.InstructionOffset), LOLONG(f.StackFrameEx.InstructionOffset),
#else
                        _u("%08x %08x %08x %s%s (%s:%d,%d)\n"),
                        (ULONG)f.StackFrameEx.FrameOffset,
                        (ULONG)f.StackFrameEx.ReturnOffset,
                        (ULONG)f.StackFrameEx.InstructionOffset,
#endif
                        isInlineFrame ? _u("--") : _u(""),
                        f.SrcInfo.Function,
                        f.SrcInfo.ImagePath,
                        f.SrcInfo.Row,
                        f.SrcInfo.Column);
                }
                else
                {
                    Out(_u("%s%s (%s:%d,%d)\n"), isInlineFrame ? _u("--") : _u(""), f.SrcInfo.Function, GetFileName(f.SrcInfo.ImagePath).c_str(), f.SrcInfo.Row, f.SrcInfo.Column);
                    if (memoryRegions)
                    {
                        ULONG64 ip = f.StackFrameEx.InstructionOffset;
                        auto it = std::find_if(memoryRegions->begin(), memoryRegions->end(),
                            [ip](const std::pair<ULONG64, ULONG32>& range) -> bool
                        {
                            return ip >= range.first && ip < range.first + range.second;
                        });
                        IfFalseAssertMsgReturn(it != memoryRegions->end(), "JS frame IP must be in enumerated memory regions");
                    }
                }
            }

            IfFailedAssertReturn(FreeStackSymFrames(pStackSymFrames));
        }
        else
        {
            break;
        }
    }

    // Before End
    if (noSuchThreadId != 0)
    {
        IfFailedAssertReturn(ReconstructStack(noSuchThreadId, NULL, 0, &pStackSymFrames, &stackSymFramesFilled));
        IfFalseAssertReturn(pStackSymFrames == NULL);
        IfFalseAssertReturn(stackSymFramesFilled == 0);
    }

    // End
    IfFailedAssertReturn(EndThreadStackReconstruction());
    IfFalseAssertReturn(ReconstructStack(noSuchThreadId, NULL, 0, &pStackSymFrames, &stackSymFramesFilled) == E_UNEXPECTED); // After End
    FreeLibrary(hInstance);
}

EXT_COMMAND(writedump,
    "Write WER js stack dump to a file",
    "{;x;file;file to write dump stream}")
{
    HRESULT hr = S_OK;
    CAtlFile file;
    CA2W filename = GetUnnamedArgStr(0);
    IfFailedAssertReturn(file.Create(filename, GENERIC_WRITE, 0, CREATE_ALWAYS));

    TestGetDump([&](LPVOID buffer, ULONG bufferSize, const DbgEngDataTarget::MemoryRegionsType*)
    {
        IfFailedAssertReturn(file.Write(buffer, bufferSize));
    });
}

EXT_COMMAND(readdump,
    "Read WER js stack dump file",
    "{;x;file;file to read}")
{
    HRESULT hr = S_OK;
    CAtlFile file;
    CA2W filename = GetUnnamedArgStr(0);
    IfFailedAssertReturn(file.Create(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING));

    // Read the file into memory
    ULONGLONG fileSize;
    ULONG bufferSize;
    IfFailedAssertReturn(file.GetSize(fileSize));
    IfFailedAssertReturn(ULongLongToULong(fileSize, &bufferSize));

    CAutoVectorPtr<BYTE> buffer;
    if (!buffer.Allocate(bufferSize))
    {
        ThrowOutOfMemory();
    }
    IfFailedAssertReturn(file.Read(buffer, bufferSize));

    TestReadDump(buffer, bufferSize, /*memoryRegions*/nullptr);
}

EXT_COMMAND(testdump,
    "Test WER js stack dump",
    "{;e,o,d=-1;maxFrames;max number of js frames to print}")
{
    const ULONG maxFrames = static_cast<ULONG>(GetUnnamedArgU64(0));
    TestGetDump([=](LPVOID buffer, ULONG bufferSize, const DbgEngDataTarget::MemoryRegionsType* memoryRegions)
    {
        TestReadDump(buffer, bufferSize, memoryRegions, maxFrames);
    });
}


HRESULT EXT_CLASS::PrivateCoCreate(LPCWSTR strModule, REFCLSID rclsid, REFIID iid, LPVOID* ppunk)
{
    typedef HRESULT(STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);

    HRESULT hr = NOERROR;
    CComPtr <IClassFactory> pClassFactory;
    FN_DllGetClassObject pProc = NULL;

    HINSTANCE hInstance = LoadLibraryEx(strModule, NULL, 0);
    IfNullGo(hInstance, E_FAIL);
    IfNullGo(pProc = (FN_DllGetClassObject)GetProcAddress(hInstance, "DllGetClassObject"), E_FAIL);
    IfFailGo(pProc(rclsid, __uuidof(IClassFactory), (LPVOID*)&pClassFactory));
    IfFailGo(pClassFactory->CreateInstance(NULL, iid, ppunk));
Error:
    return hr;
}

void EXT_CLASS::IfFailThrow(HRESULT hr, PCSTR msg)
{
    if (FAILED(hr))
    {
        ThrowStatus(hr, "%s", msg ? msg : "");
    }
}

//
// Setup sympath needed by !ldsym. Do this in test so that we no longer depend on environment setup.
// Return true if sympath is changed by this method.
//
bool EXT_CLASS::SetupSymbolPath()
{
    _TCHAR buf[MAX_PATH];
    m_Symbols3->GetSymbolPathT(buf, _countof(buf), NULL);

    CString sympath(buf);
    sympath.MakeLower();

    bool sympathChanged = false;
    {
        CString symbols;
        symbols.GetEnvironmentVariable(_T("_NTTREE"));
        symbols += _T("\\symbols.pri\\");

        CString s = symbols + _T("retail");
        if (sympath.Find(s) < 0)
        {
            sympath = s + _T(";") + sympath;
            sympathChanged = true;
        }

        s = symbols + _T("jscript");
        if (sympath.Find(s) < 0)
        {
            sympath = s + _T(";") + sympath;
            sympathChanged = true;
        }
    }

    if (sympathChanged)
    {
        m_Symbols3->SetSymbolPathT(sympath);
    }

    return sympathChanged;
}

class Module: public CAtlDllModuleT<Module> {} _Module;
