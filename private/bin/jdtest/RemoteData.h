//
// Wrapper for accessing remote debugging target data.
// Supports delayed read (we don't read target memory until we actually need to access fields that require read operation).
// Template parameters:
// - T: the remote type to wrap
// Usage example:
// - RemoteData<ScriptContext> remoteScriptContext(reader, remoteAddr);
//

using namespace JsDiag;

template <typename T>
class RemoteData
{
private:
    BYTE m_data[sizeof(T)];
    bool m_isInitialized;

protected:
    IDebugDataSpaces4* m_reader;
    const T* m_remoteAddr;

public:
    typedef T TargetType;

    // Returns strongly typed pointer to the target data buffer.
    T* ToTargetPtr()
    {
        return this->ToTargetPtrImpl();
    }

    const T* ToTargetPtr() const
    {
        return const_cast<T*>(const_cast<RemoteData*>(this)->ToTargetPtrImpl());
    }

    T* operator->()
    {
        return this->ToTargetPtr();
    }

    const T* operator->() const
    {
        return this->ToTargetPtr();
    }

    operator T*()
    {
        return operator->();
    }

    operator const T*() const
    {
        return operator->();
    }

    RemoteData(IDebugDataSpaces4* reader, const T* remoteAddr) : m_reader(reader), m_remoteAddr(remoteAddr), m_isInitialized(false) {}

    // Returns address of field at specified offset in remote type, as pointer to the specified type.
    // This is useful for structs/classes embedded into target type (i.e. when the field is struct, and not struct*).
    // Parameters:
    // - TFieldType: actual type of the field as defined in the .h file.
    // - offset: offset off m_remoteAddr for the field.
    template <typename TFieldType>
    TFieldType* GetFieldAddr(size_t offset) const
    {
        return reinterpret_cast<TFieldType*>(reinterpret_cast<BYTE*>(const_cast<T*>(m_remoteAddr)) + offset);
    }

    const void* ReadVTable() const { return ReadField<const void*>(0); }
    IDebugDataSpaces4* GetReader() const { return m_reader; }
    T* GetRemoteAddr() const { return const_cast<T*>(m_remoteAddr); }

    void Flush()
    {
        Assert(m_isInitialized);
        ULONG bytesWritten;
        HRESULT hr = m_reader->WriteVirtual((ULONG64)m_remoteAddr, m_data, _countof(m_data), &bytesWritten);
        if(bytesWritten != _countof(m_data))
        {
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::WRITE_VIRTUAL_MISMATCH);
        }
        CheckHR(hr, DiagErrorCode::WRITE_VIRTUAL);
    }

    template<typename TFieldType>
    void WriteField(size_t offset, TFieldType data) const
    {
        TFieldType* remoteFieldAddr = GetFieldAddr<TFieldType>(offset);
        HRESULT hr = m_reader->WriteVirtual((ULONG64)remoteFieldAddr, &data, sizeof(TFieldType));
        CheckHR(hr, DiagErrorCode::WRITE_VIRTUAL);
    }
private:
    // Returns a pointer to m_data (which is in local address space) interpreted as T*,
    // with data pointed to containing an instance of T read from target process.
    T* ToTargetPtrImpl()
    {
        if (!m_isInitialized)
        {
            this->Read();   // Read remote memory into m_data;
            m_isInitialized = true;
        }
        return reinterpret_cast<T*>(&m_data[0]);
    }

    void Read()
    {
        // Note: currently we read the whole type. Could be reading just needed field (at specified offset).
        ULONG bytesRead;
        HRESULT hr = m_reader->ReadVirtual((ULONG64)m_remoteAddr, m_data, sizeof(m_data), &bytesRead);
        CheckHR(hr, DiagErrorCode::READ_VIRTUAL);
        if (bytesRead != sizeof(m_data))
        {
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::READ_VIRTUAL_MISMATCH);
        }
    }
}; // RemoteData.


class Reader
{
public:
    /*static*/
    static WCHAR* ReadString(IDebugDataSpaces4* reader, ULONG64 remoteAddr, const ULONG elementCount)
    {
        WCHAR* localBuffer = new WCHAR[elementCount + 1];

        ULONG bytesRead;

        HRESULT hr = reader->ReadVirtual(remoteAddr, localBuffer, elementCount * sizeof(WCHAR), &bytesRead);
        CheckHR(hr, DiagErrorCode::READ_STRING);
        if (bytesRead != elementCount * sizeof(WCHAR))
        {
            delete[] localBuffer;
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::READ_STRING_MISMATCH);
        }
        localBuffer[elementCount] = '\0';
        return localBuffer;
    }
};

typedef RemoteData<ScriptDebugEvent> RemoteScriptDebugEvent;