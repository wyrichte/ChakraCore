//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "activaut.h"
#include "guids.h"

void EXT_CLASS::CreateDebugProcess(IJsDebugProcess** ppDebugProcess, bool debugMode) 
{
    CComPtr<IJsDebug2> jsDebug;
    CreateJsDebug(&jsDebug);

    CComPtr<DbgEngDataTarget> dataTarget;
    UT_HR_SUCCEEDED(CreateComObject(&dataTarget));
    UT_HR_SUCCEEDED(dataTarget->Init(m_Client));

    DWORD processId;
    IfFailThrow(m_System->GetCurrentProcessSystemId(&processId));

    UINT64 baseAddress; ULONG index;
    UT_HR_SUCCEEDED(FindJScriptModuleByName</*IsPublic*/false>(m_Symbols, &index, &baseAddress));

    UT_HR_SUCCEEDED(jsDebug->OpenVirtualProcess(dataTarget, /*debugMode*/ debugMode, processId, baseAddress, NULL, ppDebugProcess));
}

// EXT_DECLARE_GLOBALS must be used to instantiate
// the framework's assumed globals.
EXT_DECLARE_GLOBALS();

EXT_CLASS::EXT_CLASS():
    m_unitTestMode(true)
{
}

void EXT_CLASS::CreateJsDebug(IJsDebug2** ppDebug)
{
    CComPtr<IJsDebug2> jsDebug;

    //TODO: better logic to determine which dll to use
    HRESULT hr = PrivateCoCreate(L"chakradiagtest.dll", CLSID_ChakraDiag, IID_PPV_ARGS(&jsDebug));
    if (FAILED(hr))
    {
        UT_HR_SUCCEEDED(PrivateCoCreate(L"chakradiag.dll", CLSID_ChakraDiag, IID_PPV_ARGS(&jsDebug)));
    }

    *ppDebug = jsDebug.Detach();
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
        s = std::wstring(L"$ut$") + fmt;
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

EXT_COMMAND(jsstack,
    "Test JavaScript stack walking",
    "")
{
    
    Out(L"\nPrinting stacktrace..\n");

    DWORD threadId;
    IfFailThrow(m_System->GetCurrentThreadSystemId(&threadId));

    CComPtr<IJsDebugProcess> debugProcess;
    CreateDebugProcess(&debugProcess);
    CComPtr<IJsDebugStackWalker> stackWalker;
    UT_HR_SUCCEEDED(debugProcess->CreateStackWalker(threadId, &stackWalker));
    CComPtr<IJsDebugFrame> debugFrame;
    HRESULT hr;
    while( (hr = stackWalker->GetNext(&debugFrame)) != E_JsDEBUG_OUTSIDE_OF_VM)
    {
        IfFailThrow(hr);
        CComBSTR name;
        IfFailThrow(debugFrame->GetName(&name));
        CComBSTR url;
        DWORD line, column;

        UINT64 start, end;
        UT_HR_SUCCEEDED(debugFrame->GetStackRange(&start, &end));
        this->Verb(L"%08I64x, %08I64x ", start, end);
        UT_ASSERT(end <= start);
        UT_ASSERT(end != 0 && start != 0);

        UINT64 returnAddress;
        UT_HR_SUCCEEDED(debugFrame->GetReturnAddress(&returnAddress));
        this->Verb(L"%08I64x ", returnAddress);
        UT_ASSERT(returnAddress != 0);

        UT_HR_SUCCEEDED(debugFrame->GetDocumentPositionWithName(&url, &line, &column));

        UINT64 documentId; DWORD characterOffset; DWORD statementLength;
        UT_HR_SUCCEEDED(debugFrame->GetDocumentPositionWithId(&documentId, &characterOffset, &statementLength));

        wchar_t filename[_MAX_FNAME];
        wchar_t ext[_MAX_EXT];
        _wsplitpath_s(url, NULL, 0, NULL, 0, filename, _MAX_FNAME, ext, _MAX_EXT);

        Out(L"%s (%s%s:%u,%u)\n", name, filename, ext, line, column);

        debugFrame.Release();
    }
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
    Verb(L"Use %s\n", imageName);

    // .reload jscript9.dll, if sympath has changed. Without this FindSymbol will still fail if there was a previous failure (e.g., k) with missing symbols.
    if (sympathChanged)
    {
        CString module = CString(moduleName) + ".dll";
        UT_HR_SUCCEEDED(m_Symbols3->ReloadT(module));
    }

    const CLSID CLSID_JScript9DAC = {0x197060cb, 0x5efb, 0x4a53, 0xb0, 0x42, 0x93, 0x9d, 0xbb, 0x31, 0x62, 0x7c};
    CComPtr<IScriptDAC> pDAC;
    CComPtr<SinkDebugSite> pSinkDebugSite;

    UT_HR_SUCCEEDED(PrivateCoCreate(imageName, CLSID_JScript9DAC, IID_PPV_ARGS(&pDAC)));

    UT_HR_SUCCEEDED(CreateComObject(&pSinkDebugSite));
    UT_HR_SUCCEEDED(pSinkDebugSite->Init(this, moduleName, m_Symbols3, m_Data4));

    UT_HR_SUCCEEDED(pDAC->LoadScriptSymbols(pSinkDebugSite));

    // Print symbol names for baseline comparison
    pSinkDebugSite->PrintSymbolNames();
}


EXT_COMMAND(verifyOM,
    "Test OM error handling",
    "")
{
    CComPtr<IJsDebugProcess> debugProcess; 
    CreateDebugProcess(&debugProcess);
    
    ULONG count;
    IfFailThrow(m_System->GetNumberThreads(&count));
    UT_ASSERT_SZ(count > 1, "Count should be greater than 1 to validate");
    ULONG currentThreadId;
    IfFailThrow(m_System->GetCurrentThreadSystemId(&currentThreadId));
    CAutoVectorPtr<ULONG> threads(new ULONG[count]);
    IfFailThrow(m_System->GetThreadIdsByIndex(0, count, NULL, threads));
    ULONG otherThreadId = (ULONG)-1;
    for(ULONG i=0; i < count; i++)
    {
        if(threads[i] != currentThreadId)
        {
            otherThreadId = threads[i];
            break;
        }
    }
    UT_ASSERT(otherThreadId != (ULONG)-1);
    
    CComPtr<IJsDebugStackWalker> stackWalker;
    UT_HRESULTS_EQUAL(E_JsDEBUG_UNKNOWN_THREAD, debugProcess->CreateStackWalker(otherThreadId, &stackWalker));
    UT_HRESULTS_EQUAL(E_JsDEBUG_UNKNOWN_THREAD, debugProcess->PerformAsyncBreak(otherThreadId));
    ULONG threadIdAfterStackWalkerCreation;
    IfFailThrow(m_System->GetCurrentThreadSystemId(&threadIdAfterStackWalkerCreation));

    UT_ASSERT(threadIdAfterStackWalkerCreation == currentThreadId);
}

EXT_COMMAND(asyncBreak,
    "Put runtime into async break mode",
    "")
{
    CComPtr<IJsDebugProcess> debugProcess; 
    CreateDebugProcess(&debugProcess);
    ULONG currentThreadId;
    IfFailThrow(m_System->GetCurrentThreadSystemId(&currentThreadId));
    UT_HR_SUCCEEDED(debugProcess->PerformAsyncBreak(currentThreadId));
}

EXT_COMMAND(bp,
    "Set breakpoint",
    "{;e,o,d=0;characterOffset;The character offset where to set the breakpoint}")
{
    ULONG offset = static_cast<ULONG>(GetUnnamedArgU64(0));

    CComPtr<IJsDebugProcess> debugProcess; 
    CreateDebugProcess(&debugProcess, /*debugMode*/ true);

    ULONG currentThreadId;
    IfFailThrow(m_System->GetCurrentThreadSystemId(&currentThreadId));

    CComPtr<IJsDebugStackWalker> stackWalker;
    UT_HR_SUCCEEDED(debugProcess->CreateStackWalker(currentThreadId, &stackWalker));
    CComPtr<IJsDebugFrame> debugFrame;
    UT_HR_SUCCEEDED(stackWalker->GetNext(&debugFrame));
    UINT64 documentId; DWORD characterOffset; DWORD statementLength;
    UT_HR_SUCCEEDED(debugFrame->GetDocumentPositionWithId(&documentId, &characterOffset, &statementLength));
    UT_ASSERT(documentId != 0);
    CComPtr<IJsDebugBreakPoint> debugBreakPoint;
    UT_HR_SUCCEEDED(debugProcess->CreateBreakPoint(documentId, offset, 10, /*isEnabled*/ true, &debugBreakPoint));
    UINT64 breakPointDocumentId;
    UT_HR_SUCCEEDED(debugBreakPoint->GetDocumentPosition(&breakPointDocumentId, &characterOffset, &statementLength));
    UT_ASSERT(documentId == breakPointDocumentId);
    wprintf_s(L"Bp position: (%u, %u) \n", characterOffset, statementLength);
    BOOL isEnabled;
    UT_HR_SUCCEEDED(debugBreakPoint->IsEnabled(&isEnabled));
    UT_ASSERT(isEnabled == TRUE);
    
    //  Testing the interface
    UT_HR_SUCCEEDED(debugBreakPoint->Disable());
    UT_HR_SUCCEEDED(debugBreakPoint->IsEnabled(&isEnabled));
    UT_ASSERT(isEnabled == FALSE);

    UT_HR_SUCCEEDED(debugBreakPoint->Enable());
    UT_ASSERT(debugBreakPoint->Enable() == S_FALSE);
}

USE_DbgEngDataTarget(); // Use DbgEngDataTarget implementation

//
// GetDumpStreams on current target, and run "func" on the stream content
//
template <class Func>
void EXT_CLASS::TestGetDump(Func func)
{
    //TODO: better logic to determine which dll to use
    HINSTANCE hInstance = LoadLibraryEx(L"chakradiagtest.dll", NULL, 0);
    if (!hInstance)
    {
        hInstance = LoadLibraryEx(L"chakradiag.dll", NULL, 0);
    }
    UT_ASSERT(hInstance);

    PFN_GetDumpStreams GetDumpStreams = (PFN_GetDumpStreams)GetProcAddress(hInstance, "GetDumpStreams");
    PFN_FreeDumpStreams FreeDumpStreams = (PFN_FreeDumpStreams)GetProcAddress(hInstance, "FreeDumpStreams");
    UT_ASSERT(GetDumpStreams);
    UT_ASSERT(GetDumpStreams);
    
    CComPtr<DbgEngDataTarget> dataTarget;
    UT_HR_SUCCEEDED(CreateComObject(&dataTarget));
    UT_HR_SUCCEEDED(dataTarget->Init(m_Client));

    PMINIDUMP_USER_STREAM_INFORMATION pUserStream;
    UT_ASSERT(GetDumpStreams(dataTarget, MINIDUMP_TYPE::MiniDumpWithoutAuxiliaryState, &pUserStream) == E_INVALIDARG);
    UT_ASSERT(GetDumpStreams(NULL, MINIDUMP_TYPE::MiniDumpNormal, &pUserStream) == E_INVALIDARG);
    UT_ASSERT(GetDumpStreams(dataTarget, MINIDUMP_TYPE::MiniDumpNormal, NULL) == E_POINTER);
    UT_HR_SUCCEEDED(GetDumpStreams(dataTarget, MINIDUMP_TYPE::MiniDumpNormal, &pUserStream));

    if (pUserStream != NULL)
    {
        UT_ASSERT(pUserStream->UserStreamCount == 1);
        UT_ASSERT(pUserStream->UserStreamArray != NULL);

        PMINIDUMP_USER_STREAM pDump = pUserStream->UserStreamArray;
        UT_ASSERT(pDump->Type == MINIDUMP_STREAM_TYPE::JavaScriptDataStream);

        func(pDump->Buffer, pDump->BufferSize, &dataTarget->GetMemoryRegions());

        // Release
        UT_HR_SUCCEEDED(FreeDumpStreams(pUserStream));
    }

    FreeLibrary(hInstance);
}

// Get filename part of a path for baseline comparison
static string GetFileName(PCWSTR pPath)
{
    string path(pPath ? pPath : L"");
    string::size_type split = path.rfind(_T('\\'));
    return split != string::npos ? path.substr(split + 1) : path;
}

//
// Run jscript9diagdump to read the given dump stream content
//
void EXT_CLASS::TestReadDump(LPVOID buffer, ULONG bufferSize, _In_opt_ const DbgEngDataTarget::MemoryRegionsType* memoryRegions, const ULONG maxFrames)
{
    HINSTANCE hInstance = LoadLibraryEx(L"jscript9diagdump.dll", NULL, 0);
    UT_ASSERT(hInstance);

    PDEBUG_STACK_PROVIDER_BEGINTHREADSTACKRECONSTRUCTION BeginThreadStackReconstruction = (PDEBUG_STACK_PROVIDER_BEGINTHREADSTACKRECONSTRUCTION)GetProcAddress(hInstance, "BeginThreadStackReconstruction");
    PDEBUG_STACK_PROVIDER_RECONSTRUCTSTACK ReconstructStack = (PDEBUG_STACK_PROVIDER_RECONSTRUCTSTACK)GetProcAddress(hInstance, "ReconstructStack");
    PDEBUG_STACK_PROVIDER_FREESTACKSYMFRAMES FreeStackSymFrames = (PDEBUG_STACK_PROVIDER_FREESTACKSYMFRAMES)GetProcAddress(hInstance, "FreeStackSymFrames");
    PDEBUG_STACK_PROVIDER_ENDTHREADSTACKRECONSTRUCTION EndThreadStackReconstruction = (PDEBUG_STACK_PROVIDER_ENDTHREADSTACKRECONSTRUCTION)GetProcAddress(hInstance, "EndThreadStackReconstruction");
    JsDiag::PrivateGetStackThreadIdFunc* PrivateGetStackThreadId = (JsDiag::PrivateGetStackThreadIdFunc*)GetProcAddress(hInstance, "PrivateGetStackThreadId");
    UT_ASSERT(BeginThreadStackReconstruction);
    UT_ASSERT(ReconstructStack);
    UT_ASSERT(FreeStackSymFrames);
    UT_ASSERT(EndThreadStackReconstruction);
    UT_ASSERT(PrivateGetStackThreadId);

    // Prepare
    PSTACK_SYM_FRAME_INFO pStackSymFrames;
    ULONG stackSymFramesFilled;
    ULONG noSuchThreadId = (ULONG)-1; // If non 0, no such threadId in the dump
    UT_ASSERT(ReconstructStack(noSuchThreadId, NULL, 0, &pStackSymFrames, &stackSymFramesFilled) == E_UNEXPECTED); // Before Begin

    // Begin
    UT_ASSERT(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::ExceptionStream, buffer, bufferSize) == E_INVALIDARG);               // wrong stream type
    UT_ASSERT(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, NULL, bufferSize) == E_INVALIDARG);    // null data
    UT_ASSERT(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, buffer, 0) == E_INVALIDARG);           // 0 size data
    ULONG bad_data[] = {0, 0, 0, 0};
    UT_ASSERT(FAILED(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, bad_data, sizeof(bad_data))));  // bad data
    UT_HR_SUCCEEDED(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, buffer, bufferSize));
    UT_ASSERT(BeginThreadStackReconstruction(MINIDUMP_STREAM_TYPE::JavaScriptDataStream, buffer, bufferSize) == E_UNEXPECTED);  // already begun

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

            UT_HR_SUCCEEDED(ReconstructStack(threadId, NULL, 0, &pStackSymFrames, &stackSymFramesFilled));

            if (!m_unitTestMode)
            {
                Out(L"Thread: 0x%x\n", threadId);
#ifdef _M_X64
                Out(L"%-17s %-17s %-17s\n", L"Child-SP", L"RetAddr", L"Inst");
#else
                Out(L"%-8s %-8s %-8s\n",    L"ChildEBP", L"RetAddr", L"Inst");
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
                    UT_ASSERT(inlineIndex == nextInlineIndex++);
                }
                else
                {
                    nextInlineIndex = 0;
                }

                if (!m_unitTestMode)
                {
                    Out(
#ifdef _M_X64
                        L"%08x`%08x %08x`%08x %08x`%08x %s%s (%s:%d,%d)\n",
                        HILONG(f.StackFrameEx.StackOffset), LOLONG(f.StackFrameEx.StackOffset),
                        HILONG(f.StackFrameEx.ReturnOffset), LOLONG(f.StackFrameEx.ReturnOffset),
                        HILONG(f.StackFrameEx.InstructionOffset), LOLONG(f.StackFrameEx.InstructionOffset),
#else
                        L"%08x %08x %08x %s%s (%s:%d,%d)\n",
                        (ULONG)f.StackFrameEx.FrameOffset,
                        (ULONG)f.StackFrameEx.ReturnOffset,
                        (ULONG)f.StackFrameEx.InstructionOffset,
#endif
                        isInlineFrame ? L"--" : L"",
                        f.SrcInfo.Function,
                        f.SrcInfo.ImagePath,
                        f.SrcInfo.Row,
                        f.SrcInfo.Column);
                }
                else
                {
                    Out(L"%s%s (%s:%d,%d)\n", isInlineFrame ? L"--" : L"", f.SrcInfo.Function, GetFileName(f.SrcInfo.ImagePath).c_str(), f.SrcInfo.Row, f.SrcInfo.Column);
                    if (memoryRegions)
                    {
                        ULONG64 ip = f.StackFrameEx.InstructionOffset;
                        auto it = std::find_if(memoryRegions->begin(), memoryRegions->end(),
                            [ip](const std::pair<ULONG64, ULONG32>& range) -> bool
                        {
                            return ip >= range.first && ip < range.first + range.second;
                        });
                        UT_ASSERT_SZ(it != memoryRegions->end(), "JS frame IP must be in enumerated memory regions");
                    }
                }
            }

            UT_HR_SUCCEEDED(FreeStackSymFrames(pStackSymFrames));
        }
        else
        {
            break;
        }
    }

    // Before End
    if (noSuchThreadId != 0)
    {
        UT_HR_SUCCEEDED(ReconstructStack(noSuchThreadId, NULL, 0, &pStackSymFrames, &stackSymFramesFilled));
        UT_ASSERT(pStackSymFrames == NULL);
        UT_ASSERT(stackSymFramesFilled == 0);
    }

    // End
    UT_HR_SUCCEEDED(EndThreadStackReconstruction());
    UT_ASSERT(ReconstructStack(noSuchThreadId, NULL, 0, &pStackSymFrames, &stackSymFramesFilled) == E_UNEXPECTED); // After End
    FreeLibrary(hInstance);
}

EXT_COMMAND(writedump,
    "Write WER js stack dump to a file",
    "{;x;file;file to write dump stream}")
{
    CAtlFile file;
    CA2W filename = GetUnnamedArgStr(0);
    UT_HR_SUCCEEDED(file.Create(filename, GENERIC_WRITE, 0, CREATE_ALWAYS));

    TestGetDump([&](LPVOID buffer, ULONG bufferSize, const DbgEngDataTarget::MemoryRegionsType*)
    {
        UT_HR_SUCCEEDED(file.Write(buffer, bufferSize));
    });
}

EXT_COMMAND(readdump,
    "Read WER js stack dump file",
    "{;x;file;file to read}")
{
    CAtlFile file;
    CA2W filename = GetUnnamedArgStr(0);
    UT_HR_SUCCEEDED(file.Create(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING));

    // Read the file into memory
    ULONGLONG fileSize;
    ULONG bufferSize;
    UT_HR_SUCCEEDED(file.GetSize(fileSize));
    UT_HR_SUCCEEDED(ULongLongToULong(fileSize, &bufferSize));

    CAutoVectorPtr<BYTE> buffer;
    if (!buffer.Allocate(bufferSize))
    {
        ThrowOutOfMemory();
    }
    UT_HR_SUCCEEDED(file.Read(buffer, bufferSize));

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
    typedef HRESULT (STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);

    HRESULT hr = NOERROR;
    CComPtr <IClassFactory> pClassFactory;
    FN_DllGetClassObject pProc = NULL;

    HINSTANCE hInstance = LoadLibraryEx(strModule, NULL, 0);
    IfNullGo(hInstance, E_FAIL);
    IfNullGo(pProc = (FN_DllGetClassObject) GetProcAddress(hInstance, "DllGetClassObject"), E_FAIL);
    IfFailGo(pProc(rclsid, __uuidof(IClassFactory), (LPVOID*)&pClassFactory));
    IfFailGo(pClassFactory->CreateInstance(NULL, iid, ppunk));
Error:
    return hr;
}

void EXT_CLASS::IfFailThrow(HRESULT hr, PCSTR msg)
{
    if (FAILED(hr))
    {
        ThrowLastError(msg);
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
