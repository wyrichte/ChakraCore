//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#include <ieverp.h>                     // get IE' VER_PRODUCTBUILD

#define SCRIPT_ENGINE_MAJOR_VERSION     VER_PRODUCTMAJORVERSION
#define SCRIPT_ENGINE_MINOR_VERSION     VER_PRODUCTMINORVERSION
#define SCRIPT_ENGINE_PRODUCTBUILD      VER_PRODUCTBUILD
#define SCRIPT_ENGINE_BUILDNUMBER       VER_PRODUCTBUILD_QFE    // use Windows QFE version
