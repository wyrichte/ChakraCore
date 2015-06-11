//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
class TSGenException : public std::exception
{
public:
    TSGenException(_In_ LPCWSTR message) : std::exception((const char*) message) {}
};