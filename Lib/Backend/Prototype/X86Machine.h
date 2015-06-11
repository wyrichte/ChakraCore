/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#define X86_EAX   0x0
#define X86_ECX   0x1
#define X86_EDX   0x2
#define X86_EBX   0x3
#define X86_ESP   0x4  
#define X86_EBP   0x5
#define X86_ESI   0x6
#define X86_EDI   0x7

#define X86_REG_NONE 16 /* no register needed */

#define X86_FRAME_POINTER X86_EBP
#define X86_TEMP_REGISTER X86_EAX
#define X86_STACK_POINTER X86_ESP
#define X86_NINTREGS 8

#define X86_CALLEE_SAVED 0xf8

#define I_LDB     0x8A

#define I_LD      0x8B
#define I_LDUB1   0x0F
#define I_LDUB2   0xB6
#define I_LDSB1   0x0F
#define I_LDSB2   0xBE
#define I_LDSH1   0x0F
#define I_LDSH2   0xBF
#define I_LDUH1   0x0F
#define I_LDUH2   0xB7

#define I_SETNO_1 0x0F
#define I_SETNO_2 0x91

#define I_JO_PCREL32_1 0x0F
#define I_JO_PCREL32_2 0x80

#define I_DATA16  0x66

#define I_STH1    I_DATA16
#define I_STH2    0x89
#define I_ST      0x89
#define I_STB     0x88

#define I_ADD_RM  0x03
#define I_IMUL_RM1 0x0F
#define I_IMUL_RM2 0xAF
#define I_IMUL_I32 0x69

#define I_IDIV     0xF7
#define I_UDIV     0xF7
#define I_IDIV_RVAL 0x7
#define I_UDIV_RVAL 0x6

#define I_MUL_RM  0x 
#define I_SUB_RM  0x2B
#define I_AND_RM  0x23
#define I_OR_RM   0x0B
#define I_XOR_RM  0x33

#define I_ADD_MR  0x01
#define I_SUB_MR  0x29
#define I_AND_MR  0x21
#define I_OR_MR   0x09
#define I_XOR_MR  0x31

#define I_ADD_MI8        0x83
#define I_ADD_MI8_RVAL   0x0
#define I_SUB_MI8        0x83
#define I_SUB_MI8_RVAL   0x5
#define I_AND_MI8        0x83
#define I_AND_MI8_RVAL   0x4
#define I_OR_MI8         0x83
#define I_OR_MI8_RVAL    0x1
#define I_XOR_MI8        0x83
#define I_XOR_MI8_RVAL   0x6

#define I_ADD_MI32       0x81
#define I_ADD_MI32_RVAL  0x0
#define I_SUB_MI32       0x81
#define I_SUB_MI32_RVAL  0x5
#define I_AND_MI32       0x81
#define I_AND_MI32_RVAL  0x4
#define I_OR_MI32        0x81
#define I_OR_MI32_RVAL   0x1
#define I_XOR_MI32       0x81
#define I_XOR_MI32_RVAL  0x6

#define I_SLL_CL      0xD3
#define I_SLL_CL_RVAL 0x4
#define I_SRL_CL      0xD3
#define I_SRL_CL_RVAL 0x5
#define I_SRA_CL      0xD3
#define I_SRA_CL_RVAL 0x7

#define I_SLL_I8      0xC1
#define I_SLL_I8_RVAL 0x4
#define I_SRL_I8      0xC1
#define I_SRL_I8_RVAL 0x5
#define I_SRA_I8      0xC1
#define I_SRA_I8_RVAL 0x7

#define I_ROR         0xC1
#define I_ROR_RVAL    0x1

#define I_ROL         0xC1
#define I_ROL_RVAL    0x0

#define I_SHRD1       0x0F
#define I_SHRD2       0XAC

#define I_NOT         0xF7
#define I_NOT_RVAL    0x2
#define I_NEG         0xF7
#define I_NEG_RVAL    0x3

#define I_FADD        0xD8
#define I_FADD_RVAL   0x0
#define I_FSUB        0xD8
#define I_FSUB_RVAL   0x4
#define I_FSUBR        0xD8
#define I_FSUBR_RVAL   0x5
#define I_FMUL        0xD8
#define I_FMUL_RVAL   0x1
#define I_FDIV        0xD8
#define I_FDIV_RVAL   0x6
#define I_FDIVR        0xD8
#define I_FDIVR_RVAL   0x7

#define I_FADDD        0xDC
#define I_FADDD_RVAL   0x0
#define I_FSUBD        0xDC
#define I_FSUBD_RVAL   0x4
#define I_FSUBRD       0xDC
#define I_FSUBRD_RVAL   0x5
#define I_FMULD        0xDC
#define I_FMULD_RVAL   0x1
#define I_FDIVD        0xDC
#define I_FDIVD_RVAL   0x6
#define I_FDIVRD        0xDC
#define I_FDIVRD_RVAL   0x7

#define I_FCHS1        0xD9
#define I_FCHS2        0xE0
#define I_FABS1        0xD9
#define I_FABS2        0xE1
#define I_FSQRT1       0xD9
#define I_FSQRT2       0xFA

#define I_ICALL        0xFF
#define I_ICALL_RVAL   0x2
#define I_CALL_PCREL   0xE8
#define I_JMP_PCREL    0xE9
#define I_IJMP         0xFF
#define I_IJMP_RVAL    0x4
#define I_RET          0xC3

#define I_SET_COND_1   0x0F
#define I_SET_COND_EQ  0x94
#define I_SET_COND_NE  0x95
#define I_SET_COND_GT  0x9F
#define I_SET_COND_GTE 0x9D
#define I_SET_COND_LT  0x9C
#define I_SET_COND_LTE 0x9E

#define I_ICOND_GTU    0x87
#define I_ICOND_GEU    0x83
#define I_ICOND_LTU    0x82
#define I_ICOND_LEU    0x86
#define I_ICOND_EQ     0x84
#define I_ICOND_Z      0x84

#define I_ICOND_GT     0x8F
#define I_ICOND_GE     0x8D
#define I_ICOND_LT     0x8C
#define I_ICOND_LE     0x8E

#define I_ICOND_NE     0x85
#define I_ICOND_NZ     0x85
#define I_ICONDBYTE_GTU    0x77
#define I_ICONDBYTE_GEU    0x73
#define I_ICONDBYTE_LTU    0x72
#define I_ICONDBYTE_LEU    0x76
#define I_ICONDBYTE_EQ     0x74
#define I_ICONDBYTE_GT     0x7F
#define I_ICONDBYTE_GE     0x7D
#define I_ICONDBYTE_LT     0x7C
#define I_ICONDBYTE_LE     0x7E
#define I_ICONDBYTE_NE     0x75

#define I_CMP_MR       0x39
#define I_CMP_RM       0x3B
#define I_CMP_I8_EAX   0x3C
#define I_CMP_I8       0x83
#define I_CMP_I8_RVAL  0x7
#define I_CMP_I32      0x81
#define I_CMP_I32_RVAL 0x7

#define I_JCC1         0x0F

#define I_NOP          0x90

#define I_MOVDW_REG 0xB8
#define I_MOV_RR    0x8B
#define I_MOVB_REG  0xB0
#define I_MOVB         0xC6
#define I_MOVB_RVAL    0x0
#define I_MOVDW        0xC7
#define I_MOVDW_RVAL   0x0

#define I_FLD          0xD9
#define I_FLD_RVAL     0x0
#define I_FLDD         0xDD
#define I_FLDD_RVAL    0x0

#define I_FSTP         0xD9
#define I_FSTP_RVAL    0x3
#define I_FSTDP        0xDD
#define I_FSTDP_RVAL   0x3

#define I_FISTP        0xDB
#define I_FISTP_RVAL   0x3

#define I_FILD         0xDB
#define I_FILD_RVAL    0x0

#define I_FCOMP         0xD8
#define I_FCOMP_RVAL    0x3
#define I_FCOMPD        0xDC
#define I_FCOMPD_RVAL   0x3

#define I_FNSTSW_AX1   0xDF
#define I_FNSTSW_AX2   0xE0

#define I_FNSTCW       0xD9
#define I_FNSTCW_RVAL  0x7

#define I_FLDCW        0xD9
#define I_FLDCW_RVAL   0x5

#define I_FWAIT        0x9B

#define I_PUSH_MEM      0xFF
#define I_PUSH_MEM_RVAL 0x6

#define I_PUSH_MI32    0x68
#define I_POP_EAX      0x58
#define I_PUSH_EAX     0x50

#define I_PUSH_MI8     0x6A

#define I_PUSH_REG I_PUSH_EAX
#define I_POP_REG  I_POP_EAX

#define I_POP_EDX      0x58+2
#define I_PUSH_EDX     0x50+2

#define I_ANDB_AH      0x80
#define I_ANDB_AH_RVAL 0x4

#define I_CMPB_AH      0x80
#define I_CMPB_AH_RVAL 0x7

#define I_XORB_AH      0x80
#define I_XORB_AH_RVAL 0x6

#define I_ORB_AH       0x80
#define I_ORB_AH_RVAL  0x1

#define I_DECB         0xFE
#define I_DECB_RVAL    0x1

#define I_DEC_EAX      0x48
#define I_INC_EAX      0x40
#define I_INC_RM       0xFF
#define I_INC_RM_RVAL  0x0

#define I_CDQ          0x99

#define I_REP          0xF3
#define I_STOSB        0xAA
#define I_STOSD        0xAB
#define I_MOVSB        0xA4
#define I_MOVSD        0xA5

#define I_TEST_MI8     0xF6
#define I_TEST_MI8_RVAL    0x00

#define I_TEST_MI32    0xF7
#define I_TEST_MI32_RVAL 0x00

#define I_LEA          0x8D

#define MOD_NO_DISP_OR_DIRECT  0x0
#define MOD_BYTE_DISPLACEMENT  0x1
#define MOD_DWORD_DISPLACEMENT 0x2
#define MOD_IS_REG             0x3

#define X86_ST_GEN_OP 0x89
#define X86_LD_GEN_OP 0x8B
#define X86_MOV_IMM_BYTE_OP  0xB0  /* reg ORed in */
#define X86_MOV_IMM_DWORD_OP 0xB8  /* reg ORed in */

#define RM_IS_SIB 0x4
#define RM_IS_DIRECT 0x5

#define BASE_IS_NONE 0x5
#define INDEX_IS_NONE 0x4

#define SCALE_1 0x0
#define SCALE_2 0x1
#define SCALE_4 0x2
#define SCALE_8 0x3

inline bool X86FitsSimm8(int offset) {
  return (offset >= -128) && (offset < 128);
}

inline unsigned char
MakeModRM(int mod,int reg,int rm)
{
  AssertMsg(mod>=0 && mod<=3,"bad mod");
  AssertMsg(reg>=0 && reg<=7,"bad reg");
  AssertMsg(rm>=0 && rm<=7,"bad rm");
  return (unsigned char)((mod<<6)|(reg<<3)|rm);
}

inline unsigned char
MakeSIB(int scale,int index,int base) {
  AssertMsg(scale>=0 && scale<=3,"bad scale");
  AssertMsg(index>=0 && index<=7,"bad index");
  AssertMsg(base>=0 && base<=7,"bad base");
  return (unsigned char)((scale<<6)|(index<<3)|base);
}
