//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

#define TL1(T1) TL<T1, NullType>
#define TL2(T1, T2) TL<T1, TL1(T2)>
#define TL3(T1, T2, T3) TL<T1, TL2(T2, T3)>
#define TL4(T1, T2, T3, T4) TL<T1, TL3(T2, T3, T4)>
#define TL5(T1, T2, T3, T4, T5) TL<T1, TL4(T2, T3, T4, T5)>
#define TL6(T1, T2, T3, T4, T5, T6) TL<T1, TL5(T2, T3, T4, T5, T6)>

namespace Authoring
{
    template <class H, class T>
    struct TL
    {
        typedef H Head;
        typedef T Tail;
    };

    template <class TypeList>
    struct VisitorComposer
    {
        template <class TypeList>
        class ComposedVisitor : public TypeList::Head, public ComposedVisitor<typename TypeList::Tail>
        {
            typedef typename TypeList::Head Head;
            typedef typename Head::Context HeadContext;
            typedef ComposedVisitor<typename TypeList::Tail> Tail;
            typedef typename Tail::Context TailContext;
        public:
            typedef struct
            {
                HeadContext h;
                TailContext t;
            } Context;

            inline public bool ComposedPreorder(ParseNode *pnode, Context context)
            {
                bool h = Head::Preorder(pnode, context.h);
                bool t = Tail::ComposedPreorder(pnode, context.t);
                AssertMsg(h == t, "Unsupported composition of visitors. All visitors must all agree on which nodes to traverse");
                return t;
            }

            inline public void ComposedInorder(ParseNode *pnode, Context context)
            {
                Head::Inorder(pnode, context.h);
                Tail::ComposedInorder(pnode, context.t);
            }

            inline public void ComposedMidorder(ParseNode *pnode, Context context)
            {
                Head::Midorder(pnode, context.h);
                Tail::ComposedMidorder(pnode, context.t);
            }

            inline public void ComposedPostorder(ParseNode *pnode, Context context)
            {
                Head::Postorder(pnode, context.h);
                Tail::ComposedPostorder(pnode, context.t);
            }

            inline public void ComposedInList(ParseNode *pnode, Context context)
            {
                Head::InList(pnode, context.h);
                Tail::ComposedInList(pnode, context.t);
            }

            inline public void ComposedPassReference(ParseNode **ppnode, Context context)
            {
                Head::PassReference(ppnode, context.h);
                Tail::ComposedPassReference(ppnode, context.t);
            }
        };

        template<>
        class ComposedVisitor<NullType> 
        {
        public:
            typedef struct { } Context;

            inline public bool ComposedPreorder(ParseNode *pnode, Context context) { return true; }
            inline public void ComposedInorder(ParseNode *pnode, Context context) { }
            inline public void ComposedMidorder(ParseNode *pnode, Context context) { }
            inline public void ComposedPostorder(ParseNode *pnode, Context context) { }
            inline public void ComposedInList(ParseNode *pnode, Context context) { }
            inline public void ComposedPassReference(ParseNode **ppnode, Context context) { }
        };

        typedef typename ComposedVisitor<TypeList>::Context Context;

        ComposedVisitor<TypeList> composed;

        inline public bool Preorder(ParseNode *pnode, Context context)
        {
            return composed.ComposedPreorder(pnode, context);
        }

        inline void PassReference(ParseNode **ppnode, Context context)
        { 
            composed.ComposedPassReference(ppnode, context);
        }

        inline public void Inorder(ParseNode *pnode, Context context)
        {
            composed.ComposedInorder(pnode, context);
        }

        inline public void Midorder(ParseNode *pnode, Context context)
        {
            composed.ComposedMidorder(pnode, context);
        }

        inline public void Postorder(ParseNode *pnode, Context context)
        {
            composed.ComposedPostorder(pnode, context);
        }

        inline public void InList(ParseNode *pnode, Context context) 
        {
            composed.ComposedInList(pnode, context);
        }
    };
}