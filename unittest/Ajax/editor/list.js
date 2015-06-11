function List(isHead,data) {
    this.isHead=isHead;
    this.next=null;
    this.prev=null;
    this.data=data;
}

function ListMakeEntry(data) {
    var entry=new List(false,data);
    entry.prev=entry;
    entry.next=entry;
    return entry;
}

function ListMakeHead() {
    var entry=new List(true,null);
    entry.prev=entry;
    entry.next=entry;
    return entry;
}

function ListAdd(head,data) {
    var entry=ListMakeEntry(data);
    head.prev.next=entry;
    entry.next=head;
    entry.prev=head.prev;
    head.prev=entry;
    return(entry);
}

function ListCount(head) {
    var entry;
    var i;

    entry=head.next;
    for (i=0;!(entry.isHead);i++) {
	entry=entry.next;
    }
    return(i);
}

function ListFirst(head) {
   if (!ListEmpty(head))
        return(head.next.data);
   else return null;

}

function ListEmpty(head) {
   return(head.next==head);
}

function ListPushEntry(head,entry) {
    entry.isHead=false;
    entry.next = head.next;
    entry.prev = head;
    head.next = entry;
    entry.next.prev = entry;
}

function ListPush(head,data) {
    var entry=ListMakeEntry(data);
    entry.data=data;
    entry.isHead=false;
    entry.next = head.next;
    entry.prev = head;
    head.next = entry;
    entry.next.prev = entry;
}

function ListPopEntry(head) {
    if (head.next.isHead)
	return(null);
    else return(ListRemoveEntry(head.next));
}

function ListInsertEntry(head,entry) {
    entry.isHead=false;
    head.prev.next=entry;
    entry.next=head;
    entry.prev=head.prev;
    head.prev=entry;
    return entry;
}

function ListInsertAfter(before,data) {
    var entry=ListMakeEntry(data);    
    entry.next = before.next;
    entry.prev = before;
    before.next = entry;
    entry.next.prev = entry;
    return(entry);
}

function ListInsertBefore(after,data) {
    var entry=ListMakeEntry(data);
    return ListInsertEntryBefore(after,entry);
}

function ListInsertEntryBefore(after,entry) {
    after.prev.next=entry;
    entry.next=after;
    entry.prev=after.prev;
    after.prev=entry;
    return(entry);
}

function ListRemoveEntry(entry) {
    if (entry == null)
	return null;
    else if (entry.isHead)
	return null;
    else {
	entry.next.prev = entry.prev;
	entry.prev.next = entry.next;
    }
    return(entry);
}


