var StringHashSmallSize=7;
var StringHashMediumSize=43;
var StringHashLargeSize=97;

function StringHashEntry(key,data) {
    this.key=key;
    this.data=data;
    this.next=null;
}

function StringHashTable(size) {
    this.size=size;
    this.count=0;
    this.table=new Array(size);
    for (var i=0;i<size;i++) {
	this.table[i]=null;
    }
}

function StringHashFn(key) {
    var val=0;
    for (var i=0;i<key.length;i++) {
	val=(val<<i)^key.charCodeAt(i);
    }
    return val;
}

StringHashTable.prototype = {
    add:function(key,data) {
	var current;
	var entry=new StringHashEntry(key,data);
        var val=StringHashFn(key);
        val = val % this.size;

        for (current = this.table[val] ; current != null ; current = current.next) {
            if (key==current.key) {
                return false;
            }
        }
        entry.next = this.table[val];
        this.table[val] = entry;
        this.count++;
        return true;
    },
    count:function() { return this.count; },
    lookup:function(key) {
	var current;
	var val=StringHashFn(key);
        val = val % this.size;
        for (current = this.table[val] ; current != null ; current = current.next) {
            if (key==current.key) { 
                return(current.data);
	    }
	}
        return(null);
    }
};
