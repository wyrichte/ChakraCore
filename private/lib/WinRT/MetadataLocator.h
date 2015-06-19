//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Implementation for the metadata locator that would be used as a callback to api that gets param iid
#pragma once

namespace ProjectionModel
{
    // *******************************************************
    // Represents a the metadata locator
    // *******************************************************
    struct MetadataLocator : public IRoMetaDataLocator
    {
        ProjectionModel::ProjectionBuilder & builder;
    public:
        MetadataLocator(ProjectionModel::ProjectionBuilder & builder) 
            : builder(builder)
        {  }

        //
        // Parameters:
        //   nameElement
        //     a metadata typeref name to resolve.  
        //     Eg: "N1.N2.IFoo", or "W.F.C.IVector`1".
        //   pushMetaData
        //     data sink for providing information about the 
        //     type information for nameElement
        //
        STDMETHOD(Locate)(
            __in  PCWSTR nameElement,
            __in  IRoSimpleMetaDataBuilder& metaDataDestination
        ) const;
    };
};