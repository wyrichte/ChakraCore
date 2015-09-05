//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "WinRTPch.h"

namespace ProjectionModel
{
    using namespace Js;

    // Info:        Return true if these are the same simple interface constructor
    // Parameters:  a - one element
    //              b - other element
    bool AreSameRuntimeInterfaceConstructor(RtRUNTIMEINTERFACECONSTRUCTOR a, RtRUNTIMEINTERFACECONSTRUCTOR b)
    {
        if (a->iid->instantiated == IID_NULL && b->iid->instantiated == IID_NULL)
        {
            return a->typeId == b->typeId;
        }
        return a->iid->instantiated == b->iid->instantiated ? true : false;
    }

    // Info:        Return true if these are the same type constructor
    // Parameters:  a - one element
    //              b - other element
    bool AreSameType(RtTYPE a, RtTYPE b)
    {
        if (a == b)
        {
            return true;
        }
        if (a->typeCode!=b->typeCode)
        {
            return false;
        }
        if (TypeDefinitionType::Is(a))
        {
            auto tdta = TypeDefinitionType::From(a);
            auto tdtb = TypeDefinitionType::From(a);
            if (tdta->typeDef->td != tdtb->typeDef->td)
            {
                return false;
            }
            if (tdta->genericParameters->Count()!=tdtb->genericParameters->Count())
            {
                return false;
            }

            auto ac = tdta->genericParameters;
            auto bc = tdtb->genericParameters;
            while(ac)
            {
                if (!AreSameType(ac->First(), bc->First()))
                {
                    return false;
                }

                ac=ac->GetTail();
                bc=bc->GetTail();
            }
            return true;
        }
        Assert(0);
        return false;
    }
    // Info:        Return true if these are the same MissingInstantiationConstructor
    // Parameters:  a - one element
    //              b - other element
    bool AreSameMissingInstantiationConstructor(const MissingInstantiationConstructor * a, const MissingInstantiationConstructor * b) 
    {
        return (a->instantiatedGenericType->instantiatedNameId == b->instantiatedGenericType->instantiatedNameId);
    }

    // Info:        Return true if these are the same interface constructor
    // Parameters:  a - one element
    //              b - other element
    bool AreSameInterfaceConstructor(RtINTERFACECONSTRUCTOR a, RtINTERFACECONSTRUCTOR b)
    {
        if (a->interfaceType!=b->interfaceType)
        {
            return false;
        }
        switch(a->interfaceType)
        {
        case ifRuntimeInterfaceConstructor:
            return AreSameRuntimeInterfaceConstructor(RuntimeInterfaceConstructor::From(a), RuntimeInterfaceConstructor::From(b));
        case ifMissingInterfaceConstructor:
            {
                auto sa = MissingInterfaceConstructor::From(a);
                auto sb = MissingInterfaceConstructor::From(b);
                return sa->typeId == sb->typeId;
            }
        case ifMissingInstantiationConstructor:
            {
                auto sia = MissingInstantiationConstructor::From(a);
                auto sib = MissingInstantiationConstructor::From(b);
                return AreSameMissingInstantiationConstructor(sia, sib);
            }
        }
        Assert(0);
        throw 0;
    }

    // Info:        Return true if these are the same function
    // Parameters:  a - one element
    //              b - other element
    bool AreSameFunction(Function * a, Function * b)
    {
        if ((LPVOID)a == (LPVOID)b)
        {
            return true;
        }
        if (a->functionType!=b->functionType)
        {
            return false;
        }

        switch(a->functionType)
        {
            case functionInterfaceConstructor: 
                return AreSameInterfaceConstructor(static_cast<InterfaceConstructor*>(a),static_cast<InterfaceConstructor*>(b));
        }
        Assert(0);
        throw 0;
    }

    // Info:        Rename a property's identifier
    // Parameters:  prop - The property
    //              newName - new identifier name
    RtPROPERTY CreateRenamedPropertyIdentifierAsCopy(RtPROPERTY prop, MetadataStringId newId, ArenaAllocator * a)
    {
        switch (prop->propertyType)
        {
        case ptAbiFieldProperty:
            {
                auto p = AbiFieldProperty::From(prop);
                return RtAnew(a, AbiFieldProperty, newId, p->expr, p->type, p->fieldProperties, p->fieldOffset);
            }
        case ptOverloadParentProperty:
            {
                auto p = OverloadParentProperty::From(prop);

                ImmutableList<RtABIMETHODSIGNATURE> *arityGroups = ImmutableList<RtABIMETHODSIGNATURE>::Empty();
                auto tail = arityGroups;
                p->overloadConstructor->signature->overloads->overloads->Iterate([&](RtABIMETHODSIGNATURE oldSignature) {
                    auto newSignature = Anew(a, AbiMethodSignature, newId, oldSignature);
                    arityGroups = arityGroups->Append(newSignature, a, &tail);
                });

                auto overloadGroup = Anew(a, OverloadGroup, newId, arityGroups);
                auto overloadMethodSignature = Anew(a, OverloadedMethodSignature, newId, overloadGroup, p->overloadConstructor->signature->parameters);
                auto overloadGroupConstructor = Anew(a, OverloadGroupConstructor, overloadMethodSignature, p->overloadConstructor->properties);
                return RtAnew(a, OverloadParentProperty, newId, overloadGroupConstructor);
            }
        case ptAbiMethodProperty:
            {
                auto p = AbiMethodProperty::From(prop);
                auto oldSignature = p->body->signature;
                auto newSignature = Anew(a, AbiMethodSignature, newId, oldSignature);
                auto newBody = Anew(a, AbiMethod, newSignature, p->body->properties);
                return RtAnew(a, AbiMethodProperty, newId, newBody);
            }
        case ptAbiPropertyProperty:
            {
                auto p = AbiPropertyProperty::From(prop);
                return RtAnew(a, AbiPropertyProperty, newId, p->expr, p->getter, p->setter, p->metadataNameId);
            }
        case ptAbiArrayLengthProperty:
            {
                Assert(0); // There shouldn't be a need to rename array length property. 
                return prop;
            }
        case ptAbiTypeProperty:
            {
                auto p = AbiTypeProperty::From(prop);
                return RtAnew(a, AbiTypeProperty, newId, p->expr);
            }
        case ptAbiNamespaceProperty:
            {
                auto p = AbiNamespaceProperty::From(prop);
                return RtAnew(a, AbiNamespaceProperty, newId, p->childNamespace);
            }
        case ptAbiAddEventListenerProperty:
        case ptAbiRemoveEventListenerProperty:
        case ptAbiEventHandlerProperty:
            {
                // No need to rename addEventListener, removeEventListener or eventhandler that attach/detach event with event handlers
                Assert(0); 
                return prop;
            }
        case ptUnresolvableNameConflictProperty:
            {
                auto p = UnresolvableNameConflictProperty::From(prop);
                return RtAnew(a, UnresolvableNameConflictProperty, newId, p->expr, p->conflictingProperties);
            }
        case ptFunctionLengthProperty:
            {
                // Do not allow rename of function length
                return prop;
            }
        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Rename the given event.
    // Parameters:  eventToRename - the event to rename
    //              newName - the new name
    RtEVENT RenameEventName(RtEVENT eventToRename, MetadataStringId newId, ArenaAllocator * a
#if DBG
        , LPCWSTR newNameStr
#endif
        )
    {
#if DBG
        return Anew(a, Event, eventToRename->addOn, eventToRename->removeOn, eventToRename->metadataNameId, newId, newNameStr);
#else
        return Anew(a, Event, eventToRename->addOn, eventToRename->removeOn, eventToRename->metadataNameId, newId);
#endif
    }

    // Info:        Search the given interface constructor for a required interface with the given piid
    // Parameters:  interfaceConstructor - constructor to search
    //              piid - the PIID of the interface to search
    RtRUNTIMEINTERFACECONSTRUCTOR FindRequiredMatchingInterfaceByPiid(RtRUNTIMEINTERFACECONSTRUCTOR interfaceConstructor, const IID & piid)
    {
        if (interfaceConstructor->iid->piid == piid)
        {
            return interfaceConstructor;
        }

        auto next = interfaceConstructor->requiredInterfaces;
        while(next)
        {
            auto required = next->First();
            if (RuntimeInterfaceConstructor::Is(required))
            {
                auto rt = RuntimeInterfaceConstructor::From(required);
                if (rt->iid->piid == piid)
                {
                    return rt;
                }
            }
            next = next->GetTail();
        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Find a matching method signautre in the given set of interfaces and extra parent properties
    // Parameters:  name - the method name
    //              instantiated - the instantiated IID to search for
    //              properties - the parent properties to search
    ImmutableList<RtABIMETHODSIGNATURE> * FindMatchingMethodSignatures(MetadataStringId nameId, const IID & instantiated, ImmutableList<RtPROPERTY> * properties, ArenaAllocator * a)
    {
        struct Finder : Visitor 
        {
            MetadataStringId nameId;
            const IID & instantiated;
            ImmutableList<RtABIMETHODSIGNATURE> * found;
            ArenaAllocator * a;
            Finder(MetadataStringId nameId, const IID & instantiated, ArenaAllocator * a) 
                : Visitor(a), nameId(nameId), instantiated(instantiated), a(a), found(nullptr)
            { }
            virtual RtABIMETHODSIGNATURE VisitIndividual(RtABIMETHODSIGNATURE signature)
            {
                if (instantiated == signature->iid->instantiated && (signature->metadataNameId == nameId))
                {
                    found = found->Prepend(signature,a);
                }
                return signature;
            }
        };

        Finder finder(nameId, instantiated, a);
        finder.VisitImmutableList(properties);
        return finder.found;
    }

}