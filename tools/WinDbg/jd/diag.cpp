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
