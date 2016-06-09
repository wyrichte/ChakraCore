//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include <HostCommonPch.h>

HostConfigFlags HostConfigFlags::flags;
LPWSTR* HostConfigFlags::argsVal;
PCWSTR HostConfigFlags::jsEtwConsoleCmdLine = nullptr;
int HostConfigFlags::argsCount;
void (__stdcall *HostConfigFlags::pfnPrintUsage)();

template <>
void HostConfigFlags::Parse<bool>(ICmdLineArgsParser * parser, bool * value)
{
    *value = parser->GetCurrentBoolean();
}

template <>
void HostConfigFlags::Parse<int>(ICmdLineArgsParser * parser, int* value)
{
    try
    {
        *value = parser->GetCurrentInt();
    }
    catch (...)
    {
        // Not doing anything, the *value will remain the default one.
    }
}

template <>
void HostConfigFlags::Parse<BSTR>(ICmdLineArgsParser * parser,  BSTR * bstr)
{
    if (*bstr != NULL)
    {
        SysFreeString(*bstr);
    }
    *bstr = parser->GetCurrentString();
    if(*bstr == NULL)
    {
        *bstr = SysAllocString(_u(""));
    }
}

template <>
void HostConfigFlags::ParseAsDefault<bool>(ICmdLineArgsParser * parser, bool * value)
{
    *value = true;
}

template <>
void HostConfigFlags::ParseAsDefault<int>(ICmdLineArgsParser * parser, int* value)
{
    // No-op
}

template <>
void HostConfigFlags::ParseAsDefault<BSTR>(ICmdLineArgsParser * parser, BSTR * bstr)
{
    if (*bstr == NULL)
    {
        *bstr = SysAllocString(L"");
    }
}

HostConfigFlags::HostConfigFlags()  :
#define FLAG(Type, Name, Desc, Default) \
    Name##IsEnabled(false), \
    Name(Default),
#include "HostConfigFlagsList.h"
    nDummy(0)
{
}

bool HostConfigFlags::ParseFlag(LPCWSTR flagsString, ICmdLineArgsParser * parser) 
{
#define FLAG(Type, Name, Desc, Default) \
    if (_wcsicmp(_u(#Name), flagsString) == 0) \
    { \
        this->Name##IsEnabled = true; \
        Parse<Type>(parser, &this->Name); \
        return true; \
    }
#define FLAGA2(Acronym, Name1, Type1, Name2, Type2, Desc) \
    if (_wcsicmp(L ## #Acronym, flagsString) == 0) \
    { \
        this->Name1##IsEnabled = true; \
        ParseAsDefault<Type1>(parser, &this->Name1); \
        this->Name2##IsEnabled = true; \
        ParseAsDefault<Type2>(parser, &this->Name2); \
        return true; \
    }
#include "HostConfigFlagsList.h"
    return false;
}

void HostConfigFlags::PrintUsageString()
{
#define FLAG(Type, Name, Desc, Default) \
    wprintf(_u("%20ls          \t%ls\n"), _u(#Name), _u(#Desc));
#include "HostConfigFlagsList.h"
}

void HostConfigFlags::PrintUsage()
{
    if (pfnPrintUsage)
    {
        pfnPrintUsage();
    }
    HostConfigFlags::PrintUsageString();
    if (JScript9Interface::SupportsPrintConfigFlagsUsageString())
    {
        JScript9Interface::PrintConfigFlagsUsageString();
    }
}

int HostConfigFlags::FindArg(int argc, _In_reads_(argc) PWSTR argv[], PCWSTR targetArg, size_t targetArgLen)
{
    return FindArg(argc, argv, [=](PCWSTR arg) -> bool
    {
        return _wcsnicmp(arg, targetArg, targetArgLen) == 0;
    });
}

void HostConfigFlags::RemoveArg(int& argc, _Inout_updates_to_(argc, argc) LPWSTR argv[], int index)
{
    Assert(index >= 0 && index < argc);
    for (int i = index + 1; i < argc; ++i)
    {
        argv[i-1] = argv[i];
    }
    --argc;
}

void HostConfigFlags::AddSwitch(int& argc, _Inout_updates_to_(argc, argc) LPWSTR*& argv, _In_ PWSTR newArg)
{
    PWSTR* tempArray = new PWSTR[argc + 1];
    memcpy(tempArray, argv, sizeof(PWSTR) * argc);
    tempArray[argc] = newArg;
    argv = tempArray;
    argc++;
}

PCWSTR HostConfigFlags::ExtractSwitch(int& argc, _Inout_updates_to_(argc, argc) PWSTR argv[], PCWSTR switchNameWithColon, int switchNameWithColonLen)
{
    PCWSTR switchValue = nullptr;

    int i = FindArg(argc, argv, switchNameWithColon, switchNameWithColonLen);
    if (i >= 0)
    {
        switchValue = argv[i] + switchNameWithColonLen;
        RemoveArg(argc, argv, i);
    }

    return switchValue;
}

// Peek the full command line for "-version:" switch, so that the host can take action on desired script version.
//
int HostConfigFlags::PeekVersionSwitch(int argc, _In_reads_(argc) PWSTR argv[])
{
    int version = 0;

    const WCHAR versionSwitch[] = _u("-version:");
    int i = FindArg(argc, argv, versionSwitch);
    if (i < 0)
    {
        i = FindArg(argc, argv, _u("/version:"));
    }

    if (i >= 0)
    {
        PCWSTR versionString = argv[i] + _countof(versionSwitch) - 1; // -1 to exclude null terminator
        version = _wtoi(versionString); // result is 0 if fails
    }

    return version;
}

void HostConfigFlags::HandleJsEtwConsoleFlag(int& argc, _Inout_updates_to_(argc, argc) LPWSTR argv[])
{
    jsEtwConsoleCmdLine = ExtractSwitch(argc, argv, _u("-JsEtwConsole:"));
}

void HostConfigFlags::HandleArgsFlag(int& argc, _Inout_updates_to_(argc, argc) LPWSTR argv[])
{
    const LPCWSTR argsFlag = _u("-args");
    const LPCWSTR endArgsFlag = _u("-endargs");
    int argsFlagLen = static_cast<int>(wcslen(argsFlag));
    int i;
    for (i=1; i < argc; i++)
    {
        if (_wcsnicmp(argv[i], argsFlag, argsFlagLen) == 0)
        {
            break;
        }
    }
    int argsStart = ++i;
    for (; i < argc; i++)
    {
        if (_wcsnicmp(argv[i], endArgsFlag, argsFlagLen) == 0)
        {
            break;
        }
    }
    int argsEnd = i;

    int argsCount = argsEnd - argsStart;
    if (argsCount == 0)
    {
        return;
    }    
    HostConfigFlags::argsVal = new LPWSTR[argsCount];
    HostConfigFlags::argsCount = argsCount;
    int argIndex = argsStart;
    for (i = 0; i < argsCount; i++)
    {
        HostConfigFlags::argsVal[i] = argv[argIndex++];
    }      

    argIndex = argsStart - 1;
    for (i = argsEnd + 1; i < argc; i++)
    {
        argv[argIndex++] = argv[i];
    }
    Assert(argIndex == argc - argsCount - 1 - (argsEnd < argc));
    argc = argIndex;
}
