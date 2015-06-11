// Copyright (C) Microsoft. All rights reserved. 

#pragma once

namespace JsUtil
{
    template<class T>
    DoublyLinkedList<T>::DoublyLinkedList() : head(null), tail(null)
    {
    }

    template<class T>
    T *DoublyLinkedList<T>::Head() const
    {
        return head;
    }

    template<class T>
    T *DoublyLinkedList<T>::Tail() const
    {
        return tail;
    }

    template<class T>
    bool DoublyLinkedList<T>::Contains(T *const element) const
    {
        return T::Contains(element, head);
    }

    template<class T>
    bool DoublyLinkedList<T>::ContainsSubsequence(T *const first, T *const last) const
    {
        return T::ContainsSubsequence(first, last, head);
    }

    template<class T>
    bool DoublyLinkedList<T>::IsEmpty()
    {
        return head == null;
    }

    template<class T>
    void DoublyLinkedList<T>::Clear()
    {
        tail = head = null;
    }

    template<class T>
    void DoublyLinkedList<T>::LinkToBeginning(T *const element)
    {
        T::LinkToBeginning(element, &head, &tail);
    }

    template<class T>
    void DoublyLinkedList<T>::LinkToEnd(T *const element)
    {
        T::LinkToEnd(element, &head, &tail);
    }

    template<class T>
    void DoublyLinkedList<T>::LinkBefore(T *const element, T *const nextElement)
    {
        T::LinkBefore(element, nextElement, &head, &tail);
    }

    template<class T>
    void DoublyLinkedList<T>::LinkAfter(T *const element, T *const previousElement)
    {
        T::LinkAfter(element, previousElement, &head, &tail);
    }

    template<class T>
    T *DoublyLinkedList<T>::UnlinkFromBeginning()
    {
        T *const element = head;
        if(element)
            T::UnlinkFromBeginning(element, &head, &tail);
        return element;
    }

    template<class T>
    T *DoublyLinkedList<T>::UnlinkFromEnd()
    {
        T *const element = tail;
        if(element)
            T::UnlinkFromEnd(element, &head, &tail);
        return element;
    }

    template<class T>
    void DoublyLinkedList<T>::UnlinkPartial(T *const element)
    {
        T::UnlinkPartial(element, &head, &tail);
    }

    template<class T>
    void DoublyLinkedList<T>::Unlink(T *const element)
    {
        T::Unlink(element, &head, &tail);
    }

    template<class T>
    void DoublyLinkedList<T>::MoveToBeginning(T *const element)
    {
        T::MoveToBeginning(element, &head, &tail);
    }

    template<class T>
    void DoublyLinkedList<T>::UnlinkSubsequenceFromEnd(T *const first)
    {
        T::UnlinkSubsequenceFromEnd(first, &head, &tail);
    }

    template<class T>
    void DoublyLinkedList<T>::UnlinkSubsequence(T *const first, T *const last)
    {
        T::UnlinkSubsequence(first, last, &head, &tail);
    }

    template<class T>
    void DoublyLinkedList<T>::MoveSubsequenceToBeginning(T *const first, T *const last)
    {
        T::MoveSubsequenceToBeginning(first, last, &head, &tail);
    }
}
