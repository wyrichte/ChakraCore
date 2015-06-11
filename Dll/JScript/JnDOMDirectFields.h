//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

//
// NOTE: This file is intended to be "#include" multiple times.  The call site should define various
// macros to be executed for each entry.  Macros that are not provided will be given a default,
// empty implementation.
//
// This is DOM specific file of JnDirectFields.h
//
#if DOMEnabled
#if !defined(ENTRYDOM)
#define ENTRYDOM(name, hash)
#endif
#if !defined(ENTRYDOM_Existing)
#define ENTRYDOM_Existing(name, key)
#endif

//#include "..\..\..\..\mshtml\src\site\fastdom\fastdom_propertyidtable.inl"

#undef ENTRYDOM
#undef ENTRYDOM_Existing
#endif