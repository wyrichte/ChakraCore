// Copyright (C) Microsoft. All rights reserved.

#pragma once

enum class ExecutionMode : uint8
{
    #define EXECUTION_MODE(name) name,
    #include "ExecutionModes.h"
    #undef EXECUTION_MODE
};
ENUM_CLASS_HELPERS(ExecutionMode, uint8);

extern const char *ExecutionModeName(const ExecutionMode executionMode);
