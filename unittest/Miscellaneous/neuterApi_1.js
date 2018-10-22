// Testing that SCA based detaching and ScriptEngine based detaching works well together.

function debugDetach(arr) {
    Debug.detachAndFreeObject(arr);
}
function neuter(arr) {
    const trans = [arr];
    var blob = SCA.serialize(arr,{ context: "samethread" } , undefined, trans);
    var transferedArray = SCA.deserialize(blob);
    return transferedArray;
}

function test1() {
    const arr = new ArrayBuffer(0x2000);
    var transferedArray = neuter(arr);
    debugDetach(transferedArray);
}

function test2() {
    const arr = new ArrayBuffer(0x2000);
    var transferedArray = neuter(arr);
    transferedArray = neuter(transferedArray);
    debugDetach(transferedArray);
}

test1();
test2();
print('pass');
