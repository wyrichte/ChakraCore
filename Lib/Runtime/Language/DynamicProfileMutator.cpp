//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include <StdAfx.h>

#ifdef DYNAMIC_PROFILE_MUTATOR

char const * const DynamicProfileMutator::CreateMutatorProcName = STRINGIZE(CREATE_MUTATOR_PROC_NAME);

void
DynamicProfileMutator::Mutate(Js::FunctionBody * functionBody)
{
    Js::ScriptContext * scriptContext = functionBody->GetScriptContext();
    DynamicProfileMutator * dynamicProfileMutator = scriptContext->GetThreadContext()->dynamicProfileMutator;
    if (dynamicProfileMutator != null)
    {            
        if (functionBody->dynamicProfileInfo == null)
        {
            functionBody->dynamicProfileInfo = Js::DynamicProfileInfo::New(scriptContext->GetRecycler(), functionBody);           
        }

        dynamicProfileMutator->Mutate(functionBody->dynamicProfileInfo);
        // Save the profile information, it will help in case of Crash/Failure
        Js::DynamicProfileInfo::Save(scriptContext);
    }
}

DynamicProfileMutator *
DynamicProfileMutator::GetMutator()
{
    if (!Js::Configuration::Global.flags.IsEnabled(Js::DynamicProfileMutatorFlag))
    {
        return null;
    }

    wchar_t const * dllname = Js::Configuration::Global.flags.DynamicProfileMutatorDll;
    HMODULE hModule = ::LoadLibraryW(dllname);
    if (hModule == null)
    {
        Output::Print(L"ERROR: Unable to load dynamic profile mutator dll %s\n", dllname);
        Js::Throw::FatalInternalError();
    }

    CreateMutatorFunc procAddress = (CreateMutatorFunc)::GetProcAddress(hModule, CreateMutatorProcName);

    if (procAddress == null)
    {
        Output::Print(L"ERROR: Unable to get function %S from dll %s\n", CreateMutatorProcName, dllname);
        Js::Throw::FatalInternalError();
    }

    DynamicProfileMutator * mutator = procAddress();
    if (mutator == null)
    {
        Output::Print(L"ERROR: Failed to create mutator from dll %s\n", dllname);
        Js::Throw::FatalInternalError();
    }
    mutator->Initialize(Js::Configuration::Global.flags.DynamicProfileMutator);
    return mutator;    
}
#endif