//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "MetadataResolver.h"
#include "SortProjection.h"
#include "ProjectionToTypeScriptConverter.h"
#include "CommandLineReader.h"
#include "WinRt2TsErrors.h"

using namespace Microsoft::WRL;
using namespace std;
using namespace Metadata;
using namespace ProjectionModel;

static StringConverter g_converter;
IStringConverter& MetadataString::s_stringConverter = g_converter;

void GenerateTsFromWinmds(IMetaDataDispenserEx* dispenser, const Configuration& config)
{
    PageAllocator pageAllocator(NULL, Js::Configuration::Global.flags);
    ArenaAllocator alloc(L"WinRt2Ts", &pageAllocator, Js::Throw::OutOfMemory);

    MetadataResolver resolver(&alloc, &MetadataString::s_stringConverter, dispenser, config.winmds, config.enableVersioningAllAssemblies, config.enableVersioningWindowsAssemblies);
    ProjectionBuilder builder(&resolver, &MetadataString::s_stringConverter, &alloc, NTDDI_WINTHRESHOLD, false);

    Option<AssignmentSpace> currentAssignmentSpace;
    for (auto assembly : resolver.assemblies)
    {
        currentAssignmentSpace = builder.AddFromMetadataImport(currentAssignmentSpace, assembly);
    }

    if (currentAssignmentSpace.HasValue())
    {
        currentAssignmentSpace = SortAssignmentSpace(currentAssignmentSpace.GetValue(), &alloc, &MetadataString::s_stringConverter);

        wostream* outputStream = nullptr;
        auto_ptr<wofstream> fileStream;
        if (!config.outFilePath.empty())
        {
            fileStream.reset(new wofstream(config.outFilePath.c_str(), ios::out));
            outputStream = fileStream.get();
        }
        else
        {
            outputStream = &wcout;
        }

        IndentingWriter writer(*outputStream);
        TypeScriptEmitter tsEmitter(writer, MetadataString::s_stringConverter);
        ProjectionToTypeScriptConverter emitter(&alloc, tsEmitter, writer, MetadataString::s_stringConverter, config.emitAnyForUnresolvedTypes);

        currentAssignmentSpace.GetValue()->vars->Iterate([&](RtASSIGNMENT var) {
            emitter.EmitTopLevelNamespace(MetadataString::s_stringConverter.IdOfString(var->identifier), PropertiesObject::From(var->expr));
        });
    }
}

void RunWinRt2Ts(const Configuration& config)
{
#if DBG
    AllowHeavyOperation allow;
#endif

    ComPtr<IMetaDataDispenserEx> dispenser;
    HRESULT hr = MetaDataGetDispenser(CLSID_CorMetaDataDispenser, IID_IMetaDataDispenserEx, reinterpret_cast<void **>(dispenser.GetAddressOf()));

    if (FAILED(hr))
    {
        throw MetadataDispenserError(hr);
    }

    // The macros below define a try/catch structure that converts all exceptions to appropriate HRESULTs,
    // unless they derive from ErrorBase, in which case they are simply rethrown.
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT

    GenerateTsFromWinmds(dispenser.Get(), config);

    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr)
    catch (ErrorBase&)
    {
        // Delete the output file as it will not be well formed at this point
        DeleteFile(config.outFilePath.c_str());

        throw;
    }
    CATCH_UNHANDLED_EXCEPTION(hr)

    if (FAILED(hr))
    {
        throw UnknownError(hr);
    }
}

int __cdecl wmain(int argc, wchar_t* argv[])
{
    try
    {
        CommandLineReader reader;
        Configuration config = reader.Read(argc, argv);

        RunWinRt2Ts(config);
    }
    catch (ErrorBase& e)
    {
        wcerr << L"error " << e.Code() << L": " << e.Description() << endl;
        return 0 - e.Code();
    }

    return 0;
}