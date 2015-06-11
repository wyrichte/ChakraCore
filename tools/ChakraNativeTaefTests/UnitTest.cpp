// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"

using namespace ChakraNativeTaefTests;

// One we converted the data source, we can get rid of the custom data source
#ifdef RLEXE_XML_DATASOURCE
#define DATASOURCE(CLASSNAME, DIRNAME)  L"Export:UnitTest" L#CLASSNAME L"DataSource"
#define DEFINE_CREATEDATASOURCE(CLASSNAME, DIRNAME, DIRTAGS) \
extern "C" HRESULT __declspec(dllexport) __cdecl UnitTest##CLASSNAME##DataSource(_COM_Outptr_ IDataSource** ppDataSource, _Reserved_ void*) \
{ \
    return ChakraNativeTaefTests::CreateDataSource(DataSourceFlags::None, L#DIRNAME, DIRTAGS, ppDataSource); \
}
#else
#define DATASOURCE(CLASSNAME, DIRNAME) L"Table:Taef_Unit_" L#DIRNAME L".xml#Tests"
#define DEFINE_CREATEDATASOURCE(CLASSNAME, DIRNAME, DIRTAGS)
#endif

#define DEFINE_TEST_DIR(DIRNAME, DIRTAGS) DEFINE_TEST_DIR_(DIRNAME, DIRNAME, DIRTAGS)
#define DEFINE_TEST_DIR_(CLASSNAME, DIRNAME, DIRTAGS) \
namespace UnitTest \
{ \
    class CLASSNAME : public TestBase \
    { \
        BEGIN_TEST_CLASS(CLASSNAME) \
            TEST_CLASS_PROPERTY(L"DataSource", DATASOURCE(CLASSNAME, DIRNAME)) \
        END_TEST_CLASS() \
        \
        TEST_CLASS_SETUP(ClassSetup) { return InitTest(L#DIRNAME, false); }  \
        TEST_CLASS_CLEANUP(ClassCleanup) { return true; } \
        \
        TEST_METHOD(DynamicProfileVariants) \
        { \
            BEGIN_TEST_METHOD_PROPERTIES() \
                TEST_METHOD_PROPERTY(L"DataSource", L"Table:Variants.xml#DynamicProfileVariants") \
                TEST_METHOD_PROPERTY(L"ExecutionGroup", L"DynamicProfileVariants") \
            END_TEST_METHOD_PROPERTIES() \
            RunVariant(); \
        } \
        TEST_METHOD(NonDynamicProfileVariants) \
        { \
            BEGIN_TEST_METHOD_PROPERTIES() \
                TEST_METHOD_PROPERTY(L"DataSource", L"Table:Variants.xml#NonDynamicProfileVariants") \
            END_TEST_METHOD_PROPERTIES() \
            RunVariant(); \
        } \
    }; \
} \
\
DEFINE_CREATEDATASOURCE(CLASSNAME, DIRNAME, DIRTAGS)

#include "TestDirs.h"