//  Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "kitchen_messages.h"

// Maximum length of error messages defined 
// in the kitchen_messages.mc file (after formatting).
#define KITCHEN_ERROR_MESSAGE_CHARS     512

// This variable is injected by the linker, and refers to the
// header of the exe or dll being linked.
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

