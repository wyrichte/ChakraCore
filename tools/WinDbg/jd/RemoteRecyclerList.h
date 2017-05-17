#pragma once

#ifdef JD_PRIVATE
template <bool isSlist>
class RemoteListIterator
{
public:
    RemoteListIterator(PCSTR type, ULONG64 listPtr):
        _list(0),
        _current(0)
    {
        char typeTemplate[] = "(%%s!%s*)@$extin";
        size_t count = sizeof(typeTemplate) / sizeof(char)+strlen(type) + 1;
        _typeString = (char*)malloc(count);
        if (!_typeString)
        {
            GetExtension()->ThrowOutOfMemory();
        }
        sprintf_s(_typeString, count, typeTemplate, type);
        _list = listPtr;
        _current = listPtr;
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
    char* _typeString;

    ULONG64 _list;
    ULONG64 _current;
};

template <bool isSlist>
void DumpList(ULONG64 address, PCSTR type);
#endif
