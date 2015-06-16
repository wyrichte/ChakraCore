//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include <Stdafx.h>
#include "JsrtExternalObject.h"
#include "JsrtCustomEnumerator.h"

JsrtCustomEnumerator::JsrtCustomEnumerator(Js::ScriptContext* scriptContext, bool enumNonEnumerable, bool enumSymbols, JsMoveNextCallback moveNext, JsGetCurrentCallback getCurrent, JsEndEnumerationCallback finish, void *data, JsrtExternalObject* externalObject):
    enumNonEnumerable(enumNonEnumerable),
    enumSymbols(enumSymbols),
    JavascriptEnumerator(scriptContext),
    moveNext(moveNext),
    getCurrent(getCurrent),
    finish(finish),
    data(data),
    externalObject(externalObject)
{
}

void JsrtCustomEnumerator::Finalize(bool isShutdown)
{
    if (getCurrent != nullptr)
    {
        if (!isShutdown)
        {
            this->finish(this->data);
            this->moveNext = nullptr;
            this->getCurrent = nullptr;
            this->finish = nullptr;
            this->data = nullptr;
        }
        else
        {
            LEAK_REPORT_PRINT(L"JsrtCustomEnumerator %p: Finalize not called on shutdown\n",
                this);
        }
    }
}

Var JsrtCustomEnumerator::GetCurrentIndex()
{
    Js::ScriptContext* scriptContext = GetScriptContext();
    Assert(scriptContext != nullptr);
    Var index = nullptr;

    if (this->getCurrent != null)
    {
        BEGIN_INTERCEPTOR(scriptContext)
        {
            index = (Var)this->getCurrent(this->data);
        }
        END_INTERCEPTOR(scriptContext);
    }

    if (index && (Js::JavascriptString::Is(index) || 
        (scriptContext->GetConfig()->IsES6SymbolEnabled() && Js::JavascriptSymbol::Is(index))))
    {
        return index;
    }

    return scriptContext->GetLibrary()->GetUndefined();
}

Var JsrtCustomEnumerator::GetCurrentValue()
{
    Assert(GetScriptContext() != nullptr);
    return GetScriptContext()->GetLibrary()->GetUndefined();
}

PropertyId JsrtCustomEnumerator::GetPropertyIdOfIndex(Var index)
{
    Js::ScriptContext* scriptContext = GetScriptContext();

    if (Js::JavascriptString::Is(index))
    {
        const Js::PropertyRecord *propertyRecord;
        Js::JavascriptString *propertyName = Js::JavascriptString::FromVar(index);

        scriptContext->GetOrAddPropertyRecord(propertyName->GetString(), propertyName->GetLength(), &propertyRecord);
        return propertyRecord->GetPropertyId();
    }
    else if (scriptContext->GetConfig()->IsES6SymbolEnabled() && Js::JavascriptSymbol::Is(index))
    {
        return Js::JavascriptSymbol::FromVar(index)->GetValue()->GetPropertyId();
    }
    
    return Js::Constants::NoProperty;
}

Var JsrtCustomEnumerator::GetCurrentAndMoveNext(PropertyId& propertyId, Js::PropertyAttributes* attributes)
{
    propertyId = Js::Constants::NoProperty;
    if (MoveNext(attributes))
    {
        Var currentIndex = GetCurrentIndex();
        propertyId = GetPropertyIdOfIndex(currentIndex);
        return currentIndex;
    }
    return NULL;
}

Var JsrtCustomEnumerator::GetCurrentBothAndMoveNext(PropertyId& propertyId, Var* currentValueRef)
{
    propertyId = Js::Constants::NoProperty;
    if (MoveNext())
    {
        Var currentIndex = GetCurrentIndex();
        *currentValueRef = GetCurrentValue();
        propertyId = GetPropertyIdOfIndex(currentIndex);
        return currentIndex;
    }
    return NULL;
}

BOOL JsrtCustomEnumerator::MoveNext(Js::PropertyAttributes* attributes)
{
    BOOL itemsAvailable = FALSE;
    Js::ScriptContext* scriptContext = GetScriptContext();
    Assert(scriptContext != nullptr);

    if (this->getCurrent != null)
    {
        BEGIN_INTERCEPTOR(scriptContext)
        {
            do
            {
                itemsAvailable = this->moveNext(this->data);
            } while (
                !enumSymbols && 
                itemsAvailable && 
                this->getCurrent && 
                scriptContext->GetConfig()->IsES6SymbolEnabled() &&
                Js::JavascriptSymbol::Is(this->getCurrent(this->data)));
        }
        END_INTERCEPTOR(scriptContext)
    }

    return itemsAvailable;
}

void JsrtCustomEnumerator::Reset()
{
    if (this->getCurrent == nullptr)
    {
        return;
    }

    this->finish(this->data);
    this->moveNext = nullptr;
    this->getCurrent = nullptr;
    this->finish = nullptr;
    this->data = nullptr;

    Js::ScriptContext* scriptContext = GetScriptContext();
    Assert(scriptContext != nullptr);

    BEGIN_INTERCEPTOR(scriptContext)
    {
        externalObject->GetEnumerateCallback()((JsValueRef)this->externalObject, this->enumNonEnumerable, &this->moveNext, &this->getCurrent, &this->finish, &this->data);
        Assert(this->moveNext != nullptr && this->getCurrent != nullptr && this->finish != nullptr);
    }
    END_INTERCEPTOR(scriptContext)
}
