// Map (et al) is no longer allowed to be called as a function unless the object it is given
// for its this argument already has the [[MapData]] property on it.
// TODO: When we implement @@create support, update this test to reflect it.
/*
// Try making the global object a map and make sure HeapEnum finds the map data
Map.call(this);

var globalmap = this;
var wrapper = {
    set: function(k, v) { Map.prototype.set.call(globalmap, k, v); }
};

initmap(wrapper);

Debug.dumpHeap(this, true, true);
*/

// Simple verification of HeapEnum of Map, Set, and WeakMap
var m = new Map();
var s = new Set();
var wm = new WeakMap();
var ws = new WeakSet();

function initmap(m) {
    var o = { a: 1, b: 2, c: 3 };
    var p = "hello";
    var q = { x: 50, y: 100 };
    var r = 1023;

    m.set(o, "o");
    m.set(p, "p");
    m.set(q, "q");
    m.set(r, "r");
}

function initset(s) {
    var o = { e: 4, f: 5, g: 6 };
    var p = "goodbye";
    var q = { x: 60, y: 200 };
    var r = 65535;

    s.add(o);
    s.add(p);
    s.add(q);
    s.add(r);
}

function initweakmap(wm, a, b, c, d) {
    var o = { w: 1, e: 2, a: 3, k: 4 };
    var p = "weak";
    var q = { m: 5, a: 6, p: 7};
    var r = 511;

    wm.set(a, o);
    wm.set(b, p);
    wm.set(c, q);
    wm.set(d, r);
}

function initweakset(ws, a, b, c, d) {
    ws.add(a);
    ws.add(b);
    ws.add(c);
    ws.add(d);
}

var a = { a: "a" };
var b = { b: "b" };
var c = { c: "c" };
var d = { d: "d" };

initmap(m);
initset(s);
initweakmap(wm, a, b, c, d);
initweakset(ws, a, b, c, d);

var HETest = {
    m: m,
    s: s,
    // WeakMap enumerates in non-deterministic order and so does
    // not always produce the same output for baseline comparison.
    // Ditto for WeakSet.
    // TODO: determine how to test this
    //wm: wm,
    //ws: ws,
};

Debug.dumpHeap(HETest, true, true);
