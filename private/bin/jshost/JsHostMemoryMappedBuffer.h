#pragma once

#include "streamspublished.h"

class JsHostMemoryMappedBuffer : public Streams::IByteChunk
{
public:
    void Free() override;
    HGLOBAL ExtractDataAsHGlobal() override
    {
        return INVALID_HANDLE_VALUE;
    }

    size_t GetTotalSize() const override
    {
        return _cbFileSize;
    }

    size_t GetTotalSizeInBytes() const override
    {
        return _cbFileSize;
    }

    size_t GetFilledSize() const override
    {
        return _cbFileSize;
    }
    size_t GetFilledSizeInBytes() const override
    {
        return _cbFileSize;
    }

    void SetFilledSize(size_t size) override {}
    void SetFilledSizeInBytes(size_t sizeInBytes) override {}

    size_t GetRemainingSize() const override 
    { 
        return 0;
    }

    size_t GetRemainingSizeInBytes() const override 
    {
        return 0;
    }

    virtual void MarkAsJavascriptVar() override {};

    virtual bool ContainsJavascriptVar() override
    {
        return false;
    }

    void Duplicate(_Outptr_ Streams::IByteChunk** newChunk) const override {}
    void CopyFrom(_In_ const Streams::IByteChunk* source) override {}
    void CopyFrom(_In_reads_(length) const BYTE* source, size_t length) override {}
    void Append(_In_ const Streams::IByteChunk* source) override {}
    void Append(_In_ const Streams::IByteChunk* source, size_t offset, size_t length) override {}
    BYTE* GetRawBuffer() const;

    // TODO: This method needs to be defined in IChunkOperations
    HRESULT RecreateView();

    // Extra methods

    LPCWSTR GetScriptPath()
    {
        return _strScriptFilePath.c_str();
    }

    static JsHostMemoryMappedBuffer* Create(LPCWSTR strScriptFilePath);
private:
    JsHostMemoryMappedBuffer(LPCWSTR strScriptFilePath);
    HRESULT Initialize();

    std::wstring _strScriptFilePath;

    HANDLE _hFile;
    HANDLE _hMapping;
    BYTE*  _pBuffer;
    size_t _cbFileSize;
};
