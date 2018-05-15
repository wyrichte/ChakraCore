//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

// REVIEW (doilij): is dllexport / dllimport needed if we are using the chakra.def file?

__declspec(dllexport)
ChakraEngine * WINAPI CreateChakraEngine();
