//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace Js
{
    Var ArgumentsObject::GetCaller(ScriptContext * scriptContext)
    {
        JavascriptStackWalker walker(scriptContext);

        if (!this->AdvanceWalkerToArgsFrame(&walker))
        {
            return scriptContext->GetLibrary()->GetNull();
        }

        return ArgumentsObject::GetCaller(scriptContext, &walker, false);
    }

    Var ArgumentsObject::GetCaller(ScriptContext * scriptContext, JavascriptStackWalker *walker, bool skipGlobal)
    {
        // The arguments.caller property is equivalent to callee.caller.arguments - that is, it's the 
        // caller's arguments object (if any). Just fetch the caller and compute its arguments.
        JavascriptFunction* funcCaller = null;

        while (walker->GetCaller(&funcCaller))
        {
            if (walker->IsCallerGlobalFunction())
            {
                // Caller is global/eval. If we're in IE9 mode, and the caller is eval,
                // keep looking. Otherwise, caller is null.
                if (skipGlobal || walker->IsEvalCaller())
                {
                    continue;
                }
                funcCaller = null;
            }          
            break;
        }

        if (funcCaller == null || JavascriptOperators::GetTypeId(funcCaller) == TypeIds_Null)
        {
            return scriptContext->GetLibrary()->GetNull();
        }

        AssertMsg(JavascriptOperators::GetTypeId(funcCaller) == TypeIds_Function, "non function caller");

        CallInfo const *callInfo = walker->GetCallInfo();
        uint32 paramCount = callInfo->Count;
        CallFlags flags = callInfo->Flags;

        if (paramCount == 0 || (flags & CallFlags_Eval))
        {
            // The caller is the "global function" or eval, so we return "null".
            return scriptContext->GetLibrary()->GetNull();
        }
        
        if (!walker->GetCurrentFunction()->IsScriptFunction())
        {
            // builtin function do not have an argument object - return null.
            return scriptContext->GetLibrary()->GetNull();
        }

        Var args = walker->GetPermanentArguments();
        if (args == NULL)
        {
            args = JavascriptOperators::LoadHeapArguments(
                funcCaller, 
                paramCount - 1, 
                walker->GetJavascriptArgs(),
                scriptContext->GetLibrary()->GetNull(), 
                scriptContext->GetLibrary()->GetNull(), 
                scriptContext,
                /* formalsAreLetDecls */ false);

            walker->SetPermanentArguments(args);
        }

        return args;
    }

    bool ArgumentsObject::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_Arguments;
    }

    HeapArgumentsObject::HeapArgumentsObject(DynamicType * type) : ArgumentsObject(type), frameObject(null), formalCount(0), 
        numOfArguments(0), callerDeleted(false), deletedArgs(null)
    {
    }

    HeapArgumentsObject::HeapArgumentsObject(Recycler *recyler, ActivationObject* obj, uint32 formalCount, DynamicType * type)
        : ArgumentsObject(type), frameObject(obj), formalCount(formalCount), numOfArguments(0), callerDeleted(false), deletedArgs(null)
    {
    }

    void HeapArgumentsObject::SetNumberOfArguments(uint32 len)
    {
        numOfArguments = len;
    }

    uint32 HeapArgumentsObject::GetNumberOfArguments() const
    {
        return numOfArguments;
    }

    HeapArgumentsObject* HeapArgumentsObject::As(Var aValue)
    {
        if (ArgumentsObject::Is(aValue) && static_cast<ArgumentsObject*>(RecyclableObject::FromVar(aValue))->GetHeapArguments() == aValue)
        {
            return static_cast<HeapArgumentsObject*>(RecyclableObject::FromVar(aValue));
        }
        return NULL;
    }
    
    BOOL HeapArgumentsObject::AdvanceWalkerToArgsFrame(JavascriptStackWalker *walker)
    { 
        // Walk until we find this arguments object on the frame.
        // Note that each frame may have a HeapArgumentsObject
        // associated with it. Look for the HeapArgumentsObject.
        while (walker->Walk())
        {
            if (walker->IsJavascriptFrame() && walker->GetCurrentFunction()->IsScriptFunction())
            {
                Var args = walker->GetPermanentArguments();
                if (args == this)
                {
                    return TRUE;
                }
            }
        }

        return FALSE;
    }

    //
    // Get the next valid formal arg index held in this object.
    //
    uint32 HeapArgumentsObject::GetNextFormalArgIndex(uint32 index, BOOL enumNonEnumerable, PropertyAttributes* attributes) const
    {
        if (attributes != nullptr)
        {
            *attributes = PropertyEnumerable;
        }

        if ( ++index < formalCount )
        {
            // None of the arguments deleted
            if ( deletedArgs == null )
            {
                return index;
            }

            while ( index < formalCount )
            {
                if (!this->deletedArgs->Test(index))
                {
                    return index;
                }

                index++;
            }
        }

        return JavascriptArray::InvalidIndex;
    }

    BOOL HeapArgumentsObject::HasItemAt(uint32 index)
    {
        // If this arg index is bound to a named formal argument, get it from the local frame.
        // If not, report this fact to the caller, which will defer to the normal get-value-by-index means.
        if (IsFormalArgument(index) && (this->deletedArgs == null || !this->deletedArgs->Test(index)) )
        {
            return true;
        }

        return false;
    }

    BOOL HeapArgumentsObject::GetItemAt(uint32 index, Var* value, ScriptContext* requestContext)
    {
        // If this arg index is bound to a named formal argument, get it from the local frame.
        // If not, report this fact to the caller, which will defer to the normal get-value-by-index means.
        if (HasItemAt(index))
        {
            *value = this->frameObject->GetSlot(index);
            return true;
        }

        return false;
    }

    BOOL HeapArgumentsObject::SetItemAt(uint32 index, Var value)
    {
        // If this arg index is bound to a named formal argument, set it in the local frame.
        // If not, report this fact to the caller, which will defer to the normal set-value-by-index means.
        if (HasItemAt(index))
        {
            this->frameObject->SetSlot(SetSlotArguments(Constants::NoProperty, index, value));
            return true;
        }

        return false;
    }

    BOOL HeapArgumentsObject::DeleteItemAt(uint32 index)
    {
        if (index < formalCount)
        {
            if (this->deletedArgs == null)
            {
                Recycler *recyler = GetScriptContext()->GetRecycler();
                deletedArgs = RecyclerNew(recyler, BVSparse<Recycler>, recyler);
            }

            if (!this->deletedArgs->Test(index))
            {
                this->deletedArgs->Set(index);
                return true;
            }
        }

        return false;
    }

    BOOL HeapArgumentsObject::IsFormalArgument(uint32 index)
    {
        return index < this->numOfArguments && index < formalCount;
    }

    BOOL HeapArgumentsObject::IsFormalArgument(PropertyId propertyId)
    {
        uint32 index;
        return IsFormalArgument(propertyId, &index);
    }

    BOOL HeapArgumentsObject::IsFormalArgument(PropertyId propertyId, uint32* pIndex)
    {
        return 
            this->GetScriptContext()->IsNumericPropertyId(propertyId, pIndex) && 
            IsFormalArgument(*pIndex);
    }

    BOOL HeapArgumentsObject::IsArgumentDeleted(uint32 index) const
    {
        return this->deletedArgs != NULL && this->deletedArgs->Test(index);
    }

    ES5HeapArgumentsObject* HeapArgumentsObject::ConvertToStrictModeArgumentsObject(bool overwriteArgsUsingFrameObject)
    {
        ES5HeapArgumentsObject* es5ArgsObj = ConvertToES5HeapArgumentsObject(overwriteArgsUsingFrameObject);

        for (uint i=0; i<this->formalCount && i<numOfArguments; ++i)
        {
            es5ArgsObj->DisconnectFormalFromNamedArgument(i);
        }

        return es5ArgsObj;
    }

    ES5HeapArgumentsObject* HeapArgumentsObject::ConvertToES5HeapArgumentsObject(bool overwriteArgsUsingFrameObject)
    {
        if (!CrossSite::IsCrossSiteObjectTyped(this))
        {
            Assert(VirtualTableInfo<HeapArgumentsObject>::HasVirtualTable(this));
            VirtualTableInfo<ES5HeapArgumentsObject>::SetVirtualTable(this);
        }
        else
        {
            Assert(VirtualTableInfo<CrossSiteObject<HeapArgumentsObject>>::HasVirtualTable(this));
            VirtualTableInfo<CrossSiteObject<ES5HeapArgumentsObject>>::SetVirtualTable(this);
        }

        ES5HeapArgumentsObject* es5This = static_cast<ES5HeapArgumentsObject*>(this);

        if (overwriteArgsUsingFrameObject)
        {
            // Copy existing items to the array so that they are there before the user can call Object.preventExtensions,
            // after which we can no longer add new items to the objectArray.
            for (uint32 i = 0; i < this->formalCount && i < this->numOfArguments; ++i)
            {
                AssertMsg(this->IsFormalArgument(i), "this->IsFormalArgument(i)");
                if (!this->IsArgumentDeleted(i))
                {
                    // At this time the value doesn't matter, use one from slots.
                    this->SetObjectArrayItem(i, this->frameObject->GetSlot(i), PropertyOperation_None);
                }
            }
        }
        return es5This;

    }

    //---------------------- ES5HeapArgumentsObject -------------------------------   

    BOOL ES5HeapArgumentsObject::GetItemAt(uint32 index, Var* value, ScriptContext* requestContext)
    {
        return this->IsFormalDisconnectedFromNamedArgument(index) ?
            this->DynamicObject::GetItem(this, index, value, requestContext) :
            __super::GetItemAt(index, value, requestContext);
    }

    BOOL ES5HeapArgumentsObject::SetItemAt(uint32 index, Var value)
    {
        return this->IsFormalDisconnectedFromNamedArgument(index) ?
            this->DynamicObject::SetItem(index, value, PropertyOperation_None) :
            __super::SetItemAt(index, value);
    }

    BOOL ES5HeapArgumentsObject::DeleteItemAt(uint32 index)
    {
        BOOL result = __super::DeleteItemAt(index);
        if (result && IsFormalArgument(index))
        {
            AssertMsg(this->IsFormalDisconnectedFromNamedArgument(index), "__super::DeleteItemAt must perform the disconnect.");
            // Make sure that objectArray does not have the item ().
            if (this->HasObjectArrayItem(index))
            {
                this->DeleteObjectArrayItem(index, PropertyOperation_None);
            }
        }

        return result;
    }

    //
    // Get the next valid formal arg index held in this object.
    //
    uint32 ES5HeapArgumentsObject::GetNextFormalArgIndex(uint32 index, BOOL enumNonEnumerable, PropertyAttributes* attributes) const
    {
        return GetNextFormalArgIndexHelper(index, enumNonEnumerable, attributes);
    }

    uint32 ES5HeapArgumentsObject::GetNextFormalArgIndexHelper(uint32 index, BOOL enumNonEnumerable, PropertyAttributes* attributes) const
    {
        // Formals:
        // - deleted => not in objectArray && not connected -- do not enum, do not advance.
        // - connected,     if in objectArray -- if (enumerable) enum it, advance objectEnumerator.
        // - disconnected =>in objectArray -- if (enumerable) enum it, advance objectEnumerator.

        // We use GetFormalCount and IsEnumerableByIndex which don't change the object 
        // but are not declared as const, so do a const cast.
        ES5HeapArgumentsObject* mutableThis = const_cast<ES5HeapArgumentsObject*>(this);
        uint32 formalCount = this->GetFormalCount();
        while (++index < formalCount)
        {
            bool isDeleted = mutableThis->IsFormalDisconnectedFromNamedArgument(index) && 
                !mutableThis->HasObjectArrayItem(index);

            if (!isDeleted)
            {
                BOOL isEnumerable = mutableThis->IsEnumerableByIndex(index);

                if (enumNonEnumerable || isEnumerable)
                {
                    if (attributes != nullptr && isEnumerable)
                    {
                        *attributes = PropertyEnumerable;
                    }

                    return index;
                }
            }
        }

        return JavascriptArray::InvalidIndex;
    }

    // Disconnects indexed argument from named argument for frame object property.
    // Remove association from them map. From now on (or still) for this argument,
    // named argument's value is no longer associated with agruments[] item.
    void ES5HeapArgumentsObject::DisconnectFormalFromNamedArgument(uint32 index)
    { 
        AssertMsg(this->IsFormalArgument(index), "SetAccessorsForFormal: called for non-formal");

        if (!IsFormalDisconnectedFromNamedArgument(index))
        {
            __super::DeleteItemAt(index);
        }
    }

    BOOL ES5HeapArgumentsObject::IsFormalDisconnectedFromNamedArgument(uint32 index) 
    {
        return this->IsArgumentDeleted(index);
    }

    // Wrapper over IsEnumerable by uint32 index.
    BOOL ES5HeapArgumentsObject::IsEnumerableByIndex(uint32 index)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        Var indexNumber = JavascriptNumber::New(index, scriptContext);
        JavascriptString* indexPropertyName = JavascriptConversion::ToString(indexNumber, scriptContext);
        PropertyRecord const * propertyRecord;
        scriptContext->GetOrAddPropertyRecord(indexPropertyName->GetString(), indexPropertyName->GetLength(), &propertyRecord);
        return this->IsEnumerable(propertyRecord->GetPropertyId());
    }

    // Helper method, just to avoid code duplication.
    BOOL ES5HeapArgumentsObject::SetConfigurableForFormal(uint32 index, PropertyId propertyId, BOOL value)
    {
        AssertMsg(this->IsFormalArgument(index), "SetAccessorsForFormal: called for non-formal");

        // In order for attributes to work, the item must exist. Make sure that's the case.
        // If we were connected, we have to copy the value from frameObject->slots, otherwise which value doesn't matter.
        AutoObjectArrayItemExistsValidator autoItemAddRelease(this, index);

        BOOL result = this->DynamicObject::SetConfigurable(propertyId, value);
        autoItemAddRelease.m_isReleaseItemNeeded = !result;

        return result;
    }

    // Helper method, just to avoid code duplication.
    BOOL ES5HeapArgumentsObject::SetEnumerableForFormal(uint32 index, PropertyId propertyId, BOOL value)
    {
        AssertMsg(this->IsFormalArgument(index), "SetAccessorsForFormal: called for non-formal");

        AutoObjectArrayItemExistsValidator autoItemAddRelease(this, index);

        BOOL result = this->DynamicObject::SetEnumerable(propertyId, value);
        autoItemAddRelease.m_isReleaseItemNeeded = !result;

        return result;
    }

    // Helper method, just to avoid code duplication.
    BOOL ES5HeapArgumentsObject::SetWritableForFormal(uint32 index, PropertyId propertyId, BOOL value)
    {
        AssertMsg(this->IsFormalArgument(index), "SetAccessorsForFormal: called for non-formal");

        AutoObjectArrayItemExistsValidator autoItemAddRelease(this, index);

        BOOL isDisconnected = IsFormalDisconnectedFromNamedArgument(index);
        if (!isDisconnected && !value)
        {
            // Settings writable to false causes disconnect.
            // It will be too late to copy the value after setting writable = false, as we would not be able to. 
            // Since we are connected, it does not matter the value is, so it's safe (no matter if SetWritable fails) to copy it here.
            this->SetObjectArrayItem(index, this->frameObject->GetSlot(index), PropertyOperation_None);
        }

        BOOL result = this->DynamicObject::SetWritable(propertyId, value); // Note: this will convert objectArray to ES5Array.
        if (result && !value && !isDisconnected)
        {
            this->DisconnectFormalFromNamedArgument(index);
        }
        autoItemAddRelease.m_isReleaseItemNeeded = !result;

        return result;
    }

    // Helper method for SetPropertyWithAttributes, just to avoid code duplication.
    BOOL ES5HeapArgumentsObject::SetAccessorsForFormal(uint32 index, PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
        AssertMsg(this->IsFormalArgument(index), "SetAccessorsForFormal: called for non-formal");

        AutoObjectArrayItemExistsValidator autoItemAddRelease(this, index);

        BOOL result = this->DynamicObject::SetAccessors(propertyId, getter, setter, flags);
        if (result)
        {
            this->DisconnectFormalFromNamedArgument(index);
        }
        autoItemAddRelease.m_isReleaseItemNeeded = !result;

        return result;
    }

    // Helper method for SetPropertyWithAttributes, just to avoid code duplication.
    BOOL ES5HeapArgumentsObject::SetPropertyWithAttributesForFormal(uint32 index, PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        AssertMsg(this->IsFormalArgument(propertyId), "SetPropertyWithAttributesForFormal: called for non-formal");

        BOOL result = this->DynamicObject::SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);
        if (result)
        {
            if ((attributes & PropertyWritable) == 0)
            {
                // Setting writable to false should cause disconnect.
                this->DisconnectFormalFromNamedArgument(index);
            }

            if (!this->IsFormalDisconnectedFromNamedArgument(index))
            {
                // Update the (connected with named param) value, as above call could cause change of the value.
                BOOL tempResult = this->SetItemAt(index, value);  // Update the value in frameObject.
                AssertMsg(tempResult, "this->SetItem(index, value)");
            }
        }

        return result;
    }

    //---------------------- ES5HeapArgumentsObject::AutoObjectArrayItemExistsValidator -------------------------------   
    ES5HeapArgumentsObject::AutoObjectArrayItemExistsValidator::AutoObjectArrayItemExistsValidator(ES5HeapArgumentsObject* args, uint32 index)
        : m_args(args), m_index(index), m_isReleaseItemNeeded(false)
    {
        AssertMsg(args, "args");
        AssertMsg(args->IsFormalArgument(index), "AutoObjectArrayItemExistsValidator: called for non-formal");

        if (!args->HasObjectArrayItem(index))
        {
            m_isReleaseItemNeeded = args->SetObjectArrayItem(index, args->frameObject->GetSlot(index), PropertyOperation_None) != FALSE;
        }
    }
        
    ES5HeapArgumentsObject::AutoObjectArrayItemExistsValidator::~AutoObjectArrayItemExistsValidator()
    {
        if (m_isReleaseItemNeeded)
        {
           m_args->DeleteObjectArrayItem(m_index, PropertyOperation_None);        
        }
    }

    HeapArgumentsObject* HeapArgumentsObject::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler *recycler = scriptContext->GetRecycler();
        CopyOnWriteObject<HeapArgumentsObject> * result =
            RecyclerNew(recycler, CopyOnWriteObject<HeapArgumentsObject>, scriptContext->GetLibrary()->GetHeapArgumentsObjectType(), this, scriptContext);

        scriptContext->RecordCopyOnWrite(this, result); // We record the copy before detaching to handle cycles.

        // Copy the internal fields.
        result->numOfArguments = numOfArguments;
        result->callerDeleted = callerDeleted;
        result->formalCount = formalCount;
        result->frameObject = (Js::ActivationObject *)scriptContext->CopyOnWrite(frameObject);
        if (deletedArgs)
            result->deletedArgs = deletedArgs->CopyNew(recycler);

        // The arguments object needs to be detached immediately because is holds on to an activation object
        // and directly references its content.
        result->Detach();

        return result;
    }
}
