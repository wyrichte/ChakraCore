#pragma once
namespace Js
{
    class SimpleSourceHolder;
    class ISourceHolder : public FinalizableObject
    {
    private:
        static SimpleSourceHolder const emptySourceHolder;
        static LPCUTF8 const emptyString;

    public:
        static ISourceHolder *GetEmptySourceHolder()
        {
            return (ISourceHolder *)&emptySourceHolder;
        }

        virtual LPCUTF8 GetSource(const wchar_t* reasonString) = 0;
        virtual size_t GetByteLength(const wchar_t* reasonString) = 0;
        virtual ISourceHolder* Clone(ScriptContext* scriptContext) = 0;
        virtual bool Equals(ISourceHolder* other) = 0;
        virtual int GetHashCode() = 0;
        virtual bool IsEmpty() = 0;
        virtual bool IsDeferrable() = 0;
    };

    class SimpleSourceHolder : public ISourceHolder
    {
        friend class ISourceHolder;
    private:
        LPCUTF8 source;
        size_t byteLength;
        bool isEmpty;

        SimpleSourceHolder(LPCUTF8 source, size_t byteLength, bool isEmpty)
            : source(source),
            byteLength(byteLength),
            isEmpty(isEmpty)
        {
        }

    public:
        SimpleSourceHolder(LPCUTF8 source, size_t byteLength)
            : source(source),
            byteLength(byteLength),
            isEmpty(false)
        { 
        }

        virtual LPCUTF8 GetSource(const wchar_t* reasonString) override 
        { 
            return source; 
        }

        virtual size_t GetByteLength(const wchar_t* reasonString) override { return byteLength; }
        virtual ISourceHolder* Clone(ScriptContext* scriptContext) override;

        virtual bool Equals(ISourceHolder* other) override
        {
            return this == other || 
                (this->GetByteLength(L"Equal Comparison") == other->GetByteLength(L"Equal Comparison") 
                    && (this->GetSource(L"Equal Comparison") == other->GetSource(L"Equal Comparison") 
                        || memcmp(this->GetSource(L"Equal Comparison"), other->GetSource(L"Equal Comparison"), this->GetByteLength(L"Equal Comparison")) == 0 ));
        }

        virtual bool IsEmpty() override
        {
            return this->isEmpty;
        }

        virtual int GetHashCode() override
        {
            Assert(byteLength < MAXUINT32);
            return JsUtil::CharacterBuffer<utf8char_t>::StaticGetHashCode(source, (charcount_t)byteLength);
        }

        virtual void Finalize(bool isShutdown) override
        {
        }

        virtual void Dispose(bool isShutdown) override
        {
        }

        virtual void Mark(Recycler * recycler) override
        {
        }

        virtual bool IsDeferrable() override 
        {
            return CONFIG_FLAG(DeferLoadingAvailableSource);
        }
    };
}