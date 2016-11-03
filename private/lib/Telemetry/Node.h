#pragma once
#include "TelemetryProvider.h"

class NodeTelemetryProvider : public TelemetryProvider {
private:
    //Code for node telemetry purposes
    typedef JsUtil::BaseHashSet<const char16*, Recycler, PrimeSizePolicy> NodePackageSet;
    RecyclerRootPtr<NodePackageSet> NodePackageIncludeList;
    HCRYPTPROV hProv;
    LARGE_INTEGER freq;
    bool isHighResAvail;
    bool hasNodeModules;
    bool isPackageTelemetryFired;
public:
    NodeTelemetryProvider();
    NodeTelemetryProvider(const NodeTelemetryProvider& copy) = delete;
    ~NodeTelemetryProvider();
    void CreateHashAndFirePackageTelemetry();
    void SetIsHighResPerfCounterAvailable();
    void InitializeNodePackageList();
    void ReleaseNodePackageList();
    void AddPackageName(const char16* packageName);
    bool IsPackageTelemetryFired() { return isPackageTelemetryFired; }
    void SetIsPackageTelemetryFired(bool value) { isPackageTelemetryFired = value; }
    void TryLogNodePackage(Recycler*, const char16*url);
    HCRYPTPROV EnsureCryptoContext();
    void FirePackageTelemetryHelper();
};