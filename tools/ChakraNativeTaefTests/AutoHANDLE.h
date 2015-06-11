// Copyright(c) Microsoft Corporation.All rights reserved.

namespace ChakraNativeTaefTests
{
    class AutoHANDLE
    {
    public:
        AutoHANDLE(HANDLE handle = NULL) : handle(handle) {}
        ~AutoHANDLE()
        {
            Close();
        }
        AutoHANDLE& operator=(HANDLE handle)
        {
            Close();
            this->handle = handle;
            return *this;
        }
        operator HANDLE() const { return this->handle; }
        void Close()
        {
            if (handle != INVALID_HANDLE_VALUE && handle != NULL)
            {
                ::CloseHandle(handle);
                handle = INVALID_HANDLE_VALUE;
            }
        }
        PHANDLE operator&() { return &handle; }
    private:
        HANDLE handle;
    };
};