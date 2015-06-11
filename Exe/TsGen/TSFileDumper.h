//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "tswrapper.h"

class TSFileDumper
{
public:
    const TSWrapper::TSObjectModel* objectModel = nullptr;

    TSFileDumper() {}

    void DumpModule(_In_ const TSWrapper::Module* dumpedModule, _In_ FILE* file, const int tabs, const bool isRoot);
    void DumpClass(_In_ const TSWrapper::Class* dumpedClass, _In_ FILE* file, const int tabs);
    void DumpEnum(_In_ const TSWrapper::Enum* dumpedEnum, _In_ FILE* file, const int tabs);
    void DumpInterface(_In_ const TSWrapper::Interface* dumpedInterface, _In_ FILE* file, const int tabs);
    void DumpInterfaceAsTypeDefinition(_In_ const TSWrapper::Interface* dumpedInterface, _In_ FILE* file);
    void DumpFunction(_In_ const TSWrapper::Function* dumpedFunction, _In_ FILE* file, const int tab, const bool isStatic);
    void DumpFunctionAsTypeDefinition(_In_ const TSWrapper::Function* dumpedFunction, _In_ FILE* file);
    void DumpType(_In_ const TSWrapper::Type* dumpedType, _In_ FILE* file);
    void DumpEvent(_In_ const TSWrapper::Event* dumpedEvent, _In_ FILE* file, const int tab, const bool isStatic);
    void DumpField(_In_ const TSWrapper::Field* dumpedField, _In_ FILE* file, const int tab, const bool isStatic);
    void Dump(_In_ const TSWrapper::TSObjectModel* objectModel);
    void SetObjectModel(_In_ const TSWrapper::TSObjectModel* om);

    LPCWSTR GenerateTabsString(const int tab);
    void DumpHandler(_In_ const TSWrapper::Interface* dumpedHandler, _In_ FILE* file, const int tab);
    void DumpDocumentation(_In_ const TSWrapper::Documentation* dumpedDocumentation, _In_ FILE* file, const int tab);
};