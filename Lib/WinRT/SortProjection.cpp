//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "SortProjection.h"
#include "Visitor.h"

namespace ProjectionModel
{
    using namespace regex;

    struct CompareRefAssignments : public regex::Comparer<RtASSIGNMENT *>
    {
        static CompareRefAssignments Instance;

        bool Equals(RtASSIGNMENT *v1, RtASSIGNMENT *v2)
        {
            return Compare(v1,v2) == 0;
        }
        int GetHashCode(RtASSIGNMENT *s)
        {
            Assert(0);
            return 0;
        }
        int Compare(RtASSIGNMENT *v1, RtASSIGNMENT *v2)
        {
            Assert(v1 != nullptr && v2 != nullptr);
            return wcscmp((*v1)->identifier,(*v2)->identifier);
        }
    };

    CompareRefAssignments CompareRefAssignments::Instance;

    struct Sorter : Visitor 
    {
        Metadata::IStringConverter * stringConverter;
        Sorter(ArenaAllocator * a, Metadata::IStringConverter * stringConverter) : Visitor(a), stringConverter(stringConverter) { }
        virtual RtEXPR VisitAssignmentSpace(RtASSIGNMENTSPACE expr) 
        {
            auto varspace = AssignmentSpace::From(Visitor::VisitAssignmentSpace(expr));
            auto vars = varspace->vars->SortCurrentList(&CompareRefAssignments::Instance);
            return Anew(a,AssignmentSpace,vars);
        }

        virtual RtEXPR VisitPropertiesObject(RtPROPERTIESOBJECT expr) 
        {
            auto propertiesObject = PropertiesObject::From(Visitor::VisitPropertiesObject(expr));
            CompareRefProperties comparer(stringConverter);
            auto fields = propertiesObject->fields->SortCurrentList(&comparer);
            return Anew(a,PropertiesObject,fields);
        }
    };

    // Info:        Returns the given expression sorted by identifier names
    // Parameters:  expr - the expr to sort
    RtEXPR SortExpr(RtEXPR expr, ArenaAllocator * a, Metadata::IStringConverter * stringConverter)
    {
        Sorter sorter(a, stringConverter);
        return sorter.VisitExpr(expr);
    }

    // Info:        Returns the given assignment space sorted by identifier names
    // Parameters:  varspace - the assignment space to sort
    RtASSIGNMENTSPACE SortAssignmentSpace(RtASSIGNMENTSPACE varspace, ArenaAllocator * a, Metadata::IStringConverter * stringConverter)
    {
        return AssignmentSpace::From(SortExpr(varspace,a, stringConverter));
    }


}