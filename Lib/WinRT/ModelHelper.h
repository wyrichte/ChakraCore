//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace ProjectionModel
{
    bool AreSameRuntimeInterfaceConstructor(RtRUNTIMEINTERFACECONSTRUCTOR a, RtRUNTIMEINTERFACECONSTRUCTOR b);
    bool AreSameInterfaceConstructor(RtINTERFACECONSTRUCTOR a, RtINTERFACECONSTRUCTOR b);
    bool AreSameFunction(Function * a, Function * b);
    RtPROPERTY CreateRenamedPropertyIdentifierAsCopy(RtPROPERTY prop, MetadataStringId newId, ArenaAllocator * a);
    RtEVENT RenameEventName(RtEVENT eventToRename, MetadataStringId newName, ArenaAllocator * a
#if DBG
        , LPCWSTR newNameStr
#endif
        );
    RtRUNTIMEINTERFACECONSTRUCTOR FindRequiredMatchingInterfaceByPiid(RtRUNTIMEINTERFACECONSTRUCTOR interfaceConstructor, const IID & piid);
    ImmutableList<RtABIMETHODSIGNATURE> * FindMatchingMethodSignatures(MetadataStringId nameId, const IID & instantiated, ImmutableList<RtPROPERTY> * properties, ArenaAllocator * a);
}