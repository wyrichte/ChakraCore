//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#include "..\..\Lib\Common\core\ICustomConfigFlags.h"

class HostConfigFlags : public ICustomConfigFlags
{
public:
#define FLAG(Type, Name, Desc, Default) \
    Type Name;      \
    bool Name##IsEnabled;
#include "HostConfigFlagsList.h"

    static HostConfigFlags flags;
    static LPWSTR* argsVal;
    static PCWSTR jdtestCmdLine;
    static PCWSTR jsEtwConsoleCmdLine;
    static int argsCount;    
    static void (__stdcall *pfnPrintUsage)();

    static int FindArg(int argc, _In_reads_(argc) PWSTR argv[], PCWSTR targetArg, int targetArgLen);
    static void RemoveArg(int& argc, _Inout_updates_(argc) PWSTR argv[], int index);
    static PCWSTR ExtractSwitch(int& argc, _Inout_updates_(argc) PWSTR argv[], PCWSTR switchNameWithColon, int switchNameWithColonLen);
	static void AddSwitch(int& argc, __deref_ecount(argc) LPWSTR** argv, _In_ PWSTR newArg);
    static int PeekVersionSwitch(int argc, _In_reads_(argc) PWSTR argv[]);

    template <class Func> static int FindArg(int argc, _In_reads_(argc) PWSTR argv[], Func func);
    template <int LEN> static int FindArg(int argc, _In_reads_(argc) PWSTR argv[], const wchar_t(&targetArg)[LEN]);
    template <int LEN> static PCWSTR ExtractSwitch(int& argc, _Inout_updates_(argc) PWSTR argv[], const wchar_t (&switchNameWithColon)[LEN]);

    static void HandleArgsFlag(int& argc, __in_ecount(argc) LPWSTR argv[]);
    static void HandleJdTestFlag(int& argc, __in_ecount(argc) LPWSTR argv[]);
    static void HandleJsEtwConsoleFlag(int& argc, __in_ecount(argc) LPWSTR argv[]);

    virtual bool ParseFlag(LPCWSTR flagsString, ICmdLineArgsParser * parser) override;
    virtual void PrintUsage() override;
    static void PrintUsageString();

    static bool IsHybridDebugging()
    {
        bool isHybridDebugging = _wcsicmp(HostConfigFlags::flags.DebugLaunch, L"hybrid") == 0;
        if(isHybridDebugging)
        {
            Assert(IsDebuggerPresent());
        }
        return isHybridDebugging;
    }
private:
    int nDummy;
    HostConfigFlags();

    template <typename T>
    void Parse(ICmdLineArgsParser * parser, T * value);
};

// Find an arg in the arg list that satisfies func. Return the arg index if found.
template <class Func>
int HostConfigFlags::FindArg(int argc, _In_reads_(argc) PWSTR argv[], Func func)
{
    for (int i = 1; i < argc; ++i)
    {
        if (func(argv[i]))
        {
            return i;
        }
    }

    return -1;
}

template <int LEN>
int HostConfigFlags::FindArg(int argc, _In_reads_(argc) PWSTR argv[], const wchar_t (&targetArg)[LEN])
{
    return FindArg(argc, argv, targetArg, LEN - 1); // -1 to exclude null terminator
}    

template <int LEN>
PCWSTR HostConfigFlags::ExtractSwitch(int& argc, _Inout_updates_(argc) PWSTR argv[], const wchar_t (&switchNameWithColon)[LEN])
{
    return ExtractSwitch(argc, argv, switchNameWithColon, LEN - 1); // -1 to exclude null terminator
}
