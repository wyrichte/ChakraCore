#pragma once

#include <telemetry\MicrosoftTelemetry.h>
#include <TraceLoggingProvider.h>
#include "ChakraVersion.h"
#include "..\..\bin\Chakra\ChakraVersionBuildCommit.h"

#ifdef DBG
#define TL_BINARYFLAVOR "CHK"
#else
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
#define TL_BINARYFLAVOR "FRETEST"
#else
#define TL_BINARYFLAVOR "FRE"
#endif
#endif

#if defined(_M_IX86)
#define TL_BINARYARCH "x86"
#elif defined(_M_X64)
#define TL_BINARYARCH "amd64"
#elif defined(_M_ARM)
#define TL_BINARYARCH "arm32"
#elif defined(_M_ARM64)
#define TL_BINARYARCH "arm64"
#else
#error Unknown architecture
#endif

#define TraceLogChakra(name, ...)                                                                       \
    {                                                                                                   \
        const auto& globalFlags = Js::Configuration::Global.flags;                                      \
        if (g_TraceLoggingClient != nullptr && g_TraceLoggingClient->GetShouldLogTelemetry() == true)   \
        {                                                                                               \
            TraceLoggingWrite(                                                                          \
            g_hTraceLoggingProv,                                                                        \
            name,                                                                                       \
            TraceLoggingKeyword(MICROSOFT_KEYWORD_MEASURES),                                            \
            TraceLoggingString(VER_IEVERSION_STR, "binaryVersion"),                                     \
            TraceLoggingString(TL_BINARYFLAVOR, "binaryFlavor"),                                        \
            TraceLoggingString(TL_BINARYARCH, "binaryArch"),                                            \
            TraceLoggingString(chakraVersionBuildCommit, "chakraBuildCommit"),                          \
            TraceLoggingWideString(globalFlags.TelemetryRunType, "runType"),                            \
            TraceLoggingWideString(globalFlags.TelemetryDiscriminator1, "Discriminator1"),              \
            TraceLoggingWideString(globalFlags.TelemetryDiscriminator2, "Discriminator2"),              \
            __VA_ARGS__                                                                                 \
            );                                                                                          \
        }                                                                                               \
    }

