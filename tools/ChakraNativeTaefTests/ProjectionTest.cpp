// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"

using namespace ChakraNativeTaefTests;

// One we converted the data source, we can get rid of the custom data source
#ifdef RLEXE_XML_DATASOURCE
#define DATASOURCE(CLASSNAME, DIRNAME)  L"Export:Projection" L#CLASSNAME L"DataSource"
#define CREATE_DATASOURCE(CLASSNAME, DIRNAME, DIRTAGS) \
extern "C" HRESULT __declspec(dllexport) __cdecl Projection##CLASSNAME##DataSource(_COM_Outptr_ IDataSource** ppDataSource, _Reserved_ void*) \
{ \
    return ChakraNativeTaefTests::CreateDataSource(DataSourceFlags::ProjectionTests, L#DIRNAME, DIRTAGS, ppDataSource); \
}
#else
#define DATASOURCE(CLASSNAME, DIRNAME) L"Table:Taef_Projection_" L#DIRNAME L".xml#Tests"
#define CREATE_DATASOURCE(CLASSNAME, DIRNAME, DIRTAGS)
#endif

// NOTE: Projection test only have NoNative and ForceNative
#define DEFINE_TEST_DIR(DIRNAME, DIRTAGS) DEFINE_TEST_DIR_(DIRNAME, DIRNAME, DIRTAGS)
#define DEFINE_TEST_DIR_(CLASSNAME, DIRNAME, DIRTAGS) \
namespace ProjectionTest \
{ \
    class CLASSNAME : public TestBase \
    { \
        BEGIN_TEST_CLASS(CLASSNAME) \
            TEST_CLASS_PROPERTY(L"DataSource", DATASOURCE(CLASSNAME, DIRNAME)) \
        END_TEST_CLASS() \
        \
        TEST_CLASS_SETUP(ClassSetup) { return InitTest(L#DIRNAME, true); }  \
        TEST_CLASS_CLEANUP(ClassCleanup) { return true; } \
        \
        TEST_METHOD(ProjectionVariants) \
        { \
            BEGIN_TEST_METHOD_PROPERTIES() \
                TEST_METHOD_PROPERTY(L"DataSource", L"Table:Variants.xml#ProjectionVariants") \
            END_TEST_METHOD_PROPERTIES() \
            RunVariant(); \
        } \
    }; \
} \
\
CREATE_DATASOURCE(CLASSNAME, DIRNAME, DIRTAGS)


#include "ProjectionTestDirs.h"
