// This test is lifted (with a small tweak) from Octane's Splay benchmark.
// In the insert method inline caches for left, right and middle are shared by
// property ID.  When condition is false we always add right, then left, and 
// finally middle.  When condition becomes true we attempt to add left, then
// right.  But the cache for left has a mismatching inline cache, so we don't
// hard-code this property add.  The remaining right and middle match and we
// do hard-code.  The problem is that the unoptimized node.left changes the type
// of node, which we fail to notice, because we didn't flag this operation as
// potentially altering the type.  That's because it was an object type spec
// candidate that we just decided not to optimize.

// Run with -bgJit- -maxInterpretCount:2

var SplayTreeNode = function (key, value) {
    this.key = key;
    this.value = value;
};


var SplayTree = function () {
}

SplayTree.prototype.insert = function (condition) {
    var node = new SplayTreeNode(0, 0);
    if (condition) {
        node.left = "left";
        node.right = "right";
        node.middle = "middle";
    }
    else {
        node.right = "right";
        node.left = "left";
        node.middle = "middle";
    }
    return node;
}

var splayTree = new SplayTree();

var node;

node = splayTree.insert(false);
WScript.Echo("{ left: " + node.left + ", middle: " + node.middle + ", right: " + node.right + "}");

node = splayTree.insert(false);
WScript.Echo("{ left: " + node.left + ", middle: " + node.middle + ", right: " + node.right + "}");

node = splayTree.insert(true);
WScript.Echo("{ left: " + node.left + ", middle: " + node.middle + ", right: " + node.right + "}");
