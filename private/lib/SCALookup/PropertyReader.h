//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

class SCAPropertyReader;
struct ArrayPositionNode;

class InternalPropertyReader
{
public:
    virtual bool FindProperty(_In_ const BSTR name) = 0;
    virtual HRESULT ReadIndexableProperty(_In_ const StreamReader& reader, _Out_ VARIANT* pValue) = 0;

    static void SetVariant(_In_ UINT32 value, _Out_ VARIANT* pValue);
    static HRESULT ReadStringLength(_In_ const StreamReader& reader, _Out_ VARIANT* pValue);
    static HRESULT ReadString(_In_ const StreamReader& reader, _Out_ VARIANT* pValue);
    static HRESULT ReadDenseArray(_In_ const SCAPropertyReader* propertyReader, _In_ const StreamReader& reader, _Out_ VARIANT* outResult);
};


// This class is simply responsible for holding on to referenrs for previously created SAFEARRAYS
// If Detach isn't called, it will clean all of them up
struct ExistingArrayNode
{
    CComSafeArray<VARIANT, VT_VARIANT> m_safeArray;
    CAutoPtr<ExistingArrayNode> m_previous;
    scaposition_t m_arrayPosition;

    ExistingArrayNode(_In_ ExistingArrayNode* previous, _In_ scaposition_t arrayPosition, _In_ ULONG arrayLength)
        : m_previous(previous),
        m_arrayPosition(arrayPosition),
        m_safeArray(arrayLength)
    { }

    ~ExistingArrayNode();

    static HRESULT Push(_In_ CAutoPtr<ExistingArrayNode>& target, _In_ scaposition_t arrayPosition, _In_ ULONG arrayVar);
    static void DetachAll(_In_ CAutoPtr<ExistingArrayNode>& target);
};

// This class has two primary purposes:
//  - To serve as a node in a linked list of parent's of (and) current Array being deserialized
//  - Contain all needed deserialization-state information for each array, in order to do a non-recursive loop
struct ArrayPositionNode
{
    // Parent of the current array
    CAutoPtr<ArrayPositionNode> m_parent;
    // Position of the current array in the stream
    scaposition_t m_referencePosition;
    // As we will build up this linked list, this is MAX m_referencePosition of this and it's parents
    scaposition_t m_mostForwardKnownPosition; 
    // Next item's position of the array to be deserialized, needed for resuming when we stop and go deserialize a nested array
    scaposition_t m_nextItemsPosition;
    // True if we have jumped to an SCA Referenced position for deserializing a nested array, so we can jump back to the above
    bool m_seekBackRequired;
    // Next item index (similar purpose as m_nextItemsPosition)
    UINT32 m_currentIndex;
    // Total array length, used by the loop
    UINT32 m_arrayLength;
    // The target variant to set once this array is Popped off, until then we don't set the variant as we haven't finished deserializing this yet; and we use empty VARIANT's to denote invalid keys
    VARIANT* m_targetVariant;
    //This class doesn't manage the memory of this safeArray, but needs a reference to it
    SAFEARRAY *m_safeArrayRef;

    ArrayPositionNode(_In_opt_ ArrayPositionNode* parent, _In_ scaposition_t referencePosition, _In_ scaposition_t mostForwardKnownPosition, _In_ UINT32 arrayLength, _In_ VARIANT* targetVariant, _In_ SAFEARRAY *safeArrayRef)
        : m_parent(parent),
        m_referencePosition(referencePosition),
        m_mostForwardKnownPosition(mostForwardKnownPosition),
        m_currentIndex(0),
        m_arrayLength(arrayLength),
        m_targetVariant(targetVariant),
        m_nextItemsPosition(0),
        m_seekBackRequired(false),
        m_safeArrayRef(safeArrayRef)
    { 
    }

    bool IsCycle(_In_ scaposition_t nextArray);
    HRESULT CheckForSeekBack(_In_ const StreamReader& reader);
    static HRESULT Push(_In_ CAutoPtr<ArrayPositionNode> &target, _In_ scaposition_t nextArray, _In_ UINT32 arrayLength, _In_ VARIANT *targetVariant, _In_ SAFEARRAY *safeArrayRef);
    static void Pop(_In_ CAutoPtr<ArrayPositionNode> &target);
};

class LengthPropertyReader: public InternalPropertyReader
{
private:
    enum ReadStage
    {
        NONE,   // Initial
        LENGTH, // On length
        UNKNOWN // Past length, e.g. length.abc
    };

    ReadStage m_stage;

protected:
    bool IsLength() const
    {
        return m_stage == LENGTH;
    }

public:
    LengthPropertyReader()
        : m_stage(NONE)
    {
    }

    virtual bool FindProperty(_In_ const BSTR name);
};

class ArrayInternalPropertyReader : public LengthPropertyReader
{
public:
    virtual HRESULT ReadIndexableProperty(_In_ const StreamReader& reader, _Out_ VARIANT* pValue);
};

class StringInternalPropertyReader : public LengthPropertyReader
{
public:
    virtual HRESULT ReadIndexableProperty(_In_ const StreamReader& reader, _Out_ VARIANT* pValue);
};

class RegExpInternalPropertyReader : public InternalPropertyReader
{
private:
    enum ReadStage
    {
        NONE,           // Initial
        SOURCE,
        GLOBAL,
        IGNORECASE,
        MULTILINE,
        SOURCE_LENGTH,  // On source.length
        UNKNOWN         // Past known property, e.g. source.abc, source.length.abc
    };

    ReadStage m_stage;

public:
    RegExpInternalPropertyReader()
        : m_stage(NONE)
    {
    }

    virtual bool FindProperty(_In_ const BSTR name);
    virtual HRESULT ReadIndexableProperty(_In_ const StreamReader& reader, _Out_ VARIANT* pValue);
};

class SCAPropertyReader
{
private:
    const StreamReader& m_reader;
    bool m_found;
    bool m_canPropertyBeAdded;
    InternalPropertyReader* m_pInternalPropertyReader;

    // stock internal property readers
    ArrayInternalPropertyReader m_arrayInternalPropertyReader;
    StringInternalPropertyReader m_stringInternalPropertyReader;
    RegExpInternalPropertyReader m_regExpInternalPropertyReader;


    HRESULT FindObjectPropertyName(_In_ const BSTR name);
    bool TryFindInternalProperty(_In_ InternalPropertyReader* pInternalPropertyReader, _In_ const BSTR name);

public:
    SCAPropertyReader(_In_ const StreamReader& reader)
        : m_reader(reader),
        m_found(false),
        m_canPropertyBeAdded(false),
        m_pInternalPropertyReader(nullptr)
    {
    }

    bool Found() const
    {
        return m_found;
    }

    bool CanPropertyBeAdded() const
    {
        return m_canPropertyBeAdded;
    }

    HRESULT FindPropertyByName(_In_ const BSTR name);
    HRESULT ReadValueOfType(_In_ bool allowDenseArrayForKeys, _In_ SCATypeId typeId, _Out_ VARIANT* pValue) const;
    HRESULT ReadIndexablePropertyValue(_In_ bool allowDenseArrayForKeys, _Out_ VARIANT* pValue) const;
};
