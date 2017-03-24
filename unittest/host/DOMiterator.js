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
  
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
