#pragma once

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// for now we use the custom data source
//#define RLEXE_XML_DATASOURCE

#include <windows.h>
#include <stdlib.h> // for _countof

#include "AutoHANDLE.h"
#include "Module.h"
#include "RLExeXmlReader.h"


#include "WexTestClass.h"

using namespace WEX::Logging;
using namespace WEX::TestExecution;

#include "TestBase.h"