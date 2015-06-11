//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once


// 
// Warnings
// 

#pragma warning(disable: 4100)  // unreferenced formal parameter
#pragma warning(disable: 4127)  // constant expression for Trace/Assert
#pragma warning(disable: 4200)  // nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable: 4201)  // nameless unions are part of C++
#pragma warning(disable: 4512)  // private operator= are good to have
#pragma warning(disable: 4481)  // allow use of abstract and override keywords

// warnings caused by normal optimizations
#if DBG
#else // DBG
#pragma warning(disable: 4702)  // unreachable code caused by optimizations
#pragma warning(disable: 4189)  // initialized but unused variable
#pragma warning(disable: 4390)  // empty controlled statement
#endif // DBG
