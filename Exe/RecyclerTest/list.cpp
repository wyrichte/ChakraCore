/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include <cassert>

void tracked::list::initialize(int ndata)
{
    assert(root == 0);
    assert(last == 0);

    root = (node*)objtracker->track_malloc(sizeof node);
    root->next = 0;
    objtracker->allocator->update_pointer((void**)&root->data, objtracker->track_malloc(sizeof(int)*ndata));

    // quickest for now...
    char *tmpbuf = (char*)_alloca(sizeof(int)*ndata);
    memset(tmpbuf, 0, sizeof(int)*ndata);
    objtracker->allocator->update_buffer((void**)root->data, tmpbuf, sizeof(int)*ndata);

    last = root;
}

tracked::list::node *tracked::list::append(int ndata)
{
    if(frozen)
        while(1);

    if(root == 0)
        initialize(ndata);
    else
    {
        objtracker->allocator->update_pointer((void**)&last->next, (node*)objtracker->track_malloc(sizeof node));
        objtracker->allocator->update_pointer((void**)&last->next->next, NULL);
        objtracker->allocator->update_pointer((void**)&last->next->data, (int*)objtracker->track_malloc(sizeof(int)*ndata));

        char *tmpbuf = (char*)_alloca(sizeof(int)*ndata);
        memset(tmpbuf, 0, sizeof(int)*ndata);
        objtracker->allocator->update_buffer((void**)last->next->data, tmpbuf, sizeof(int)*ndata);

        last = last->next;
    }
    return last;
}

void tracked::list::prune(node *n)
{
    if(frozen)
        while(1);

    //dump();
    if(n != root)
    {
        node *p = root;
        while(p->next != n)
            p = p->next;

        assert(p->next == n);
        objtracker->allocator->update_pointer((void**)&p->next, 0);

        last = p;
    }
    else
    {
        root = last = 0;
    }

    while(n)
    {
        node *tmp = n->next;
        objtracker->track_free(n->data);
        objtracker->track_free(n);
        n = tmp;
    }
}

void tracked::list::dump()
{
    frozen=true;
    printf("=== list dump ===\n");
    node *p = root;
    while(p)
    {
        printf("%p: data (%p)\n", p, p->data);
        p = p->next;
    }
    frozen=false;
}

void tracked::list::reverse()
{
   if(frozen)
        while(1);

   node *prev = NULL;
   node *p = root;
   last = root;
   while (p != NULL) 
   { 
      node *tmp = p->next;
      objtracker->allocator->update_pointer((void**)&p->next, prev);
      prev = p; 
      p = tmp; 
   } 
   root = prev;
}

tracked::list::~list()
{
    prune(root);
}