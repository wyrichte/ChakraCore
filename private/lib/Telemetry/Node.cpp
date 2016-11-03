#include "Node.h"
#include "TelemetryPch.h"
#include "Telemetry.h"
#include "TelemetryMacros.h"

using namespace Js;

NodeTelemetryProvider::NodeTelemetryProvider() : hasNodeModules(false), isPackageTelemetryFired(false), hProv(NULL), freq({ 0 })
{
    SetIsHighResPerfCounterAvailable(); // Used as a telemetry point
}

NodeTelemetryProvider::~NodeTelemetryProvider()
{

}

void NodeTelemetryProvider::FirePackageTelemetryHelper()
{
    Assert(!(this->IsPackageTelemetryFired()));
    this->CreateHashAndFirePackageTelemetry();
    this->ReleaseNodePackageList();
    this->SetIsPackageTelemetryFired(true);
}


void NodeTelemetryProvider::TryLogNodePackage(Recycler* recycler, const char16* packageName)
{
    char16* name = nullptr;
    const char16* nodeModule = _u("node_modules");
    bool isNodeModule = packageName && wcswcs(packageName, nodeModule);
    if (isNodeModule)
    {
        const char16 NODE_MODULES[] = _u("node_modules\\");
        char16* startPos = wcswcs(packageName, NODE_MODULES);
        char16* curr = startPos;

        // Find the last node_modules in the path
        while (curr != nullptr)
        {
            curr = curr + _countof(NODE_MODULES);
            startPos = curr - 1;
            curr = wcswcs(curr, NODE_MODULES);
        }
        // now startPos is at the package name
        char16 ch = _u('\\');
        char16* endPos = wcschr(startPos, ch);
        size_t len = 0;
        if (endPos == nullptr) // for cases like node_modules\\foo.js i.e. which doesn't have sub-directory
        {
            len = wcslen(startPos);
        }
        else
        {
            len = (size_t)(endPos - startPos);
        }

        if (len>0)
        {
            name = RecyclerNewArrayLeaf(recycler, char16, len + 1);
            js_wmemcpy_s(name, len, startPos, len);
            name[len] = _u('\0');
            this->AddPackageName(name);
        }
    }
}

HCRYPTPROV NodeTelemetryProvider::EnsureCryptoContext()
{
    if (NULL == hProv)
    {
        if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        {
            Assert(hProv == NULL);
            return NULL;
        }
    }
    return hProv;
}


void NodeTelemetryProvider::CreateHashAndFirePackageTelemetry()
{
    // Fire Hashed Package Counts and Hashed Packages
    int packageCount = 0;
    int upto = 0;
    char16* buf = _u('\0');
    double hashTime = 0.0;

    ThreadContext* threadContext = ThreadContext::GetThreadContextList();
    if (threadContext != nullptr && this->hasNodeModules)
    {
        LARGE_INTEGER hiResHashStartTime = { 0 };
        ULONGLONG hashStartTime = 0;
        if (this->isHighResAvail)
        {
            QueryPerformanceCounter(&(hiResHashStartTime));
        }
        else
        {
            hashStartTime = GetTickCount64();
        }
        HCRYPTPROV hProv = NULL;
        HCRYPTHASH hHash = NULL;
        hProv = this->EnsureCryptoContext();
        static const DWORD MaxHashLength = 128;
        static const DWORD MaxTraceLogLength = 65536;
        static const DWORD MaxNumberPackages = MaxTraceLogLength / MaxHashLength;

        packageCount = this->NodePackageIncludeList->Count();
        upto = packageCount < MaxNumberPackages ? packageCount : MaxNumberPackages;

        buf = RecyclerNewArrayLeaf(threadContext->GetRecycler(), char16, (upto * MaxHashLength) + 1);
        uint counter = 0;
        for (int j = 0; j < upto; j++)
        {
            const char16* stringToHash = this->NodePackageIncludeList->GetValueAt(j);
            size_t strSize = wcslen(stringToHash);
            if (strSize > INT_MAX)
            {
                return;
            }

            if (hProv &&
                CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
            {
                if (!CryptHashData(hHash, reinterpret_cast<const BYTE*>(stringToHash), static_cast<DWORD>(strSize) * sizeof(char16), 0))
                {
                    return;
                }

                DWORD hashLength;
                DWORD dwSize = sizeof(hashLength);
                if (!CryptGetHashParam(hHash, HP_HASHSIZE, reinterpret_cast<BYTE*>(&hashLength), &dwSize, 0))
                {
                    return;
                }

                if (hashLength > MaxHashLength)
                {
                    return;
                }

                BYTE* hashedData = RecyclerNewArrayLeaf(threadContext->GetRecycler(), BYTE, hashLength + 1);

                if (hashedData == nullptr)
                {
                    return;
                }

                if (!CryptGetHashParam(hHash, HP_HASHVAL, reinterpret_cast<BYTE*>(hashedData), &hashLength, 0))
                {
                    return;
                }

                if (hHash != NULL)
                {
                    CryptDestroyHash(hHash);
                }

                for (DWORD i = 0; i < hashLength && counter + 1 < (upto * MaxHashLength) + 1; ++i)
                {
                    char16 tmp[3];
                    swprintf_s(tmp, _u("%02X"), hashedData[i]);
                    buf[counter] = tmp[0];
                    buf[counter + 1] = tmp[1];
                    counter += 2;
                }

                if (counter >= (upto * MaxHashLength) + 1)
                {
                    AssertMsg(false, "Buffer overflow");
                    return;
                }
                buf[counter] = _u(';');
                counter++;
            }
            else
            {
                return;
            }
        }
        if (counter >= (upto * MaxHashLength) + 1)
        {
            AssertMsg(false, "Buffer overflow");
            return;
        }
        buf[counter] = _u('\0');

        if (this->isHighResAvail)
        {
            LARGE_INTEGER hiResHashStopTime = { 0 };
            QueryPerformanceCounter(&hiResHashStopTime);
            hashTime = ((hiResHashStopTime.QuadPart - hiResHashStartTime.QuadPart)* 1000.00 / freq.QuadPart);
        }
        else
        {
            hashTime = (double)(GetTickCount64() - hashStartTime);
        }
    }
    // we want to see telemetry even if package count is 0, this will give a good sense of ratio. Obvious down side is that we will get telemetry from ALL jsrt apps not just Node but we can
    // easily filter that.
    TraceLogChakra(
        TL_CHAKRANODEPACK,
        TraceLoggingUInt32(packageCount, "PackageCount"),
        TraceLoggingUInt32(upto, "HashedPackageCount"),
        TraceLoggingFloat64(hashTime, "HashTime"),
        TraceLoggingWideString(buf, "PackageHash")
    );
    if (hProv != NULL)
    {
        CryptReleaseContext(hProv, 0);
        hProv = NULL;
    }
}

void NodeTelemetryProvider::SetIsHighResPerfCounterAvailable()
{
    if (!QueryPerformanceFrequency(&freq))
    {
        this->isHighResAvail = false;
    }
    else
    {
        this->isHighResAvail = true;
    }
}


void NodeTelemetryProvider::InitializeNodePackageList()
{
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    if (threadContext != nullptr && threadContext->GetRecycler() != nullptr)
    {
        Recycler * recycler = threadContext->GetRecycler();
        NodePackageIncludeList.Root(RecyclerNew(recycler, NodePackageSet, recycler), recycler);
        this->hasNodeModules = true;
    }
}

void NodeTelemetryProvider::AddPackageName(const char16* packageName)
{
    if (this->NodePackageIncludeList == nullptr)
    {
        this->InitializeNodePackageList();
    }

    if (this->NodePackageIncludeList != nullptr)
    {
        this->NodePackageIncludeList->AddNew(packageName);
    }
}


void NodeTelemetryProvider::ReleaseNodePackageList()
{
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    if (threadContext != nullptr && this->NodePackageIncludeList != nullptr)
    {
        this->NodePackageIncludeList.Unroot(threadContext->GetRecycler());
    }
}