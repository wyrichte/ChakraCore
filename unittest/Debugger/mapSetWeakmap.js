function test1()
{
    var m = new Map();
    var s = new Set();
    var wm = new WeakMap();
    var ws = new WeakSet();

    m;
    // evaluate to depth 3 to verify that there are no children when
    // the map/set/weakmap/weakset is empty with size == 0
    /**bp:locals(3)**/

    m.set(5, { a: 1, b: 2, c: 3 });
    m.set(m, m);
    m.set(63, new Number(42));

    s.add(5);
    s.add(s);
    s.add(3.14);

    /*
     * WeakMap keys are hashed by their memory addresses and thus go into the hashtable in random order.
     * They have no particular deterministic enumeration order and thus if we try to test the debugger
     * display of a WeakMap by baseline comparison, we will some times get failures due to the entries
     * showing up in a different order.
     * Ditto for WeakSet
     *
     * So instead of testing with multiple keys we can test with a single entry in the WeakMap/WeakSet
     * so that the baseline is stable
     * 
     *
     * For now we will not include this testing in the unittests.
    */
    
    wm.set(m, 5);
    ws.add(m);

    m;
    /**bp:locals(4)**/

    m.delete(m);
    s.delete(s);    
    wm.delete(m);
    ws.delete(m);

    m;
    /**bp:locals(4)**/

    m.set("another", "entry");
    s.add("abc");
   
    var p = { x: "some", y: "other", z: "object" };
    wm.set(p, "bar");
    ws.add(p);

    m;
    /**bp:locals(4)**/
}

function test2()
{
    var map = new Map();
    var set = new Set();
    var weakmap = new WeakMap();
    var weakset = new WeakSet();

    // Map, Set, and WeakMap are no longer allowed to be called as functions unless
    // the object they are given for their this argument already has the corresponding
    // [[MapData]], [[SetData]], or [[WeakMapData]] property on it.
    /*
    var x = {};
    Map.call(x);
    Set.call(x);
    WeakMap.call(x);

    // x should show up with type "Object (Map)"
    x.__proto__ = Map.prototype;

    var y = {};
    Map.call(y);
    Set.call(y);
    WeakMap.call(y);

    // y should show up with type "Object (Set)"
    y.__proto__ = Set.prototype;

    var z = {};
    Map.call(z);
    Set.call(z);
    WeakMap.call(z);

    // z should show up with type "Object (WeakMap)"
    z.__proto__ = WeakMap.prototype;

    var w = {};
    Map.call(w);
    Set.call(w);
    WeakMap.call(w);
    */

    // Do not change w's prototype; should show type as just "Object"

    map; /**bp:evaluate("map", 0, LOCALS_TYPE),
             evaluate("set", 0, LOCALS_TYPE),
             evaluate("weakmap", 0, LOCALS_TYPE),
             evaluate("weakset", 0, LOCALS_TYPE),
             evaluate("Map.prototype", 0, LOCALS_TYPE),
             evaluate("Set.prototype", 0, LOCALS_TYPE),
             evaluate("WeakMap.prototype", 0, LOCALS_TYPE),
             evaluate("WeakSet.prototype", 0, LOCALS_TYPE)**/
    /* ** TODO re-enable
             evaluate("x", 0, LOCALS_TYPE),
             evaluate("y", 0, LOCALS_TYPE),
             evaluate("z", 0, LOCALS_TYPE),
             evaluate("w", 0, LOCALS_TYPE),
             **/
}

var o = { };
o.__proto__ = null;

test1.apply(o);
test2.apply(o);

WScript.Echo("pass");
