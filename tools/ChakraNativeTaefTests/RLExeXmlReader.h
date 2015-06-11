// Copyright (c) Microsoft Corporation. All rights reserved.

#ifdef RLEXE_XML_DATASOURCE
#include "idatasource.h"
namespace ChakraNativeTaefTests
{
    enum DataSourceFlags
    {
        None = 0,
        ProjectionTests = 0x1,
        JsEtwConsole = 0x2
    };
    class RLExeXmlDataSource
    {
    public:
        static HRESULT Open(wchar_t const * filename, wchar_t const * dirtags, DataSourceFlags flags, WEX::TestExecution::IDataSource ** dataSource);
        static HRESULT GenerateXmlDataSource(wchar_t const * rlexexml, wchar_t const * dirtags, DataSourceFlags flags, wchar_t const * outputFileName);
    };

    
};
#endif