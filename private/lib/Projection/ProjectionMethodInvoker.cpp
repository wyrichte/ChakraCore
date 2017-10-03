//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "ProjectionPch.h"

extern "C" void __cdecl _alloca_probe_16();
#ifdef _M_IX86
#ifdef _CONTROL_FLOW_GUARD
extern "C" PVOID __guard_check_icall_fptr;  
#endif
#endif 

namespace Projection
{
    using namespace ProjectionModel;

    bool IsSimpleActivatableFactory(RtABIMETHODSIGNATURE signature)
    {
        return signature->iid->instantiated==__uuidof(IActivationFactory) && signature->vtableIndex==0;
    }

    // Info:        Construct the method invoker
    // Parameters:  signature - model signature to invoke
    //              scriptEngine - script engine
    ProjectionMethodInvoker::ProjectionMethodInvoker(RtABIMETHODSIGNATURE signature, ProjectionContext *projectionContext) :
#ifdef _M_ARM32_OR_ARM64
        parameterLocations(nullptr),
#else
        stack(nullptr), 
#endif
        signature(signature),
        marshal(CalleeTransfersOwnership, projectionContext, false)
    {  
        // Since each arena allocator is atlease 4K in size, reuse the arena of projectionmarshaler - both are going to have same life time so it is safe to 
        // share the arena instead of using 2 pages of memory per method call when we could use 1page at a time.
#ifndef _M_ARM32_OR_ARM64
        stack = AnewArray(marshal.alloc, byte, signature->GetParameters()->sizeOfCallstack);
#endif
    }

    // Info:        Build the callstack with the given arguments
    // Parameters:  
    //   _this     - "this" pointer.
    //   arguments - Javascript arguments.
    void ProjectionMethodInvoker::BuildCallStack(IUnknown* _this, Js::Arguments arguments, bool isDelegate)
    {
#ifdef _M_ARM32_OR_ARM64
        this->callLayout.Clear();

        // 1st iteration: 
        // - determine location of each argument.
        // - determine size of data needed.
        // - allocate data for callLayout. 

        this->parameterLocations = AnewArrayZ(marshal.alloc, ParameterLocation, this->signature->GetParameters()->allParameters->Count());
        InitParameterLocations(this->parameterLocations);

#if _M_ARM
        this->callLayout.AllocateData(marshal.alloc, callingConvention.GetGeneralRegisterCount(),
            callingConvention.GetFloatRegisters(), callingConvention.GetStackSlotCount() * sizeof(void*));
#else
        this->callLayout.AllocateData(marshal.alloc, callingConvention.GetGeneralRegisterCount(),
            callingConvention.GetFloatRegisterCount(), callingConvention.GetStackSlotCount() * sizeof(void*));
#endif

        // 2nd iteration: fill-up callLayout: place each argument into its location.
#else
        memset(stack, 0, signature->GetParameters()->sizeOfCallstack);
        byte * location = stack;
#endif

        this->signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateN([&](int parameterLocationIndex, RtABIPARAMETER parameter) {
#ifdef _M_ARM32_OR_ARM64
            // Get the place where to write the value.
            byte* location = this->callLayout.GetParameterLocation(&this->parameterLocations[parameterLocationIndex]);
#endif

            Js::ScriptContext *scriptContext = marshal.projectionContext->GetScriptContext();

            // Throw if it is missing type or unconstructable type
            bool wasMissingType = false;
            if (!marshal.CanMarshalType(parameter->type, isDelegate, &wasMissingType))
            {
                Js::VerifyCatastrophic(signature->nameId != MetadataStringIdNil);
                Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidFunctionSignature, StringOfId(scriptContext, signature->nameId));
            }

            RtABIPARAMETER lengthParam = (ArrayType::Is(parameter->type) && parameter->IsArrayParameterWithLengthAttribute()) ? ((AbiArrayParameterWithLengthAttribute *)parameter)->GetLengthParameter(signature->GetParameters()->allParameters) : nullptr;
            if (wasMissingType)
            {
                if (parameter->isOut)
                {
                    marshal.WriteOutParameter(nullptr, parameter, location, sizeof(void *), signature->nameId);
                }
                else
                {
                    marshal.WriteInParameter(nullptr, parameter, location, sizeof(void *));
                }
            }
            else if (lengthParam != nullptr && lengthParam->isIn)
            {
                // PassArray or FillArray pattern with [in] lengthAttribute
                // We can only use [in] length attribute as thats the only one that has been applied at this point of the time
                
                AssertMsg(lengthParam->inParameterIndex < 65536, "Invalid metadata: ECMA-335 specifies parameter index as 2-byte integer.");
                uint32 lengthForArray = (uint32)Js::JavascriptConversion::ToInt32(arguments[(int)lengthParam->inParameterIndex + 1], scriptContext);
                marshal.WriteInArrayTypeIndividual(arguments[(int)parameter->inParameterIndex + 1], ArrayType::From(parameter->type), false, location, location + sizeof(LPVOID), parameter->isOut, true, lengthForArray);
            }
            else if (parameter->isOut)
            {
                AssertMsg(parameter->inParameterIndex < 65536, "Invalid metadata: ECMA-335 specifies parameter index as 2-byte integer.");
                Var inOutArg = parameter->isIn ? arguments[(int)parameter->inParameterIndex + 1] : nullptr;
                marshal.WriteOutParameter(inOutArg, parameter, location, parameter->GetSizeOnStack(), signature->nameId);
            } 
            else
            {
                AssertMsg(parameter->inParameterIndex < 65536, "Invalid metadata: ECMA-335 specifies parameter index as 2-byte integer.");
                marshal.WriteInParameter(arguments[(int)parameter->inParameterIndex + 1], parameter, location, parameter->GetSizeOnStack());
            }

#ifndef _M_ARM32_OR_ARM64
            //If we have a missing type, then we build up the stack as a null pointer of size(void *) rather than 0 (missing type)
            location += wasMissingType ? sizeof(void *) : parameter->GetSizeOnStack();
#endif

        });

#ifdef _M_ARM32_OR_ARM64
        // Place "this".
        *((IUnknown**)this->callLayout.GeneralRegisters) = _this;
#else
        Assert(0==signature->GetParameters()->sizeOfCallstack-(location-stack));
#endif
    }

    // Info:        Read the out parameters from the call stack
    // Parameters:  arguments - Javascript arguments
    Var ProjectionMethodInvoker::ReadOutParameters(Js::Arguments arguments, ParameterMarker* parameterMarker)
    {
        size_t outArgumentCount = signature->GetParameters()->allParameters->Count() - signature->inParameterCount;
        Js::ScriptContext *scriptContext = marshal.projectionContext->GetScriptContext();

        // create an object to aggregate all the out parameters. 
        Var instance = NULL;
        if (outArgumentCount == 0)
        {
            instance = scriptContext->GetLibrary()->GetUndefined();
        }
        else if (outArgumentCount > 1)
        {
            instance = scriptContext->GetLibrary()->CreateObject();
        }

        // Marshal out the parameters from the stack
#ifndef _M_ARM32_OR_ARM64
        byte * location = stack;
#endif
        signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateN([&](int parameterLocationIndex, RtABIPARAMETER parameter) 
        {
#ifdef _M_ARM32_OR_ARM64
            AssertMsg(this->parameterLocations, "At this time BuildCallStack should've been already called and parameterLocations should've been allocated.");
            Assert(&this->parameterLocations[parameterLocationIndex]);
            byte* location = this->callLayout.GetParameterLocation(&this->parameterLocations[parameterLocationIndex]);
#endif
            bool isMissingType = false;
            bool isMashalable = marshal.CanMarshalType(parameter->type, true, &isMissingType);
            Assert(isMashalable);

            if (parameter->isOut)
            {
                AssertMsg(parameter->inParameterIndex < 65536, "Invalid metadata: ECMA-335 specifies parameter index as 2-byte integer.");
                Var inOutArgument = parameter->isIn ? arguments[(int)parameter->inParameterIndex + 1] : nullptr;
                Var parameterValue = nullptr;
                if (isMissingType)
                {
                    parameterValue = scriptContext->GetLibrary()->GetNull();
                }
                else if (parameter->IsArrayParameterWithLengthAttribute())
                {
                    auto lengthParam = ((AbiArrayParameterWithLengthAttribute *)parameter)->GetLengthParameter(signature->GetParameters()->allParameters);
#ifdef _M_ARM32_OR_ARM64
                    byte *lengthParamLocation = this->callLayout.GetParameterLocation(&this->parameterLocations[((AbiArrayParameterWithLengthAttribute *)parameter)->lengthIsParameter]);
#else
                    byte *lengthParamLocation = stack;
                    signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateFirstN(((AbiArrayParameterWithLengthAttribute *)parameter)->lengthIsParameter, [&](RtABIPARAMETER parameter) {
                        lengthParamLocation += parameter->GetSizeOnStack();
                    });                        
#endif
                    Var lengthVar = (lengthParam->isIn) ? arguments[(int)lengthParam->inParameterIndex + 1] : marshal.ReadOutParameter(nullptr, lengthParam, lengthParamLocation, lengthParam->GetSizeOnStack(), signature->nameId);
        
                    uint32 lengthForArray = (unsigned __int32)Js::JavascriptConversion::ToInt32(lengthVar, marshal.projectionContext->GetScriptContext());

                    if (ArrayType::Is(parameter->type))
                    {
                        Assert(inOutArgument != nullptr);
                        parameterValue = marshal.ReadOutArrayType(inOutArgument, ArrayType::From(parameter->type), location, sizeof(uint), location + sizeof(LPVOID), sizeof(LPVOID), signature->nameId, false, true, false, true, lengthForArray);
                    }
                    else
                    {
                        Assert(ByRefType::Is(parameter->type) && ArrayType::Is(ByRefType::From(parameter->type)->pointedTo));
                        Assert(inOutArgument == nullptr);
                        parameterValue = marshal.ReadOutArrayType(nullptr, ArrayType::From(ByRefType::From(parameter->type)->pointedTo), *(byte**)location, sizeof(uint), *(byte**)(location + sizeof(LPVOID)), sizeof(LPVOID), signature->nameId, true, true, false, true, lengthForArray);
                    }
                }
                else
                {
                    if (parameterMarker && parameterMarker->type == ParameterMarkerType_Async && parameter == ((AsyncParameterMarker*)parameterMarker)->parameter)
                    {
                        PromiseConstructorArguments constructorArguments(((AsyncParameterMarker*)parameterMarker)->asyncOperationId);

                        parameterValue = marshal.ReadOutParameter(inOutArgument, parameter, location, parameter->GetSizeOnStack(), signature->nameId, true, false, &constructorArguments);
                    }
                    else
                    {
                        parameterValue = marshal.ReadOutParameter(inOutArgument, parameter, location, parameter->GetSizeOnStack(), signature->nameId);
                    }
                }


                if (!parameter->isIn)
                {
                    if (outArgumentCount == 1)
                    {
                        instance = parameterValue;
                    }
                    else
                    {
                        Assert(instance != NULL);
                        Js::PropertyRecord const * propRecord;
                        LPCWSTR parameterName = StringOfId(scriptContext, parameter->id);
                        scriptContext->GetOrAddPropertyRecord(parameterName, Js::JavascriptString::GetBufferLength(parameterName), &propRecord);
                        Assert(Js::Constants::NoProperty != propRecord->GetPropertyId());
                        Js::JavascriptOperators::OP_SetProperty(instance, propRecord->GetPropertyId(), parameterValue, scriptContext);                
                    }
                }
            }
#ifndef _M_ARM32_OR_ARM64
            location += isMissingType ? sizeof(void *) : parameter->GetSizeOnStack();
#endif
        });

#ifndef _M_ARM32_OR_ARM64
        Assert(0==signature->GetParameters()->sizeOfCallstack-(location-stack));
#endif
        return instance;
    }

    // Info:        Invoke the method through the given unknown and vtable offset
    //              Leaves result on context stack.
    //              Caller is suppose to call this function with _this addrefed, making sure it would stay alive for this function call. 
    //              It is responsibility of this function to release the _this ptr after the call is complete
    // Parameters:  _this - the unknown to invoke on
    //              vtableIndex - the vtable offset
    //              arguments - Javascript arguments
    //              isDefaultInterface - whether _this is a pointer to a default interface
    // Returns:     the HRESULT from the ABI call
    HRESULT ProjectionMethodInvoker::InvokeUnknown(IUnknown * _this, int vtableIndex, Js::Arguments arguments, bool isDefaultInterface, bool isDelegate)
    {
        IncrementInvokeCount();

        // If the interface is a default interface, we don't need to release 
        // since we already don't add ref the cached default interface for this call
        if (!isDefaultInterface)
        {
            // Release this unknown when the call is complete.
            this->marshal.RecordToUndo(_this, true);
        }
        Assert(signature->GetParameters()->sizeOfCallstack!=0xffffffff);
        void* methodAddress = (*((LPVOID**)_this))[vtableIndex];

        // Build callstack with in parameters (which can throw exception if the parameter conversion isnt supported) and allocate space for out parameters if needed
        BuildCallStack(_this, arguments, isDelegate);
        
        Js::ScriptContext *scriptContext = marshal.projectionContext->GetScriptContext();

        // Invoke method
#if defined(_M_IX86) || defined(_M_X64)
        AssertMsg(signature->GetParameters()->sizeOfCallstack < UINT32_MAX, "Size of call stack would exceed OS capabilities and parameter/index combination (only 64k parameters permitted).");
        uint argsSize = (uint)signature->GetParameters()->sizeOfCallstack;
#endif
        HRESULT hr = S_OK;

        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);

        BEGIN_LEAVE_SCRIPT(scriptContext)
        {

#if _M_IX86

            void *data;   
            void *savedEsp;

            __asm 
            {
                // Save ESP
                mov ecx, esp
                mov savedEsp, ecx
                mov eax, argsSize
                // Make sure we don't go beyond guard page
                cmp eax, 0x1000
                jge alloca_probe
                sub esp, eax
                jmp dbl_align
alloca_probe:
                // Use alloca to allocate more then a page size
                // Alloca assumes eax, contains size, and adjust ESP while
                // probing each page.
                call _alloca_probe_16
dbl_align:
                // 8-byte align frame to improve floating point perf of our JIT'd code.
                and esp, -8

                mov data, esp   
            }

            {
                DWORD* dest = (DWORD*)data;
                DWORD* src = (DWORD*)stack;
                for(unsigned int i =0; i < argsSize/sizeof(DWORD); i++)
                {
                    dest[i] = src[i];
                }
            }

#ifdef ENABLE_JS_ETW
            if (EventEnabledJSCRIPT_PROJECTION_RAWMETHODCALL_START())
            {
                LPCWSTR runtimeClassName = StringOfId(scriptContext, signature->runtimeClassNameId);
                LPCWSTR methodName = StringOfId(scriptContext, signature->nameId);
                EventWriteJSCRIPT_PROJECTION_RAWMETHODCALL_START(runtimeClassName, methodName);
            }
#endif

            __asm
            {
#ifdef _CONTROL_FLOW_GUARD
                // verify that the call target is valid
                mov  ecx, methodAddress
                call [__guard_check_icall_fptr]
#endif
            }

            // call variable argument function provided in entryPoint
            _asm
            {
                push _this
                call methodAddress

                // Restore ESP
                mov ecx, savedEsp
                mov esp, ecx

                // save the return value from realsum.
                mov hr, eax;
            }
#elif _M_X64
            if (EventEnabledJSCRIPT_PROJECTION_RAWMETHODCALL_START())
            {
                LPCWSTR runtimeClassName = StringOfId(scriptContext, signature->runtimeClassNameId);
                LPCWSTR methodName = StringOfId(scriptContext, signature->nameId);
                JS_ETW(EventWriteJSCRIPT_PROJECTION_RAWMETHODCALL_START(runtimeClassName, methodName));
            }
            hr = amd64_ProjectionCall(methodAddress, _this, stack, argsSize/sizeof(LPVOID));
#elif defined(_M_ARM)
            hr = arm_ProjectionCall(methodAddress, &this->callLayout);
#elif defined(_M_ARM64)
            hr = arm64_ProjectionCall(methodAddress, &this->callLayout);
#else
            AssertMsg(FALSE, "unsupported platform");
            Assert(methodAddress);  // To avoid varning C4189 local variable is initialized but not refereced error for now
            hr = E_FAIL;
#endif
#ifdef ENABLE_JS_ETW
            if (EventEnabledJSCRIPT_PROJECTION_RAWMETHODCALL_STOP())
            {
                LPCWSTR runtimeClassName = StringOfId(scriptContext, signature->runtimeClassNameId);
                LPCWSTR methodName = StringOfId(scriptContext, signature->nameId);
                EventWriteJSCRIPT_PROJECTION_RAWMETHODCALL_STOP(runtimeClassName, methodName);
            }
#endif
        }
        END_LEAVE_SCRIPT(scriptContext);

        return hr;
    }

    // Info:        Read out parameter or map and throw an exception
    // Parameters:  hr - the hresult
    //              boundsToUndefined - special handling for E_BOUND
    //              arguments - Javascript arguments
    Var ProjectionMethodInvoker::ReadOutOrThrow(HRESULT hr, bool boundsToUndefined, Js::Arguments args, ParameterMarker* parameterMarker)
    {
        if (FAILED(hr))
        {
            Js::ScriptContext *scriptContext = marshal.projectionContext->GetScriptContext();
            if(hr == E_BOUNDS && boundsToUndefined)
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
        }

        // Since the call was successful, all the out resources need to be released
        marshal.SetReleaseOutResources();

        Var result; 
        result = ReadOutParameters(args, parameterMarker);
        return result;
    }
}
