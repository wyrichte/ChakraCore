//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "SCALookupPch.h"

// Known internal property names
static const WCHAR PROPERTYNAME_LENGTH[]        = L"length";
static const WCHAR PROPERTYNAME_SOURCE[]        = L"source";
static const WCHAR PROPERTYNAME_GLOBAL[]        = L"global";
static const WCHAR PROPERTYNAME_IGNORECASE[]    = L"ignoreCase";
static const WCHAR PROPERTYNAME_MULTILINE[]     = L"multiline";

//
// Test if a given name equals a known string exactly.
//
template <UINT size>
bool Equals(_In_reads_(nameLen) LPCWSTR name, UINT nameLen, const WCHAR (&str)[size])
{
    static_assert(size > 0, "size includes null terminator");

    return nameLen == size - 1 && wmemcmp(name, str, nameLen) == 0;
}

//
// Test if a given name equals a known string exactly.
//
template <UINT size>
bool Equals(_In_ BSTR name, const WCHAR (&str)[size])
{
    return Equals(name, ::SysStringLen(name), str);
}

// Here the destructor servers a very important purpose, it cleans up the linked list non-recursively.
// In addition, it must clean SAFEARRAY of references to other SAFEARRAYs, before deleting them.
ExistingArrayNode::~ExistingArrayNode()
{
    if (m_safeArray == nullptr)
    {
        return;
    }

    CComSafeArray<VARIANT, VT_VARIANT> currentArray(this->m_safeArray.Detach());
    while (currentArray != nullptr)
    {
        for (ULONG i = 0; i < currentArray.GetCount(); i++)
        {
            if (currentArray.GetAt(static_cast<LONG>(i)).vt == (VT_ARRAY | VT_VARIANT))
            {
                // This simple clears the item, and does *NOT* clean up the memory.
                // Reason for that, is because we could have very deeply nested array's and stack overflow here,
                // Instead we unravel the linked-list here and clean it up in a loop instead of recursively.
                VARIANT empty;
                VariantInit(&empty);
                currentArray.SetAt(i, empty); //Clear it
            }
        }

        if (FAILED(currentArray.Destroy()))
        {
            ASSERT(false); //FAIL on chk, try to clean up the fre
            currentArray.Detach();
        }

        if (m_previous != nullptr)
        {
            //Remove m_previous from linked list
            CAutoPtr<ExistingArrayNode> previous(m_previous.Detach());  //Removes link in brackets: Current (->) Previous -> Previous2
            m_previous.Attach(previous->m_previous.Detach()); //Removes node in brackets, updates link: Current  (Previous) -> Previous2
            currentArray.Attach(previous->m_safeArray.Detach());
        }
    }
}

HRESULT ExistingArrayNode::Push(_In_ CAutoPtr<ExistingArrayNode>& target, _In_ scaposition_t arrayPosition, _In_ ULONG arrayLength)
{
    ExistingArrayNode* tmp = target.Detach();
    ExistingArrayNode* newNode = new ExistingArrayNode(tmp, arrayPosition, arrayLength);

    if (newNode == nullptr)
    {
        target.Attach(tmp);
        return E_OUTOFMEMORY;
    }

    ASSERT(newNode->m_previous == tmp);
    target.Attach(newNode);

    return S_OK;
}

// This method does two things:
// - It detaches all SAFEARRAY's
// - It iterates through and *non-recursively* deletes the linked list
// Target at the end, will be nullptr.
void ExistingArrayNode::DetachAll(_In_ CAutoPtr<ExistingArrayNode>& target)
{
    CAutoPtr<ExistingArrayNode> current(target.Detach());
    // Loop through all nodes in the list, and detach/delete the here

    while (current != nullptr)
    {
        current->m_safeArray.Detach();
        //Scope
        {
            CAutoPtr<ExistingArrayNode> tmp(current.Detach());
            current.Attach(tmp->m_previous.Detach());
        }
    }

    ASSERT(target == nullptr);
}

bool ArrayPositionNode::IsCycle(_In_ scaposition_t nextArray)
{
    ArrayPositionNode* current = this;

    while (current != nullptr)
    {
        if (nextArray > current->m_mostForwardKnownPosition)
        {
            return false;
        }

        if (current->m_referencePosition == nextArray || current->m_mostForwardKnownPosition == nextArray)
        {
            return true;
        }

        current = current->m_parent;
    }

    return false;
}

HRESULT ArrayPositionNode::CheckForSeekBack(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    if (this->m_seekBackRequired)
    {
        IfFailGo(reader.Seek(m_nextItemsPosition));
        m_seekBackRequired = false;
    }
Error:
    return hr;
}

HRESULT ArrayPositionNode::Push(_In_ CAutoPtr<ArrayPositionNode> &target, _In_ scaposition_t nextArray, _In_ UINT32 arrayLength, _In_ VARIANT *targetVariant, _In_ SAFEARRAY *safeArrayRef)
{
    ArrayPositionNode* tmp = target.Detach();

    scaposition_t mostFurthestPosition = (tmp != nullptr && nextArray < tmp->m_mostForwardKnownPosition) ? tmp->m_mostForwardKnownPosition : nextArray;
    ArrayPositionNode* newNode = new ArrayPositionNode(tmp, nextArray, mostFurthestPosition, arrayLength, targetVariant, safeArrayRef);

    if (newNode == nullptr)
    {
        target.Attach(tmp);
        return E_OUTOFMEMORY;
    }
    ASSERT(newNode->m_parent == tmp);
    target.Attach(newNode);

    return S_OK;
}

void ArrayPositionNode::Pop(_In_ CAutoPtr<ArrayPositionNode> &target)
{
    ASSERT(target != nullptr);
    ArrayPositionNode* parent = target->m_parent.Detach();

    target->m_targetVariant->vt = (VT_ARRAY | VT_VARIANT);
    target->m_targetVariant->parray = target->m_safeArrayRef;

    target.Free();
    target.Attach(parent);
}


//
// Convert a UINT32 value to an I4 or R8 VARIANT (if can't fit in I4).
//
void InternalPropertyReader::SetVariant(_In_ UINT32 value, _Out_ VARIANT* pValue)
{
    ASSERT(V_VT(pValue) == VT_EMPTY); // out pValue must have been initialized with VariantInit

    INT32 intValue = static_cast<INT32>(value);

    if (intValue >= 0)
    {
        V_VT(pValue) = VT_I4;
        V_I4(pValue) = intValue;
    }
    else
    {
        V_VT(pValue) = VT_R8;
        V_R8(pValue) = static_cast<double>(value);
    }
}

//
// Read "string".length property from: byteLen
//
HRESULT InternalPropertyReader::ReadStringLength(_In_ const StreamReader& reader, _Out_ VARIANT* pValue)
{
    HRESULT hr = S_OK;

    UINT32 byteLength;
    IfFailGo(reader.Read(&byteLength));

    UINT32 length = byteLength / sizeof(WCHAR);
    SetVariant(length, pValue);

Error:
    return hr;
}


HRESULT InternalPropertyReader::ReadDenseArray(_In_ const SCAPropertyReader* propertyReader, _In_ const StreamReader& reader, _Out_ VARIANT* outResult)
{
    //This method could be very nicely written as a recursion; however, we want to handle as nested arrays as possible so it will be written in form of two loops.

    HRESULT hr = S_OK;
    SCATypeId outTypeId = SCA_None;
    scaposition_t currentPosition = 0;
    UINT32 arrayLength = 0;
    VARIANT *targetVariant = outResult;
    
    // This will always point to the most nested array in the chain of arrays that are in the process of deserializaiton.
    // It has a reference to the previous node.
    CAutoPtr<ArrayPositionNode> currentArrayNode;

    // This is a linked-list of all arrays we have constructed, will be used to lookup references to already deserialized arrays (that aren't loops)
    // This will also be used to cleanup arrays on error
    CAutoPtr<ExistingArrayNode> existingArrays;
    
    IfFailGo(reader.GetPosition(&currentPosition));
LPushNewArray:
    IfFailGo(reader.Read(&arrayLength));
    
    IfFailGo(ExistingArrayNode::Push(existingArrays, currentPosition, arrayLength));
    IfFailGo(ArrayPositionNode::Push(currentArrayNode, currentPosition, arrayLength, targetVariant, existingArrays->m_safeArray));
    
    while (currentArrayNode != nullptr)
    {
        VARIANT* currentVarArray = static_cast<VARIANT *>(currentArrayNode->m_safeArrayRef->pvData);

        // Check if we need to backtrack, then check if there are more elements
        while (SUCCEEDED(hr = currentArrayNode->CheckForSeekBack(reader)) && currentArrayNode->m_currentIndex < currentArrayNode->m_arrayLength)
        {
            targetVariant = currentVarArray + currentArrayNode->m_currentIndex;

            IfFailGo(reader.Read(&outTypeId));

            if (outTypeId == SCA_Reference)
            {
                IfFailGo(reader.Read(&currentPosition));
                IfFailGo(reader.GetPosition(&currentArrayNode->m_nextItemsPosition));

                IfFailGo(reader.Seek(currentPosition));
                IfFailGo(reader.Read(&outTypeId));

                ASSERT(outTypeId != SCA_Reference); //We shouldn't have a double reference
                IfFalseReturnError(outTypeId != SCA_Reference, E_SCA_DATACORRUPT);

                currentArrayNode->m_seekBackRequired = true; //Once we are done processing child array or string, jump back to this position in the IStream
            }

            // We need to increment before 'goto LPushNewArray', and before loop ends.
            currentArrayNode->m_currentIndex++;

            if (outTypeId == SCA_DenseArray)
            {
                IfFailGo(reader.GetPosition(&currentPosition)); //Read again in case of SCA_Reference or first time in case of not
                IfFalseReturnError(!currentArrayNode->IsCycle(currentPosition), S_OK /* S_OK and outResult == VT_NULL|VT_EMPTY means invalid key */);

                goto LPushNewArray; //breaks through two loops
            }
            // else

            IfFailGo(propertyReader->ReadValueOfType(false /* We explicitly handle DenseArray's here, don't handle them in ReadValueType */, outTypeId, targetVariant));
            IfFalseReturnError(V_VT(targetVariant) != VT_NULL && V_VT(targetVariant) != VT_EMPTY, S_OK);
        }

        IfFailGo(hr);

        //Read past SCA_PROPERTY_TERMINATOR and the preceeding properties
        IfFailGo(SkipWalker::WalkObjectProperties(reader));

        hr = S_OK;
        //Pop, and loop back to see if we have a parent array to process.
        ArrayPositionNode::Pop(currentArrayNode); 
    }
    
    ASSERT(outResult->vt == (VT_ARRAY | VT_VARIANT));
    
    //This will make sure that we don't clean up all the SAFEARRAYs
    ExistingArrayNode::DetachAll(existingArrays);

Error:
    return hr;
}

//
// Read string property from: byteLen content [padding]
//
HRESULT InternalPropertyReader::ReadString(_In_ const StreamReader& reader, _Out_ VARIANT* pValue)
{
    HRESULT hr = S_OK;

    BSTR bstr;
    IfFailGo(BSTRStringWalker::Read(reader, &bstr));
    V_VT(pValue) = VT_BSTR;
    V_BSTR(pValue) = bstr;

Error:
    return hr;
}

bool LengthPropertyReader::FindProperty(_In_ const BSTR name)
{
    switch(m_stage)
    {
    case NONE: // none -> none or length
        if (Equals(name, PROPERTYNAME_LENGTH))
        {
            m_stage = LENGTH;
            return true;
        }
        // Otherwise stay in NONE. This allows multiple initial tests.
        break;

    case LENGTH: // length -> unknown
        m_stage = UNKNOWN;
        break;
    }

    ASSERT(!IsLength());
    return false;
}

HRESULT ArrayInternalPropertyReader::ReadIndexableProperty(_In_ const StreamReader& reader, _Out_ VARIANT* pValue)
{
    ASSERT(IsLength()); // Must be reading length property. We only say "found" for length property.

    HRESULT hr = S_OK;

    UINT32 length;
    IfFailGo(reader.Read(&length));

    SetVariant(length, pValue);

Error:
    return hr;
}

HRESULT StringInternalPropertyReader::ReadIndexableProperty(_In_ const StreamReader& reader, _Out_ VARIANT* pValue)
{
    ASSERT(IsLength()); // Must be reading length property. We only say "found" for length property.

    return ReadStringLength(reader, pValue);
}

bool RegExpInternalPropertyReader::FindProperty(_In_ const BSTR name)
{
    switch (m_stage)
    {
    case NONE: // none -> none, or source/global/ignoreCase/multiline
        if (Equals(name, PROPERTYNAME_SOURCE))
        {
            m_stage = SOURCE;
            return true;
        }
        else if (Equals(name, PROPERTYNAME_GLOBAL))
        {
            m_stage = GLOBAL;
            return true;
        }
        else if (Equals(name, PROPERTYNAME_IGNORECASE))
        {
            m_stage = IGNORECASE;
            return true;
        }
        else if (Equals(name, PROPERTYNAME_MULTILINE))
        {
            m_stage = MULTILINE;
            return true;
        }
        // Otherwise stay in NONE. This allows multiple initial tests.
        break;

    case SOURCE: // source -> source.length or unknown
        if (Equals(name, PROPERTYNAME_LENGTH))
        {
            m_stage = SOURCE_LENGTH;
            return true;
        }
        m_stage = UNKNOWN;
        break;

    default: // Other -> unknown
        m_stage = UNKNOWN;
        break;
    }

    ASSERT(m_stage == NONE || m_stage == UNKNOWN);
    return false;
}

HRESULT RegExpInternalPropertyReader::ReadIndexableProperty(_In_ const StreamReader& reader, _Out_ VARIANT* pValue)
{
    HRESULT hr = S_OK;

    switch (m_stage)
    {
    case SOURCE:
        IfFailGo(ReadString(reader, pValue));
        break;

    case SOURCE_LENGTH:
        IfFailGo(ReadStringLength(reader, pValue));
        break;

    case GLOBAL:
    case IGNORECASE:
    case MULTILINE:
        break; // These are considerred non-indexable. Returns with S_OK/VT_EMPTY.

    default:
        ASSERT(false); // Otherwise this shouldn't be called.
        break;
    }

Error:
    return hr;
}
