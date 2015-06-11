/*
SIMD.js opcodes
- All opcodes are typed. 
- Used as bytecode for AsmJs Interpreter.
- Used as IR by the backend only for both AsmJs and non-AsmJs code.
*/

// used as both AsmJs bytecode and IR
#ifndef MACRO_SIMD
#define MACRO_SIMD(opcode, asmjsLayout, opCodeAttrAsmJs, OpCodeAttr)
#endif

#ifndef MACRO_SIMD_WMS
#define MACRO_SIMD_WMS(opcode, asmjsLayout, opCodeAttrAsmJs, OpCodeAttr)
#endif

// used as AsmJs bytecode only
#ifndef MACRO_SIMD_ASMJS_ONLY_WMS
#define MACRO_SIMD_ASMJS_ONLY_WMS(opcode, asmjsLayout, opCodeAttrAsmJs, OpCodeAttr)
#endif

// used as IR only
#ifndef MACRO_SIMD_BACKEND_ONLY
#define MACRO_SIMD_BACKEND_ONLY(opcode, asmjsLayout, opCodeAttrAsmJs, OpCodeAttr)
#endif

//                              OpCode                             , LayoutAsmJs                , OpCodeAttrAsmJs,          OpCodeAttr
//                                |                                    |                                |                       |
//                                v                                    v                                v                       v
MACRO_SIMD                  ( Simd128_Start                     , Empty                              , None           ,        None          )               // Just a marker to indicate SIMD opcodes region

// Int32x4
MACRO_SIMD_WMS              ( Simd128_IntsToI4                  , Int32x4_1Int4                     , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Splat_I4                  , Int32x4_1Int1                     , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromFloat64x2_I4          , Int32x4_1Float64x2_1              , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromFloat64x2Bits_I4      , Int32x4_1Float64x2_1              , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromFloat32x4_I4          , Int32x4_1Float32x4_1              , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromFloat32x4Bits_I4      , Int32x4_1Float32x4_1              , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Neg_I4                    , Int32x4_2                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Add_I4                    , Int32x4_3                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Sub_I4                    , Int32x4_3                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Mul_I4                    , Int32x4_3                         , None           ,        OpCanCSE          )
//MACRO_SIMD_WMS              ( Simd128_Swizzle_I4                , Int32x4_2Int4                     , None            ,       OpCanCSE          )
//MACRO_SIMD_WMS              ( Simd128_Shuffle_I4                , Int32x4_3Int4                     , None            ,       OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithX_I4                  , Int32x4_2Int1                     , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithY_I4                  , Int32x4_2Int1                     , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithZ_I4                  , Int32x4_2Int1                     , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithW_I4                  , Int32x4_2Int1                     , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Lt_I4                     , Int32x4_3                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Gt_I4                     , Int32x4_3                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Eq_I4                     , Int32x4_3                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Select_I4                 , Int32x4_4                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_And_I4                    , Int32x4_3                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Or_I4                     , Int32x4_3                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Xor_I4                    , Int32x4_3                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Not_I4                    , Int32x4_2                         , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Shr_I4                    , Int32x4_2Int1                     , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_ShrA_I4                   , Int32x4_2Int1                     , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Shl_I4                    , Int32x4_2Int1                     , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_LdLane_I4                 , Int1Int32x4_1IntConst1            , None           ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_LdSignMask_I4             , Int1Int32x4_1                     , None           ,        OpCanCSE          )
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_Ld_I4                     , Int32x4_2                         , None           ,        None          )
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_LdSlot_I4                 , ElementSlot                       , None           ,        None          )     
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_StSlot_I4                 , ElementSlot                       , None           ,        None          )         
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_Return_I4                 , Int32x4_2                         , None           ,        None          )
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_I_ArgOut_I4               , Reg1Int32x4_1                     , None           ,        None          )  
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_I_Conv_VTI4               , Int32x4_2                         , None           ,        None          )

// Float32x4  
MACRO_SIMD_WMS              ( Simd128_FloatsToF4                , Float32x4_1Float4                 , None          ,        OpCanCSE          )  
MACRO_SIMD_WMS              ( Simd128_Splat_F4                  , Float32x4_1Float1                 , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromFloat64x2_F4          , Float32x4_1Float64x2_1            , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromFloat64x2Bits_F4      , Float32x4_1Float64x2_1            , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromInt32x4_F4            , Float32x4_1Int32x4_1              , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromInt32x4Bits_F4        , Float32x4_1Int32x4_1              , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Abs_F4                    , Float32x4_2                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Neg_F4                    , Float32x4_2                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Add_F4                    , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Sub_F4                    , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Mul_F4                    , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Div_F4                    , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Clamp_F4                  , Float32x4_4                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Min_F4                    , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Max_F4                    , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Rcp_F4                    , Float32x4_2                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_RcpSqrt_F4                , Float32x4_2                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Sqrt_F4                   , Float32x4_2                       , None          ,        OpCanCSE          )
//MACRO_SIMD_WMS              ( Simd128_Swizzle_F4                , Float32x4_2Int4                   , None          ,        OpCanCSE          )
//MACRO_SIMD_WMS              ( Simd128_Shuffle_F4                , Float32x4_3Int4                   , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithX_F4                  , Float32x4_2Float1                 , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithY_F4                  , Float32x4_2Float1                 , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithZ_F4                  , Float32x4_2Float1                 , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithW_F4                  , Float32x4_2Float1                 , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Lt_F4                     , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_LtEq_F4                   , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Eq_F4                     , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Neq_F4                    , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Gt_F4                     , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_GtEq_F4                   , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Select_F4                 , Float32x4_1Int32x4_1Float32x4_2   , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_And_F4                    , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Or_F4                     , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Xor_F4                    , Float32x4_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Not_F4                    , Float32x4_2                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_LdLane_F4                 , Float1Float32x4_1IntConst1        , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_LdSignMask_F4             , Int1Float32x4_1                   , None          ,        OpCanCSE          )

MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_Ld_F4                     , Float32x4_2                       , None          ,        None          )  
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_LdSlot_F4                 , ElementSlot                       , None          ,        None          )  
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_StSlot_F4                 , ElementSlot                       , None          ,        None          )         
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_Return_F4                 , Float32x4_2                       , None          ,        None          )
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_I_ArgOut_F4               , Reg1Float32x4_1                   , None          ,        None          )  
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_I_Conv_VTF4               , Float32x4_2                       , None          ,        None          )

// Float64x2
MACRO_SIMD_WMS              ( Simd128_DoublesToD2               , Float64x2_1Double2                , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Splat_D2                  , Float64x2_1Double1                , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromFloat32x4_D2          , Float64x2_1Float32x4_1            , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromFloat32x4Bits_D2      , Float64x2_1Float32x4_1            , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromInt32x4_D2            , Float64x2_1Int32x4_1              , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_FromInt32x4Bits_D2        , Float64x2_1Int32x4_1              , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Abs_D2                    , Float64x2_2                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Neg_D2                    , Float64x2_2                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Add_D2                    , Float64x2_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Sub_D2                    , Float64x2_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Mul_D2                    , Float64x2_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Div_D2                    , Float64x2_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Clamp_D2                  , Float64x2_4                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Min_D2                    , Float64x2_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Max_D2                    , Float64x2_3                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Rcp_D2                    , Float64x2_2                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_RcpSqrt_D2                , Float64x2_2                       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Sqrt_D2                   , Float64x2_2                       , None          ,        OpCanCSE          )
//MACRO_SIMD_WMS              ( Simd128_Swizzle_D2                , Float64x2_2Int2                 , None          ,        OpCanCSE          )
//MACRO_SIMD_WMS              ( Simd128_Shuffle_D2                , Float64x2_3Int2                 , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithX_D2                  , Float64x2_2Double1                , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_WithY_D2                  , Float64x2_2Double1                , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_Lt_D2                     , Float64x2_3                       , None          ,        OpCanCSE          )  
MACRO_SIMD_WMS              ( Simd128_LtEq_D2                   , Float64x2_3                       , None          ,        OpCanCSE          )  
MACRO_SIMD_WMS              ( Simd128_Eq_D2                     , Float64x2_3                       , None          ,        OpCanCSE          )  
MACRO_SIMD_WMS              ( Simd128_Neq_D2                    , Float64x2_3                       , None          ,        OpCanCSE          )  
MACRO_SIMD_WMS              ( Simd128_Gt_D2                     , Float64x2_3                       , None          ,        OpCanCSE          )  
MACRO_SIMD_WMS              ( Simd128_GtEq_D2                   , Float64x2_3                       , None          ,        OpCanCSE          )  
MACRO_SIMD_WMS              ( Simd128_Select_D2                 , Float64x2_1Int32x4_1Float64x2_2   , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_LdLane_D2                 , Double1Float64x2_1IntConst1       , None          ,        OpCanCSE          )
MACRO_SIMD_WMS              ( Simd128_LdSignMask_D2             , Int1Float64x2_1                   , None          ,        OpCanCSE          )

MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_Ld_D2                     , Float64x2_2                       , None          ,        None          )    
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_LdSlot_D2                 , ElementSlot                       , None          ,        None          )                     
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_StSlot_D2                 , ElementSlot                       , None          ,        None          )         
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_Return_D2                 , Float64x2_2                       , None          ,        None          )
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_I_ArgOut_D2               , Reg1Float64x2_1                   , None          ,        None          )                     
MACRO_SIMD_ASMJS_ONLY_WMS   ( Simd128_I_Conv_VTD2               , Float64x2_2                       , None          ,        None          )

MACRO_SIMD_BACKEND_ONLY     ( Simd128_LdC                       , Empty                              , None           ,        OpCanCSE          )               // Load Simd128 const stack slot
MACRO_SIMD                  ( Simd128_End                       , Empty                              , None           ,        None          )               // Just a marker to indicate SIMD opcodes region

#undef MACRO_SIMD
#undef MACRO_SIMD_WMS
#undef MACRO_SIMD_ASMJS_ONLY_WMS
#undef MACRO_SIMD_BACKEND_ONLY
