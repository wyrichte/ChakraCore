//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "guids.h"

#include "werdump.h"
#include "MockDataTarget.h"
#include "DiagException.h"
#include "BasePtr.h"
#include "AutoPtr.h"
#include "DiagAutoPtr.h"
#include "thrownew.h"
#include "Serializer.h"
#include "Serializer.inl"
#include "ScriptDump.h"
#include "ScriptDumpReader.h"

using namespace JsDiag;

//
// Ensure to load chakradiag
//
void EXT_CLASS_BASE::EnsureJsDebug(PCWSTR jscript9diagPath /*= nullptr*/)
{
    if (m_jsDebug && !jscript9diagPath)
    {
        return; // Use existing one
    }

    PCWSTR paths[] = { jscript9diagPath, _u("chakradiagtest.dll"), _u("chakradiag.dll") };
    const int begin = jscript9diagPath ? 0 : 1;
    const int end = jscript9diagPath ? 1 : _countof(paths);

    for (int i = begin; i < end; i++)
    {
        m_jsDebug.Release();
        if (SUCCEEDED(PrivateCoCreate(paths[i], CLSID_ChakraDiag, IID_PPV_ARGS(&m_jsDebug))))
        {
            return; // Success
        }
    }

    ThrowLastError("Failed to load chakradiag");
}

JD_PRIVATE_COMMAND(diagvar,
    "Print var",
    "{;e;var;The Js::Var to print}{p;x;path;Path to chakradiag dll}")
{
    ULONG64 var = GetUnnamedArgU64(0);

    CA2W path = GetArgStr("p", false);
    PCWSTR diagImage = HasArg("p") ? path : (PCWSTR)nullptr;
    EnsureJsDebug(diagImage);

    CComPtr<IJsDebugProcess> debugProcess;
    CreateDebugProcess(&debugProcess);

    CComPtr<IJsDebugProcessPrivate> debugProcessPrivate;
    IfFailThrow(debugProcess->QueryInterface(&debugProcessPrivate), "Private inspection APIs not found. Ensure the correct version of chakradiag.dll is specified");

    CComPtr<IJsDebugProperty> debugProperty;
    IfFailThrow(debugProcessPrivate->InspectVar((void*)var, &debugProperty));

    Print(debugProperty, /*radix*/10, /*maxDepth*/2);
}

JD_PRIVATE_COMMAND(diagstack,
            "Print JavaScript stack",
            "{n;b;;Print frame number}{p;x;path;Path to chakradiag dll}")
{
    bool printFrameNo = HasArg("n");
    CA2W path = GetArgStr("p", false);
    PCWSTR diagImage = HasArg("p") ? path : (PCWSTR)nullptr;

    EnsureJsDebug(diagImage);

    CComPtr<IJsDebugStackWalker> stackWalker;
    CreateStackWalker(&stackWalker);

    CComPtr<IJsDebugFrame> debugFrame;
    HRESULT hr;
    int frameNo = 0;
    while( (hr = stackWalker->GetNext(&debugFrame)) != E_JsDEBUG_OUTSIDE_OF_VM)
    {
        IfFailThrow(hr);
        CComBSTR name;
        IfFailThrow(debugFrame->GetName(&name));
        CComBSTR url;
        DWORD line, column;

        IfFailThrow(debugFrame->GetDocumentPositionWithName(&url, &line, &column));

        char16 filename[_MAX_FNAME];
        char16 ext[_MAX_EXT];
        _wsplitpath_s(url, NULL, 0, NULL, 0, filename, _MAX_FNAME, ext, _MAX_EXT);

        if (printFrameNo)
        {
            Out(_u("%2x %s (%s%s:%u,%u)\n"), frameNo++, (BSTR)name, filename, ext, line, column);
        }
        else
        {
            Out(_u("%s (%s%s:%u,%u)\n"), (BSTR)name, filename, ext, line, column);
        }

        UINT64 start, end;
        IfFailThrow(debugFrame->GetStackRange(&start, &end));
        Assert(end == NULL || end < start);

        debugFrame.Release();
    }
}

static PCSTR GetDiagErrorCodeName(DiagErrorCode diagErrorCode)
{
    switch (diagErrorCode)
    {
#define ENUM__(x) case DiagErrorCode::##x: return #x;
#include "DiagErrorCode.h"
    default:
        return "Unknown DiagErrorCode";
    }
}

JD_PRIVATE_COMMAND(jsstream,
    "Print jsstream data in this dump (dump debugging only)",
    "")
{
    const int MAX_LEN = 0x10000; // 64k

    CAutoVectorPtr<BYTE> buffer;
    if (!buffer.Allocate(MAX_LEN))
    {
        ThrowLastError("Failed to allocate buffer to read stream");
    }

    DEBUG_READ_USER_MINIDUMP_STREAM userStream;
    userStream.StreamType = MINIDUMP_STREAM_TYPE::JavaScriptDataStream;
    userStream.Flags = 0;
    userStream.Offset = 0;
    userStream.Buffer = buffer;
    userStream.BufferSize = MAX_LEN * sizeof(BYTE);
    userStream.BufferUsed = 0;

    HRESULT hr = m_Advanced2->Request(DEBUG_REQUEST_READ_USER_MINIDUMP_STREAM, &userStream, sizeof(userStream), NULL, 0, NULL);
    if (SUCCEEDED(hr))
    {
        Out("Found JavaScriptDataStream, size: 0x%x\n", userStream.BufferUsed);

        try
        {
            // Try to decode the data
            MemoryReadStream stream(buffer, userStream.BufferUsed);
            WerMessage message;

            Serializer::Deserialize(&stream, NULL, &message);

            // Verify magic cookie
            message.ValidateMagicCookie();

            if (FAILED(message.ErrorHr))
            {
                if (PreferDML())
                {
                    Dml("HRESULT <link cmd=\"!err 0x%x\">0x%x</link>, DiagErrorCode: %s (%d)\n",
                        message.ErrorHr, message.ErrorHr, GetDiagErrorCodeName(message.ErrorCode), message.ErrorCode);
                }
                else
                {
                    Out("HRESULT 0x%x /*\"!err 0x%x\" to lookup*/, DiagErrorCode: %s (%d)\n",
                        message.ErrorHr, message.ErrorHr, GetDiagErrorCodeName(message.ErrorCode), message.ErrorCode);
                }
            }
            else
            {
                Out("%d JavaScript stack(s)\n", message.StackCount);
                for (ULONG i = 0; i < message.StackCount; i++)
                {
                    const WerStack& stack = message.Stacks[i];
                    Out("Thread 0x%x\n", stack.ThreadId);
                    for (ULONG k = 0; k < stack.FrameCount; k++)
                    {
                        const WerFrame& frame = stack.Frames[k];
                        Out("%08I64x %08I64x %S [%S @ %d,%d]\n",
                            frame.FrameBase, frame.ReturnAddress, frame.FunctionName, frame.Uri, frame.Row, frame.Col);
                    }
                    Out("\n");
                }
            }
        }
        catch (...) // Something wrong -- OOM or corrupted stream -- dump the raw data instead
        {
            for (ULONG i = 0; i < userStream.BufferUsed; i += 0x10)
            {
                char tmp[128];
                char* p = tmp;

                const ULONG end = std::min(i + 0x10, userStream.BufferUsed);
                for (ULONG k = i; k < end; k++)
                {
                    sprintf_s(p, tmp + _countof(tmp) - p, "%02X ", buffer[k]);
                    p += 3;

                    if (k - i == 7)
                    {
                        p[-1] = '-';
                    }
                }

                while (p < tmp + 0x10 * 3)
                {
                    *p++ = ' ';
                }

                *p++ = ' ';
                for (ULONG k = i; k < end; k++)
                {
                    *p++ = isprint(buffer[k]) ? static_cast<char>(buffer[k]) : '.';
                }

                *p = '\0';
                Out("%08X  %s\n", i, tmp);
            }
        }
    }
    else
    {
        Out("ERROR: Failed to retrieve JavaScriptDataStream\n");
    }
}

//
// Ensure we are at a stack frame.
//  frame == -1: Try to stay at current frame. This allows multiple eval() at the same frame.
//
void EXT_CLASS_BASE::EnsureStackFrame(int frame/*= -1*/)
{
    if (m_jsFrame && (frame < 0 || frame == m_jsFrameNumber))
    {
        return; // Done, use existing frame
    }

    CComPtr<IJsDebugStackWalker> stackWalker;
    CreateStackWalker(&stackWalker);
    Assert(!m_jsFrame && m_jsFrameNumber == -1); // CreateStackWalker clears them

    HRESULT hr;
    while((hr = stackWalker->GetNext(&m_jsFrame)) != E_JsDEBUG_OUTSIDE_OF_VM)
    {
        IfFailThrow(hr);

        if (++m_jsFrameNumber >= frame)
        {
            return; // Found
        }

        m_jsFrame.Release();
    }

    ThrowLastError("Fail to get requested stack frame. Is debugger at the script thread?");
}

JD_PRIVATE_COMMAND(diagframe,
    "Inspect a JavaScript stack frame",
    "{;e,o,d=-1;frameNo;Javascript stack frame number}{r;s,o;radix;radix for formatting numbers}{d;e,o,d=3;depth;recursion depth}")
{
    const int frameNo = static_cast<int>(GetUnnamedArgU64(0));

    EnsureStackFrame(frameNo);
    Assert(m_jsFrame);

    if (m_jsFrame)
    {
        CComPtr<IJsDebugProperty> prop;
        IfFailThrow(m_jsFrame->GetDebugProperty(&prop));

        int radix = 10;
        if (HasArg("r"))
        {
            radix = atoi(GetArgStr("r", false));
            if (radix < 2 || radix > 36)
            {
                throw ExtInvalidArgumentException("radix must be within 2-36");
            }
        }

        int maxDepth = 3;
        if (HasArg("d"))
        {
            maxDepth = (int)GetArgU64("d");
        }
        Print(prop, radix, maxDepth);
    }
}

JD_PRIVATE_COMMAND(diageval,
    "Evaluate an expression on current JS stack frame",
    "{{custom}}")
{
    EnsureStackFrame();

    if (m_jsFrame)
    {
        CA2W s = GetRawArgStr();

        CComBSTR bstrError;
        CComPtr<IJsDebugProperty> prop;
        IfFailThrow(m_jsFrame->Evaluate(s, &prop, &bstrError));

        if (bstrError)
        {
            Out(_u("ERROR: %s\n"), (BSTR)bstrError);
        }
        else
        {
            AutoJsDebugPropertyInfo info;
            IfFailThrow(prop->GetPropertyInfo(10, &info));

            Print(prop, /*radix*/10, /*maxDepth*/2);
        }
    }
}

JD_PRIVATE_COMMAND(utmode,
    "Turn on/off chakradiag unit test mode",
    "{;e,o,d=1;on;Use 1 to turn on, 0 to turn off}")
{
    m_unitTestMode = GetUnnamedArgU64(0) != 0;
}

JD_PRIVATE_COMMAND(dumptrace,
    "Dump in-memory trace from (the process/dump must've been run with -trace:... -InMemoryTrace)",
    "")
{
    ExtRemoteTyped memoryLogger = ExtRemoteTyped(FillModule2("(%s!Js::MemoryLogger*)%s!Output::s_inMemoryLogger"));
    ULONG64 memoryLoggerAddr = memoryLogger.GetPtr();
    AutoBuffer<char16> reuseBuf;
    if (memoryLoggerAddr)
    {
        ULONG current = memoryLogger.Field("m_current").GetUlong(); // current is the slot to add next item.
        ULONG capacity = memoryLogger.Field("m_capacity").GetUlong();
        ExtRemoteTyped log = memoryLogger.Field("m_log");

        if (log[current].GetPtr())
        {
            // We have run across the bottom, so current is the 1st available log.
            // Print from current till end.
            for (ULONG i = current; i < capacity; ++i)
            {
                this->DumpStackTraceEntry(log[i].GetPtr(), reuseBuf);
            }
        }

        for (ULONG i = 0; i < current; ++i)
        {
            // Print from 0 till current - 1.
            this->DumpStackTraceEntry(log[i].GetPtr(), reuseBuf);
        }
    }
}

// The addr is address of a string (char16*) that contains addrs of functions on stack delimited by space.
void EXT_CLASS_BASE::DumpStackTraceEntry(ULONG64 addr, AutoBuffer<char16>& buf)
{
    const char16 callStackPrefix[] = _u("call stack:");
    ULONG charCount = _countof(callStackPrefix);
    buf.EnsureCapacity(charCount);

    ExtRemoteData entry(addr, charCount);
    entry.GetString(buf, charCount);
    if (!wcsncmp(callStackPrefix, buf, charCount - 1))
    {
        // Parse stack trace.
        Out(_u("%s\n"), callStackPrefix);
        ULONG neededChars;
        entry.GetString((char16*)NULL, 0, ULONG_MAX, false, &neededChars);
        buf.EnsureCapacity(neededChars);
        entry.GetString(buf, neededChars);

        char16* context;
        const char16* delims = _u(" \n");
        char16* tok = wcstok_s(buf + wcslen(callStackPrefix), delims, &context);
        while (tok)
        {
            char16* endptr;
            ULONG64 ip = _wcstoui64(tok, &endptr, 16);
            if (!*endptr && ip != _UI64_MAX)
            {
                WCHAR nameBuffer[255];
                ULONG nameSize;
                CComQIPtr<IDebugSymbols3> symbols3(m_Symbols);
                HRESULT hr = symbols3->GetNameByOffsetWide(ip, nameBuffer, sizeof(nameBuffer), &nameSize, NULL);
                if (SUCCEEDED(hr))
                {
                    Out(_u("  %p %s\n"), ip, nameBuffer);
                }
                else
                {
                    Out(_u("  %p\n"), ip);
                }
            }
            tok = wcstok_s(NULL, delims, &context);
        }
    }
    else
    {
        // String w/o stack, just print it.
        Out(_u("%mu"), addr);
    }
}

void EXT_CLASS_BASE::Uninitialize()
{
    // We must release these references on cleanup. Extension is a global instance itself,
    // releasing in global destructor is too late.
    m_jsDebug.Release();
    m_jsFrame.Release();
}

void EXT_CLASS_BASE::Out(_In_ PCSTR fmt, ...)
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

void EXT_CLASS_BASE::Out(_In_ PCWSTR fmt, ...)
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

USE_DbgEngDataTarget(); // Use DbgEngDataTarget implementation

void EXT_CLASS_BASE::CreateStackWalker(IJsDebugStackWalker** ppStackWalker)
{
    EnsureJsDebug();

    CComPtr<IJsDebugProcess> debugProcess;
    CreateDebugProcess(&debugProcess);
    DWORD threadId;
    IfFailThrow(m_System->GetCurrentThreadSystemId(&threadId));
    IfFailThrow(debugProcess->CreateStackWalker(threadId, ppStackWalker));

    // Created a new stack walker. Release last stack frame object.
    m_jsFrame.Release();
    m_jsFrameNumber = -1;
}

void EXT_CLASS_BASE::CreateDebugProcess(IJsDebugProcess** ppDebugProcess)
{
    CComPtr<DbgEngDataTarget> dataTarget;
    IfFailThrow(CreateComObject(&dataTarget));
    IfFailThrow(dataTarget->Init(m_Client));

    DWORD processId;
    IfFailThrow(m_System->GetCurrentProcessSystemId(&processId));

    UINT64 baseAddress; ULONG index;
    IfFailThrow(FindJScriptModuleByName(m_Symbols, &index, &baseAddress));

    // Skip validateDebugMode, so that a user can always decode the script stack. (Subsequent locals inspection could fail.)
    IfFailThrow(m_jsDebug->OpenVirtualProcess(dataTarget, /*validateDebugMode*/false, processId, baseAddress, NULL, ppDebugProcess));
}

void EXT_CLASS_BASE::Print(IJsDebugProperty* prop, int radix, int maxDepth)
{
    PCWSTR fmt = _u("%0s%-24s %-16s %-16s\n");
    Out(fmt, _u(""), _u("#Name"), _u("#Value"), _u("#Type"));
    Print(prop, fmt, radix, 0, maxDepth);
}

static void JoinLines(BSTR& bstr)
{
    if (wcschr(bstr, _u('\n')))
    {
        std::wstring str(bstr, ::SysStringLen(bstr));
        std::wstring::size_type i = 0;

        while ((i = str.find_first_of(_u("\n\r"), i)) != std::wstring::npos)
        {
            if (str[i] == _u('\n'))
            {
                str.replace(i, 1, _u("\\n"));
            }
            else
            {
                str.replace(i, 1, _u(""));
            }
        }

        ::SysFreeString(bstr);
        bstr = ::SysAllocStringLen(str.c_str(), (UINT)str.length());
    }
}

void EXT_CLASS_BASE::Print(IJsDebugProperty* property, PCWSTR format, int radix, int depth, int maxDepth)
{
    AutoJsDebugPropertyInfo info;
    IfFailThrow(property->GetPropertyInfo(radix, &info));    

    JoinLines(info.value); // Join Value lines
    Out(format, _u(""), info.name, info.value, info.type);

    if ((info.attr & JS_PROPERTY_ATTRIBUTES::JS_PROPERTY_HAS_CHILDREN) && depth++ < maxDepth)
    {
        CComPtr<IJsEnumDebugProperty> pEnum;
        IfFailThrow(property->GetMembers(JS_PROPERTY_MEMBERS::JS_PROPERTY_MEMBERS_ALL, &pEnum));

        if (!pEnum)
        {
            return;
        }

        WCHAR fmt[32];
        {
            const int NAME_FIELD_LEN = 24;
            const int indent = depth * 4;
            swprintf_s(fmt, _u("%%%ds%%-%ds"), indent, NAME_FIELD_LEN - indent);
            wcscat_s(fmt, _u(" %-16s %-16s\n"));
        }

        ULONG claimedCount = 0, actualCount = 0;
        IfFailThrow(pEnum->GetCount(&claimedCount));

        for(;;)
        {
            CComPtr<IJsDebugProperty> prop;
            ULONG count;

            IfFailThrow(pEnum->Next(1, &prop, &count));
            if (count == 0)
            {
                break;
            }

            Assert(count == 1);
            ++actualCount;

            Print(prop, fmt, radix, depth, maxDepth);
        }

        AssertMsg(actualCount == claimedCount, "GetCount() mismatches real property count");
    }
}


ULONG64 EXT_CLASS_BASE::GetEnumValue(const char* enumName, bool useMemoryNamespace, ULONG64 default)
{
    char buf[MAX_PATH];
    char const * prefix = useMemoryNamespace ? this->FillModuleAndMemoryNS("%s!%s") : this->FillModule("%s!");
    sprintf_s(buf, "@@c++(%s%s)", prefix, enumName);
    try
    {
        return this->EvalExprU64(buf);
    }
    catch (...)
    {
        return default;
    }
}

class Module: public CAtlDllModuleT<Module> {} _Module;
