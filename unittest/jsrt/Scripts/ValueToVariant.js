function BaseTest() {
    this.num = 123456;
    this.str = "hello";
    this.bool = true;
    this.func = function (arg1, arg2) {
            return arg1 + arg2;
    }    
}

function Test() {
    this.num1 = 3.141592;
    this.str1 = "world";    
}

Test.prototype = new BaseTest();
var test = new Test();