async function asyncFactorial(num) { // async factorial
    var result;
    if (num == 1)
        result = 1;
    else
        result = num * await asyncFactorial(num - 1);
    return result;
}

var asyncFactorialLambda = async(num) => num == 1 ? 1 : num * await asyncFactorialLambda(num - 1);

var HETest = {
    asyncMethod: asyncFactorial,
    asyncMethodPromise: asyncFactorial(5)
};

Debug.dumpHeap(HETest, true, true);