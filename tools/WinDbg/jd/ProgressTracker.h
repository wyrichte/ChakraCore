//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class ProgressTracker
{
public:
    ProgressTracker(char const * message, uint frequency, uint maxIterations);
    void Inc();
    void ResetIter(char const * newMessage);
    void Done(char const * doneMessage);
private:
    char const * message;

    ULONG64 start;
    ULONG64 last;
    uint iter;
    uint maxIterations;
    uint frequency;
};