//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////
// MockExternalObject.h is used by static lib shared between trident and chakra. Its size
// MUST be the same as Js::CustomExternalObject and Js::ExternalObject. There is an assert
// in dll\jscript\customexternaltype.h to ensure that. 
// If there is change in CustomExternalObject, we need to have matching change here. Matching
// mshtml.dll needs to be shipped with jscript9.dll
/////////////////////////////////////////////////////////
#pragma once

namespace Js
{
    class MockExternalObject
    {
        void* vtbl;
        void* type;
        void* auxSlots;
        void* objectArrayOrFlags;
        void* finalizer;
        void* cachedJavascriptDispatch;
    };
}