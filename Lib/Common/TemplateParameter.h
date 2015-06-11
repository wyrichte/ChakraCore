//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

// -----------------------------------------------------------------------------------------------------------------------------
// Template parameter constraints
// See http://www2.research.att.com/~bs/bs_faq2.html
// -----------------------------------------------------------------------------------------------------------------------------

namespace TemplateParameter
{
    template<class T, class Base>
    class SameOrDerivedFrom
    {
    private:
        static void Constrain(T *const t)
        {
#pragma warning(suppress: 4189) // C4189: local variable is initialized but not referenced
            Base *const b = t;
        }

    public:
        SameOrDerivedFrom()
        {
#pragma warning(suppress: 4189) // C4189: local variable is initialized but not referenced
            void (*const p)(T *const t) = Constrain;
        }
    };

    template<class T>
    struct Box
    {
    };
};
