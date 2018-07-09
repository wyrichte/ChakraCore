#pragma once

template <bool isSlist>
class RemoteListIterator
{
public:
    RemoteListIterator(PCSTR type, ULONG64 listPtr):
        _list(0),
        _current(0)
    {
        Initialize(type, listPtr);
    }

    RemoteListIterator(ExtRemoteTyped list)
    {
        char const * typeName = JDUtil::StripStructClass(list.GetTypeName());
        char const * expectedPrefix = isSlist ? "SListBase<" : "DListBase<";
        if (strstr(typeName, expectedPrefix) != typeName)
        {
            GetExtension()->Err("%s expected, but got %s instead", isSlist? "SListBase" : "DListBase", typeName);
            GetExtension()->ThrowLastError("Unable to iterate SList/DList type");
        }
        char const * currTypeName = typeName + strlen(expectedPrefix);
        char * lastCurrTypeName = strrchr(currTypeName, ',');
        if (lastCurrTypeName == nullptr)
        {
            GetExtension()->Err("Unable to parse type name from %s", typeName);
            GetExtension()->ThrowLastError("Unable to iterate SList/DList type");
        }
        *lastCurrTypeName = 0;
        Initialize(currTypeName, list.GetPtr());
    }

    bool IsHead(ULONG64 node)
    {
        return (node == _list);
    }

    bool Next()
    {
        ExtRemoteData next(_current, g_Ext->m_PtrSize);

        if (IsHead(next.GetPtr()) || next.GetPtr() == 0)
        {
            _current = 0;
            return false;
        }

        _current = next.GetPtr();
        return true;
    }

    ULONG64 GetDataPtr()
    {
        int offsetOfData = 2; // Node class has next, previous, data

        if (isSlist)
        {
            offsetOfData = 1; // Node class has next and data
        }

        return _current + g_Ext->m_PtrSize * offsetOfData;
    }

    ExtRemoteTyped Data()
    {
        ExtRemoteTyped node(GetExtension()->FillModule(_typeString), GetDataPtr());
        return node;
    }

    ~RemoteListIterator()
    {
        if (_typeString)
        {
            free(_typeString);
        }
    }

private: 
    void Initialize(PCSTR type, ULONG64 listPtr)
    {
        char typeTemplate[] = "(%%s!%s*)@$extin";
        size_t count = sizeof(typeTemplate) / sizeof(char) + strlen(type) + 1;
        _typeString = (char*)malloc(count);
        if (!_typeString)
        {
            GetExtension()->ThrowOutOfMemory();
        }
        sprintf_s(_typeString, count, typeTemplate, type);
        _list = listPtr;
        _current = listPtr;
    }
    char* _typeString;

    ULONG64 _list;
    ULONG64 _current;
};

template <bool isSlist>
void DumpList(ULONG64 address, PCSTR type);


template <typename Fn>
static bool SListForEach(ExtRemoteTyped list, Fn fn)
{
    RemoteListIterator<true> it(list);
    while (it.Next())
    {
        if (fn(it.Data()))
        {
            return true;
        }
    }
    return false;
}

template <typename Fn>
static bool DListForEach(ExtRemoteTyped list, Fn fn)
{
    RemoteListIterator<false> it(list);
    while (it.Next())
    {
        if (fn(it.Data()))
        {
            return true;
        }
    }
    return false;
}