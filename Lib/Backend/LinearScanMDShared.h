//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class LinearScan;
class LinearScanMDShared
{
public:    
    void Init(LinearScan * linearScan) { this->linearScan = linearScan; }
    
protected:
    LinearScan * linearScan;         
};