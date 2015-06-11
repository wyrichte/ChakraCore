/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
class BasicBlock;
class RegisterDescription;

struct Instruction {
  Js::OpCode op;        
  Js::RegSlot rd,rs1,rs2;
  int ordinal;
  List<Instruction*> *rs1Defs;
  List<Instruction*> *rs2Defs;
  int mrd,mrs1,mrs2;
  BasicBlock *basicBlock;
  bool isBranchInstruction;  
};

struct LdStrInstruction : Instruction {
    Js::Atom str;
};

struct RegexInstruction : Instruction {
    regex::Regex *regularExpression;
};

struct ArgInstruction : Instruction {
    Js::RegSlot argIndex; 
};

struct StElemInstruction: Instruction {
    Js::RegSlot val;
    int mval;
};

struct BranchInstruction : Instruction{
    BasicBlock *branchTarget;
};

struct BranchImmInstruction : BranchInstruction{
    int immInt;
};

struct DoubleConstInstruction : Instruction {
    double immDouble;
};

struct IntConstInstruction : Instruction {
    int immInt;
};

struct CallInstruction : Instruction {
    int nArgs; // 0 to 2; use rs1 then rs1; store result in rd
};

struct CallIMInstruction : CallInstruction {
    int memberId;
};

struct IntrinsicCallInstruction : CallInstruction {
    void *fn;  // function to call; will cast this to the appropriate type
};

struct CallDirectInstruction : CallInstruction {
    Js::FunctionBody* fnBod;
};

struct Int64ConstInstruction : Instruction {
    _int64 immInt64;
};

struct OpCount {
    unsigned int op;
    int count;
};
