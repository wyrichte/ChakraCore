#pragma once
using namespace JsUtil;

/*
*	CaseNode - represents the case statements (not the case block) in the switch statement
*/
class CaseNode
{
private:    
    uint32              offset;          //offset - indicates the bytecode offset of the case instruction
    uint32              targetOffset;    //targetOffset - indicates the bytecode offset of the target instruction (case block)
    IR::BranchInstr*    caseInstr; // caseInstr - stores the case instruction
    IR::Opnd*           lowerBound;	//lower bound - used for integer cases

public:
    CaseNode(IR::BranchInstr* caseInstr, uint32 offset, uint32 targetOffset, IR::Opnd* lowerBound = null)
        : caseInstr(caseInstr),
        offset(offset),
        targetOffset(targetOffset),
        lowerBound(lowerBound)
    {
    }

    IntConstType GetSrc2IntConst()
    {
        AssertMsg(caseInstr->GetSrc2()->GetStackSym()->IsIntConst(),"Source2 operand is not an integer constant");
        return caseInstr->GetSrc2()->GetStackSym()->GetIntConstValue();
    }

    Js::JavascriptString* GetSrc2StringConst()
    {
        AssertMsg(caseInstr->GetSrc2()->GetStackSym()->m_isStrConst,"Source2 operand is not an integer constant");
        return Js::JavascriptString::FromVar(caseInstr->GetSrc2()->GetStackSym()->GetConstAddress());
    }

    bool IsSrc2IntConst()
    {
        return caseInstr->GetSrc2()->GetStackSym()->IsIntConst();
    }

    bool IsSrc2StrConst()
    {
        return caseInstr->GetSrc2()->GetStackSym()->m_isStrConst;
    }
    uint32 GetOffset()
    {
        return offset;
    }

    uint32 GetTargetOffset()
    {
        return targetOffset;
    }

    IR::Opnd* GetUpperBound()
    {
        return caseInstr->GetSrc2();
    }

    IR::Opnd* GetLowerBound()
    {
        return lowerBound;
    }
    
    void SetLowerBound(IR::Opnd* lowerBound)
    {
        this->lowerBound = lowerBound;
    }

    IR::BranchInstr* GetCaseInstr()
    {
        return caseInstr;
    }
};

template <>
struct DefaultComparer<CaseNode *> 
{
public:    
    static int Compare(CaseNode* caseNode1, CaseNode* caseNode2);
    static bool Equals(CaseNode* x, CaseNode* y);    
    static uint GetHashCode(CaseNode * caseNode);
};

