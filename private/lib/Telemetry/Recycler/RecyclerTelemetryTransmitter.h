
#pragma once

#include "Memory\RecyclerTelemetryInfo.h"

namespace Js
{
    bool TransmitRecyclerTelemetry(RecyclerTelemetryInfo& info);
    bool TransmitRecyclerTelemetryError(const RecyclerTelemetryInfo& info, const char * msg);
    bool IsTelemetryProviderEnabled();
}
