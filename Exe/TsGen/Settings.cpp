//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "settings.h"

// Info:        Apply the given flag to the settings structure
// Parameter:   flag - command-line or response file flag
//              settings - the settings structure to mutate
void ApplyFlag(_In_ LPCWSTR flag, _Inout_ Settings* settings, _Inout_ ArenaAllocator* alloc)
{
    if (settings->state == SettingsState::NextIsNamespace)
    {
        settings->rootNamespace = _wcsdup(flag);
        settings->state = SettingsState::Initial;
        wprintf(L"<alternate-root-namespace namespace='%s'/>\n", settings->rootNamespace);
    }
    else if (settings->state == SettingsState::NextIsReference)
    {
        settings->references = settings->references->Prepend(_wcsdup(flag), alloc);
        settings->state = SettingsState::Initial;
    }
    else if (settings->state == SettingsState::NextIsPlatformVersion)
    {
        if (wcscmp(flag, L"8.1") == 0)
        {
            settings->targetPlatformVersion = NTDDI_WINBLUE;
        }
        else if (wcscmp(flag, L"8.0") == 0)
        {
            settings->targetPlatformVersion = NTDDI_WIN8;
        }
        else
        {
            settings->targetPlatformVersion = 0xFFFFFFFF;
        }
        settings->state = SettingsState::Initial;
    }
    else if (_wcsicmp(flag, L"-rootNamespace") == 0)
    {
        settings->state = SettingsState::NextIsNamespace;
    }
    else if (_wcsicmp(flag, L"-r") == 0)
    {
        settings->state = SettingsState::NextIsReference;
    }
    else if (_wcsicmp(flag, L"-dumpCallPatterns") == 0)
    {
        wprintf(L"<dumping-call-patterns/>\n");
        settings->outputType = OutputType::DumpCallPatterns;
    }
    else if (StringUtils::EndsWith(flag, L".winmd"))
    {
        settings->winmds = settings->winmds->Prepend(_wcsdup(flag), alloc);
    }
    else if (flag[0] == L'@')
    {
        settings->responseFile = _wcsdup(flag + 1);
    }
    else if (_wcsicmp(flag, L"-targetPlatformVersion") == 0)
    {
        settings->state = SettingsState::NextIsPlatformVersion;
    }
    else if (_wcsicmp(flag, L"-enableVersioningAllAssemblies") == 0)
    {
        settings->enableVersioningAllAssemblies = true;
    }
    else if (StringUtils::EndsWith(flag, L".xml"))
    {
        settings->xmlDocumentationFiles = settings->xmlDocumentationFiles->Prepend(_wcsdup(flag), alloc);
    }
    else
    {
        settings->outputFile = _wcsdup(flag);
    }
}

// Info:        Apply the given flags to the settings structure
// Parameter:   argc - count of flags
//              argv - array of flags
//              settings - the settings structure to mutate
void Settings::ApplyFlagsFromCommandLine(int argc, __in_ecount(argc) LPWSTR argv[], _Inout_ Settings* settings, _Inout_ ArenaAllocator* alloc)
{
    for (int i = 1; i < argc; ++i)
    {
        ApplyFlag(argv[i], settings, alloc);
    }
}

// Info:        Read a response file.
//              Response file has a very specific format: must be utf8, must have one flag per line.
// Parameter:   argc - count of flags
//              argv - array of flags
//              settings - the settings structure to mutate
void Settings::ApplyFlagsFromResponseFile(_In_ LPCWSTR responseFile, _Inout_ Settings* settings, _Inout_ ArenaAllocator* alloc)
{
    FILE* file;
    auto error = _wfopen_s(&file, responseFile, L"r,ccs=UTF-8");
    Assert(error == 0);
    ThrowIfFalse(file != nullptr, L"Couldn't open response file");
    
    wchar_t line[MAX_PATH + 1];
    settings->state = SettingsState::Initial;
    while (fgetws(line, MAX_PATH, file))
    {
        StringUtils::TrimTrailingWhiteSpace(line);
        ApplyFlag(line, settings, alloc);
    }

    fclose(file);
}
