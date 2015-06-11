//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace ProjectionModel
{
    RtASSIGNMENTSPACE SortAssignmentSpace(RtASSIGNMENTSPACE varspace, ArenaAllocator * a, Metadata::IStringConverter * stringConverter);
    RtEXPR SortExpr(RtEXPR expr, ArenaAllocator * a, Metadata::IStringConverter * stringConverter);
}