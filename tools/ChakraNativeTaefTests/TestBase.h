
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

namespace ChakraNativeTaefTests
{
    class TestBase
    {
        static const int DIFF_OUTPUT_MAX_RETRY = 3;
        static const int DIFF_OUTPUT_WAIT = 200; // ms

    protected:
        TestBase();
        bool InitTest(wchar_t const * testDir, bool isProjection);
        void RunVariant();
    private:
        // Ported from RL
        static bool CheckForPass(wchar_t const * filename);
    
        bool FileDiff(wchar_t const * baseLine, wchar_t const * diffFile);
        bool Diff(wchar_t const * baseLine, wchar_t const * diffFile);
        bool RunTest(WEX::Common::String& fullCommandline, 
            WEX::Common::String const& outputFileName, bool pass, WEX::Common::String const& baselineFileName);
        bool Filter(wchar_t const * filterName, wchar_t const * currentValue, wchar_t const * filteredMessage, wchar_t const * variant);
        void LogFilter(wchar_t const * filteredMessage, wchar_t const * variant);
        HRESULT GetTestFlags(WEX::Common::String& testflags);

    private:
        SetVerifyOutput verifySettings;
        wchar_t const *testDir;
        WEX::Common::String logsDir;
        WEX::Common::String fullTestDir;
        int testIndex;
        bool isProjection;

    };

#ifdef RLEXE_XML_DATASOURCE
    HRESULT CreateDataSource(DataSourceFlags flags, wchar_t const * dirname, wchar_t const * dirtags, IDataSource ** ppDataSource);
#endif
};