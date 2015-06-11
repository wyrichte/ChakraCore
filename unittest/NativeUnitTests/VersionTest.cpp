// Copyright (C) Microsoft. All rights reserved.

#include "stdafx.h"
#include "ScriptDirectUnitTests.h"

// If there is no change in the jscript directory echo PASSED
// If there is change - ensure byteCodeCacheReleaseFileVersion.h has also changed. If it has not echo FAILED.
void RunVersionTest() {
    system("call sd opened 2>nul|findstr /C:\"jscript/\" >nul 2>&1 && ((call sd opened|findstr /C:\"byteCodeCacheReleaseFileVersion.h\" >nul 2>&1) && echo PASSED || echo FAILED) || echo PASSED ");
    Print("");
    Print("It is necessary to update GUID version in byteCodeCacheReleaseFileVersion with every code change in the jscript directory.");
    Print("");
}