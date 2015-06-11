//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{    
    ScriptFunctionType::ScriptFunctionType(ScriptFunctionType * type)
        : DynamicType(type), entryPointInfo(type->GetEntryPointInfo())
    {}

    ScriptFunctionType::ScriptFunctionType(ScriptContext* scriptContext, RecyclableObject* prototype, 
        JavascriptMethod entryPoint, ProxyEntryPointInfo * entryPointInfo, DynamicTypeHandler * typeHandler,
        bool isLocked, bool isShared)
        : DynamicType(scriptContext, TypeIds_Function, prototype, entryPoint, typeHandler, isLocked, isShared), 
        entryPointInfo(entryPointInfo)
    {

    }

    ScriptFunctionType * ScriptFunctionType::New(FunctionProxy * proxy, bool isShared)
    {
        Assert(proxy->GetFunctionProxy() == proxy);
        ScriptContext * scriptContext = proxy->GetScriptContext();
        JavascriptLibrary * library = scriptContext->GetLibrary();
        DynamicObject * functionPrototype = library->GetFunctionPrototype();
        JavascriptMethod address = (JavascriptMethod)proxy->GetDefaultEntryPointInfo()->address;
        return RecyclerNew(scriptContext->GetRecycler(), ScriptFunctionType,
            scriptContext, functionPrototype, 
            address,
            proxy->GetDefaultEntryPointInfo(),
            proxy->IsLambda() ?
                library->GetDeferredFunctionTypeHandler() :
                JavascriptLibrary::GetDeferredPrototypeFunctionTypeHandler(scriptContext),
            isShared, isShared);
    }
};