//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "BackEnd.h"
#include "DisassembleX86.h"

#if defined(_M_X86)
#if DBG_DUMP

#undef ASSERT
#define ASSERT(x) AssertMsg(x,"disasm");
#define CRASH(m) AssertMsg(false,m);
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif // !defined(_CRT_SECURE_NO_WARNINGS)
#define TRUE 1
#define FALSE 0
#define PREFIX_REPZ  1
#define PREFIX_REPNZ 2
#define PREFIX_LOCK  4
#define PREFIX_CS    8
#define PREFIX_SS    0x10
#define PREFIX_DS    0x20
#define PREFIX_ES    0x40
#define PREFIX_FS    0x80
#define PREFIX_GS    0x100
#define PREFIX_DATA  0x200
#define PREFIX_ADR   0x400
#define PREFIX_FWAIT 0x800

#define MAX_NAME_SIZE 128
#pragma warning(disable : 4996)

/*  disassembly state  */
struct DIS_STATE {
  unsigned int pc;
  unsigned char *insn_buf;   /*  pointer to base of orig instruction data  */
  int insn_buf_length;       /*  length of it  */
  unsigned char *insn_bufp;  /*  curr pointer into it  */
  char temp_name_buf[MAX_NAME_SIZE];  /*  build name here  */
  char op_out[3][MAX_NAME_SIZE]; /*  build operand names here */
/*  char *op_out[3];           /*  build operand names here */
  int op_index[3];           /*  keep addr vs. string info for each op  */
  int op_address[3];         /*  address info per op  */
  int which_op;              /*  current operand  */
  char data_flag, addr_flag; /*  operand and addr size toggle (16/32 bit) */
  int prefix_flags;
  unsigned char mod, reg, rm; /*  instruction type codes  */
  char rm_op_len;             /* length of r/m operand */
};

static const DISx86_OP disx86[] = {
  /* 00 */
  { "addb", Eb, Gb },
  { "addS", Ev, Gv },
  { "addb", Gb, Eb },
  { "addS", Gv, Ev },
  { "addb", AL, Ib },
  { "addS", eAX, Iv },
  { "pushl",  es },
  { "popl", es },
  /* 08 */
  { "orb",  Eb, Gb },
  { "orS",  Ev, Gv },
  { "orb",  Gb, Eb },
  { "orS",  Gv, Ev },
  { "orb",  AL, Ib },
  { "orS",  eAX, Iv },
  { "pushl",  cs },
  { "(bad)" },  /* 0x0f extended opcode escape */
  /* 10 */
  { "adcb", Eb, Gb },
  { "adcS", Ev, Gv },
  { "adcb", Gb, Eb },
  { "adcS", Gv, Ev },
  { "adcb", AL, Ib },
  { "adcS", eAX, Iv },
  { "pushl",  ss },
  { "popl", ss },
  /* 18 */
  { "sbbb", Eb, Gb },
  { "sbbS", Ev, Gv },
  { "sbbb", Gb, Eb },
  { "sbbS", Gv, Ev },
  { "sbbb", AL, Ib },
  { "sbbS", eAX, Iv },
  { "pushl",  ds },
  { "popl", ds },
  /* 20 */
  { "andb", Eb, Gb },
  { "andS", Ev, Gv },
  { "andb", Gb, Eb },
  { "andS", Gv, Ev },
  { "andb", AL, Ib },
  { "andS", eAX, Iv },
  { "(bad)" },      /* SEG ES prefix */
  { "daa" },
  /* 28 */
  { "subb", Eb, Gb },
  { "subS", Ev, Gv },
  { "subb", Gb, Eb },
  { "subS", Gv, Ev },
  { "subb", AL, Ib },
  { "subS", eAX, Iv },
  { "(bad)" },      /* SEG CS prefix */
  { "das" },
  /* 30 */
  { "xorb", Eb, Gb },
  { "xorS", Ev, Gv },
  { "xorb", Gb, Eb },
  { "xorS", Gv, Ev },
  { "xorb", AL, Ib },
  { "xorS", eAX, Iv },
  { "(bad)" },      /* SEG SS prefix */
  { "aaa" },
  /* 38 */
  { "cmpb", Eb, Gb },
  { "cmpS", Ev, Gv },
  { "cmpb", Gb, Eb },
  { "cmpS", Gv, Ev },
  { "cmpb", AL, Ib },
  { "cmpS", eAX, Iv },
  { "(bad)" },      /* SEG DS prefix */
  { "aas" },
  /* 40 */
  { "incS", eAX },
  { "incS", eCX },
  { "incS", eDX },
  { "incS", eBX },
  { "incS", eSP },
  { "incS", eBP },
  { "incS", eSI },
  { "incS", eDI },
  /* 48 */
  { "decS", eAX },
  { "decS", eCX },
  { "decS", eDX },
  { "decS", eBX },
  { "decS", eSP },
  { "decS", eBP },
  { "decS", eSI },
  { "decS", eDI },
  /* 50 */
  { "pushS",  eAX },
  { "pushS",  eCX },
  { "pushS",  eDX },
  { "pushS",  eBX },
  { "pushS",  eSP },
  { "pushS",  eBP },
  { "pushS",  eSI },
  { "pushS",  eDI },
  /* 58 */
  { "popS", eAX },
  { "popS", eCX },
  { "popS", eDX },
  { "popS", eBX },
  { "popS", eSP },
  { "popS", eBP },
  { "popS", eSI },
  { "popS", eDI },
  /* 60 */
  { "pusha" },
  { "popa" },
  { "boundS", Gv, Ma },
  { "arpl", Ew, Gw },
  { "(bad)" },      /* seg fs */
  { "(bad)" },      /* seg gs */
  { "(bad)" },      /* op size prefix */
  { "(bad)" },      /* adr size prefix */
  /* 68 */
  { "pushS",  Iv },   /* x86 book wrong */
  { "imulS",  Gv, Ev, Iv },
  { "pushl",  sIb },    /* push of byte really pushes 4 bytes */
  { "imulS",  Gv, Ev, Ib },
  { "insb", Yb, indirDX },
  { "insS", Yv, indirDX },
  { "outsb",  indirDX, Xb },
  { "outsS",  indirDX, Xv },
  /* 70 */
  { "jo",   Jb },
  { "jno",  Jb },
  { "jb",   Jb },
  { "jae",  Jb },
  { "je",   Jb },
  { "jne",  Jb },
  { "jbe",  Jb },
  { "ja",   Jb },
  /* 78 */
  { "js",   Jb },
  { "jns",  Jb },
  { "jp",   Jb },
  { "jnp",  Jb },
  { "jl",   Jb },
  { "jnl",  Jb },
  { "jle",  Jb },
  { "jg",   Jb },
  /* 80 */
  { GRP1b },
  { GRP1S },
  { "(bad)" },
  { GRP1Ss },
  { "testb",  Eb, Gb },
  { "testS",  Ev, Gv },
  { "xchgb",  Eb, Gb },
  { "xchgS",  Ev, Gv },
  /* 88 */
  { "movb", Eb, Gb },
  { "movS", Ev, Gv },
  { "movb", Gb, Eb },
  { "movS", Gv, Ev },
  { "movw", Ew, Sw },
  { "leaS", Gv, M },
  { "movw", Sw, Ew },
  { "popS", Ev },
  /* 90 */
  { "nop" },
  { "xchgS",  eCX, eAX },
  { "xchgS",  eDX, eAX },
  { "xchgS",  eBX, eAX },
  { "xchgS",  eSP, eAX },
  { "xchgS",  eBP, eAX },
  { "xchgS",  eSI, eAX },
  { "xchgS",  eDI, eAX },
  /* 98 */
  { "cwtl" },
  { "cltd" },
  { "lcall",  Ap },
  { "(bad)" },    /* fwait */
  { "pushf" },
  { "popf" },
  { "sahf" },
  { "lahf" },
  /* a0 */
  { "movb", AL, Ob },
  { "movS", eAX, Ov },
  { "movb", Ob, AL },
  { "movS", Ov, eAX },
  { "movsb",  Yb, Xb },
  { "movsS",  Yv, Xv },
  { "cmpsb",  Yb, Xb },
  { "cmpsS",  Yv, Xv },
  /* a8 */
  { "testb",  AL, Ib },
  { "testS",  eAX, Iv },
  { "stosb",  Yb, AL },
  { "stosS",  Yv, eAX },
  { "lodsb",  AL, Xb },
  { "lodsS",  eAX, Xv },
  { "scasb",  AL, Xb },
  { "scasS",  eAX, Xv },
  /* b0 */
  { "movb", AL, Ib },
  { "movb", CL, Ib },
  { "movb", DL, Ib },
  { "movb", BL, Ib },
  { "movb", AH, Ib },
  { "movb", CH, Ib },
  { "movb", DH, Ib },
  { "movb", BH, Ib },
  /* b8 */
  { "movS", eAX, Iv },
  { "movS", eCX, Iv },
  { "movS", eDX, Iv },
  { "movS", eBX, Iv },
  { "movS", eSP, Iv },
  { "movS", eBP, Iv },
  { "movS", eSI, Iv },
  { "movS", eDI, Iv },
  /* c0 */
  { GRP2b },
  { GRP2S },
  { "ret",  Iw },
  { "ret" },
  { "lesS", Gv, Mp },
  { "ldsS", Gv, Mp },
  { "movb", Eb, Ib },
  { "movS", Ev, Iv },
  /* c8 */
  { "enter",  Iw, Ib },
  { "leave" },
  { "lret", Iw },
  { "lret" },
  { "int3" },
  { "int",  Ib },
  { "into" },
  { "iret" },
  /* d0 */
  { GRP2b_one },
  { GRP2S_one },
  { GRP2b_cl },
  { GRP2S_cl },
  { "aam",  Ib },
  { "aad",  Ib },
  { "(bad)" },
  { "xlat" },
  /* d8 */
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  /* e0 */
  { "loopne", Jb },
  { "loope",  Jb },
  { "loop", Jb },
  { "jCcxz",  Jb },
  { "inb",  AL, Ib },
  { "inS",  eAX, Ib },
  { "outb", Ib, AL },
  { "outS", Ib, eAX },
  /* e8 */
  { "call", Av },
  { "jmp",  Jv },
  { "ljmp", Ap },
  { "jmp",  Jb },
  { "inb",  AL, indirDX },
  { "inS",  eAX, indirDX },
  { "outb", indirDX, AL },
  { "outS", indirDX, eAX },
  /* f0 */
  { "(bad)" },      /* lock prefix */
  { "(bad)" },
  { "(bad)" },      /* repne */
  { "(bad)" },      /* repz */
  { "hlt" },
  { "cmc" },
  { GRP3b },
  { GRP3S },
  /* f8 */
  { "clc" },
  { "stc" },
  { "cli" },
  { "sti" },
  { "cld" },
  { "std" },
  { GRP4 },
  { GRP5 },
};

static const DISx86_OP disx86_twobyte[] = {
  /* 00 */
  { GRP6 },
  { GRP7 },
  { "larS", Gv, Ew },
  { "lslS", Gv, Ew },
  { "(bad)" },
  { "(bad)" },
  { "clts" },
  { "(bad)" },
  /* 08 */
  { "invd" },
  { "wbinvd" },
  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 10 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 18 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 20 */
  /* these are all backward in appendix A of the intel book */
  { "movl", Rd, Cd },
  { "movl", Rd, Dd },
  { "movl", Cd, Rd },
  { "movl", Dd, Rd },
  { "movl", Rd, Td },
  { "(bad)" },
  { "movl", Td, Rd },
  { "(bad)" },
  /* 28 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 30 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 38 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 40 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 48 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 50 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 58 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 60 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 68 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 70 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 78 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* 80 */
  { "jo", Jv },
  { "jno", Jv },
  { "jb", Jv },
  { "jae", Jv },
  { "je", Jv },
  { "jne", Jv },
  { "jbe", Jv },
  { "ja", Jv },
  /* 88 */
  { "js", Jv },
  { "jns", Jv },
  { "jp", Jv },
  { "jnp", Jv },
  { "jl", Jv },
  { "jge", Jv },
  { "jle", Jv },
  { "jg", Jv },
  /* 90 */
  { "seto", Eb },
  { "setno", Eb },
  { "setb", Eb },
  { "setae", Eb },
  { "sete", Eb },
  { "setne", Eb },
  { "setbe", Eb },
  { "seta", Eb },
  /* 98 */
  { "sets", Eb },
  { "setns", Eb },
  { "setp", Eb },
  { "setnp", Eb },
  { "setl", Eb },
  { "setge", Eb },
  { "setle", Eb },
  { "setg", Eb },
  /* a0 */
  { "pushl", fs },
  { "popl", fs },
  { "(bad)" },
  { "btS", Ev, Gv },
  { "shldS", Ev, Gv, Ib },
  { "shldS", Ev, Gv, CL },
  { "(bad)" },
  { "(bad)" },
  /* a8 */
  { "pushl", gs },
  { "popl", gs },
  { "(bad)" },
  { "btsS", Ev, Gv },
  { "shrdS", Ev, Gv, Ib },
  { "shrdS", Ev, Gv, CL },
  { "(bad)" },
  { "imulS", Gv, Ev },
  /* b0 */
  { "cmpxchgb", Eb, Gb },
  { "cmpxchgS", Ev, Gv },
  { "lssS", Gv, Mp }, /* x86 lists only Mp */
  { "btrS", Ev, Gv },
  { "lfsS", Gv, Mp }, /* x86 lists only Mp */
  { "lgsS", Gv, Mp }, /* x86 lists only Mp */
  { "movzbS", Gv, Eb },
  { "movzwS", Gv, Ew },
  /* b8 */
  { "(bad)" },
  { "(bad)" },
  { GRP8 },
  { "btcS", Ev, Gv },
  { "bsfS", Gv, Ev },
  { "bsrS", Gv, Ev },
  { "movsbS", Gv, Eb },
  { "movswS", Gv, Ew },
  /* c0 */
  { "xaddb", Eb, Gb },
  { "xaddS", Ev, Gv },
  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* c8 */
  { "bswap", eAX },
  { "bswap", eCX },
  { "bswap", eDX },
  { "bswap", eBX },
  { "bswap", eSP },
  { "bswap", eBP },
  { "bswap", eSI },
  { "bswap", eDI },
  /* d0 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* d8 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* e0 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* e8 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* f0 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  /* f8 */
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
  { "(bad)" },  { "(bad)" },  { "(bad)" },  { "(bad)" },
};

static const char *names32[]={
  "eax","ecx","edx","ebx", "esp","ebp","esi","edi",
};

char *RegName(int i) {
    return names32[i];
}


static const char *names16[] = {
  "ax","cx","dx","bx","sp","bp","si","di",
};
static const char *names8[] = {
  "al","cl","dl","bl","ah","ch","dh","bh",
};
static const char *names_seg[] = {
  "es","cs","ss","ds","fs","gs","?","?",
};

DISx86_OP grps[][8] = {
  /* GRP1b */
  {
    { "addb", Eb, Ib },
    { "orb",  Eb, Ib },
    { "adcb", Eb, Ib },
    { "sbbb", Eb, Ib },
    { "andb", Eb, Ib },
    { "subb", Eb, Ib },
    { "xorb", Eb, Ib },
    { "cmpb", Eb, Ib }
  },
  /* GRP1S */
  {
    { "addS", Ev, Iv },
    { "orS",  Ev, Iv },
    { "adcS", Ev, Iv },
    { "sbbS", Ev, Iv },
    { "andS", Ev, Iv },
    { "subS", Ev, Iv },
    { "xorS", Ev, Iv },
    { "cmpS", Ev, Iv }
  },
  /* GRP1Ss */
  {
    { "addS", Ev, sIb },
    { "orS",  Ev, sIb },
    { "adcS", Ev, sIb },
    { "sbbS", Ev, sIb },
    { "andS", Ev, sIb },
    { "subS", Ev, sIb },
    { "xorS", Ev, sIb },
    { "cmpS", Ev, sIb }
  },
  /* GRP2b */
  {
    { "rolb", Eb, Ib },
    { "rorb", Eb, Ib },
    { "rclb", Eb, Ib },
    { "rcrb", Eb, Ib },
    { "shlb", Eb, Ib },
    { "shrb", Eb, Ib },
    { "(bad)" },
    { "sarb", Eb, Ib },
  },
  /* GRP2S */
  {
    { "rolS", Ev, Ib },
    { "rorS", Ev, Ib },
    { "rclS", Ev, Ib },
    { "rcrS", Ev, Ib },
    { "shlS", Ev, Ib },
    { "shrS", Ev, Ib },
    { "(bad)" },
    { "sarS", Ev, Ib },
  },
  /* GRP2b_one */
  {
    { "rolb", Eb },
    { "rorb", Eb },
    { "rclb", Eb },
    { "rcrb", Eb },
    { "shlb", Eb },
    { "shrb", Eb },
    { "(bad)" },
    { "sarb", Eb },
  },
  /* GRP2S_one */
  {
    { "rolS", Ev },
    { "rorS", Ev },
    { "rclS", Ev },
    { "rcrS", Ev },
    { "shlS", Ev },
    { "shrS", Ev },
    { "(bad)" },
    { "sarS", Ev },
  },
  /* GRP2b_cl */
  {
    { "rolb", Eb, CL },
    { "rorb", Eb, CL },
    { "rclb", Eb, CL },
    { "rcrb", Eb, CL },
    { "shlb", Eb, CL },
    { "shrb", Eb, CL },
    { "(bad)" },
    { "sarb", Eb, CL },
  },
  /* GRP2S_cl */
  {
    { "rolS", Ev, CL },
    { "rorS", Ev, CL },
    { "rclS", Ev, CL },
    { "rcrS", Ev, CL },
    { "shlS", Ev, CL },
    { "shrS", Ev, CL },
    { "(bad)" },
    { "sarS", Ev, CL }
  },
  /* GRP3b */
  {
    { "testb",  Eb, Ib },
    { "(bad)",  Eb },
    { "notb", Eb },
    { "negb", Eb },
    { "mulb", AL, Eb },
    { "imulb",  AL, Eb },
    { "divb", AL, Eb },
    { "idivb",  AL, Eb }
  },
  /* GRP3S */
  {
    { "testS",  Ev, Iv },
    { "(bad)" },
    { "notS", Ev },
    { "negS", Ev },
    { "mulS", eAX, Ev },
    { "imulS",  eAX, Ev },
    { "divS", eAX, Ev },
    { "idivS",  eAX, Ev },
  },
  /* GRP4 */
  {
    { "incb", Eb },
    { "decb", Eb },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
  },
  /* GRP5 */
  {
    { "incS", Ev },
    { "decS", Ev },
    { "call", indirEv },
    { "lcall",  indirEv },
    { "jmp",  indirEv },
    { "ljmp", indirEv },
    { "pushS",  Ev },
    { "(bad)" },
  },
  /* GRP6 */
  {
    { "sldt", Ew },
    { "str",  Ew },
    { "lldt", Ew },
    { "ltr",  Ew },
    { "verr", Ew },
    { "verw", Ew },
    { "(bad)" },
    { "(bad)" }
  },
  /* GRP7 */
  {
    { "sgdt", Ew },
    { "sidt", Ew },
    { "lgdt", Ew },
    { "lidt", Ew },
    { "smsw", Ew },
    { "(bad)" },
    { "lmsw", Ew },
    { "invlpg", Ew },
  },
  /* GRP8 */
  {
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { "btS",  Ev, Ib },
    { "btsS", Ev, Ib },
    { "btrS", Ev, Ib },
    { "btcS", Ev, Ib },
  }
};

char *float_mem[] = {
  /* d8 */
  "fadds",
  "fmuls",
  "fcoms",
  "fcomps",
  "fsubs",
  "fsubrs",
  "fdivs",
  "fdivrs",
  /*  d9 */
  "flds",
  "(bad)",
  "fsts",
  "fstps",
  "fldenv",
  "fldcw",
  "fNstenv",
  "fNstcw",
  /* da */
  "fiaddl",
  "fimull",
  "ficoml",
  "ficompl",
  "fisubl",
  "fisubrl",
  "fidivl",
  "fidivrl",
  /* db */
  "fildl",
  "(bad)",
  "fistl",
  "fistpl",
  "(bad)",
  "fldt",
  "(bad)",
  "fstpt",
  /* dc */
  "faddl",
  "fmull",
  "fcoml",
  "fcompl",
  "fsubl",
  "fsubrl",
  "fdivl",
  "fdivrl",
  /* dd */
  "fldl",
  "(bad)",
  "fstl",
  "fstpl",
  "frstor",
  "(bad)",
  "fNsave",
  "fNstsw",
  /* de */
  "fiadd",
  "fimul",
  "ficom",
  "ficomp",
  "fisub",
  "fisubr",
  "fidiv",
  "fidivr",
  /* df */
  "fild",
  "(bad)",
  "fist",
  "fistp",
  "fbld",
  "fildll",
  "fbstp",
  "fistpll",
};

#define ST OP_ST, 0
#define STi OP_STi, 0

#define FGRPd9_2 NULL, NULL, 0
#define FGRPd9_4 NULL, NULL, 1
#define FGRPd9_5 NULL, NULL, 2
#define FGRPd9_6 NULL, NULL, 3
#define FGRPd9_7 NULL, NULL, 4
#define FGRPda_5 NULL, NULL, 5
#define FGRPdb_4 NULL, NULL, 6
#define FGRPde_3 NULL, NULL, 7
#define FGRPdf_4 NULL, NULL, 8

DISx86_OP float_reg[][8] = {
  /* d8 */
  {
    { "fadd", ST, STi },
    { "fmul", ST, STi },
    { "fcom", STi },
    { "fcomp",  STi },
    { "fsub", ST, STi },
    { "fsubr",  ST, STi },
    { "fdiv", ST, STi },
    { "fdivr",  ST, STi },
  },
  /* d9 */
  {
    { "fld",  STi },
    { "fxch", STi },
    { FGRPd9_2 },
    { "(bad)" },
    { FGRPd9_4 },
    { FGRPd9_5 },
    { FGRPd9_6 },
    { FGRPd9_7 },
  },
  /* da */
  {
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { FGRPda_5 },
    { "(bad)" },
    { "(bad)" },
  },
  /* db */
  {
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { FGRPdb_4 },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
  },
  /* dc */
  {
    { "fadd", STi, ST },
    { "fmul", STi, ST },
    { "(bad)" },
    { "(bad)" },
    { "fsub", STi, ST },
    { "fsubr",  STi, ST },
    { "fdiv", STi, ST },
    { "fdivr",  STi, ST },
  },
  /* dd */
  {
    { "ffree",  STi },
    { "(bad)" },
    { "fst",  STi },
    { "fstp", STi },
    { "fucom",  STi },
    { "fucomp", STi },
    { "(bad)" },
    { "(bad)" },
  },
  /* de */
  {
    { "faddp",  STi, ST },
    { "fmulp",  STi, ST },
    { "(bad)" },
    { FGRPde_3 },
    { "fsubp",  STi, ST },
    { "fsubrp", STi, ST },
    { "fdivp",  STi, ST },
    { "fdivrp", STi, ST },
  },
  /* df */
  {
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
    { FGRPdf_4 },
    { "(bad)" },
    { "(bad)" },
    { "(bad)" },
  },
};


char *fgrps[][8] = {
  /* d9_2  0 */
  {
    "fnop","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* d9_4  1 */
  {
    "fchs","fabs","(bad)","(bad)","ftst","fxam","(bad)","(bad)",
  },

  /* d9_5  2 */
  {
    "fld1","fldl2t","fldl2e","fldpi","fldlg2","fldln2","fldz","(bad)",
  },

  /* d9_6  3 */
  {
    "f2xm1","fyl2x","fptan","fpatan","fxtract","fprem1","fdecstp","fincstp",
  },

  /* d9_7  4 */
  {
    "fprem","fyl2xp1","fsqrt","fsincos","frndint","fscale","fsin","fcos",
  },

  /* da_5  5 */
  {
    "(bad)","fucompp","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* db_4  6 */
  {
    "feni(287 only)","fdisi(287 only)","fNclex","fNinit",
    "fNsetpm(287 only)","(bad)","(bad)","(bad)",
  },

  /* de_3  7 */
  {
    "(bad)","fcompp","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* df_4  8 */
  {
    "fNstsw","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },
};

void advance_byte(DIS_STATE *state);
void do_address(DIS_STATE *state, int address);
int do_prefix(DIS_STATE *state);
void putop(DIS_STATE *state, __in LPCSTR templ);
void dofloat(DIS_STATE *state);
void do_opcode(DIS_STATE *state);
void append_prefix(DIS_STATE *state);
int get32(DIS_STATE *state);
int get16(DIS_STATE *state);
void set_op(DIS_STATE *state, int op);

/*  advance_byte - move the current pointer forward, copying the byte
 * to the new buffer and updating the current insn's length.
 */
void advance_byte(DIS_STATE *state)
{
  /* FIX: check if length has gone past size of insn */
  state->insn_bufp++;
}

void
do_address(DIS_STATE *state, int address)
{
  char scratchbuf[MAX_NAME_SIZE];

  sprintf_s(scratchbuf, sizeof(scratchbuf), "0x%x",address);
  strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), scratchbuf);
}

/*  do_prefix - check if the instruction has any prefixes; if so, set the
 * appropriate flags, copy the bytes over, and advance the state.  Return
 * TRUE if the prefix is actually an entire instruction, false otherwise.
 */
int do_prefix(DIS_STATE *state)
{
  int prefixes = 0;

  while (TRUE) {
    switch(*state->insn_bufp) {
    case 0xf3:
      prefixes |= PREFIX_REPZ;
      break;
    case 0xf2:
      prefixes |= PREFIX_REPNZ;
      break;
    case 0xf0:
      prefixes |= PREFIX_LOCK;
      break;
    case 0x2e:
      prefixes |= PREFIX_CS;
      break;
    case 0x36:
      prefixes |= PREFIX_SS;
      break;
    case 0x3e:
      prefixes |= PREFIX_DS;
      break;
    case 0x26:
      prefixes |= PREFIX_ES;
      break;
    case 0x64:
      prefixes |= PREFIX_FS;
      break;
    case 0x65:
      prefixes |= PREFIX_GS;
      break;
    case 0x66:
      prefixes |= PREFIX_DATA;
      break;
    case 0x67:
      prefixes |= PREFIX_ADR;
      break;
    case 0x9b:
      prefixes |= PREFIX_FWAIT;
      break;
    default:
      goto done;
      break;
    }
    advance_byte(state);
  }
 done:
  state->prefix_flags = prefixes;

  if (prefixes & PREFIX_REPZ)
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "repz ");
  if (prefixes & PREFIX_REPNZ)
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "repnz ");
  if (prefixes & PREFIX_LOCK)
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "lock ");

  if ((prefixes & PREFIX_FWAIT)
      && ((*state->insn_bufp < 0xd8) || (*state->insn_bufp > 0xdf)))
    {
      /*      printf("FWAIT check: make sure this works right\n"); */
      /* fwait not followed by floating point instruction */
      strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "fwait");
      return(TRUE);
    }

  state->data_flag = 1;
  state->addr_flag = 1;

  if (prefixes & PREFIX_DATA)
    state->data_flag ^= 1;

  if (prefixes & PREFIX_ADR) {
    state->addr_flag ^= 1;
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "addr16 ");
  }

  return(FALSE);
}

/* putop - given a template for the opcode, where capital letters are macros,
 * expand appropriately and add to the current temp name in state.
 */
void putop (DIS_STATE *state, __in LPCSTR templ)
{
  LPCSTR p;
  char *obufp = state->temp_name_buf + strlen(state->temp_name_buf);

  for (p = templ; *p; p++) {
    switch (*p) {
    default:
      *obufp++ = *p;
      break;
    case 'C':   /* For jcxz/jecxz */
      if (state->addr_flag == 0)
        *obufp++ = 'e';
      break;
    case 'N':
      if ((state->prefix_flags & PREFIX_FWAIT) == 0)
        *obufp++ = 'n';
      break;
    case 'S':
      /* operand size flag */
      if (state->data_flag)
        *obufp++ = 'l';
      else
        *obufp++ = 'w';
      break;
    }
  }
  *obufp = 0;
}

/*  dofloat - parse a floating point operation and and its operands.  */
void dofloat (DIS_STATE *state)
{
  DISx86_OP *dp;
  unsigned char floatop;
  int len, i;

  floatop = *(state->insn_bufp-1);

  if (state->mod != 3)
    {
      putop(state, float_mem[(floatop - 0xd8) * 8 + state->reg]);
      len = (int) strlen(state->temp_name_buf);      /*  pad out spacing  */
      for (i = len ; i < 6 ; i++)
        strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), " ");
      strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), " ");

      state->which_op = 0;
      OP_E (state,v_mode);
      return;
    }
  advance_byte(state);

  dp = &float_reg[floatop - 0xd8][state->reg];
  if (dp->name == NULL) {
    putop (state, fgrps[dp->bytemode1][state->rm]);

    len = (int) strlen(state->temp_name_buf);      /*  pad out spacing  */
    for (i = len ; i < 6 ; i++)
      strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), " ");
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), " ");

    /* instruction fnstsw is only one with strange arg */
    if (floatop == 0xdf && (*state->insn_bufp == 0xe0))
      strcpy_s(state->op_out[0], sizeof(state->op_out[0]), "eax");
  }
  else {
    putop(state,dp->name);             /* add instruction's name  */
    len = (int) strlen(state->temp_name_buf);      /*  pad out spacing  */
    for (i = len ; i < 6 ; i++)
      strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), " ");
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), " ");

    if (dp->op1) {
      state->which_op = 0;
      (*dp->op1)(state,dp->bytemode1);
    }
    if (dp->op2) {
      state->which_op = 1;
      (*dp->op2)(state,dp->bytemode2);
    }
  }
}

/*  do_opcode - we have disassembled the prefix; now do the rest of the
 * instruction.
 */
void do_opcode(DIS_STATE *state)
{
  DISx86_OP *dp;
  int i, len, needcomma;

  /*  decode the op code byte  */
  if (*state->insn_bufp == 0x0f) {
    advance_byte(state);
    dp = &disx86_twobyte[*state->insn_bufp];
  }
  else
    dp = &disx86[*(state->insn_bufp)];
  advance_byte(state);

  /*  check address mode modifiers  */
  state->mod = (*(state->insn_bufp) >> 6) & 3;
  state->reg = (*(state->insn_bufp) >> 3) & 7;
  state->rm  = *(state->insn_bufp) & 7;

  if ((dp->name == NULL) && (dp->bytemode1 == FLOATCODE))
    dofloat(state);
  else {                               /* get name and parse operands  */
    if (dp->name == NULL)
      dp = &grps[dp->bytemode1][state->reg];

    putop(state,dp->name);             /* add instruction's name  */
    len = (int) strlen(state->temp_name_buf);      /*  pad out spacing  */
    for (i = len ; i < 6 ; i++)
      strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), " ");
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), " ");

    state->which_op = 0;
    if (dp->op1)
      (*dp->op1)(state,dp->bytemode1);

    state->which_op = 1;
    if (dp->op2)
      (*dp->op2)(state,dp->bytemode2);

    state->which_op = 2;
    if (dp->op3)
      (*dp->op3)(state,dp->bytemode3);
  }

  needcomma = 0;                           /*  get string for each operand  */
  for (i = 0 ; i < 3 ; i++) {
    if (*(state->op_out[i])) {
      if (needcomma)
        strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), ",");
      if (state->op_index[i] != -1)
        do_address(state,state->op_address[state->op_index[i]]);
      else
        strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), state->op_out[i]);
      needcomma = 1;
    }
  }
}

/*  append_prefix - use prefix flag to append prefix name to insn  */
void append_prefix (DIS_STATE *state)
{
  int prefixes = state->prefix_flags;

  if (prefixes & PREFIX_CS)
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "cs:");
  if (prefixes & PREFIX_DS)
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "ds:");
  if (prefixes & PREFIX_SS)
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "ss:");
  if (prefixes & PREFIX_ES)
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "es:");
  if (prefixes & PREFIX_FS)
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "fs:");
  if (prefixes & PREFIX_GS)
    strcat_s(state->temp_name_buf, sizeof(state->temp_name_buf), "gs:");
}

/* ******************************************************************
 *                    Opcode Format Helper Functions
 * ******************************************************************
 */

int OP_ST (DIS_STATE *state, int ignore)
{
  ASSERT(state->which_op != -1);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]), "st");
  return (0);
}

int OP_STi (DIS_STATE *state, int ignore)
{
  char scratchbuf[MAX_NAME_SIZE];

  ASSERT(state->which_op != -1);
  sprintf_s(scratchbuf, sizeof(scratchbuf), "st(%d)", state->rm);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]), scratchbuf);
  return (0);
}

int OP_indirE (DIS_STATE *state, int bytemode)
{
  ASSERT(state->which_op != -1);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]), "*");
  OP_E (state,bytemode);
  return (0);
}

int OP_E (DIS_STATE *state, int bytemode)
{
  char scratchbuf[MAX_NAME_SIZE];
  int disp;
  int havesib;
  int base = 0; // Initialize to suppress C4701 false positive
  int index = 0; // Initialize to suppress C4701 false positive
  int scale = 0; // Initialize to suppress C4701 false positive
  int havebase;
  unsigned char *operand_start;

  ASSERT(state->which_op != -1);

  operand_start=state->insn_bufp;
  /* skip mod/rm byte */
  advance_byte(state);

  havesib = 0;
  havebase = 0;
  disp = 0;

  if (state->mod == 3)
    {
      switch (bytemode)
      {
      case b_mode:
        strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]), names8[state->rm]);
        break;
      case w_mode:
        strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]), names16[state->rm]);
        break;
      case v_mode:
        if (state->data_flag)
          strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names32[state->rm]);
        else
          strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names16[state->rm]);
        break;
      default:
        CRASH("Bad disassembly table");
      }
      state->rm_op_len=(char)(state->insn_bufp-operand_start);
      return (0);
    }

  append_prefix(state);

  if (state->rm == 4)
    {
      havesib = 1;
      havebase = 1;
      scale = (*state->insn_bufp >> 6) & 3;
      index = (*state->insn_bufp >> 3) & 7;
      base = *state->insn_bufp & 7;
      advance_byte(state);
    }

  switch (state->mod)
    {
    case 0:
      switch (state->rm)
      {
      case 4:
        /* implies havesib and havebase */
        if (base == 5)
          {
            havebase = 0;
            disp = get32(state);
          }
        break;
      case 5:
        disp = get32(state);
        break;
      default:
        havebase = 1;
        base = state->rm;
        break;
      }
      break;
    case 1:
      disp = (int) *state->insn_bufp;
      advance_byte(state);
      if (state->rm != 4)
      {
        havebase = 1;
        base = state->rm;
      }
      break;
    case 2:
      disp = get32(state);
      if (state->rm != 4)
      {
        havebase = 1;
        base = state->rm;
      }
      break;
    }

  if (state->mod != 0 || state->rm == 5 ||
      (havesib && base == 5))
    {
      sprintf_s(scratchbuf, sizeof(scratchbuf), "0x%x", disp);
      strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
    }

  if (havebase || havesib)
    {
      strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),"(");
      if (havebase)
        strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names32[base]);
      if (havesib)
        {
          if (index != 4)
            {
              sprintf_s(scratchbuf, sizeof(scratchbuf), ",%s", names32[index]);
              strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
            }
          sprintf_s(scratchbuf, sizeof(scratchbuf), ",%d", 1 << scale);
          strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
        }
      strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),")");
    }
  state->rm_op_len=(char)(state->insn_bufp-operand_start);
  return (0);
}

int OP_G (DIS_STATE *state, int bytemode)
{
  ASSERT(state->which_op != -1);

  switch (bytemode)
    {
    case b_mode:
      strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names8[state->reg]);
      break;
    case w_mode:
      strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names16[state->reg]);
      break;
    case d_mode:
      strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names32[state->reg]);
      break;
    case v_mode:
      if (state->data_flag)
        strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names32[state->reg]);
      else
        strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names16[state->reg]);
      break;
    default:
      CRASH("I shouldn't be here");
      break;
    }
  return (0);
}

/*  get32 - get next four bytes as 32-bit address  */
int get32(DIS_STATE *state)
{
  int x = 0;

  x = *state->insn_bufp & 0xff;
  advance_byte(state);
  x |= (*state->insn_bufp & 0xff) << 8;
  advance_byte(state);
  x |= (*state->insn_bufp & 0xff) << 16;
  advance_byte(state);
  x |= (*state->insn_bufp & 0xff) << 24;
  advance_byte(state);
  return (x);
}

/*  get16 - get next two bytes as 16-bit address  */
int get16(DIS_STATE *state)
{
  int x = 0;

  x = *state->insn_bufp & 0xff;
  advance_byte(state);
  x |= (*state->insn_bufp & 0xff) << 8;
  advance_byte(state);
  return (x);
}

void set_op (DIS_STATE *state, int op)
{
  ASSERT(state->which_op != -1);
  state->op_index[state->which_op] = state->which_op;
  state->op_address[state->which_op] = op;
}

int OP_REG (DIS_STATE *state, int code)
{
  char *s = NULL;

  ASSERT(state->which_op != -1);
  switch (code) {
  case indir_dx_reg:
    s = "(%dx)";
    break;
  case ax_reg:
  case cx_reg:
  case dx_reg:
  case bx_reg:
  case sp_reg:
  case bp_reg:
  case si_reg:
  case di_reg:
    s = names16[code - ax_reg];
    break;
  case es_reg:
  case ss_reg:
  case cs_reg:
  case ds_reg:
  case fs_reg:
  case gs_reg:
    s = names_seg[code - es_reg];
    break;
  case al_reg:
  case ah_reg:
  case cl_reg:
  case ch_reg:
  case dl_reg:
  case dh_reg:
  case bl_reg:
  case bh_reg:
    s = names8[code - al_reg];
    break;
  case eAX_reg:
  case eCX_reg:
  case eDX_reg:
  case eBX_reg:
  case eSP_reg:
  case eBP_reg:
  case eSI_reg:
  case eDI_reg:
    if (state->data_flag)
      s = names32[code - eAX_reg];
    else
      s = names16[code - eAX_reg];
    break;
  default:
    CRASH("I shouldn't be here");
    s = "";		// must set it to a value to avoid C4703 error
    break;
  }
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),s);
  return (0);
}

int OP_I (DIS_STATE *state, int bytemode)
{
  char scratchbuf[MAX_NAME_SIZE];
  int op = 0; // Initialize to suppress C4701 false positive

  ASSERT(state->which_op != -1);
  switch (bytemode) {
  case b_mode:
    op = *state->insn_bufp & 0xff;
    advance_byte(state);
    break;
  case v_mode:
    if (state->data_flag)
      op = get32(state);
    else
      op = get16(state);
    break;
  case w_mode:
    op = get16(state);
    break;
  default:
    CRASH("I shouldn't be here");
  }
  sprintf_s(scratchbuf, sizeof(scratchbuf), "0x%x", op);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
  return (0);
}

int OP_sI (DIS_STATE *state, int bytemode)
{
  char scratchbuf[MAX_NAME_SIZE];
  int op;

  ASSERT(state->which_op != -1);
  switch (bytemode) {
  case b_mode:
    op = *state->insn_bufp;
    advance_byte(state);
    break;
  case v_mode:
    if (state->data_flag)
      op = get32(state);
    else
      op = (short) get16(state);
    break;
  case w_mode:
    op = (short) get16(state);
    break;
  default:
    CRASH("I shouldn't be here");
    return (0);
  }
  sprintf_s (scratchbuf, sizeof(scratchbuf), "0x%x", op);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
  return (0);
}

int OP_J (DIS_STATE *state, int bytemode)
{
  char scratchbuf[MAX_NAME_SIZE];
  int disp;
  int mask = -1;

  ASSERT(state->which_op != -1);
  switch (bytemode) {
  case b_mode:
    disp = (char) *state->insn_bufp;
    advance_byte(state);
    break;
  case v_mode:
    if (state->data_flag)
      disp = get32(state);
    else {
      disp = (short) get16(state);
      /* for some reason, a data16 prefix on a jump instruction
         means that the pc is masked to 16 bits after the
         displacement is added!  */
      mask = 0xffff;
    }
    break;
  default:
    CRASH("I shouldn't be here");
  }
#ifdef PATCH_ORIG
  disp = (state->insn_bufp - state->insn_buf + disp + 0x1000) & mask;
#endif
  disp = (state->insn_bufp - state->insn_buf + disp) & mask;
  set_op(state,disp+state->pc);
  sprintf_s(scratchbuf, sizeof(scratchbuf), "0x%x", disp+state->pc);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
  return (0);
}

static const char *sreg[] = {
    "es","cs","ss","ds","fs","gs","?","?",
};

int OP_SEG (DIS_STATE *state, int dummy)
{
  ASSERT(state->which_op != -1);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),sreg[state->reg]);
  return (0);
}

int OP_DIR (DIS_STATE *state, int size)
{
  char scratchbuf[MAX_NAME_SIZE];
  int seg, offset;

  ASSERT(state->which_op != -1);
  switch (size) {
  case lptr:
    if (state->addr_flag) {
      offset = get32(state);
      seg = get16(state);
    }
    else {
      offset = get16(state);
      seg = get16(state);
    }
    sprintf_s(scratchbuf, sizeof(scratchbuf), "0x%x,0x%x", seg, offset);
    strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
    break;
  case v_mode:
    if (state->addr_flag)
      offset = get32(state);
    else
      offset = (short) get16(state);
    offset = (int) (state->insn_bufp - state->insn_buf + offset + state->pc);
    set_op(state,offset);
    sprintf_s(scratchbuf, sizeof(scratchbuf), "0x%x", offset);
    strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
    break;
  default:
    CRASH("I shouldn't be here");
  }
  return (0);
}

int OP_OFF (DIS_STATE *state, int bytemode)
{
  char scratchbuf[MAX_NAME_SIZE];
  int off;

  ASSERT(state->which_op != -1);
  if (state->addr_flag)
    off = get32(state);
  else
    off = get16(state);

  sprintf_s(scratchbuf, sizeof(scratchbuf), "0x%x", off);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
  return (0);
}

int OP_ESDI (DIS_STATE *state, int dummy)
{
  ASSERT(state->which_op != -1);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),"es:(");
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]), (state->addr_flag ? "edi" : "di"));
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),")");
  return (0);
}

int OP_DSSI (DIS_STATE *state, int dummy)
{
  ASSERT(state->which_op != -1);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),"ds:(");
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]), (state->addr_flag ? "esi" : "si"));
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),")");
  return (0);
}

int OP_ONE (DIS_STATE *state, int dummy)
{
  ASSERT(state->which_op != -1);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),"1");
  return (0);
}

int OP_C (DIS_STATE *state, int dummy)
{
  char scratchbuf[MAX_NAME_SIZE];

  ASSERT(state->which_op != -1);
  advance_byte(state);                        /* skip mod/rm */
  sprintf_s(scratchbuf, sizeof(scratchbuf), "cr%d", state->reg);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
  return (0);
}

int OP_D (DIS_STATE *state, int dummy)
{
  char scratchbuf[MAX_NAME_SIZE];

  ASSERT(state->which_op != -1);
  advance_byte(state);                        /* skip mod/rm */
  sprintf_s(scratchbuf, sizeof(scratchbuf), "db%d", state->reg);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),scratchbuf);
  return (0);
}

int OP_T (DIS_STATE *state, int dummy)
{
  char scratchbuf[MAX_NAME_SIZE];

  ASSERT(state->which_op != -1);
  advance_byte(state);                        /* skip mod/rm */
  sprintf_s(scratchbuf, sizeof(scratchbuf), "tr%d", state->reg);
  strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]), scratchbuf);
  return (0);
}

int OP_rm (DIS_STATE *state, int bytemode)
{
  ASSERT(state->which_op != -1);
  switch (bytemode) {
  case d_mode:
    strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names32[state->rm]);
    break;
  case w_mode:
    strcat_s(state->op_out[state->which_op], sizeof(state->op_out[state->which_op]),names16[state->rm]);
    break;
  }
  return (0);
}

DIS_STATE the_dis_state;
DIS_STATE *dis_state = &the_dis_state;

void
disasm_print_insn_help(unsigned char* buf_ptr,unsigned int pc,int terse)
{
  unsigned char *bufp;
  int bytes;

  dis_state->pc=pc;
  dis_state->insn_buf=buf_ptr;
  dis_state->insn_bufp=dis_state->insn_buf;

  dis_state->temp_name_buf[0]
    = *dis_state->op_out[0]
    = *dis_state->op_out[1]
    = *dis_state->op_out[2]
    = '\0';
  dis_state->op_index[0]
    = dis_state->op_index[1]
    = dis_state->op_index[2]
    = -1;
  dis_state->which_op = -1;

  if (!terse)
    printf("%08x  ",pc);

  if (!do_prefix(dis_state))
    do_opcode(dis_state);

  bytes = 0;
  for (bufp=dis_state->insn_buf;
       bufp<dis_state->insn_bufp;
       bufp++)
  {
    printf(" %02x",*bufp);

    ++bytes;
    if (!terse)
    {
      if (bytes == 5 && bufp + 1 < dis_state->insn_bufp)
      {
        printf("\n          ");
        bytes = 0;
      }
    }
  }
  for (; bytes < 5; bytes++)
  {
    printf("   ");
  }

  printf("  %s",dis_state->temp_name_buf);

  if (!terse)
    putchar('\n');
}

void
disasm_print_insn(unsigned char* buf_ptr,unsigned int pc)
{
  disasm_print_insn_help(buf_ptr,pc,1);
}

int
print_insn_from_buf(unsigned char *buf_ptr,unsigned int pc)
{
  disasm_print_insn_help(buf_ptr,pc,0);
  return((int)(dis_state->insn_bufp-dis_state->insn_buf));
}

/***********************************************************************/

void print_raw_as_insns(unsigned char *buf,int len,unsigned int start_addr)
{
  unsigned char *buf_ptr;
  int insn_len;
  unsigned int pc=start_addr;
  int grief_count=0;

  buf_ptr=buf;
  while (((buf_ptr-buf)<len))
    {
      if (grief_count>30)
        break;
      if (*buf_ptr)
        {
          insn_len=print_insn_from_buf(buf_ptr,pc);
          buf_ptr+=insn_len;
          pc+=insn_len;
          grief_count=0;
        }
      else
        {
          printf("%08x\n",pc);
          grief_count++;
          buf_ptr++;
          pc++;
        }
    }
  fflush(stdout);
}


// Provide the below copies so the output is sent to Output:;print

void
disasm_print_insn_help2(unsigned char* buf_ptr,unsigned int pc,int terse)
{
  unsigned char *bufp;
  int bytes;

  dis_state->pc=pc;
  dis_state->insn_buf=buf_ptr;
  dis_state->insn_bufp=dis_state->insn_buf;

  dis_state->temp_name_buf[0]
    = *dis_state->op_out[0]
    = *dis_state->op_out[1]
    = *dis_state->op_out[2]
    = '\0';
  dis_state->op_index[0]
    = dis_state->op_index[1]
    = dis_state->op_index[2]
    = -1;
  dis_state->which_op = -1;

  if (!terse)
    Output::Print(L"%08x  ",pc);

  if (!do_prefix(dis_state))
    do_opcode(dis_state);

  bytes = 0;
  for (bufp=dis_state->insn_buf;
       bufp<dis_state->insn_bufp;
       bufp++)
  {
    Output::Print(L" %02x",*bufp);

    ++bytes;
    if (!terse)
    {
      if (bytes == 5 && bufp + 1 < dis_state->insn_bufp)
      {
        Output::Print(L"\n          ");
        bytes = 0;
      }
    }
  }
  for (; bytes < 5; bytes++)
  {
    Output::Print(L"   ");
  }

  Output::Print(L"  %S",dis_state->temp_name_buf);

  if (!terse)
    Output::Print(L"\n");
}


int
print_insn_from_buf2(unsigned char *buf_ptr,unsigned int pc)
{
  disasm_print_insn_help2(buf_ptr,pc,0);
  return((int)(dis_state->insn_bufp-dis_state->insn_buf));
}
#endif
#endif