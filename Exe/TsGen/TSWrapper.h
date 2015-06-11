//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "wrapper.h"

namespace TSWrapper
{
    class Interface;
    class Function;
    class Element;

    // A class representing the parameter element in the xml documentation
    class ParameterDocumentation
    {
    public:
        LPCWSTR name = nullptr;
        LPCWSTR description = nullptr;
        ArenaAllocator* alloc = nullptr;

        ~ParameterDocumentation()
        {
        }

        static ParameterDocumentation* GetParameterDocumentation(_In_ const XmlReader::Param* param, _Inout_ ArenaAllocator* alloc)
        {
            if (!param)
            {
                return nullptr;
            }

            ParameterDocumentation* resultingParameterDocumentation = Anew(alloc, ParameterDocumentation);

            resultingParameterDocumentation->name = param->name;
            resultingParameterDocumentation->description = param->description;
            
            return resultingParameterDocumentation;
        }
    };

    // A class representing a member element in the xml documentation
    class Documentation
    {
    public:
        LPCWSTR summary = nullptr;
        LPCWSTR returns = nullptr;
        LPCWSTR deprecated = nullptr;
        LPCWSTR name = nullptr;
        ImmutableList<ParameterDocumentation*>* parameters = nullptr;
        ArenaAllocator* alloc = nullptr;

        ~Documentation()
        {
            DeleteListIfNotNull(alloc, parameters, ParameterDocumentation*);
        }

        void AddParameter(_In_ ParameterDocumentation* addedParameter, _Inout_ ArenaAllocator* alloc)
        {
            parameters = parameters->Prepend(addedParameter, alloc);
        }

        static Documentation* GetDocumentation(_In_ const XmlReader::Member* member, _Inout_ ArenaAllocator* alloc)
        {
            if (!member)
            {
                return nullptr;
            }

            Documentation* resultingDocumentation = Anew(alloc, Documentation);

            resultingDocumentation->summary = member->summary;
            resultingDocumentation->returns = member->returns;
            resultingDocumentation->deprecated = member->deprecated;
            resultingDocumentation->name = member->name;
            
            member->params->Iterate(
                [&resultingDocumentation, &alloc](_In_ XmlReader::Param* param)
                {
                    resultingDocumentation->AddParameter(ParameterDocumentation::GetParameterDocumentation(param, alloc), alloc);
                });
            
            return resultingDocumentation;
        }

        static LPCWSTR ConcatElementName(_In_ LPCWSTR path, _In_ Element* element, _Inout_ ArenaAllocator* alloc);
    };

    enum struct Types
    {
        BasicType,
        ObjectType,
        FunctionType
    };

    class Element
    {
    public:
        LPCWSTR name = nullptr;
        LPCWSTR metadataName = nullptr;
        Documentation* documentation = nullptr;
        ArenaAllocator* alloc = nullptr;
        bool hasWebHostHiddenAttribute = false;
        bool hasExclusiveToAttribute = false;
        bool hasOverloadAttribute = false;

        Element() {}
        Element(_In_ LPCWSTR name) : name(name) {}
        ~Element()
        {
        }

        // Returns the element's name in the format used by the xml documentation (Check the documentation section here http://msdn.microsoft.com/en-us/library/vstudio/ms228593(v=vs.110).aspx)
        virtual LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc) = 0;
        
        // Returns the element's prefix character used by the xml documentation (Check the documentation section here http://msdn.microsoft.com/en-us/library/vstudio/ms228593(v=vs.110).aspx)
        virtual LPCWSTR GetDocumentationTypePrefix() = 0;
    };

    class Type
    {
    public:
        Types type;
        int arrayDepth = 0;
        LPCWSTR basicType = nullptr;
        Interface* objectType = nullptr;
        Function* functionType = nullptr;
        ImmutableList<Type*>* subTypes;
        bool isRefType = false;
        bool isGenericParameterType = false;
        int genericParameterIndex = -1;
        bool isOutOnlyParameter = false;
        bool isInOnlyParameter = false;
        bool isInOutParameter = false;
        LPCWSTR basicTypeOriginalName = nullptr;
        ArenaAllocator* alloc = nullptr;

        Type() {}
        ~Type()
        {
            DeleteListIfNotNull(alloc, subTypes, Type*);
        }
        
        void AddSubType(_In_ const Type* addedType, _Inout_ ArenaAllocator* alloc)
        {
            subTypes = subTypes->Prepend((Type*) addedType, alloc);
        }

        // Returns the name of the type in the format used in the xml documentation (Check the documentation section here http://msdn.microsoft.com/en-us/library/vstudio/ms228593(v=vs.110).aspx)
        LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc)
        {
            LPCWSTR qualifiedName = nullptr;
            
            switch (type)
            {
            case Types::BasicType:
            {
                qualifiedName = basicTypeOriginalName;
            }
            }
            
            if (genericParameterIndex != -1)
            {
                LPCWSTR genericIndexStr = StringUtils::IntToStr(genericParameterIndex, alloc);
                qualifiedName = StringUtils::Concat(L"`", genericIndexStr, alloc);
            }

            if (qualifiedName)
            {
                if (subTypes)
                {
                    qualifiedName = StringUtils::Concat(qualifiedName, L"`", alloc);
                    qualifiedName = StringUtils::Concat(qualifiedName, StringUtils::IntToStr((int)subTypes->Count(), alloc), alloc);
                }

                for (int q = 0; q < arrayDepth; q++)
                {
                    qualifiedName = StringUtils::Concat(qualifiedName, L"[]", alloc);
                }

                if (isRefType)
                {
                    qualifiedName = StringUtils::Concat(qualifiedName, L"@", alloc);
                }
            }
            
            return qualifiedName;
        }

        static Type* GetBasicType(_In_ LPCWSTR basicType, _In_ LPCWSTR basicTypeOriginalName, _Inout_ ArenaAllocator* alloc)
        {
            Type* resultingType = Anew(alloc, Type);
            resultingType->basicType = basicType;
            resultingType->type = Types::BasicType;
            resultingType->basicTypeOriginalName = basicTypeOriginalName;

            return resultingType;
        }

        static Type* GetObjectType(_In_ const Interface* interfaceType, _Inout_ ArenaAllocator* alloc)
        {
            Type* resultingType = Anew(alloc, Type);
            resultingType->objectType = (Interface*) interfaceType;
            resultingType->type = Types::ObjectType;

            return resultingType;
        }

        static Type* GetFunctionType(_In_ const Function* functionType, _Inout_ ArenaAllocator* alloc)
        {
            Type* resultingType = Anew(alloc, Type);
            resultingType->functionType = (Function*) functionType;
            resultingType->type = Types::FunctionType;

            return resultingType;
        }
    };

    class Event : virtual public Element
    {
    public:
        Type* eventType = nullptr;

        LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc)
        {
            LPCWSTR output = StringUtils::RemoveWhiteSpaces(metadataName, alloc);
            return StringUtils::Replace(output, L'.', L'#', alloc);
        }

        LPCWSTR GetDocumentationTypePrefix()
        {
            return L"E:";
        }
    };

    class Field : virtual public Element
    {
    public:
        Type* type = nullptr;
        LPCWSTR metadataName = nullptr;
        bool isField = false;
        bool isOptional = false;
        bool hasRetValAttribute = false;

        Field() {}
        Field(_In_ LPCWSTR name) : Element(name) {}
        Field(_In_ LPCWSTR name, _In_ Type* type) : Element(name), type(type) {}
        ~Field()
        {
        }

        void addType(Type* typeInterface)
        {
            this->type = typeInterface;
        }

        LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc)
        {
            LPCWSTR output = StringUtils::RemoveWhiteSpaces(metadataName, alloc);
            return StringUtils::Replace(output, L'.', L'#', alloc);
        }

        LPCWSTR GetDocumentationTypePrefix()
        {
            if (isField)
            {
                return L"F:";
            }

            switch (type->type)
            {
            case Types::FunctionType:
            {
                return L"M:";
            }
            case Types::ObjectType:
            {
                return L"T:";
            }
            case Types::BasicType:
            {
                return L"P:";
            }
            }

            return L"P:";
        }
    };

    class Interface : virtual public Element
    {
    public:
        ImmutableList<Function*>* functions = nullptr;
        ImmutableList<Field*>* fields = nullptr;
        ImmutableList<Function*>* staticFunctions = nullptr;
        ImmutableList<Field*>* staticFields = nullptr;
        ImmutableList<LPCWSTR>* implementedInterfaces = nullptr;
        ImmutableList<LPCWSTR>* genericParameters = nullptr;
        ImmutableList<Event*>* events = nullptr;
        ImmutableList<Event*>* staticEvents = nullptr;
        bool isHandler = false;

        Interface() {}
        Interface(_In_ LPCWSTR name) : Element(name) {}

        ~Interface()
        {
            DeleteListIfNotNull(alloc, functions, Function*);
            DeleteListIfNotNull(alloc, fields, Field*);
            DeleteListIfNotNull(alloc, staticFunctions, Function*);
            DeleteListIfNotNull(alloc, staticFields, Field*);
            DeleteListIfNotNull(alloc, events, Event*);
            DeleteListIfNotNull(alloc, staticEvents, Event*);
        }

        void AddFunction(_In_ Function* addedFunction, _Inout_ ArenaAllocator* alloc)
        {
            functions = functions->Prepend(addedFunction, alloc);
        }

        void AddFunctions(_In_ ImmutableList<Function*>* addedFunctions, _Inout_ ArenaAllocator* alloc)
        {
            addedFunctions->Iterate(
                [&](_In_ Function* addedFunction)
            {
                AddFunction(addedFunction, alloc);
            });
        }

        void AddField(_In_ Field* addedField, _Inout_ ArenaAllocator* alloc)
        {
            fields = fields->Prepend(addedField, alloc);
        }

        void AddStaticField(_In_ Field* addedStaticField, _Inout_ ArenaAllocator* alloc)
        {
            staticFields = staticFields->Prepend(addedStaticField, alloc);
        }

        void AddStaticFunction(_In_ Function* addedStaticFunction, _Inout_ ArenaAllocator* alloc)
        {
            staticFunctions = staticFunctions->Prepend(addedStaticFunction, alloc);
        }

        void AddStaticFunctions(_In_ ImmutableList<Function*>* addedFunctions, _Inout_ ArenaAllocator* alloc)
        {
            addedFunctions->Iterate(
                [&](_In_ Function* addedFunction)
            {
                AddStaticFunction(addedFunction, alloc);
            });
        }

        void AddImplementedInterface(_In_ LPCWSTR addedImplementedInterface, _Inout_ ArenaAllocator* alloc)
        {
            implementedInterfaces = implementedInterfaces->Prepend(addedImplementedInterface, alloc);
        }

        void AddDelegate(_In_ const Interface* AddedDelegate, _Inout_ ArenaAllocator* alloc)
        {
            AddedDelegate->functions->Iterate(
                [&](_In_ const Function* innerFunction)
            {
                AddFunction((Function*)innerFunction, alloc);
            });
        }

        void AddGenericParameter(_In_ LPCWSTR addedGenericParameterName, _Inout_ ArenaAllocator* alloc)
        {
            genericParameters = genericParameters->Prepend(addedGenericParameterName, alloc);
        }

        void AddEvent(_In_ const Event* addedEvent, _Inout_ ArenaAllocator* alloc)
        {
            if (addedEvent)
            {
                events = events->Prepend((Event*)addedEvent, alloc);
            }
        }

        void AddStaticEvent(_In_ const Event* addedStaticEvent, _Inout_ ArenaAllocator* alloc)
        {
            if (addedStaticEvent)
            {
                staticEvents = staticEvents->Prepend((Event*)addedStaticEvent, alloc);
            }
        }

        LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc)
        {
            LPCWSTR output = StringUtils::RemoveWhiteSpaces(metadataName, alloc);
            return StringUtils::Replace(output, L'.', L'#', alloc);
        }

        LPCWSTR GetDocumentationTypePrefix()
        {
            return L"T:";
        }
    };

    class Function : virtual public Element
    {
    public:
        ImmutableList<Field*>* parameters = nullptr;
        Type* returnType = nullptr;
        bool isConstructor = false;
        bool isOptional = false;

        Function() {}
        Function(_In_ LPCWSTR name) : Element(name) {}
        ~Function()
        {
            DeleteListIfNotNull(alloc, parameters, Field*);
        }

        void AddParameter(_In_ Field* addedParameter, _Inout_ ArenaAllocator* alloc)
        {
            parameters = parameters->Prepend(addedParameter, alloc);
        }

        LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc)
        {
            LPCWSTR qualifiedName = StringUtils::Replace(metadataName, L'.', L'#', alloc);
            qualifiedName = StringUtils::CapitalizeFirstLetter(qualifiedName, alloc);

            if (isConstructor)
            {
                qualifiedName = L"#ctor";
            }
            
            if (parameters->Count() > 0)
            {
                bool appended = false;
                parameters->IterateBetween(
                    [&](_In_ Field* parameter)
                    {
                        if (!parameter->hasRetValAttribute && !(isConstructor && !parameter->type->isInOnlyParameter))
                        {
                            if (!appended)
                            {
                                qualifiedName = StringUtils::Concat(qualifiedName, L"(", alloc);
                            }
                            qualifiedName = StringUtils::Concat(qualifiedName, parameter->type->GetNameInXmlFormat(alloc), alloc);
                            appended = true;
                        }
                    },
                    [&](_In_ Field*, _In_ Field* current)
                    {
                        if (appended && !current->hasRetValAttribute && !(isConstructor && !current->type->isInOnlyParameter))
                        {
                            qualifiedName = StringUtils::Concat(qualifiedName, L",", alloc);
                        }
                    });

                    if (appended)
                    {
                        qualifiedName = StringUtils::Concat(qualifiedName, L")", alloc);
                    }
            }
            
            return qualifiedName;
        }

        LPCWSTR GetDocumentationTypePrefix()
        {
            return L"M:";
        }
    };

    class Class : virtual public Element
    {
    public:
        ImmutableList<Function*>* functions = nullptr;
        ImmutableList<Field*>* fields = nullptr;
        ImmutableList<Interface*>* implementedInterfaces = nullptr;
        ImmutableList<Function*>* staticFunctions = nullptr;
        ImmutableList<Field*>* staticFields = nullptr;
        ImmutableList<Event*>* events = nullptr;
        ImmutableList<Event*>* staticEvents = nullptr;

        Class() {}
        Class(_In_ LPCWSTR name) : Element(name) {}
        ~Class()
        {
            DeleteListIfNotNull(alloc, functions, Function*);
            DeleteListIfNotNull(alloc, fields, Field*);
            DeleteListIfNotNull(alloc, staticFunctions, Function*);
            DeleteListIfNotNull(alloc, staticFields, Field*);
            DeleteListIfNotNull(alloc, events, Event*);
            DeleteListIfNotNull(alloc, staticEvents, Event*);
            DeleteListIfNotNull(alloc, implementedInterfaces, Interface*);
        }

        void AddFunction(_In_ Function* addedFunction, _Inout_ ArenaAllocator* alloc)
        {
            functions = functions->Prepend(addedFunction, alloc);
        }

        void AddField(_In_ Field* addedField, _Inout_ ArenaAllocator* alloc)
        {
            fields = fields->Prepend(addedField, alloc);
        }

        void AddImplementedInterface(_In_ Interface* addedImplementedInterface, _Inout_ ArenaAllocator* alloc)
        {
            implementedInterfaces = implementedInterfaces->Prepend(addedImplementedInterface, alloc);
        }

        void AddStaticField(_In_ Field* addedStaticField, _Inout_ ArenaAllocator* alloc)
        {
            staticFields = staticFields->Prepend(addedStaticField, alloc);
        }

        void AddStaticFunction(_In_ Function* addedStaticFunction, _Inout_ ArenaAllocator* alloc)
        {
            staticFunctions = staticFunctions->Prepend(addedStaticFunction, alloc);
        }

        void AddStaticFunctions(_In_ ImmutableList<Function*>* addedFunctions, _Inout_ ArenaAllocator* alloc)
        {
            addedFunctions->Iterate(
                [&](_In_ Function* addedFunction)
                {
                    AddStaticFunction(addedFunction, alloc);
                });
        }

        void AddFunctions(_In_ ImmutableList<Function*>* addedFunctions, _Inout_ ArenaAllocator* alloc)
        {
            addedFunctions->Iterate(
                [&](_In_ const Function* addedFunction)
                {
                    AddFunction((Function*) addedFunction, alloc);
                });
        }

        void AddEvent(_In_ Event* addedEvent, _Inout_ ArenaAllocator* alloc)
        {
            if (addedEvent)
            {
                events = events->Prepend(addedEvent, alloc);
            }
        }

        void AddStaticEvent(_In_ Event* addedStaticEvent, _Inout_ ArenaAllocator* alloc)
        {
            if (addedStaticEvent)
            {
                staticEvents = staticEvents->Prepend(addedStaticEvent, alloc);
            }
        }

        LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc)
        {
            LPCWSTR output = StringUtils::RemoveWhiteSpaces(name, alloc);
            return StringUtils::Replace(output, L'.', L'#', alloc);
        }

        LPCWSTR GetDocumentationTypePrefix()
        {
            return L"T:";
        }
    };

    class EnumEntry : virtual public Element
    {
    public:

        EnumEntry() {}
        EnumEntry(_In_ LPCWSTR name) : Element(name) {}

        LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc)
        {
            LPCWSTR output = StringUtils::RemoveWhiteSpaces(name, alloc);
            return StringUtils::CapitalizeFirstLetter(output, alloc);
        }

        LPCWSTR GetDocumentationTypePrefix()
        {
            return L"F:";
        }
    };

    class Enum : virtual public Element
    {
    public:
        ImmutableList<EnumEntry*>* entries = nullptr;

        Enum() {}
        Enum(_In_ LPCWSTR name) : Element(name) {}
        ~Enum()
        {
            DeleteListIfNotNull(alloc, entries, EnumEntry*);
        }

        void AddEntry(_In_ EnumEntry* addedEntry, _Inout_ ArenaAllocator* alloc)
        {
            entries = entries->Prepend(addedEntry, alloc);
        }

        LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc)
        {
            LPCWSTR output = StringUtils::RemoveWhiteSpaces(metadataName, alloc);
            return StringUtils::Replace(output, L'.', L'#', alloc);
        }

        LPCWSTR GetDocumentationTypePrefix()
        {
            return L"T:";
        }
    };

    class Module : virtual public Element
    {
    public:
        ImmutableList<Field*>* fields = nullptr;
        ImmutableList<Function*>* functions = nullptr;
        ImmutableList<Interface*>* interfaces = nullptr;
        ImmutableList<Class*>* classes = nullptr;
        ImmutableList<Enum*>* enums = nullptr;
        ImmutableList<Module*>* modules = nullptr;

        Module() {}
        Module(_In_ LPCWSTR name) : Element(name) {}
        ~Module()
        {
            DeleteListIfNotNull(alloc, fields, Field*);
            DeleteListIfNotNull(alloc, functions, Function*);
            DeleteListIfNotNull(alloc, interfaces, Interface*);
            DeleteListIfNotNull(alloc, classes, Class*);
            DeleteListIfNotNull(alloc, enums, Enum*);
            DeleteListIfNotNull(alloc, modules, Module*);
        }

        void AddModule(_In_ Module* addedModule)
        {
            modules = modules->Prepend(addedModule, alloc);
        }

        void AddClass(_In_ Class* addedClass)
        {
            classes = classes->Prepend(addedClass, alloc);
        }

        void AddEnum(_In_ Enum* addedEnum)
        {
            enums = enums->Prepend(addedEnum, alloc);
        }

        void AddInterface(_In_ Interface* addedInterface)
        {
            interfaces = interfaces->Prepend(addedInterface, alloc);
        }

        void AddFunction(_In_ Function* addedFunction)
        {
            functions = functions->Prepend(addedFunction, alloc);
        }

        void AddFunctions(_In_ ImmutableList<Function*>* addedFunctions)
        {
            addedFunctions->Iterate(
                [&](_In_ Function* addedFunction)
                {
                    AddFunction(addedFunction);
                });
        }

        void AddField(_In_ Field* addedField)
        {
            fields = fields->Prepend(addedField, alloc);
        }

        LPCWSTR GetNameInXmlFormat(_Inout_ ArenaAllocator* alloc)
        {
            LPCWSTR output = StringUtils::RemoveWhiteSpaces(name, alloc);
            return StringUtils::Replace(output, L'.', L'#', alloc);
        }

        LPCWSTR GetDocumentationTypePrefix()
        {
            return L"N:";
        }
    };

    class TSObjectModel : virtual public Wrapper::ObjectModel
    {
    public:
        ImmutableList<Module*>* modules = nullptr;
        
        TSObjectModel() {}
        TSObjectModel(_In_ ArenaAllocator* allocator, _In_ Wrapper::MetadataContext* resolver, _In_ Settings* settings, _In_ ProjectionModel::ProjectionBuilder* builder);
        ~TSObjectModel()
        {
            DeleteListIfNotNull(alloc, modules, Module*);
        }

    private:
        Module* ExploreNamespace(_In_ RtPROPERTIESOBJECT exploredNamespace, _In_ LPCWSTR name);
        Class* ExploreRuntimeClass(_In_ RtRUNTIMECLASSCONSTRUCTOR exploredClass, _In_ LPCWSTR name);
        Interface* ExploreRuntimeInterface(_In_ RtRUNTIMEINTERFACECONSTRUCTOR exploredInterface, _In_ LPCWSTR name);
        Interface* ExploreInterface(_In_ const ProjectionModel::InterfaceConstructor* exploredInterface, _In_ LPCWSTR name);
        Enum* ExploreEnum(_In_ RtENUM exploredEnum, _In_ LPCWSTR name);
        Function* ExploreFunctionParameters(_In_ ImmutableList<RtPARAMETER>* parameters, _In_ RtTYPE returnType, _In_ LPCWSTR name, const int retValIndex);
        Function* ExploreFunction(_In_ RtABIMETHODSIGNATURE prop, _In_ LPCWSTR name);
        ImmutableList<Function*>* ExploreOverloadFunctions(_In_ RtFUNCTION exploredOverloadFunctions);
        Function* ExploreFunctionProperty(_In_ RtPROPERTY functionProperty, _In_ LPCWSTR functionName);
        Field* ExploreField(_In_ ProjectionModel::RtABIPROPERTYPROPERTY prop, _In_ LPCWSTR name);
        TSWrapper::Type* ExploreType(_In_ RtTYPE exploredType);
        Interface* ExploreStruct(_In_ RtSTRUCTCONSTRUCTOR exploredStruct, _In_ LPCWSTR name);
        Field* ExploreArray(_In_ RtPROPERTY exploredArray, _In_ LPCWSTR name);
        Function* ExploreRuntimeClassConstructor(_In_ RtMETHODSIGNATURE methodSignature);
        Interface* ExploreDelegate(_In_ RtDELEGATECONSTRUCTOR exploredDelegate, _In_ LPCWSTR name);
        TSWrapper::Event* ExploreEvent(RtEVENT evnt);
        void AttachDocumentation(_In_ Module* documentedModule, _In_ LPCWSTR path);
        void AttachDocumentation(_In_ Class* documentedClass, _In_ LPCWSTR path);
        void AttachDocumentation(_In_ Enum* documentedEnum, _In_ LPCWSTR path);
        void AttachDocumentation(_In_ Interface* documentedInterface, _In_ LPCWSTR path);
        void AttachDocumentation(_In_ Function* documentedFunction, _In_ LPCWSTR path);
        void AttachDocumentation(_In_ Field* documentedField, _In_ LPCWSTR path);
        void AttachDocumentation(_In_ EnumEntry* documentedEnumEntry, _In_ LPCWSTR path);
        void AttachDocumentation(_In_ Event* documentedEvent, _In_ LPCWSTR path);

        void AnalyzeElementAttributes(_Inout_ Element* element, _In_ const Metadata::TypeDefProperties* typeDef);
    };
}