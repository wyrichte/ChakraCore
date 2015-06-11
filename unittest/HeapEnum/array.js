var HETest = {};

HETest.newEmptyArrayLength5000 = new Array();
HETest.length = 5000;

HETest.smallArrayWithNumbers= new Array(1,2,3);

HETest.smallArrayWithMultipleTypes = new Array("y_1", 2, "y_3", "y_4");
HETest.smallArrayWithMultipleTypes.property1 = "Array property 1";
HETest.smallArrayWithMultipleTypes.property2 = HETest.smallArrayWithNumbers;
HETest.smallArrayWithMultipleTypes.property3 = new Array("embedded array element 1");

HETest.delete2ndElem = new Array("y_1", 2, "y_3", "y_4");
delete HETest.delete2ndElem[2];

HETest.z = new Array(HETest.smallArrayWithMultipleTypes, "Second element", new Object());

HETest.sparse1 = new Array();
HETest.sparse1[1000] = "Single element in sparse array";

HETest.sparse2 = new Array();
HETest.sparse2[10] = "Index 10 element in sparse array";
HETest.sparse2[100] = "Index 100 element in sparse array";
HETest.sparse2[1000] = "Index 1000 element in sparse array";

HETest.sparseAllDeleted = new Array();
HETest.sparseAllDeleted[1000] = "Single element in 2nd sparse array";

HETest.sparse3 = new Array();
HETest.sparse3[10] = "Index 10 element in sparse array 3";
HETest.sparse3[100] = "Index 100 element in sparse array 3";
HETest.sparse3[1000] = "Index 1000 element in sparse array 3";
delete HETest.sparse3[100];

delete HETest.sparseAllDeleted[HETest.sparseAllDeleted.length - 1];

HETest.nativeIntArray = [1, 2, 3, 4];
HETest.nativeFloatArray = [1.1, 2.1, 3.1, 4.1];

function guarded_call(func) {
    try {
        func();
    } catch (e) {
    }
}

var a = [];
guarded_call(function () {
    var base = 4294967290;
    for (var i = 0; i < 10; i++) {
        a[base + i] = 100 + i;
    }
    delete a[base + 3];
    a.unshift(200, 201, 202, 203);
});

HETest.unshifted = a;

Debug.dumpHeap(HETest, true);


