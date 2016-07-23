//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>

#include <stdio.h>
#include <tchar.h>

#include "Cor.h"
#include "wrl.h"
#include "rometadata.h"

#include "Common.h"
#include "WinRTLib.h"

#define INLINE_TEST_METHOD_MARKUP

#include "wexstring.h"
#include "wextestclass.h"
#include "verify.h"

#define ofNoTransform 0x00001000	// Disable automatic transforms of .winmd files.