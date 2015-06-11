//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#ifdef JD_PRIVATE

//
// Represents a data value that might be uninitialized.
//
template <class T>
class Nullable
{
private:
    bool m_hasValue;
    T m_value;

public:
    Nullable()
        : m_hasValue(false)
    {
    }

    bool HasValue() const
    {
        return m_hasValue;
    }

    operator const T&() const
    {
        Assert(HasValue());
        return m_value;
    }

    const Nullable<T>& operator=(const T& value)
    {
        m_value = value;
        m_hasValue = true;

        return *this;
    }
};

#endif //JD_PRIVATE
