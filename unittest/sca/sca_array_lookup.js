function createNestedArray(level) {
    var toReturn = [];
    var current = toReturn;
    for (var i = 1; i < level ; i++) {
        var newArray = [];
        current.push(new Date());
        current.push(i);
        current.push("a");
        current.push(newArray);
        current.push("b");
        current.push(i * -1);
        current.push(new Date());
        current = newArray;
    }

    return toReturn;
}
var recursionDepth = 1500;
var b = createNestedArray(recursionDepth);
var data = SCA.serialize({ a: b });
var result = SCA.lookupEx(data, [["a"]]);

var current = b;
for (var i = 1; i < recursionDepth; i++) {
    if (!(current[0] instanceof Date && current[1] === Number(current[1]) && current[2] === current[2].toString()
            && current[3] instanceof Array &&
            current[6] instanceof Date && current[5] === Number(current[5]) && current[4] === current[4].toString())) {
        throw new Error("Incorrect key at depth: " + i);
    }
    current = current[3];
}

function doLookup(obj, lookup){
  try {
    var data = SCA.serialize(obj);
    var res = SCA.lookupEx(data, lookup);
    WScript.Echo(JSON.stringify(res));
    return res;
  } catch (e) {
    WScript.Echo(e);
  }
  finally{
  }
}

WScript.Echo();
WScript.Echo("Test Valid Keys");
doLookup({a:[1, 2, 3]}, "a");
doLookup({a:[1, new Date(0), 3]}, "a");
doLookup({a:[1, 2, "sometest"]}, "a");
doLookup({a:[[new Date(0), 2, "3"], 2, 3]}, "a");
var withProp = [1, 2, 3];
withProp.test = 5;
doLookup({a:withProp}, "a");
doLookup({a:{b:[1, [2, [3]]] }}, ["a", "b"]);
var sameRef = [0];
var returned = doLookup({a:[[1, sameRef], [2, sameRef]]}, "a");
if(returned[0][1] !== returned[1][1])
{
  WScript.Echo("Two arrays pointing to the same one isn't preserved on lookup.");
}

WScript.Echo();
WScript.Echo("Test Cycles");
var arr = [1, 2, 3];
arr.push(arr);
doLookup({a:arr}, "a");
doLookup({a:[0, arr]}, "a");

var newArr = [1, 2, 3];
newArr.push([4, 5, newArr]);
doLookup({a:newArr}, "a");
doLookup({a:[newArr]}, "a");

WScript.Echo();
WScript.Echo("Test Invalid Keys");
var sparseArray = [];
sparseArray[1000] = 5;
doLookup({a:sparseArray}, "a");
doLookup({a:[{}]}, "a");
doLookup({a:[1, 2, undefined]}, "a");
doLookup({a:[1, 2, null]}, "a");
var unsetArray = [];
unsetArray[0] = 1;
unsetArray[2] = 2;
doLookup({a:unsetArray}, "a");


function doKeyToStreamToKey(obj) {
    try {
        var data = SCA.serialize(obj);
        return SCA.dataToKey(data);
    } catch (ex) {
        WScript.Echo(ex);
    }
}

WScript.Echo(JSON.stringify(doKeyToStreamToKey(["as", [1, new Date(2014, 7, 13), 2.3]])));
WScript.Echo(JSON.stringify(doKeyToStreamToKey(["as", [1, undefined, 2.3]])));
var a = [1, 2, 3];
a.push(a);
WScript.Echo(JSON.stringify(doKeyToStreamToKey(a)));
WScript.Echo(JSON.stringify(doKeyToStreamToKey("a")));
WScript.Echo(JSON.stringify(doKeyToStreamToKey(1)));
WScript.Echo(JSON.stringify(doKeyToStreamToKey(new Date(2014, 7, 13))));
WScript.Echo(JSON.stringify(doKeyToStreamToKey([])));