/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

namespace tracked
{
    struct list
    {
        struct node
        {
            node *next;
            int *data;
        };


        tracker *objtracker;
        node *root;
        node *last;
        BOOL frozen;


        list(tracker *objtracker) : root(0), last(0), objtracker(objtracker), frozen(false) { }
        ~list();

        void SetFrozen(BOOL b) { frozen=b; }
        void initialize(int ndata);
        node *append(int ndata);
        void prune(node *n);
        void dump();
        void reverse();
    };

}
