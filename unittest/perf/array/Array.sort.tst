var IM = 139968;
var IA = 3877;
var IC = 29573;
var last = 42;

function genRandom(max) 
{ 
    return(max * (last = (last * IA + IC) % IM) / IM); 
}

function sortOrder(num1, num2)
{
    return (num1 - num2);
}

var arr1 = null;

function PreTestArraySort1()
{
    var count = TestArraySort1.count;
    arr1 = new Array(count);

    for (var i=0; i<count; ++i) {
        arr1[i] = genRandom(1.0);
    }
}

function TestArraySort1()
{
   arr1.sort(sortOrder);
}

TestArraySort1.count = 10000;
TestArraySort1.testId = "Array.sort.1";
TestArraySort1.description = "array.sort(sortOrder) (" + TestArraySort1.count + ")";
TestArraySort1.iterations = 1000;
TestArraySort1.quantifier = 100;
TestArraySort1.preCondition = PreTestArraySort1;
Register(TestArraySort1);


var arr2 = null;

function PreTestArraySort2()
{
    var count = TestArraySort2.count;
    arr2 = new Array(count);

    for (var i=0; i<count; ++i) {
        arr2[i] = genRandom(1.0);
    }
}

function TestArraySort2()
{
   arr2.sort();
}

TestArraySort2.count = 10000;
TestArraySort2.testId = "Array.sort.2";
TestArraySort2.description = "array.sort() (" + TestArraySort2.count + ")";
TestArraySort2.iterations = 10;
TestArraySort2.quantifier = 10;
TestArraySort2.preCondition = PreTestArraySort2;
Register(TestArraySort2);
