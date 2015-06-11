function userCode() {
    abc.def = 10;
}

var testF1 = 10;
function testF2() {
    Debug.setNonUserCodeExceptions = true;
    testF1 = function () {
        try {
            userCode();
        }
        catch(e) {
        }
    }
    
    var k = 100;
    try {
        userCode();
    }
    catch(e) {
        k+= 10;
    }
    return k;
}

function testF3() {
    try {
        userCode();
    }
    catch(e) {
    }
    return 10;
}

function testF4() {
    Debug.setNonUserCodeExceptions = true;
    var k1 = 100;
    try {
        kkk.mmm = 10;
    }
    catch (e) {
        k1 += 10;
    }
    return k1;
}

testF2();
testF1();
testF3();
testF4();
