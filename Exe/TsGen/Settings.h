//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TSGenException.h"

using namespace regex;

enum struct OutputType
{
    LanguageService,
    DumpCallPatterns
};

enum struct SettingsState
{
    Initial,
    NextIsNamespace,
    NextIsReference,
    NextIsPlatformVersion
};

class Settings
{
public:
    ImmutableList<LPCWSTR>* winmds;
    ImmutableList<LPCWSTR>* references;
    DWORD targetPlatformVersion;
    LPWSTR outputFile;
    OutputType outputType;
    LPWSTR rootNamespace;
    LPCWSTR responseFile;
    SettingsState state;
    bool enableVersioningAllAssemblies;
    ImmutableList<LPCWSTR>* xmlDocumentationFiles;

    Settings()
        : winmds(nullptr),
        references(nullptr),
        outputFile(L"con"),
        outputType(OutputType::LanguageService),
        rootNamespace(L"this"),
        responseFile(nullptr),
        targetPlatformVersion(0xFFFFFFFF),
        state(SettingsState::Initial),
        enableVersioningAllAssemblies(false),
        xmlDocumentationFiles(nullptr)
    { }

    static void ApplyFlagsFromResponseFile(_In_ LPCWSTR responseFile, _Inout_ Settings* settings, _Inout_ ArenaAllocator* alloc);
    static void ApplyFlagsFromCommandLine(int argc, __in_ecount(argc) LPWSTR argv[], _Inout_ Settings* settings, _Inout_ ArenaAllocator* alloc);
};