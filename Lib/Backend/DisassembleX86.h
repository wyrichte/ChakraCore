/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#if defined(_M_X86)
#if DBG_DUMP
struct DIS_STATE;

int OP_ST (DIS_STATE *state, int ignore);
int OP_STi (DIS_STATE *state, int ignore);
int OP_indirE (DIS_STATE *state, int bytemode);
int OP_E (DIS_STATE *state, int bytemode);
int OP_G (DIS_STATE *state, int bytemode);
int OP_REG (DIS_STATE *state, int code);
int OP_I (DIS_STATE *state, int bytemode);
int OP_sI (DIS_STATE *state, int bytemode);
int OP_J (DIS_STATE *state, int bytemode);
int OP_SEG (DIS_STATE *state, int dummy);
int OP_DIR (DIS_STATE *state, int size);
int OP_OFF (DIS_STATE *state, int bytemode);
int OP_ESDI (DIS_STATE *state, int dummy);
int OP_DSSI (DIS_STATE *state, int dummy);
int OP_ONE (DIS_STATE *state, int dummy);
int OP_C (DIS_STATE *state, int dummy);
int OP_D (DIS_STATE *state, int dummy);
int OP_T (DIS_STATE *state, int dummy);
int OP_rm (DIS_STATE *state, int bytemode);
void print_raw_as_insns(unsigned char *buf,int len,unsigned int start_addr);

#define Eb OP_E, b_mode
#define indirEb OP_indirE, b_mode
#define Gb OP_G, b_mode
#define Ev OP_E, v_mode
#define indirEv OP_indirE, v_mode
#define Ew OP_E, w_mode
#define Ma OP_E, v_mode
#define M OP_E, 0
#define Mp OP_E, 0    /* ? */
#define Gv OP_G, v_mode
#define Gw OP_G, w_mode
#define Rw OP_rm, w_mode
#define Rd OP_rm, d_mode
#define Ib OP_I, b_mode
#define sIb OP_sI, b_mode /* sign extened byte */
#define Iv OP_I, v_mode
#define Iw OP_I, w_mode
#define Jb OP_J, b_mode
#define Jv OP_J, v_mode
#define ONE OP_ONE, 0
#define Cd OP_C, d_mode
#define Dd OP_D, d_mode
#define Td OP_T, d_mode

#define eAX OP_REG, eAX_reg
#define eBX OP_REG, eBX_reg
#define eCX OP_REG, eCX_reg
#define eDX OP_REG, eDX_reg
#define eSP OP_REG, eSP_reg
#define eBP OP_REG, eBP_reg
#define eSI OP_REG, eSI_reg
#define eDI OP_REG, eDI_reg
#define AL OP_REG, al_reg
#define CL OP_REG, cl_reg
#define DL OP_REG, dl_reg
#define BL OP_REG, bl_reg
#define AH OP_REG, ah_reg
#define CH OP_REG, ch_reg
#define DH OP_REG, dh_reg
#define BH OP_REG, bh_reg
#define AX OP_REG, ax_reg
#define DX OP_REG, dx_reg
#define indirDX OP_REG, indir_dx_reg

#define Sw OP_SEG, w_mode
#define Ap OP_DIR, lptr
#define Av OP_DIR, v_mode
#define Ob OP_OFF, b_mode
#define Ov OP_OFF, v_mode
#define Xb OP_DSSI, b_mode
#define Xv OP_DSSI, v_mode
#define Yb OP_ESDI, b_mode
#define Yv OP_ESDI, v_mode

#define es OP_REG, es_reg
#define ss OP_REG, ss_reg
#define cs OP_REG, cs_reg
#define ds OP_REG, ds_reg
#define fs OP_REG, fs_reg
#define gs OP_REG, gs_reg

#define b_mode 1
#define v_mode 2
#define w_mode 3
#define d_mode 4

#define es_reg 100
#define cs_reg 101
#define ss_reg 102
#define ds_reg 103
#define fs_reg 104
#define gs_reg 105
#define eAX_reg 107
#define eCX_reg 108
#define eDX_reg 109
#define eBX_reg 110
#define eSP_reg 111
#define eBP_reg 112
#define eSI_reg 113
#define eDI_reg 114

#define lptr 115

#define al_reg 116
#define cl_reg 117
#define dl_reg 118
#define bl_reg 119
#define ah_reg 120
#define ch_reg 121
#define dh_reg 122
#define bh_reg 123

#define ax_reg 124
#define cx_reg 125
#define dx_reg 126
#define bx_reg 127
#define sp_reg 128
#define bp_reg 129
#define si_reg 130
#define di_reg 131

#define indir_dx_reg 150

#define GRP1b NULL, NULL, 0
#define GRP1S NULL, NULL, 1
#define GRP1Ss NULL, NULL, 2
#define GRP2b NULL, NULL, 3
#define GRP2S NULL, NULL, 4
#define GRP2b_one NULL, NULL, 5
#define GRP2S_one NULL, NULL, 6
#define GRP2b_cl NULL, NULL, 7
#define GRP2S_cl NULL, NULL, 8
#define GRP3b NULL, NULL, 9
#define GRP3S NULL, NULL, 10
#define GRP4  NULL, NULL, 11
#define GRP5  NULL, NULL, 12
#define GRP6  NULL, NULL, 13
#define GRP7 NULL, NULL, 14
#define GRP8 NULL, NULL, 15

#define FLOATCODE 50
#define FLOAT NULL, NULL, FLOATCODE

struct DISx86_OP {
  char *name;
  int (*op1)(DIS_STATE *, int);
  int bytemode1;
  int (*op2)(DIS_STATE *, int);
  int bytemode2;
  int (*op3)(DIS_STATE *, int);
  int bytemode3;
};

extern char *RegName(int i);
#endif
#endif