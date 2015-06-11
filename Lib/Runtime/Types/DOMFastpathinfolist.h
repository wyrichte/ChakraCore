//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------


#ifndef _ONE_SIMPLESLOT_RECORD
#error _ONE_SIMPLESLOT_RECORD must be defined before including this file
#endif

#undef ONE_SIMPLESLOT_RECORD
#define ONE_SIMPLESLOT_RECORD(x) _ONE_SIMPLESLOT_RECORD(SimpleSlotGetter##x, SimpleSlotSetter##x, DOMFastPath<x>::EntryInfo::SimpleSlotGetter, DOMFastPath<x>::EntryInfo::SimpleSlotSetter, DOMFastPath<x>::EntrySimpleSlotGetter, DOMFastPath<x>::EntrySimpleSlotSetter, (HelperMethodAttribute)(AttrCanThrow | AttrInVariant), AttrCanThrow)

#define TEN_SIMPLESLOT_RECORD(x) \
    ONE_SIMPLESLOT_RECORD(x##0) \
    ONE_SIMPLESLOT_RECORD(x##1) \
    ONE_SIMPLESLOT_RECORD(x##2) \
    ONE_SIMPLESLOT_RECORD(x##3) \
    ONE_SIMPLESLOT_RECORD(x##4) \
    ONE_SIMPLESLOT_RECORD(x##5) \
    ONE_SIMPLESLOT_RECORD(x##6) \
    ONE_SIMPLESLOT_RECORD(x##7) \
    ONE_SIMPLESLOT_RECORD(x##8) \
    ONE_SIMPLESLOT_RECORD(x##9) \

#define ONE_HUNDRED_SIMPLESLOT_RECORD()\
    ONE_SIMPLESLOT_RECORD(0) \
    ONE_SIMPLESLOT_RECORD(1) \
    ONE_SIMPLESLOT_RECORD(2) \
    ONE_SIMPLESLOT_RECORD(3) \
    ONE_SIMPLESLOT_RECORD(4) \
    ONE_SIMPLESLOT_RECORD(5) \
    ONE_SIMPLESLOT_RECORD(6) \
    ONE_SIMPLESLOT_RECORD(7) \
    ONE_SIMPLESLOT_RECORD(8) \
    ONE_SIMPLESLOT_RECORD(9) \
    TEN_SIMPLESLOT_RECORD(1) \
    TEN_SIMPLESLOT_RECORD(2) \
    TEN_SIMPLESLOT_RECORD(3) \
    TEN_SIMPLESLOT_RECORD(4) \
    TEN_SIMPLESLOT_RECORD(5) \
    TEN_SIMPLESLOT_RECORD(6) \
    TEN_SIMPLESLOT_RECORD(7) \
    TEN_SIMPLESLOT_RECORD(8) \
    TEN_SIMPLESLOT_RECORD(9) \

ONE_HUNDRED_SIMPLESLOT_RECORD()
