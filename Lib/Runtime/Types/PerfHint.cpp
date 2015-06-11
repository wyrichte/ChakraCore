#include "StdAfx.h"

const PerfHintItem s_perfHintContainer[] =
{
#define PERFHINT_REASON(name, isNotOptimized, level, desc, consequences, suggestion) {desc, consequences, suggestion, level, isNotOptimized},
#include "PerfHintDescriptions.h"
#undef PERFHINT_REASON
};

void WritePerfHint(PerfHints hint, Js::FunctionBody * functionBody, uint byteCodeOffset /*= Js::Constants::NoByteCodeOffset*/)
{
    Assert(functionBody);
    Assert(((int)hint) < _countof(s_perfHintContainer));

    PerfHintItem item = s_perfHintContainer[(uint)hint];

    int level = CONFIG_FLAG(PerfHintLevel);
    Assert(level <= (int)PerfHintLevels::VERBOSE);

    if ((int)item.level <= level)
    {
        ULONG lineNumber = functionBody->GetLineNumber();
        LONG columnNumber = functionBody->GetColumnNumber();
        if (byteCodeOffset != Js::Constants::NoByteCodeOffset)
        {
            functionBody->GetLineCharOffset(byteCodeOffset, &lineNumber, &columnNumber, false/*canAllocateLineCache*/);

            // returned values are 0-based. Adjusting.
            lineNumber++;
            columnNumber++;
        }

        // We will be going to print the short name.
        TCHAR shortName[255];
        Js::FunctionBody::GetShortNameFromUrl(functionBody->GetSourceName(), shortName, 255);

        OUTPUT_TRACE(Js::PerfHintPhase, L"%s : %s {\n      Function : %s [%s @ %u, %u]\n  Consequences : %s\n    Suggestion : %s\n}\n",
            item.isNotOptimized ? L"Not optimized" : L"Optimized",
            item.description,
            functionBody->GetExternalDisplayName(),
            shortName,
            lineNumber,
            columnNumber,
            item.consequences,
            item.suggestion);
        Output::Flush();
    }
}

