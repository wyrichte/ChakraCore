/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
enum RelocType {
    RelocTypeBranch,
    RelocTypeCallPcrel,
    RelocTypeLocalSlotsOffset
};

struct Reloc {
    RelocType relocType;
    Instruction *consumer;  
    unsigned int offset;    // for branches
};

struct NativeReloc {
    RelocType relocType;
    unsigned int consumerOffset;  // offset in instruction stream; contains 4-byte value to be updated
};
