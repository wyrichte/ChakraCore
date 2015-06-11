// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "WexTestClass.h"

namespace ChakraNativeTaefTests
{
    class Globals
    {
    public:        
        static bool EnsureInitialized();        

        static wchar_t const * GetTestDir(bool isProjection) { return isProjection ? projectionTestDir : unitTestDir; }
        static wchar_t const * GetTestOutDir(bool isProjection) { return isProjection ? projectionTestOutDir : unitTestOutDir; }

        static wchar_t const * GetTestOutDir();
        
        // Configurable execution environment        
        static WEX::Common::String targetOS;
        static WEX::Common::String targetArch;    
        static WEX::Common::String targetFlavor;
        static WEX::Common::String testBinDir;
        
        // Parameters
        static WEX::Common::String extraFlags;
        static bool skipExec;
        static bool verbose;        

        // Implicit execution environment        
        static WEX::Common::String jshostPath;
        static WEX::Common::String diffPath;    
        static WEX::Common::String baselineArchOverride;
        static WEX::Common::String baselineOSOverride;
        static WEX::Common::String flagsArchOverride;
        static WEX::Common::String flagsOSOverride;

        // State
        static bool hasDiff;
        
    private:
        static WEX::Common::String unitTestDir;
        static WEX::Common::String unitTestOutDir;

        static WEX::Common::String projectionTestDir;
        static WEX::Common::String projectionTestOutDir;

        static bool EnsureTestBinDir();
        static bool EnsureTestSourceDir();
        static bool EnsurePath();
        static bool EnsureArch();
        static bool EnsureFlavor();
        static bool EnsureParameters();
        static bool GetNTROOT(WEX::Common::String& NTROOT);

        static bool hasInitialized;
    };

    
};
