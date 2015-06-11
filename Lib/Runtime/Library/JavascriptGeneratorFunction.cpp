//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    FunctionInfo JavascriptGeneratorFunction::functionInfo(&JavascriptGeneratorFunction::EntryGeneratorFunctionImplementation, (FunctionInfo::Attributes)(FunctionInfo::DoNotProfile | FunctionInfo::SkipDefaultNewObject));

    JavascriptGeneratorFunction::JavascriptGeneratorFunction(DynamicType* type)
        : ScriptFunctionBase(type, &functionInfo),
        scriptFunction(nullptr)
    {
        // Constructor used during copy on write.
        DebugOnly(VerifyEntryPoint());
    }

    JavascriptGeneratorFunction::JavascriptGeneratorFunction(DynamicType* type, GeneratorVirtualScriptFunction* scriptFunction)
        : ScriptFunctionBase(type, &functionInfo),
        scriptFunction(scriptFunction)
    {
        DebugOnly(VerifyEntryPoint());
    }

    bool JavascriptGeneratorFunction::Is(Var var)
    {
        if (JavascriptFunction::Is(var))
        {
            JavascriptFunction* obj = JavascriptFunction::FromVar(var);

            return VirtualTableInfo<JavascriptGeneratorFunction>::HasVirtualTable(obj)
                || VirtualTableInfo<CrossSiteObject<JavascriptGeneratorFunction>>::HasVirtualTable(obj);
        }

        return false;
    }

    JavascriptGeneratorFunction* JavascriptGeneratorFunction::FromVar(Var var)
    {
        Assert(JavascriptGeneratorFunction::Is(var));

        return static_cast<JavascriptGeneratorFunction*>(var);
    }

    JavascriptGeneratorFunction* JavascriptGeneratorFunction::OP_NewScGenFunc(FrameDisplay *environment, FunctionProxy** proxyRef)
    {
        FunctionProxy* functionProxy = *proxyRef;
        ScriptContext* scriptContext = functionProxy->GetScriptContext();

        bool hasSuperReference = (functionProxy->GetAttributes() & Js::FunctionInfo::Attributes::HasSuperReference) ? true : false;
        bool isDefaultConstructor = (functionProxy->GetAttributes() & Js::FunctionInfo::Attributes::IsDefaultConstructor) ? true : false;

        AssertMsg(!isDefaultConstructor, "How is generator function is a default constructor?");

        GeneratorVirtualScriptFunction* scriptFunction = scriptContext->GetLibrary()->CreateGeneratorVirtualScriptFunction(functionProxy);
        scriptFunction->SetEnvironment(environment);
        scriptFunction->SetHasSuperReference(hasSuperReference);

        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_FUNCTION(scriptFunction, EtwTrace::GetFunctionId(functionProxy)));

        JavascriptGeneratorFunction* genFunc = scriptContext->GetLibrary()->CreateGeneratorFunction(functionInfo.GetOriginalEntryPoint(), scriptFunction);
        scriptFunction->SetRealGeneratorFunction(genFunc);

        return genFunc;
    }

    Var JavascriptGeneratorFunction::EntryGeneratorFunctionImplementation(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(stackArgs, callInfo);

        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptGeneratorFunction* generatorFunction = JavascriptGeneratorFunction::FromVar(function);

        // InterpreterStackFrame takes a pointer to the args, so copy them to the recycler heap
        // and use that buffer for this InterpreterStackFrame.
        // TODO[ianhall][generators]: Investigate if there are optimizations that assume these args are on the stack
        Var* argsHeapCopy = RecyclerNewArray(scriptContext->GetRecycler(), Var, stackArgs.Info.Count);
        js_memcpy_s(argsHeapCopy, sizeof(Var) * stackArgs.Info.Count, stackArgs.Values, sizeof(Var) * stackArgs.Info.Count);
        Arguments heapArgs(callInfo, argsHeapCopy);

        DynamicObject* prototype = scriptContext->GetLibrary()->CreateGeneratorConstructorPrototypeObject();
        JavascriptGenerator* generator = scriptContext->GetLibrary()->CreateGenerator(heapArgs, generatorFunction->scriptFunction, prototype);
        // Set the prototype from constructor
        JavascriptOperators::OrdinaryCreateFromConstructor(function, generator, prototype, scriptContext);
        if (callInfo.Flags & CallFlags_New)
        {
            // If the generator is invoked through new then set the this object as we skipped the new object creation in CallAsConstructor
            heapArgs.Values[0] = generator;
        }

        return generator;
    }

    Var JavascriptGeneratorFunction::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);

        return JavascriptFunction::NewInstanceHelper(function->GetScriptContext(), function, callInfo, args, /* isGenerator: */ true);
    }

    JavascriptGeneratorFunction* JavascriptGeneratorFunction::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        DynamicType* type = scriptContext->GetLibrary()->CreateDeferredPrototypeGeneratorFunctionType(functionInfo.GetOriginalEntryPoint());
        auto *result = RecyclerNew(scriptContext->GetRecycler(), CopyOnWriteObject<JavascriptGeneratorFunction>, type, this, scriptContext);

        Assert(this->GetFunctionInfo() == &functionInfo);
        Assert(result->GetFunctionInfo() == &functionInfo);

        // No need to make a copy of scriptFunction since it is private and cannot be mutated (effectively read-only)
        result->scriptFunction = this->scriptFunction;
        return result;
    }

    JavascriptString* JavascriptGeneratorFunction::GetDisplayNameImpl() const
    {
        return scriptFunction->GetDisplayNameImpl();
    }

    Var JavascriptGeneratorFunction::GetHomeObj() const
    {
        return scriptFunction->GetHomeObj();
    }

    void JavascriptGeneratorFunction::SetHomeObj(Var homeObj)
    {
        scriptFunction->SetHomeObj(homeObj);
    }

    void JavascriptGeneratorFunction::SetComputedNameVar(Var computedNameVar) 
    { 
        scriptFunction->SetComputedNameVar(computedNameVar);
    }

    Var JavascriptGeneratorFunction::GetComputedNameVar() const
    { 
        return scriptFunction->GetComputedNameVar(); 
    }

    
    Var JavascriptGeneratorFunction::GetSourceString() const
    {
        return scriptFunction->GetSourceString();
    }

    Var JavascriptGeneratorFunction::EnsureSourceString()
    {
        return scriptFunction->EnsureSourceString();
    }

    bool JavascriptGeneratorFunction::CloneMethod(JavascriptFunction** pnewMethod, const Var newHome)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        FunctionProxy* proxy = JavascriptOperators::GetDeferredDeserializedFunctionProxy(this->scriptFunction);

        GeneratorVirtualScriptFunction* scriptFunction = scriptContext->GetLibrary()->CreateGeneratorVirtualScriptFunction(proxy);
        scriptFunction->SetEnvironment(this->scriptFunction->GetEnvironment());
        scriptFunction->SetHasSuperReference(this->scriptFunction->HasSuperReference());
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_FUNCTION(scriptFunction, EtwTrace::GetFunctionId(proxy)));
        
        JavascriptGeneratorFunction* genFunc = scriptContext->GetLibrary()->CreateGeneratorFunction(this->GetFunctionInfo()->GetOriginalEntryPoint(), scriptFunction);
        scriptFunction->SetRealGeneratorFunction(genFunc);
        genFunc->SetHomeObj(newHome);

        *pnewMethod = genFunc;
        return true;
    }
}