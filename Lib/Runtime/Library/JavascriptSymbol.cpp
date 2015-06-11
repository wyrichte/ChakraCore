//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    Var JavascriptSymbol::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(SymbolCount);

        // SkipDefaultNewObject function flag should have revent the default object
        // being created, except when call true a host dispatch
        Assert(!(callInfo.Flags & CallFlags_New) || args[0] == null
            || JavascriptOperators::GetTypeId(args[0]) == TypeIds_HostDispatch);

        if (callInfo.Flags & CallFlags_New)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_ErrorOnNew, L"Symbol");
        }

        JavascriptString* description;

        if (args.Info.Count > 1)
        {
            description = JavascriptConversion::ToString(args[1], scriptContext);
        }
        else
        {
            description = scriptContext->GetLibrary()->GetEmptyString();
        }

        return scriptContext->GetLibrary()->CreateSymbol(description);
    }

    // Symbol.prototype.valueOf as described in ES6 spec (draft 22) 19.4.3.3
    Var JavascriptSymbol::EntryValueOf(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        if (JavascriptSymbol::Is(args[0]))
        {
            return args[0];
        }
        else if (JavascriptSymbolObject::Is(args[0]))
        {
            return scriptContext->GetLibrary()->CreateSymbol(JavascriptSymbolObject::FromVar(args[0])->GetValue());
        }
        else
        {
            return TryInvokeRemotelyOrThrow(EntryValueOf, scriptContext, args, JSERR_This_NeedSymbol, L"Symbol.prototype.valueOf");
        }
    }

    // Symbol.prototype.toString as described in ES6 spec (draft 22) 19.4.3.2
    Var JavascriptSymbol::EntryToString(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        AssertMsg(args.Info.Count, "Should always have implicit 'this'.");
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        const PropertyRecord* val;
        Var aValue = args[0];
        if (JavascriptSymbol::Is(aValue))
        {
            val = JavascriptSymbol::FromVar(aValue)->GetValue();
        }
        else if (JavascriptSymbolObject::Is(aValue))
        {
            val = JavascriptSymbolObject::FromVar(aValue)->GetValue();
        }
        else
        {
            return TryInvokeRemotelyOrThrow(EntryToString, scriptContext, args, JSERR_This_NeedSymbol, L"Symbol.prototype.toString");
        }

        return JavascriptSymbol::ToString(val, scriptContext);
    }

    // Symbol.for as described in ES6 spec (draft 22) 19.4.2.2
    Var JavascriptSymbol::EntryFor(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        AssertMsg(args.Info.Count, "Should always have implicit 'this'.");
        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        Assert(!(callInfo.Flags & CallFlags_New));

        JavascriptString* key;
        if (args.Info.Count > 1)
        {
            key = JavascriptConversion::ToString(args[1], scriptContext);
        }
        else
        {
            key = library->GetUndefinedDisplayString();
        }

        // Search the global symbol registration map for a symbol with description equal to the string key.
        // The map can only have one symbol with that description so if we found a symbol, that is the registered
        // symbol for the string key.
        const Js::PropertyRecord* propertyRecord = scriptContext->GetThreadContext()->GetSymbolFromRegistrationMap(key->GetString());

        // If we didn't find a PropertyRecord in the map, we'll create a new symbol with description equal to the key string.
        // This is the only place we add new PropertyRecords to the map, so we should never have multiple PropertyRecords in the 
        // map with the same string key value (since we would return the one we found above instead of creating a new one).
        if (propertyRecord == nullptr)
        {
            propertyRecord = scriptContext->GetThreadContext()->AddSymbolToRegistrationMap(key->GetString(), key->GetLength());
        }

        Assert(propertyRecord != nullptr);

        return library->CreateSymbol(propertyRecord);
    }

    // Symbol.keyFor as described in ES6 spec (draft 22) 19.4.2.7
    Var JavascriptSymbol::EntryKeyFor(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        AssertMsg(args.Info.Count, "Should always have implicit 'this'.");
        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count < 2 || !JavascriptSymbol::Is(args[1]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedSymbol, L"Symbol.keyFor");
        }

        JavascriptSymbol* sym = JavascriptSymbol::FromVar(args[1]);
        const wchar_t* key = sym->GetValue()->GetBuffer();

        // Search the global symbol registration map for a key equal to the description of the symbol passed into Symbol.keyFor.
        // Symbol.for creates a new symbol with description equal to the key and uses that key as a mapping to the new symbol.
        // There will only be one symbol in the map with that string key value.
        const Js::PropertyRecord* propertyRecord = scriptContext->GetThreadContext()->GetSymbolFromRegistrationMap(key);

        // If we found a PropertyRecord in the map, make sure it is the same symbol that was passed to Symbol.keyFor. 
        // If the two are different, it means the symbol passed to keyFor has the same description as a symbol registered via
        // Symbol.for _but_ is not the symbol returned from Symbol.for.
        if (propertyRecord != nullptr && propertyRecord == sym->GetValue())
        {
            return JavascriptString::NewCopyBuffer(key, sym->GetValue()->GetLength(), scriptContext);
        }

        return library->GetUndefined();
    }

    // Symbol.prototype[@@toPrimitive] as described in ES6 spec (draft 26) July 18 2014 19.4.3.4
    Var JavascriptSymbol::EntrySymbolToPrimitive(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        if (JavascriptSymbol::Is(args[0]))
        {
            return args[0];
        }
        else if (JavascriptSymbolObject::Is(args[0]))
        {
            return scriptContext->GetLibrary()->CreateSymbol(JavascriptSymbolObject::FromVar(args[0])->GetValue());
        }
        else
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_Invalid, L"Symbol[Symbol.toPrimitive]");
        }
    }

    RecyclableObject * JavascriptSymbol::CloneToScriptContext(ScriptContext* requestContext)
    {
        // PropertyRecords are per-ThreadContext so we can just create a new primitive wrapper 
        // around the PropertyRecord stored in this symbol via the other context library.
        return requestContext->GetLibrary()->CreateSymbol(this->GetValue());
    }

    Var JavascriptSymbol::TryInvokeRemotelyOrThrow(JavascriptMethod entryPoint, ScriptContext * scriptContext, Arguments & args, long errorCode, PCWSTR varName)
    {
        if (JavascriptOperators::GetTypeId(args[0]) == TypeIds_HostDispatch)
        {
            Var result;
            if (RecyclableObject::FromVar(args[0])->InvokeBuiltInOperationRemotely(entryPoint, args, &result))
            {
                return result;
            }
        }
        // Don't error if we disabled implicit calls
        if (scriptContext->GetThreadContext()->RecordImplicitException())
        {
            JavascriptError::ThrowTypeError(scriptContext, errorCode, varName);
        }
        else
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
    }
} // namespace Js
