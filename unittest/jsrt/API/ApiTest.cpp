//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ApiTest.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace JsrtUnitTests
{
    template ApiTest<JsRuntimeAttributeNone>;
    template ApiTest<JsRuntimeAttributeDisableBackgroundWork>;
    template ApiTest<JsRuntimeAttributeAllowScriptInterrupt>;
    template ApiTest<JsRuntimeAttributeEnableIdleProcessing>;
    template ApiTest<JsRuntimeAttributeDisableNativeCodeGeneration>;
    template ApiTest<JsRuntimeAttributeDisableEval>;
    template ApiTest<(JsRuntimeAttributes) (JsRuntimeAttributeDisableBackgroundWork | JsRuntimeAttributeAllowScriptInterrupt | JsRuntimeAttributeEnableIdleProcessing)>;
}
