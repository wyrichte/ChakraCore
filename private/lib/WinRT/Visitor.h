//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//
// A visitor over the projection model, it copies current type only if one of the derived method creates new one.
//----------------------------------------------------------------------------
#pragma once
#include "Metadata.h"

namespace ProjectionModel
{
    struct Visitor
    {
        ArenaAllocator * a;
        Visitor(ArenaAllocator * a) : a(a) { }


        virtual RtTYPE VisitArrayType(RtARRAYTYPE type) sealed;
        virtual RtTYPE VisitMissingGenericInstantiationType(RtMISSINGGENERICINSTANTIATIONTYPE type) sealed; 
        virtual RtTYPE VisitGenericClassVarType(RtGENERICCLASSVARTYPE type) sealed;
        virtual RtTYPE VisitSystemGuidType(RtSYSTEMGUIDTYPE type) sealed;
        
        virtual RtTYPE VisitWindowsFoundationDateTimeType(RtWINDOWSFOUNDATIONDATETIMETYPE type) sealed;
        virtual RtTYPE VisitWindowsFoundationTimeSpanType(RtWINDOWSFOUNDATIONTIMESPANTYPE type) sealed;
        virtual RtTYPE VisitWindowsFoundationEventRegistrationTokenType(RtWINDOWSFOUNDATIONEVENTREGISTRATIONTOKENTYPE type) sealed;
        virtual RtTYPE VisitWindowsFoundationHResultType(RtWINDOWSFOUNDATIONHRESULTTYPE type) sealed;

        virtual RtTYPE VisitInterfaceType(RtINTERFACETYPE type) sealed;
        virtual RtTYPE VisitClassType(RtCLASSTYPE type) sealed;
        virtual RtTYPE VisitStructType(RtSTRUCTTYPE type) sealed;
        virtual RtTYPE VisitDelegateType(RtDELEGATETYPE type) sealed;
        virtual RtTYPE VisitEnumType(RtENUMTYPE type) sealed;
        virtual RtTYPE VisitByRefType(RtBYREFTYPE type) sealed;
        virtual RtTYPE VisitMissingNamedType(RtMISSINGNAMEDTYPE type) sealed;
        virtual RtTYPE VisitBasicType(RtBASICTYPE type) sealed;
        virtual RtTYPE VisitUnprojectableType(RtUNPROJECTABLETYPE type) sealed;
        virtual RtTYPE VisitVoidType(RtVOIDTYPE type) sealed;
        virtual RtTYPE VisitGenericParameterType(RtGENERICPARAMETERTYPE type) sealed;
        virtual RtEVENT VisitIndividual(RtEVENT evnt);
        virtual RtTYPE VisitIndividual(RtTYPE type);
        virtual RtEXPR VisitPropertiesObject(RtPROPERTIESOBJECT expr);
        virtual RtEXPR VisitAssignmentSpace(RtASSIGNMENTSPACE expr);
        virtual MetadataStringId VisitPropertyName(MetadataStringId nameId) sealed;
        virtual RtPROPERTY VisitAbiFieldProperty(RtABIFIELDPROPERTY prop) sealed;
        virtual RtPROPERTY VisitOverloadParentProperty(RtOVERLOADPARENTPROPERTY prop) sealed;
        virtual RtPROPERTY VisitAbiMethodProperty(RtABIMETHODPROPERTY prop) sealed;
        virtual RtPROPERTY VisitAbiPropertyProperty(RtABIPROPERTYPROPERTY prop) sealed;
        virtual RtPROPERTY VisitAbiArrayLengthProperty(RtABIARRAYLENGTHPROPERTY prop) sealed;
        virtual RtPROPERTY VisitAbiTypeProperty(RtABITYPEPROPERTY prop) sealed;
        virtual RtPROPERTY VisitAbiNamespaceProperty(RtABINAMESPACEPROPERTY prop) sealed;
        virtual RtPROPERTY VisitAbiAddEventListenerProperty(RtABIADDEVENTLISTENERPROPERTY prop);
        virtual RtPROPERTY VisitAbiRemoveEventListenerProperty(RtABIREMOVEEVENTLISTENERPROPERTY prop) sealed;
        virtual RtPROPERTY VisitAbiEventHandlerProperty(RtABIEVENTHANDLERPROPERTY prop) sealed;
        virtual RtPROPERTY VisitUnresolvableNameConflictProperty(RtUNRESOLVABLENAMECONFLICTPROPERTY prop) sealed;
        virtual RtPROPERTY VisitFunctionLengthProperty(RtFUNCTIONLENGTHPROPERTY prop) sealed;
        virtual RtPROPERTY VisitIndividual(RtPROPERTY prop);
        virtual RtASSIGNMENT VisitIndividual(RtASSIGNMENT assignment);
        virtual RtPARAMETERS VisitParameters(RtPARAMETERS parameters) sealed;
        virtual RtPARAMETER VisitIndividual(RtPARAMETER parameter);
        virtual RtABIMETHODSIGNATURE VisitIndividual(RtABIMETHODSIGNATURE signature);
        virtual RtMETHODSIGNATURE VisitTypeConstructorMethodSignature(const TypeConstructorMethodSignature * signature) sealed;
        virtual RtMETHODSIGNATURE VisitMissingTypeConstructorMethodSignature(const MissingTypeConstructorMethodSignature * signature) sealed;
        virtual RtMETHODSIGNATURE VisitOverloadedMethodSignature(const OverloadedMethodSignature * signature) sealed;
        virtual RtMETHODSIGNATURE VisitMissingDelegateInvokeMethodSignature(const MissingDelegateInvokeMethodSignature * signature) sealed;
        virtual RtMETHODSIGNATURE VisitGenericDelegateInvokeMethodSignature(const GenericDelegateInvokeMethodSignature * signature) sealed;
        virtual RtMETHODSIGNATURE VisitUncallableMethodSignature(const UncallableMethodSignature * signature) sealed;
        virtual RtMETHODSIGNATURE VisitMethodSignature(RtMETHODSIGNATURE signature) sealed; 
        virtual RtOVERLOADGROUP VisitOverloadGroup(RtOVERLOADGROUP overloads) sealed;
        virtual RtEXPR VisitOverloadGroupConstructor(const OverloadGroupConstructor * function); 
        virtual RtEXPR VisitAbiMethod(const AbiMethod * function) sealed; 
        virtual RtEXPR VisitSystemGuidConstructor(const SystemGuidConstructor * function) sealed; 

        virtual RtEXPR VisitWindowsFoundationDateTimeConstructor(const WindowsFoundationDateTimeConstructor * function) sealed;
        virtual RtEXPR VisitWindowsFoundationTimeSpanConstructor(const WindowsFoundationTimeSpanConstructor * function) sealed;
        virtual RtEXPR VisitWindowsFoundationEventRegistrationTokenConstructor(const WindowsFoundationEventRegistrationTokenConstructor * function);
        virtual RtEXPR VisitWindowsFoundationHResultConstructor(const WindowsFoundationHResultConstructor * function);

        virtual RtEXPR VisitBasicTypeConstructor(const BasicTypeConstructor * function) sealed; 
        virtual RtEXPR VisitNativeTypeConstructor(const NativeTypeConstructor * function) sealed;
        virtual RtEXPR VisitMissingTypeConstructor(const MissingTypeConstructor * function);
        virtual RtEXPR VisitStructConstructor(RtSTRUCTCONSTRUCTOR function) sealed;
        virtual RtEXPR VisitEnum(RtENUM _enum) sealed;
        virtual RtRUNTIMEINTERFACECONSTRUCTOR VisitRuntimeInterfaceConstructor(const RtRUNTIMEINTERFACECONSTRUCTOR iface) sealed;
        virtual const MissingInstantiationConstructor * VisitMissingInstantiationConstructor(const MissingInstantiationConstructor * iface) sealed;
        virtual const MissingInterfaceConstructor * VisitMissingInterfaceConstructor(const MissingInterfaceConstructor * iface) sealed;
        virtual RtINTERFACECONSTRUCTOR VisitIndividual(RtINTERFACECONSTRUCTOR constructor);
        virtual RtEXPR VisitGenericClassVarConstructor(const GenericClassVarConstructor * function) sealed;
        virtual RtEXPR VisitArrayConstructor(const ArrayConstructor * function) sealed;
        virtual RtEXPR VisitVoidConstructor(const VoidConstructor * function) sealed;
        virtual RtEXPR VisitRuntimeClassConstructor(const RtRUNTIMECLASSCONSTRUCTOR function) sealed;
        virtual RtEXPR VisitDelegateConstructor(RtDELEGATECONSTRUCTOR function) sealed;
        virtual RtEXPR VisitFunction(RtFUNCTION function) sealed;
        virtual RtEXPR VisitNullLiteral(RtEXPR expr);
        virtual RtEXPR VisitInt32lLiteral(const Int32Literal * expr) sealed;
        virtual RtEXPR VisitUInt32lLiteral(const UInt32Literal * expr) sealed;
        virtual RtEXPR VisitModOptExpr(const ModOptExpr * expr) sealed;
        virtual RtEXPR VisitByRefExpr(const ByRefExpr * expr) sealed;
        virtual RtEXPR VisitExpr(RtEXPR expr) sealed;

        template<class T>
        ImmutableList<const T *> * VisitImmutableList(ImmutableList<const T *> *list);
    };

    // Visitor which can extract all the ABI Method signatures from an expression
    struct ExtractSignatures : Visitor
    {
        ImmutableList<RtABIMETHODSIGNATURE> * signatures;
        MetadataStringId constructorNameId;
        MetadataStringId ctorMetadataId;
        ExtractSignatures(ArenaAllocator * a)
            : Visitor(a), signatures(nullptr)
        {

        }
        RtABIMETHODSIGNATURE VisitIndividual(RtABIMETHODSIGNATURE signature) override
        {
            auto areSameSignature = [&](RtABIMETHODSIGNATURE otherSignature) {
                return signature->vtableIndex==otherSignature->vtableIndex && signature->iid->instantiated == otherSignature->iid->instantiated;
            };

            // Only add unique signatures
            if (!signatures->ContainsWhere(areSameSignature))
            {
                signatures = signatures->Prepend(signature,a);
            }

            return Visitor::VisitIndividual(signature);
        }
    };
}

