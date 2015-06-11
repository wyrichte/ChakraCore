//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class StringFileReader : public SimpleComObject<IAuthorFileReader>
    {
    private:
        const wchar_t* string;
        size_t stringLength;
        charcount_t offset;
    public:
        StringFileReader(const wchar_t* string, size_t stringLength): string(string), stringLength(stringLength), offset(0) { }

        STDMETHOD(Read)(long length, __out_ecount(length) wchar_t *buffer, __out_ecount(1) long *read);
        STDMETHOD(Seek)(long offset);
        STDMETHOD(Close)();
    };

    class MemoryAuthoringFile : public SimpleComObject<IAuthorFile>
    {
    private:
        const wchar_t* text;
        size_t length;
        AuthorFileStatus status;
#if DEBUG
        const wchar_t* name;
        size_t nameLen;
#endif
    public:
        MemoryAuthoringFile(const wchar_t* text, size_t length
#if DEBUG
            , const wchar_t* name = nullptr, size_t nameLen = 0
#endif
            ): text(text), length(length)
#if DEBUG
            , name(name), nameLen(nameLen)
#endif
        { }

        AuthorFileStatus LastStatus() { return this->status; }

        void AdjustLength(int adjustment) { length = (size_t)((int)length + adjustment); }
        size_t Length() { return length; }

        STDMETHOD(GetDisplayName)(BSTR *name);     
        STDMETHOD(GetLength)(long *length);
        STDMETHOD(GetReader)(IAuthorFileReader **result);
        STDMETHOD(StatusChanged)(AuthorFileStatus newStatus);
    };
}