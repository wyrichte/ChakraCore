//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//
// Prints the JavaScript file for the given projection. Functions are stubbed
// out. There are XML comments to aid Intellisense.
//----------------------------------------------------------------------------
#pragma once

namespace ProjectionModel
{
    // Dump a .js file matching the given projection. Functions are stubbed out.
    struct JavaScriptStubDumper
    {
        static void Dump(ProjectionModel::ProjectionBuilder & builder, RtASSIGNMENTSPACE expr, LPCWSTR rootNamespace, FILE * file, ArenaAllocator * a);
    };
}
