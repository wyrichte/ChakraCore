/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
class BitArray {
  unsigned int numBits;
  unsigned int bytesAllocated;
  unsigned char *bytes;
 public:
  BitArray(unsigned int numBits,ArenaAllocator *alloc) {
    unsigned int numBytes=(numBits + 7) / 8;
    this->numBits=numBits;
    this->bytesAllocated=numBytes;
    bytes=(unsigned char *)alloc->Alloc(numBytes);
  }

  inline bool GetBit(unsigned int bitPosition) {
    unsigned int byteNumber = bitPosition >> 3;
    unsigned int bitOffset  = bitPosition & 7;

    AssertMsg(bitPosition  < numBits,"Bit position out of range");
    return((bytes[byteNumber] & (1 << bitOffset))!=0);
  }

  inline void SetBit(unsigned int bitPosition) {
    unsigned int byteNumber = bitPosition>>3;
    unsigned int bitOffset  = bitPosition & 7;

    AssertMsg(bitPosition  < numBits,"Bit position out of range");
    bytes[byteNumber] |= (1<<bitOffset);
  }

  inline void ClearBit(unsigned int bitPosition) {
    unsigned int byteNumber = bitPosition>>3;
    unsigned int bitOffset  = bitPosition & 7;

    AssertMsg(bitPosition  < numBits,"Bit position out of range");
    bytes[byteNumber] &= (~(1<<bitOffset));
  }

  void Print() {
    unsigned int i;
    for (i = 0; i < numBits; i++) {
        printf("%c\n",i,GetBit(i)? '1' : '0');
    }
  }
};
