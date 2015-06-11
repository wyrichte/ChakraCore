function Dump(x, dump)
{
    //dump = true;
    if (dump != undefined)
    {
        WScript.Echo(x);
    }
}

var HETest = {};

HETest.x = new Object({p1: 1, p2: 2, p3: 3});

HETest.y = new Object();
HETest.y[0] = "y_1";
HETest.y[1] = 2;
HETest.y[2] = "y_3";
HETest.y[3] = "y_4";
HETest.y.property1 = "Object name property 1";
HETest.y.property2 = HETest.x;
HETest.y.property3 = new Object("embedded array element 1");

HETest.delete2ndIndexProp = new Object({prop1: HETest.y, prop2: "Second name property", prop3: new Object() });
HETest.delete2ndIndexProp[0] = "y_1";
HETest.delete2ndIndexProp[1] = 2;
HETest.delete2ndIndexProp[3] = "y_3";
HETest.delete2ndIndexProp[3] = "y_4";
delete HETest.delete2ndIndexProp[2];

HETest.delete2ndNameProp = new Object({prop1: HETest.y, prop2: "Second name property", prop3: new Object() });
HETest.delete2ndNameProp[0] = "y_1";
HETest.delete2ndNameProp[1] = 2;
HETest.delete2ndNameProp[3] = "y_3";
HETest.delete2ndNameProp[3] = "y_4";
delete HETest.delete2ndNameProp.prop2;

HETest.z = new Object( {prop1: HETest.y, prop2: "Second name property", prop3: new Object() });

HETest.sparse = new Object();
HETest.sparse[1000] = "Single element in sparse array";

HETest.sparse2 = new Object();
HETest.sparse[10] = "Index 10 element in sparse array";
HETest.sparse[100] = "Index 100 element in sparse array";
HETest.sparse[1000] = "Index 1000 element in sparse array";

HETest.sparseAllDeleted = new Object();
HETest.sparseAllDeleted[1000] = "Single element in 2nd sparse array";
delete HETest.sparseAllDeleted[HETest.sparseAllDeleted.length-1];

HETest.protoObject = { fooprop1: "foo prop 1", fooprop2: "foo prop 2" };
HETest.CtorFunctionNewProto = function () {this.pProp1 = "CtorFunctionNewProto Prop1";}
HETest.CtorFunctionNewProto.prototype = HETest.protoObject;
HETest.newCtorFunctionNewProto = new HETest.CtorFunctionNewProto();
HETest.CtorFunctionSameProto = function() {this.qprop1 = "CtorFunctionSameProto prop1"; }
HETest.newCtorFunctionSameProto = new HETest.CtorFunctionSameProto();

Dump(HETest.newCtorFunctionNewProto.fooprop1);
Dump(HETest.newCtorFunctionSameProto.fooprop1);

Debug.dumpHeap(HETest, true);

