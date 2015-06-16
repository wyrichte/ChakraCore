//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    RuntimeFunction::RuntimeFunction(DynamicType * type)
        : JavascriptFunction(type), functionNameId(null)
    {}

    RuntimeFunction::RuntimeFunction(DynamicType * type, FunctionInfo * functionInfo)
        : JavascriptFunction(type, functionInfo), functionNameId(null)
    {}

    RuntimeFunction::RuntimeFunction(DynamicType * type, FunctionInfo * functionInfo, ConstructorCache* cache)
        : JavascriptFunction(type, functionInfo, cache), functionNameId(null)
    {}

    Var
    RuntimeFunction::EnsureSourceString()
    {
        JavascriptLibrary* library = this->GetLibrary();
        ScriptContext * scriptContext = library->GetScriptContext();
        if (this->functionNameId == null)
        {
            this->functionNameId = library->GetFunctionDisplayString();
        }
        else
        {            
            if (TaggedInt::Is(this->functionNameId))
            {
                if (this->GetScriptContext()->GetConfig()->IsES6FunctionNameEnabled() && this->GetTypeHandler()->IsDeferredTypeHandler())
                {
                    this->SetPropertyWithAttributes(PropertyIds::name, this->GetDisplayName(true), PropertyConfigurable, nullptr);
                }
                this->functionNameId = GetNativeFunctionDisplayString(scriptContext, scriptContext->GetPropertyString(TaggedInt::ToInt32(this->functionNameId)));
            }          
        }
        Assert(JavascriptString::Is(this->functionNameId));
        return this->functionNameId;
    }

    void 
    RuntimeFunction::SetFunctionNameId(Var nameId)
    {
        Assert(functionNameId == NULL);
        Assert(TaggedInt::Is(nameId) || Js::JavascriptString::Is(nameId));

        // We are only reference the propertyId, it needs to be tracked to stay alive
        Assert(!TaggedInt::Is(nameId) || this->GetScriptContext()->IsTrackedPropertyId(TaggedInt::ToInt32(nameId)));        
        this->functionNameId = nameId;
    }

    bool RuntimeFunction::CloneMethod(JavascriptFunction** pnewMethod, const Var newHome)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        *pnewMethod = scriptContext->GetLibrary()->CloneBuiltinFunction(this);
        return true;
    }
};