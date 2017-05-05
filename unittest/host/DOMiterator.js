if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in
  this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

function iterate(iter) {
    var actual = [];
    var retVal = iter.next();
    while(!retVal.done)
    {
        actual.push(retVal.value);
        retVal = iter.next();
    }

    return actual;
}

// Since the map/set return values out of order. we will validate the individual one.
function areEqual(expected, actual, message) {
    for (var i of actual) {
        var found = false;
        if (Array.isArray(i)) {
            for (var j in expected) {
                if (expected[j][0] == i[0]) {
                    found = (expected[j][1] == i[1]);
                    break;
                }
            }
        }
        else{
            found = expected.includes(i);
        }
        
        if (!found) {
            assert.fail(i + ' ' +message);
        }
    }
}

var tests = [
  {
    name: "Mock DOM array object and basic methods",
    body: function () {
        var obj = CreateDomArrayObject();
        obj.AddObject(101);
        obj.AddObject(102);
        assert.areEqual(obj.ItemAt(0), 101);
        assert.areEqual(obj.ItemAt(1), 102);
        assert.areEqual(obj[0], 101);
        assert.areEqual(obj[1], 102);
        assert.areEqual(obj.length, 2);
    }
  },
  {
    name: "MockDOM array object - foreach, entries, values, keys methods",
    body: function () {
        var obj = CreateDomArrayObject();
        obj.AddObject(201);
        obj.AddObject(202);
        obj.AddObject('end');
        assert.areEqual(obj.length, 3);
        
        var expected = [201, 202, 'end'];
        
        var actual = [];
        obj.forEach(function(aa) {
            actual.push(aa);
        });
        
        assert.areEqual(expected, actual, "validation after forEach");

        var iter = obj.keys();
        actual = iterate(iter);
        assert.areEqual([0, 1, 2], actual, "validation after keys");
        
        iter = obj.values();
        actual = iterate(iter);
        assert.areEqual(expected, actual, "validation after values");
        
        iter = obj.entries();
        actual = iterate(iter);
        assert.areEqual([[0, 201], [1, 202], [2, 'end']], actual, "validation after entries");
    }
  },
  {
    name: "MockDOM array object - symbol.iterator and for..of loop",
    body: function () {
        var obj = CreateDomArrayObject();
        obj.AddObject(301);
        obj.AddObject(302);
        obj.AddObject('end');
        
        var iter = obj[Symbol.iterator]();
        var actual = iterate(iter);
        assert.areEqual([301, 302, 'end'], actual, "validation after Symbol.iterator");
        
        actual = [];
        for(var i of obj) {
            actual.push(i);
        }

        assert.areEqual([301, 302, 'end'], actual, "validation after for..of loop");
    }
  },
  {
    name: "MockDOM Map object - entries, values, keys methods and for..of validation",
    body: function () {
        var obj = CreateDomMapObject();
        obj.Set('test1','value1');
        obj.Set('key2','value2');
        obj.Set('key3', 100);
        assert.areEqual(obj.Get('test1'), 'value1');
        assert.areEqual(obj.Get('key2'), 'value2');
        assert.areEqual(obj.Get('key3'), 100);
        
        var iter = obj.keys();
        var actual = iterate(iter);
        areEqual(['key2', 'key3', 'test1'], actual, "validation after keys");
        
        iter = obj.values();
        actual = iterate(iter);

        areEqual(['value2', 100, 'value1'], actual, "validation after values");
        
        iter = obj.entries();
        actual = iterate(iter);
        areEqual([['key2', 'value2'], ['key3', 100], ['test1', 'value1']], actual, "validation after entries");
        
        actual = [];
        for(var i of obj) {
            actual.push(i);
        }

        areEqual([['key2', 'value2'], ['key3', 100], ['test1', 'value1']], actual, "validation after for..of loop");
        
        assert.areEqual(0, obj.entries.length);
        assert.areEqual(0, obj.keys.length);
        assert.areEqual(0, obj.values.length);
        assert.areEqual(0, obj.entries().next.length);
    }
  },
  {
    name: "MockDOM Set object - for..of validation",
    body: function () {
        var obj = CreateDomSetObject();
        obj.Set('test1','value1');
        obj.Set('key2','value2');
        obj.Set('key3', 100);
        
        var actual = [];
        for(var i of obj) {
            actual.push(i);
        }

        areEqual([['key2', 'value2'], ['test1', 'value1'], ['key3', 100]], actual, "validation after for..of loop");
        
        // Other function validations are not required as Set is similar to Map, just different type id
    }
  },
  {
    name: "Mock objects error handling",
    body: function () {
        var map = CreateDomMapObject();
        var set = CreateDomSetObject();
        assert.areNotEqual(map.__proto__, set.__proto__);
        
        map.Set('k1', 'v1');
        set.Set('k2', 'v2');
        
        [undefined, null, {}, [], new Map(), set].forEach(function(item) {
             assert.throws(() => { map.__proto__.entries.call(item); }, TypeError, "Calling entries with different iterable object is invalid", "Invalid iterable object");
             assert.throws(() => { map.__proto__.keys.call(item); }, TypeError, "Calling keys with different iterable object is invalid", "Invalid iterable object");
             assert.throws(() => { map.__proto__.values.call(item); }, TypeError, "Calling values with different iterable object is invalid", "Invalid iterable object");
        });

        [undefined, null, {}, [], new Map(), map].forEach(function(item) {
             assert.throws(() => { set.__proto__.entries.call(item); }, TypeError, "Calling entries with different iterable object is invalid", "Invalid iterable object");
             assert.throws(() => { set.__proto__.keys.call(item); }, TypeError, "Calling keys with different iterable object is invalid", "Invalid iterable object");
             assert.throws(() => { set.__proto__.values.call(item); }, TypeError, "Calling values with different iterable object is invalid", "Invalid iterable object");
        });
        
        var iter1 = map.entries();
        var iter2 = set.entries();

        [undefined, null, [1].entries(), (new Map()).entries(), iter2].forEach(function(item) {
             assert.throws(() => { iter1.__proto__.next.call(item); }, TypeError, "Calling next with different iterator object is invalid", "Iterator.prototype.next : Invalid iterator object");
        });

        [undefined, null, [1].entries(), (new Map()).entries(), iter1].forEach(function(item) {
             assert.throws(() => { iter2.__proto__.next.call(item); }, TypeError, "Calling next with different iterator object is invalid", "Iterator.prototype.next : Invalid iterator object");
        });
     }
  },
  {
    name: "Mock object's mutation validation",
    body: function () {
        var map = CreateDomMapObject();
        map.Set('k1', 'v1');
        
        var actual = [];
        var first = true;
        for(var val of map) {
            actual.push(val[0]);
            if (first) {
                map.Set('k2', 2);
                first = false;
            }
        }
        
        areEqual(['k1', 'k2'], actual, "validation after mutation happened in for..of loop");
     }
  },
  {
    name: "Mock object's iterator prototype validation",
    body: function () {
        var map = CreateDomMapObject();
        map.Set('k1', 'v1');
        
        var keys = map.keys();
        var fn = keys.__proto__.__proto__[Symbol.iterator];
        assert.areEqual("function [Symbol.iterator]() { [native code] }", fn.toString(), "Validating that dom iterator has %IteratorPrototype% in the chain");
     }
  },
  
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
