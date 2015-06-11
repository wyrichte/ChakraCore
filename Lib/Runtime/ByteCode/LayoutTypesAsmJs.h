// Copyright (C) Microsoft. All rights reserved.

//
// NOTE: This file is intended to be "#include" multiple times.  The call site must define the macro
// "MACRO" to be executed for each entry.
//

#ifndef LAYOUT_TYPE
#define LAYOUT_TYPE(layout)
#endif

#ifndef LAYOUT_TYPE_WMS
#define LAYOUT_TYPE_WMS(layout) \
    LAYOUT_TYPE(layout##_Small) \
    LAYOUT_TYPE(layout##_Medium) \
    LAYOUT_TYPE(layout##_Large)
#endif

#ifndef LAYOUT_TYPE_WMS
#define LAYOUT_TYPE_WMS(layout) \
    LAYOUT_TYPE(layout##_Small) \
    LAYOUT_TYPE(layout##_Medium) \
    LAYOUT_TYPE(layout##_Large)
#endif

// For duplicates layout from LayoutTypes.h
#ifdef EXCLUDE_DUP_LAYOUT
#define LAYOUT_TYPE_DUP(...) 
#define LAYOUT_TYPE_WMS_DUP(...)
#else
#define LAYOUT_TYPE_DUP LAYOUT_TYPE
#define LAYOUT_TYPE_WMS_DUP LAYOUT_TYPE_WMS
#endif

// These layout are already defined in LayoutTypes.h
// We redeclare them here to keep the same layout and use them
// in other context
LAYOUT_TYPE_WMS_DUP(ElementSlot)
LAYOUT_TYPE_DUP    ( StartCall     )
LAYOUT_TYPE_DUP    ( Empty         )


LAYOUT_TYPE_WMS    ( AsmTypedArr   )
LAYOUT_TYPE_WMS    ( AsmCall       )
LAYOUT_TYPE        ( AsmBr         )
LAYOUT_TYPE_WMS    ( AsmReg1       ) // Generic layout with 1 RegSlot
LAYOUT_TYPE_WMS    ( AsmReg2       ) // Generic layout with 2 RegSlot
LAYOUT_TYPE_WMS    ( AsmReg3       ) // Generic layout with 3 RegSlot
LAYOUT_TYPE_WMS    ( AsmReg4       ) // Generic layout with 4 RegSlot
LAYOUT_TYPE_WMS    ( AsmReg5       ) // Generic layout with 5 RegSlot
#ifdef SIMD_JS_ENABLED
LAYOUT_TYPE_WMS    ( AsmReg6       ) // Generic layout with 6 RegSlot
LAYOUT_TYPE_WMS    ( AsmReg7       ) // Generic layout with 7 RegSlot
LAYOUT_TYPE_WMS    ( AsmReg2IntConst1 ) // Generic layout with 2 RegSlots and 1 Int Constant
#endif
LAYOUT_TYPE_WMS    ( Int1Double1   ) // 1 int register and 1 double register
LAYOUT_TYPE_WMS    ( Int1Float1    ) // 1 int register and 1 float register
LAYOUT_TYPE_WMS    ( Double1Int1   ) // 1 double register and 1 int register
LAYOUT_TYPE_WMS    ( Double1Float1 ) // 1 double register and 1 float register
LAYOUT_TYPE_WMS    ( Double1Reg1   ) // 1 double register and 1 var register
LAYOUT_TYPE_WMS    ( Float1Reg1   ) // 1 double register and 1 var register
LAYOUT_TYPE_WMS    ( Int1Reg1      ) // 1 int register and 1 var register
LAYOUT_TYPE_WMS    ( Reg1Double1   ) // 1 var register and 1 double register
LAYOUT_TYPE_WMS    ( Reg1Float1    ) // 1 var register and 1 Float register
LAYOUT_TYPE_WMS    ( Reg1Int1      ) // 1 var register and 1 int register
LAYOUT_TYPE_WMS    ( Int1Const1    ) // 1 int register and 1 const int value
LAYOUT_TYPE_WMS    ( Double1Addr1  ) // 1 double register and 1 const double* value
LAYOUT_TYPE_WMS    ( Float1Addr1   ) // 1 float register and 1 const float* value
LAYOUT_TYPE_WMS    ( Double1Const2 ) // 1 double register and 2 const double value
LAYOUT_TYPE_WMS    ( Double2Const1 ) // 2 double register and 1 const double value
LAYOUT_TYPE_WMS    ( Int1Double2   ) // 1 int register and 2 double register ( double comparisons )
LAYOUT_TYPE_WMS    ( Int1Float2    ) // 1 int register and 2 float register ( float comparisons )
LAYOUT_TYPE_WMS    ( Int2          ) // 2 int register
LAYOUT_TYPE_WMS    ( Int3          ) // 3 int register
LAYOUT_TYPE_WMS    ( Double2       ) // 2 double register
LAYOUT_TYPE_WMS    ( Float2        ) // 2 float register
LAYOUT_TYPE_WMS    ( Float3        ) // 3 float register
LAYOUT_TYPE_WMS    ( Float1Double1 ) // 2 double register
LAYOUT_TYPE_WMS    ( Float1Int1    ) // 2 double register
LAYOUT_TYPE_WMS    ( Double3       ) // 3 double register
LAYOUT_TYPE_WMS    ( BrInt1        ) // Conditionnal branching with 1 int
LAYOUT_TYPE_WMS    ( BrInt2        ) // Conditionnal branching with 2 int
LAYOUT_TYPE_WMS    ( AsmUnsigned1        ) // Conditionnal branching with 2 int

#ifdef SIMD_JS_ENABLED
// Float32x4
LAYOUT_TYPE_WMS    ( Float32x4_2 )
LAYOUT_TYPE_WMS    ( Float32x4_3 )
LAYOUT_TYPE_WMS    ( Float32x4_4 )
LAYOUT_TYPE_WMS    ( Float32x4_1Float4 )
LAYOUT_TYPE_WMS    ( Float32x4_2Int4 ) 
LAYOUT_TYPE_WMS    ( Float32x4_3Int4 ) 
LAYOUT_TYPE_WMS    ( Float32x4_1Float1 )
LAYOUT_TYPE_WMS    ( Float32x4_2Float1 )
LAYOUT_TYPE_WMS    ( Float32x4_1Float64x2_1 )
LAYOUT_TYPE_WMS    ( Float32x4_1Int32x4_1 )
LAYOUT_TYPE_WMS    ( Float32x4_1Int32x4_1Float32x4_2 )
LAYOUT_TYPE_WMS    ( Reg1Float32x4_1 )
LAYOUT_TYPE_WMS    ( Float1Float32x4_1IntConst1 )
LAYOUT_TYPE_WMS    ( Int1Float32x4_1)
// Int32x4
LAYOUT_TYPE_WMS    ( Int32x4_2) 
LAYOUT_TYPE_WMS    ( Int32x4_3) 
LAYOUT_TYPE_WMS    ( Int32x4_4) 
LAYOUT_TYPE_WMS    ( Int32x4_1Int4 )
LAYOUT_TYPE_WMS    ( Int32x4_1Int1 )
LAYOUT_TYPE_WMS    ( Int32x4_2Int1 )
LAYOUT_TYPE_WMS    ( Reg1Int32x4_1 )
LAYOUT_TYPE_WMS    ( Int32x4_1Float32x4_1 )
LAYOUT_TYPE_WMS    ( Int32x4_1Float64x2_1 )
LAYOUT_TYPE_WMS    ( Int1Int32x4_1IntConst1)
LAYOUT_TYPE_WMS    ( Int1Int32x4_1)

// Float64x2
LAYOUT_TYPE_WMS    ( Float64x2_2 ) 
LAYOUT_TYPE_WMS    ( Float64x2_3 ) 
LAYOUT_TYPE_WMS    ( Float64x2_4 ) 
LAYOUT_TYPE_WMS    ( Float64x2_1Double2 ) 
LAYOUT_TYPE_WMS    ( Float64x2_1Double1 )
LAYOUT_TYPE_WMS    ( Float64x2_2Double1 ) 
LAYOUT_TYPE_WMS    ( Float64x2_2Int2 ) 
LAYOUT_TYPE_WMS    ( Float64x2_3Int2 ) 
LAYOUT_TYPE_WMS    ( Float64x2_1Float32x4_1 ) 
LAYOUT_TYPE_WMS    ( Float64x2_1Int32x4_1 ) 
LAYOUT_TYPE_WMS    ( Float64x2_1Int32x4_1Float64x2_2 ) 
LAYOUT_TYPE_WMS    ( Reg1Float64x2_1 )
LAYOUT_TYPE_WMS    ( Double1Float64x2_1IntConst1 ) 
LAYOUT_TYPE_WMS    ( Int1Float64x2_1)
#endif

#undef LAYOUT_TYPE_DUP
#undef LAYOUT_TYPE_WMS_DUP
#undef LAYOUT_TYPE
#undef LAYOUT_TYPE_WMS
#undef EXCLUDE_DUP_LAYOUT