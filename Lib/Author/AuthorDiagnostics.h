//---------------------------------------------------------------------------
// Copyright (C) 2011 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Authoring
{
    //
    //  TypeStats
    //
    struct TypeStats
    {
        LPCWSTR TypeName;
        volatile DWORD Instances;

        TypeStats(LPCWSTR typeName);
        void IncrementInstances();
        void DecrementInstances();
        void Dump();

        TypeStats* Next;
    };

// Creates a static TypeStats variable for a type when TRACK_TYPE_STATS is enabled
#ifdef TRACK_TYPE_STATS
#define TYPE_STATS(Type, TypeName) TypeStats Type##::_typeStats(TypeName);
#else
    #define TYPE_STATS(Type, TypeName)
#endif

    //
    //  AuthorDiagnostics
    //
    class AuthorDiagnostics
    {
    public:
        static IAuthorDiagnostics* CreateInstance(Js::ScriptContext* scriptContext);
        static void DumpTypes();
        static void DumpMemoryUsage();
        static int TotalMemoryOf(LPCWSTR name);
    };
}
