//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

namespace JsDiag
{
    //
    // AutoPtr using operator delete (rather than HeapDelete as AutoPtr in lib\common\memory\AutoPtr.h).
    //
    template <typename T>
    class AutoPtr : public BasePtr<T>
    {
    public:
        AutoPtr() {}
        AutoPtr(T* p) : BasePtr(p) {}

        ~AutoPtr()
        {
            this->Clear();
        }

        AutoPtr& operator=(T * p)
        {
            this->Clear();
            this->ptr = p;
            return *this;
        }

    private:
        void Clear()
        {
            if (this->ptr != NULL)
            {
                delete this->ptr;
                this->ptr = NULL;
            }
        }
    };

    //
    // AutoPtr using operator delete[] (rather than HeapDelete as AutoArrayPtr in lib\common\memory\AutoPtr.h).
    //
    template <typename T>
    class AutoArrayPtr : public BasePtr<T>
    {
    public:
        AutoArrayPtr() {}
        AutoArrayPtr(T* ptr) : BasePtr(ptr) {}

        ~AutoArrayPtr()
        {
            Clear();
        }

        AutoArrayPtr& operator=(T * ptr)
        {
            Clear();
            this->ptr = ptr;
            return *this;
        }
    private:
        void Clear()
        {
            if (ptr != NULL)
            {
                delete[] this->ptr;
                this->ptr = NULL;
            }
        }
    };

    class AutoHandle
    {
    public:
        AutoHandle(HANDLE handle = NULL) : handle(handle) {}
        ~AutoHandle() { Close(); }

        HANDLE* operator&() { return &this->handle; }
        operator HANDLE() const { return this->handle; }

        void Close()
        {
            if (this->handle)
            {
                CloseHandle(this->handle);
                this->handle = NULL;
            }
        }
    private:
        HANDLE handle;
    };

} // namespace JsDiag
