
#pragma once

#include "Memory\RecyclerTelemetryInfo.h"

namespace Js
{
    bool TransmitRecyclerTelemetryStats(RecyclerTelemetryInfo& info);
    bool TransmitRecyclerTelemetryError(const RecyclerTelemetryInfo& info, const char * msg);
    bool IsTelemetryProviderEnabled();
    bool TransmitRecyclerHeapUsage(size_t totalHeapBytes, size_t usedHeapBytes, double heapUsedRatio);
}
