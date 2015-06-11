//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//
// This tool reads in Window RT metadata files, projects into TypeScript shape and
// outputs stubbed .d.ts files for use by the language service.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include <metahost.h>
#include <sdkddkver.h>
#include "tsfiledumper.h"

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#define NTDDI_VERSION NTDDI_WIN8
#include <rometadata.h>

/*
    This is the starting point for TSGen
*/
int _cdecl wmain(int argc, __in_ecount(argc) LPWSTR argv[])
{
    try{
        ULONG stackGuarantee = 16 * 1024;
        SetThreadStackGuarantee(&stackGuarantee);
        HRESULT hr = S_OK;
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        ThrowIfFalse(SUCCEEDED(hr), L"Couldn't initialize thread information");
        BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
        {
            PageAllocator pageAllocator(nullptr);
            ArenaAllocator alloc(L"TsGen", &pageAllocator, Js::Throw::OutOfMemory);
            Settings settings;

            Settings::ApplyFlagsFromCommandLine(argc, argv, &settings, &alloc);

            if (settings.responseFile)
            {
                Settings::ApplyFlagsFromResponseFile(settings.responseFile, &settings, &alloc);
            }

            IMetaDataDispenser* dispenser = nullptr;
            hr = ::MetaDataGetDispenser(CLSID_CorMetaDataDispenser, IID_IMetaDataDispenser, (void**)&dispenser);
            ThrowIfFalse(SUCCEEDED(hr), L"Internal Error - Couldn't initialize the dispenser");
            Wrapper::MetadataContext resolver(&alloc, dispenser, settings.winmds, settings.references, settings.enableVersioningAllAssemblies);
            ProjectionModel::ProjectionBuilder builder(&resolver, &resolver, &alloc, settings.targetPlatformVersion);
            
            TSWrapper::TSObjectModel objectModel(&alloc, &resolver, &settings, &builder);

            TSFileDumper dumper = TSFileDumper();
            dumper.Dump(&objectModel);
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr)
        if (FAILED(hr))
        {
            ThrowIfFalse(SUCCEEDED(hr), L"Internal Error");
        }
    }
    catch (TSGenException& e)
    {
        wprintf(L"%s", e.what());
    }

    return 0;
}