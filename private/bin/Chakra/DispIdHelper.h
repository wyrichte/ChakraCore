#pragma once

#include "mshtmdid.h"

// These are logical transformations
// The types are still DISPID, we just transform the space
inline DISPID PropertyIdToExpandoDispId(const DISPID& pid)
{
    return (DISPID) ((pid - Js::InternalPropertyIds::Count) + DISPID_EXPANDO_BASE);
}

inline DISPID ExpandoDispIdToPropertyId(const DISPID& id)
{
    return (DISPID) ((id - DISPID_EXPANDO_BASE) + Js::InternalPropertyIds::Count);
}

inline bool IsExpandoDispId(DISPID dispid)
{
    return (dispid >= DISPID_EXPANDO_BASE && dispid <= DISPID_EXPANDO_MAX);
}
