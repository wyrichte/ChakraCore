//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "tsfiledumper.h"
#include "HelperTypesDefinitions.h"

const int TABSIZE = 4;

// Returns a string of spaces with a given size
LPCWSTR TSFileDumper::GenerateTabsString(const int tab)
{
    wchar_t* tabs = AnewArray(objectModel->alloc, wchar_t, (tab * TABSIZE + 1));
    for (int q = 0; q < TABSIZE * tab; q++)
    {
        *(tabs + q) = L' ';
    }
    *(tabs + tab * TABSIZE) = L'\0';
    return (LPCWSTR)tabs;
}

// Writes a given documentation to the output file
void TSFileDumper::DumpDocumentation(_In_ const TSWrapper::Documentation* dumpedDocumentation, _In_ FILE* file, const int tab)
{
    if (!dumpedDocumentation)
    {
        return;
    }

    LPCWSTR tabs = GenerateTabsString(tab);

    fwprintf(file, L"%s/**\n", tabs);
    if (dumpedDocumentation->summary)
    {
        fwprintf(file, L"%s * %s\n", tabs, dumpedDocumentation->summary);
    }
    dumpedDocumentation->parameters->Iterate(
        [&](_In_ TSWrapper::ParameterDocumentation* parameter)
        {
            fwprintf(file, L"%s * @param %s %s\n", tabs, parameter->name, parameter->description);
        });
    if (dumpedDocumentation->returns)
    {
        fwprintf(file, L"%s * @returns %s\n", tabs, dumpedDocumentation->returns);
    }
    fwprintf(file, L"%s */\n", tabs);
}

void TSFileDumper::SetObjectModel(_In_ const TSWrapper::TSObjectModel* om)
{
    objectModel = om;
}

// Writes type information to the output file
void TSFileDumper::DumpType(_In_ const TSWrapper::Type* dumpedType, _In_ FILE* file)
{
    if (dumpedType->type == TSWrapper::Types::BasicType)
    {
        fwprintf(file, L"%s", dumpedType->basicType);
        if (dumpedType->subTypes)
        { 
            fwprintf(file, L"<");
            dumpedType->subTypes->IterateBetween(
                [&](_In_ const TSWrapper::Type* subType)
                {
                    DumpType(subType, file);
                },
                [&](_In_ const TSWrapper::Type*, _In_ const TSWrapper::Type*)
                {
                    fwprintf(file, L", ");
                });
            fwprintf(file, L">");
        }
    }
    else if (dumpedType->type == TSWrapper::Types::ObjectType)
    {
        DumpInterfaceAsTypeDefinition(dumpedType->objectType, file);
    }
    else if (dumpedType->type == TSWrapper::Types::FunctionType)
    {
        fwprintf(file, L"%s", dumpedType->functionType->name);
        DumpFunctionAsTypeDefinition(dumpedType->functionType, file);
    }

    for (int depth = 0; depth < dumpedType->arrayDepth; depth++)
    {
        fwprintf(file, L"[]");
    }
}

// Writes field's information and documentation to the output file
void TSFileDumper::DumpField(_In_ const TSWrapper::Field* dumpedField, _In_ FILE* file, const int tab, const bool isStatic)
{
    LPCWSTR tabs = GenerateTabsString(tab);
    LPCWSTR optionalTerm = L"";
    if (dumpedField->isOptional)
    {
        optionalTerm = L"?";
    }

    DumpDocumentation(dumpedField->documentation, file, tab);
    fwprintf(file, L"%s%s%s: ", tabs, dumpedField->name, optionalTerm);
    DumpType(dumpedField->type, file);
    fwprintf(file, L";\n");
}

// Writes function's information to the output file
void TSFileDumper::DumpFunction(_In_ const TSWrapper::Function* dumpedFunction, _In_ FILE* file, const int tab, const bool isStatic)
{
    LPCWSTR tabs = GenerateTabsString(tab);

    DumpDocumentation(dumpedFunction->documentation, file, tab);

    LPCWSTR optionalTerm = L"";
    if (dumpedFunction->isOptional)
    {
        optionalTerm = L"?";
    }

    if (isStatic)
    {
        fwprintf(file, L"%sstatic %s%s(", tabs, dumpedFunction->name, optionalTerm);
    }
    else
    {
        fwprintf(file, L"%s%s%s(", tabs, dumpedFunction->name, optionalTerm);
    }
    
    bool appended = false;

    dumpedFunction->parameters->IterateBetween(
        [&](_In_ const TSWrapper::Field* innerParameter)
        {
            if (innerParameter->type->isInOnlyParameter)
            {
                fwprintf(file, L"%s: ", innerParameter->name);
                DumpType(innerParameter->type, file);
                appended = true;
            }
        }, 
        [&](_In_ const TSWrapper::Field* previousParameter, _In_ const TSWrapper::Field* nextParameter) 
        { 
            if (appended && nextParameter->type->isInOnlyParameter)
            {
                fwprintf(file, L", ");
            }
        });

    fwprintf(file, L")");
    if (dumpedFunction->returnType)
    {
        fwprintf(file, L": ");
        DumpType(dumpedFunction->returnType, file);
    }
    else if (!dumpedFunction->isConstructor)
    {
        fwprintf(file, L": void");
    }
    fwprintf(file, L";\n");
}

// Writes a handler's interface to the output file
void TSFileDumper::DumpHandler(_In_ const TSWrapper::Interface* dumpedHandler, _In_ FILE* file, const int tab)
{
    LPCWSTR tabs = GenerateTabsString(tab);
    LPCWSTR oneTab = GenerateTabsString(1);

    DumpDocumentation(dumpedHandler->documentation, file, tab);

    fwprintf(file, L"%sinterface %s", tabs, dumpedHandler->name);

    if (dumpedHandler->genericParameters)
    {
        fwprintf(file, L"<");
        dumpedHandler->genericParameters->IterateBetween(
            [&](_In_ LPCWSTR genericTypeName)
            {
                fwprintf(file, L"%s", genericTypeName);
            },
            [&](_In_ LPCWSTR, _In_ LPCWSTR)
            {
                fwprintf(file, L", ");
            });
        fwprintf(file, L"> ");
    }
    else
    {
        fwprintf(file, L" ");
    }

    if (dumpedHandler->implementedInterfaces->Count() > 0)
    {
        fwprintf(file, L"extends");
        dumpedHandler->implementedInterfaces->IterateBetween(
            [&](_In_ LPCWSTR interfaceName)
            {
                fwprintf(file, L" %s", interfaceName);
            },
            [&](_In_ LPCWSTR, _In_ LPCWSTR)
            {
                fwprintf(file, L",");
            });
        fwprintf(file, L" ");
    }

    fwprintf(file, L"{\n");

    dumpedHandler->functions->Iterate(
        [&](_In_ const TSWrapper::Function* innerFunction)
        {
            fwprintf(file, L"%s%s", tabs, oneTab);
            DumpFunctionAsTypeDefinition(innerFunction, file);
            fwprintf(file, L";\n");
        });

    fwprintf(file, L"%s}\n", tabs);
}

// Writes a function information as a type definition (i.e. as a parameter's type or a return type) to the output file
void TSFileDumper::DumpFunctionAsTypeDefinition(_In_ const TSWrapper::Function* dumpedFunction, _In_ FILE* file)
{
    LPCWSTR optionalTerm = L"";
    if (dumpedFunction->isOptional)
    {
        optionalTerm = L"?";
    }

    fwprintf(file, L"(");

    bool appended = false;

    dumpedFunction->parameters->IterateBetween(
        [&](_In_ const TSWrapper::Field* parameter)
        {
            if (parameter->type->isInOnlyParameter)
            {
                fwprintf(file, L"%s%s: ", parameter->name, optionalTerm);
                DumpType(parameter->type, file);
                appended = true;
            }
        },
        [&](_In_ const TSWrapper::Field*, _In_ const TSWrapper::Field* current)
        { 
            if (appended && current->type->isInOnlyParameter)
            {
                fwprintf(file, L", ");
            }
        });

    fwprintf(file, L"): ");
    if (dumpedFunction->returnType)
    {
        DumpType(dumpedFunction->returnType, file);
    }
    else
    {
        fwprintf(file, L"void");
    }
}

// Writes the interface's information and its inner body to the output file
void TSFileDumper::DumpInterface(_In_ const TSWrapper::Interface* dumpedInterface, _In_ FILE* file, const int tab)
{
    if (wcscmp(dumpedInterface->name, L"IMapView") == 0 || wcscmp(dumpedInterface->name, L"IMap") == 0)
    {
        return;
    }

    if (dumpedInterface->hasExclusiveToAttribute || dumpedInterface->hasWebHostHiddenAttribute)
    {
        return;
    }

    if (dumpedInterface->isHandler)
    {
        DumpHandler(dumpedInterface, file, tab);
        return;
    }
    LPCWSTR tabs = GenerateTabsString(tab);
    LPCWSTR oneTab = GenerateTabsString(1);

    DumpDocumentation(dumpedInterface->documentation, file, tab);

    fwprintf(file, L"%sinterface %s", tabs, dumpedInterface->name);

    if (dumpedInterface->genericParameters)
    {
        fwprintf(file, L"<");
        dumpedInterface->genericParameters->IterateBetween(
            [&](_In_ LPCWSTR genericTypeName)
            {
                fwprintf(file, L"%s", genericTypeName);
            },
            [&](_In_ LPCWSTR, _In_ LPCWSTR)
            {
                fwprintf(file, L", ");
            });
        fwprintf(file, L"> ");
    }
    else
    {
        fwprintf(file, L" ");
    }

    if (dumpedInterface->implementedInterfaces->Count() > 0)
    {
        fwprintf(file, L"extends");
        dumpedInterface->implementedInterfaces->IterateBetween(
            [&](_In_ LPCWSTR interfaceName)
            {
                fwprintf(file, L" %s", interfaceName);
            },
            [&](_In_ LPCWSTR, _In_ LPCWSTR)
            {
                fwprintf(file, L",");
            });
            fwprintf(file, L" ");
    }

    fwprintf(file, L"{\n");

    dumpedInterface->fields->Iterate(
        [&](_In_ const TSWrapper::Field* innerField)
        {
            DumpField(innerField, file, tab + 1, /*isStatic = */false);
        });

    dumpedInterface->functions->Iterate(
        [&](_In_ const TSWrapper::Function* innerFunction)
        {
            DumpFunction(innerFunction, file, tab + 1, /*isStatic = */false);
        });

    dumpedInterface->staticFields->Iterate(
        [&](_In_ const TSWrapper::Field* innerStaticField)
        {
            DumpField(innerStaticField, file, tab + 1, /*isStatic = */true);
        });

    dumpedInterface->staticFunctions->Iterate(
        [&](_In_ const TSWrapper::Function* innerStaticFunction)
        {
            DumpFunction(innerStaticFunction, file, tab + 1, /*isStatic = */true);
        });

    dumpedInterface->events->Iterate(
        [&](_In_ const TSWrapper::Event* innerEvent)
        {
            DumpEvent(innerEvent, file, tab + 1, /*isStatic = */false);
        });

    dumpedInterface->staticEvents->Iterate(
        [&](_In_ const TSWrapper::Event* innerStaticEvent)
        {
            DumpEvent(innerStaticEvent, file, tab + 1, /*isStatic = */true);
        });

    if (dumpedInterface->events->Count() > 0)
    {
        fwprintf(file, L"%s%saddEventListener(type: string, listener: any): void;\n", tabs, oneTab);
        fwprintf(file, L"%s%sremoveEventListener(type: string): void;\n", tabs, oneTab);
    }

    if (dumpedInterface->staticEvents->Count() > 0)
    {
        fwprintf(file, L"%s%sstatic addEventListener(type: string, listener: any): void;\n", tabs, oneTab);
        fwprintf(file, L"%s%sstatic removeEventListener(type: string): void;\n", tabs, oneTab);
    }

    fwprintf(file, L"%s}\n", tabs);
}

// Write an interface's information as a type definition (i.e. a parameter's type, field's type or return type) to the output file
void TSFileDumper::DumpInterfaceAsTypeDefinition(_In_ const TSWrapper::Interface* dumpedInterface, _In_ FILE* file)
{
    fwprintf(file, L"{ ");

    dumpedInterface->fields->Iterate(
        [&](_In_ const TSWrapper::Field* innerField)
        {
            fwprintf(file, L"%s: ", innerField->name);
            DumpType(innerField->type, file);
            fwprintf(file, L"; ");
        });

    fwprintf(file, L"}");
}

// Write an enum's information and body to the output file
void TSFileDumper::DumpEnum(_In_ const TSWrapper::Enum* dumpedEnum, _In_ FILE* file, const int tab)
{
    if (dumpedEnum->hasExclusiveToAttribute || dumpedEnum->hasWebHostHiddenAttribute)
    {
        return;
    }

    LPCWSTR tabs = GenerateTabsString(tab);
    LPCWSTR oneTab = GenerateTabsString(1);

    DumpDocumentation(dumpedEnum->documentation, file, tab);

    fwprintf(file, L"%senum %s {\n", tabs, dumpedEnum->name);

    int enumEntriesCounter = 0;

    dumpedEnum->entries->IterateBetween(
        [&](_In_ const TSWrapper::EnumEntry* entry)
        {
            DumpDocumentation(entry->documentation, file, tab + 1);
            fwprintf(file, L"%s%s%s = %d", tabs, oneTab, entry->name, enumEntriesCounter);
            enumEntriesCounter++;
        },
        [&](_In_ const TSWrapper::EnumEntry*, _In_ const TSWrapper::EnumEntry*)
        {
            fwprintf(file, L",\n"); 
        });

    fwprintf(file, L"\n%s}\n", tabs);
}

// Writes an event's information to the output file
void TSFileDumper::DumpEvent(_In_ const TSWrapper::Event* dumpedEvent, _In_ FILE* file, const int tab, const bool isStatic)
{
    LPCWSTR tabs = GenerateTabsString(tab);
    
    LPCWSTR eventName = dumpedEvent->name;

    DumpDocumentation(dumpedEvent->documentation, file, tab);
    LPCWSTR staticTerm = L"";
    if (isStatic)
    {
        staticTerm = L"static ";
    }

    fwprintf(file, L"%s%saddEventListener(type: \"%s\", listener: (ev: ", tabs, staticTerm, eventName);
    DumpType(dumpedEvent->eventType, file);
    fwprintf(file, L") => any): void;\n");

    fwprintf(file, L"%s%sremoveEventListener(type: \"%s\"): void;\n", tabs, staticTerm, eventName);
    DumpDocumentation(dumpedEvent->documentation, file, tab);
    
    fwprintf(file, L"%s%son%s: (ev: ", tabs, staticTerm, eventName);
    DumpType(dumpedEvent->eventType, file);
    fwprintf(file, L") => any;\n");
}

// Writes a class' information and body to the output file
void TSFileDumper::DumpClass(_In_ const TSWrapper::Class* dumpedClass, _In_ FILE* file, const int tab)
{
    if (dumpedClass->hasExclusiveToAttribute || dumpedClass->hasWebHostHiddenAttribute)
    {
        return;
    }

    LPCWSTR tabs = GenerateTabsString(tab);
    LPCWSTR oneTab = GenerateTabsString(1);

    DumpDocumentation(dumpedClass->documentation, file, tab);

    fwprintf(file, L"%sclass %s ", tabs, dumpedClass->name);

    if (dumpedClass->implementedInterfaces->Count() > 0)
    {
        fwprintf(file, L"implements");
        dumpedClass->implementedInterfaces->IterateBetween(
            [&](_In_ TSWrapper::Interface* implementedInterface) 
            {
                fwprintf(file, L" %s", implementedInterface->name);
            },
            [&](_In_ TSWrapper::Interface*, _In_ TSWrapper::Interface*)
            {
                fwprintf(file, L","); 
            });
        fwprintf(file, L" ");
    }

    fwprintf(file, L"{\n");

    dumpedClass->fields->Iterate(
        [&](_In_ const TSWrapper::Field* innerField)
        {
            DumpField(innerField, file, tab + 1, /*isStatic = */false);
        });

    dumpedClass->functions->Iterate(
        [&](_In_ const TSWrapper::Function* innerFunction)
        {
            DumpFunction(innerFunction, file, tab + 1, /*isStatic = */false);
        });

    dumpedClass->staticFields->Iterate(
        [&](_In_ const TSWrapper::Field* innerStaticField)
        {
            DumpField(innerStaticField, file, tab + 1, /*isStatic = */true);
        });

    dumpedClass->staticFunctions->Iterate(
        [&](_In_ const TSWrapper::Function* innerStaticFunction)
        {
            DumpFunction(innerStaticFunction, file, tab + 1, /*isStatic = */true);
        });

    dumpedClass->events->Iterate(
        [&](_In_ const TSWrapper::Event* innerEvent)
        {
            DumpEvent(innerEvent, file, tab + 1, /*isStatic = */false);
        });

    dumpedClass->staticEvents->Iterate(
        [&](_In_ const TSWrapper::Event* innerStaticEvent)
        {
            DumpEvent(innerStaticEvent, file, tab + 1, /*isStatic = */true);
        });

    if (dumpedClass->events->Count() > 0)
    {
        fwprintf(file, L"%s%saddEventListener(type: string, listener: any): void;\n", tabs, oneTab);
        fwprintf(file, L"%s%sremoveEventListener(type: string): void;\n", tabs, oneTab);
    }

    if (dumpedClass->staticEvents->Count() > 0)
    {
        fwprintf(file, L"%s%sstatic addEventListener(type: string, listener: any): void;\n", tabs, oneTab);
        fwprintf(file, L"%s%sstatic removeEventListener(type: string): void;\n", tabs, oneTab);
    }

    fwprintf(file, L"%s}\n", tabs);
}

// Writes a module's information and body to the output file
void TSFileDumper::DumpModule(_In_ const TSWrapper::Module* dumpedModule, _In_ FILE* file, const int tab, const bool isRoot)
{
    LPCWSTR tabs = GenerateTabsString(tab);

    DumpDocumentation(dumpedModule->documentation, file, tab);

    if (isRoot)
    {
        fwprintf(file, L"%sdeclare module %s {\n", tabs, dumpedModule->name);
    }
    else
    {
        fwprintf(file, L"%smodule %s {\n", tabs, dumpedModule->name);
    }

    dumpedModule->modules->Iterate(
        [&](_In_ const TSWrapper::Module* innerModule)
        {
            DumpModule(innerModule, file, tab + 1, /*isRoot = */false);
        });

    dumpedModule->interfaces->Iterate(
        [&](_In_ const TSWrapper::Interface* innerInterface)
        {
            DumpInterface(innerInterface, file, tab + 1);
        });

    dumpedModule->enums->Iterate(
        [&](_In_ const TSWrapper::Enum* innerEnum)
        {
            DumpEnum(innerEnum, file, tab + 1);
        });

    dumpedModule->classes->Iterate(
        [&](_In_ const TSWrapper::Class* innerClass)
        {
            DumpClass(innerClass, file, tab + 1);
        });

    dumpedModule->fields->Iterate(
        [&](_In_ const TSWrapper::Field* innerField)
        {
            DumpField(innerField, file, tab + 1, /*isStatic = */false);
        });

    fwprintf(file, L"%s}\n", tabs);
}

// Writes a typescript object model's information to the output file
void TSFileDumper::Dump(_In_ const TSWrapper::TSObjectModel* objectModel)
{
    SetObjectModel(objectModel);
    
    FILE* file;
    HRESULT hr = _wfopen_s(&file, objectModel->settings->outputFile, L"w,ccs=UTF-8");
    if (hr != S_OK)
    {
        wprintf(L"<couldn't-open-outputfile filename='%s'/>\n", objectModel->settings->outputFile);
        return;
    }

    fwprintf(file, L"/// <reference path=\"winrt.common.d.ts\"/>\n\n");

    objectModel->modules->Iterate(
        [&](_In_ const TSWrapper::Module* module)
        {
            DumpModule(module, file, /*tab = */0, /*isRoot = */true);
        });

    fclose(file);

    FILE* winrtCommonFile;
    hr = _wfopen_s(&winrtCommonFile, L"winrt.common.d.ts", L"w,ccs=UTF-8");
    if (hr != S_OK)
    {
        wprintf(L"<couldn't-open-outputfile filename='%s'/>\n", objectModel->settings->outputFile);
        return;
    }

    fwprintf(winrtCommonFile, TSGEN_PROMISE_DEFINITION_BODY);
    fwprintf(winrtCommonFile, TSGEN_IMAP_IMAPVIEW_DEFINITION_BODY);

    fclose(winrtCommonFile);
}