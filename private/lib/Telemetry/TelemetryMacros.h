#include <telemetry\MicrosoftTelemetry.h>
#include "ChakraVersion.h"

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


#define TraceLogChakra(name, ...) \
    if (g_TraceLoggingClient != nullptr && g_TraceLoggingClient->GetShouldLogTelemetry() == true) { \
        TraceLoggingWrite( \
        g_hTraceLoggingProv, \
        name, \
        TraceLoggingKeyword(MICROSOFT_KEYWORD_MEASURES), \
        TraceLoggingString(VER_IEVERSION_STR, "binaryVersion"), \
        TraceLoggingString(TL_BINARYFLAVOR, "binaryFlavor"), \
        TraceLoggingString(TL_BINARYARCH, "binaryArch"), \
        __VA_ARGS__ \
        ); \
    }

// Because directly using the telemetry macros has proved to be convoluted, here we
// unwrap the macros into a few separate macros, which allow us to keep supporting/
// fixing fewer files.
// This prologue sets the warnings ignored or elevated during the event, assigns us
// the logging level MEASURES, and does the common components of the struct.
#define TELPROLOGUE(eventname)\
    if(g_TraceLoggingClient != nullptr && g_TraceLoggingClient->GetShouldLogTelemetry() == true) \
    { \
        __pragma(warning(push)) __pragma(warning(disable:4127 4132 6001)) __pragma(warning(error:4047)) __pragma(execution_character_set(push,"UTF-8")) __pragma(pack(push, 1)) do \
        { \
            typedef _TlgTagEnc<0> _TlgTagTy; \
            enum \
            { \
                _TlgLevelConst = 5 /*MEASURES*/ \
            }; \
            static struct { \
                UCHAR _TlyBlobTyp; \
                UCHAR _TlgChannel; \
                UCHAR _TlgLevel; \
                UCHAR _TlgOpcode; \
                ULONGLONG _TlgKeyword; \
                UINT16 _TlgEvtMetaSize; \
                _TlgTagTy::type _TlgEvtTags; \
                char _TlgName[sizeof(eventname)]; \
                char _TlgNameBV[sizeof("binaryVersion")]; \
                UINT8 _TlgInBV; \
                char _TlgNameBF[sizeof("binaryFlavor")]; \
                UINT8 _TlgInBF; \
                char _TlgNameBA[sizeof("binaryArch")]; \
                UINT8 _TlgInBA; \
                char _TlgNameAI[sizeof("activityID")]; \
                UINT8 _TlgInAI; \
                char _TlgNameHI[sizeof("hostingInterface")]; \
                UINT8 _TlgInHI; \
                char _TlgNameIJ[sizeof("isJSRT")]; \
                UINT8 _TlgInIJ; \
                char _TlgNameJR[sizeof("JsrtRuntime")]; \
                UINT8 _TlgInJR;
// this is followed by the definitions for the rest of the stuff in the event

// The first midlogue closes out the definition of the telemetry data struct, which
// is immediately followed by the instantiation of the single instance of it. It is
// allocated in a special region of the binary, which is necessary for some tooling
// to pick it up properly.
#define TELMIDLOGUEPART1(eventname) \
            } __declspec(allocate(".rdata$zETW1")) __declspec(align(1)) const _TlgEvent = { \
                _TlgBlobEvent3, \
                11, \
                _TlgLevelConst, \
                0, \
                0 |_TraceLoggingKeyword<0x0000400000000000>::value, \
                sizeof(_TlgEvent)-11-1, \
                _TlgTagTy::value, \
                (eventname), \
                ("binaryVersion"), TlgInANSISTRING, \
                ("binaryFlavor"), TlgInANSISTRING, \
                ("binaryArch"), TlgInANSISTRING, \
                ("activityID"), TlgInGUID, \
                ("hostingInterface"), TlgInUINT32, \
                ("isJSRT"), TlgInBOOL32, \
                ("JsrtRuntime"), TlgInPOINTER

// Now that we've constructed the metadata for the event, we need to actually build
// and send an instance of the actual event data to the telemetry subsystem.
#define TELMIDLOGUEPART2() \
            }; \
            TraceLoggingHProvider const _TlgProv = (g_hTraceLoggingProv); \
            if((UCHAR)_TlgLevelConst < _TlgProv->LevelPlus1 && _TlgKeywordOn(_TlgProv, _TlgEvent._TlgKeyword)) \
            { \
                EVENT_DATA_DESCRIPTOR _TlgData[2 +7

#define TELMIDLOGUEPART3() \
                ]; \
                UINT32 _TlgIdx = 2; \
                ( \
                    _TlgCreateSz( &_TlgData[_TlgIdx], (VER_IEVERSION_STR)), \
                    _TlgIdx += 1, \
                    _TlgCreateSz( &_TlgData[_TlgIdx], (TL_BINARYFLAVOR)), \
                    _TlgIdx += 1, \
                    _TlgCreateSz( &_TlgData[_TlgIdx], (TL_BINARYARCH)), \
                    _TlgIdx += 1, \
                    _TlgCreateDesc<GUID>( &_TlgData[_TlgIdx], (activityId)), \
                    _TlgIdx += 1, \
                    _TlgCreateDesc<UINT32>( &_TlgData[_TlgIdx], (hostType)), \
                    _TlgIdx += 1, \
                    _TlgCreateDesc<INT32>( &_TlgData[_TlgIdx], (isJSRT)), \
                    _TlgIdx += 1, \
                    _TlgCreateDesc<void const*>( &_TlgData[_TlgIdx], (threadContext->GetJSRTRuntime())), \
                    _TlgIdx += 1,

#define TELEPILOGUE() \
                    __pragma(warning(disable:26000)) __annotation(L"_TlgWrite"), \
                    _TlgWrite(_TlgProv, &_TlgEvent._TlgChannel, 0, 0, _TlgIdx, _TlgData) \
                ); \
            } \
        } \
        while (0) __pragma(pack(pop)) __pragma(execution_character_set(pop)) __pragma(warning(pop)); \
    }



