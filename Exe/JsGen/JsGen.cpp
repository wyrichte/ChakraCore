//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//
// This tool reads in Window RT metadata files, projects into JavaScript shape and
// outputs stubbed .js files for use by the language service.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "JavaScriptStubDumper.h"
#include <metahost.h>
#include <sdkddkver.h>

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#define NTDDI_VERSION NTDDI_WIN8
#include <rometadata.h>

using namespace Js;
using namespace regex;
using namespace ProjectionModel;

// Info:        Exit immediately if the condistion is false
// Parameter:   condition - check this
//              code - the exit code to exit with
#define ExitIfFalse(condition, code) \
    if (!(condition)) { wprintf(L"<unexpected-condition code='%d'/>", code); exit(code); }

void EtwCallbackApi::OnSessionChange(ULONG /*controlCode*/, PVOID /*callbackContext*/)
{
    // Stub method to satisfy the linker. If we need ETW rundown in JsGen, either link with
    // jscript.runtime.lib or move the code for OnSessionChange in EtwTrace.cpp out to a lib
    // that links with jsgen
}

// Info:        Import a particular assembly from the dispenser
// Parameter:   dispenser - metadata dispenser
//              filename - filename of the assembly to read
// Return:      the importer
IMetaDataImport2* ImportAssembly(IMetaDataDispenser * dispenser, LPCWSTR filename)
{
    // TODO: Use factored out metadata reader
    IMetaDataImport2 * import;

    auto hr = dispenser->OpenScope(filename, (ofRead | ofNoTransform), IID_IMetaDataImport2, (LPUNKNOWN*)&import); 
    if (FAILED(hr))
    {
        UINT oldcp = GetConsoleOutputCP();

        if (0 != SetConsoleOutputCP(CP_UTF8))
        {
#pragma warning(disable: 38021) // From MSDN: For the code page UTF-8 dwFlags must be set to either 0 or WC_ERR_INVALID_CHARS. Otherwise, the function fails with ERROR_INVALID_FLAGS.
            int bufferSize = WideCharToMultiByte(CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL);
            char* fileNameUTF8 = new char[bufferSize];
            WideCharToMultiByte(CP_UTF8, 0, filename, -1, fileNameUTF8, bufferSize, NULL, NULL);
#pragma warning(default: 38021) // From MSDN: For the code page UTF-8 dwFlags must be set to either 0 or WC_ERR_INVALID_CHARS. Otherwise, the function fails with ERROR_INVALID_FLAGS.
            wprintf(L"<ignored-missing-metadatafile filename='%S'/>\n", fileNameUTF8);
            delete[] fileNameUTF8;
        }
        else
        {
            wprintf(L"<ignored-missing-metadatafile filename='%S'/>\n", filename);
        }

        SetConsoleOutputCP(oldcp);

        return nullptr;
    }
    return import;
}

// Info:        Determine whether a string ends with a particular suffix
// Parameter:   str - the string to check
//              suffix - the suffix to check for
bool EndsWith(LPCWSTR str, LPCWSTR suffix)
{
    auto strLen = wcslen(str);
    auto suffixLen = wcslen(suffix);
    auto strSuffix = str + (strLen - suffixLen);
    return _wcsnicmp(strSuffix,suffix,suffixLen)==0;
}

// Info:        Return true if ch contains a whitespace char
// Parameter:   ch - the string to check
bool IsWhiteSpaceChar(wchar_t ch)
{
    // This list doesn't need to be exhaustive. We only communicate with the language service
    return ch==0x0a || ch==0x0d || ch==' ' || ch=='\t';
}

// Info:        Return true if ch contains a whitespace char
// Parameter:   ch - the string to check
void TrimTrailingWhiteSpace(__in_ecount(MAX_PATH+1) wchar_t * str)
{
    auto pos = wcslen(str)-1;
    while(pos>0 && IsWhiteSpaceChar(str[pos]))
    {
        str[pos]=0x00;
        --pos;
    }
}

enum OutputType
{
    otLanguageService,
    otDumpCallPatterns
};

enum SettingsState
{
    ssInitial,
    ssNextIsNamespace,
    ssNextIsReference,
    ssNextIsPlatformVersion
};

struct Settings
{
    ImmutableList<LPCWSTR> * winmds;
    ImmutableList<LPCWSTR> * references;
    DWORD targetPlatformVersion;
    LPWSTR outputFile;
    OutputType outputType;
    LPWSTR rootNamespace;
    LPCWSTR responseFile;
    SettingsState state;
    bool enableVersioningAllAssemblies;
    Settings() 
        : winmds(nullptr), 
          references(nullptr),
          outputFile(L"con"),
          outputType(otLanguageService),
          rootNamespace( L"this"),
          responseFile(nullptr),
          targetPlatformVersion(0xFFFFFFFF),
          state(ssInitial),
          enableVersioningAllAssemblies(false)
    { }
};

// Info:        Apply the given flag to the settings structure
// Parameter:   flag - command-line or response file flag
//              settings - the settings structure to mutate
void ApplyFlag(LPCWSTR flag, Settings * settings, ArenaAllocator * alloc)
{
    if (settings->state == ssNextIsNamespace)
    {
        settings->rootNamespace = _wcsdup(flag);
        settings->state = ssInitial;
        wprintf(L"<alternate-root-namespace namespace='%s'/>\n", settings->rootNamespace);

    } else if (settings->state == ssNextIsReference)
    {
        settings->references = settings->references->Prepend(_wcsdup(flag),alloc);
        settings->state = ssInitial;
    } else if (settings->state == ssNextIsPlatformVersion)
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
        settings->state = ssInitial;
    } else if(wcscmp(flag, L"-rootNamespace") == 0)
    {
        settings->state = ssNextIsNamespace;
    } else if(wcscmp(flag, L"-r") == 0)
    {
        settings->state = ssNextIsReference;
    } else if(wcscmp(flag, L"-dumpCallPatterns") == 0)
    {
        wprintf(L"<dumping-call-patterns/>\n");
        settings->outputType = otDumpCallPatterns;
    } 
    else if (EndsWith(flag,L".winmd"))
    {
        settings->winmds = settings->winmds->Prepend(_wcsdup(flag),alloc);
    } else if (flag[0]==L'@')
    {
        settings->responseFile = _wcsdup(flag+1);
    } else if (wcscmp(flag, L"-targetPlatformVersion") == 0)
    {
        settings->state = ssNextIsPlatformVersion;
    } else if (wcscmp(flag, L"-enableVersioningAllAssemblies") == 0)
    {
        settings->enableVersioningAllAssemblies = true;
    } else
    {
        settings->outputFile = _wcsdup(flag);
    } 
}
// Info:        Apply the given flags to the settings structure
// Parameter:   argc - count of flags
//              argv - array of flags
//              settings - the settings structure to mutate
void ApplyFlagsFromCommandLine(int argc, __in_ecount(argc) LPWSTR argv[], Settings * settings, ArenaAllocator * alloc)
{    
    for(int i = 1; i<argc; ++i)
    {
        ApplyFlag(argv[i], settings, alloc);
    }
}

// Info:        Read a response file.
//              Response file has a very specific format: must be utf8, must have one flag per line.
// Parameter:   argc - count of flags
//              argv - array of flags
//              settings - the settings structure to mutate
void ApplyFlagsFromResponseFile(LPCWSTR responseFile, Settings * settings, ArenaAllocator * alloc)
{
    FILE * file;
    auto error = _wfopen_s(&file,responseFile,L"r,ccs=UTF-8");
    //Ensure file was opened successfully
    Assert(error == 0);
    ExitIfFalse(file!=nullptr,-1001);
    wchar_t line[MAX_PATH+1];
    settings->state = ssInitial;
    while(fgetws(line,MAX_PATH,file))
    {
        TrimTrailingWhiteSpace(line);
        ApplyFlag(line,settings,alloc);
    }
}

struct MetadataContext : ProjectionModel::ITypeResolver, Metadata::IStringConverter
{
    ArenaAllocator * alloc;
    IMetaDataDispenser * dispenser;
    typedef JsUtil::BaseDictionary<MetadataStringId, Metadata::Assembly*, ArenaAllocator, PrimeSizePolicy> TMetadata;
    typedef JsUtil::BaseDictionary<MetadataStringId, Metadata::TypeDefProperties*, ArenaAllocator, PrimeSizePolicy> TTypeDefs;
    typedef JsUtil::BaseDictionary<WCHAR*, MetadataStringId, ArenaAllocator, PrimeSizePolicy> TStringToProjectionTypeId;
    typedef JsUtil::BaseDictionary<MetadataStringId, WCHAR*, ArenaAllocator, PrimeSizePolicy> TProjectionTypeIdToString;
    TMetadata * metadata;
    TTypeDefs * typeDefs;
    TStringToProjectionTypeId * stringToProjectionTypeId;
    TProjectionTypeIdToString * projectionTypeIdToString;
    ImmutableList<Metadata::Assembly*> * assemblies;
    ImmutableList<Metadata::Assembly*> * referenceAssemblies;
    Js::DelayLoadWinRtRoParameterizedIID roParameterizedIIDDelayLoad;
    MetadataStringId nextTypeId;
    bool enableVersioningAllAssemblies;
    
    MetadataContext(ArenaAllocator * alloc, IMetaDataDispenser * dispenser, ImmutableList<LPCWSTR> * winmds, ImmutableList<LPCWSTR> * references, bool enableVersioningAllAssemblies) 
        : alloc(alloc), dispenser(dispenser), assemblies(nullptr), referenceAssemblies(nullptr), nextTypeId(1000), enableVersioningAllAssemblies(enableVersioningAllAssemblies)
    {
        roParameterizedIIDDelayLoad.EnsureFromSystemDirOnly();

        metadata = Anew(alloc, TMetadata, alloc);
        typeDefs = Anew(alloc, TTypeDefs, alloc);
        stringToProjectionTypeId = Anew(alloc, TStringToProjectionTypeId, alloc, 0);
        projectionTypeIdToString = Anew(alloc, TProjectionTypeIdToString, alloc, 0);

        winmds->Iterate([&](LPCWSTR winmd) {
            auto key = IdOfString(winmd);
            if (!metadata->ContainsKey(key)) 
            {
                auto import = ImportAssembly(dispenser, winmd);
                if (import)
                {
                    auto assembly = Anew(alloc, Metadata::Assembly, import, this, alloc, IsVersionedAssembly(winmd));
                    metadata->Add(key, assembly);
                    assemblies = assemblies->Prepend(assembly,alloc);
                    referenceAssemblies = referenceAssemblies->Prepend(assembly,alloc);
                }
            }
        });
        references->Iterate([&](LPCWSTR winmd) {
            auto key = IdOfString(winmd);
            if (!metadata->ContainsKey(key)) 
            {
                auto import = ImportAssembly(dispenser, winmd);
                if (import)
                {
                    auto assembly = Anew(alloc, Metadata::Assembly, import, this, alloc, IsVersionedAssembly(winmd));
                    metadata->Add(key, assembly);
                    referenceAssemblies = referenceAssemblies->Prepend(assembly,alloc);
                }
            }
        });
    }


    // Info:        Resolve a type name to a type def
    // Parameter:   typeName - the type name
    //              typeDef - receives the typeDef
    // Return:      S_OK if the type was found, E_FAIL if not
    HRESULT ResolveTypeName(MetadataStringId typeId, LPCWSTR typeName, Metadata::TypeDefProperties ** typeDef) override
    {
        auto key = typeId;
        if(!typeDefs->TryGetValue(key,typeDef))
        {
            auto references = referenceAssemblies;
            while(references)
            {
                LPCWSTR typeDefName = typeName;
                if (!typeDefName)
                {
                    typeDefName = StringOfId(typeId);
                }
                auto reference = references->First();
                *typeDef = const_cast<Metadata::TypeDefProperties *>(reference->FindTopLevelTypeDefByName(typeDefName));
                if (*typeDef)
                {
                    VerifyCatastrophic((*typeDef)->id == key);
                    typeDefs->Add(key,*typeDef);
                    return S_OK;
                }
                references = references->GetTail();
            }

            // Error recovery path. Add null to dictionary so that we don't try to find this missing type again.
            typeDefs->Add(key,nullptr);
            return E_FAIL;
        }
        return *typeDef ? S_OK : E_FAIL;
    }

    // Info:        Get the DelayLoadWinRtRoParameterizedIID
    Js::DelayLoadWinRtRoParameterizedIID *GetRoParameterizedIIDDelayLoad() 
    { 
        return &roParameterizedIIDDelayLoad; 
    }

    // Info:        Get the string id for a given string.
    // Parameter:   sz - the string
    // Return:      The id of the string
    MetadataStringId IdOfString(LPCWSTR sz) 
    {
        auto key = const_cast<WCHAR*>(sz);
        MetadataStringId typeId = MetadataStringIdNil;
        if (sz) // nullptr <=> MetadataStringIdNil
        {
            if (!stringToProjectionTypeId->TryGetValue(key,&typeId))
            {
                typeId = nextTypeId;
                ++nextTypeId;
                size_t strLen = wcslen(sz) + 1;
                LPWSTR string = AnewArrayZ(alloc, wchar_t, strLen);
                wcscpy_s(string, strLen, sz);
                stringToProjectionTypeId->Add(string, typeId);
                projectionTypeIdToString->Add(typeId, string);
            }
        }
        return typeId;
    }

    // Info:        Get the string for a given string id.
    // Parameter:   id - the id
    // Return:      The string
    LPCWSTR StringOfId(MetadataStringId id)
    {
        WCHAR * sz = nullptr;
        if (id != MetadataStringIdNil)
        {
            projectionTypeIdToString->TryGetValue(id,&sz);
        }
        return sz;

    }

    // Info:        Determine whether this assembly should be versioned
    // Parameter:   assemblyName - the name of the assembly
    // Return:      Whether or not this assembly should be versioned
    bool IsVersionedAssembly(LPCWSTR assemblyName)
    {
        // If versioning is enabled for all assemblies, always return true
        if (enableVersioningAllAssemblies)
        {
            return true;
        }

        // Otherwise, versioning is only enabled for first-party Windows winmds (assemblies starting with "Windows.").
        LPCWSTR firstPartyWinmdPrefix = L"Windows.";
        size_t prefixCharCount = wcslen(firstPartyWinmdPrefix);

        if (wcslen(assemblyName) <= prefixCharCount)
        {
            return false;
        }

        return (_wcsnicmp(assemblyName, firstPartyWinmdPrefix, prefixCharCount) == 0);
    }
};
 
int _cdecl wmain2(int argc, __in_ecount(argc) LPWSTR argv[])
{
    ULONG stackGuarantee = 16*1024;
    SetThreadStackGuarantee(&stackGuarantee);        

    ExitIfFalse(SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)),-1005);

    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        PageAllocator pageAllocator(NULL, Js::Configuration::Global.flags);
        ArenaAllocator alloc(L"JsGen", &pageAllocator, Js::Throw::OutOfMemory);

        Settings settings;
        ApplyFlagsFromCommandLine(argc, argv, &settings, &alloc);

        if (settings.responseFile)
        {
            ApplyFlagsFromResponseFile(settings.responseFile, &settings, &alloc);
        }

        IMetaDataDispenser * dispenser = nullptr;
        hr = MetaDataGetDispenser(CLSID_CorMetaDataDispenser, IID_IMetaDataDispenser, (void**)&dispenser);
        ExitIfFalse(SUCCEEDED(hr), -1002);

        MetadataContext resolver(&alloc, dispenser, settings.winmds, settings.references, settings.enableVersioningAllAssemblies);
        ProjectionModel::ProjectionBuilder builder(&resolver, &resolver, &alloc, settings.targetPlatformVersion);
#if DBG
        ProjectionModel::AllowHeavyOperation allow;
#endif
        RtASSIGNMENTSPACE expr = nullptr;
        auto assemblies = resolver.assemblies;
        while(assemblies)
        {
            auto assembly = assemblies->First();
            expr = builder.AddFromMetadataImport(expr, assembly);
            assemblies = assemblies->GetTail();
        }

        // Write the file.
        if(expr)
        {
            bool isConsoleOut = (wcscmp(L"con",settings.outputFile)==0);
            wchar_t tempFile[MAX_PATH] = L"";
            ExitIfFalse(SUCCEEDED(StringCchCat(tempFile, MAX_PATH, settings.outputFile)),-1007);
            if(!isConsoleOut)
            {
                ExitIfFalse(SUCCEEDED(StringCchCat(tempFile, MAX_PATH, L".tmp")),-1008);
            }
            FILE * file;
            auto error = _wfopen_s(&file,tempFile,L"w,ccs=UTF-8");
            //Ensure file was opened successfully
            Assert(error == 0);

            auto sortedExpr = ProjectionModel::SortAssignmentSpace(expr,&alloc,&resolver);
            switch(settings.outputType)
            {
            case otLanguageService:
                ProjectionModel::JavaScriptStubDumper::Dump(builder, sortedExpr, settings.rootNamespace, file, &alloc);
                break;
            case otDumpCallPatterns:
                {
                    struct CompareRefGroupCount : public regex::Comparer<ImmutableList<LPCWSTR>**>
                    {
                        bool Equals(ImmutableList<LPCWSTR> ** v1, ImmutableList<LPCWSTR> ** v2)
                        {
                            return Compare(v1,v2) == 0;
                        }
                        int GetHashCode(ImmutableList<LPCWSTR> ** s)
                        {
                            Assert(0);
                            return 0;
                        }
                        int Compare(ImmutableList<LPCWSTR> ** v1, ImmutableList<LPCWSTR> ** v2)
                        {
                            Assert(v1 != nullptr && v2 != nullptr);
                            auto c1 = (*v1)->Count();
                            auto c2 = (*v2)->Count();
                            if (c1>c2)
                            {
                                return -1;
                            } else if (c1<c2)
                            {
                                return 1;
                            }
                            return 0;
                        }
                    };
                    CompareRefGroupCount countCompare;

                    ProjectionModel::ExtractSignatures extract(&alloc);
                    extract.VisitExpr(sortedExpr);
                    extract.signatures->SelectNotNull<LPCWSTR>([&](RtABIMETHODSIGNATURE signature)->LPCWSTR {
                        auto pattern = signature->GetParameters()->callPattern;
                        if (wcsstr(pattern,L"uncallable")) // Remove generics
                        {
                            return nullptr;
                        } 
                        if (wcsstr(pattern,L"missing")) // Remove missing
                        {
                            return nullptr;
                        } 
                        return pattern;
                    }, &alloc)
                    ->SortCurrentList(&CompareRefConstNames::Instance)
                    ->GroupByAdjacentOnCurrentList([&](LPCWSTR p1, LPCWSTR p2) {
                        return wcscmp(p1,p2)==0;
                    }, &alloc)
                    ->SortCurrentList(&countCompare)
                    ->Iterate([&](ImmutableList<LPCWSTR> * group) {
                        fwprintf(file, L"%6d %s\n", (int)group->Count(), group->First());
                    });
                }
                break;

            default:
                ExitIfFalse(false, -1004);
            }

            // Close and copy
            fclose(file);
            if(!isConsoleOut)
            {
                ExitIfFalse(CopyFileW(tempFile, settings.outputFile, FALSE),-1009);
                DeleteFileW(tempFile);
            }
            return 0;
        }
        else
        {
            wprintf(L"<no-metadata-files/>\n");
            ExitIfFalse(false, -1003);
        }
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr)
    if (FAILED(hr))
    {
        wprintf(L"<exception hr='%X'/>\n", hr);
        ExitIfFalse(SUCCEEDED(hr), -1006);
    }

    return 0;
}

// Report the error code to console
int ExceptionFilter(DWORD exceptionCode) 
{ 
    wprintf(L"<catastrophic-exception code='%X'/>\n", exceptionCode);
    return EXCEPTION_EXECUTE_HANDLER; 
} 

int _cdecl wmain(int argc, __in_ecount(argc) LPWSTR argv[]) 
{ 
    __try 
    { 
        // Call the real entrypoint. 
        return wmain2(argc, argv); 
    } 
    __except(ExceptionFilter(GetExceptionCode())) 
    { 
        exit(GetExceptionCode()); 
    } 
}

