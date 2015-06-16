//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Memory
{
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
class ForcedMemoryConstraint
{
public:
    static void Apply();

private:
    static void FragmentAddressSpace(size_t usableSize);
};
#endif
}