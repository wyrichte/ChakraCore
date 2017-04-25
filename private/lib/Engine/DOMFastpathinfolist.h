//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#ifndef _ONE_SIMPLESLOT_RECORD
#error _ONE_SIMPLESLOT_RECORD must be defined before including this file
#endif

#ifndef SIMPLESLOT_RECORD_KIND
#error SIMPLESLOT_RECORD_KIND must be defined before including this file
#endif

#undef ONE_SIMPLESLOT_RECORD
#define ONE_SIMPLESLOT_RECORD(kind, x) _ONE_SIMPLESLOT_RECORD(Simple##kind##SlotGetter##x, Simple##kind##SlotSetter##x, DOMFastPath<x>::EntryInfo::Simple##kind##SlotGetter, DOMFastPath<x>::EntryInfo::Simple##kind##SlotSetter, DOMFastPath<x>::EntrySimple##kind##SlotGetter, DOMFastPath<x>::EntrySimple##kind##SlotSetter, (HelperMethodAttribute)(AttrCanThrow | AttrInVariant), AttrCanThrow)

#define TEN_SIMPLESLOT_RECORD(kind, x) \
    ONE_SIMPLESLOT_RECORD(kind, x##0) \
    ONE_SIMPLESLOT_RECORD(kind, x##1) \
    ONE_SIMPLESLOT_RECORD(kind, x##2) \
    ONE_SIMPLESLOT_RECORD(kind, x##3) \
    ONE_SIMPLESLOT_RECORD(kind, x##4) \
    ONE_SIMPLESLOT_RECORD(kind, x##5) \
    ONE_SIMPLESLOT_RECORD(kind, x##6) \
    ONE_SIMPLESLOT_RECORD(kind, x##7) \
    ONE_SIMPLESLOT_RECORD(kind, x##8) \
    ONE_SIMPLESLOT_RECORD(kind, x##9) \

#define ONE_HUNDRED_SIMPLESLOT_RECORD(kind)\
    ONE_SIMPLESLOT_RECORD(kind, 0) \
    ONE_SIMPLESLOT_RECORD(kind, 1) \
    ONE_SIMPLESLOT_RECORD(kind, 2) \
    ONE_SIMPLESLOT_RECORD(kind, 3) \
    ONE_SIMPLESLOT_RECORD(kind, 4) \
    ONE_SIMPLESLOT_RECORD(kind, 5) \
    ONE_SIMPLESLOT_RECORD(kind, 6) \
    ONE_SIMPLESLOT_RECORD(kind, 7) \
    ONE_SIMPLESLOT_RECORD(kind, 8) \
    ONE_SIMPLESLOT_RECORD(kind, 9) \
    TEN_SIMPLESLOT_RECORD(kind, 1) \
    TEN_SIMPLESLOT_RECORD(kind, 2) \
    TEN_SIMPLESLOT_RECORD(kind, 3) \
    TEN_SIMPLESLOT_RECORD(kind, 4) \
    TEN_SIMPLESLOT_RECORD(kind, 5) \
    TEN_SIMPLESLOT_RECORD(kind, 6) \
    TEN_SIMPLESLOT_RECORD(kind, 7) \
    TEN_SIMPLESLOT_RECORD(kind, 8) \
    TEN_SIMPLESLOT_RECORD(kind, 9) \

ONE_HUNDRED_SIMPLESLOT_RECORD(SIMPLESLOT_RECORD_KIND)

