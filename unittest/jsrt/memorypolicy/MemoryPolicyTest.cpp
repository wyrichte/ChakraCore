//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "MemoryPolicyTest.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace JsrtUnitTests
{           
    template MemoryPolicyTest<JsRuntimeAttributeNone>;
    template MemoryPolicyTest<JsRuntimeAttributeDisableBackgroundWork>;
}
