//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once


///----------------------------------------------------------------------------
///----------------------------------------------------------------------------
///
/// class TreeNode
///
/// General DataStructure for an N-ary tree.
///
///----------------------------------------------------------------------------
///----------------------------------------------------------------------------

template<class T, int N>
class TreeNode
{

// Data
private:
    T                   value;
    TreeNode *          children[N];                                                            
    TreeNode<T, N> *    parent;

// Contructor
public:
    TreeNode(TreeNode<T, N> * parent = NULL)
    {
        this->parent    = parent;
        for(int i = 0; i < N; i++)
        {
            this->children[i] = NULL;
        }
    }

// Methods
public:
    bool ChildExistsAt(int i)
    {
        return NULL != this->children[i];
    }

    TreeNode<T, N> * GetChildAt(int i)
    {
        return this->children[i];
    }

    void SetChildAt(int i, TreeNode<T, N> *node)
    {
        this->children[i]   = node;
    }

    TreeNode<T, N> * GetParent()
    {
         return this->parent;
    }

    void SetParent(TreeNode<T, N>* parent)
    {
        this->parent = parent;
    }

    T * GetValue()
    {
        return &this->value;
    }

    void SetValue(const T value)
    {
        this->value = value;
    }

};

template <typename T>
struct TreeNodeComparer
{
    __inline static int Compare(T x, T y)
    {
        return x - y;
    }

};

template <typename T>
class BinarySearchTree
{
public:
    typedef TreeNode<T, 2> BSTNode;

    enum NodeType
    {
        LeftNode,
        RightNode
    };

    BinarySearchTree(ArenaAllocator* arena):
        root(NULL)
#ifndef HEAP_ALLOC
        , alloc(arena)
#endif
    {
    }

#ifdef HEAP_ALLOC
    void Free(BSTNode* node)
    {
        BSTNode* left = node->GetChildAt(LeftNode);
        BSTNode* right = node->GetChildAt(RightNode);

        if (left)
        {
            Free(left);
        }

        if (right)
        {
            Free(right);
        }

        delete node;
    }

    ~BinarySearchTree()
    {
        Free(root);
    }
#endif


    bool Insert(T data)
    {
        if (root == null)
        {
            root = NewNode(data);
            root->SetValue(data);
            root->SetParent(null);
            return true;
        }

        BSTNode* current = root;
        while (current != null) 
        {
            T* key = current->GetValue();

            TreeNodeComparer<T> tComparer;

            int value = tComparer.Compare(data, *key);

            if (value == 0) 
            {
                return false;
            } 
            else if (value < 0) 
            {
                if (!current->ChildExistsAt(LeftNode)) 
                {
                    BSTNode* newNode = NewNode(data, current);
                    current->SetChildAt(LeftNode, newNode);
                    newNode->SetParent(current);
                    return true;
                } 
                else 
                {
                    current = current->GetChildAt(LeftNode);
                }
            }
            else 
            {
                if (!current->ChildExistsAt(RightNode))
                {
                    BSTNode* newNode = NewNode(data, current);
                    current->SetChildAt(RightNode, newNode);
                    newNode->SetParent(current);
                    return true;
                } 
                else
                {
                    current = current->GetChildAt(RightNode);
                }
            }
        }

        return true;
    }

    bool Contains(const T& data)
    {
        return (Find(data) != null ? true : false);
    }

    bool Delete(const T& data)
    {
        BSTNode* node = Find(data);
#ifdef HEAP_ALLOC
        bool removed = true;
#endif

        if (node != null)
        {
            BSTNode* left = node->GetChildAt(LeftNode);
            BSTNode* right = node->GetChildAt(RightNode);

            if (left && right)
            {
#ifdef HEAP_ALLOC
                removed = false; // We don't remove in this case, we just replace
#endif

                BSTNode* successor = FindMin(right);
                node->SetValue(*(successor->GetValue()));
                ReplaceNodeInParent(successor, successor->GetChildAt(RightNode));
            }
            else if (left)
            {
                ReplaceNodeInParent(node, left);
            }
            else if (right)
            {
                ReplaceNodeInParent(node, right);
            }
            else
            {
                ReplaceNodeInParent(node, null);
            }

#ifdef HEAP_ALLOC
            // The root node isn't deleted, its value is just updated
            if (removed)
            {
                delete node;
            }
#endif

            return true;
        }

        return false;
    }


    BSTNode* Find(const T& data)
    {
        BSTNode* current = root;

        while (current)
        {
            T* key = current->GetValue();

            TreeNodeComparer<T> tComparer;

            int value = tComparer.Compare(data, (*key));

            if (value == 0)
            {
                return current; 
            }
            else if (value < 0)
            {
                current = current->GetChildAt(LeftNode);
            }
            else
            {
                current = current->GetChildAt(RightNode);
            }
        }

        return null;
    }

#if DBG_DUMP
    void Dump()
    {
        printf("\nTree:\n");
        Dump(root, 0);
    }

    void Dump(BSTNode* node, int tabs)
    {
        if (node == null)
        {
            return;
        }

        for (int i = 0; i < tabs; i++)
        {
            printf(" ");
        }

        printf("%d\n", (*(node->GetValue())));

        for (int i = 0; i < 2; i++)
        {
            Dump(node->GetChildAt(i), tabs + 1);
        }
    }
#endif

private:
    BSTNode* NewNode(const T& data, BSTNode* parent = null)
    {
#ifndef HEAP_ALLOC
        BSTNode* newNode = Anew(this->alloc, BSTNode);
#else
        BSTNode* newNode = new BSTNode();
#endif
        newNode->SetValue(data);
        newNode->SetParent(null);
        return newNode;
    }

    // Find the smallest child of a non-leaf node
    BSTNode* FindMin(BSTNode* node)
    {
        BSTNode* current = node;

        while (current->GetChildAt(LeftNode))
        {
            current = current->GetChildAt(LeftNode);
        }

        return current;
    }

    // Replaces node in node->parent with replacement
    void ReplaceNodeInParent(BSTNode* node, BSTNode* replacement)
    {
        BSTNode* parent = node->GetParent();

        if (parent != null)
        {
            if (node == parent->GetChildAt(LeftNode))
            {
                parent->SetChildAt(LeftNode, replacement);
            }
            else
            {
                parent->SetChildAt(RightNode, replacement);
            }
        }
        else
        {
            root = replacement;
        }

        if (replacement != null)
        {
            replacement->SetParent(parent);
        }
    }

#ifndef HEAP_ALLOC
    ArenaAllocator* alloc;
#endif
    BSTNode* root;
};
