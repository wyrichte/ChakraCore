//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"

#include "Types\NullTypeHandler.h"
#include "Types\SimpleTypeHandler.h"
#include "Types\PathTypeHandler.h"
#include "Types\PropertyIndexRanges.h"
#include "Types\SimpleDictionaryPropertyDescriptor.h"
#include "Types\SimpleDictionaryTypeHandler.h"
#include "Types\SimpleDictionaryUnorderedTypeHandler.h"

#include "Library\BoundFunction.h"
#include "Library\JSONString.h"
#include "Library\SingleCharString.h"
#include "Library\SubString.h"
#include "Library\BufferStringBuilder.h"

// Parser includes
#include "errstr.h"
// TODO: clean up the need of these regex related header here just for GroupInfo needed in JavascriptRegExpConstructor
#include "RegexCommon.h"
#include "Library\JavascriptRegExpConstructor.h"

// Reuse this GUID to identify build
#include "ByteCodeCacheReleaseFileVersion.h"

extern HANDLE g_hInstance;

void DiagHookDumpGlobals()
{
    // Dump a build GUID for ChakraDiag to verify builds match
    REFIID buildGuid = byteCodeCacheReleaseFileVersion;
    LPOLESTR guidStr;
    if (SUCCEEDED(StringFromIID(buildGuid, &guidStr)))
    {
        wprintf(_u("Build\t%s\n"), guidStr);
        CoTaskMemFree(guidStr);
    }

    // Dump module offsets of some globals
    INT_PTR baseAddr = reinterpret_cast<INT_PTR>(g_hInstance);

#define ENTRY(field, name) \
    wprintf(_u(#name) _u("\t%Ix\n"), static_cast<size_t>(reinterpret_cast<INT_PTR>(&##field##) - baseAddr));
#include "DiagGlobalList.h"
#undef ENTRY
}
